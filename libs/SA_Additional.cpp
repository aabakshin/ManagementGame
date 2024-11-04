/* Файл реализации модуля SA_Additional */

#ifndef SA_ADDITIONAL_CPP
#define SA_ADDITIONAL_CPP

#ifndef __CPP_FILE__
#define __CPP_FILE__
#endif

#include "../includes/SA_Additional.hpp"
#include "../includes/OperStack.hpp"
#include "../includes/LabelsList.hpp"
#include <cstdlib>
#include <cstring>
#include <cstdio>

/* Стековый контейнер для хранения символов арифметических и логических операций */
static OperStack* opstack = NULL;

/* Глобальный список определённых в игровом скрипте переменных */
PolizItem* vars_list = NULL;

/* Глобальный список определённых в игровом скрипте меток */
LabelsList* labels_list = NULL;

/* Список действующих игровых операторов */
const char* game_operators[] = 
{
			"print",
			"buy",
			"sell",
			"prod",
			"build",
			"endturn",
			NULL
};

/* Список действующих игровых функций */
const char* game_functions[] = 
{
			"?my_id",
			"?turn",
			"?players",
			"?active_players",
			"?supply",
			"?raw_price",
			"?demand",
			"?production_price",
			"?money",
			"?raw",
			"?production",
			"?factories",
			"?manufactured",
			"?result_raw_sold",
			"?result_raw_price",
			"?result_prod_bought",
			"?result_prod_price",
			NULL
};


int isValidGameFunction(const char* str)
{
	if ( str == NULL )
		return -1;

	for  ( int i = 0; game_functions[i] != NULL; i++ )
		if ( strcmp(game_functions[i], str) == 0 )
			return i;

	return -1;
}

int isValidGameOperator(const char* str)
{
	if ( str == NULL )
		return -1;

	for  ( int i = 0; game_operators[i] != NULL; i++ )
		if ( strcmp(game_operators[i], str) == 0 )
			return i;

	return -1;
}

int isALOperation(LexemList* list)
{
	if ( (list == NULL) || (list->data == NULL) )
		return 0;

	const char* operations[] =
	{
				"!",
				"*",
				"/",
				"%",
				"+",
				"-",
				"<",
				">",
				"=",
				"!=",
				"&",
				"|",
				NULL
	};

	for ( int i = 0; operations[i] != NULL; i++ )
		if ( strcmp(list->data->lexem, operations[i]) == 0 )
			return 1;

	return 0;
}

int getOperPrior(LexemList* list, const char* operation, int oper_size)
{	
	if ( (list == NULL) && (operation == NULL) && (oper_size < 1) )
		return -1;

	if ( (list != NULL) && (operation == NULL) && (oper_size < 1) )
	{
		if ( 
				(list->data->lexem[0] == '-') && 
				( (list->next != NULL) && (list->next->data != NULL) && (list->next->data->lexem_type == LEXEM_TYPE_INTEGER) ) && 
				( (list->prev != NULL) && (list->prev->data != NULL) && (list->prev->data->lexem_type != LEXEM_TYPE_INTEGER) )
		   )
		{
			delete[] list->data->lexem;
			list->data->lexem = new char[list->data->size+1];
			if ( !list->data->lexem )
			{
				fprintf(stderr, "%s", "\n[SyntaxAnalyzer]: In function getOperPrior memory error\n");
				return -2;
			}
			list->data->size++;
			const char* un = "-U";
			int i;
			for ( i = 0; un[i]; i++ )
				list->data->lexem[i] = un[i];
			list->data->lexem[i] = '\0';
		}
		
		if ( (strcmp(list->data->lexem, "-U") == 0) || (list->data->lexem[0] == '!') ) 
		{
			return 6;
		}

		if ( (list->data->lexem[0] == '*') || (list->data->lexem[0] == '/') || (list->data->lexem[0] == '%') )
		{
			return 5;
		}

		if ( (list->data->lexem[0] == '+') || (list->data->lexem[0] == '-') )
		{
			return 4;
		}

		if ( (list->data->lexem[0] == '<') || (list->data->lexem[0] == '>') )
		{
			return 3;
		}

		if ( (list->data->lexem[0] == '=') || (strcmp(list->data->lexem, "!=") == 0 ) )
		{
			return 2;
		}
		
		if ( (list->data->lexem[0] == '&') || (list->data->lexem[0] == '|') )
		{
			return 1;
		}
	}

	if ( (list == NULL) && (operation != NULL) && (oper_size > 0) )
	{		
		if ( (operation[0] == '&') || (operation[0] == '|') )
		{
			return 1;
		}
		
		if ( (operation[0] == '=') || (strcmp(operation, "!=") == 0 ) )
		{
			return 2;
		}

		if ( (operation[0] == '<') || (operation[0] == '>') )
		{
			return 3;
		}

		if ( (operation[0] == '+') || (operation[0] == '-') )
		{
			return 4;
		}

		if ( (operation[0] == '*') || (operation[0] == '/') || (operation[0] == '%') )
		{
			return 5;
		}

		if ( (strcmp(operation, "-U") == 0) || (operation[0] == '!') ) 
		{
			return 6;
		}
	}
	
	return -1;
}


