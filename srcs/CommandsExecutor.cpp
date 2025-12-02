#ifndef COMMANDS_EXECUTOR_CPP
#define COMMANDS_EXECUTOR_CPP


#include "CommandsExecutor.hpp"
#include "MGLib.h"
#include "Player.hpp"
#include <cstdlib>
#include <cstring>


// Описаны в модуле MGLib
extern const char* info_game_messages[];

// Описаны в модуле ServerCore
extern int send_message( const int, const char**, const int, const char* );


static int help_function( CommandsExecutor*, Banker*, int sender_num, int j );
static int market_function( CommandsExecutor*, Banker*, int sender_num, int j );
static int player_function( CommandsExecutor*, Banker*, int sender_num, int j );
static int list_function( CommandsExecutor*, Banker*, int sender_num, int j );
static int produce_function( CommandsExecutor*, Banker*, int sender_num, int j );
static int build_function( CommandsExecutor*, Banker*, int sender_num, int j );
static int buy_function( CommandsExecutor*, Banker*, int sender_num, int j );
static int sell_function( CommandsExecutor*, Banker*, int sender_num, int j );
static int turn_function( CommandsExecutor*, Banker*, int sender_num, int j );
static int quit_function( CommandsExecutor*, Banker*, int sender_num, int j );

static int help_command_handler( Banker*, int sender_num, const CommandsExecutor::CommandParams& );
static int market_command_handler( Banker*, int sender_num, const CommandsExecutor::CommandParams& );
static int player_command_handler( Banker*, int sender_num, const CommandsExecutor::CommandParams& );
static int list_command_handler( Banker*, int sender_num, const CommandsExecutor::CommandParams& );
static int prod_command_handler( Banker*, int sender_num, const CommandsExecutor::CommandParams& );
static int build_command_handler( Banker*, int sender_num, const CommandsExecutor::CommandParams& );
static int buy_command_handler( Banker*, int sender_num, const CommandsExecutor::CommandParams& );
static int sell_command_handler( Banker*, int sender_num, const CommandsExecutor::CommandParams& );
static int turn_command_handler( Banker*, int sender_num, const CommandsExecutor::CommandParams& );
static int quit_command_handler( Banker*, int sender_num, const CommandsExecutor::CommandParams& );

static int send_helpcmd_message( Banker*, int );
static int send_marketcmd_message( Banker*, int );
static int send_playercmdnotfound_message( Banker*, int );
static int send_playercmd_message( Banker*, int );
static int send_listcmd_message( Banker*, int );
static int send_prodcmdsuccess_message( Banker*, int );
static int send_prodcmdnofacts_message( Banker*, int );
static int send_prodcmdnomoney_message( Banker*, int );
static int send_prodcmdnosource_message( Banker*, int );
static int send_buildcmdincorrectarg_message( Banker*, int );
static int send_buildfactslistempty_message( Banker*, int );
static int send_buildfactslist_message( Banker*, int );
static int send_buildcmdsuccess_message( Banker*, int );
static int send_buildcmdnomoney_message( Banker*, int );
static int send_buycmdalreadysent_message( Banker*, int );
static int send_buycmdnomoney_message( Banker*, int );
static int send_buycmdsuccess_message( Banker*, int sender_num, int source_amount, int source_price );
static int send_buycmdincorrectprice_message( Banker*, int );
static int send_buycmdincorrectamount_message(Banker*, int );
static int send_sellcmdalreadysent_message( Banker*, int );
static int send_sellcmdsuccess_message( Banker*, int sender_num, int product_amount, int product_price );
static int send_sellcmdincorrectprice_message( Banker*, int );
static int send_sellcmdincorrectamount_message( Banker*, int );
static int send_turn_command_message( Banker*, int );


// Список действительных игровых команд
static const char* valid_commands[] = {
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
				nullptr
};

// Функции-обработчики валидных команд, выполняющие конкретные действия
static int (*commands_handlers[])( Banker*, int, const CommandsExecutor::CommandParams& ) =
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
					nullptr
};

