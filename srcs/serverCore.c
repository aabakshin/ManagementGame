/* Файл реализации модуля serverCore */

#ifndef SERVER_CORE_C
#define SERVER_CORE_C

#include "../includes/serverCore.h"
#include "../includes/MGLib.h"
#include "../includes/CommandsHandler.h"
#include "../includes/MarketRequest.h"
#include "../includes/Player.h"

/* Системные константы */
enum
{
	LISTEN_QUEUE_LEN			=						  5,
	START_MONEY					=					  10000,
	START_SOURCES				=						  4,
	START_PRODUCTS				=					      2,
	START_FACTORIES				=					      2,
	SOURCE_UNIT_CHARGE			=						300,
	PRODUCT_UNIT_CHARGE			=						500,
	FACTORY_UNIT_CHARGE			=					   1000,
	NEW_FACTORY_UNIT_COST		=					   5000
};

/* Константы для нумерации индексов валидных команд(порядок должен совпадать с порядком следования элементов в valid_commands) */
enum
{
	HELP_COMMAND_NUM,
	MARKET_COMMAND_NUM,
	PLAYER_COMMAND_NUM,
	LIST_COMMAND_NUM,
	PROD_COMMAND_NUM,
	BUILD_COMMAND_NUM,
	BUY_COMMAND_NUM,
	SELL_COMMAND_NUM,
	TURN_COMMAND_NUM,
	QUIT_COMMAND_NUM
};

/* Список валидных команд, которые понимает сервер */
const char* valid_commands[] = {
				"help",
				"market",
				"player",
				"list",
				"prod",
				"build",
				"buy",
				"sell",
				"turn",
				"quit",
				NULL
};

/* Список функций-обработчиков валидных команд(порядок должен совпадать с порядком следования элементов в valid_commands) */
void (*commands_handlers[])(Banker*, CommandHandlerParams* ) = {
					help_command_handler,
					market_command_handler,
					player_command_handler,
					list_command_handler,
					prod_command_handler,
					build_command_handler,
					buy_command_handler,
					sell_command_handler,
					turn_command_handler,
					quit_command_handler,
					NULL
};

/* Описаны в модуле Banker */
extern double amount_multiplier_table[MARKET_LEVEL_NUMBER][2];
extern int price_table[MARKET_LEVEL_NUMBER][2]; 
extern const int states_market_chance[MARKET_LEVEL_NUMBER][MARKET_LEVEL_NUMBER];

/* Флаг срабатывания таймера */
static int alrm_flag = 0;

/* Флаг завершения работы программы */
static int exit_flag = 0;

/* Номер полученного сигнала */
static int sig_number = 0;

/* Флаг о готовности всех игроков к игре */
static int players_prepared = 0;

/* Флаг установки таймера */
static int timer_set = 0;

/* Счётчик событий в логе сервера */
int log_info_count = 0;

/* Ф-я-обработчик сигнала SIGALRM */
void alrm_handler(int sig_no)
{
	int save_errno = errno;
	signal(SIGALRM, alrm_handler);
	
	sig_number = sig_no;
	alrm_flag = 1;
	alarm(0);

	errno = save_errno;
}

/* Ф-я-обработчик сигнала SIGINT */
void exit_handler(int sig_no)
{
	int save_errno = errno;
	signal(SIGINT, exit_handler);
		
	sig_number = sig_no;
	exit_flag = 1;

	errno = save_errno;
}

/* Ф-я отправки данных на клиентский сокет */
int send_data(int fd, const char* send_buf, int mes_len, const char* ip)
{
	int success_flag = 1;

	log_info_count++;
	printf("\n==================== (%d) ====================\n", log_info_count);
	printf("buffer = %s\n", send_buf);
	int wc = write(fd, send_buf, mes_len);
	
	if ( wc < 0 )
		success_flag = 0;
	
	printf("Sent to [%s] %d\\%d bytes\n", ip, wc, mes_len);
	printf("==================== (%d) ====================\n\n", log_info_count);
	
	if ( !success_flag )
		return 0;

	return 1;
}

/* Ф-я подготовки и отправки сообщения на клиентский сокет */
int send_message(int fd, char** message_tokens, int tokens_amount, const char* ip)
{
	if ( 
			( fd < 0 )						|| 
			( message_tokens == NULL )		||
			( *message_tokens == NULL )		||
			( tokens_amount < 1 )			||
			( ip == NULL )
	   )
		return 0;


	char buffer[BUFSIZE];
	int i, j, k;

	for ( i = 0; message_tokens[0][i]; i++ )
		buffer[i] = message_tokens[0][i];
	
	if ( tokens_amount > 1 )
	{
		for ( j = 1; j < tokens_amount; j++ )
		{
			for ( k = 0; message_tokens[j][k]; k++, i++ )
				buffer[i] = message_tokens[j][k];
			buffer[i] = '|';
			i++;
		}
	}
	
	buffer[i-1] = '\n';
	buffer[i] = '\0';
	int mes_len = i;
	
	if ( !send_data(fd, buffer, mes_len, ip) )
		return 0;
	
	return 1;
}

/* Вспомогательная функция для функций-обработчиков команд */
static int make_cmd_param(char* buf, int buf_size, char* str)
{
	if ( (buf == NULL) || (str == NULL) || (buf_size < 1) )
		return 0;

	int k;
	for ( k = 0; str[k]; k++ )
	{
		if ( k >= buf_size-1 )
			break;

		buf[k] = str[k];
	}

	if ( buf[k-1] == '\n' )
		buf[k-1] = '\0';
	else
		buf[k] = '\0';

	return 1;
}

