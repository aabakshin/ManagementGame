#ifndef COMMANDS_HANDLER_C
#define COMMANDS_HANDLER_C

#include "../includes/CommandsHandler.h"
#include "../includes/Banker.h"
#include "../includes/MGLib.h"
#include "../includes/Player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Системные игровые константы */
enum
{
	PRODUCTION_COST				=			2000,
	NEW_FACTORY_COST			=			5000
};

/* Описана в модуле serverCore */
extern int log_info_count;

/* Описана в модуле serverCore */
extern int send_data(int fd, const char* send_buf, int mes_len, const char* ip);

/* Описана в модуле serverCore */
extern int send_message(int fd, char** message_tokens, int tokens_amount, const char* ip);

/* Описана в модуле serverCore */
extern int player_left_game(Banker*, Player*, int, fd_set*);

/* Описана в модуле serverCore */
extern int server_end_work(Banker*, Player*, int, fd_set*);

void help_command_handler(Banker* b, CommandHandlerParams* chp)
{
	char* mes_tokens[11];
	int tokns_amnt = 11;

	mes_tokens[0] = "*INFO_MESSAGE|HELP_COMMAND|";

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
				send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
				return;
			}
		}
	}
}

void market_command_handler(Banker* b, CommandHandlerParams* chp)
{
	char* mes_tokens[5];
	int tokns_amnt = 5;
	mes_tokens[0] = "*INFO_MESSAGE|MARKET_COMMAND|";

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
				send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
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
		const char* error_message = "*INFO_MESSAGE|PLAYER_COMMAND_NOT_FOUND\n";
		int em_len = strlen(error_message);
		send_data(chp->fd, error_message, em_len, cur_p->ip);
		return;
	}

	int tokns_amnt = (cur_p->is_bot) ? 10 : 9;
	char* mes_tokens[tokns_amnt];
	mes_tokens[0] = "*INFO_MESSAGE|PLAYER_COMMAND|";

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

	send_message(chp->fd, mes_tokens, tokns_amnt, cur_p->ip);
}

void list_command_handler(Banker* b, CommandHandlerParams* chp)
{
	int alive_amount = b->alive_players;

	char* mes_tokens[2];
	int tokns_amnt = 2;
	mes_tokens[0] = "*INFO_MESSAGE|LIST_COMMAND|";

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
				send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
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

				const char* success_message = "*INFO_MESSAGE|PROD_COMMAND_SUCCESS\n";
				int sm_len = strlen(success_message);
				send_data(chp->fd, success_message, sm_len, p->ip);
			}
			else
			{
				const char* factories_message = "*INFO_MESSAGE|PROD_COMMAND_NO_FACTORIES\n";
				int sm_len = strlen(factories_message);
				send_data(chp->fd, factories_message, sm_len, p->ip);
			}
		}
		else
		{
			const char* money_message = "*INFO_MESSAGE|PROD_COMMAND_NO_MONEY\n";
			int sm_len = strlen(money_message);
			send_data(chp->fd, money_message, sm_len, p->ip);
		}
	}
	else
	{
		const char* sources_message = "*INFO_MESSAGE|PROD_COMMAND_NO_SOURCE\n";
		int sm_len = strlen(sources_message);
		send_data(chp->fd, sources_message, sm_len, p->ip);
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
			const char* err_msg = "*ERROR_MESSAGE|COMMAND_INCORRECT_ARGUMENTS_NUM\n";
			int mes_len = strlen(err_msg);
			send_data(p->fd, err_msg, mes_len, p->ip);
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


				mes_tokens[0] = "*INFO_MESSAGE|BUILDING_FACTORIES_LIST|";

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

				send_message(p->fd, mes_tokens, tokns_amnt, p->ip);

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
				const char* bl_empty_mes = "*INFO_MESSAGE|BUILDING_FACTORIES_LIST_EMPTY\n";
				int mes_len = strlen(bl_empty_mes);
				send_data(p->fd, bl_empty_mes, mes_len, p->ip);
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

				const char* success_message = "*INFO_MESSAGE|BUILD_COMMAND_SUCCESS\n";
				int mes_len = strlen(success_message);
				send_data(chp->fd, success_message, mes_len, p->ip);
			}
			else
			{
				const char* error_message = "*ERROR_MESSAGE|COMMAND_INTERNAL_ERROR\n";
				int em_len = strlen(error_message);
				send_data(chp->fd, error_message, em_len, p->ip);
			}
		}
		else
		{
			const char* no_money_mes = "*INFO_MESSAGE|BUILD_COMMAND_NO_MONEY\n";
			int mes_len = strlen(no_money_mes);
			send_data(chp->fd, no_money_mes, mes_len, p->ip);
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
		const char* message = "*INFO_MESSAGE|BUY_COMMAND_ALREADY_SENT\n";
		int mes_len = strlen(message);
		send_data(chp->fd, message, mes_len, p->ip);
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
				const char* no_money_mes = "*INFO_MESSAGE|BUY_COMMAND_NO_MONEY\n";
				int mes_len = strlen(no_money_mes);
				send_data(chp->fd, no_money_mes, mes_len, data.p->ip);
				return;
			}

			mr_insert(&b->sources_requests, &data);
			p->sent_source_request = 1;

			char* mes_tokens[3];
			int tokns_amnt = 3;

			mes_tokens[0] = "*INFO_MESSAGE|BUY_COMMAND_SUCCESS|";

			char sa[10];
			itoa(source_amount, sa, 9);
			mes_tokens[1] = sa;

			char sp[20];
			itoa(source_price, sp, 19);
			mes_tokens[2] = sp;

			send_message(chp->fd, mes_tokens, tokns_amnt, p->ip);
		}
		else
		{
			const char* incorrect_price_mes = "*INFO_MESSAGE|BUY_COMMAND_INCORRECT_PRICE\n";
			int mes_len = strlen(incorrect_price_mes);
			send_data(chp->fd, incorrect_price_mes, mes_len, p->ip);
		}
	}
	else
	{
		const char* incorrect_amount_mes = "*INFO_MESSAGE|BUY_COMMAND_INCORRECT_AMOUNT\n";
		int mes_len = strlen(incorrect_amount_mes);
		send_data(chp->fd, incorrect_amount_mes, mes_len, p->ip);
	}
}