// Эти функции выполняют вспомогательную работу по подготовке параметров игровой команды перед её выполнением
static int (*exec_game_function[])( CommandsExecutor*, Banker*, int, int ) =
{
				help_function,
				market_function,
				player_function,
				list_function,
				produce_function,
				build_function,
				buy_function,
				sell_function,
				turn_function,
				quit_function,
				nullptr
};


CommandsExecutor::CommandParams::CommandParams()
{
	SetParam1( nullptr );
	SetParam2( nullptr );
}

CommandsExecutor::CommandsExecutor()
{
	SetCmdTokensAmount( 0 );

	for ( int i = 0; i < MAX_CMD_TOKENS_AMOUNT; ++i )
		SetCmdToken(i, nullptr);
}

const char* CommandsExecutor::GetCmdToken( int idx ) const
{
	if ( ( idx < 0 ) || ( idx >= MAX_CMD_TOKENS_AMOUNT ) )
		return nullptr;

	return command_tokens[idx];
}

void CommandsExecutor::SetCmdTokensAmount( int tokens_amount )
{
	if ( ( tokens_amount < 0 ) || ( tokens_amount > MAX_CMD_TOKENS_AMOUNT ) )
	{
		return;
		// throw InvalidCmdTokensAmountException();
	}

	cmd_tokens_amount = tokens_amount;
}

void CommandsExecutor::SetCmdToken( int idx, char* cmd_token )
{
	if ( ( idx < 0 ) || ( idx >= MAX_CMD_TOKENS_AMOUNT ) )
	{
		return;
		// throw InvalidIdxException();
	}

	command_tokens[idx] = cmd_token;
}

void CommandsExecutor::SetCmdParams( const void* value1, const void* value2 )
{
	cmd_params.SetParam1( value1 );
	cmd_params.SetParam2( value2 );
}

int CommandsExecutor::process_command( Banker* b, int sender_num )
{
	if ( b == nullptr )
		return INTERNAL_COMMAND_ERROR;


	char command_str[100];
	strcpy(command_str, GetCmdToken( 0 ));

	for ( int j = 0; valid_commands[j] != nullptr; ++j )
		if ( strcmp(command_str, valid_commands[j]) == 0 )
			return exec_game_function[j](this, b, sender_num, j);


	return UNKNOWN_COMMAND_ERROR;
}

int CommandsExecutor::make_cmd_tokens( const char* read_buf )
{
	if (
					( read_buf == nullptr )				||
					( *read_buf == '\0' )				||
					( *read_buf == '\n' )
		)
		return 0;

	char* istr = strtok(const_cast<char*>(read_buf), " ");
	int j = 0;
	while ( ( istr != nullptr ) && ( j < MAX_CMD_TOKENS_AMOUNT ) )
	{
		SetCmdToken(j, istr);
		j++;
		istr = strtok(nullptr, " ");
	}
	SetCmdTokensAmount( j );

	return 1;
}


static int help_function( CommandsExecutor* cmd_hndl, Banker* b, int sender_num, int j )
{
	commands_handlers[j](b, sender_num, cmd_hndl->GetCmdParams());

	return HELP_COMMAND_NUM;
}

static int market_function( CommandsExecutor* cmd_hndl, Banker* b, int sender_num, int j )
{
	commands_handlers[j](b, sender_num, cmd_hndl->GetCmdParams());

	return MARKET_COMMAND_NUM;
}

static int player_function( CommandsExecutor* cmd_hndl, Banker* b, int sender_num, int j )
{
	if ( cmd_hndl->GetCmdTokensAmount() < 2 )
	{
		return INCORRECT_ARGS_COMMAND_ERROR;
	}

	char param1_str[100];
	strcpy(param1_str, cmd_hndl->GetCmdToken(1));
	cut_str(param1_str, 100, '\n');

	int player_number = atoi(param1_str);
	cmd_hndl->SetCmdParams( reinterpret_cast<void*>(&player_number), nullptr );

	commands_handlers[j](b, sender_num, cmd_hndl->GetCmdParams());

	return PLAYER_COMMAND_NUM;
}

