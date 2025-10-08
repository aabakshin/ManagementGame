/*
 *	Модуль Banker содержит основные константы и структуры, отвечающие за хранение информации об игровой сессии.
 *	Этот модуль вызывается в следующих модулях: CommandsHandler, serverCore
 */

#ifndef BANKER_H
#define BANKER_H


#include "MarketRequest.h"
#include "Player.h"
#include "MGLib.h"
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
};
typedef struct Banker Banker;



int banker_init(Banker* b);

int get_player_idx_by_num(Banker* b, int player_number);

// расчёт и оплата игровых издержек каждым игроком
int pay_charges(Banker* banker, fd_set* readfds, AuctionReport* ar, MarketRequest** new_source_request_ptr, MarketRequest** new_prod_request_ptr);

// формирование отчёта о прошедших событиях на текущем ходе
int report_on_turn(Banker* banker, AuctionReport* ar, MarketRequest* new_source_request, MarketRequest* new_prod_request);

// вычисление нового значения состояния рынка
int change_market_state(Banker* banker);

// запуск аукционов, auction_type принимает два значения: 0(аукцион сырья), 1(аукцион продукции)
MarketRequest* start_auction(Banker* banker, AuctionReport* ar, int auction_type);

// проверка строящихся заводов у игроков
int check_building_factories(Banker* banker, fd_set* readfds);

// очистка сведений о покинувшем игру игроке
int clean_player_record(Banker* banker, Player* p);


#endif
