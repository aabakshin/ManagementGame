/* Файл реализации для модуля LexemList */

#ifndef LEXEMLIST_CPP
#define LEXEMLIST_CPP

#ifndef __CPP_FILE__
#define __CPP_FILE__
#endif

#include "../includes/LexemList.hpp"
#include "../includes/Lexem.hpp"
#include <cstdlib>
#include <cstdio>
#include <cstring>

void ll_insert(LexemList** lex_list_ptr, Lexem* lex)
{
	if ( (lex_list_ptr == NULL) || (lex == NULL) )
	{
		return;
	}
	
	LexemList* newPtr = NULL;
	newPtr = new LexemList;
	if ( newPtr == NULL )
	{
		return;
	}
	newPtr->data = lex;
	newPtr->next = NULL;
	newPtr->prev = NULL;

	LexemList* prevPtr = NULL;
	LexemList* curPtr = *lex_list_ptr;

	while ( curPtr != NULL )
	{
		prevPtr = curPtr;
		curPtr = curPtr->next;
	}

	if ( prevPtr == NULL )
	{
		*lex_list_ptr = newPtr;
	}
	else
	{
		prevPtr->next = newPtr;
		newPtr->prev = prevPtr;
	}

}

const char* ll_delete(LexemList** lex_list_ptr, Lexem* lex)
{
	if ( (lex_list_ptr == NULL) || (*lex_list_ptr == NULL) || (lex == NULL) )
	{
		return NULL;
	}

	LexemList* tempPtr = NULL;
	LexemList* prevPtr = NULL;
	LexemList* curPtr = *lex_list_ptr;
	
	while ( (curPtr != NULL) && (curPtr->data != lex) )
	{
		prevPtr = curPtr;
		curPtr = curPtr->next;
	}

	if ( curPtr == NULL )
	{
		return NULL;
	}
	
	const char* delValue = curPtr->data->lexem;
	tempPtr = curPtr;
	if ( prevPtr == NULL )
	{
		*lex_list_ptr = curPtr->next;
		if ( *lex_list_ptr != NULL )
			(*lex_list_ptr)->prev = NULL;
		delete tempPtr;

		return delValue;
	}

	if ( curPtr->next == NULL )
	{
		prevPtr->next = NULL;
		delete tempPtr;

		return delValue;
	}

	prevPtr->next = curPtr->next;
	curPtr->next->prev = prevPtr;
	delete tempPtr;

	return delValue;
}

LexemList* ll_find_lexem(LexemList* lex_list, const char* str_lex)
{
	if ( (lex_list == NULL) || (str_lex == NULL) || (str_lex[0] == '\0') )
	{
		return NULL;
	}
	

	while ( (lex_list != NULL) && (strcmp(str_lex, lex_list->data->lexem) != 0) )
	{
		lex_list = lex_list->next;
	}
	
	return lex_list;
}

int ll_get_size(LexemList* lex_list)
{
	if ( lex_list == NULL )
	{
		return 0;
	}

	int size = 0;
	while ( lex_list != NULL )
	{
		size++;
		lex_list = lex_list->next;
	}

	return size;
}

int ll_clear(LexemList** lex_list_ptr, int clear)
{
	if ( (lex_list_ptr == NULL) || (*lex_list_ptr == NULL) )
	{
		return 0;
	}

	int size = ll_get_size(*lex_list_ptr);
	for ( int i = 1; i <= size; i++ )
	{
		Lexem* lexem = (*lex_list_ptr)->data;	
		ll_delete(lex_list_ptr, lexem);
		
		if ( clear )
			if ( lexem )
				delete lexem;

		/*if ( value != NULL )
			printf("Deleted \"%s\" value\n", value);*/
	}
	
	size = ll_get_size(*lex_list_ptr);
	if ( size < 1 )
		return 1;

	return 0;
}	

void ll_print(LexemList* lex_list)
{
	if ( lex_list == NULL )
	{
		return;
	}

	int size = ll_get_size(lex_list);
	for ( int i = 1; i <= size; i++ )
	{
		const char* value = lex_list->data->lexem;
		/*for ( int j = 0; value[j]; j++ )
			printf("%c ", value[j]);
		putchar('\n');*/
		printf("[%3d]: lexem:\"%32s\", lexem_size:%4d, ", i, (value == NULL) ? "NULL" : value, lex_list->data->size);
		switch ( lex_list->data->lexem_type )
		{
			case LEXEM_TYPE_INTEGER:
				printf("%12s%15s", "lexem_type: ", "INTEGER, ");
				break;
			case LEXEM_TYPE_STRING:
				printf("%12s%15s", "lexem_type: ", "STRING, ");
				break;
			case LEXEM_TYPE_KEYWORD:
				printf("%12s%15s", "lexem_type: ", "KEYWORD, ");
				break;
			case LEXEM_TYPE_IDENTIFIER:
				printf("%12s%15s", "lexem_type: ", "IDENTIFIER, ");
				break;
			case LEXEM_TYPE_ASSIGN:
				printf("%12s%15s", "lexem_type: ", "ASSIGN, ");
				break;
			case LEXEM_TYPE_DELIMITER:
				printf("%12s%15s", "lexem_type: ", "DELIMITER, ");
		};
		printf("%15s%3d\n", "num_str = ", lex_list->data->num_str);

		lex_list = lex_list->next;
	}
}

#endif
