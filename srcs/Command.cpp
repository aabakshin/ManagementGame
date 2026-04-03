#ifndef COMMAND_CPP_SENTINEL
#define COMMAND_CPP_SENTINEL


#include "Command.hpp"
#include "BrokerMessages.hpp"
#include "MGLib.h"
#include "MessageTokens.hpp"
#include <cstring>
#include <cstdlib>


// Описаны в модуле MGLib
extern const char* info_game_messages[];


static const char* const true_str				=		"true";
static const char* const false_str				=		"false";
static const char* const list_param_value		=		"list";


static const char* const valid_commands[] = {
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


Command::Command( int tokens_count )
{
	msg_tokens.MakeMessageTokens( tokens_count );
}

void Command::SetName( const char* cmd_name )
{
	int i;
	for ( i = 0; ( cmd_name[i] != '\0' ) && ( i < COMMAND_NAME_SIZE-1 ); ++i )
		name[i] = cmd_name[i];
	name[i] = '\0';
}

Command::CommandParams::CommandParams()
{
	SetParam1( nullptr );
	SetParam2( nullptr );
}

void Command::SetCmdParams( const void* value1, const void* value2 )
{
	cmd_params.SetParam1( value1 );
	cmd_params.SetParam2( value2 );
}


HelpCommand::HelpCommand( int tokens_count ) : Command( tokens_count )
{
	SetName(valid_commands[HELP_COMMAND_NUM]);
}

void HelpCommand::PrepareAndProc( int session_id, int sender_player_id, int cmd_tokens_amount, const char* param1, const char* param2, const BCBrokerMessages& BCbroker )
{
	Process( session_id, sender_player_id, GetCmdParams(), BCbroker );
}

void HelpCommand::Process( int session_id, int sender_player_id, const Command::CommandParams& params, const BCBrokerMessages& BCbroker )
{
	strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[HELP_COMMAND_SUCCESS], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
	for ( int i = 1, j = 0; i < HELP_CMD_TOKENS_NUM; ++j, ++i )
		strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[i]), valid_commands[j], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
	const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( HELP_CMD_TOKENS_NUM );
}


MarketCommand::MarketCommand( int tokens_count ) : Command( tokens_count )
{
	SetName(valid_commands[MARKET_COMMAND_NUM]);
}

void MarketCommand::PrepareAndProc( int session_id, int sender_player_id, int cmd_tokens_amount, const char* param1, const char* param2, const BCBrokerMessages& BCbroker )
{
	Process( session_id, sender_player_id, GetCmdParams(), BCbroker );
}

void MarketCommand::Process( int session_id, int sender_player_id, const Command::CommandParams& params, const BCBrokerMessages& BCbroker )
{
	const int tokens[] { BCbroker.MARKET_SOURCES_AMOUNT_TOKEN, BCbroker.MARKET_SOURCE_MIN_PRICE_TOKEN, BCbroker.MARKET_PRODUCTS_AMOUNT_TOKEN, BCbroker.MARKET_PRODUCT_MAX_PRICE_TOKEN };

	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SESSION_ID_PARAM_TOKEN]), 9);
	const_cast<BCBrokerMessages&>(BCbroker).PutMessage( GetMessageTokens().GetValue(), BCbroker.SESSION_ID_PARAM_TOKEN+1 );

	strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[MARKET_COMMAND_SUCCESS], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
	for ( int i = 1, j = 0; i < MARKET_CMD_TOKENS_NUM; ++j, ++i )
		strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[i]), const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( tokens[j] ), MessageTokens::MESSAGE_TOKEN_SIZE-2 );
	const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( MARKET_CMD_TOKENS_NUM );
}


PlayerCommand::PlayerCommand( int tokens_count ) : Command( tokens_count )
{
	SetName(valid_commands[PLAYER_COMMAND_NUM]);
}

void PlayerCommand::PrepareAndProc( int session_id, int sender_player_id, int cmd_tokens_amount, const char* param1, const char* param2, const BCBrokerMessages& BCbroker )
{
	if ( cmd_tokens_amount >= 2 )
	{
		char player_id_buf[100];
		strcpy(player_id_buf, param1);
		cut_str(player_id_buf, 100, '\n');

		int player_number = atoi(player_id_buf);
		SetCmdParams(reinterpret_cast<void*>(&player_number), nullptr);
	}

	Process( session_id, sender_player_id, GetCmdParams(), BCbroker );
}

