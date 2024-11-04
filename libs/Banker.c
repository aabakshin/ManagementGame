/*
 *	Файл реализации для модуля Banker.
 *	Содержит описание функции инициализации структуры банкира, 
 *	а также глобальные определения
 *
 */

#ifndef BANKER_C
#define BANKER_C

#include "../includes/Banker.h"

/* Список действительных команд. Описан в модуле serverCore */
extern const char* valid_commands[];

/* Таблица множителей для формулы вычисления нового состояния рынка в соответствии с текущим уровнем */
double amount_multiplier_table[MARKET_LEVEL_NUMBER][2] = {
					{ 1.0, 3.0 },
					{ 1.5, 2.5 },
					{ 2.0, 2.0 },
					{ 2.5, 2.5 },
					{ 3.0, 1.0 }
};

/* Таблица цен для формулы вычисления нового состояния рынка в соответствии с текущим уровнем */
int price_table[MARKET_LEVEL_NUMBER][2] = { 
					{ 800, 6500 }, 
					{ 650, 6000 }, 
					{ 500, 5500 }, 
					{ 400, 5000 }, 
					{ 300, 4500 } 
};

/* Таблица вероятностных переходов для формулы вычисления шанса перехода на другой уровень рынка */
const int states_market_chance[MARKET_LEVEL_NUMBER][MARKET_LEVEL_NUMBER] = {
				{ 4, 4, 2, 1, 1 },
				{ 3, 4, 3, 1, 1 },
				{ 1, 3, 4, 3, 1 },
				{ 1, 1, 3, 4, 3 },
				{ 1, 1, 2, 4, 4 }
}; 

/* Параметры рынка */
MarketState cur_ms;

int banker_init(Banker* b)
{
	if ( b == NULL )
		return 0;

	b->lobby_players = 0;
	b->alive_players = 0;
	b->ready_players = 0;
	b->game_started = 0;
	b->products_requests = NULL;
	b->sources_requests = NULL;
	b->turn_number = 0;
	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
		b->pl_array[i] = NULL;

	cur_ms.max_product_price = 0;
	cur_ms.min_source_price = 0;
	cur_ms.product_amount = 0;
	cur_ms.source_amount = 0;
	
	b->cur_market_lvl = 0;
	b->cur_market_state = &cur_ms;
	b->valid_commands = valid_commands;

	return 1;
}

#endif
