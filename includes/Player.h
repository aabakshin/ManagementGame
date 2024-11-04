/*
 * Модуль Player содержит структуру данных игрока
 * Этот модуль вызывается в след. модулях: Banker, CommandsHandler, MarketRequest, serverCore
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "BuildList.h"

/* Системная константа */
enum
{
	IPV4_ADDRESS_SIZE = 50
};

struct Player
{
	int number;
	int fd;
	char ip[IPV4_ADDRESS_SIZE];
	
	int money;
	int old_money;
	int income;
	int sources;
	int products;
	int wait_factories;
	int work_factories;
	int build_factories;
	int produced_on_turn;
	
	int is_bot;
	int is_ident_message_recv;
	int is_turn;
	int is_prod;
	int sent_source_request;
	int sent_products_request;

	BuildList* build_list;
};
typedef struct Player Player;

#endif