void PlayerCommand::Process( int session_id, int sender_player_id, const Command::CommandParams& params, const BCBrokerMessages& BCbroker )
{
	int target_player_id = *( reinterpret_cast<int*>(const_cast<void*>(params.GetParam1())) );

	if ( (target_player_id < 1) || (target_player_id > MAX_PLAYERS) )
	{
		strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[PLAYER_COMMAND_INCORRECT_ID], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}

	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SESSION_ID_PARAM_TOKEN]), 9);
	itoa(sender_player_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SENDER_PLAYER_ID_PARAM_TOKEN]), 9);
	itoa(target_player_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.TARGET_PLAYER_ID_PARAM_TOKEN]), 9);
	const_cast<BCBrokerMessages&>(BCbroker).PutMessage( GetMessageTokens().GetValue(), BCbroker.TARGET_PLAYER_ID_PARAM_TOKEN+1 );

	if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.TARGET_PLAYER_NOT_FOUND_TOKEN ), true_str) == 0 )
	{
		strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[PLAYER_COMMAND_NOT_FOUND], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}

	const int tokens[] {
								BCbroker.TARGET_PLAYER_UID_TOKEN,
								BCbroker.TARGET_PLAYER_MONEY_TOKEN,
								BCbroker.TARGET_PLAYER_INCOME_TOKEN,
								BCbroker.TARGET_PLAYER_SOURCES_TOKEN,
								BCbroker.TARGET_PLAYER_PRODUCTS_TOKEN,
								BCbroker.TARGET_PLAYER_WAIT_FACTORIES_TOKEN,
								BCbroker.TARGET_PLAYER_WORK_FACTORIES_TOKEN,
								BCbroker.TARGET_PLAYER_BUILT_FACTORIES_TOKEN,
								BCbroker.TARGET_PLAYER_PRODUCED_TOKEN
	};

	strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[PLAYER_COMMAND_SUCCESS], MessageTokens::MESSAGE_TOKEN_SIZE-2 );

	int i = 1;
	for ( int j = 0; ( i < PLAYER_CMD_TOKENS_NUM ) && ( tokens[j] != BCbroker.TARGET_PLAYER_PRODUCED_TOKEN ); ++j, ++i )
		strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[i]), const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( tokens[j] ), MessageTokens::MESSAGE_TOKEN_SIZE-2 );

	if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.SENDER_PLAYER_IS_BOT_TOKEN ), true_str) == 0 )
	{
		strncpy( const_cast<char*>( const_cast<MessageTokens&>(GetMessageTokens())[i]),const_cast<BCBrokerMessages&>(BCbroker).TakeMessage(BCbroker.TARGET_PLAYER_PRODUCED_TOKEN),MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( PLAYER_CMD_TOKENS_NUM );
		return;
	}

	const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( PLAYER_CMD_TOKENS_NUM-1 );
}


ListCommand::ListCommand( int tokens_count ) : Command( tokens_count )
{
	SetName(valid_commands[LIST_COMMAND_NUM]);
}

void ListCommand::PrepareAndProc( int session_id, int sender_player_id, int cmd_tokens_amount, const char* param1, const char* param2, const BCBrokerMessages& BCbroker )
{
	Process( session_id, sender_player_id, GetCmdParams(), BCbroker );
}

void ListCommand::Process( int session_id, int sender_player_id, const Command::CommandParams& params, const BCBrokerMessages& BCbroker )
{
	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SESSION_ID_PARAM_TOKEN]), 9);
	const_cast<BCBrokerMessages&>(BCbroker).PutMessage( GetMessageTokens().GetValue(), BCbroker.SESSION_ID_PARAM_TOKEN+1 );

	strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[LIST_COMMAND_SUCCESS], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
	strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[1]), const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.ALIVE_PLAYERS_TOKEN ), MessageTokens::MESSAGE_TOKEN_SIZE-2 );
	const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( LIST_CMD_TOKENS_NUM );
}


ProdCommand::ProdCommand( int tokens_count ) : Command( tokens_count )
{
	SetName(valid_commands[PROD_COMMAND_NUM]);
}

void ProdCommand::PrepareAndProc( int session_id, int sender_player_id, int cmd_tokens_amount, const char* param1, const char* param2, const BCBrokerMessages& BCbroker )
{
	Process( session_id, sender_player_id, GetCmdParams(), BCbroker );
}

