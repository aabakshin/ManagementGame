/* Файл реализации для модуля Banker */

#ifndef BANKER_C
#define BANKER_C

#include "../includes/Banker.h"


// Описаны в модуле MGLib
extern const char* info_game_messages[];
extern const char* error_game_messages[];

// Описаны в <string.h>
extern int strcmp(const char* s1, const char* s2);
extern char* strcpy(char* dest, const char* src);

// Описана в <stdio.h>
extern int printf(const char*, ...);

// Описана в модуле serverCore
extern int server_quit_player(Banker* b, int i, fd_set* readfds, Player* p);


int send_wfnt_message(Banker*, Player*);
int send_victory_message(Banker* b, Player* p);
int send_auctionreport_message(Banker* b, Player* p, AuctionReport* ar);
static int send_successchargespay_message(int total_charges, Banker* b, Player* p);
static int send_playerbankrot_message(int total_charges, Banker* b, Player* p);
static int send_payfactorysuccess_message(Banker* b, Player* p);
static int send_factorybuilt_message(Banker* b, Player* p);
int send_produced_message(Banker* b, Player* p);
int send_startinseconds_message(Banker* b, Player* p);
int send_gamestarted_message(Banker* b, Player* p);
int send_startgameinfo_message(Banker* b, Player* p);
int send_startcancelled_message(Banker* b, Player* p);
int send_newplayerconnect_message(Banker* b, Player* p);
int send_gamenotstarted_message(Banker* b, Player* p);
int send_lostlobbyplayer_message(Banker* b, Player* p);
int send_lostaliveplayer_message(int left_pl_num, Banker* b, Player* p);
int send_newturn_message(Banker* b, Player* p);






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