int isCorrectVarLValueStatement(LexemList* list)
{
	if ( (list == NULL) || (list->data == NULL) )
	{
		return 0;
	}
	
	int len = strlen(list->data->lexem);
	if ( (list->data->lexem[0] == '$') && (len > 1) )
	{
		return 1;
	}

	return 0;
}

int isCorrectVarRValueStatement(LexemList* list)
{
	if ( (list == NULL) || (list->data == NULL) )
	{
		return 0;
	}

	int count_open = 0;
	while ( (list != NULL) && (list->data != NULL) && (list->data->lexem[0] == '(') )
	{
		count_open++;
		list = list->next;
	}

	if ( (list == NULL) && (list->data == NULL) )
	{
		return 0;
	}

	int len = strlen(list->data->lexem);
	if ( (list->data->lexem[0] == '$') && (len > 1) )
	{
		list = list->next;
	}
	else
	{
		return 0;
	}

	int count_close = 0;
	while ( (list != NULL) && (list->data != NULL) && (list->data->lexem[0] == ')') )
	{
		count_close++;
		list = list->next;
	}

	if ( count_open == count_close )
	{
		return 1;
	}

	return 0;
}

int isCorrectArithmeticStatement(LexemList* list, LexemList** result_list)
{
	if ( (list == NULL) || (list->data == NULL) )
	{
		/*printf("\n[1] 0\n");*/
		return 0;
	}

	if ( *result_list != NULL )
		ll_clear(result_list, 1);
	*result_list = NULL;

	op_clear(&opstack);
	opstack = NULL;
	
	op_push(&opstack, "(", 2);
	
	while ( (list != NULL) && (list->data != NULL) && ( (list->data->lexem[0] != ';') && (strcmp(list->data->lexem, "then") != 0) ) )
	{
		/*printf("list = %s\n", list->data->lexem);*/

		if ( opstack == NULL )
		{
			ll_clear(result_list, 1);
			*result_list = NULL;
			/*printf("\n[2] 0\n");*/
			return 0;
		}

		int len = strlen(list->data->lexem);
		if ( (list->data->lexem_type == LEXEM_TYPE_INTEGER) || ( (list->data->lexem[0] == '$') && (len > 1) ) )
		{
			char buf[100];
			int i;
			for ( i = 0; list->data->lexem[i]; i++ )
				buf[i] = list->data->lexem[i];
			buf[i] = '\0';

			ll_insert(result_list, new Lexem(buf, i, list->data->num_str, list->data->lexem_type));
		}
		else if ( list->data->lexem[0] == '(' )
		{
			op_push(&opstack, "(", 2);
		}
		else if ( list->data->lexem[0] == ')' )
		{
			while ( (opstack != NULL) && (opstack->data[0] != '(') )
			{
				char* op = op_pop(&opstack);
				char buf[100];
				
				int i;
				for ( i = 0; op[i]; i++ )
					buf[i] = op[i];
				buf[i] = '\0';
				
				delete[] op;
				op = NULL;

				ll_insert(result_list, new Lexem(buf, i, list->data->num_str, LEXEM_TYPE_DELIMITER));
			}

			if ( (opstack == NULL) || (opstack->data[0] != '(') )
			{
				op_clear(&opstack);
				ll_clear(result_list, 1);
				*result_list = NULL;
				/*printf("\n[3] 0\n");*/
				return 0;
			}

			char* p = op_pop(&opstack);
			delete[] p;
			p = NULL;
		}
		else if ( isALOperation(list) )
		{
			int cur_oper_prior = getOperPrior(list, NULL, 0);
			int stack_oper_prior = getOperPrior(NULL, opstack->data, opstack->data_size);

			if ( (opstack->data[0] == '(') || ( stack_oper_prior < cur_oper_prior ) )
			{
				op_push(&opstack, list->data->lexem, list->data->size);
			}
			else if ( stack_oper_prior >= cur_oper_prior )
			{
				while ( stack_oper_prior >= cur_oper_prior )
				{
					char* op = op_pop(&opstack);
					char buf[100];
					
					int i;
					for ( i = 0; op[i]; i++ )
						buf[i] = op[i];
					buf[i] = '\0';
					
					delete[] op;
					op = NULL;

					ll_insert(result_list, new Lexem(buf, i, list->data->num_str, LEXEM_TYPE_DELIMITER));

					cur_oper_prior = getOperPrior(list, list->data->lexem, list->data->size);
					stack_oper_prior = getOperPrior(list, opstack->data, opstack->data_size);
				}

				ll_insert(result_list, list->data);
			}
		}
		else
		{
			if ( ( ll_get_size(*result_list) == 1 ) && ((*result_list)->data->lexem_type == LEXEM_TYPE_INTEGER) )
				break;

			op_clear(&opstack);
			ll_clear(result_list, 1);
			*result_list = NULL;
			/*printf("\n[4] 0\n");*/
			return 0;
		}

		list = list->next;
	}
	
	while ( opstack->data[0] != '(' )
	{
		char* op = op_pop(&opstack);
		char buf[100];
		
		int i;
		for ( i = 0; op[i]; i++ )
			buf[i] = op[i];
		buf[i] = '\0';
		
		delete[] op;
		op = NULL;

		ll_insert(result_list, new Lexem(buf, i, list->data->num_str, LEXEM_TYPE_DELIMITER));
	}
	
	char* op = op_pop(&opstack);
	delete[] op;
	op = NULL;

	if ( (opstack == NULL) && (*result_list != NULL) )
	{
		/*printf("\n[1] 1\n");*/
		return 1;
	}

	op_clear(&opstack);
	ll_clear(result_list, 1);
	*result_list = NULL;
	
	/*printf("\n[5] 0\n");*/
	return 0;
}

