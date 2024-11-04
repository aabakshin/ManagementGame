/* 
 * Модуль MainInfo содержит внутреннее состояние искусственного игрока,
 * некоторую общую информацию, а также функции инициализации и очистки структуры
 * Этот модуль вызывается в след. модулях: botCore, PolizElem
 * */

#ifndef MAIN_INFO_HPP
#define MAIN_INFO_HPP

/* Системная константа */
enum
{
	MAX_PLAYERS = 8
};

/* Структура внутреннего представления искусственного игрока */
struct PlayerInfo
{
	int player_num;
	int money;
	int income;
	int raw;
	int prod;
	int wait_fact;
	int work_fact;
	int build_fact;
	int manufactured;
	int res_raw_sold;
	int res_raw_price;
	int res_prod_bought;
	int res_prod_price;
};

/* Структура, содержащая общую информацию об искусственном клиенте */
struct MainInfo
{
	int pid;
	int fd;
	int execute_script;

	int my_id;
	int turn;
	int total_players;
	int alive_players;
	int cur_sources_buy;
	int cur_sources_min_price;
	int cur_products_sell;
	int cur_products_max_price;
	
	int last_player_num;
	PlayerInfo* p_info[MAX_PLAYERS];
};

/* Интерфейсы инициализации и очистки данных */
int mi_init(MainInfo* mi_ptr);
int mi_clear(MainInfo* mi_ptr);

#endif