int send_wfnt_message(Banker* b, Player* p)
{
	if (
				( b == NULL )				||
				( p == NULL )
		)
		return 0;

	char w_y_p_a[10];
	int wait_yet_players_amount = b->alive_players - b->ready_players;
	itoa(wait_yet_players_amount, w_y_p_a, 9);

	int tokns_amnt = 2;
	char* mes_tokens[] =
	{
			(char*)info_game_messages[WAIT_FOR_NEXT_TURN],
			w_y_p_a,
			NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

int send_victory_message(Banker* b, Player* p)
{
	if (
				( b == NULL )			||
				( p == NULL )
		)
		return 0;

	const char* victory_message[] =
	{
			info_game_messages[VICTORY_MESSAGE],
			NULL
	};
	send_message(p->fd, victory_message, 1, p->ip);

	return 1;
}

int send_auctionreport_message(Banker* b, Player* p, AuctionReport* ar)
{
	if (
			( b == NULL )					||
			( p == NULL )					||
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

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int send_successchargespay_message(int total_charges, Banker* b, Player* p)
{
	if (
				( b == NULL )			||
				( p == NULL )
		)
		return 0;

	char charges[20];
	itoa(total_charges, charges, 19);

	int tokns_amnt = 2;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[SUCCESS_CHARGES_PAY],
				charges,
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int send_playerbankrot_message(int total_charges, Banker* b, Player* p)
{
	if (
				( b == NULL )			||
				( p == NULL )
		)
		return 0;

	char charges[20];
	itoa(total_charges, charges, 19);

	int tokns_amnt = 2;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[PLAYER_BANKROT],
				charges,
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int send_payfactorysuccess_message(Banker* b, Player* p)
{
	if (
				( b == NULL )			||
				( p == NULL )
		)
		return 0;

	const char* success_pay_mes[] =
	{
			info_game_messages[PAY_FACTORY_SUCCESS],
			NULL
	};
	send_message(p->fd, success_pay_mes, 1, p->ip);

	return 1;
}

static int send_factorybuilt_message(Banker* b, Player* p)
{
	if (
				( b == NULL )			||
				( p == NULL )
		)
		return 0;

	const char* factory_built_mes[] =
	{
			info_game_messages[FACTORY_BUILT],
			NULL
	};
	send_message(p->fd, factory_built_mes, 1, p->ip);

	return 1;
}

int send_produced_message(Banker* b, Player* p)
{
	if (
			( b == NULL )			||
			( p == NULL )
		)
		return 0;

	char am_prd[10];
	itoa(p->produced_on_turn, am_prd, 9);

	int tokns_amnt = 2;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[PRODUCED],
				am_prd,
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

int send_startinseconds_message(Banker* b, Player* p)
{
	if (
			( b == NULL )			||
			( p == NULL )
		)
		return 0;

	char tts[10];
	itoa(TIME_TO_START, tts, 9);

	int tokns_amnt = 2;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[STARTINSECONDS],
				tts,
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

int send_gamestarted_message(Banker* b, Player* p)
{
	if (
			( b == NULL )					||
			( p == NULL )
		)
		return 0;

	const char* message[] =
	{
				info_game_messages[GAME_STARTED],
				NULL
	};

	send_message(p->fd, message, 1, p->ip);

	return 1;
}

int send_startgameinfo_message(Banker* b, Player* p)
{
	if (
			( b == NULL )					||
			( p == NULL )
		)
		return 0;

	char p_num[10];
	itoa(p->number, p_num, 9);

	char ap[10];
	itoa(b->alive_players, ap, 9);

	char tn[10];
	itoa(b->turn_number, tn, 9);

	char p_money[20];
	itoa(p->money, p_money, 19);

	char p_sources[10];
	itoa(p->sources, p_sources, 9);

	char p_products[10];
	itoa(p->products, p_products, 9);

	char p_wf[10];
	itoa(p->wait_factories, p_wf, 9);

	char p_wrkf[10];
	itoa(p->work_factories, p_wrkf, 9);

	char p_bf[10];
	itoa(p->build_factories, p_bf, 9);

	char sa[10];
	char msp[10];
	char pa[10];
	char mpp[10];

	if ( p->is_bot )
	{
		itoa(b->cur_market_state->source_amount, sa, 9);
		itoa(b->cur_market_state->min_source_price, msp, 9);
		itoa(b->cur_market_state->product_amount, pa, 9);
		itoa(b->cur_market_state->max_product_price, mpp, 9);
	}

	int tokns_amnt = (p->is_bot) ? 14 : 10;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[STARTING_GAME_INFORMATION],
				p_num,
				ap,
				tn,
				p_money,
				p_sources,
				p_products,
				p_wf,
				p_wrkf,
				p_bf,
				sa,
				msp,
				pa,
				mpp,
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

int send_startcancelled_message(Banker* b, Player* p)
{
	if (
					( b == NULL )			||
					( p == NULL )
		)
		return 0;


	const char* mes_tokens[] =
	{
				info_game_messages[STARTCANCELLED],
				NULL
	};
	send_message(p->fd, mes_tokens, 1, p->ip);

	return 1;
}

int send_newplayerconnect_message(Banker* b, Player* p)
{
	if (
					( b == NULL )			||
					( p == NULL )
		)
		return 0;

	char lp_buf[10];
	itoa(b->lobby_players, lp_buf, 9);

	char max_pl_buf[10];
	itoa(MAX_PLAYERS, max_pl_buf, 9);

	int tokns_amnt = 3;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[NEW_PLAYER_CONNECT],
				lp_buf,
				max_pl_buf,
				NULL
	};
	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

int send_gamenotstarted_message(Banker* b, Player* p)
{
	if (
					( b == NULL )			||
					( p == NULL )
		)
		return 0;

	const char* message[] =
	{
				info_game_messages[GAME_NOT_STARTED],
				NULL
	};
	send_message(p->fd, message, 1, p->ip);

	return 1;
}

int send_lostlobbyplayer_message(Banker* b, Player* p)
{
	if (
					( b == NULL )			||
					( p == NULL )
		)
		return 0;

	char lp_buf[10];
	itoa(b->lobby_players, lp_buf, 9);

	char max_pl_buf[10];
	itoa(MAX_PLAYERS, max_pl_buf, 9);

	int tokns_amnt = 3;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[LOST_LOBBY_PLAYER],
				lp_buf,
				max_pl_buf,
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

int send_lostaliveplayer_message(int left_pl_num, Banker* b, Player* p)
{
	if (
					( b == NULL )			||
					( p == NULL )
		)
		return 0;


	char ap_buf[10];
	itoa(b->alive_players, ap_buf, 9);

	char left_p_num_buf[10];
	itoa(left_pl_num, left_p_num_buf, 9);

	int tokns_amnt = 3;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[LOST_ALIVE_PLAYER],
				ap_buf,
				left_p_num_buf,
				NULL
	};


	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

int send_newturn_message(Banker* b, Player* p)
{
	if (
					( b == NULL )			||
					( p == NULL )
		)
		return 0;

	char nt[10];
	itoa(b->turn_number, nt, 9);

	char sa[10];
	char msp[10];
	char pa[10];
	char mpp[10];

	if ( p->is_bot )
	{
		itoa(b->cur_market_state->source_amount, sa, 9);
		itoa(b->cur_market_state->min_source_price, msp, 9);
		itoa(b->cur_market_state->product_amount, pa, 9);
		itoa(b->cur_market_state->max_product_price, mpp, 9);
	}

	int tokns_amnt = (p->is_bot) ? 6 : 2;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[NEW_TURN],
				nt,
				sa,
				msp,
				pa,
				mpp,
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

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

	return 1;
}

int get_player_idx_by_num(Banker* b, int player_number)
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		Player* p = b->pl_array[i];
		if ( p != NULL )
			if ( p->number == player_number )
				return i;
	}

	return -1;
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
				send_successchargespay_message(total_charges, banker, p);
			}
			else
			{
				send_playerbankrot_message(total_charges, banker, p);
				server_quit_player(banker, i, readfds, p);
			}
		}
	}

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

	for ( int i = 0; i < MAX_PLAYERS; i++ )
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
					int total_charges = NEW_FACTORY_UNIT_COST / 2;
					p->money -= total_charges;
					if ( p->money >= 0 )
					{
						send_payfactorysuccess_message(banker, p);
					}
					else
					{
						send_playerbankrot_message(total_charges, banker, p);
						server_quit_player(banker, i, readfds, p);
					}
				}
				else if ( list->turns_left == 0 )
				{
					bl_delete(&p->build_list, 0);
					p->build_factories--;
					p->wait_factories++;

					send_factorybuilt_message(banker, p);

					list = p->build_list;
					continue;
				}

				list = list->next;
			}
		}
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

#endif
