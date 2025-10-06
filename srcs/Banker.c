/* Файл реализации для модуля Banker */

#ifndef BANKER_C
#define BANKER_C

#include "../includes/Banker.h"


// Описаны в модуле MGLib
extern const char* info_game_messages[];
extern const char* error_game_messages[];

// Описаны в модуле CommandsHandler
extern const char* valid_commands[];
extern int (*commands_handlers[])(Banker*, Player*, CommandHandlerParams* );


// Описаны в модуле serverCore
extern int send_message(int fd, const char** message_tokens, int tokens_amount, const char* ip);

// Описаны в <string.h>
extern int strcmp(const char* s1, const char* s2);
extern char* strcpy(char* dest, const char* src);

// Описана в <stdio.h>
extern int printf(const char*, ...);


// отправка специального сообщения игроку
static int send_wfnt_message(int wypa, int fd, const char* ip);



/* Таблица множителей для формулы вычисления нового состояния рынка в соответствии с текущим уровнем */
double amount_multiplier_table[MARKET_LEVEL_NUMBER][2] = {
					{ 1.0, 3.0 },
					{ 1.5, 2.5 },
					{ 2.0, 2.0 },
					{ 2.5, 2.5 },
					{ 3.0, 1.0 }
};

/* Таблица цен для формулы вычисления нового состояния рынка в соответствии с текущим уровнем */
int price_table[MARKET_LEVEL_NUMBER][2] = {
					{ 800, 6500 },
					{ 650, 6000 },
					{ 500, 5500 },
					{ 400, 5000 },
					{ 300, 4500 }
};

/* Таблица вероятностных переходов для формулы вычисления шанса перехода на другой уровень рынка */
const int states_market_chance[MARKET_LEVEL_NUMBER][MARKET_LEVEL_NUMBER] = {
				{ 4, 4, 2, 1, 1 },
				{ 3, 4, 3, 1, 1 },
				{ 1, 3, 4, 3, 1 },
				{ 1, 1, 3, 4, 3 },
				{ 1, 1, 2, 4, 4 }
};

/* Параметры рынка */
MarketState cur_ms;


static int send_wfnt_message(int wypa, int fd, const char* ip)
{
	if (
				( wypa < 0 )			||
				( fd < 0 )				||
				( ip == NULL )			||
				( *ip == '\0' )
		)
		return 0;

	char w_y_p_a[10];
	itoa(wypa, w_y_p_a, 9);

	int tokns_amnt = 2;
	char* mes_tokens[] =
	{
			(char*)info_game_messages[WAIT_FOR_NEXT_TURN],
			w_y_p_a,
			NULL
	};

	send_message(fd, (const char**)mes_tokens, tokns_amnt, ip);

	return 1;
}

int banker_init(Banker* b)
{
	if ( b == NULL )
		return 0;

	b->lobby_players = 0;
	b->alive_players = 0;
	b->ready_players = 0;
	b->game_started = 0;
	b->products_requests = NULL;
	b->sources_requests = NULL;
	b->turn_number = 0;

	for ( int i = 0; i < MAX_PLAYERS; i++ )
		b->pl_array[i] = NULL;

	cur_ms.max_product_price = 0;
	cur_ms.min_source_price = 0;
	cur_ms.product_amount = 0;
	cur_ms.source_amount = 0;

	b->cur_market_lvl = 0;
	b->cur_market_state = &cur_ms;
	b->valid_commands = valid_commands;

	return 1;
}

int last_man_stand(Banker* b)
{
	Player* last_player;
	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		last_player = b->pl_array[i];
		if ( last_player != NULL )
			break;
	}

	if ( i >= MAX_PLAYERS )
		return 0;

	const char* victory_message[] =
	{
			info_game_messages[VICTORY_MESSAGE],
			NULL
	};
	send_message(last_player->fd, victory_message, 1, last_player->ip);

	return last_player->number;
}