/* формирование и отправка отчёта каждому игроку по прошедшему аукциону */
static int make_auction_report(Banker* b, AuctionReport* ar)
{
	if ( (b == NULL) || (ar == NULL) || (b->ready_players < 1) )
		return 0;

	char* mes_tokens[6 * MAX_PLAYERS + 1];
	int tokns_amnt = 6*MAX_PLAYERS+1;
	
	int j;
	for ( j = 0; j < tokns_amnt; j++ )
		mes_tokens[j] = NULL;
	
	struct player_report
	{
		char tn[16];
		char pnum[16];
		char ssnum[16];
		char spnum[24];
		char bpnum[16];
		char bprnum[24];
	};
	typedef struct player_report player_report;
	
	player_report pr[MAX_PLAYERS] = { 0 };


	mes_tokens[0] = "*INFO_MESSAGE|AUCTION_RESULTS|";
	int k = 1;

	for ( j = 0; j < b->ready_players; j++ )
	{
		/*printf("ar[%d].turn = %d\n", j, ar[j].turn);*/
		int turn_number = ar[j].turn;
		itoa(turn_number, pr[j].tn, 15);
		mes_tokens[k] = pr[j].tn;
		k++;

		/*printf("ar[%d].p->number = %d\n", j, ar[j].p->number);*/
		int p_num = ar[j].p->number;
		itoa(p_num, pr[j].pnum, 15);
		mes_tokens[k] = pr[j].pnum;
		k++;

		/*printf("ar[%d].sold_sources = %d\n", j, ar[j].sold_sources);*/
		int ss_num = ar[j].sold_sources;
		itoa(ss_num, pr[j].ssnum, 15);
		mes_tokens[k] = pr[j].ssnum;
		k++;

		/*printf("ar[%d].sold_price = %d\n", j, ar[j].sold_price);*/
		int sp_num = ar[j].sold_price;
		itoa(sp_num, pr[j].spnum, 23);
		mes_tokens[k] = pr[j].spnum;
		k++;

		/*printf("ar[%d].bought_prods = %d\n", j, ar[j].bought_prods);*/
		int bp_num = ar[j].bought_prods;
		itoa(bp_num, pr[j].bpnum, 15);
		mes_tokens[k] = pr[j].bpnum;
		k++;

		/*printf("ar[%d].bought_price = %d\n", j, ar[j].bought_price);*/
		int bpr_num = ar[j].bought_price;
		itoa(bpr_num, pr[j].bprnum, 23);
		mes_tokens[k] = pr[j].bprnum;
		k++;
	}
	tokns_amnt = k;
	
	/*
	for ( j = 0; j < tokns_amnt; j++ )
		printf("mes_tokens[%d] = %s\n", j, mes_tokens[j]);
	putchar('\n');
	*/

	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* p = b->pl_array[i];
		if ( p != NULL )
			send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
	}

	return 1;
}

/* расчёт и оплата игровых издержек каждым игроком */
static int pay_charges(Banker* banker, fd_set* readfds, AuctionReport* ar, MarketRequest** new_source_request_ptr, MarketRequest** new_prod_request_ptr)
{
	if ( (banker == NULL) || (readfds == NULL) )
		return 0;

	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* p = banker->pl_array[i];
		int total_charges = 0;
		if ( p != NULL )
		{
			int j;
			for ( j = 0; j < banker->ready_players; j++ )
				if ( p == ar[j].p )
					break;

			if ( j >= banker->ready_players )
				return 0;

			total_charges = (p->sources-ar[j].sold_sources) * SOURCE_UNIT_CHARGE;
			total_charges += p->products * PRODUCT_UNIT_CHARGE;
			total_charges += p->wait_factories * FACTORY_UNIT_CHARGE;
			total_charges += p->work_factories * FACTORY_UNIT_CHARGE;

			if ( total_charges <= p->money )
			{
				p->money -= total_charges;
					
				char* mes_tokens[2];
				int tokns_amnt = 2;
				mes_tokens[0] = "*INFO_MESSAGE|SUCCESS_CHARGES_PAY|";
				
				char charges[20];
				itoa(total_charges, charges, 19);
				mes_tokens[1] = charges;

				send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
			}
			else
			{
				char* mes_tokens[2];
				int tokns_amnt = 2;
				
				mes_tokens[0] = "*INFO_MESSAGE|PLAYER_BANKROT|";
				
				char charges[20];
				itoa(total_charges, charges, 19);
				mes_tokens[1] = charges;
				
				send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
				
				player_left_game(banker, p, i, readfds);
				
				if ( banker->alive_players == 1 )
				{
					Player* p;
					for ( i = 0; i < MAX_PLAYERS; i++ )
					{
						p = banker->pl_array[i];
						if ( p != NULL )
							break;
					}

					if ( i >= MAX_PLAYERS )
						continue;
					
					mr_clear(new_source_request_ptr);
					mr_clear(new_prod_request_ptr);

					server_end_work(banker, p, i, readfds);
				}
				else
				{	
					int tokns_amnt = 3;
					char* mes_tokens[tokns_amnt];
	
					mes_tokens[0] = "*INFO_MESSAGE|LOST_ALIVE_PLAYER|";

					char ap_buf[10];
					int ap = banker->alive_players;
					itoa(ap, ap_buf, 9);
					mes_tokens[1] = ap_buf;

					char left_p_num_buf[10];
					int left_pl_num = p->number;
					itoa(left_pl_num, left_p_num_buf, 9);
					mes_tokens[2] = left_p_num_buf;

					for ( i = 0; i < MAX_PLAYERS; i++ )
					{
						Player* p = banker->pl_array[i];
						if ( p != NULL )
							send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
					}
				}

			} /* end else */
		
		} /* end if */
	
	} /* end for */
	
	return 1;
}

/* формирование отчёта о прошедших событиях на текущем ходе */
static int report_on_turn(Banker* banker, AuctionReport* ar, MarketRequest* new_source_request, MarketRequest* new_prod_request)
{
	if ( 
			(banker == NULL)	||
			(ar == NULL)
		)
		return 0;

	printf("\n\n\n<<<<<<<<<< Report on Month #%d >>>>>>>>>>\n", banker->turn_number);
	printf("\n%s\n", "Players statistics:");
	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* p = banker->pl_array[i];
		if ( p != NULL )
			printf("\tPlayer #%d:   money: %dР   produced products: %d\n", p->number, p->money, p->produced_on_turn);
	}

	printf("\n%s\n", "Sources auction:");
	MarketRequest* nsr = new_source_request;
	while ( nsr != NULL )
	{
		printf("Request of Player #%d:\n", nsr->market_data.p->number);
		int j;
		for ( j = 0; j < banker->ready_players; j++ )
			if ( ar[j].p == nsr->market_data.p )
				break;

		if ( j >= banker->ready_players )
		{
			nsr = nsr->next;
			continue;
		}

		printf("\tPrice to sell: %d\n\tAmount: %d\n\tIs proceed: %s\n\n", nsr->market_data.price, (nsr->market_data.success) ? ar[j].sold_sources : nsr->market_data.amount, nsr->market_data.success ? "yes" : "no");
		nsr = nsr->next;
	}
	
	MarketRequest* npr = new_prod_request;
	printf("\n%s\n", "Products auction");
	while ( npr != NULL )
	{
		printf("Request of Player #%d:\n", npr->market_data.p->number);
		int j;
		for ( j = 0; j < banker->ready_players; j++ )
			if ( ar[j].p == npr->market_data.p )
				break;

		if ( j >= banker->ready_players )
		{
			npr = npr->next;
			continue;
		}

		printf("\tPrice to sell: %d\n\tAmount: %d\n\tIs proceed: %s\n\n", npr->market_data.price, (npr->market_data.success) ? ar[j].bought_prods : npr->market_data.amount, npr->market_data.success ? "yes" : "no");
		npr = npr->next;
	}
	printf("\n<<<<<<<<<< Report on Month #%d >>>>>>>>>>\n", banker->turn_number);
	
	return 1;
}

