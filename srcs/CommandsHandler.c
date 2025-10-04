/* Реализация объекта CommandsHandler */
#ifndef COMMANDS_HANDLER_C
#define COMMANDS_HANDLER_C


#include "../includes/CommandsHandler.h"
#include "../includes/Banker.h"
#include "../includes/MGLib.h"
#include <stdlib.h>


// Описаны в модуле MGLib
extern const char* info_game_messages[];
extern const char* error_game_messages[];

// Описаны в <string.h>
extern char* strncpy(char* dest, const char* src, size_t n);
extern int strcmp(const char* s1, const char* s2);


static int send_help_command_message(Banker*, Player*, CommandHandlerParams*);
static int send_market_command_message(Banker*, Player*, CommandHandlerParams*);
static int send_player_command_message(Banker*, Player*, CommandHandlerParams*);
static int send_list_command_message(Banker*, Player*, CommandHandlerParams*);
static int send_prod_command_message(Banker*, Player*, CommandHandlerParams*);
static int send_build_command_message(Banker*, Player*, CommandHandlerParams*);
static int send_buy_command_message(Banker*, Player*, CommandHandlerParams*);
static int send_sell_command_message(Banker*, Player*, CommandHandlerParams*);
static int send_turn_command_message(Banker*, Player*, CommandHandlerParams*);
static int send_quit_command_message(Banker*, Player*, CommandHandlerParams*);



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

/* Список функций-обработчиков валидных команд(порядок должен совпадать с порядком следования элементов в valid_commands) */
int (*commands_handlers[])(Banker*, Player*, CommandHandlerParams* ) =
{
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




static int send_help_command_message(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
		return 0;

	int tokns_amnt = 11;
	char* mes_tokens[tokns_amnt];

	mes_tokens[0] = (char*)info_game_messages[HELP_COMMAND];
	for ( int j = 0, i = 1; b->valid_commands[j] != NULL; j++, i++ )
		mes_tokens[i] = (char*) b->valid_commands[j];

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int send_market_command_message(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
		return 0;

	char s_amount[10];
	itoa(b->cur_market_state->source_amount, s_amount, 9);

	char s_min_price[10];
	itoa(b->cur_market_state->min_source_price, s_min_price, 9);

	char p_amount[10];
	itoa(b->cur_market_state->product_amount, p_amount, 9);

	char p_max_price[10];
	itoa(b->cur_market_state->max_product_price, p_max_price, 9);


	const int tokns_amnt = 5;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[MARKET_COMMAND],
				s_amount,
				s_min_price,
				p_amount,
				p_max_price,
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int send_player_command_message(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
		return 0;

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


	char p_number[10];
	itoa(other_p->number, p_number, 9);

	char p_money[20];
	itoa(other_p->money, p_money, 19);

	char p_income[20];
	itoa(other_p->income, p_income, 19);

	char p_sources[10];
	itoa(other_p->sources, p_sources, 9);

	char p_products[10];
	itoa(other_p->products, p_products, 9);

	char p_w_f[10];
	itoa(other_p->wait_factories, p_w_f, 9);

	char p_wrk_f[10];
	itoa(other_p->work_factories, p_wrk_f, 9);

	char p_b_f[10];
	itoa(other_p->build_factories, p_b_f, 9);

	char produced[10];
	if ( p->is_bot )
	{
		itoa(other_p->produced_on_turn, produced, 9);
	}

	int tokns_amnt = (p->is_bot) ? 10 : 9;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[PLAYER_COMMAND],
				p_number,
				p_money,
				p_income,
				p_sources,
				p_products,
				p_w_f,
				p_wrk_f,
				p_b_f,
				produced,
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int send_list_command_message(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
		return 0;

	char alive_p[10];
	itoa(b->alive_players, alive_p, 9);

	int tokns_amnt = 2;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[LIST_COMMAND],
				alive_p,
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int send_prod_command_message(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
		return 0;

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

static int send_build_command_message(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
		return 0;

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

static int send_buy_command_message(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
		return 0;

	if ( p->sent_source_request )
	{
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

			char sa[10];
			itoa(source_amount, sa, 9);

			char sp[20];
			itoa(source_price, sp, 19);

			int tokns_amnt = 3;
			char* mes_tokens[] =
			{
						(char*)info_game_messages[BUY_COMMAND_SUCCESS],
						sa,
						sp,
						NULL
			};

			send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
		}
		else
		{
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
		const char* incorrect_amount_mes[] =
		{
						info_game_messages[BUY_COMMAND_INCORRECT_AMOUNT],
						NULL
		};
		send_message(p->fd, incorrect_amount_mes, 1, p->ip);
	}

	return 1;
}

static int send_sell_command_message(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
		return 0;

	if ( p->sent_products_request )
	{
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

			char pa[10];
			itoa(product_amount, pa, 9);

			char pp[20];
			itoa(product_price, pp, 19);

			int tokns_amnt = 3;
			char* mes_tokens[] =
			{
						(char*)info_game_messages[SELL_COMMAND_SUCCESS],
						pa,
						pp,
						NULL
			};

			send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
		}
		else
		{
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
		const char* incorrect_amount_mes[] =
		{
						info_game_messages[SELL_COMMAND_INCORRECT_AMOUNT],
						NULL
		};
		send_message(p->fd, incorrect_amount_mes, 1, p->ip);
	}

	return 1;
}

static int send_turn_command_message(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )			||
			( p == NULL )			||
			( chp == NULL )
		)
		return 0;

	p->is_turn = 1;
	b->ready_players++;

	int wait_yet_players_amount = b->alive_players - b->ready_players;

	char w_y_p_a[10];
	itoa(wait_yet_players_amount, w_y_p_a, 9);

	int tokns_amnt = 2;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[TURN_COMMAND_SUCCESS],
				w_y_p_a,
				NULL
	};

	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* other_p = b->pl_array[i];
		if ( other_p != NULL )
			if ( other_p->is_turn )
				send_message(other_p->fd, (const char**)mes_tokens, tokns_amnt, other_p->ip);
	}

	return 1;
}

static int send_quit_command_message(Banker* b, Player* p, CommandHandlerParams* chp)
{
	return 1;
}


static int help_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )									||
			( p == NULL )									||
			( chp == NULL )									||
			( !send_help_command_message(b, p, chp) )
		)
	{
		return 0;
	}

	return 1;
}