static int list_function( CommandsExecutor* cmd_hndl, Banker* b, int sender_num, int j )
{
	commands_handlers[j](b, sender_num, cmd_hndl->GetCmdParams());

	return LIST_COMMAND_NUM;
}

static int produce_function( CommandsExecutor* cmd_hndl, Banker* b, int sender_num, int j )
{
	if ( !b->GetPlayerByNum(sender_num)->IsTurn() )
	{
		commands_handlers[j](b, sender_num, cmd_hndl->GetCmdParams());

		return PROD_COMMAND_NUM;
	}

	return WFNT_COMMAND_ERROR;
}

static int build_function( CommandsExecutor* cmd_hndl, Banker* b, int sender_num, int j )
{
	if ( !b->GetPlayerByNum(sender_num)->IsTurn() )
	{
		char param1_str[100];
		if ( cmd_hndl->GetCmdTokensAmount() >= 2 )
		{
			strcpy(param1_str, cmd_hndl->GetCmdToken( 1 ));
			cut_str(param1_str, 100, '\n');

			cmd_hndl->SetCmdParams( reinterpret_cast<void*>(param1_str), nullptr );
		}

		commands_handlers[j](b, sender_num, cmd_hndl->GetCmdParams());

		return BUILD_COMMAND_NUM;
	}

	return WFNT_COMMAND_ERROR;
}

static int buy_function( CommandsExecutor* cmd_hndl, Banker* b, int sender_num, int j )
{
	if ( !b->GetPlayerByNum(sender_num)->IsTurn() )
	{
		if ( cmd_hndl->GetCmdTokensAmount() < 3 )
		{
			return INCORRECT_ARGS_COMMAND_ERROR;
		}

		char param1_str[100];
		strcpy(param1_str, cmd_hndl->GetCmdToken( 1 ));
		cut_str(param1_str, 100, '\n');
		int sources_amount = atoi(param1_str);
		cmd_hndl->SetCmdParams( reinterpret_cast<void*>(&sources_amount), nullptr );

		char param2_str[100];
		strcpy(param2_str, cmd_hndl->GetCmdToken( 2 ));
		cut_str(param2_str, 100, '\n');
		int sources_price = atoi(param2_str);
		cmd_hndl->SetCmdParams( reinterpret_cast<void*>(&sources_price), nullptr );

		commands_handlers[j](b, sender_num, cmd_hndl->GetCmdParams());

		return BUY_COMMAND_NUM;
	}

	return WFNT_COMMAND_ERROR;
}

static int sell_function( CommandsExecutor* cmd_hndl, Banker* b, int sender_num, int j )
{
	if ( !b->GetPlayerByNum(sender_num)->IsTurn() )
	{
		if ( cmd_hndl->GetCmdTokensAmount() < 3 )
		{
			return INCORRECT_ARGS_COMMAND_ERROR;
		}

		char param1_str[100];
		strcpy(param1_str, cmd_hndl->GetCmdToken( 1 ));
		cut_str(param1_str, 100, '\n');
		int products_amount = atoi(param1_str);
		cmd_hndl->SetCmdParams( reinterpret_cast<void*>(&products_amount), nullptr );

		char param2_str[100];
		strcpy(param2_str, cmd_hndl->GetCmdToken( 2 ));
		cut_str(param2_str, 100, '\n');
		int products_price = atoi(param2_str);
		cmd_hndl->SetCmdParams( reinterpret_cast<void*>(&products_price), nullptr );

		commands_handlers[j](b, sender_num, cmd_hndl->GetCmdParams());

		return SELL_COMMAND_NUM;
	}

	return WFNT_COMMAND_ERROR;
}

static int turn_function( CommandsExecutor* cmd_hndl, Banker* b, int sender_num, int j )
{
	if ( !b->GetPlayerByNum(sender_num)->IsTurn() )
	{
		commands_handlers[j](b, sender_num, cmd_hndl->GetCmdParams());

		return TURN_COMMAND_NUM;
	}

	return WFNT_COMMAND_ERROR;
}