/* вычисление нового значения состояния рынка */
static int change_market_state(Banker* banker)
{
	if ( banker == NULL )
		return 0;

	int r = 1 + (int)(12.0 * rand() / (RAND_MAX + 1.0));
	
	int sum = 0;
	int i;
	for ( i = 0; i < MARKET_LEVEL_NUMBER; i++ )
	{
		sum += states_market_chance[banker->cur_market_lvl-1][i];
		if ( sum >= r )
			break;
	}
	
	if ( i < MARKET_LEVEL_NUMBER )
		banker->cur_market_lvl = i+1;
	
	int cur_lvl = banker->cur_market_lvl;
	banker->cur_market_state->source_amount = amount_multiplier_table[cur_lvl-1][0]*banker->alive_players;
	banker->cur_market_state->min_source_price = price_table[cur_lvl-1][0];
	banker->cur_market_state->product_amount = amount_multiplier_table[cur_lvl-1][1]*banker->alive_players;	
	banker->cur_market_state->max_product_price = price_table[cur_lvl-1][1];
	
	return 1;
}

/* запуск аукционов */
static MarketRequest* start_auction(Banker* banker, AuctionReport* ar, int auction_type) /*auction_type == 0 => sources, 1 => products*/
{
	if ( (banker == NULL) || (ar == NULL) )
		return NULL;

	/* Если игрок не заявился на аукцион, добавить его пустую заявку */
	int empty_list = 0;
	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* p = banker->pl_array[i];	
		if ( p != NULL )
		{
			MarketRequest* request = ( auction_type == 0 ) ? banker->sources_requests : banker->products_requests;
			if ( request == NULL )
			{
				empty_list = 1;
				break;
			}

			while ( request != NULL )
			{
				if ( request->market_data.p == p )
					break;

				request = request->next;
			}

			if ( request == NULL )
			{
				MarketData md;
				md.p = p;
				md.amount = 0;
				md.price = 0;
				md.success = 0;
				mr_insert( (auction_type == 0) ? &banker->sources_requests : &banker->products_requests, &md);
			}
		}
	}
	/*-----------------------------------------------------------------------*/
				
			
	MarketRequest* new_request = NULL;
	if ( !empty_list )
	{
		MarketRequest* arr[banker->ready_players];
		int arr_int[banker->ready_players];
		int arr_indexes[banker->ready_players];
		
		int i;
		for (i = 0; i < banker->ready_players; i++ )
		{
			arr[i] = NULL;
			arr_indexes[i] = 0;
		}

		MarketRequest* request = (auction_type == 0 ) ? banker->sources_requests : banker->products_requests;
		i = 0;
		while ( request != NULL )
		{
			arr[i] = request;
			i++;
			if ( request->next == NULL )
				break;
			request = request->next;
		}

		for ( i = 0; i < banker->ready_players; i++ )
			arr_int[i] = arr[i]->market_data.price;

		heap_sort(arr_int, banker->ready_players, (auction_type == 0) ? 1 : 0);

		int j = 0;
		i = 0;

		while ( i < banker->ready_players )
		{
			if ( (arr[i]->market_data.price == arr_int[j]) && !arr_indexes[i] )
			{
				mr_insert(&new_request, &arr[i]->market_data);
				arr_indexes[i] = 1;
				i = 0;
				j++;
				if ( j == banker->ready_players )
					break;
				continue;
			}
			i++;
		}
					
		mr_clear( (auction_type == 0) ? &banker->sources_requests : &banker->products_requests );
		if (auction_type == 0) 
			banker->sources_requests = NULL;
		else 
			banker->products_requests = NULL;

		
		int max_sources = banker->cur_market_state->source_amount;
		int max_products = banker->cur_market_state->product_amount;
		MarketRequest* nr = new_request;	
		while ( nr != NULL )
		{
			if ( nr->market_data.price < 1 )
			{
				nr = nr->next;
				continue;
			}
				
			if ( nr->market_data.amount <= ((auction_type == 0) ? max_sources : max_products)  )
			{
				if ( nr->market_data.amount > 0)
				{
					if ( auction_type == 0  )
					{
						nr->market_data.p->money -= nr->market_data.amount * nr->market_data.price;
						nr->market_data.p->sources += nr->market_data.amount;
						max_sources -= nr->market_data.amount;
					}
					else
					{
						nr->market_data.p->money += nr->market_data.amount * nr->market_data.price;
						nr->market_data.p->products -= nr->market_data.amount;
						max_products -= nr->market_data.amount;
					}
					nr->market_data.success = 1;

					int k;
					for ( k = 0; k < banker->ready_players; k++ )
					{
						if ( ar[k].p == nr->market_data.p )
						{
							( auction_type == 0 ) ? (ar[k].sold_sources = nr->market_data.amount) : (ar[k].bought_prods = nr->market_data.amount);
							( auction_type == 0 ) ? (ar[k].sold_price = nr->market_data.price) : (ar[k].bought_price = nr->market_data.price);
							break;
						}
					}
				}
			}
			else
			{
				int saved_max_sources = 0;
				int saved_max_products = 0;

				if ( ((auction_type == 0) ? max_sources : max_products) > 0)
				{
					if ( auction_type == 0)
					{
						nr->market_data.p->money -= max_sources*nr->market_data.price;
						nr->market_data.p->sources += max_sources;
					}
					else
					{
						nr->market_data.p->money += max_products*nr->market_data.price;
						nr->market_data.p->products -= max_products;
					}
					nr->market_data.success = 1;
					(auction_type == 0) ? (saved_max_sources = max_sources) : (saved_max_products = max_products);
					(auction_type == 0) ? (max_sources = 0) : (max_products = 0);
				}

				int k;
				for ( k = 0; k < banker->ready_players; k++ )
				{
					if ( ar[k].p == nr->market_data.p )
					{
						( auction_type == 0 ) ? (ar[k].sold_sources = saved_max_sources) : (ar[k].bought_prods = saved_max_products);
						( auction_type == 0 ) ? (ar[k].sold_price = nr->market_data.price) : (ar[k].bought_price = nr->market_data.price);
						break;
					}
				}
			}

			nr = nr->next;
		}

	} /* end of if (!empty_sources_list) */
	/*-------------------------------------------------------------------------------------------*/		
	return new_request;
}