int make_auction_report(Banker* b, AuctionReport* ar)
{
	if (
			( b == NULL )					||
			( ar == NULL )					||
			( b->ready_players < 1 )
		)
		return 0;

	enum
	{
			PL_REP_FIELDS_NUM		=		6,
			TURN_SIZE				=		16,
			PLAYER_NUM_SIZE			=		16,
			SOLD_SOURCES_SIZE		=		16,
			SOLD_PRICE_SIZE			=		24,
			BOUGHT_PRODS_SIZE		=		16,
			BOUGHT_PRICE_SIZE		=		24

	};
	struct player_report
	{
		char tn[TURN_SIZE];
		char pnum[PLAYER_NUM_SIZE];
		char ssnum[SOLD_SOURCES_SIZE];
		char spnum[SOLD_PRICE_SIZE];
		char bpnum[BOUGHT_PRODS_SIZE];
		char bprnum[BOUGHT_PRICE_SIZE];
	};
	typedef struct player_report player_report;
	player_report pr[MAX_PLAYERS] = { 0 };

/////
	int tokns_amnt = PL_REP_FIELDS_NUM * MAX_PLAYERS + 1;
	char* mes_tokens[tokns_amnt];

	for ( int j = 0; j < tokns_amnt; j++ )
		mes_tokens[j] = NULL;

	mes_tokens[0] = (char*)info_game_messages[AUCTION_RESULTS];

	int k = 1;
	for ( int j = 0; j < b->ready_players; j++ )
	{
		itoa(ar[j].turn, pr[j].tn, TURN_SIZE);
		mes_tokens[k] = pr[j].tn;
		k++;

		itoa(ar[j].p->number, pr[j].pnum, PLAYER_NUM_SIZE);
		mes_tokens[k] = pr[j].pnum;
		k++;

		itoa(ar[j].sold_sources, pr[j].ssnum, SOLD_SOURCES_SIZE);
		mes_tokens[k] = pr[j].ssnum;
		k++;

		itoa(ar[j].sold_price, pr[j].spnum, SOLD_PRICE_SIZE);
		mes_tokens[k] = pr[j].spnum;
		k++;

		itoa(ar[j].bought_prods, pr[j].bpnum, BOUGHT_PRODS_SIZE);
		mes_tokens[k] = pr[j].bpnum;
		k++;

		itoa(ar[j].bought_price, pr[j].bprnum, BOUGHT_PRICE_SIZE);
		mes_tokens[k] = pr[j].bprnum;
		k++;
	}
	tokns_amnt = k;

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		Player* p = b->pl_array[i];
		if ( p != NULL )
			send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
	}

	return 1;
}

int pay_charges(Banker* banker, fd_set* readfds, AuctionReport* ar, MarketRequest** new_source_request_ptr, MarketRequest** new_prod_request_ptr)
{
	if (
				( banker == NULL )			||
				( readfds == NULL )
		)
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
/////
				int tokns_amnt = 2;
				char* mes_tokens[tokns_amnt];

				mes_tokens[0] = (char*)info_game_messages[SUCCESS_CHARGES_PAY];

				char charges[20];
				itoa(total_charges, charges, 19);
				mes_tokens[1] = charges;

				send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
			}
			else
			{
/////
				int tokns_amnt = 2;
				char* mes_tokens[tokns_amnt];

				mes_tokens[0] = (char*)info_game_messages[PLAYER_BANKROT];

				char charges[20];
				itoa(total_charges, charges, 19);
				mes_tokens[1] = charges;

				send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);



				int left_pl_num = p->number;
				int p_fd = p->fd;
				if ( clean_player_record(banker, p) )
				{
					banker->pl_array[i] = NULL;
					FD_CLR(p_fd, readfds);
				}



				if ( banker->alive_players == 1 )
				{
					int winner_number = last_man_stand(banker);
					printf("\n\n<<<<< GAME IS FINISHED. PLAYER #%d IS WINNER! >>>>>\n\n", winner_number);
					server_stop(banker, readfds, 0);
				}
				else
				{
/////
					int tokns_amnt = 3;
					char* mes_tokens[tokns_amnt];
					mes_tokens[0] = (char*)info_game_messages[LOST_ALIVE_PLAYER];

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
							send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
					}
				}
			} /* end else */
		} /* end if */
	} /* end for */

	return 1;
}