void ProdCommand::Process( int session_id, int sender_player_id, const Command::CommandParams& params, const BCBrokerMessages& BCbroker )
{
	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SESSION_ID_PARAM_TOKEN]), 9);
	itoa(sender_player_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SENDER_PLAYER_ID_PARAM_TOKEN]), 9);
	const_cast<BCBrokerMessages&>(BCbroker).PutMessage( GetMessageTokens().GetValue(), BCbroker.SENDER_PLAYER_ID_PARAM_TOKEN+1 );


	if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.PLAYER_IS_TURN_TOKEN ), true_str) == 0 )
	{
		strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[WAIT_FOR_NEXT_TURN], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}


	if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.PROD_CMD_SOURCES_CONDITION_SUCCESS_TOKEN ), true_str) == 0 )
	{
		if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.PROD_CMD_MONEY_CONDITION_SUCCESS_TOKEN ), true_str) == 0 )
		{
			if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.PROD_CMD_WAIT_FACTORIES_CONDITION_SUCCESS_TOKEN ), true_str) == 0 )
			{
				const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.PROD_CMD_UPDATE_GAME_STATE_TOKEN );
				strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[PROD_COMMAND_SUCCESS], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
				const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( PROD_CMD_TOKENS_NUM );
				return;
			}
			strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[PROD_COMMAND_NO_FACTORIES], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
			const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
			return;
		}
		strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[PROD_COMMAND_NO_MONEY], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}
	strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[PROD_COMMAND_NO_SOURCE], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
	const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
}


BuildCommand::BuildCommand( int tokens_count ) : Command( tokens_count )
{
	SetName(valid_commands[BUILD_COMMAND_NUM]);
}

void BuildCommand::PrepareAndProc( int session_id, int sender_player_id, int cmd_tokens_amount, const char* param1, const char* param2, const BCBrokerMessages& BCbroker )
{
	if ( cmd_tokens_amount >= 2 )
	{
		char build_count_param[100];
		strcpy(build_count_param, param1);
		cut_str(build_count_param, 100, '\n');

		int builds_count = atoi(build_count_param);
		SetCmdParams(reinterpret_cast<void*>(&builds_count), nullptr);
	}

	Process( session_id, sender_player_id, GetCmdParams(), BCbroker );
}

void BuildCommand::Process( int session_id, int sender_player_id, const Command::CommandParams& params, const BCBrokerMessages& BCbroker )
{
	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SESSION_ID_PARAM_TOKEN]), 9);
	itoa(sender_player_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SENDER_PLAYER_ID_PARAM_TOKEN]), 9);
	const_cast<BCBrokerMessages&>(BCbroker).PutMessage( GetMessageTokens().GetValue(), BCbroker.SENDER_PLAYER_ID_PARAM_TOKEN+1 );


	if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.PLAYER_IS_TURN_TOKEN ), true_str) == 0 )
	{
		strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[WAIT_FOR_NEXT_TURN], MessageTokens::MESSAGE_TOKEN_SIZE-2);
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}


	char* arg1_p = reinterpret_cast<char*>( const_cast<void*>(params.GetParam1()) );
	if ( arg1_p != nullptr )
	{
		char arg[100];
		strncpy(arg, arg1_p, 99);

		if ( strcmp( arg, list_param_value) != 0 )
		{
			strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[BUILD_COMMAND_INCORRECT_ARG], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
			const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
			return;
		}

		if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.BUILD_CMD_PLAYER_BUILDS_LIST_IS_EMPTY_TOKEN ), false_str) == 0 )
		{
			strncpy(const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[BUILDING_FACTORIES_LIST], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
			strncpy(const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[1]), const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.BUILD_CMD_PLAYER_GET_BUILDS_LIST_SIZE_TOKEN ), MessageTokens::MESSAGE_TOKEN_SIZE-2 );
			strncpy(const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[2]), const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.BUILD_CMD_PLAYER_GET_BUILDS_LIST_TOKEN ), MessageTokens::MESSAGE_TOKEN_SIZE-2 );
			const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( BUILD_CMD_TOKENS_NUM );
			return;
		}

		strncpy(const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[BUILDING_FACTORIES_LIST_EMPTY], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}

	if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.BUILD_CMD_MONEY_CONDITION_SUCCESS_TOKEN ), true_str) == 0 )
	{
		const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.BUILD_CMD_UPDATE_GAME_STATE_TOKEN );
		strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[BUILD_COMMAND_SUCCESS], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}
	strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[BUILD_COMMAND_NO_MONEY], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
	const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
}


BuyCommand::BuyCommand( int tokens_count ) : Command( tokens_count )
{
	SetName(valid_commands[BUY_COMMAND_NUM]);
}