/* проверка строящихся заводов у игроков */
static int check_building_factories(Banker* banker, fd_set* readfds)
{
	if ( (banker == NULL) || (readfds == NULL) )
		return 0;

	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* p = banker->pl_array[i];
		if ( p != NULL )
		{
			BuildList* list = p->build_list;
			while ( list != NULL )
			{
				list->turns_left--;
				
				if ( list->turns_left == 1 )
				{
					if ( p->money >= NEW_FACTORY_UNIT_COST/2 )
					{
						p->money -= NEW_FACTORY_UNIT_COST/2;

						const char* success_pay_mes = "*INFO_MESSAGE|PAY_FACTORY_SUCCESS\n";
						int mes_len = strlen(success_pay_mes);
						send_data(p->fd, success_pay_mes, mes_len, p->ip);
					}
					else
					{
						char* mes_tokens[2];
						int tokns_amnt = 2;
						
						mes_tokens[0] = "*INFO_MESSAGE|PLAYER_BANKROT|";
						
						char charges[20];
						itoa(NEW_FACTORY_UNIT_COST/2, charges, 19);
						mes_tokens[1] = charges;

						send_message(p->fd, mes_tokens, tokns_amnt, p->ip);

						player_left_game(banker, p, i, readfds);
						
						if ( banker->alive_players == 1 )
						{
							Player* p;
							for ( i = 0; i < MAX_PLAYERS; i++ )
							{
								p = banker->pl_array[i];
								if ( p != NULL )
									break;
							}
							if ( i >= MAX_PLAYERS )
								continue;
							
							server_end_work(banker, p, i, readfds);
						}
						else
						{
							int tokns_amnt = 3;
							char* mes_tokens[tokns_amnt];
			
							mes_tokens[0] = "*INFO_MESSAGE|LOST_ALIVE_PLAYER|";

							char ap_buf[10];
							int ap = banker->alive_players;
							itoa(ap, ap_buf, 9);
							mes_tokens[1] = ap_buf;

							char left_p_num_buf[10];
							int left_pl_num = p->number;
							itoa(left_pl_num, left_p_num_buf, 9);
							mes_tokens[2] = left_p_num_buf;

							for ( i = 0; i < MAX_PLAYERS; i++)
							{
								Player* p = banker->pl_array[i];
								if ( p != NULL )
									send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
							}
						}
					}
				}
				else if ( list->turns_left == 0 )
				{
					bl_delete(&p->build_list, 0);
					p->build_factories--;
					p->wait_factories++;


					const char* factory_built_mes = "*INFO_MESSAGE|FACTORY_BUILT\n";
					int f_b_len = strlen(factory_built_mes);
					send_data(p->fd, factory_built_mes, f_b_len, p->ip);
				}

				list = list->next;
			}
			/* while list != NULL */
		}
		/* p != NULL */
	} 

	return 1;
}

/* очистка сведений о покинувшем игру игроке */
int player_left_game(Banker* banker, Player* p, int i, fd_set* readfds)
{
	if ( (banker == NULL) || (p == NULL) || (i < 0) || (readfds == NULL) )
		return 0;
	
	if ( !banker->game_started)
	{
		banker->lobby_players--;
	}
	else
	{
		banker->alive_players--;
		if ( p->is_turn )
			banker->ready_players--;
		
		bl_clear(&p->build_list);			
		banker->pl_array[i]->build_list = NULL;

		mr_delete(&banker->sources_requests, p);
		mr_delete(&banker->products_requests, p);
	}

	FD_CLR(p->fd, readfds);
	close(p->fd);
	printf("Lost connection from [%s]\n", p->ip);
	free(p);
	banker->pl_array[i] = NULL;

	return 1;
}

/* отправка специального сообщения игроку */
static int send_wfnt_message(Banker* banker, Player* p)
{
	int wait_yet_players_amount = banker->alive_players - banker->ready_players; 
	
	char* mes_tokens[2];
	int tokns_amnt = 2;
	mes_tokens[0] = "*INFO_MESSAGE|WAIT_FOR_NEXT_TURN|";
	
	char w_y_p_a[10];
	itoa(wait_yet_players_amount, w_y_p_a, 9);
	mes_tokens[1] = w_y_p_a;
	
	if ( !send_message(p->fd, mes_tokens, tokns_amnt, p->ip) )
		return 0;
	
	return 1;
}