int isCorrectGameStatement(LexemList* list, LexemList** expr_list)
{
	if ( (list == NULL) || (list->data == NULL) )
	{
		return 0;
	}
	
	int op_num = -1;
	if ( (op_num = isValidGameFunction(list->data->lexem)) < 0 )
	{
		return 0;
	}

	/*printf("\n%s: op_num: %d\n", list->data->lexem, op_num);*/
	if ( op_num >= MONEY )
	{
		if ( (list->next == NULL) || (list->next->data == NULL) )
		{
			return 0;
		}

		if ( list->next->data->lexem[0] != '(' )
		{
			return 0;
		}
		
		LexemList* expr = list->next->next;
		/*printf("\nexpr: %s\n", expr->data->lexem);*/

		if ( (expr == NULL) || (expr->data == NULL) )
		{
			return 0;
		}

		if ( !isCorrectVarRValueStatement(expr) )
		{
			if ( !isCorrectGameStatement(expr, expr_list) )
			{
				LexemList* help_list = NULL;
				if ( !isCorrectArithmeticStatement(expr, &help_list) )
				{
					ll_clear(&help_list, 1);
					return 0;
				}
				
				*expr_list = help_list;
			}
			else
			{
				ll_insert(expr_list, expr->data);
			}
		}

		if ( (expr->next == NULL) || (expr->next->data == NULL) )
		{
			ll_clear(expr_list, 0);
			*expr_list = NULL;
			return 0;
		}

		if ( expr->next->data->lexem[0] != ')' )
		{
			ll_clear(expr_list, 0);
			*expr_list = NULL;
			return 0;
		}
	}

	return 1;
}