static int quit_function( CommandsExecutor* cmd_hndl, Banker* b, int sender_num, int j )
{
	commands_handlers[j](b, sender_num, cmd_hndl->GetCmdParams());

	return QUIT_COMMAND_NUM;
}


static int send_helpcmd_message( Banker* b, int sender_num )
{
	const char* mes_tokens[11];

	mes_tokens[0] = const_cast<char*>(info_game_messages[HELP_COMMAND]);
	for ( int j = 0, i = 1; valid_commands[j] != nullptr; ++j, ++i )
		mes_tokens[i] = const_cast<char*>(valid_commands[j]);

	send_message(b->GetPlayerByNum(sender_num)->GetFd(), mes_tokens, 11, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_marketcmd_message( Banker* b, int sender_num )
{
	char s_amount[10];
	itoa(b->GetCurrentMarketState().GetSourcesAmount(), s_amount, 9);

	char s_min_price[10];
	itoa(b->GetCurrentMarketState().GetSourceMinPrice(), s_min_price, 9);

	char p_amount[10];
	itoa(b->GetCurrentMarketState().GetProductsAmount(), p_amount, 9);

	char p_max_price[10];
	itoa(b->GetCurrentMarketState().GetProductMaxPrice(), p_max_price, 9);


	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[MARKET_COMMAND]),
				s_amount,
				s_min_price,
				p_amount,
				p_max_price,
				nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), mes_tokens, 5, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_playercmdnotfound_message( Banker* b, int sender_num )
{
	const char* error_message[] =
	{
				info_game_messages[PLAYER_COMMAND_NOT_FOUND],
				nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), error_message, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_playercmd_message( Banker* b, int sender_num, int target_num )
{
	Player* target_p = b->GetPlayerByNum(target_num);

	char p_number[10];
	itoa(target_p->GetUID(), p_number, 9);

	char p_money[20];
	itoa(target_p->GetMoney(), p_money, 19);

	char p_income[20];
	itoa(target_p->GetIncome(), p_income, 19);

	char p_sources[10];
	itoa(target_p->GetSources(), p_sources, 9);

	char p_products[10];
	itoa(target_p->GetProducts(), p_products, 9);

	char p_w_f[10];
	itoa(target_p->GetWaitFactories(), p_w_f, 9);

	char p_wrk_f[10];
	itoa(target_p->GetWorkFactories(), p_wrk_f, 9);

	char p_b_f[10];
	itoa(target_p->GetBuiltFactories(), p_b_f, 9);

	char produced[10];
	int tokns_amnt = 9;
	if ( b->GetPlayerByNum(sender_num)->IsBot() )
	{
		itoa(target_p->GetProduced(), produced, 9);
		tokns_amnt = 10;
	}

	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[PLAYER_COMMAND]),
				p_number,
				p_money,
				p_income,
				p_sources,
				p_products,
				p_w_f,
				p_wrk_f,
				p_b_f,
				produced,
				nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), mes_tokens, tokns_amnt, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_listcmd_message( Banker* b, int sender_num )
{
	char alive_p[10];
	itoa(b->GetAlivePlayers(), alive_p, 9);


	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[LIST_COMMAND]),
				alive_p,
				nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), const_cast<const char**>(mes_tokens), 2, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_prodcmdsuccess_message( Banker* b, int sender_num )
{
	const char* success_message[] =
	{
				info_game_messages[PROD_COMMAND_SUCCESS],
				nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), success_message, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_prodcmdnofacts_message( Banker* b, int sender_num )
{
	const char* factories_message[] =
	{
					info_game_messages[PROD_COMMAND_NO_FACTORIES],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), factories_message, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_prodcmdnomoney_message( Banker* b, int sender_num )
{
	const char* money_message[] =
	{
					info_game_messages[PROD_COMMAND_NO_MONEY],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), money_message, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_prodcmdnosource_message( Banker* b, int sender_num )
{
	const char* sources_message[] =
	{
					info_game_messages[PROD_COMMAND_NO_SOURCE],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), sources_message, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_buildcmdincorrectarg_message( Banker* b, int sender_num )
{
	const char* incorrect_arg_value[] =
	{
					info_game_messages[BUILD_COMMAND_INCORRECT_ARG],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), incorrect_arg_value, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_buildfactslistempty_message( Banker* b, int sender_num )
{
	const char* bl_empty_mes[] =
	{
					info_game_messages[BUILDING_FACTORIES_LIST_EMPTY],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), bl_empty_mes, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_buildfactslist_message( Banker* b, int sender_num )
{
	const Player::BuildsList& player_builds = b->GetPlayerByNum(sender_num)->GetBuildsFactories();

	if ( !player_builds.IsEmpty() )
	{
		const int tokns_amnt = player_builds.GetSize() * 2 + 2;
		char* mes_tokens[tokns_amnt];

		for ( int j = 0; j < tokns_amnt; ++j )
			mes_tokens[j] = nullptr;


		mes_tokens[0] = const_cast<char*>(info_game_messages[BUILDING_FACTORIES_LIST]);

		char la_buf[10];
		itoa(player_builds.GetSize(), la_buf, 9);
		mes_tokens[1] = la_buf;

		int i = 2;
		for ( Player::BuildsList::BuildsItem* node = player_builds.GetFirst(); node != nullptr; node = node->GetNext() )
		{
			mes_tokens[i] = new char[10];
			itoa( node->GetData().GetBuildNumber(), mes_tokens[i], 9);
			++i;

			mes_tokens[i] = new char[10];
			itoa(node->GetData().GetTurnsLeft(), mes_tokens[i], 9);
			++i;
		}

		send_message(b->GetPlayerByNum(sender_num)->GetFd(), const_cast<const char**>(mes_tokens), tokns_amnt, b->GetPlayerByNum(sender_num)->GetAddr());

		for ( int j = 2; j < tokns_amnt; ++j )
		{
			if ( mes_tokens[j] != nullptr )
			{
				delete[] mes_tokens[j];
				mes_tokens[j] = nullptr;
			}
		}

		return 1;
	}

	send_buildfactslistempty_message(b, sender_num);

	return 1;
}

static int send_buildcmdsuccess_message( Banker* b, int sender_num )
{
	const char* success_message[] =
	{
					info_game_messages[BUILD_COMMAND_SUCCESS],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), success_message, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_buildcmdnomoney_message( Banker* b, int sender_num )
{
	const char* no_money_mes[] =
	{
					info_game_messages[BUILD_COMMAND_NO_MONEY],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), no_money_mes, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_buycmdalreadysent_message( Banker* b, int sender_num )
{
	const char* message[] =
	{
					info_game_messages[BUY_COMMAND_ALREADY_SENT],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), message, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_buycmdnomoney_message( Banker* b, int sender_num )
{
	const char* no_money_mes[] =
	{
					info_game_messages[BUY_COMMAND_NO_MONEY],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), no_money_mes, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_buycmdsuccess_message( Banker* b, int sender_num, int source_amount, int source_price )
{
	char sa[10];
	itoa(source_amount, sa, 9);

	char sp[20];
	itoa(source_price, sp, 19);


	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[BUY_COMMAND_SUCCESS]),
				sa,
				sp,
				nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), mes_tokens, 3, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_buycmdincorrectprice_message( Banker* b, int sender_num )
{
	const char* incorrect_price_mes[] =
	{
					info_game_messages[BUY_COMMAND_INCORRECT_PRICE],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), incorrect_price_mes, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_buycmdincorrectamount_message( Banker* b, int sender_num )
{
	const char* incorrect_amount_mes[] =
	{
					info_game_messages[BUY_COMMAND_INCORRECT_AMOUNT],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), incorrect_amount_mes, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_sellcmdalreadysent_message( Banker* b, int sender_num )
{
	const char* message[] =
	{
					info_game_messages[SELL_COMMAND_ALREADY_SENT],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), message, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_sellcmdsuccess_message( Banker* b, int sender_num, int product_amount, int product_price )
{
	char pa[10];
	itoa(product_amount, pa, 9);

	char pp[20];
	itoa(product_price, pp, 19);


	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[SELL_COMMAND_SUCCESS]),
				pa,
				pp,
				nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), mes_tokens, 3, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_sellcmdincorrectprice_message( Banker* b, int sender_num )
{
	const char* incorrect_price_mes[] =
	{
					info_game_messages[SELL_COMMAND_INCORRECT_PRICE],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), incorrect_price_mes, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_sellcmdincorrectamount_message( Banker* b, int sender_num )
{
	const char* incorrect_amount_mes[] =
	{
					info_game_messages[SELL_COMMAND_INCORRECT_AMOUNT],
					nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), incorrect_amount_mes, 1, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int send_turn_command_message( Banker* b, int sender_num )
{
	int wait_yet_players_amount = b->GetAlivePlayers() - b->GetReadyPlayers();

	char w_y_p_a[10];
	itoa(wait_yet_players_amount, w_y_p_a, 9);


	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[TURN_COMMAND_SUCCESS]),
				w_y_p_a,
				nullptr
	};
	send_message(b->GetPlayerByNum(sender_num)->GetFd(), mes_tokens, 2, b->GetPlayerByNum(sender_num)->GetAddr());

	return 1;
}

static int help_command_handler( Banker* b, int sender_num, const CommandsExecutor::CommandParams& params )
{
	if ( !send_helpcmd_message(b, sender_num) )
	{
		return 0;
	}

	return 1;
}

static int market_command_handler( Banker* b, int sender_num, const CommandsExecutor::CommandParams& params )
{
	if ( !send_marketcmd_message(b, sender_num) )
	{
		return 0;
	}

	return 1;
}

static int player_command_handler( Banker* b, int sender_num, const CommandsExecutor::CommandParams& params )
{
	int target_num = *( reinterpret_cast<int*>(const_cast<void*>(params.GetParam1())) );
	if ( (target_num < 1) || (target_num > MAX_PLAYERS) )
	{
		send_playercmdnotfound_message(b, sender_num);
		return 1;
	}
	send_playercmd_message(b, sender_num, target_num);

	return 1;
}

static int list_command_handler( Banker* b, int sender_num, const CommandsExecutor::CommandParams& params )
{
	if ( !send_listcmd_message(b, sender_num) )
	{
		return 0;
	}

	return 1;
}

static int prod_command_handler( Banker* b, int sender_num, const CommandsExecutor::CommandParams& params )
{
	Player* p = b->GetPlayerByNum(sender_num);

	if ( p->GetSources() >= 1 )
	{
		if ( p->GetMoney() >= PRODUCTION_PRODUCT_COST )
		{
			if ( p->GetWaitFactories() > 0 )
			{
				p->SetWaitFactories( p->GetWaitFactories() - 1 );
				p->SetWorkFactories( p->GetWorkFactories() + 1 );
				p->SetSources( p->GetSources() - 1 );
				p->SetMoney( p->GetMoney() - PRODUCTION_PRODUCT_COST );
				p->SetProd();

				send_prodcmdsuccess_message(b, sender_num);
			}
			else
			{
				send_prodcmdnofacts_message(b, sender_num);
			}
		}
		else
		{
			send_prodcmdnomoney_message(b, sender_num);
		}
	}
	else
	{
		send_prodcmdnosource_message(b, sender_num);
	}

	return 1;
}

static int build_command_handler( Banker* b, int sender_num, const CommandsExecutor::CommandParams& params )
{
	char* arg1_p = reinterpret_cast<char*>( const_cast<void*>(params.GetParam1()) );
	if ( arg1_p != nullptr )
	{
		char arg[100];
		strncpy(arg, arg1_p, 99);

		if ( strcmp( arg, "list") != 0 )
		{
			send_buildcmdincorrectarg_message(b, sender_num);
		}
		else
		{
			send_buildfactslist_message(b, sender_num);
		}
	}
	else
	{
		Player* p = b->GetPlayerByNum(sender_num);
		if ( p->GetMoney() >= NEW_FACTORY_UNIT_COST/2 )
		{
			p->GetBuildsFactories().Insert( p->GetBuildsFactories().GetValidNum(), TURNS_TO_BUILD );
			p->SetMoney( p->GetMoney() - NEW_FACTORY_UNIT_COST/2 );
			p->SetBuiltFactories( p->GetBuiltFactories() + 1 );

			send_buildcmdsuccess_message(b, sender_num);
		}
		else
		{
			send_buildcmdnomoney_message(b, sender_num);
		}
	}

	return 1;
}

static int buy_command_handler( Banker* b, int sender_num, const CommandsExecutor::CommandParams& params )
{
	Player* p = b->GetPlayerByNum(sender_num);
	if ( p->IsSentSourceRequest() )
	{
		send_buycmdalreadysent_message(b, sender_num);
		return 1;
	}

	int source_amount = *( reinterpret_cast<int*>(const_cast<void*>(params.GetParam1())) );
	int source_price = *( reinterpret_cast<int*>(const_cast<void*>(params.GetParam2())) );
	int cur_min_source_price = b->GetCurrentMarketState().GetSourceMinPrice();

	if ( ( source_amount > 0 ) && ( source_amount <= b->GetCurrentMarketState().GetSourcesAmount() ) )
	{
		if ( source_price >= cur_min_source_price )
		{
			if ( p->GetMoney() < source_price * source_amount )
			{
				send_buycmdnomoney_message(b, sender_num);
				return 1;
			}

			b->GetSourcesRequests().Insert( sender_num, source_amount, source_price );
			p->SetSentSourceRequest();

			send_buycmdsuccess_message(b, sender_num, source_amount, source_price);
		}
		else
		{
			send_buycmdincorrectprice_message(b, sender_num);
		}
	}
	else
	{
		send_buycmdincorrectamount_message(b, sender_num);
	}

	return 1;
}

static int sell_command_handler( Banker* b, int sender_num, const CommandsExecutor::CommandParams& params )
{
	Player* p = b->GetPlayerByNum(sender_num);
	if ( p->IsSentProductsRequest() )
	{
		send_sellcmdalreadysent_message(b, sender_num);
		return 1;
	}

	int product_amount = *( reinterpret_cast<int*>(const_cast<void*>(params.GetParam1())) );
	int product_price = *( reinterpret_cast<int*>(const_cast<void*>(params.GetParam2())) );
	int cur_max_product_price = b->GetCurrentMarketState().GetProductMaxPrice();

	if ( ( product_amount > 0 ) && ( product_amount <= p->GetProducts() ) )
	{
		if ( ( product_price > 0 ) && ( product_price <= cur_max_product_price ) )
		{
			b->GetProductsRequests().Insert( sender_num, product_amount, product_price );
			p->SetSentProductsRequest();

			send_sellcmdsuccess_message(b, sender_num, product_amount, product_price);
		}
		else
		{
			send_sellcmdincorrectprice_message(b, sender_num);
		}
	}
	else
	{
		send_sellcmdincorrectamount_message(b, sender_num);
	}

	return 1;
}

static int turn_command_handler( Banker* b, int sender_num, const CommandsExecutor::CommandParams& params )
{
	b->GetPlayerByNum(sender_num)->SetTurn();
	b->SetReadyPlayers( b->GetReadyPlayers() + 1 );

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		Player* p = b->GetPlayer(i);
		if ( p != nullptr )
			if ( p->IsTurn() )
				send_turn_command_message(b, p->GetUID());
	}

	return 1;
}

static int quit_command_handler( Banker* b, int sender_num, const CommandsExecutor::CommandParams& params )
{
	return 1;
}

#endif
