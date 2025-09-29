#ifndef COMMANDS_HANDLER_C
#define COMMANDS_HANDLER_C

#include "../includes/CommandsHandler.h"
#include "../includes/Banker.h"
#include "../includes/MGLib.h"
#include "../includes/Player.h"


extern int strcmp(const char* s1, const char* s2);


/* Системные игровые константы */
enum
{
	PRODUCTION_COST				=			2000,
	NEW_FACTORY_COST			=			5000
};

// Описаны в модуле MGLib
extern const char* info_game_messages[];
extern const char* error_game_messages[];

/* Описана в модуле serverCore */
extern int log_info_count;

/* Описана в модуле serverCore */
extern int send_message(int fd, const char** message_tokens, int tokens_amount, const char* ip);

/* Описана в модуле serverCore */
extern int player_left_game(Banker*, Player*, int, fd_set*);

/* Описана в модуле serverCore */
extern int server_end_work(Banker*, Player*, int, fd_set*);

void help_command_handler(Banker* b, CommandHandlerParams* chp)
{
	char* mes_tokens[11];
	int tokns_amnt = 11;

	mes_tokens[0] = (char*)info_game_messages[HELP_COMMAND];

	int i = 1, j;
	for ( j = 0; b->valid_commands[j] != NULL; j++, i++ )
		mes_tokens[i] = (char*) b->valid_commands[j];

	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* p = b->pl_array[i];
		if ( p != NULL )
		{
			if ( p->fd == chp->fd )
			{
				send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
				return;
			}
		}
	}
}

void market_command_handler(Banker* b, CommandHandlerParams* chp)
{
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

	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* p = b->pl_array[i];
		if ( p != NULL )
		{
			if ( p->fd == chp->fd )
			{
				send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
				return;
			}
		}
	}
}

void player_command_handler(Banker* b, CommandHandlerParams* chp)
{
	int player_number = *((int*) chp->param1);
	int i;
	Player* other_p;
	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		other_p = b->pl_array[i];
		if ( other_p != NULL )
			if ( other_p->number == player_number)
				break;
	}


	Player* cur_p;
	int j;
	for ( j = 0; j < MAX_PLAYERS; j++ )
	{
		cur_p = b->pl_array[j];
		if ( cur_p != NULL )
			if ( cur_p->fd == chp->fd )
				break;
	}
	if ( j >= MAX_PLAYERS )
		return;


	if ( i >= MAX_PLAYERS )
	{
		const char* error_message[] =
		{
					info_game_messages[PLAYER_COMMAND_NOT_FOUND],
					NULL
		};
		send_message(chp->fd, error_message, 1, cur_p->ip);
		return;
	}

	int tokns_amnt = (cur_p->is_bot) ? 10 : 9;
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
	if ( cur_p->is_bot )
	{
		itoa(other_p->produced_on_turn, produced, 9);
		mes_tokens[9] = produced;
	}

	send_message(chp->fd, (const char**)mes_tokens, tokns_amnt, cur_p->ip);
}

void list_command_handler(Banker* b, CommandHandlerParams* chp)
{
	int alive_amount = b->alive_players;

	char* mes_tokens[2];
	int tokns_amnt = 2;
	mes_tokens[0] = (char*)info_game_messages[LIST_COMMAND];

	char alive_p[10];
	itoa(alive_amount, alive_p, 9);
	mes_tokens[1] = alive_p;

	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* p = b->pl_array[i];
		if ( p != NULL )
		{
			if ( p->fd == chp->fd )
			{
				send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
				return;
			}
		}
	}
}

