/* Модуль содержит вспомогательные функции для модуля SyntaxAnalyzer */

#ifndef SA_ADDITIONAL_HPP
#define SA_ADDITIONAL_HPP

#include "LexemList.hpp"
#include "PolizElem.hpp"
#include "PolizItem.hpp"

/* Константы для индексации списка игровых операторов */
enum
{
	PRINT_OPERATOR,
	BUY_OPERATOR,
	SELL_OPERATOR,
	PROD_OPERATOR,
	BUILD_OPERATOR,
	ENDTURN_OPERATOR
};

/* Константы для индексации игровых функций */
enum
{
	MY_ID,
	TURN,
	PLAYERS,
	ACTIVE_PLAYERS,
	SUPPLY,
	RAW_PRICE,
	DEMAND,
	PROD_PRICE,
	MONEY,
	RAW,
	PRODUCTION,
	FACTORIES,
	MANUFACTURED,
	RES_RAW_SOLD,
	RES_RAW_PRICE,
	RES_PROD_BOUGHT,
	RES_PROD_PRICE
};

/* является ли входная строка именем действующей игровой функции */
int isValidGameFunction(const char* str);

/* является ли входная строка именем действующего игрового оператора */
int isValidGameOperator(const char* str);

/* является ли список лексем list арифм./логич. операцией */
int isALOperation(LexemList* list);

/* получить приоритет арифм./логич. операции */
int getOperPrior(LexemList* list, const char* operation, int oper_size);

/* является ли список лексем list корректным lvalue выражением */
int isCorrectVarLValueStatement(LexemList* list);

/* является ли список лексем list корректным rvalue выражением */
int isCorrectVarRValueStatement(LexemList* list);

/* является ли список лексем list корректным арифм./логич. выражением. Если да, то в result_list записывается список лексем в постфиксной форме */
int isCorrectArithmeticStatement(LexemList* list, LexemList** result_list);

/* является ли список лексем list корректным игровым выражением. Если да, то в expr_list записывается список игровых лексем */
int isCorrectGameStatement(LexemList* list, LexemList** expr_list);

/* преобразовывает список лексем, явл. арифм./логич. выражением, в ПОЛИЗ список исполняемых инструкций */
int ProcessArithmeticLexem(PolizItem** ret_list_ptr, LexemList* arithmetic_statement);

/* преобразовывает список лексем, явл. игровым выражением, в ПОЛИЗ список исполняемых инструкций (если игр. функция имеет параметр ) */
int ProcessGameLexemArgument(PolizItem** ret_list, LexemList* game_statement, const char* game_function_name);

/* преобразовывает список лексем, явл. игровым выражением, в ПОЛИЗ список исполняемых инструкций */
int ProcessGameLexem(PolizItem** ret_list, LexemList* game_statement);

/* преобразовывает rvalue лексему в ПОЛИЗ объект */
PolizElem* TransformRVal(LexemList* rvalue);

/* преобразовывает арифм./логич. выражение в список объектов PolizElem */
PolizItem* TransformArithmetic(LexemList* arith_statement);

/* преобразовывает игровое выражение в список объектов PolizElem */
PolizItem* TransformGameStatement(LexemList* game_statement);

/* требует ли преобразованная игровая функция обработку параметров со стека или нет */
int isSimpleGF(PolizItem* item);

#endif