int ProcessArithmeticLexem(PolizItem** ret_list_ptr, LexemList* arithmetic_statement)
{
	if ( (ret_list_ptr == NULL) || (arithmetic_statement == NULL) )
	{
		fprintf(stderr, "\n[SyntaxAnalyzer]: ret_list_ptr OR arithmetic_statement is NULL\n");
		return 0;
	}

	int len = strlen(arithmetic_statement->data->lexem);
	if ( (arithmetic_statement->data->lexem[0] == '$') && (len > 1) )
	{
		PolizItem* vl = vars_list;
		PolizVar* x = NULL;
		while (vl != NULL )
		{
			x = dynamic_cast<PolizVar*>(vl->p);
			if ( x )
				if ( strcmp(x->GetVarName(), arithmetic_statement->data->lexem) == 0 )
					break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			pi_insert(ret_list_ptr, new PolizVarAddr(x->GetVarAddr()->Get()));
			return 1;
		}
		
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d): In function: TransformArithmetic() - variable (%s) is undeclared in program!\n", arithmetic_statement->data->num_str, arithmetic_statement->data->lexem);
		return 0;
	}
	
	if ( arithmetic_statement->data->lexem_type == LEXEM_TYPE_INTEGER )
	{
		int res = atoi(arithmetic_statement->data->lexem);
		pi_insert(ret_list_ptr, new PolizInt(res));
		return 1;
	}
	
	if ( strcmp(arithmetic_statement->data->lexem, "-U") == 0 )
	{
		pi_insert(ret_list_ptr, new PolizFunUnMinus());
		return 1;
	}	
	
	if ( strcmp(arithmetic_statement->data->lexem, "!") == 0)
	{
		pi_insert(ret_list_ptr, new PolizFunNot());
		return 1;
	}
	
	if ( strcmp(arithmetic_statement->data->lexem, "*") == 0 )
	{
		pi_insert(ret_list_ptr, new PolizFunMul());
		return 1;
	}
	
	if ( strcmp(arithmetic_statement->data->lexem, "/") == 0 )
	{
		pi_insert(ret_list_ptr, new PolizFunDiv());
		return 1;
	}
	
	if ( strcmp(arithmetic_statement->data->lexem, "%") == 0 )
	{
		pi_insert(ret_list_ptr, new PolizFunRem());
		return 1;
	}
	
	if ( strcmp(arithmetic_statement->data->lexem, "+") == 0 )
	{
		pi_insert(ret_list_ptr, new PolizFunPlus());
		return 1;
	}
	
	if ( strcmp(arithmetic_statement->data->lexem, "-") == 0 )
	{
		pi_insert(ret_list_ptr, new PolizFunMinus());
		return 1;
	}
	
	if ( strcmp(arithmetic_statement->data->lexem, "<") == 0 )
	{
		pi_insert(ret_list_ptr, new PolizFunLess());
		return 1;
	}
	
	if ( strcmp(arithmetic_statement->data->lexem, ">") == 0 )
	{
		pi_insert(ret_list_ptr, new PolizFunGreater());
		return 1;
	}
	
	if ( strcmp(arithmetic_statement->data->lexem, "=") == 0 )
	{
		pi_insert(ret_list_ptr, new PolizFunEqual());
		return 1;
	}
	
	if ( strcmp(arithmetic_statement->data->lexem, "!=") == 0 )
	{
		pi_insert(ret_list_ptr, new PolizFunNotEqual());
		return 1;
	}
	
	if ( strcmp(arithmetic_statement->data->lexem, "&") == 0 )
	{
		pi_insert(ret_list_ptr, new PolizFunAnd());
		return 1;
	}
	
	if ( strcmp(arithmetic_statement->data->lexem, "|") == 0 )
	{
		pi_insert(ret_list_ptr, new PolizFunOr());
		return 1;
	}
	
	fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d): In function: TransformArithmetic() - expected a valid operation sign or \"rvalue\"!\n", arithmetic_statement->data->num_str);
	return 0;
}