void sell_command_handler(Banker* b, CommandHandlerParams* chp)
{
	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
		if ( b->pl_array[i] != NULL )
			if ( b->pl_array[i]->fd == chp->fd)
				break;

	if ( i >= MAX_PLAYERS )
		return;

	Player* p = b->pl_array[i];

	if ( p->sent_products_request )
	{
		const char* message = "*INFO_MESSAGE|SELL_COMMAND_ALREADY_SENT\n";
		int mes_len = strlen(message);
		send_data(chp->fd, message, mes_len, p->ip);
		return;
	}

	int product_amount = *( (int*) chp->param1 );
	int product_price = *( (int*) chp->param2 );
	int cur_max_product_price = b->cur_market_state->max_product_price;
	/*printf("\nproduct_price = %d\ncur_max_product_price = %d\n", product_price, b->cur_market_state->max_product_price);*/

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

			mes_tokens[0] = "*INFO_MESSAGE|SELL_COMMAND_SUCCESS|";

			char pa[10];
			itoa(product_amount, pa, 9);
			mes_tokens[1] = pa;

			char pp[20];
			itoa(product_price, pp, 19);
			mes_tokens[2] = pp;

			send_message(chp->fd, mes_tokens, tokns_amnt, p->ip);
		}
		else
		{
			const char* incorrect_price_mes = "*INFO_MESSAGE|SELL_COMMAND_INCORRECT_PRICE\n";
			int mes_len = strlen(incorrect_price_mes);
			send_data(chp->fd, incorrect_price_mes, mes_len, p->ip);
		}
	}
	else
	{
		const char* incorrect_amount_mes = "*INFO_MESSAGE|SELL_COMMAND_INCORRECT_AMOUNT\n";
		int mes_len = strlen(incorrect_amount_mes);
		send_data(chp->fd, incorrect_amount_mes, mes_len, p->ip);
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

	mes_tokens[0] = "*INFO_MESSAGE|TURN_COMMAND_SUCCESS|";

	char w_y_p_a[10];
	itoa(wait_yet_players_amount, w_y_p_a, 9);
	mes_tokens[1] = w_y_p_a;

	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* p = b->pl_array[i];
		if ( p != NULL )
			if ( p->is_turn )
				send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
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

		mes_tokens[0] = "*INFO_MESSAGE|LOST_ALIVE_PLAYER|";

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
				send_message(p->fd, mes_tokens, tokns_amnt, p->ip);
		}
	}
}
#endif
