/*
 *	Модуль serverCore содержит основные функции для организации работы сервера.
 *	Отвечает за подготовку данных к отправке, проверку игровых событий и реагирование на них.
 *	Передаёт команды и параметры на обработку в модуль CommandsHandler.
 *	Этот модуль вызывается только в модуле server.
 */

#ifndef SERVER_CORE_H
#define SERVER_CORE_H

#include <signal.h>
#include "SystemHeaders.h"
#include "Banker.h"

/* Структура, содержащая данные для передачи в ф-ю обработки команд */
struct ProcessCommandParams
{
	Banker* banker;
	Player* p;
	fd_set* readfds;
	char** command_tokens;
	int tokens_amount;
};
typedef struct ProcessCommandParams ProcessCommandParams;

int server_init( char* port );
int server_run( Banker* banker, int ls );
//int send_data(int fd, const char* send_buf, int mes_len, const char* ip);
int send_message(int fd, const char** message_tokens, int tokens_amount, const char* ip);
int player_left_game(Banker*, Player*, int, fd_set*);
int server_end_work(Banker* banker, Player* p, int i, fd_set* readfds);

#endif