int report_on_turn(Banker* banker, AuctionReport* ar, MarketRequest* new_source_request, MarketRequest* new_prod_request)
{
	if (
			( banker == NULL )	||
			( ar == NULL )
		)
		return 0;

	printf("\n\n\n<<<<<<<<<< Report on Month #%d >>>>>>>>>>\n", banker->turn_number);
	printf("\n%s\n", "Players statistics:");
	for ( int i = 0; i < MAX_PLAYERS; i++ )
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
	printf("\n%s\n", "Products auction:");
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

int change_market_state(Banker* banker)
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

MarketRequest* start_auction(Banker* banker, AuctionReport* ar, int auction_type)
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
		if ( auction_type == 0 )
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
				if ( nr->market_data.amount > 0 )
				{
					if ( auction_type == 0 )
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

int check_building_factories(Banker* banker, fd_set* readfds)
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
/////
						const char* success_pay_mes[] =
						{
								info_game_messages[PAY_FACTORY_SUCCESS],
								NULL
						};
						send_message(p->fd, success_pay_mes, 1, p->ip);
					}
					else
					{
/////
						int tokns_amnt = 2;
						char* mes_tokens[tokns_amnt];

						mes_tokens[0] = (char*)info_game_messages[PLAYER_BANKROT];

						char charges[20];
						itoa(NEW_FACTORY_UNIT_COST/2, charges, 19);
						mes_tokens[1] = charges;

						send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);



						int left_pl_num = p->number;
						int p_fd = p->fd;
						if ( clean_player_record(banker, p) )
						{
							banker->pl_array[i] = NULL;
							FD_CLR(p_fd, readfds);
						}



						if ( banker->alive_players == 1 )
						{
							int winner_number = last_man_stand(banker);
							printf("\n\n<<<<< GAME IS FINISHED. PLAYER #%d IS WINNER! >>>>>\n\n", winner_number);
							server_stop(banker, readfds, 0);
						}
						else
						{
/////
							int tokns_amnt = 3;
							char* mes_tokens[tokns_amnt];

							mes_tokens[0] = (char*)info_game_messages[LOST_ALIVE_PLAYER];

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
									send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
							}
						}

					}
				}
				else if ( list->turns_left == 0 )
				{
					bl_delete(&p->build_list, 0);
					p->build_factories--;
					p->wait_factories++;


					const char* factory_built_mes[] =
					{
							info_game_messages[FACTORY_BUILT],
							NULL
					};
					send_message(p->fd, factory_built_mes, 1, p->ip);

					list = p->build_list;
					continue;
				}

				list = list->next;
			}
			/* while list != NULL */
		}
		/* p != NULL */
	}

	return 1;
}

int clean_player_record(Banker* banker, Player* p)
{
	if (
						( banker == NULL )			||
						( p == NULL )
		)
		return 0;
	
	int record_num = p->number;

	if ( !banker->game_started )
	{
		banker->lobby_players--;
	}
	else
	{
		banker->alive_players--;
		if ( p->is_turn )
			banker->ready_players--;

		bl_clear(&p->build_list);
		p->build_list = NULL;
		printf("[+] Build list of player #%d has cleared\n", p->number);

		mr_delete(&banker->sources_requests, p);
		mr_delete(&banker->products_requests, p);
	}
	free(p);
	
	printf("[+] Player #%d record has deleted\n", record_num);

	return 1;
}




