/*
 *	Модуль botCore отвечает за инициализацию
 *	клиента(ИИ), установку связи с сервером
 *	и обработку искусственным игроком ответов
 *	от сервера.
 *	Этот модуль вызывается только в модуле bot_mg
 */

#ifndef BOT_CORE_HPP
#define BOT_CORE_HPP

#include "SystemHeaders.h"

extern "C"
{
	#include "MGLib.h"
}

#include "MainInfo.hpp"
#include "PolizItem.hpp"
#include "LexemAnalyzer.hpp"
#include "SyntaxAnalyzer.hpp"

/* Системные константы */
enum
{
	EXIT_CODE				=			-2,
	ADDRESS_BUFFER_SIZE		=		   100,
	SEND_BUFFER_SIZE		=		  1024,
	RECV_BUFFER_SIZE		=		  1024
};

/* Интерфейс модуля */
int bot_connect(const char* addr, const char* port);
int bot_check_server_response(char* buffer);

#endif