int ProcessGameLexemArgument(PolizItem** ret_list, LexemList* game_statement, const char* game_function_name)
{
	if ( (ret_list == NULL) || (game_function_name == NULL) )
	{
		fprintf(stderr, "\n[SyntaxAnalyzer]: In function: TransformGameStatement() - ret_list OR game_function_name is NULL\n");
		return 0;
	}

	if ( (game_statement == NULL) || (game_statement->data == NULL) )
	{
		fprintf(stderr, "\n[SyntaxAnalyzer]: In function: TransformGameStatement() - in a \"%s\" game statement expected \"rvalue\" expression!\n", game_function_name);
		return 0;
	}

	LexemList* help_list = NULL;
	int len = strlen(game_statement->data->lexem);
	if ( (game_statement->data->lexem[0] == '$') && (len > 1) )
	{
		PolizElem* elem = TransformRVal(game_statement);
		if ( !elem )
		{
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line: (%d) In function: TransformGameStatement() - in a \"%s\" game statement expected CORRECT \"rvalue\" expression!\n", game_statement->data->num_str, game_function_name);
			return 0;
		}
		
		PolizInt* int1 = dynamic_cast<PolizInt*>(elem);
		PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(elem);
		if ( int1 )
		{
			pi_insert(ret_list, int1);
			return 1;
		}
		
		if ( va1 )
		{
			pi_insert(ret_list, va1);
			return 1;
		}

		fprintf(stderr, "\n[SyntaxAnalyzer]: Line: (%d) In function: TransformGameStatement() - in a \"%s\" game statement expected CORRECT \"rvalue\" expression!\n", game_statement->data->num_str, game_function_name);
		return 0;
	}
	
	if ( game_statement->data->lexem_type == LEXEM_TYPE_INTEGER )
	{
		int res = atoi(game_statement->data->lexem);
		PolizInt* int1 = new PolizInt(res);
		if ( !int1 )
		{
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line: (%d) In function: TransformGameStatement() - in a \"%s\" game statement error with \"new\" operator!\n", game_statement->data->num_str, game_function_name);
			return 0;
		}

		pi_insert(ret_list, int1);
		return 1;
	}
	
	if ( isValidGameFunction(game_statement->data->lexem) > -1 )
	{
		PolizItem* res = TransformGameStatement(game_statement);
		if ( !res ) 
		{
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line: (%d) In function: TransformGameStatement() - in a \"%s\" game statement expected CORRECT game statement!\n", game_statement->data->num_str, game_function_name);
			return 0;
		}

		PolizItem* ret = *ret_list;
		while ( (ret != NULL) && (ret->next != NULL) )
			ret = ret->next;
		
		if ( ret == NULL )
			*ret_list = res;
		else
			ret->next = res;
		res->prev = ret;

		return 1;
	}
	
	if ( isCorrectArithmeticStatement(game_statement, &help_list) )
	{
		ll_clear(&help_list, 1);
		PolizItem* res = TransformArithmetic(game_statement);
		if ( !res )
		{
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line: (%d) In function: TransformGameStatement() - in a \"%s\" game statement expected CORRECT arithmetic statement!\n", game_statement->data->num_str, game_function_name);
			return 0;
		}

		PolizItem* ret = *ret_list;
		while ( (ret != NULL) && (ret->next != NULL) )
			ret = ret->next;
		
		if ( ret == NULL )
			*ret_list = res;
		else
			ret->next = res;
		res->prev = ret;

		return 1;
	}
	
	fprintf(stderr, "\n[SyntaxAnalyzer]: Line: (%d) In function: TransformGameStatement() - in a \"%s\" game statement error expected \"rvalue\", integer,"
					" game function or arithmetic statement!\n", game_statement->data->num_str, game_function_name);
	return 0;
}

