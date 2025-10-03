/*
 *	Файл реализации для модуля Banker.
 *	Содержит описание функции инициализации структуры банкира, а также глобальные определения
 */

#ifndef BANKER_C
#define BANKER_C

#include "../includes/Banker.h"
#include "../includes/MGLib.h"



// Описаны в модуле MGLib
extern const char* info_game_messages[];
extern const char* error_game_messages[];

// Описаны в модуле serverCore
extern int send_message(int fd, const char** message_tokens, int tokens_amount, const char* ip);
extern int player_left_game(Banker*, Player*);

// stdlib.h
extern int strcmp(const char* s1, const char* s2);

// string.h
extern char* strncpy(char* dest, const char* src, size_t n);


/* Список действительных игровых команд */
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

static int help_command_handler(Banker*, Player*, CommandHandlerParams*);
static int market_command_handler(Banker*, Player*, CommandHandlerParams*);
static int player_command_handler(Banker*, Player*, CommandHandlerParams*);
static int list_command_handler(Banker*, Player*, CommandHandlerParams*);
static int prod_command_handler(Banker*, Player*, CommandHandlerParams*);
static int build_command_handler(Banker*, Player*, CommandHandlerParams*);
static int buy_command_handler(Banker*, Player*, CommandHandlerParams*);
static int sell_command_handler(Banker*, Player*, CommandHandlerParams*);
static int turn_command_handler(Banker*, Player*, CommandHandlerParams*);
static int quit_command_handler(Banker*, Player*, CommandHandlerParams*);

/* Список функций-обработчиков валидных команд(порядок должен совпадать с порядком следования элементов в valid_commands) */
int (*commands_handlers[])(Banker*, Player*, CommandHandlerParams* ) = {
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
	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
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
/////
	const char* victory_message[] =
	{
			info_game_messages[VICTORY_MESSAGE],
			NULL
	};
	send_message(last_player->fd, victory_message, 1, last_player->ip);

	return last_player->number;
}


static int help_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
	{
		return 0;
	}

/////
	int tokns_amnt = 11;
	char* mes_tokens[tokns_amnt];

	mes_tokens[0] = (char*)info_game_messages[HELP_COMMAND];

	for ( int j = 0, i = 1; b->valid_commands[j] != NULL; j++, i++ )
		mes_tokens[i] = (char*) b->valid_commands[j];

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int market_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
	{
		return 0;
	}

	char* mes_tokens[5];
	int tokns_amnt = 5;

	mes_tokens[0] = (char*)info_game_messages[MARKET_COMMAND];

	char s_amount[10];
	itoa(b->cur_market_state->source_amount, s_amount, 9);
	mes_tokens[1] = s_amount;

	char s_min_price[10];
	itoa(b->cur_market_state->min_source_price, s_min_price, 9);
	mes_tokens[2] = s_min_price;

	char p_amount[10];
	itoa(b->cur_market_state->product_amount, p_amount, 9);
	mes_tokens[3] = p_amount;

	char p_max_price[10];
	itoa(b->cur_market_state->max_product_price, p_max_price, 9);
	mes_tokens[4] = p_max_price;

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int player_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
	{
		return 0;
	}


	int player_number = *((int*) chp->param1);
	int i;
	Player* other_p;
	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		other_p = b->pl_array[i];
		if ( other_p != NULL )
			if ( other_p->number == player_number )
				break;
	}

	if ( i >= MAX_PLAYERS )
	{
		const char* error_message[] =
		{
					info_game_messages[PLAYER_COMMAND_NOT_FOUND],
					NULL
		};
		send_message(p->fd, error_message, 1, p->ip);
		return 1;
	}

	int tokns_amnt = (p->is_bot) ? 10 : 9;
	char* mes_tokens[tokns_amnt];
	mes_tokens[0] = (char*)info_game_messages[PLAYER_COMMAND];

	char p_number[10];
	itoa(other_p->number, p_number, 9);
	mes_tokens[1] = p_number;

	char p_money[20];
	itoa(other_p->money, p_money, 19);
	mes_tokens[2] = p_money;

	char p_income[20];
	itoa(other_p->income, p_income, 19);
	mes_tokens[3] = p_income;

	char p_sources[10];
	itoa(other_p->sources, p_sources, 9);
	mes_tokens[4] = p_sources;

	char p_products[10];
	itoa(other_p->products, p_products, 9);
	mes_tokens[5] = p_products;

	char p_w_f[10];
	itoa(other_p->wait_factories, p_w_f, 9);
	mes_tokens[6] = p_w_f;

	char p_wrk_f[10];
	itoa(other_p->work_factories, p_wrk_f, 9);
	mes_tokens[7] = p_wrk_f;

	char p_b_f[10];
	itoa(other_p->build_factories, p_b_f, 9);
	mes_tokens[8] = p_b_f;

	char produced[10];
	if ( p->is_bot )
	{
		itoa(other_p->produced_on_turn, produced, 9);
		mes_tokens[9] = produced;
	}

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int list_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL)
		)
	{
		return 0;
	}

	int alive_amount = b->alive_players;

	char* mes_tokens[2];
	int tokns_amnt = 2;
	mes_tokens[0] = (char*)info_game_messages[LIST_COMMAND];

	char alive_p[10];
	itoa(alive_amount, alive_p, 9);
	mes_tokens[1] = alive_p;

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int prod_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
	{
		return 0;
	}

	if ( p->sources >= 1 )
	{
		if ( p->money >= PRODUCTION_PRODUCT_COST )
		{
			if ( p->wait_factories >= 1 )
			{
				p->wait_factories--;
				p->work_factories++;
				p->sources -= 1;
				p->money -= PRODUCTION_PRODUCT_COST;
				p->is_prod = 1;

				const char* success_message[] =
				{
							info_game_messages[PROD_COMMAND_SUCCESS],
							NULL
				};
				send_message(p->fd, success_message, 1, p->ip);
			}
			else
			{
				const char* factories_message[] =
				{
								info_game_messages[PROD_COMMAND_NO_FACTORIES],
								NULL
				};
				send_message(p->fd, factories_message, 1, p->ip);
			}
		}
		else
		{
			const char* money_message[] =
			{
							info_game_messages[PROD_COMMAND_NO_MONEY],
							NULL
			};
			send_message(p->fd, money_message, 1, p->ip);
		}
	}
	else
	{
		const char* sources_message[] =
		{
						info_game_messages[PROD_COMMAND_NO_SOURCE],
						NULL
		};
		send_message(p->fd, sources_message, 1, p->ip);
	}

	return 1;
}