void prod_command_handler(Banker* b, CommandHandlerParams* chp)
{
	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
		if ( b->pl_array[i] != NULL )
			if ( b->pl_array[i]->fd == chp->fd)
				break;

	if ( i >= MAX_PLAYERS )
		return;

	Player* p = b->pl_array[i];
	if ( p->sources >= 1 )
	{
		if ( p->money >= PRODUCTION_COST )
		{
			if ( p->wait_factories >= 1 )
			{
				p->wait_factories--;
				p->work_factories++;
				p->sources -= 1;
				p->money -= PRODUCTION_COST;
				p->is_prod = 1;

				const char* success_message[] =
				{
							info_game_messages[PROD_COMMAND_SUCCESS],
							NULL
				};
				send_message(chp->fd, success_message, 1, p->ip);
			}
			else
			{
				const char* factories_message[] =
				{
								info_game_messages[PROD_COMMAND_NO_FACTORIES],
								NULL
				};
				send_message(chp->fd, factories_message, 1, p->ip);
			}
		}
		else
		{
			const char* money_message[] =
			{
							info_game_messages[PROD_COMMAND_NO_MONEY],
							NULL
			};
			send_message(chp->fd, money_message, 1, p->ip);
		}
	}
	else
	{
		const char* sources_message[] =
		{
						info_game_messages[PROD_COMMAND_NO_SOURCE],
						NULL
		};
		send_message(chp->fd, sources_message, 1, p->ip);
	}
}

void build_command_handler(Banker* b, CommandHandlerParams* chp)
{
	//////////////////////////////////////////////////////////
	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
		if ( b->pl_array[i] != NULL )
			if ( b->pl_array[i]->fd == chp->fd)
				break;

	if ( i >= MAX_PLAYERS )
		return;
	//////////////////////////////////////////////////////////


	Player* p = b->pl_array[i];

	if ( chp->param1 != NULL )
	{
		char arg[100];
		char* arg1_p = (char*)chp->param1;
		int j;
		for ( j = 0; arg1_p[j]; j++ )
			arg[j] = arg1_p[j];
		arg[j] = '\0';

		/*printf("arg = %s\n", arg);*/

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
		if ( p->money >= NEW_FACTORY_COST/2 )
		{
			if ( bl_insert(&p->build_list) )
			{
				p->money -= NEW_FACTORY_COST/2;
				p->build_factories += 1;

				const char* success_message[] =
				{
								info_game_messages[BUILD_COMMAND_SUCCESS],
								NULL
				};
				send_message(chp->fd, success_message, 1, p->ip);
			}
			else
			{
				const char* error_message[] =
				{
								info_game_messages[COMMAND_INTERNAL_ERROR],
								NULL
				};
				send_message(chp->fd, error_message, 1, p->ip);
			}
		}
		else
		{
			const char* no_money_mes[] =
			{
							info_game_messages[BUILD_COMMAND_NO_MONEY],
							NULL
			};
			send_message(chp->fd, no_money_mes, 1, p->ip);
		}
	}
}

void buy_command_handler(Banker* b, CommandHandlerParams* chp)
{
	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
		if ( b->pl_array[i] != NULL )
			if ( b->pl_array[i]->fd == chp->fd)
				break;

	if ( i >= MAX_PLAYERS )
		return;

	Player* p = b->pl_array[i];

	if ( p->sent_source_request )
	{
		const char* message[] =
		{
						info_game_messages[BUY_COMMAND_ALREADY_SENT],
						NULL
		};
		send_message(chp->fd, message, 1, p->ip);
		return;
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
				send_message(chp->fd, no_money_mes, 1, data.p->ip);
				return;
			}

			mr_insert(&b->sources_requests, &data);
			p->sent_source_request = 1;

			char* mes_tokens[3];
			int tokns_amnt = 3;

			mes_tokens[0] = (char*)info_game_messages[BUY_COMMAND_SUCCESS];

			char sa[10];
			itoa(source_amount, sa, 9);
			mes_tokens[1] = sa;

			char sp[20];
			itoa(source_price, sp, 19);
			mes_tokens[2] = sp;

			send_message(chp->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
		}
		else
		{
			const char* incorrect_price_mes[] =
			{
							info_game_messages[BUY_COMMAND_INCORRECT_PRICE],
							NULL
			};
			send_message(chp->fd, incorrect_price_mes, 1, p->ip);
		}
	}
	else
	{
		const char* incorrect_amount_mes[] =
		{
						info_game_messages[BUY_COMMAND_INCORRECT_AMOUNT],
						NULL
		};
		send_message(chp->fd, incorrect_amount_mes, 1, p->ip);
	}
}

