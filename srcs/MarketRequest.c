/* Файл реализации модуля MarketRequest */

#ifndef MARKET_REQUEST_C
#define MARKET_REQUEST_C

#include "../includes/MarketRequest.h"
#include <stdlib.h>

int mr_is_empty(MarketRequest* node)
{
	return node == NULL;
}

int mr_insert(MarketRequest** nodePtr, MarketData* data)
{
	if ( nodePtr == NULL )
		return 0;

	if ( *nodePtr == NULL )
	{
		MarketRequest* newPtr = NULL;

		newPtr = malloc(sizeof(struct MarketRequest));
		if ( newPtr == NULL )
			return 0;

		newPtr->market_data.amount = data->amount;
		newPtr->market_data.price = data->price;
		newPtr->market_data.p = data->p;
		newPtr->market_data.success = data->success;
		newPtr->next = NULL;
		newPtr->prev = NULL;
		*nodePtr = newPtr;
	}
	else
	{
		MarketRequest* prevPtr = NULL;
		MarketRequest* curPtr = *nodePtr;
		MarketRequest* newPtr = NULL;
		newPtr = malloc(sizeof(struct MarketRequest));
		if ( newPtr == NULL )
			return 0;
		
		newPtr->market_data.amount = data->amount;
		newPtr->market_data.price = data->price;
		newPtr->market_data.p = data->p;
		newPtr->market_data.success = data->success;

		newPtr->next = NULL;
		while ( curPtr != NULL )
		{
			prevPtr = curPtr;
			curPtr = curPtr->next;
		}
		
		newPtr->prev = prevPtr;
		prevPtr->next = newPtr;
	}

	return 1;
}

int mr_delete(MarketRequest** nodePtr, Player* p)
{
	if ( nodePtr == NULL )
		return 0;

	if ( *nodePtr == NULL )
		return 0;
	
	MarketRequest* curPtr = *nodePtr;
	
	while ( (curPtr != NULL) && (curPtr->market_data.p != NULL) && (curPtr->market_data.p != p)  )
	{
		curPtr = curPtr->next;
	}
	
	if ( curPtr == NULL )
		return 0;
	

	if ( curPtr->prev == NULL )
	{			
		if ( curPtr->next == NULL )
		{
			free(curPtr);
			*nodePtr = NULL;
			return 1;
		}
		
		curPtr->next->prev = NULL;
		*nodePtr = curPtr->next;
		curPtr->next = NULL;
		free(curPtr);

		return 1;
	}
	
	if ( curPtr->next == NULL )
	{
		curPtr->prev->next = NULL;
		curPtr->prev = NULL;
		free(curPtr);
	}
	else
	{
		curPtr->next->prev = curPtr->prev;
		curPtr->prev->next = curPtr->next;
		curPtr->next = NULL;
		curPtr->prev = NULL;
		free(curPtr);
	}
	
	return 1;
}

int mr_clear(MarketRequest** nodePtr)
{
	if ( nodePtr == NULL )
		return 0;

	if ( *nodePtr == NULL )
		return 0;
	
	MarketRequest* node = *nodePtr;
	int size = mr_get_size(node);
	
	int i;
	for ( i = 1; i <= size; i++ )
	{
		mr_delete(nodePtr, (*nodePtr)->market_data.p);
	}
	return 1;
}

int mr_get_size(MarketRequest* node)
{
	if ( node == NULL )
		return 0;

	int size = 1;
	while ( node != NULL )
	{
		if ( node->next == NULL)
			break;
		node = node->next;
		size++;
	}

	return size;
}

#endif
