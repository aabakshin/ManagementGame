/*
 * Модуль clientCore отвечает за правильное взаимодействие клиента и сервера, 
 * реализуя протокол общения и обработку данных, а также инициализацию сокета
 * Этот модуль используется только в модуле client   
 *
 * */

#ifndef CLIENTCORE_H
#define CLIENTCORE_H

#include "SystemHeaders.h"
#include "MGLib.h"
#include "CommandsHistoryList.h"

/*const char* server_codes[] = {
			"HELP_COMMAND",
			"MARKET_COMMAND",
			"PLAYER_COMMAND_NOT_FOUND",
			"PLAYER_COMMAND",
			"LIST_COMMAND",
			"PROD_COMMAND_SUCCESS",
			"PROD_COMMAND_NO_FACTORIES",
			"PROD_COMMAND_NO_MONEY",
			"PROD_COMMAND_NO_SOURCE",
			"BUILD_COMMAND_SUCCESS",
			"BUILD_COMMAND_NO_MONEY",
			"BUY_COMMAND_NO_MONEY",
			"BUY_COMMAND_ALREADY_SENT",
			"BUY_COMMAND_SUCCESS",
			"BUY_COMMAND_INCORRECT_PRICE",
			"BUY_COMMAND_INCORRECT_AMOUNT",
			"SELL_COMMAND_ALREADY_SENT",
			"SELL_COMMAND_SUCCESS",
			"SELL_COMMAND_INCORRECT_PRICE",
			"SELL_COMMAND_INCORRECT_AMOUNT",
			"TURN_COMMAND_SUCCESS",
			"VICTORY_MESSAGE",
			"LOST_ALIVE_PLAYER",
			"PRODUCED",
			"GAME_ALREADY_STARTED",
			"SERVER_FULL",
			"OUT_OF_MEMORY",
			"NEW_PLAYER_CONNECT",
			"START30SECONDS",
			"STARTCANCELLED",
			"GAME_STARTED",
			"STARTING_GAME_INFORMATION",
			"GAME_NOT_STARTED",
			"WAIT_FOR_NEXT_TURN",
			"UNKNOWN_COMMAND",
			"LOST_LOBBY_PLAYER",
			"SUCCESS_CHARGES_PAY",
			"PLAYER_BANKROT",
			"PAY_FACTORY_SUCCESS",
			"FACTORY_BUILT",
			"NEW_TURN",
			"AUCTION_RESULTS",
			NULL
};*/

/* Системные константы */
enum
{
	EXIT_CODE						=								-2,
	TIMEOUT							=							100000,
	ADDRESS_BUFFER_SIZE				=							   100,
	RECEIVE_BUFFER_SIZE				=							  1024,
	SEND_BUFFER_SIZE				=							  1024,
	HISTORY_COMMANDS_LIST_SIZE		=								 5
};

/* Интерфейс модуля */
int client_init(const char* addr, const char* port);
int check_server_response(char* buffer);
int get_string(char* buffer, int buffer_size);

#endif
