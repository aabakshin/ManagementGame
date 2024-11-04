/*
 *	Модуль Banker содержит основные константы и структуры, 
 *	отвечающие за хранение информации об игровой сессии
 *	
 *	Этот модуль вызывается в следующих модулях:
 *	CommandsHandler, serverCore
 *
 */


#ifndef BANKER_H
#define BANKER_H

#include "MarketRequest.h"
#include "Player.h"
#include <stdlib.h>

/* Сервисные и игровые константы */
enum
{
	BUFSIZE							=		1024,
	ADDRESS_BUFFER					=		 100,
	MAX_PLAYERS						=		   8,
	MIN_PLAYERS_TO_START			=		   2,
	TIME_TO_START					=		  10,
	MARKET_LEVEL_NUMBER				=		   5,
	START_MARKET_LEVEL				=		   3
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

/* 
 * Инициализация основной серверной структуры 
 * Предполагается, что она будет создана на стеке(автоматически)
 */
int banker_init(Banker* b);

#endif
