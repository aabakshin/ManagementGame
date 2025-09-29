/*
 *	Модуль MGLib содержит вспомогательные функции общего назначения для реализации различных частей программы
 *	Этот модуль вызывается в след. модулях: botCore, clientCore, CommandsHandler, serverCore, PolizElem
 */

#ifndef MGLIB_H
#define MGLIB_H


enum
{
					AUCTION_RESULTS,
					SUCCESS_CHARGES_PAY,
					PLAYER_BANKROT,
					LOST_ALIVE_PLAYER,
					WAIT_FOR_NEXT_TURN,
					PRODUCED,
					STARTINSECONDS,
					GAME_STARTED,
					STARTING_GAME_INFORMATION,
					STARTCANCELLED,
					PAY_FACTORY_SUCCESS,
					FACTORY_BUILT,
					UNKNOWN_COMMAND,
					VICTORY_MESSAGE,
					GAME_ALREADY_STARTED,
					SERVER_FULL,
					NEW_PLAYER_CONNECT,
					GAME_NOT_STARTED,
					LOST_LOBBY_PLAYER,
					NEW_TURN,
					HELP_COMMAND,
					MARKET_COMMAND,
					PLAYER_COMMAND_NOT_FOUND,
					PLAYER_COMMAND,
					LIST_COMMAND,
					PROD_COMMAND_SUCCESS,
					PROD_COMMAND_NO_FACTORIES,
					PROD_COMMAND_NO_MONEY,
					PROD_COMMAND_NO_SOURCE,
					BUILDING_FACTORIES_LIST,
					BUILDING_FACTORIES_LIST_EMPTY,
					BUILD_COMMAND_SUCCESS,
					BUILD_COMMAND_NO_MONEY,
					BUY_COMMAND_ALREADY_SENT,
					BUY_COMMAND_NO_MONEY,
					BUY_COMMAND_SUCCESS,
					BUY_COMMAND_INCORRECT_PRICE,
					BUY_COMMAND_INCORRECT_AMOUNT,
					SELL_COMMAND_ALREADY_SENT,
					SELL_COMMAND_SUCCESS,
					SELL_COMMAND_INCORRECT_PRICE,
					SELL_COMMAND_INCORRECT_AMOUNT,
					TURN_COMMAND_SUCCESS
};

enum
{
					COMMAND_INCORRECT_ARGUMENTS_NUM,
					COMMAND_INTERNAL_ERROR
};


/* Пирамидальная сортировка */
void heap_sort(int* values, int size, int ascending);

/* Удаляет лишние пробелы из строки */
void delete_spaces(char* buffer, int* bufsize);

/* Превращает число в строку */
void itoa(int number, char* num_buf, int max_buf_len);

/* Читает строку из станд. потока ввода. Является аналогом fgets */
int readline(int fd, char* buf, int bufsize);

#endif
