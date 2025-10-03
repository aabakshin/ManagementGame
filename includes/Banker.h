/*
 *	Модуль Banker содержит основные константы и структуры, отвечающие за хранение информации об игровой сессии.
 *	Этот модуль вызывается в следующих модулях: CommandsHandler, serverCore
 */

#ifndef BANKER_H
#define BANKER_H

#include "MarketRequest.h"
#include "Player.h"
#include <stdlib.h>


enum
{
	BUFSIZE							=		1024,
	ADDRESS_BUFFER					=		 100
};

enum
{
	MAX_PLAYERS						=		   8,
	MIN_PLAYERS_TO_START			=		   2,
	TIME_TO_START					=		  10,
	MARKET_LEVEL_NUMBER				=		   5,
	START_MARKET_LEVEL				=		   3
};

enum
{
	START_MONEY						=						100000,
	START_SOURCES					=							 4,
	START_PRODUCTS					=							 2,
	START_FACTORIES					=					         2,
	SOURCE_UNIT_CHARGE				=						   300,
	PRODUCT_UNIT_CHARGE				=						   500,
	FACTORY_UNIT_CHARGE				=						  1000,
	NEW_FACTORY_UNIT_COST			=						  5000,
	PRODUCTION_PRODUCT_COST			=						  2000
};


struct CommandHandlerParams
{
	void* param1;
	void* param2;
};
typedef struct CommandHandlerParams CommandHandlerParams;

/* Константы для нумерации индексов валидных команд(порядок должен совпадать с порядком следования элементов в valid_commands) */
enum
{
	ERROR_COMMAND_NUM		=		-1,
	HELP_COMMAND_NUM,
	MARKET_COMMAND_NUM,
	PLAYER_COMMAND_NUM,
	LIST_COMMAND_NUM,
	PROD_COMMAND_NUM,
	BUILD_COMMAND_NUM,
	BUY_COMMAND_NUM,
	SELL_COMMAND_NUM,
	TURN_COMMAND_NUM,
	QUIT_COMMAND_NUM,
	UNKNOWN_COMMAND_NUM
};


/* Информация для отчёта по аукционам */
struct AuctionReport
{
	int turn;
	Player* p;
	int sold_sources;
	int sold_price;
	int bought_prods;
	int bought_price;
};
typedef struct AuctionReport AuctionReport;

/* Текущее состояние рынка */
struct MarketState
{
	int source_amount;
	int min_source_price;
	int product_amount;
	int max_product_price;
};
typedef struct MarketState MarketState;

/* Содержит основную информацию об игровом состоянии */
struct Banker
{
	int turn_number;
	int game_started;
	int alive_players;
	int ready_players;
	int lobby_players;
	int cur_market_lvl;
	Player* pl_array[MAX_PLAYERS];
	MarketState* cur_market_state;
	MarketRequest* sources_requests;
	MarketRequest* products_requests;
	const char** valid_commands;
};
typedef struct Banker Banker;



int banker_init(Banker* b);
int last_man_stand(Banker* b);


#endif
