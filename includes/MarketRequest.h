/*
 *	Модуль MarketRequest представляет контейнер для хранения запросов
 *	клиентов для участие в аукционах. А также предоставляет интерфейс
 *	для взаимодействия с контейнером.
 *	Этот модуль используется в след. модулях: Banker, serverCore
 */


#ifndef MARKET_REQUEST_H
#define MARKET_REQUEST_H

#include "Player.h"

/* Содержимое заявки игрока на аукцион */
struct MarketData
{
	Player* p;
	int amount;
	int price;
	int success;
};
typedef struct MarketData MarketData;

/* Структура, представляющая эл-т контейнера */
struct MarketRequest
{
	MarketData market_data;
	struct MarketRequest* next;
	struct MarketRequest* prev;
};
typedef struct MarketRequest MarketRequest;

/* Интерфейс для взаимодействия с контейнером */
int mr_is_empty(MarketRequest* node);
int mr_insert(MarketRequest** nodePtr, MarketData* data);
int mr_delete(MarketRequest** nodePtr, Player* p);
int mr_clear(MarketRequest** nodePtr);
int mr_get_size(MarketRequest* node);

#endif
