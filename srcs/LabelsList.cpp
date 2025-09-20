/* Файл реализации модуля LabelsList */

#ifndef LABELSLIST_CPP
#define LABELSLIST_CPP

#ifndef __CPP_FILE__
#define __CPP_FILE__
#endif

#include "../includes/LabelsList.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>


LabelData::LabelData(char* str, int size, PolizItem* op)
{
	if ( (str == NULL) || (size < 1) || (op == NULL) )
	{
		fprintf(stderr, "%s", "\n[LabelData]: LabelData construction error\n");
		return;
	}

	this->name = new char[size];
	if ( !this->name )
	{
		fprintf(stderr, "%s", "\n[LabelData]: LabelData memory error\n");
		return;
	}
	this->name_size = size;

	int i;
	for ( i = 0; i < size-1; i++ )
		this->name[i] = str[i];
	this->name[i] = '\0';
	
	this->oper = op;
}

LabelData::~LabelData()
{
	if ( this->name )
		delete[] this->name;
	this->name = NULL;
}


void label_insert(LabelsList** list_ptr, LabelData* ld)
{
	if ( (list_ptr == NULL) || (ld == NULL) )
	{
		fprintf(stderr, "%s", "\n[LabelsList]: In function label_insert: list_ptr OR ld is NULL\n");
		return;
	}

	LabelsList* newPtr = NULL;
	newPtr = new LabelsList;
	if ( !newPtr )
	{
		fprintf(stderr, "%s", "\n[LabelsList]: In function label_insert: memory error\n");
		return;
	}
	newPtr->data = ld;
	newPtr->next = NULL;
	newPtr->prev = NULL;


	LabelsList* prevPtr = NULL;
	LabelsList* curPtr = *list_ptr;

	if ( *list_ptr == NULL )
	{
		*list_ptr = newPtr;
		return;
	}
	
	while ( curPtr != NULL )
	{
		prevPtr = curPtr;
		curPtr = curPtr->next;
	}

	prevPtr->next = newPtr;
	newPtr->prev = prevPtr;
}

LabelData* label_delete(LabelsList** list_ptr, LabelData* ld)
{
	if ( (list_ptr == NULL) || (*list_ptr == NULL) || (ld == NULL) )
	{
		fprintf(stderr, "%s", "\n[LabelsList]: In function label_delete: list_ptr, *list_ptr OR ld is NULL\n");
		return NULL;
	}

	LabelsList* tempPtr = NULL;
	LabelsList* prevPtr = NULL;
	LabelsList* curPtr = *list_ptr;
	
	while ( (curPtr != NULL) && ( curPtr->data != ld) )
	{
		prevPtr = curPtr;
		curPtr = curPtr->next;
	}
	
	if ( curPtr == NULL )
	{
		return NULL;
	}
	
	LabelData* retValue = curPtr->data;
	tempPtr = curPtr;

	if ( curPtr == *list_ptr )
	{
		*list_ptr = (*list_ptr)->next;
		if ( *list_ptr != NULL )
			(*list_ptr)->prev = NULL;
		
		delete tempPtr;
		return retValue;
	}

	if ( curPtr->next == NULL )
	{
		prevPtr->next = NULL;
		delete tempPtr;
		return retValue;
	}

	prevPtr->next = curPtr->next;
	curPtr->next->prev = prevPtr;

	delete tempPtr;
	return retValue;
}

LabelsList* label_find(LabelsList* list, const char* name)
{
	if ( (list == NULL) || (list->data == NULL) || (name == NULL) || (*name == '\0') )
	{
		return NULL;
	}
	
	while ( (list != NULL) && (list->data != NULL) && ( strcmp(list->data->name, name) != 0 ) )
	{
		list = list->next;
	}

	if ( (list == NULL) || (list->data == NULL) )
	{
		return NULL;
	}

	return list;
}

void label_print(LabelsList* list)
{
	if ( list == NULL )
	{
		return;
	}
	
	putchar('\n');
	while ( list != NULL )
	{
		printf("%s --> ", list->data->name);
		list = list->next;
	}
	printf("%s", "NULL\n");
}

int label_clear(LabelsList** list_ptr, int clear)
{
	if ( list_ptr == NULL )
	{
		fprintf(stderr, "%s", "\n[LabelsList]: In function label_clear: list_ptr is NULL\n");
		return 0;
	}

	if ( *list_ptr == NULL )
		return 1;

	int size = label_get_size(*list_ptr);

	int i;
	for ( i = 1; i <= size; i++ )
	{
		LabelData* del_value = label_delete(list_ptr, (*list_ptr)->data);
		if ( del_value && clear )
			delete del_value;
	}

	if ( label_get_size(*list_ptr) == 0 )
		return 1;

	return 0;
}

int label_get_size(LabelsList* list)
{
	if ( list == NULL )
	{
		return 0;
	}

	int size = 0;
	while ( list != NULL )
	{
		size++;
		list = list->next;
	}
	
	return size;
}

#endif
