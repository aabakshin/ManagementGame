/* 
 *	Модуль LexemAnalyzer предоставляет объект, переводящий
 *	текстовое представление скрипта игрока в игровые лексемы
 *	Этот модуль вызывается в след. модулях: botCore
 * */

#ifndef LEXEM_ANALYZER_HPP
#define LEXEM_ANALYZER_HPP

#include <stdio.h>
#include <stdlib.h>
#include "LexemList.hpp"

/* Системная константа */
enum
{
	BUFSIZE = 1024
};

/* Константы представляющие состояния автомата для перевода текста в лексемы */
enum lex_states
{
	START,
	NUMBER,
	IDENTIFIER,
	KEYWORD,
	ASSIGN,
	STRING,
	DELIMITERS
};

class LexemAnalyzer
{
	lex_states cur_state;
	char* buffer;
	int size;
public:
	LexemAnalyzer();
	LexemList* Run(FILE* fd);
	~LexemAnalyzer();
};



#endif