void BuyCommand::PrepareAndProc( int session_id, int sender_player_id, int cmd_tokens_amount, const char* param1, const char* param2, const BCBrokerMessages& BCbroker )
{
	if ( cmd_tokens_amount < 3 )
	{
		return;
		// throw IncorrectCmdArgsException();
	}

	char buy_count_param[100];
	strcpy(buy_count_param, param1);
	cut_str(buy_count_param, 100, '\n');
	int sources_amount = atoi(buy_count_param);

	char buy_price_param[100];
	strcpy(buy_price_param, param2);
	cut_str(buy_price_param, 100, '\n');
	int sources_price = atoi(buy_price_param);

	SetCmdParams(reinterpret_cast<void*>(&sources_amount), reinterpret_cast<void*>(&sources_price));

	Process( session_id, sender_player_id, GetCmdParams(), BCbroker );
}

void BuyCommand::Process( int session_id, int sender_player_id, const Command::CommandParams& params, const BCBrokerMessages& BCbroker )
{
	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SESSION_ID_PARAM_TOKEN]), 9);
	itoa(sender_player_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SENDER_PLAYER_ID_PARAM_TOKEN]), 9);
	const_cast<BCBrokerMessages&>(BCbroker).PutMessage( GetMessageTokens().GetValue(), BCbroker.SENDER_PLAYER_ID_PARAM_TOKEN+1 );


	if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.PLAYER_IS_TURN_TOKEN ), true_str) == 0 )
	{
		strncpy(const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[WAIT_FOR_NEXT_TURN], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}


	if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.BUY_CMD_IS_SENT_SOURCE_REQUEST ), true_str) == 0 )
	{
		strncpy(const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[BUY_COMMAND_ALREADY_SENT], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}


	int source_amount = *( reinterpret_cast<int*>(const_cast<void*>(params.GetParam1())) );
	int source_price = *( reinterpret_cast<int*>(const_cast<void*>(params.GetParam2())) );

	itoa(source_amount, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SOURCES_AMOUNT_PARAM_TOKEN]), 9);
	itoa(source_price, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SOURCE_PRICE_PARAM_TOKEN]), 19);
	const_cast<BCBrokerMessages&>(BCbroker).PutMessage( GetMessageTokens().GetValue(), BCbroker.SOURCE_PRICE_PARAM_TOKEN+1 );


	if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.BUY_CMD_SOURCES_CONDITION_SUCCESS_TOKEN ), true_str) == 0 )
	{
		if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.BUY_CMD_PRICE_CONDITION_SUCCESS_TOKEN ), true_str) == 0 )
		{
			if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.BUY_CMD_MONEY_CONDITION_SUCCESS_TOKEN ), true_str) == 0 )
			{
				strncpy(const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[BUY_COMMAND_NO_MONEY], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
				const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
				return;
			}

			const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.BUY_CMD_UPDATE_GAME_STATE_TOKEN );

			strncpy(const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[BUY_COMMAND_SUCCESS], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
			itoa(source_amount, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[1]), 9);
			itoa(source_price, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[2]), 19);
			const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( BUY_CMD_TOKENS_NUM );
			return;
		}

		strncpy(const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[BUY_COMMAND_INCORRECT_PRICE], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}

	strncpy(const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[BUY_COMMAND_INCORRECT_AMOUNT], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
	const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
}


SellCommand::SellCommand( int tokens_count ) : Command( tokens_count )
{
	SetName(valid_commands[SELL_COMMAND_NUM]);
}

void SellCommand::PrepareAndProc( int session_id, int sender_player_id, int cmd_tokens_amount, const char* param1, const char* param2, const BCBrokerMessages& BCbroker )
{
	if ( cmd_tokens_amount < 3 )
	{
		return;
		// throw IncorrectCmdArgsException();
	}

	char prods_amount_param[100];
	strcpy(prods_amount_param, param1);
	cut_str(prods_amount_param, 100, '\n');
	int products_amount = atoi(prods_amount_param);

	char prods_price_param[100];
	strcpy(prods_price_param, param2);
	cut_str(prods_price_param, 100, '\n');
	int products_price = atoi(prods_price_param);

	SetCmdParams(reinterpret_cast<void*>(&products_amount), reinterpret_cast<void*>(&products_price));

	Process( session_id, sender_player_id, GetCmdParams(), BCbroker );
}