/* обработка пользовательской игровой команды */
static int process_command(ProcessCommandParams* pcp)
{
	Banker* banker = pcp->banker;
	Player* p = pcp->p;
	fd_set* readfds = pcp->readfds;
	char** command_tokens = pcp->command_tokens;
	int tokens_amount = pcp->tokens_amount;
	

	if ( (command_tokens == NULL) || (*command_tokens == NULL) || (tokens_amount < 1) )
		return 0;
		

	char command_str[100];
	int k;
	for ( k = 0; command_tokens[0][k]; k++ )
		command_str[k] = command_tokens[0][k];
	
	if (command_str[k-1] == '\n' )
		command_str[k-1] = '\0';
	else
		command_str[k] = '\0';
	
	int j;
	for ( j = 0; valid_commands[j] != NULL; j++ )
	{
		if ( strcmp(command_str, valid_commands[j]) == 0 )
		{
			CommandHandlerParams cmd_hdl_params;
			cmd_hdl_params.fd = p->fd;
			cmd_hdl_params.param1 = NULL;
			cmd_hdl_params.param2 = NULL;

			if ( j == PLAYER_COMMAND_NUM )
			{
				if ( tokens_amount < 2 )
				{
					const char* error_message = "*ERROR_MESSAGE|COMMAND_INCORRECT_ARGUMENTS_NUM\n";
					int em_len = strlen(error_message);
					send_data(p->fd, error_message, em_len, p->ip);
				}
				else
				{
					char param1_str[100];
					make_cmd_param(param1_str, 100, command_tokens[1]);
					int player_number = atoi(param1_str);
					cmd_hdl_params.param1 = (void*) &player_number;
					commands_handlers[j](banker, &cmd_hdl_params);
				}
			}
			else if ( j == BUY_COMMAND_NUM )
			{
				if ( !(p->is_turn) )
				{
					if ( tokens_amount < 3 )
					{
						const char* error_message = "*ERROR_MESSAGE|COMMAND_INCORRECT_ARGUMENTS_NUM\n";
						int em_len = strlen(error_message);
						send_data(p->fd, error_message, em_len, p->ip);
					}
					else
					{
						char param1_str[100];
						make_cmd_param(param1_str, 100, command_tokens[1]);
						int sources_amount = atoi(param1_str);
						cmd_hdl_params.param1 = (void*) &sources_amount;

						char param2_str[100];
						make_cmd_param(param2_str, 100, command_tokens[2]);
						int sources_price = atoi(param2_str);
						cmd_hdl_params.param2 = (void*) &sources_price;

						commands_handlers[j](banker, &cmd_hdl_params);
					}
				}
				else
				{
					send_wfnt_message(banker, p);
				}
			}
			else if ( j == SELL_COMMAND_NUM )
			{
				if ( !(p->is_turn) )
				{
					if ( tokens_amount < 3 )
					{
						const char* error_message = "*ERROR_MESSAGE|COMMAND_INCORRECT_ARGUMENTS_NUM\n";
						int em_len = strlen(error_message);
						send_data(p->fd, error_message, em_len, p->ip);
					}
					else
					{
						char param1_str[100];
						make_cmd_param(param1_str, 100, command_tokens[1]);
						int products_amount = atoi(param1_str);
						cmd_hdl_params.param1 = (void*) &products_amount;

						char param2_str[100];
						make_cmd_param(param2_str, 100, command_tokens[2]);
						int products_price = atoi(param2_str);
						cmd_hdl_params.param2 = (void*) &products_price;

						commands_handlers[j](banker, &cmd_hdl_params);
					}
				}
				else
				{
					send_wfnt_message(banker, p);
				}
			}
			else if ( j == PROD_COMMAND_NUM )
			{
				if ( !(p->is_turn) )
				{
					commands_handlers[j](banker, &cmd_hdl_params);
				}
				else
				{
					send_wfnt_message(banker, p);
				}
			}
			else if ( j == BUILD_COMMAND_NUM )
			{
				if ( !(p->is_turn) )
				{
					if ( tokens_amount >= 2 )
					{
						char param1_str[100];
						make_cmd_param(param1_str, 100, command_tokens[1]);
						char* list_subcommand = param1_str;
						cmd_hdl_params.param1 = (void*) list_subcommand;
					}
					commands_handlers[j](banker, &cmd_hdl_params);
				}
				else
				{
					send_wfnt_message(banker, p);
				}
			}
			else if ( j == TURN_COMMAND_NUM )
			{
				if ( !(p->is_turn) )
				{
					commands_handlers[j](banker, &cmd_hdl_params);
				}
				else
				{
					send_wfnt_message(banker, p);
				}
			}
			else if ( j == QUIT_COMMAND_NUM )
			{
				cmd_hdl_params.param1 = (void*) &readfds;
				commands_handlers[j](banker, &cmd_hdl_params);
			}
			else
				commands_handlers[j](banker, &cmd_hdl_params);
			break;
		}
	}
	
	if ( valid_commands[j] == NULL )
	{
		const char* unknown_cmd_message = "*INFO_MESSAGE|UNKNOWN_COMMAND\n";
		int mes_len = strlen(unknown_cmd_message);
		send_data(p->fd, unknown_cmd_message, mes_len, p->ip);
	}

	return 1;
}

/* очистка сервером всех данных по окончанию работы */
int server_end_work(Banker* banker, Player* p, int i, fd_set* readfds)
{
	const char* victory_message = "*INFO_MESSAGE|VICTORY_MESSAGE\n";
	int mes_len = strlen(victory_message);
	send_data(p->fd, victory_message, mes_len, p->ip);

	bl_clear(&p->build_list);
	banker->pl_array[i]->build_list = NULL;
	mr_clear(&banker->sources_requests);
	mr_clear(&banker->products_requests);
	FD_CLR(p->fd, readfds);
	close(p->fd);
	printf("Lost connection from [%s]\n", p->ip);
	printf("\n\n<<<<< GAME IS FINISHED. PLAYER #%d IS WINNER! >>>>>\n\n", p->number);
	free(p);
	banker->pl_array[i] = NULL;

	exit(0);
}

/* инициализация работы сервера */
int server_init(char* port)
{
	printf("%s\n", "Configuring local address...");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo* bind_address;
	if ( getaddrinfo(NULL, port, &hints, &bind_address) != 0 )
	{
		fprintf(stderr, "An error has occured with \"getaddrinfo\" (errno code = %d)\n", errno);
		return -1;
	}

	printf("%s\n", "Creating listening socket...");
	int ls = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	if ( ls == -1 )
	{
		fprintf(stderr, "socket() failed. (errno code = %d)\n", errno);
		return -1;
	}
	
	int opt = 1;
	setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	printf("%s\n", "Binding socket to address...");
	if ( bind(ls, bind_address->ai_addr, bind_address->ai_addrlen) )
	{
		fprintf(stderr, "bind() failed. (errno code = %d)\n", errno);
		return -1;
	}
	freeaddrinfo(bind_address);

	printf("%s\n", "Listening...");
	if ( listen(ls, LISTEN_QUEUE_LEN) < 0 )
	{
		fprintf(stderr, "listen() failed. (errno code = %d\n)", errno);
		return -1;
	}
	printf("Waiting connections to %s port...\n", port);
	
	return ls;
}

/* остановка сервера вручную */
static void server_force_stop(Banker* banker)
{
	if ( banker == NULL )
		exit(1);

	printf("%s", "\n\n========== SERVER IS STOPPING WORK FORCELY ==========\n");

	mr_clear(&banker->products_requests);
	printf("%s","[+] Products request list has cleared\n");

	mr_clear(&banker->sources_requests);
	printf("%s", "[+] Sources request list has cleared\n");

	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* p = banker->pl_array[i];
		if ( p != NULL )
		{
			bl_clear(&p->build_list);
			printf("[+] Build list of player #%d has cleared\n", p->number);
			
			int record_num = p->number;
			close(p->fd);
			printf("[+] Lost connection from [%s]\n", p->ip);
			free(p);
			banker->pl_array[i] = NULL;
			printf("[+] Player #%d record has deleted\n", record_num);
		}
	}
	printf("%s", "========== SERVER IS STOPPING WORK FORCELY ==========\n\n");

	exit(0);		
}