int ProcessGameLexem(PolizItem** ret_list, LexemList* game_statement)
{
	if ( (ret_list == NULL) || (game_statement == NULL) )
	{
		fprintf(stderr, "\n[SyntaxAnalyzer]: In function ProcessGameLexem - ret_list OR game_statement is NULL\n");
		return 0;
	}

	if ( strcmp(game_statement->data->lexem, game_functions[MY_ID]) == 0 )
	{
		pi_insert(ret_list, new PolizGFMyId());
		return 1;
	}
	
	if ( strcmp(game_statement->data->lexem, game_functions[TURN]) == 0 )
	{
		pi_insert(ret_list, new PolizGFTurn());
		return 1;
	}

	if ( strcmp(game_statement->data->lexem, game_functions[PLAYERS]) == 0 )
	{
		pi_insert(ret_list, new PolizGFPlayers());
		return 1;
	}
	
	if ( strcmp(game_statement->data->lexem, game_functions[ACTIVE_PLAYERS]) == 0 )
	{
		pi_insert(ret_list, new PolizGFAPlayers());
		return 1;
	}
	
	if ( strcmp(game_statement->data->lexem, game_functions[SUPPLY]) == 0 )
	{
		pi_insert(ret_list, new PolizGFSupply());
		return 1;
	}
	
	if ( strcmp(game_statement->data->lexem, game_functions[RAW_PRICE]) == 0 )
	{
		pi_insert(ret_list, new PolizGFRawPrice());
		return 1;
	}
	
	if ( strcmp(game_statement->data->lexem, game_functions[DEMAND]) == 0 )
	{
		pi_insert(ret_list, new PolizGFDemand());
		return 1;
	}
	
	if ( strcmp(game_statement->data->lexem, game_functions[PROD_PRICE]) == 0 )
	{
		pi_insert(ret_list, new PolizGFProdPrice());
		return 1;
	}

	if ( strcmp(game_statement->data->lexem, game_functions[MONEY]) == 0 )
	{
		game_statement = game_statement->next;
		
		if ( !ProcessGameLexemArgument(ret_list, game_statement, game_functions[MONEY]) )
		{
			return 0;
		}

		pi_insert(ret_list, new PolizGFMoney());

		return 1;
	}

	if ( strcmp(game_statement->data->lexem, game_functions[RAW]) == 0 )
	{
		game_statement = game_statement->next;

		if ( !ProcessGameLexemArgument(ret_list, game_statement, game_functions[RAW]) )
		{
			return 0;
		}
	
		pi_insert(ret_list, new PolizGFRaw());

		return 1;
	}
	
	if ( strcmp(game_statement->data->lexem, game_functions[PRODUCTION]) == 0 )
	{
		game_statement = game_statement->next;
		
		if ( !ProcessGameLexemArgument(ret_list, game_statement, game_functions[PRODUCTION]) )
		{
			return 0;
		}

		pi_insert(ret_list, new PolizGFProduction());

		return 1;
	}
	
	if ( strcmp(game_statement->data->lexem, game_functions[FACTORIES]) == 0 )
	{
		game_statement = game_statement->next;

		if ( !ProcessGameLexemArgument(ret_list, game_statement, game_functions[FACTORIES]) )
		{
			return 0;
		}

		pi_insert(ret_list, new PolizGFFactories());

		return 1;
	}
	
	if ( strcmp(game_statement->data->lexem, game_functions[MANUFACTURED]) == 0 )
	{
		game_statement = game_statement->next;
	
		if ( !ProcessGameLexemArgument(ret_list, game_statement, game_functions[MANUFACTURED]) )
		{
			return 0;
		}

		pi_insert(ret_list, new PolizGFManufactured());

		return 1;
	}
	
	if ( strcmp(game_statement->data->lexem, game_functions[RES_RAW_SOLD]) == 0 )
	{
		game_statement = game_statement->next;

		if ( !ProcessGameLexemArgument(ret_list, game_statement, game_functions[RES_RAW_SOLD]) )
		{
			return 0;
		}

		pi_insert(ret_list, new PolizGFResRawSold());

		return 1;
	}

	if ( strcmp(game_statement->data->lexem, game_functions[RES_RAW_PRICE]) == 0 )
	{
		game_statement = game_statement->next;

		if ( !ProcessGameLexemArgument(ret_list, game_statement, game_functions[RES_RAW_PRICE]) )
		{
			return 0;
		}

		pi_insert(ret_list, new PolizGFResRawPrice());

		return 1;
	}

	if ( strcmp(game_statement->data->lexem, game_functions[RES_PROD_BOUGHT]) == 0 )
	{
		game_statement = game_statement->next;

		if ( !ProcessGameLexemArgument(ret_list, game_statement, game_functions[RES_PROD_BOUGHT]) )
		{
			return 0;
		}

		pi_insert(ret_list, new PolizGFResProdBought());

		return 1;
	}

	if ( strcmp(game_statement->data->lexem, game_functions[RES_PROD_PRICE]) == 0 )
	{
		game_statement = game_statement->next;

		if ( !ProcessGameLexemArgument(ret_list, game_statement, game_functions[RES_PROD_PRICE]) )
		{
			return 0;
		}

		pi_insert(ret_list, new PolizGFResProdPrice());

		return 1;
	}
	
	fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - argument \"%s\" is not a valid game function!\n", game_statement->data->num_str, game_statement->data->lexem);
	return 0;
}