static int build_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
	{
		return 0;
	}

	if ( chp->param1 != NULL )
	{
		char arg[100];
		char* arg1_p = (char*)chp->param1;
		strncpy(arg, arg1_p, 99);


		if ( strcmp( arg, "list") != 0 )
		{
			const char* err_msg[] =
			{
							error_game_messages[COMMAND_INCORRECT_ARGUMENTS_NUM],
							NULL
			};
			send_message(p->fd, err_msg, 1, p->ip);
		}
		else
		{
			BuildList* build_list = p->build_list;

			if ( build_list != NULL )
			{
				int list_amount = bl_get_size(build_list);
				int tokns_amnt = list_amount * 2 + 2;
				char* mes_tokens[tokns_amnt];

				for ( int j = 0; j < tokns_amnt; ++j )
					mes_tokens[j] = NULL;


				mes_tokens[0] = (char*)info_game_messages[BUILDING_FACTORIES_LIST];

				char la_buf[10];
				itoa(list_amount, la_buf, 9);
				mes_tokens[1] = la_buf;

				int i = 2;
				while ( build_list != NULL )
				{
					mes_tokens[i] = malloc( 10 );
					itoa(build_list->build_number, mes_tokens[i], 9);
					++i;


					mes_tokens[i] = malloc( 10 );
					itoa(build_list->turns_left, mes_tokens[i], 9);
					++i;


					build_list = build_list->next;
				}

				send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

				for ( int j = 2; j < tokns_amnt; ++j )
				{
					if ( mes_tokens[j] != NULL )
					{
						free( mes_tokens[j] );
						mes_tokens[j] = NULL;
					}
				}
			}
			else
			{
				const char* bl_empty_mes[] =
				{
								info_game_messages[BUILDING_FACTORIES_LIST_EMPTY],
								NULL
				};
				send_message(p->fd, bl_empty_mes, 1, p->ip);
			}
		}
	}
	else
	{
		if ( p->money >= NEW_FACTORY_UNIT_COST/2 )
		{
			if ( bl_insert(&p->build_list) )
			{
				p->money -= NEW_FACTORY_UNIT_COST/2;
				p->build_factories += 1;

				const char* success_message[] =
				{
								info_game_messages[BUILD_COMMAND_SUCCESS],
								NULL
				};
				send_message(p->fd, success_message, 1, p->ip);
			}
			else
			{
				const char* error_message[] =
				{
								info_game_messages[COMMAND_INTERNAL_ERROR],
								NULL
				};
				send_message(p->fd, error_message, 1, p->ip);
			}
		}
		else
		{
			const char* no_money_mes[] =
			{
							info_game_messages[BUILD_COMMAND_NO_MONEY],
							NULL
			};
			send_message(p->fd, no_money_mes, 1, p->ip);
		}
	}

	return 1;
}

