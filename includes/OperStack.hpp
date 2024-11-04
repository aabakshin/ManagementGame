/*
 *	Модуль OperStack предоставляет стековый контейнер
 *	для хранения символов арифметических операций
 *	Этот модуль вызывается только в модуле SA_Additional
 */

#ifndef OPERSTACK_HPP
#define OPERSTACK_HPP

/* Структура представляющая элемент контейнера */
struct OperStack
{
	char* data;
	int data_size;
	OperStack* next;
};

/* Интерфейс контейнера */
void op_push(OperStack** stack_ptr, const char* data, int data_size);
char* op_pop(OperStack** stack_ptr);
void op_print(OperStack* stack);
int op_clear(OperStack** stack_ptr);
int op_size(OperStack* stack);

#endif
