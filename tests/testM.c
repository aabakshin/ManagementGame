/* Тестовый модуль для проверки контейнера MarketRequest */

#include <stdio.h>
#include "../includes/MarketRequest.h"

void heap_make(int* values, int size)
{
	int i;
	for ( i = size-1; i > 0; i-- )
	{
		int idx = i;
		while ( idx != 0 )
		{
			int parent = (idx - 1) / 2;
			
			if ( values[idx] <= values[parent] )
				break;
			
			int temp = values[idx];
			values[idx] = values[parent];
			values[parent] = temp;
				
			idx = parent;
		}
	}
}
void heap_sort(int* values, int size)
{
	heap_make(values, size);
	
	int i;
	for ( i = size-1; i > 0; i-- )
	{
		int temp = values[i];
		values[i] = values[0];
		values[0] = temp;
		
		heap_make(values, i);
	}
}

int main(int argc, char** argv)
{
	/*printf("MarketRequest item size = %lu\n", sizeof(struct MarketRequest));*/
	MarketRequest* sources_request = NULL;
	/*MarketRequest* products_request = NULL;*/

	Player players[5];
	players[0].number = 1;
	players[0].fd = 4;
	players[0].build_factories = 0;
	players[0].build_list = NULL;
	players[0].wait_factories = 2;
	players[0].work_factories = 1;
	players[0].money = 7900;
	players[0].sources = 3;
	players[0].products = 1;
	/**/
	players[1].number = 2;
	players[1].fd = 5;
	players[1].build_factories = 0;
	players[1].build_list = NULL;
	players[1].wait_factories = 1;
	players[1].work_factories = 3;
	players[1].money = 2000;
	players[1].sources = 1;
	players[1].products = 2;
	/**/
	players[2].number = 3;
	players[2].fd = 6;
	players[2].build_factories = 0;
	players[2].build_list = NULL;
	players[2].wait_factories = 0;
	players[2].work_factories = 2;
	players[2].money = 900;
	players[2].sources = 0;
	players[2].products = 5;
	/**/
	players[3].number = 4;
	players[3].fd = 7;
	players[3].build_factories = 0;
	players[3].build_list = NULL;
	players[3].wait_factories = 1;
	players[3].work_factories = 0;
	players[3].money = 6500;
	players[3].sources = 0;
	players[3].products = 0;
	/**/
	players[4].number = 5;
	players[4].fd = 8;
	players[4].build_factories = 0;
	players[4].build_list = NULL;
	players[4].wait_factories = 4;
	players[4].work_factories = 4;
	players[4].money = 15000;
	players[4].sources = 1;
	players[4].products = 3;
	

	MarketData m_data[5];
	m_data[0].p = &players[3];
	m_data[0].amount = 3;
	m_data[0].price = 300;
	
	m_data[1].p = &players[0];
	m_data[1].amount = 0;
	m_data[1].price = 0;

	m_data[2].p = &players[1];
	m_data[2].amount = 0;
	m_data[2].price = 0;

	m_data[3].p = &players[2];
	m_data[3].amount = 4;
	m_data[3].price = 200;

	m_data[4].p = &players[4];
	m_data[4].amount = 5;
	m_data[4].price = 100;
	

	mr_insert(&sources_request, &m_data[4]);
	mr_insert(&sources_request, &m_data[1]);
	mr_insert(&sources_request, &m_data[2]);
	mr_insert(&sources_request, &m_data[3]);
	mr_insert(&sources_request, &m_data[0]);
	

	MarketRequest* arr[5];
	int arr_int[5];
	int arr_indexes[5];

	int i;
	for (i = 0; i < 5; i++ )
	{
		arr[i] = NULL;
		arr_indexes[i] = 0;
	}

	MarketRequest* listPtr = sources_request;
	i = 0;
	while (listPtr != NULL)
	{
		/*printf("\nRequest of player #%d:\n"
			   "Amount of sources requested: %d\n"
			   "Player wants buy for %dP\n\n", listPtr->market_data.p->number, listPtr->market_data.amount, listPtr->market_data.price);*/
		
		arr[i] = listPtr; 
		i++;
		if (listPtr->next == NULL)
			break;

		listPtr = listPtr->next;
	}
	/*printf("%s", "\n\n\n");*/
	for ( i = 0; i < 5; i++ )
	{
		/*printf("#%d price = %d\n", arr[i]->market_data.p->number, arr[i]->market_data.price);*/
		arr_int[i] = arr[i]->market_data.price;
	}

	heap_sort(arr_int, 5);
	
	/*printf("%s", "\n\n\n");*/
	/*for ( i = 0; i < 5; i++ )
	{
		printf("#%d price = %d\n", i+1, arr_int[i]);
	}*/
	int j = 0;
	i = 0;
	/*int count = 0;*/
	MarketRequest* new_sources_request = NULL;
	while ( i < 5 )
	{
		/*count++;
		printf("[%d]\narr[i] = %d\narr_int[j] = %d\narr_indexes[i] = %d\n>>>>>>>>>\n", count, arr[i]->market_data.price, arr_int[j], arr_indexes[i]);*/
		if ( (arr[i]->market_data.price == arr_int[j]) && !arr_indexes[i] )
		{
			mr_insert(&new_sources_request, &arr[i]->market_data);
			arr_indexes[i] = 1;
			i = 0;
			j++;
			if ( j == 5 )
				break;
			continue;
		}
		i++;
	}
	
	
	
	printf("\n\n%s\n", "-----------------------------------------------------------");
	listPtr = new_sources_request;
	while (listPtr != NULL)
	{
		printf("\nRequest of player #%d:\n"
			   "Amount of sources requested: %d\n"
			   "Player wants buy for %dP\n\n", listPtr->market_data.p->number, listPtr->market_data.amount, listPtr->market_data.price);
		
		if (listPtr->next == NULL)
			break;

		listPtr = listPtr->next;
	}
	

	/*
	listPtr = sources_request;
	while ( listPtr != NULL )
	{
		if (listPtr->market_data.p == &players[3] )
		{
			unsigned int number = listPtr->market_data.p->number;
			if ( mr_delete(&sources_request, listPtr->market_data.p) )
			{
				printf("Request of player #%d has proceeded.\n", number);
				listPtr = sources_request;
				break;
			}
		}
		listPtr = listPtr->next;
	}
	*/

	/*printf("size = %d\n", mr_get_size(sources_request));*/
	
	/*printf("%s", "Clearing list of requests for \"source\".\n");*/
	mr_clear(&new_sources_request);
	new_sources_request = NULL;
	mr_clear(&sources_request);
	sources_request = NULL;

	/*printf("size = %d\n", mr_get_size(sources_request));*/

	return 0;
}