void sell_command_handler(Banker* b, CommandHandlerParams* chp)
{
	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
		if ( b->pl_array[i] != NULL )
			if ( b->pl_array[i]->fd == chp->fd )
				break;

	if ( i >= MAX_PLAYERS )
		return;

	Player* p = b->pl_array[i];

	if ( p->sent_products_request )
	{
		const char* message[] =
		{
						info_game_messages[SELL_COMMAND_ALREADY_SENT],
						NULL
		};
		send_message(chp->fd, message, 1, p->ip);
		return;
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

			char* mes_tokens[3];
			int tokns_amnt = 3;

			mes_tokens[0] = (char*)info_game_messages[SELL_COMMAND_SUCCESS];

			char pa[10];
			itoa(product_amount, pa, 9);
			mes_tokens[1] = pa;

			char pp[20];
			itoa(product_price, pp, 19);
			mes_tokens[2] = pp;

			send_message(chp->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
		}
		else
		{
			const char* incorrect_price_mes[] =
			{
							info_game_messages[SELL_COMMAND_INCORRECT_PRICE],
							NULL
			};
			send_message(chp->fd, incorrect_price_mes, 1, p->ip);
		}
	}
	else
	{
		const char* incorrect_amount_mes[] =
		{
						info_game_messages[SELL_COMMAND_INCORRECT_AMOUNT],
						NULL
		};
		send_message(chp->fd, incorrect_amount_mes, 1, p->ip);
	}
}

void turn_command_handler(Banker* b, CommandHandlerParams* chp)
{
	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
		if ( b->pl_array[i] != NULL )
			if ( b->pl_array[i]->fd == chp->fd)
				break;

	if ( i >= MAX_PLAYERS )
		return;

	Player* p = b->pl_array[i];
	p->is_turn = 1;
	b->ready_players++;

	int wait_yet_players_amount = b->alive_players - b->ready_players;

	char* mes_tokens[2];
	int tokns_amnt = 2;

	mes_tokens[0] = (char*)info_game_messages[TURN_COMMAND_SUCCESS];

	char w_y_p_a[10];
	itoa(wait_yet_players_amount, w_y_p_a, 9);
	mes_tokens[1] = w_y_p_a;

	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* p = b->pl_array[i];
		if ( p != NULL )
			if ( p->is_turn )
				send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
	}
}

void quit_command_handler(Banker* b, CommandHandlerParams* chp)
{
	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
		if ( b->pl_array[i] != NULL )
			if ( b->pl_array[i]->fd == chp->fd )
				break;

	if ( i >= MAX_PLAYERS )
		return;

	Player* p = b->pl_array[i];
	int left_pl_num = p->number;
	player_left_game(b, p, i, (fd_set*) chp->param1);

	if ( b->alive_players == 1 )
	{
		for ( i = 0; i < MAX_PLAYERS; i++ )
			if ( b->pl_array[i] != NULL )
				break;

		if ( i >= MAX_PLAYERS )
			return;

		Player* p = b->pl_array[i];
		server_end_work(b, p, i, (fd_set*) chp->param1);
	}
	else if ( b->alive_players > 1 )
	{
		int tokns_amnt = 3;
		char* mes_tokens[tokns_amnt];

		mes_tokens[0] = (char*)info_game_messages[LOST_ALIVE_PLAYER];

		char ap_buf[10];
		int ap = b->alive_players;
		itoa(ap, ap_buf, 9);
		mes_tokens[1] = ap_buf;

		char left_p_num_buf[10];
		itoa(left_pl_num, left_p_num_buf, 9);
		mes_tokens[2] = left_p_num_buf;

		for ( i = 0; i < MAX_PLAYERS; i++ )
		{
			Player* p = b->pl_array[i];
			if ( p != NULL )
				send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);
		}
	}
}
#endif
