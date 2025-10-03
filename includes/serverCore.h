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


int server_init( char* );
int server_run( Banker*, int );
int send_message( int, const char**, int, const char* );
int player_left_game( Banker*, Player* );


#endif