void SellCommand::Process( int session_id, int sender_player_id, const Command::CommandParams& params, const BCBrokerMessages& BCbroker )
{
	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SESSION_ID_PARAM_TOKEN]), 9);
	itoa(sender_player_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SENDER_PLAYER_ID_PARAM_TOKEN]), 9);
	const_cast<BCBrokerMessages&>(BCbroker).PutMessage( GetMessageTokens().GetValue(), BCbroker.SENDER_PLAYER_ID_PARAM_TOKEN+1 );


	if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.PLAYER_IS_TURN_TOKEN ), true_str) == 0 )
	{
		strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[WAIT_FOR_NEXT_TURN], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}


	if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.SELL_CMD_IS_SENT_PRODUCT_REQUEST ), true_str) == 0 )
	{
		strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[SELL_COMMAND_ALREADY_SENT], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}


	int product_amount = *( reinterpret_cast<int*>(const_cast<void*>(params.GetParam1())) );
	int product_price = *( reinterpret_cast<int*>(const_cast<void*>(params.GetParam2())) );

	itoa(product_amount, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.PRODUCTS_AMOUNT_PARAM_TOKEN]), 9);
	itoa(product_price, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.PRODUCT_PRICE_PARAM_TOKEN]), 19);
	const_cast<BCBrokerMessages&>(BCbroker).PutMessage( GetMessageTokens().GetValue(), BCbroker.PRODUCT_PRICE_PARAM_TOKEN+1 );


	if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.SELL_CMD_AMOUNT_CONDITION_SUCCESS_TOKEN ), true_str) == 0 )
	{
		if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.SELL_CMD_PRICE_CONDITION_SUCCESS_TOKEN ), true_str) == 0 )
		{
			const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.SELL_CMD_UPDATE_GAME_STATE_TOKEN );

			strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[SELL_COMMAND_SUCCESS], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
			itoa(product_amount, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[1]), 9);
			itoa(product_price, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[2]), 19);
			const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( SELL_CMD_TOKENS_NUM );
			return;
		}

		strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[SELL_COMMAND_INCORRECT_PRICE], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}

	strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[SELL_COMMAND_INCORRECT_AMOUNT], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
	const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
}


TurnCommand::TurnCommand( int tokens_count ) : Command( tokens_count )
{
	SetName(valid_commands[TURN_COMMAND_NUM]);
}

void TurnCommand::PrepareAndProc( int session_id, int sender_player_id, int cmd_tokens_amount, const char* param1, const char* param2, const BCBrokerMessages& BCbroker )
{
	Process( session_id, sender_player_id, GetCmdParams(), BCbroker );
}

void TurnCommand::Process( int session_id, int sender_player_id, const Command::CommandParams& params, const BCBrokerMessages& BCbroker )
{
	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SESSION_ID_PARAM_TOKEN]), 9);
	itoa(sender_player_id, const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[BCbroker.SENDER_PLAYER_ID_PARAM_TOKEN]), 9);
	const_cast<BCBrokerMessages&>(BCbroker).PutMessage( GetMessageTokens().GetValue(), BCbroker.SENDER_PLAYER_ID_PARAM_TOKEN+1 );


	if ( strcmp(const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.PLAYER_IS_TURN_TOKEN ), true_str) == 0 )
	{
		strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[WAIT_FOR_NEXT_TURN], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
		const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( 1 );
		return;
	}

	const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.TURN_CMD_UPDATE_GAME_STATE_TOKEN );

	strncpy(const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[TURN_COMMAND_SUCCESS], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
	strncpy( const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[1]), const_cast<BCBrokerMessages&>(BCbroker).TakeMessage( BCbroker.TURN_CMD_GET_WYPA_TOKEN ), MessageTokens::MESSAGE_TOKEN_SIZE-2 );
	const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( TURN_CMD_TOKENS_NUM );
}


QuitCommand::QuitCommand( int tokens_count ) : Command( tokens_count )
{
	SetName(valid_commands[QUIT_COMMAND_NUM]);
}

void QuitCommand::PrepareAndProc( int session_id, int sender_player_id, int cmd_tokens_amount, const char* param1, const char* param2, const BCBrokerMessages& BCbroker )
{
	Process( session_id, sender_player_id, GetCmdParams(), BCbroker );
}

void QuitCommand::Process( int session_id, int sender_player_id, const Command::CommandParams& params, const BCBrokerMessages& BCbroker )
{
	strncpy(const_cast<char*>(const_cast<MessageTokens&>(GetMessageTokens())[0]), info_game_messages[QUIT_COMMAND_SUCCESS], MessageTokens::MESSAGE_TOKEN_SIZE-2 );
	const_cast<MessageTokens&>(GetMessageTokens()).SetMsgTokensCount( QUIT_CMD_TOKENS_NUM );
}

#endif