/* запуск главного игрового цикла */
int server_run(Banker* banker, int ls)
{
	signal(SIGINT, exit_handler);
	signal(SIGALRM, alrm_handler);

	srand(time(0));
	

	while ( 1 )
	{	
		if ( exit_flag )
			server_force_stop(banker);
	
		fd_set readfds;
		int max_fd = ls;
		
		FD_ZERO(&readfds);
		FD_SET(ls, &readfds);

		int i;
		for ( i = 0; i < MAX_PLAYERS; i++ )
		{
			Player* p = banker->pl_array[i];
			if ( p != NULL )
			{
				if ( banker->game_started && (!p->is_prod) )
				{
					int amount_products_prev = p->products;
					int w_f = p->work_factories;
					while ( p->work_factories > 0 )
					{
						p->work_factories--;
						p->products++;
						p->wait_factories++;
					}
					if ( w_f > 0 )
					{
						p->produced_on_turn = p->products - amount_products_prev;
						
						char* mes_tokens[2];
						int tokns_amnt = 2;
						mes_tokens[0] = "*INFO_MESSAGE|PRODUCED|";
						
						char am_prd[10];
						itoa(p->produced_on_turn, am_prd, 9);
						mes_tokens[1] = am_prd;
						
						send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
					}
				}

				int fd = p->fd;
				FD_SET( fd, &readfds);
				if ( fd > max_fd )
					max_fd = fd;
			}
		}
		/* end for */
	
		if ( !banker->game_started )
		{
			if ( !timer_set && (banker->lobby_players >= MIN_PLAYERS_TO_START) && (banker->lobby_players < MAX_PLAYERS) )
			{
				alarm(TIME_TO_START);
				timer_set = 1;
				
				char* mes_tokens[2];
				int tokns_amnt = 2;
				mes_tokens[0] = "*INFO_MESSAGE|STARTINSECONDS|";
				
				char tts[10];
				itoa(TIME_TO_START, tts, 9);
				mes_tokens[1] = tts;

				for ( i = 0; i < MAX_PLAYERS; i++ )
				{
					Player* p = banker->pl_array[i];
					if ( p != NULL )
						send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
				}
			}
			else if ( banker->lobby_players == MAX_PLAYERS )
			{		
				alarm(0);
				alrm_flag = 0;
				timer_set = 0;
				
				/*FORCE START THE GAME*/
				banker->game_started = 1;
				/*FORCE START THE GAME*/


				const char* message = "*INFO_MESSAGE|GAME_STARTED\n";
				int msg_len = strlen(message);

				int i;
				for ( i = 0; i < MAX_PLAYERS; i++ )
				{
					Player* p = banker->pl_array[i];
					if ( p != NULL )
						send_data(p->fd, message, msg_len, p->ip);
				}

				continue;
			}
		}
		else
		{
			if ( !players_prepared )
			{
				players_prepared = 1;
				
				banker->alive_players = banker->lobby_players;
				banker->lobby_players = 0;
				banker->turn_number = 1;
				
				banker->cur_market_lvl = START_MARKET_LEVEL;
				banker->cur_market_state->source_amount = amount_multiplier_table[START_MARKET_LEVEL-1][0]*banker->alive_players;
				banker->cur_market_state->min_source_price = price_table[START_MARKET_LEVEL-1][0];
				banker->cur_market_state->product_amount = amount_multiplier_table[START_MARKET_LEVEL-1][1]*banker->alive_players;
				banker->cur_market_state->max_product_price = price_table[START_MARKET_LEVEL-1][1];

				int i;
				for ( i = 0; i < MAX_PLAYERS; i++ )
				{
					Player* p = banker->pl_array[i];
					if ( p != NULL )
					{
						p->money = START_MONEY;
						p->sources = START_SOURCES;
						p->products = START_PRODUCTS;
						p->wait_factories = START_FACTORIES;
						p->old_money = p->money;
						
						
						int tokns_amnt = (p->is_bot) ? 14 : 10;

						char* mes_tokens[tokns_amnt];
						mes_tokens[0] = "*INFO_MESSAGE|STARTING_GAME_INFORMATION|";
						
						char p_num[10];
						itoa(p->number, p_num, 9);
						mes_tokens[1] = p_num;
						
						char ap[10];
						itoa(banker->alive_players, ap, 9);
						mes_tokens[2] = ap;

						char tn[10];
						itoa(banker->turn_number, tn, 9);
						mes_tokens[3] = tn;

						char p_money[20];
						itoa(p->money, p_money, 19);
						mes_tokens[4] = p_money;

						char p_sources[10];
						itoa(p->sources, p_sources, 9);
						mes_tokens[5] = p_sources;

						char p_products[10];
						itoa(p->products, p_products, 9);
						mes_tokens[6] = p_products;

						char p_wf[10];
						itoa(p->wait_factories, p_wf, 9);
						mes_tokens[7] = p_wf;

						char p_wrkf[10];
						itoa(p->work_factories, p_wrkf, 9);
						mes_tokens[8] = p_wrkf;

						char p_bf[10];
						itoa(p->build_factories, p_bf, 9);
						mes_tokens[9] = p_bf;
						
						char sa[10];
						char msp[10];
						char pa[10];
						char mpp[10];
						if ( p->is_bot )
						{
							itoa(banker->cur_market_state->source_amount, sa, 9);
							mes_tokens[10] = sa;

							itoa(banker->cur_market_state->min_source_price, msp, 9);
							mes_tokens[11] = msp;

							itoa(banker->cur_market_state->product_amount, pa, 9);
							mes_tokens[12] = pa;

							itoa(banker->cur_market_state->max_product_price, mpp, 9);
							mes_tokens[13] = mpp;
						}
						
						send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
					}
				}
			} /* end if (!players_prepared) */
		} /* end server_banker.game_started */


		int res = select(max_fd+1, &readfds, NULL, NULL, NULL);

		if ( res == -1)
		{
			if ( errno == EINTR )
			{
				if ( (sig_number == SIGALRM) && alrm_flag && (!banker->game_started) )
				{
					alrm_flag = 0;
					timer_set = 0;

					if ( banker->lobby_players < MIN_PLAYERS_TO_START )
					{
						const char* message = "*INFO_MESSAGE|STARTCANCELLED\n";
						int msg_len = strlen(message);

						int i;
						for ( i = 0; i < MAX_PLAYERS; i++ )
						{
							Player* p = banker->pl_array[i];
							if ( p != NULL )
								send_data(p->fd, message, msg_len, p->ip);
						}
					}
					else
					{
						if ( banker->lobby_players < MAX_PLAYERS )
						{
							/* START THE GAME */
							banker->game_started = 1;
							/* START THE GAME */
							

							const char* message = "*INFO_MESSAGE|GAME_STARTED\n";
							int msg_len = strlen(message);

							int i;
							for ( i = 0; i < MAX_PLAYERS; i++ )
							{
								Player* p = banker->pl_array[i];
								if ( p != NULL )
									send_data(p->fd, message, msg_len, p->ip);
							}
						}
						continue;
					}
				}
				fprintf(stderr, "\nGot some signal (#%d).\n", sig_number);
			}
			else
			{
				fprintf(stderr, "\nselect() failed. (errno code = %d)\n", errno);
			}
			continue;
		}
		/* end if (res == -1) */


		if ( FD_ISSET(ls, &readfds) )
		{
			char address_buffer[ADDRESS_BUFFER];
			char service_buffer[ADDRESS_BUFFER];
			struct sockaddr_storage client_address;
			socklen_t client_address_len = sizeof(client_address);

			int cs = accept(ls, (struct sockaddr*) &client_address, &client_address_len);
			if ( cs == -1 )
			{
				fprintf(stderr, "accept() failed. (errno code = %d)\n", errno);
				return 1;
			}
			
			getnameinfo((struct sockaddr*) &client_address, client_address_len, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST | NI_NUMERICSERV );
			
			int addr_len = strlen(address_buffer);
			int i, j;
			i = addr_len;
			address_buffer[addr_len] = ':';
			i++;
			for ( j = 0; service_buffer[j]; j++, i++ )
				address_buffer[i] = service_buffer[j];
			address_buffer[i] = '\0';

			printf("New client (%s) is connected.\n", address_buffer);
			
			if ( banker->game_started )
			{
				const char* message = "*INFO_MESSAGE|GAME_ALREADY_STARTED\n";
				int msg_len = strlen(message);
				send_data(cs, message, msg_len, address_buffer);				
				
				close(cs);
				printf("Lost connection from (%s)\n", address_buffer);
			}
			else
			{
				int i;
				for ( i = 0; i < MAX_PLAYERS; i++ )
				{
					if ( banker->pl_array[i] == NULL )
						break;
				}
				 
				if ( i >= MAX_PLAYERS )
				{
					const char* message = "*INFO_MESSAGE|SERVER_FULL\n";
					int msg_len = strlen(message);
					send_data(cs, message, msg_len, address_buffer);
					
					close(cs);
					printf("Lost connection from (%s)\n", address_buffer);
				}
				else
				{
					Player* p = malloc(sizeof(struct Player));
					if ( p == NULL )
					{
						const char* message = "*INFO_MESSAGE|OUT_OF_MEMORY\n";
						int msg_len = strlen(message);
						send_data(cs, message, msg_len, address_buffer);
						
						close(cs);
						printf("Lost connection from (%s)\n", address_buffer);
					}
					else
					{
						p->fd = cs;
						p->number = i+1;
						
						int j, k = 0;
						for ( j = 0; address_buffer[j]; j++, k++ ) 
							p->ip[k] = address_buffer[j];
						p->ip[k] = '\0';

						p->money = 0;
						p->old_money = 0;
						p->income = 0;
						p->sources = 0;
						p->products = 0;
						p->wait_factories = 0;
						p->work_factories = 0;
						p->build_factories = 0;
						p->build_list = NULL;
						p->is_turn = 0;
						p->is_prod = 0;
						p->is_ident_message_recv = 0;
						p->produced_on_turn = 0;
						p->sent_source_request = 0;
						p->sent_products_request = 0;

						banker->pl_array[i] = p;
						banker->lobby_players++;
					
						int lp = banker->lobby_players;
						int max_pl = MAX_PLAYERS;

						char* mes_tokens[3];
						int tokns_amnt = 3;

						mes_tokens[0] = "*INFO_MESSAGE|NEW_PLAYER_CONNECT|";
						
						char lp_buf[10];
						itoa(lp, lp_buf, 9);
						mes_tokens[1] = lp_buf;

						char max_pl_buf[10];
						itoa(max_pl, max_pl_buf, 9);
						mes_tokens[2] = max_pl_buf;

						for ( i = 0; i < MAX_PLAYERS; i++ )
						{
							Player* p = banker->pl_array[i];
							if ( p != NULL )
								send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
						}
					}
				}
			}
		}
		/* end if FD_ISSET */
	

		char read_buf[BUFSIZE];
		for ( i = 0; i < MAX_PLAYERS; i++ )
		{	
			Player* p = banker->pl_array[i];
			if ( p != NULL )
			{
				if ( FD_ISSET(p->fd, &readfds) )
				{
					int rc = readline(p->fd, read_buf, BUFSIZE-1);
					if ( rc > 0 )
					{
						printf("Received %d bytes from [%s]\n", rc, p->ip);
	
						int j;
						for ( j = 0; j < rc; j++ )
							if ( read_buf[j] == '\n' )
								break;

						if ( j < BUFSIZE-1 )
							read_buf[j] = '\0';
						else
							read_buf[BUFSIZE-1] = '\0';
						
						
						int size = strlen(read_buf)+1;
	
						delete_spaces(read_buf, &size);

						if ( !banker->game_started )
						{
							if ( !p->is_ident_message_recv )
							{
								if ( (strcmp(read_buf, "./bot_mg4") == 0) || (strcmp(read_buf, "./bot_mg4\n") == 0) )
									p->is_bot = 1;
								else
									p->is_bot = 0;

								p->is_ident_message_recv = 1;
							}
							else
							{
								const char* message = "*INFO_MESSAGE|GAME_NOT_STARTED\n";
								int mes_len = strlen(message);
								send_data(p->fd, message, mes_len, p->ip);
							}

							continue;
						}	
						
						/* Обработка данных от игрока, когда игра началась */
						
						
						char* tokens[3] = {
								NULL,
								NULL,
								NULL
						};
						int tokens_amnt = 0;
						char* istr = strtok(read_buf, " ");
						j = 0;
						while ( (istr != NULL) && ( j < 3 ) )
						{
							tokens[j] = istr;
							j++;
							istr = strtok(NULL, " ");
						}
						tokens_amnt = j;
						
						if ( tokens_amnt < 1 )
							continue;
						
						
						/*
						putchar('\n');
						for ( j = 0; j < tokens_amount; j++ )
							printf("%s\n", tokens[j]);
						putchar('\n');
						*/
						
						
						ProcessCommandParams pcp;
						pcp.banker = banker;
						pcp.p = p;
						pcp.readfds = &readfds;
						pcp.command_tokens = tokens;
						pcp.tokens_amount = tokens_amnt;
						
						process_command(&pcp);
						

					} /* end of if (rc > 0) */
					else
					{
						if ( !banker->game_started )
						{
							player_left_game(banker, p, i, &readfds);

							int lp = banker->lobby_players;
							int max_pl = MAX_PLAYERS;
							
							char* mes_tokens[3];
							int tokns_amnt = 3;
							
							mes_tokens[0] = "*INFO_MESSAGE|LOST_LOBBY_PLAYER|";
	
							char lp_buf[10];
							itoa(lp, lp_buf, 9);
							mes_tokens[1] = lp_buf;							

							char max_pl_buf[10];
							itoa(max_pl, max_pl_buf, 9);
							mes_tokens[2] = max_pl_buf;

							for ( i = 0; i < MAX_PLAYERS; i++ )
							{
								Player* p = banker->pl_array[i];
								if ( p != NULL )
									send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
							}
						}
						else
						{
							int left_pl_num = p->number;
							player_left_game(banker, p, i, &readfds);
							
							if ( banker->alive_players == 1 )
							{
								Player* p;
								for ( i = 0; i < MAX_PLAYERS; i++ )
								{
									p = banker->pl_array[i];
									if ( p != NULL )
										break;
								}
								if ( i >= MAX_PLAYERS )
									continue;
								
								server_end_work(banker, p, i, &readfds);
							}
							else
							{
								int tokns_amnt = 3;
								char* mes_tokens[tokns_amnt];
				
								mes_tokens[0] = "*INFO_MESSAGE|LOST_ALIVE_PLAYER|";

								char ap_buf[10];
								int ap = banker->alive_players;
								itoa(ap, ap_buf, 9);
								mes_tokens[1] = ap_buf;

								char left_p_num_buf[10];
								itoa(left_pl_num, left_p_num_buf, 9);
								mes_tokens[2] = left_p_num_buf;

								for ( i = 0; i < MAX_PLAYERS; i++ )
								{
									Player* p = banker->pl_array[i];
									if ( p != NULL )
										send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
								}
							}
						} /* end else */
					} /* end else */
				} /* end if FD_ISSET */
			} /* end server_banker.pl_array[i] != NULL */
		} /* end for */

		if ( banker->game_started )
		{
			if ( banker->ready_players == banker->alive_players )
			{	
				check_building_factories(banker, &readfds);			
				
				AuctionReport ar[banker->ready_players];
				int j = 0;
				for ( i = 0; i < MAX_PLAYERS; i++ )
				{
					Player* p = banker->pl_array[i];
					if ( p != NULL )
					{
						ar[j].turn = banker->turn_number;
						ar[j].p = p;
						ar[j].sold_price = 0;
						ar[j].sold_sources = 0;
						ar[j].bought_price = 0;
						ar[j].bought_prods = 0;
						j++;
						if ( j >= banker->ready_players )
							break;
					}
				}				

				MarketRequest* new_source_request = start_auction(banker, ar, 0); /* 0 - sources auction */
				MarketRequest* new_prod_request = start_auction(banker, ar, 1); /* 1 - product auction */
				
				/*
				printf("%s", "\n========== DEBUG ==========\n");
				for ( j = 0; j < banker->ready_players; j++ )
				{
					printf("ar[%d].turn = %d\n"
						   "ar[%d].p->number = %d\n"
						   "ar[%d].sold_price = %d\n"
						   "ar[%d].sold_sources = %d\n"
						   "ar[%d].bought_price = %d\n"
						   "ar[%d].bought_prods = %d\n\n", 
						   j, ar[j].turn, j, ar[j].p->number, j, ar[j].sold_price, j, ar[j].sold_sources, j, ar[j].bought_price, j, ar[j].bought_prods);
				}
				printf("%s", "\n========== DEBUG ==========\n");
				*/

				make_auction_report(banker, ar);
				
				pay_charges(banker, &readfds, ar, &new_source_request, &new_prod_request);
				
				report_on_turn(banker, ar, new_source_request, new_prod_request);			
				
				if ( new_source_request != NULL)
					mr_clear(&new_source_request);
				if ( new_prod_request != NULL )
					mr_clear(&new_prod_request);

				change_market_state(banker);

				banker->turn_number++;
				banker->ready_players = 0;

				for ( i = 0; i < MAX_PLAYERS; i++ )
				{
					Player* p = banker->pl_array[i];
					if ( p != NULL )
					{
						p->sent_source_request = 0;
						p->sent_products_request = 0;
						p->produced_on_turn = 0;
						p->is_turn = 0;
						p->is_prod = 0;
						p->income = p->money - p->old_money;
						
						/*int old_money = p->old_money;*/
						p->old_money = p->money;
						/*printf("\n---------------------------------- (DEBUG) ---------------------------------\\\n");
						printf("[%d]: p->income = %d, p->money = %d, p->old_money = %d\n", p->number, p->income, p->money, old_money);
						printf("------------------------------------ (DEBUG) -------------------------------/\n\n");*/
						
						int tokns_amnt = (p->is_bot) ? 6 : 2;
						char* mes_tokens[tokns_amnt];

						mes_tokens[0] = "*INFO_MESSAGE|NEW_TURN|";
						
						char new_turn[10];
						itoa(banker->turn_number, new_turn, 9);
						mes_tokens[1] = new_turn;
						
						char sa[10];
						char msp[10];
						char pa[10];
						char mpp[10];
						if ( p->is_bot )
						{
							itoa(banker->cur_market_state->source_amount, sa, 9);
							mes_tokens[2] = sa;

							itoa(banker->cur_market_state->min_source_price, msp, 9);
							mes_tokens[3] = msp;

							itoa(banker->cur_market_state->product_amount, pa, 9);
							mes_tokens[4] = pa;

							itoa(banker->cur_market_state->max_product_price, mpp, 9);
							mes_tokens[5] = mpp;
						}

						send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
					}
				}

			} /* end if server_banker.ready_players == alive_players */
		
		} /* end if server_banker.game_started */ 

	} /* end while ( 1 ) */ 
}

#endif