static int buy_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
	{
		return 0;
	}

	if ( p->sent_source_request )
	{
/////
		const char* message[] =
		{
						info_game_messages[BUY_COMMAND_ALREADY_SENT],
						NULL
		};
		send_message(p->fd, message, 1, p->ip);

		return 1;
	}


	int source_amount = *( (int*) chp->param1 );
	int source_price = *( (int*) chp->param2 );
	int cur_min_source_price = b->cur_market_state->min_source_price;


	if ( (source_amount > 0) && (source_amount <= b->cur_market_state->source_amount) )
	{
		if ( source_price >= cur_min_source_price )
		{
			MarketData data;
			data.amount = source_amount;
			data.price = source_price;
			data.success = 0;
			data.p = p;

			if ( data.p->money < data.price*data.amount )
			{
/////
				const char* no_money_mes[] =
				{
								info_game_messages[BUY_COMMAND_NO_MONEY],
								NULL
				};
				send_message(p->fd, no_money_mes, 1, p->ip);

				return 1;
			}

			mr_insert(&b->sources_requests, &data);
			p->sent_source_request = 1;

/////
			int tokns_amnt = 3;
			char* mes_tokens[tokns_amnt];

			mes_tokens[0] = (char*)info_game_messages[BUY_COMMAND_SUCCESS];

			char sa[10];
			itoa(source_amount, sa, 9);
			mes_tokens[1] = sa;

			char sp[20];
			itoa(source_price, sp, 19);
			mes_tokens[2] = sp;

			send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
		}
		else
		{
/////
			const char* incorrect_price_mes[] =
			{
							info_game_messages[BUY_COMMAND_INCORRECT_PRICE],
							NULL
			};
			send_message(p->fd, incorrect_price_mes, 1, p->ip);
		}
	}
	else
	{
/////
		const char* incorrect_amount_mes[] =
		{
						info_game_messages[BUY_COMMAND_INCORRECT_AMOUNT],
						NULL
		};
		send_message(p->fd, incorrect_amount_mes, 1, p->ip);
	}

	return 1;
}

static int sell_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
	{
		return 0;
	}

	if ( p->sent_products_request )
	{
/////
		const char* message[] =
		{
						info_game_messages[SELL_COMMAND_ALREADY_SENT],
						NULL
		};
		send_message(p->fd, message, 1, p->ip);

		return 1;
	}

	int product_amount = *( (int*) chp->param1 );
	int product_price = *( (int*) chp->param2 );
	int cur_max_product_price = b->cur_market_state->max_product_price;


	if ( ( product_amount > 0 ) && ( product_amount <= p->products ) )
	{
		if ( ( product_price > 0 ) && ( product_price <= cur_max_product_price ) )
		{
			MarketData data;
			data.amount = product_amount;
			data.price = product_price;
			data.success = 0;
			data.p = p;

			mr_insert(&b->products_requests, &data);
			p->sent_products_request = 1;

/////
			int tokns_amnt = 3;
			char* mes_tokens[tokns_amnt];

			mes_tokens[0] = (char*)info_game_messages[SELL_COMMAND_SUCCESS];

			char pa[10];
			itoa(product_amount, pa, 9);
			mes_tokens[1] = pa;

			char pp[20];
			itoa(product_price, pp, 19);
			mes_tokens[2] = pp;

			send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
		}
		else
		{
/////
			const char* incorrect_price_mes[] =
			{
							info_game_messages[SELL_COMMAND_INCORRECT_PRICE],
							NULL
			};
			send_message(p->fd, incorrect_price_mes, 1, p->ip);
		}
	}
	else
	{
/////
		const char* incorrect_amount_mes[] =
		{
						info_game_messages[SELL_COMMAND_INCORRECT_AMOUNT],
						NULL
		};
		send_message(p->fd, incorrect_amount_mes, 1, p->ip);
	}

	return 1;
}

static int turn_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
	{
		return 0;
	}

	p->is_turn = 1;
	b->ready_players++;

	int wait_yet_players_amount = b->alive_players - b->ready_players;

/////
	int tokns_amnt = 2;
	char* mes_tokens[tokns_amnt];

	mes_tokens[0] = (char*)info_game_messages[TURN_COMMAND_SUCCESS];

	char w_y_p_a[10];
	itoa(wait_yet_players_amount, w_y_p_a, 9);
	mes_tokens[1] = w_y_p_a;

	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* other_p = b->pl_array[i];
		if ( other_p != NULL )
			if ( other_p->is_turn )
				send_message(other_p->fd, (const char**)mes_tokens, tokns_amnt, other_p->ip);
	}

	return 1;
}

static int quit_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )					||
			( p == NULL )					||
			( chp == NULL )					||
			( !player_left_game(b, p) )
		)
	{
		return 0;
	}

	return 1;
}

#endif
