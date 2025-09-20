/* Файл реализации модуля OperStack */

#ifndef OPERSTACK_CPP
#define OPERSTACK_CPP

#ifndef __CPP_FILE__
#define __CPP_FILE__
#endif

#include "../includes/OperStack.hpp"
#include <cstdlib>
#include <cstdio>

void op_push(OperStack** stack_ptr, const char* data, int data_size)
{
	if ( (stack_ptr == NULL) || (data == NULL) || (data_size < 1) )
	{
		fprintf(stderr, "%s", "\n[OperStack]: ch_push() stack_ptr is NULL\n");
		return;
	}
	
	OperStack* newPtr = NULL;
	newPtr = new OperStack;
	if ( !newPtr )
	{
		fprintf(stderr, "%s", "\n[OperStack]: ch_push() newPtr memory error\n");
		return;
	}
	newPtr->data = new char[data_size];
	if ( !newPtr->data )
	{
		fprintf(stderr, "%s", "\n[OperStack]: ch_push() newPtr->data memory error\n");
		return;
	}
	int i;
	for ( i = 0; i < data_size-1; i++ )
		newPtr->data[i] = data[i];
	newPtr->data[i] = '\0';
	newPtr->data_size = data_size;
	newPtr->next = NULL;

	if ( *stack_ptr == NULL )
	{
		*stack_ptr = newPtr;
		return;
	}
	
	newPtr->next = *stack_ptr;
	*stack_ptr = newPtr;
}

char* op_pop(OperStack** stack_ptr)
{
	if ( (stack_ptr == NULL) || (*stack_ptr == NULL) )
		return NULL;
	
	OperStack* tempPtr = NULL;
	char* popValue = (*stack_ptr)->data;
	tempPtr = *stack_ptr;
	*stack_ptr = (*stack_ptr)->next;
	delete tempPtr;

	return popValue;
}

void op_print(OperStack* stack)
{
	if ( stack == NULL )
		return;
	
	putchar('\n');
	while ( stack != NULL )
	{
		printf("%s --> ", stack->data);
		stack = stack->next;
	}
	printf("%s", "NULL\n");
}

int op_clear(OperStack** stack_ptr)
{
	if ( (stack_ptr == NULL) || (*stack_ptr == NULL) )
		return 0;

	int size = op_size(*stack_ptr);
	int del_count = 0;

	for ( int i = 1; i <= size; i++ )
	{
		char* popValue = op_pop(stack_ptr);
		if ( popValue != NULL )
		{
			del_count++;
			delete[] popValue;
		}
		else
			break;
	}

	if ( del_count == size )
		return 1;

	return 0;
}

int op_size(OperStack* stack)
{
	if ( stack == NULL )
		return 0;

	int size = 0;

	while ( stack != NULL )
	{
		size++;
		stack = stack->next;
	}

	return size;
}

#endif
