/* Файл реализации модуля MainInfo */

#ifndef MAIN_INFO_CPP
#define MAIN_INFO_CPP

#ifndef __CPP_FILE__
#define __CPP_FILE__
#endif

#include "../includes/MainInfo.hpp"
#include <stdio.h>
#include <stdlib.h>

int mi_init(MainInfo* mi_ptr)
{
	if ( mi_ptr == NULL )
	{
		fprintf(stderr, "%s", "\n[MainInfo]: In function mi_init() \"mi_ptr\" is NULL\n");
		return 0;
	}

	(*mi_ptr).fd = -1;
	(*mi_ptr).pid = -1;
	(*mi_ptr).execute_script = 0;
	
	(*mi_ptr).my_id = -1;
	(*mi_ptr).turn = -1;
	(*mi_ptr).total_players = -1;
	(*mi_ptr).alive_players = -1;
	(*mi_ptr).cur_sources_buy = -1;
	(*mi_ptr).cur_sources_min_price = -1;
	(*mi_ptr).cur_products_sell = -1;
	(*mi_ptr).cur_products_max_price = -1;
	
	(*mi_ptr).last_player_num = -1;

	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
		(*mi_ptr).p_info[i] = NULL;

	return 1;
}

int mi_clear(MainInfo* mi_ptr)
{
	if ( mi_ptr == NULL )
	{
		fprintf(stderr, "%s", "\n[MainInfo]: In function mi_clear() mi_ptr is NULL\n");
		return 0;
	}

	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		PlayerInfo* pi = mi_ptr->p_info[i];
		if ( pi )
			delete pi;

		mi_ptr->p_info[i] = NULL;
	}

	return 1;
}

#endif
