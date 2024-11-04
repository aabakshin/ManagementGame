/*
 *	Модуль LexemList содержит контейнер для объектов
 *	представляющих лексемы и интерфейс для взаимодействия
 *	с контейнером.
 *	Этот модуль вызывается в след. модулях: LexemAnalyzer, SA_Additional, SyntaxAnalyzer
 */

#ifndef LEXEMLIST_HPP
#define LEXEMLIST_HPP

#include "Lexem.hpp"

/* Структура, представляющая элемент контейнера */
struct LexemList
{
	Lexem* data;
	LexemList* next;
	LexemList* prev;
};

/* Интерфейс для взаимодействия с контейнером */
void ll_insert(LexemList**, Lexem*);
const char* ll_delete(LexemList**, Lexem*);
LexemList* ll_find_lexem(LexemList*, const char*);
int ll_clear(LexemList**, int);
void ll_print(LexemList*);
int ll_get_size(LexemList*);

#endif
