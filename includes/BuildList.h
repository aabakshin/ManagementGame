/*
 *	Модуль BuildList представляет контейнер для
 *	хранения очереди заявок на строительство заводов для каждого игрока
 *	Этот модуль вызывается в только в модуле Player
 *
 */


#ifndef BUILD_LIST_H
#define BUILD_LIST_H

/* Кол-во ходов для постройки завода */
enum
{
	TURNS_TO_BUILD = 5
};

/* 
 * Автореферентная структура хранит порядковый номер заявки
 * и кол-во оставшихся ходов до появления нового завода
 */
struct BuildList
{
	int build_number; 
	int turns_left;

	struct BuildList* next;
	struct BuildList* prev;
};
typedef struct BuildList BuildList;

/* Интерфейс контейнера BuildList */
int bl_is_empty(BuildList* node);
int bl_insert(BuildList** nodePtr);
int bl_delete(BuildList** nodePtr, int clear);
int bl_clear(BuildList** nodePtr);
int bl_get_size(BuildList* node);
void bl_print(BuildList* node);

#endif
