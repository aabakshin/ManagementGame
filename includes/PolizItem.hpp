/* Модуль PolizItem предоставляет контейнер для объектов PolizElem */

#ifndef POLIZ_ITEM_HPP
#define POLIZ_ITEM_HPP

typedef class PolizElem PolizElem;

/* Структура, представляет элемент контейнера */
struct PolizItem
{
	int elem_number;
	PolizElem* p;
	PolizItem* next;
	PolizItem* prev;
};

/* Интерфейс контейнера */
void pi_insert(PolizItem** list_ptr, PolizElem* elem);
PolizElem* pi_delete(PolizItem** list_ptr, PolizElem* elem);
PolizItem* pi_find(PolizItem* list, PolizElem* elem);
int pi_clear(PolizItem** list_ptr, int clear);
void pi_print(PolizItem* list);
int pi_size(PolizItem* list);

#endif