PolizElem* TransformRVal(LexemList* rvalue)
{
	if ( (rvalue == NULL) || (rvalue->data == NULL) )	
	{
		fprintf(stderr, "%s", "\n[SyntaxAnalyzer]: In function: TransformRVal() - expected arithmetic statement\n");
		return NULL;
	}

	PolizItem* vl = vars_list;
	PolizVar* x = NULL;
	while (vl != NULL )
	{
		x = dynamic_cast<PolizVar*>(vl->p);
		if ( x )
			if ( strcmp(x->GetVarName(), rvalue->data->lexem) == 0 )
				break;

		vl = vl->next;
	}
	
	if ( vl != NULL )
	{
		return new PolizVarAddr(x->GetVarAddr()->Get());				
	}
	
	fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d): In function: TransformRVal() - variable \"%s\" is undeclared in program!\n", rvalue->data->num_str, rvalue->data->lexem);
	return NULL;
}


PolizItem* TransformArithmetic(LexemList* arith_statement)
{
	PolizItem* ret_list = NULL;

	if ( (arith_statement == NULL) || (arith_statement->data == NULL) )
	{
		fprintf(stderr, "%s", "\n[SyntaxAnalyzer]: In function: TransformArithmetic() - expected arithmetic statement\n");
		return NULL;
	}

	while ( (arith_statement != NULL) && (arith_statement->data != NULL) )
	{
		if ( !ProcessArithmeticLexem(&ret_list, arith_statement) )
		{
			return NULL;
		}

		arith_statement = arith_statement->next;
	}

	return ret_list;
}

PolizItem* TransformGameStatement(LexemList* game_statement)
{
	PolizItem* ret_list = NULL;

	if ( (game_statement == NULL) || (game_statement->data == NULL) )
	{
		fprintf(stderr, "%s", "\n[SyntaxAnalyzer]: In function: TransformGameStatement() - expected a game statement!\n");
		return NULL;
	}
	
	if ( !ProcessGameLexem(&ret_list, game_statement) )
	{
		return NULL;
	}

	return ret_list;
}

int isSimpleGF(PolizItem* item)
{
	/* [!] В функции делается допущение что item->p относится к объекту класса игровых функций [!]*/

	if ( (item == NULL) || (item->p == NULL) )
	{
		if ( (item != NULL) && (item->p == NULL) )
			fprintf(stderr, "\n[SA_Additional]: In function isSimpleGF() \"item->next->p\" is NULL\n");

		return 0;
	}

	if (
	   ( dynamic_cast<PolizGFMyId*>(item->p) != NULL )			||
	   ( dynamic_cast<PolizGFTurn*>(item->p) != NULL )			||
	   ( dynamic_cast<PolizGFPlayers*>(item->p) != NULL )		||
	   ( dynamic_cast<PolizGFAPlayers*>(item->p) != NULL )		||
	   ( dynamic_cast<PolizGFSupply*>(item->p) != NULL )		||
	   ( dynamic_cast<PolizGFRawPrice*>(item->p) != NULL )		||
	   ( dynamic_cast<PolizGFDemand*>(item->p) != NULL )		||
	   ( dynamic_cast<PolizGFProdPrice*>(item->p) != NULL )
	   )
	{
		return 1;
	}
	
	return 0;
}

#endif