int process_command(Banker* b, Player* p, const char** command_tokens, int tokens_amount)
{
	if (
					( b == NULL )						||
					( p == NULL )						||
					( command_tokens == NULL )			||
					( *command_tokens == NULL )			||
					( tokens_amount < 1 )
		)
		return ERROR_COMMAND_NUM;



	char command_str[100];
	strcpy(command_str, command_tokens[0]);
	cut_str(command_str, 100, '\n');


	CommandHandlerParams cmd_hdl_params;
	cmd_hdl_params.param1 = NULL;
	cmd_hdl_params.param2 = NULL;


	int j;
	for ( j = 0; valid_commands[j] != NULL; j++ )
	{
		if ( strcmp(command_str, valid_commands[j]) == 0 )
		{
			if ( j == PLAYER_COMMAND_NUM )
			{
				if ( tokens_amount < 2 )
				{
					const char* error_message[] =
					{
								error_game_messages[COMMAND_INCORRECT_ARGUMENTS_NUM],
								NULL
					};
					send_message(p->fd, error_message, 1, p->ip);
				}
				else
				{
					char param1_str[100];
					strcpy(param1_str, command_tokens[1]);
					cut_str(param1_str, 100, '\n');

					int player_number = atoi(param1_str);
					cmd_hdl_params.param1 = (void*) &player_number;


					commands_handlers[j](b, p, &cmd_hdl_params);
				}

				return PLAYER_COMMAND_NUM;
			}

			if ( j == BUY_COMMAND_NUM )
			{
				if ( !(p->is_turn) )
				{
					if ( tokens_amount < 3 )
					{
						const char* error_message[] =
						{
									error_game_messages[COMMAND_INCORRECT_ARGUMENTS_NUM],
									NULL
						};
						send_message(p->fd, error_message, 1, p->ip);
					}
					else
					{
						char param1_str[100];
						strcpy(param1_str, command_tokens[1]);
						cut_str(param1_str, 100, '\n');

						int sources_amount = atoi(param1_str);
						cmd_hdl_params.param1 = (void*) &sources_amount;


						char param2_str[100];
						strcpy(param2_str, command_tokens[2]);
						cut_str(param2_str, 100, '\n');

						int sources_price = atoi(param2_str);
						cmd_hdl_params.param2 = (void*) &sources_price;


						commands_handlers[j](b, p, &cmd_hdl_params);
					}
				}
				else
				{
					int wait_yet_players_amount = b->alive_players - b->ready_players;
					send_wfnt_message(wait_yet_players_amount, p->fd, p->ip);
				}

				return BUY_COMMAND_NUM;
			}

			if ( j == SELL_COMMAND_NUM )
			{
				if ( !(p->is_turn) )
				{
					if ( tokens_amount < 3 )
					{
						const char* error_message[] =
						{
									error_game_messages[COMMAND_INCORRECT_ARGUMENTS_NUM],
									NULL
						};
						send_message(p->fd, error_message, 1, p->ip);
					}
					else
					{
						char param1_str[100];
						strcpy(param1_str, command_tokens[1]);
						cut_str(param1_str, 100, '\n');

						int products_amount = atoi(param1_str);
						cmd_hdl_params.param1 = (void*) &products_amount;


						char param2_str[100];
						strcpy(param2_str, command_tokens[2]);
						cut_str(param2_str, 100, '\n');

						int products_price = atoi(param2_str);
						cmd_hdl_params.param2 = (void*) &products_price;


						commands_handlers[j](b, p, &cmd_hdl_params);
					}
				}
				else
				{
					int wait_yet_players_amount = b->alive_players - b->ready_players;
					send_wfnt_message(wait_yet_players_amount, p->fd, p->ip);
				}

				return SELL_COMMAND_NUM;
			}

			if ( j == PROD_COMMAND_NUM )
			{
				if ( !(p->is_turn) )
				{
					commands_handlers[j](b, p, &cmd_hdl_params);
				}
				else
				{
					int wait_yet_players_amount = b->alive_players - b->ready_players;
					send_wfnt_message(wait_yet_players_amount, p->fd, p->ip);
				}

				return PROD_COMMAND_NUM;
			}

			if ( j == BUILD_COMMAND_NUM )
			{
				if ( !(p->is_turn) )
				{
					if ( tokens_amount >= 2 )
					{
						char param1_str[100];
						strcpy(param1_str, command_tokens[1]);
						cut_str(param1_str, 100, '\n');

						char* list_subcommand = param1_str;
						cmd_hdl_params.param1 = (void*) list_subcommand;
					}
					commands_handlers[j](b, p, &cmd_hdl_params);
				}
				else
				{
					int wait_yet_players_amount = b->alive_players - b->ready_players;
					send_wfnt_message(wait_yet_players_amount, p->fd, p->ip);
				}

				return BUILD_COMMAND_NUM;
			}

			if ( j == TURN_COMMAND_NUM )
			{
				if ( !(p->is_turn) )
				{
					commands_handlers[j](b, p, &cmd_hdl_params);
				}
				else
				{
					int wait_yet_players_amount = b->alive_players - b->ready_players;
					send_wfnt_message(wait_yet_players_amount, p->fd, p->ip);
				}

				return TURN_COMMAND_NUM;
			}

			if ( j == QUIT_COMMAND_NUM )
			{
				if ( commands_handlers[j](b, p, &cmd_hdl_params) )
					return QUIT_COMMAND_NUM;

				return ERROR_COMMAND_NUM;
			}
		}
	}

	const char* unknown_cmd_message[] =
	{
				info_game_messages[UNKNOWN_COMMAND],
				NULL
	};
	send_message(p->fd, unknown_cmd_message, 1, p->ip);

	return UNKNOWN_COMMAND_NUM;
}

#endif
