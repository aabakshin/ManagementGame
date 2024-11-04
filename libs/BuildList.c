/* Файл реализации модуля BuildList */

#ifndef BUILD_LIST_C
#define BUILD_LIST_C

#include "../includes/BuildList.h"
#include <stdlib.h>

/* Пуст контейнер */
int bl_is_empty(BuildList* node)
{
	return node == NULL;
}

/* Вставка нового элемента в конец контейнера */
int bl_insert(BuildList** nodePtr)
{
	if ( nodePtr == NULL )
		return 0;

	if ( *nodePtr == NULL )
	{
		BuildList* newPtr = NULL;

		newPtr = malloc(sizeof(struct BuildList));
		if ( newPtr == NULL )
			return 0;

		newPtr->build_number = 1;
		newPtr->turns_left = TURNS_TO_BUILD;
		newPtr->next = NULL;
		newPtr->prev = NULL;
		*nodePtr = newPtr;
	}
	else
	{
		int cur_num = (*nodePtr)->build_number;
		BuildList* prevPtr = NULL;
		BuildList* curPtr = *nodePtr;
		BuildList* newPtr = NULL;
		newPtr = malloc(sizeof(struct BuildList));
		if ( newPtr == NULL )
			return 0;
		newPtr->turns_left = TURNS_TO_BUILD;
		newPtr->next = NULL;

		while ( curPtr != NULL )
		{
			prevPtr = curPtr;
			curPtr = curPtr->next;
			cur_num++;
		}
		
		newPtr->prev = prevPtr;
		newPtr->build_number = cur_num;
		prevPtr->next = newPtr;
	}

	return 1;
}

/* 
 * Удаление последнего эл-та из контейнера 
 * При установке параметра clear при вызове функции bl_clear очищается список с первого эл-та.
 * Если нужно удалить конкретный эл-т этот параметр должен быть равен 0
 * 
 */
int bl_delete(BuildList** nodePtr, int clear)
{
	if ( nodePtr == NULL )
		return 0;

	if ( *nodePtr == NULL )
		return 0;
	
	BuildList* curPtr = *nodePtr;
	
	if ( !clear )
	{
		while ( (curPtr != NULL) && (curPtr->turns_left != 0) )
			curPtr = curPtr->next;
	
		if ( curPtr == NULL )
			return 0;
	}

	int deleteValue = curPtr->build_number;

	if ( curPtr->prev == NULL )
	{			
		if ( curPtr->next == NULL )
		{
			free(curPtr);
			*nodePtr = NULL;
			return deleteValue;
		}
		
		curPtr->next->prev = NULL;
		*nodePtr = curPtr->next;
		curPtr->next = NULL;
		free(curPtr);

		return deleteValue;
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
	
	return deleteValue;
}


/* Очищает полностью весь контейнер с содержимым */
int bl_clear(BuildList** nodePtr)
{
	if ( nodePtr == NULL )
		return 0;

	if ( *nodePtr == NULL )
		return 0;
	
	BuildList* node = *nodePtr;
	int size = bl_get_size(node);
	
	/*int result;*/
	int i;
	for ( i = 1; i <= size; i++ )
			bl_delete(nodePtr, 1);
		/*if ( (result = bl_delete(nodePtr, (*nodePtr)->build_number)) )
			printf("Deleted node #%d\n", result);
		else
			printf("Node #%d does not exist in the list!\n", (*nodePtr)->build_number);*/

	/*if ( bl_is_empty(*nodePtr) )
		printf("%s", "List has been cleared.\n");*/

	return 1;
}

/* Возвращает кол-во элементов в контейнере */
int bl_get_size(BuildList* node)
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
