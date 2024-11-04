/* 
 *	Модуль SyntaxAnalyzer представляет синтаксический анализатор
 *	игрового скрипта, прошедшего через этап лексического анализа.
 *	Объект-анализатор содержит пять основных скриптовых операторов:
 *	метка, присваивание, безусловный, условный и игровой.
 *	Объект хранятся в ПОЛИЗ нотации. Весь список интерпретируемых команд
 *	хранится по указателю operators_list.
 * */

#ifndef SYNTAX_ANALYZER_HPP
#define SYNTAX_ANALYZER_HPP

#include "Lexem.hpp"
#include "LexemList.hpp"
#include "PolizItem.hpp"
#include "PolizElem.hpp"
#include "LabelsList.hpp"
#include <cstdlib>

class SyntaxAnalyzer
{
	PolizItem* operators_list;
public:
	SyntaxAnalyzer() : operators_list(NULL) {}
	~SyntaxAnalyzer() {}
	PolizItem* Run(LexemList**);
private:
	PolizItem* isCorrectLabelOperator(LexemList*, LexemList**);
	PolizItem* isCorrectAssignOperator(LexemList*, LexemList**);
	PolizItem* isCorrectGoOperator(LexemList*, LexemList**);
	PolizItem* isCorrectIfOperator(LexemList*, LexemList**);
	PolizItem* isCorrectGameOperator(LexemList*, LexemList**);
};


#endif
