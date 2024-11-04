/*
 *	Модуль LabelsList предоставляет контейнер для хранения "меток" - объектов 
 *	игрового скриптового языка, используемых в операторах безусловного перехода
 *	Этот модуль вызывается в след. модулях: SA_Additional, SyntaxAnalyzer
 */

#ifndef LABELSLIST_HPP
#define LABELSLIST_HPP

#include "PolizItem.hpp"

/* Структура, представляющая внутреннее состояние метки - её имя и указатель на команду */
struct LabelData
{
	char* name;
	int name_size;
	PolizItem* oper;
public:
	~LabelData();
	LabelData(char* str, int size, PolizItem* op);
private:
	LabelData() {}
};

/* Структура, представляющая контейнер для хранения объектов-меток */
struct LabelsList
{
	LabelData* data;
	LabelsList* next;
	LabelsList* prev;
};

/* Интерфейс для работы с контейнером */
void label_insert(LabelsList** list_ptr, LabelData* ld);
LabelData* label_delete(LabelsList** list_ptr, LabelData* ld);
LabelsList* label_find(LabelsList* list, const char* name);
void label_print(LabelsList* list);
int label_clear(LabelsList** list_ptr, int clear);
int label_get_size(LabelsList* list);

#endif