static int market_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )									||
			( p == NULL )									||
			( chp == NULL )									||
			( !send_market_command_message(b, p, chp) )
		)
	{
		return 0;
	}

	return 1;
}

static int player_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )									||
			( p == NULL )									||
			( chp == NULL )									||
			( !send_player_command_message(b, p, chp) )
		)
	{
		return 0;
	}

	return 1;
}

static int list_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )									||
			( p == NULL )									||
			( chp == NULL )									||
			( !send_list_command_message(b, p, chp) )
		)
	{
		return 0;
	}

	return 1;
}

static int prod_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )									||
			( p == NULL )									||
			( chp == NULL )									||
			( !send_prod_command_message(b, p, chp) )
		)
	{
		return 0;
	}

	return 1;
}

static int build_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )									||
			( p == NULL )									||
			( chp == NULL )									||
			( !send_build_command_message(b, p, chp) )
		)
	{
		return 0;
	}

	return 1;
}

static int buy_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )									||
			( p == NULL )									||
			( chp == NULL )									||
			( !send_buy_command_message(b, p, chp) )
		)
	{
		return 0;
	}

	return 1;
}

static int sell_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )									||
			( p == NULL )									||
			( chp == NULL )									||
			( !send_sell_command_message(b, p, chp) )
		)
	{
		return 0;
	}

	return 1;
}

static int turn_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )									||
			( p == NULL )									||
			( chp == NULL )									||
			( !send_turn_command_message(b, p, chp) )
		)
	{
		return 0;
	}

	return 1;
}

static int quit_command_handler(Banker* b, Player* p, CommandHandlerParams* chp)
{
	if (
			( b == NULL )					||
			( p == NULL )					||
			( chp == NULL )
		)
	{
		return 0;
	}

	return 1;
}

#endif
