/* Файл реализации модуля SyntaxAnalyzer */

#ifndef SYNTAX_ANALYZER_CPP
#define SYNTAX_ANALYZER_CPP

#ifndef __CPP_FILE__
#define __CPP_FILE__
#endif

#include <cstdio>
#include <cstring>
#include "../includes/SyntaxAnalyzer.hpp"
#include "../includes/Lexem.hpp"
#include "../includes/LexemList.hpp"
#include "../includes/PolizElem.hpp"
#include "../includes/SA_Additional.hpp"

/* Описаны в модуле SA_Additional */
extern const char* game_operators[];
extern LabelsList* labels_list;
extern PolizItem* vars_list;

PolizItem* SyntaxAnalyzer::isCorrectLabelOperator(LexemList* list, LexemList** next_lexem)
{
	PolizItem* ret_list = NULL;

	if ( (list->next == NULL) || (list->next->data == NULL) )
	{
		*next_lexem = NULL;
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected a \":\" symbol in \"label\" operator!\n", list->data->num_str);
		return NULL;
	}

	if ( list->next->data->lexem[0] != ':' )
	{
		*next_lexem = NULL;
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected a \":\" symbol in \"label\" operator!\n", list->data->num_str);
		return NULL;
	}
	
	if ( (list->next->next == NULL) || (list->next->next->data == NULL) )
	{
		*next_lexem = NULL;
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"assign\", \"goto\", \"if\" or \"game\" operator!\n", list->data->num_str);
		return NULL;
	}
	
	if ( (ret_list = isCorrectAssignOperator(list->next->next, next_lexem)) )
	{
		if ( label_find(labels_list, list->data->lexem) == NULL ) 
			label_insert(&labels_list, new LabelData(list->data->lexem, list->data->size, ret_list));
		return ret_list;
	}
	
	if ( (ret_list = isCorrectGoOperator(list->next->next, next_lexem)) )
	{
		if ( label_find(labels_list, list->data->lexem) == NULL ) 
			label_insert(&labels_list, new LabelData(list->data->lexem, list->data->size, ret_list));
		return ret_list;
	}

	if ( (ret_list = isCorrectIfOperator(list->next->next, next_lexem)) )
	{
		if ( label_find(labels_list, list->data->lexem) == NULL ) 
			label_insert(&labels_list, new LabelData(list->data->lexem, list->data->size, ret_list));
		return ret_list;
	}

	if ( (ret_list = isCorrectGameOperator(list->next->next, next_lexem)) )
	{
		if ( label_find(labels_list, list->data->lexem) == NULL ) 
			label_insert(&labels_list, new LabelData(list->data->lexem, list->data->size, ret_list));
		return ret_list;
	}
	
	*next_lexem = NULL;
	fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"assign\", \"goto\", \"if\" or \"game\" operator!\n", list->data->num_str);
	return NULL;
}

PolizItem* SyntaxAnalyzer::isCorrectAssignOperator(LexemList* list, LexemList** next_lexem)
{
	int correct = 0;
	PolizItem* ret_list = NULL;

	if ( (list == NULL) || (list->data == NULL) )
	{
		fprintf(stderr, "%s", "\n[SyntaxAnalyzer]: expected an assign operator!\n");
		*next_lexem = NULL;
		return NULL;
	}
	
	LexemList* lvalue = NULL;
	LexemList* rvalue = NULL;
	LexemList* arith_statement = NULL;
	LexemList* game_statement = NULL;
	LexemList* help_list = NULL;
	
	if ( isCorrectVarLValueStatement(list) )
	{
		correct = 1;
		lvalue = list;
	}
	
	if ( !correct )
	{
		if ( list->data->num_str > 2 )
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - an error with \"lvalue\" operand in assign operator!\n", list->data->num_str);
		*next_lexem = NULL;
		return NULL;
	}
	
	correct = 0;

	if ( (list->next == NULL) || (list->next->data == NULL) || (list->next->data->lexem_type != LEXEM_TYPE_ASSIGN) )
	{
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected assign lexem in assign operator!\n", list->data->num_str);
		*next_lexem = NULL;
		return NULL;
	}
	
	if ( (list->next->next == NULL) || (list->next->next->data == NULL ) )
	{
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - an error[0] with \"rvalue\" operand in assign operator!\n", list->data->num_str);
		*next_lexem = NULL;
		return NULL;
	}
		
	if ( isCorrectVarRValueStatement(list->next->next) )
	{
		correct = 1;
		rvalue = list->next->next;
	}
	else if ( isCorrectArithmeticStatement(list->next->next, &help_list) )
	{
		correct = 1;
		arith_statement = help_list;
	}
	else if ( isCorrectGameStatement(list->next->next, &help_list) )
	{
		correct = 1;
		ll_insert(&game_statement, list->next->next->data);
		if ( help_list != NULL )
		{
			game_statement->next = help_list;
			help_list->prev = game_statement;
		}
	}
	
	if ( !correct )
	{
		if ( arith_statement )
			ll_clear(&help_list, 1);
		else if ( game_statement )
			ll_clear(&game_statement, 0);

		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - an error[1] with \"rvalue\" operand in assign operator!\n", list->data->num_str);
		*next_lexem = NULL;
		return NULL;
	}

	LexemList* l = list->next->next->next;

	while ( (l != NULL) && (l->data != NULL) && (l->data->lexem[0] != ';') )
	{
		l = l->next;
	}

	if ( (l == NULL) || (l->data == NULL) )
	{
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected[0] a \";\" symbol in assign operator!\n", list->data->num_str);
		
		if ( arith_statement )
			ll_clear(&help_list, 1);
		else if ( game_statement )
			ll_clear(&game_statement, 0);
		
		*next_lexem = NULL;
		return NULL;
	}

	*next_lexem = l->next;
	

	PolizItem* vl = vars_list;
	PolizVar* x = NULL;
	PolizVarAddr* lval = NULL;
	while ( vl != NULL )
	{
		x = dynamic_cast<PolizVar*>(vl->p);
		if ( x )
			if ( strcmp(x->GetVarName(), lvalue->data->lexem) == 0 )
				break;

		vl = vl->next;
	}
			
	if ( vl == NULL )
	{
		int size = strlen(lvalue->data->lexem)+1;
		PolizInt int1(0);
		PolizVar* z = new PolizVar(lvalue->data->lexem, size, &int1);
		lval = z->GetVarAddr();
	}
	else
	{
		lval = x->GetVarAddr();
	}

	if ( rvalue )
	{
		PolizElem* elem = TransformRVal(rvalue);
		if ( !elem )
		{
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - variable (%s) is undeclared in program!\n", list->data->num_str, rvalue->data->lexem);
			*next_lexem = NULL;
			return NULL;
		}
		
		PolizInt* rval_int = dynamic_cast<PolizInt*>(elem);
		PolizVarAddr* rval_var = dynamic_cast<PolizVarAddr*>(elem);
		if ( rval_int )
		{
			pi_insert(&ret_list, rval_int);
		}
		else if ( rval_var )
		{
			pi_insert(&ret_list, rval_var);
		}
		pi_insert(&ret_list, new PolizVarAddr(lval->Get()) );
		pi_insert(&ret_list, new PolizAssign() );

		return ret_list;
	}
		
	if ( arith_statement )
	{
		PolizItem* ret = TransformArithmetic(arith_statement);
		if ( !ret )
		{
			ll_clear(&help_list, 1);
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not an arithmetic statement!\n", list->data->num_str);
			*next_lexem = NULL;
			return NULL;
		}
		
		PolizItem* ret_first = ret;
		while ( ret->next != NULL )
		{
			ret = ret->next;
		}		

		pi_insert(&ret_list, new PolizVarAddr(lval->Get()) );
		pi_insert(&ret_list, new PolizAssign() );
		
		ret->next = ret_list;
		ret_list->prev = ret;
		ret_list = ret_first;

		ll_clear(&help_list, 1);

		return ret_list;
	}
		
	if ( game_statement ) 
	{
		PolizItem* ret = TransformGameStatement(game_statement);
		if ( !ret )
		{
			ll_clear(&game_statement, 0);
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not a game statement!\n", list->data->num_str);
			*next_lexem = NULL;
			return NULL;
		}
		
		PolizItem* ret_first = ret;
		while ( ret->next != NULL )
		{
			ret = ret->next;
		}

		pi_insert(&ret_list, new PolizVarAddr(lval->Get()) );
		pi_insert(&ret_list, new PolizAssign() );
		
		ret->next = ret_list;
		ret_list->prev = ret;
		ret_list = ret_first;
		
		ll_clear(&game_statement, 0);

		return ret_list;
	}
	
	fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - an error[2] with \"rvalue\" operand in assign operator!\n", list->data->num_str);
	*next_lexem = NULL;
	return NULL;
}

PolizItem* SyntaxAnalyzer::isCorrectGoOperator(LexemList* list, LexemList** next_lexem)
{
	PolizItem* ret_list = NULL;
	int correct = 0;

	if ( (list == NULL) || (list->data == NULL) )
	{
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"goto\" operator!\n", list->data->num_str);
		*next_lexem = NULL;
		return NULL;
	}

	if ( (list->next == NULL) || (list->next->data == NULL) )
	{
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected label statement in \"goto\" operator!\n", list->data->num_str);
		*next_lexem = NULL;
		return NULL;
	}
	
	LabelsList* label = NULL;
	if ( (label = label_find(labels_list, list->next->data->lexem)) != NULL )
	{
		correct = 1;
	}
	
	if ( !correct )
	{
		if ( list->data->num_str > 2 )
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected CORRECT label statement in \"goto\" operator!\n", list->data->num_str);
		*next_lexem = NULL;
		return NULL;
	}

	LexemList* l = list->next->next;

	if ( (l == NULL) || (l->data == NULL) )
	{
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected[0] a \";\" symbol in \"goto\" operator!\n", list->data->num_str);
		*next_lexem = NULL;
		return NULL;
	}
	
	if ( l->data->lexem[0] != ';' )
	{
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected[1] a \";\" symbol in \"goto\" operator!\n", l->data->num_str);
		*next_lexem = NULL;
		return NULL;
	}
	
	*next_lexem = l->next;

	pi_insert(&ret_list, new PolizLabel(label->data->oper));
	pi_insert(&ret_list, new PolizOpGo());

	return ret_list;	
}

PolizItem* SyntaxAnalyzer::isCorrectIfOperator(LexemList* list, LexemList** next_lexem)
{
	PolizItem* ret_list = NULL;
	int correct = 0;

	if ( (list == NULL) || (list->data == NULL) )
	{
		fprintf(stderr, "%s", "\n[SyntaxAnalyzer]: expected \"if\" operator!\n");
		*next_lexem = NULL;
		return NULL;
	}
	
	if ( (list->next == NULL) || (list->next->data == NULL) )
	{
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected arithmetic or game expression in \"if\" operator!\n", list->data->num_str);
		*next_lexem = NULL;
		return NULL;
	}
	
	LexemList* help_list = NULL;
	LexemList* arithmetic_statement = NULL;
	LexemList* game_statement = NULL;

	if ( isCorrectArithmeticStatement(list->next, &help_list) )
	{
		correct = 1;
		arithmetic_statement = help_list;
	}
	else if ( isCorrectGameStatement(list->next, &help_list) )
	{
		correct = 1;
		ll_insert(&game_statement, list->next->data);
		if ( help_list != NULL )
		{
			game_statement->next = help_list;
			help_list->prev = game_statement;
		}
	}
	
	if ( !correct )
	{
		if ( arithmetic_statement )
			ll_clear(&help_list, 1);
		else if ( game_statement )
			ll_clear(&game_statement, 0);

		if ( list->data->num_str > 2 )
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected CORRECT arithmetic or game expression in \"if\" operator!\n", list->data->num_str);
		*next_lexem = NULL;
		return NULL;
	}
	
	
	if ( arithmetic_statement )
	{
		PolizItem* ret = TransformArithmetic(arithmetic_statement);
		if ( !ret )
		{
			ll_clear(&help_list, 1);
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not an arithmetic statement!\n", list->data->num_str);
			*next_lexem = NULL;
			return NULL;
		}
	
		PolizItem* ret_first = ret;
		while ( ret->next != NULL )
		{
			ret = ret->next;
		}		

		pi_insert(&ret_list, new PolizOpIfThen() );
		
		ret->next = ret_list;
		ret_list->prev = ret;
		ret_list = ret_first;

		ll_clear(&help_list, 1);
	}
	else if ( game_statement )
	{
		PolizItem* ret = TransformGameStatement(game_statement);
		if ( !ret )
		{
			ll_clear(&game_statement, 0);
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not a game statement!\n", list->data->num_str);
			*next_lexem = NULL;
			return NULL;
		}
		
		PolizItem* ret_first = ret;
		while ( ret->next != NULL )
		{
			ret = ret->next;
		}
		
		ll_clear(&game_statement, 0);

		pi_insert(&ret_list, new PolizOpIfThen() );
		
		ret->next = ret_list;
		ret_list->prev = ret;
		ret_list = ret_first;
	}


	LexemList* then = list->next->next;
	while ( (then != NULL) && (then->data != NULL) && (strcmp("then", then->data->lexem) != 0) )
	{
		then = then->next;
	}

	if ( (then == NULL) || (then->data == NULL) )
	{
		if ( arithmetic_statement )
			ll_clear(&help_list, 1);
		else if ( game_statement )
			ll_clear(&game_statement, 0);

		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"then\" label!\n", list->data->num_str);
		*next_lexem = NULL;
		return NULL;
	}

	if ( (then->next == NULL) || (then->next->data == NULL) )
	{
		if ( arithmetic_statement )
			ll_clear(&help_list, 1);
		else if ( game_statement )
			ll_clear(&game_statement, 0);

		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"assign\", \"goto\" or game operator!\n", list->data->num_str);
		*next_lexem = NULL;
		return NULL;
	}
	
	PolizItem* result = NULL;
	if ( (result = isCorrectAssignOperator(then->next, next_lexem)) )
	{
		PolizItem* ret = ret_list;
		while ( (ret != NULL) && (ret->next != NULL) )
			ret = ret->next;
		
		if ( ret == NULL )
			ret_list = result;
		else
			ret->next = result;
		result->prev = ret;

		return ret_list;
	}
	
	if ( (result = isCorrectGoOperator(then->next, next_lexem)) )
	{
		PolizItem* ret = ret_list;
		while ( (ret != NULL) && (ret->next != NULL) )
			ret = ret->next;
		
		if ( ret == NULL)
			ret_list = result;
		else
			ret->next = result;
		result->prev = ret;	

		return ret_list;
	}

	if ( (result = isCorrectGameOperator(then->next, next_lexem)) )
	{
		PolizItem* ret = ret_list;
		while ( (ret != NULL) && ret->next != NULL )
			ret = ret->next;
		
		if ( ret == NULL)
			ret_list = result;
		else	
			ret->next = result;
		result->prev = ret;
		
		return ret_list;
	}
	
	fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"assign\", \"goto\" or game operator!\n", list->data->num_str);
	*next_lexem = NULL;
	return NULL;
}

PolizItem* SyntaxAnalyzer::isCorrectGameOperator(LexemList* list, LexemList** next_lexem)
{
	PolizItem* ret_list = NULL;
	PolizItem* buf_list = NULL;

	if ( (list == NULL) || (list->data == NULL) )
	{	
		fprintf(stderr, "%s", "\n[SyntaxAnalyzer]: expected game operator!\n");
		*next_lexem = NULL;
		return NULL;
	}

	if ( strcmp(game_operators[ENDTURN_OPERATOR], list->data->lexem) == 0 )
	{
		if ( (list->next == NULL) || (list->next->data == NULL) )
		{
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"%s\" operator!\n", list->data->num_str, game_operators[ENDTURN_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}

		if ( list->next->data->lexem[0] != ';' )
		{
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \";\" in \"%s\" operator!\n", list->data->num_str, game_operators[ENDTURN_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		*next_lexem = list->next->next;

		pi_insert(&ret_list, new PolizEndturn());

		return ret_list;
	}

	if ( strcmp(game_operators[BUILD_OPERATOR], list->data->lexem) == 0 )
	{
		int correct = 0;

		if ( (list->next == NULL) || (list->next->data == NULL) )
		{
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[BUILD_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		LexemList* rvalue = NULL;
		LexemList* arithmetic_statement = NULL;
		LexemList* game_statement = NULL;
		LexemList* help_list = NULL;


		if ( isCorrectVarRValueStatement(list->next) )
		{
			correct = 1;
			rvalue = list->next;
		}
		else if ( isCorrectArithmeticStatement(list->next, &help_list) )
		{
			correct = 1;
			arithmetic_statement = help_list;
		}
		else if ( isCorrectGameStatement(list->next, &help_list) )
		{
			correct = 1;
			ll_insert(&game_statement, list->next->data);
			if ( help_list != NULL )
			{
				game_statement->next = help_list;
				help_list->prev = game_statement;
			}
		}

		if ( !correct )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected correct \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[BUILD_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		if ( (list->next->next == NULL) || (list->next->next->data == NULL) )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \";\" in \"%s\" operator!\n", list->data->num_str, game_operators[BUILD_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}

		if ( list->next->next->data->lexem[0] != ';' )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \";\" in \"%s\" operator!\n", list->data->num_str, game_operators[BUILD_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		LexemList* l = list->next->next;
		*next_lexem = l->next;

		if ( rvalue )
		{
			PolizElem* elem = TransformRVal(rvalue);
			if ( !elem )
			{
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - variable (%s) is undeclared in program!\n", list->data->num_str, rvalue->data->lexem);
				*next_lexem = NULL;
				return NULL;
			}
			
			PolizInt* rval_int = dynamic_cast<PolizInt*>(elem);
			PolizVarAddr* rval_var = dynamic_cast<PolizVarAddr*>(elem);
			if ( rval_int )
			{
				pi_insert(&ret_list, rval_int);
			}
			else if ( rval_var )
			{
				pi_insert(&ret_list, rval_var);
			}
			pi_insert(&ret_list, new PolizBuild());

			return ret_list;
		}
		
		if ( arithmetic_statement )
		{
			PolizItem* ret = TransformArithmetic(arithmetic_statement);
			if ( !ret )
			{
				ll_clear(&help_list, 1);
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not an arithmetic expression!\n", list->data->num_str);
				*next_lexem = NULL;
				return NULL;
			}

			PolizItem* ret_first = ret;
			while ( ret->next != NULL )
			{
				ret = ret->next;
			}
				
			pi_insert(&ret_list, new PolizBuild() );
			
			ret->next = ret_list;
			ret_list->prev = ret;
			ret_list = ret_first;

			ll_clear(&help_list, 1);

			return ret_list;
		}
		
		if ( game_statement )
		{
			PolizItem* ret = TransformGameStatement(game_statement);
			if ( !ret )
			{
				ll_clear(&game_statement, 0);
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not a game statement!\n", list->data->num_str);
				*next_lexem = NULL;
				return NULL;
			}

			PolizItem* ret_first = ret;
			while ( ret->next != NULL )
			{
				ret = ret->next;
			}

			pi_insert(&ret_list, new PolizBuild() );
			
			ret->next = ret_list;
			ret_list->prev = ret;
			ret_list = ret_first;

			ll_clear(&game_statement, 0);

			return ret_list;
		}

		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[BUILD_OPERATOR]);
		*next_lexem = NULL;
		return NULL;
	}

	if ( strcmp(game_operators[PROD_OPERATOR], list->data->lexem) == 0 )
	{
		int correct = 0;

		if ( (list->next == NULL) || (list->next->data == NULL) )
		{
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[PROD_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		LexemList* rvalue = NULL;
		LexemList* arithmetic_statement = NULL;
		LexemList* game_statement = NULL;
		LexemList* help_list = NULL;
		
		if ( isCorrectVarRValueStatement(list->next) )
		{
			correct = 1;
			rvalue = list->next;
		}
		else if ( isCorrectArithmeticStatement(list->next, &help_list) )
		{
			correct = 1;
			arithmetic_statement = help_list;
		}
		else if ( isCorrectGameStatement(list->next, &help_list) )
		{
			correct = 1;
			ll_insert(&game_statement, list->next->data);
			if ( help_list != NULL)
			{
				game_statement->next = help_list;
				help_list->prev = game_statement;
			}
		}
	
		if ( !correct )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected CORRECT \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[PROD_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		if ( (list->next->next == NULL) || (list->next->next->data == NULL) )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \";\" in \"%s\" operator!\n", list->data->num_str, game_operators[PROD_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}

		if ( list->next->next->data->lexem[0] != ';' )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \";\" in \"%s\" operator!\n", list->data->num_str, game_operators[PROD_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		LexemList* l = list->next->next;
		*next_lexem = l->next;


		if ( rvalue )
		{
			PolizElem* elem = TransformRVal(rvalue);
			if ( !elem )
			{
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - variable (%s) is undeclared in program!\n", list->data->num_str, rvalue->data->lexem);
				*next_lexem = NULL;
				return NULL;
			}
			
			PolizInt* rval_int = dynamic_cast<PolizInt*>(elem);
			PolizVarAddr* rval_var = dynamic_cast<PolizVarAddr*>(elem);
			if ( rval_int )
			{
				pi_insert(&ret_list, rval_int);
			}
			else if ( rval_var )
			{
				pi_insert(&ret_list, rval_var);
			}
			pi_insert(&ret_list, new PolizProd());

			return ret_list;
		}
		
		if ( arithmetic_statement )
		{
			PolizItem* ret = TransformArithmetic(arithmetic_statement);
			if ( !ret )
			{
				ll_clear(&help_list, 1);
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not an arithmetic expression!\n", list->data->num_str);
				*next_lexem = NULL;
				return NULL;
			}
			
			PolizItem* ret_first = ret;
			while ( ret->next != NULL )
			{
				ret = ret->next;
			}		

			pi_insert(&ret_list, new PolizProd() );
			
			ret->next = ret_list;
			ret_list->prev = ret;
			ret_list = ret_first;
			
			ll_clear(&help_list, 1);

			return ret_list;
		}
		
		if ( game_statement )
		{
			PolizItem* ret = TransformGameStatement(game_statement);
			if ( !ret )
			{
				ll_clear(&game_statement, 0);
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not a game statement!\n", list->data->num_str);
				*next_lexem = NULL;
				return NULL;
			}
			
			PolizItem* ret_first = ret;
			while ( ret->next != NULL )
			{
				ret = ret->next;
			}

			pi_insert(&ret_list, new PolizProd() );
			
			ret->next = ret_list;
			ret_list->prev = ret;
			ret_list = ret_first;

			ll_clear(&game_statement, 0);

			return ret_list;
		}

		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[PROD_OPERATOR]);
		*next_lexem = NULL;
		return NULL;
	}

	if ( strcmp(game_operators[SELL_OPERATOR], list->data->lexem) == 0 )
	{
		int correct = 0;

		if ( (list->next == NULL) || (list->next->data == NULL) )
		{
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[SELL_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		LexemList* rvalue = NULL;
		LexemList* arithmetic_statement = NULL;
		LexemList* game_statement = NULL;
		LexemList* help_list = NULL;

		if ( isCorrectVarRValueStatement(list->next) )
		{
			correct = 1;
			rvalue = list->next;
		}
		else if ( isCorrectArithmeticStatement(list->next, &help_list) )
		{
			correct = 1;
			arithmetic_statement = help_list;
		}
		else if ( isCorrectGameStatement(list->next, &help_list) )
		{
			correct = 1;
			ll_insert(&game_statement, list->next->data);
			if ( help_list != NULL)
			{
				game_statement->next = help_list;
				help_list->prev = game_statement;
			}
		}
	
		if ( !correct )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected CORRECT \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[SELL_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}

		if ( rvalue )
		{
			PolizElem* elem = TransformRVal(rvalue);
			if ( !elem )
			{
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - variable (%s) is undeclared in program!\n", list->data->num_str, rvalue->data->lexem);
				*next_lexem = NULL;
				return NULL;
			}
			
			PolizInt* rval_int = dynamic_cast<PolizInt*>(elem);
			PolizVarAddr* rval_var = dynamic_cast<PolizVarAddr*>(elem);
			if ( rval_int )
			{
				pi_insert(&ret_list, rval_int);
			}
			else if ( rval_var )
			{
				pi_insert(&ret_list, rval_var);
			}
		}
		else if ( arithmetic_statement )
		{
			PolizItem* as1 = TransformArithmetic(arithmetic_statement);
			if ( !as1 )
			{
				ll_clear(&help_list, 1);
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not an arithmetic expression!\n", list->data->num_str);
				*next_lexem = NULL;
				return NULL;
			}
			
			if ( buf_list == NULL )
			{
				buf_list = as1;
			}
			else
			{
				PolizItem* buf_first = buf_list;
				while ( buf_list->next != NULL )
				{
					buf_list = buf_list->next;
				}

				buf_list->next = as1;
				as1->prev = buf_list;
				buf_list = buf_first;
			}

			ll_clear(&help_list, 1);
		}
		else if ( game_statement )
		{
			PolizItem* gs1 = TransformGameStatement(game_statement);
			if ( !gs1 )
			{
				ll_clear(&game_statement, 0);
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not a game statement!\n", list->data->num_str);
				*next_lexem = NULL;
				return NULL;
			}
			
			if ( buf_list == NULL )
			{
				buf_list = gs1;
			}
			else
			{
				PolizItem* buf_first = buf_list;
				while ( buf_list->next != NULL )
				{
					buf_list = buf_list->next;
				}

				buf_list->next = gs1;
				gs1->prev = buf_list;
				buf_list = buf_first;
			}

			ll_clear(&game_statement, 0);
		}
		
		help_list = NULL;
		rvalue = NULL;
		arithmetic_statement = NULL;
		game_statement = NULL;

		correct = 0;
		LexemList* l = list->next->next;
		
		if ( (l == NULL) || (l->data == NULL) )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);
			
			pi_clear(&buf_list, 1);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \",\" in \"%s\" operator!\n", list->data->num_str, game_operators[SELL_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		while ( (l != NULL) && (l->data != NULL) && (l->data->lexem[0] != ',') )
		{
			l = l->next;
		}

		if ( (l == NULL) || (l->data == NULL) )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);
			
			pi_clear(&buf_list, 1);
	
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \",\" in \"%s\" operator!\n", list->data->num_str, game_operators[SELL_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		if ( (l->next == NULL) || (l->next->data == NULL) )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);
			
			pi_clear(&buf_list, 1);
	
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[SELL_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		if ( isCorrectVarRValueStatement(l->next) )
		{
			correct = 1;
			rvalue = l->next;
		}
		else if ( isCorrectArithmeticStatement(l->next, &help_list) )
		{
			correct = 1;
			arithmetic_statement = help_list;
		}
		else if ( isCorrectGameStatement(l->next, &help_list) )
		{
			correct = 1;
			ll_insert(&game_statement, l->next->data);
			if ( help_list != NULL )
			{
				game_statement->next = help_list;
				help_list->prev = game_statement;
			}
		}
	
		if ( !correct )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);

			pi_clear(&buf_list, 1);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected CORRECT \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[SELL_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}

		if ( (l->next->next == NULL) || (l->next->next->data == NULL) )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);

			pi_clear(&buf_list, 1);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \";\" in \"%s\" operator!\n", list->data->num_str, game_operators[SELL_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}

		if ( l->next->next->data->lexem[0] != ';' )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);

			pi_clear(&buf_list, 1);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \";\" in \"%s\" operator!\n", list->data->num_str, game_operators[SELL_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		LexemList* ll = l->next->next;
		*next_lexem = ll->next;

		if ( rvalue )
		{
			PolizElem* elem = TransformRVal(rvalue);
			if ( !elem )
			{
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - variable (%s) is undeclared in program!\n", list->data->num_str, rvalue->data->lexem);
				*next_lexem = NULL;
				return NULL;
			}
			
			PolizInt* rval_int = dynamic_cast<PolizInt*>(elem);
			PolizVarAddr* rval_var = dynamic_cast<PolizVarAddr*>(elem);
			if ( rval_int )
			{
				pi_insert(&ret_list, rval_int);
			}
			else if ( rval_var )
			{
				pi_insert(&ret_list, rval_var);
			}

			PolizItem* buf_list_first = buf_list; 
			while ( buf_list->next != NULL )
			{
				buf_list = buf_list->next;
			}

			while ( buf_list != NULL )
			{
				pi_insert(&ret_list, buf_list->p);

				buf_list = buf_list->prev;
			}
			buf_list = buf_list_first;
			
			pi_clear(&buf_list, 0);

			pi_insert(&ret_list, new PolizSell());
			
			return ret_list;
		}
		
		if ( arithmetic_statement )
		{
			PolizItem* ret = TransformArithmetic(arithmetic_statement);
			if ( !ret )
			{
				ll_clear(&help_list, 1);
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not an arithmetic expression!\n", list->data->num_str);
				*next_lexem = NULL;
				return NULL;
			}
			
			if ( buf_list == NULL )
			{
				buf_list = ret;
			}
			else
			{
				PolizItem* buf_first = buf_list;
				while ( buf_list->next != NULL )
				{
					buf_list = buf_list->next;
				}

				buf_list->next = ret;
				ret->prev = buf_list;
				buf_list = buf_first;
			}

			PolizItem* buf_list_first = buf_list; 
			while ( buf_list->next != NULL )
			{
				buf_list = buf_list->next;
			}

			while ( buf_list != NULL )
			{
				pi_insert(&ret_list, buf_list->p);

				buf_list = buf_list->prev;
			}
			buf_list = buf_list_first;
			
			pi_clear(&buf_list, 0);		

			pi_insert(&ret_list, new PolizSell() );
		
			ll_clear(&help_list, 1);

			return ret_list;
		}
		
		if ( game_statement )
		{
			PolizItem* ret = TransformGameStatement(game_statement);
			if ( !ret )
			{
				ll_clear(&game_statement, 0);
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not a game statement!\n", list->data->num_str);
				*next_lexem = NULL;
				return NULL;
			}
			
			if ( buf_list == NULL )
			{
				buf_list = ret;
			}
			else
			{
				PolizItem* buf_first = buf_list;
				while ( buf_list->next != NULL )
				{
					buf_list = buf_list->next;
				}

				buf_list->next = ret;
				ret->prev = buf_list;
				buf_list = buf_first;
			}

			PolizItem* buf_list_first = buf_list; 
			while ( buf_list->next != NULL )
			{
				buf_list = buf_list->next;
			}
	

			while ( buf_list != NULL )
			{
				if ( isSimpleGF(buf_list) )
				{
					if ( !isSimpleGF(buf_list->next) )
					{
						pi_insert(&ret_list, buf_list->p);
						if ( (buf_list->next != NULL) && (buf_list->next->p != NULL) )
							pi_insert(&ret_list, buf_list->next->p);
						
						buf_list = buf_list->prev;
						continue;
					}
					pi_insert(&ret_list, buf_list->p);
				}
	
				if ( dynamic_cast<PolizConst*>(buf_list->p) != NULL )
				{
					pi_insert(&ret_list, buf_list->p);
				}
				buf_list = buf_list->prev;
			}
			buf_list = buf_list_first;
			
			pi_clear(&buf_list, 0);		

			pi_insert(&ret_list, new PolizSell() );
		
			ll_clear(&game_statement, 0);

			return ret_list;
		}

		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[SELL_OPERATOR]);
		*next_lexem = NULL;
		return NULL;
	}

	if ( strcmp(game_operators[BUY_OPERATOR], list->data->lexem) == 0 )
	{
		int correct = 0;

		if ( (list->next == NULL) || (list->next->data == NULL) )
		{
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[BUY_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}


		LexemList* rvalue = NULL;
		LexemList* arithmetic_statement = NULL;
		LexemList* game_statement = NULL;
		LexemList* help_list = NULL;

		if ( isCorrectVarRValueStatement(list->next) )
		{
			correct = 1;
			rvalue = list->next;
		}
		else if ( isCorrectArithmeticStatement(list->next, &help_list) )
		{
			correct = 1;
			arithmetic_statement = help_list;
		}
		else if ( isCorrectGameStatement(list->next, &help_list) )
		{
			correct = 1;
			ll_insert(&game_statement, list->next->data);
			if ( help_list != NULL )
			{
				game_statement->next = help_list;
				help_list->prev = game_statement;
			}
		}
	
		if ( !correct )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected CORRECT \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[BUY_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}

		if ( rvalue )
		{
			PolizElem* elem = TransformRVal(rvalue);
			if ( !elem )
			{
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - variable (%s) is undeclared in program!\n", list->data->num_str, rvalue->data->lexem);
				*next_lexem = NULL;
				return NULL;
			}
			
			PolizInt* rval_int = dynamic_cast<PolizInt*>(elem);
			PolizVarAddr* rval_var = dynamic_cast<PolizVarAddr*>(elem);
			if ( rval_int )
			{
				pi_insert(&ret_list, rval_int);
			}
			else if ( rval_var )
			{
				pi_insert(&ret_list, rval_var);
			}
		}
		else if ( arithmetic_statement )
		{
			PolizItem* as1 = TransformArithmetic(arithmetic_statement);
			if ( !as1 )
			{
				ll_clear(&help_list, 1);
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not an arithmetic expression!\n", list->data->num_str);
				*next_lexem = NULL;
				return NULL;
			}

			if ( buf_list == NULL )
			{
				buf_list = as1;
			}
			else
			{
				PolizItem* buf_first = buf_list;
				while ( buf_list->next != NULL )
				{
					buf_list = buf_list->next;
				}

				buf_list->next = as1;
				as1->prev = buf_list;
				buf_list = buf_first;
			}

			ll_clear(&help_list, 1);
		}
		else if ( game_statement )
		{
			PolizItem* gs1 = TransformGameStatement(game_statement);
			if ( !gs1 )
			{
				ll_clear(&game_statement, 0);
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not a game statement!\n", list->data->num_str);
				*next_lexem = NULL;
				return NULL;
			}
			
			if ( buf_list == NULL )
			{
				buf_list = gs1;
			}
			else
			{
				PolizItem* buf_first = buf_list;
				while ( buf_list->next != NULL )
				{
					buf_list = buf_list->next;
				}

				buf_list->next = gs1;
				gs1->prev = buf_list;
				buf_list = buf_first;
			}		

			ll_clear(&game_statement, 0);
		}
		
		help_list = NULL;
		rvalue = NULL;
		arithmetic_statement = NULL;
		game_statement = NULL;

		correct = 0;
		LexemList* l = list->next->next;
		
		if ( (l == NULL) || (l->data == NULL) )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);
			
			pi_clear(&buf_list, 1);
	
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \",\" in \"%s\" operator!\n", list->data->num_str, game_operators[BUY_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		while ( (l != NULL) && (l->data != NULL) && (l->data->lexem[0] != ',') )
		{
			l = l->next;
		}

		if ( (l == NULL) || (l->data == NULL) )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);
			
			pi_clear(&buf_list, 1);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \",\" in \"%s\" operator!\n", list->data->num_str, game_operators[BUY_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		if ( (l->next == NULL) || (l->next->data == NULL) )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);
			
			pi_clear(&buf_list, 1);
	
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[BUY_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}
		
		if ( isCorrectVarRValueStatement(l->next) )
		{
			correct = 1;
			rvalue = l->next;
		}
		else if ( isCorrectArithmeticStatement(l->next, &help_list) )
		{
			correct = 1;
			arithmetic_statement = help_list;
		}
		else if ( isCorrectGameStatement(l->next, &help_list) )
		{
			correct = 1;
			ll_insert(&game_statement, l->next->data);
			if ( help_list != NULL)
			{
				game_statement->next = help_list;
				help_list->prev = game_statement;
			}
		}
	
		if ( !correct )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);
			
			pi_clear(&buf_list, 1);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected CORRECT \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[BUY_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}

		if ( (l->next->next == NULL) || (l->next->next->data == NULL) )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);

			pi_clear(&buf_list, 1);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \";\" in \"%s\" operator!\n", list->data->num_str, game_operators[BUY_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}

		if ( l->next->next->data->lexem[0] != ';' )
		{
			if ( arithmetic_statement )
				ll_clear(&help_list, 1);
			else if ( game_statement )
				ll_clear(&game_statement, 0);

			pi_clear(&buf_list, 1);

			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \";\" in \"%s\" operator!\n", list->data->num_str, game_operators[BUY_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}

		LexemList* ll = l->next->next;
		*next_lexem = ll->next;
		
		if ( rvalue )
		{
			PolizElem* elem = TransformRVal(rvalue);
			if ( !elem )
			{
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - variable (%s) is undeclared in program!\n", list->data->num_str, rvalue->data->lexem);
				*next_lexem = NULL;
				return NULL;
			}
			
			PolizInt* rval_int = dynamic_cast<PolizInt*>(elem);
			PolizVarAddr* rval_var = dynamic_cast<PolizVarAddr*>(elem);
			if ( rval_int )
			{
				pi_insert(&ret_list, rval_int);
			}
			else if ( rval_var )
			{
				pi_insert(&ret_list, rval_var);
			}

			PolizItem* buf_list_first = buf_list; 
			while ( buf_list->next != NULL )
			{
				buf_list = buf_list->next;
			}

			while ( buf_list != NULL )
			{
				pi_insert(&ret_list, buf_list->p);

				buf_list = buf_list->prev;
			}
			buf_list = buf_list_first;
			
			pi_clear(&buf_list, 0);

			pi_insert(&ret_list, new PolizBuy());

			return ret_list;
		}

		if ( arithmetic_statement )
		{
			PolizItem* ret = TransformArithmetic(arithmetic_statement);
			if ( !ret )
			{
				ll_clear(&help_list, 1);
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not an arithmetic expression!\n", list->data->num_str);
				*next_lexem = NULL;
				return NULL;
			}

			if ( buf_list == NULL )
			{
				buf_list = ret;
			}
			else
			{
				PolizItem* buf_first = buf_list;
				while ( buf_list->next != NULL )
				{
					buf_list = buf_list->next;
				}

				buf_list->next = ret;
				ret->prev = buf_list;
				buf_list = buf_first;
			}

			PolizItem* buf_list_first = buf_list; 
			while ( buf_list->next != NULL )
			{
				buf_list = buf_list->next;
			}

			while ( buf_list != NULL )
			{
				pi_insert(&ret_list, buf_list->p);

				buf_list = buf_list->prev;
			}
			buf_list = buf_list_first;
			
			pi_clear(&buf_list, 0);		

			pi_insert(&ret_list, new PolizBuy() );

			ll_clear(&help_list, 1);

			return ret_list;
		}

		if ( game_statement )
		{
			PolizItem* ret = TransformGameStatement(game_statement);
			if ( !ret )
			{
				ll_clear(&game_statement, 0);
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not a game statement!\n", list->data->num_str);
				*next_lexem = NULL;
				return NULL;
			}
		
			if ( buf_list == NULL )
			{
				buf_list = ret;
			}
			else
			{
				PolizItem* buf_first = buf_list;
				while ( buf_list->next != NULL )
				{
					buf_list = buf_list->next;
				}

				buf_list->next = ret;
				ret->prev = buf_list;
				buf_list = buf_first;
			}

			PolizItem* buf_list_first = buf_list; 
			while ( buf_list->next != NULL )
			{
				buf_list = buf_list->next;
			}
			

			while ( buf_list != NULL )
			{
				if ( isSimpleGF(buf_list) )
				{
					if ( !isSimpleGF(buf_list->next) )
					{
						pi_insert(&ret_list, buf_list->p);
						if ( (buf_list->next != NULL) && (buf_list->next->p != NULL) )
							pi_insert(&ret_list, buf_list->next->p);
						
						buf_list = buf_list->prev;
						continue;
					}	
					pi_insert(&ret_list, buf_list->p);
				}	
				
				if ( dynamic_cast<PolizConst*>(buf_list->p) != NULL )
				{
					pi_insert(&ret_list, buf_list->p);
				}
				buf_list = buf_list->prev;
			}
			buf_list = buf_list_first;
			
			pi_clear(&buf_list, 0);		

			pi_insert(&ret_list, new PolizBuy() );
		
			ll_clear(&game_statement, 0);		

			return ret_list;
		}

		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[BUY_OPERATOR]);
		*next_lexem = NULL;
		return NULL;
	}

	if ( strcmp(game_operators[PRINT_OPERATOR], list->data->lexem) == 0 )
	{
		LexemList* l = list->next;

		if ( (l == NULL) || (l->data == NULL) )
		{
			fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected string, \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[PRINT_OPERATOR]);
			*next_lexem = NULL;
			return NULL;
		}

		while ( (l != NULL) && (l->data != NULL) )
		{
			LexemList* rvalue = NULL;
			LexemList* arithmetic_statement = NULL;
			LexemList* game_statement = NULL;
			LexemList* help_list = NULL;
			int correct = 0;

			int len = strlen(l->data->lexem);
			if ( (l->data->lexem[0] == '"') && (l->data->lexem[len-1] == '"') )
			{
				pi_insert(&buf_list, new PolizString(l->data->lexem, l->data->size));
				l = l->next;
				continue;
			}
			else if ( isCorrectVarRValueStatement(l) )
			{
				correct = 1;
				rvalue = l;
			}
			else if ( isCorrectArithmeticStatement(l, &help_list) )
			{
				correct = 1;
				arithmetic_statement = help_list;
			}
			else if ( isCorrectGameStatement(l, &help_list) )
			{
				correct = 1;
				ll_insert(&game_statement, l->data);
				if ( help_list != NULL )
				{
					game_statement->next = help_list;
					help_list->prev = game_statement;
				}
			}
			else if ( l->data->lexem[0] == ',' )
			{
				l = l->next;
				continue;
			}
			else if ( l->data->lexem[0] == ';' )
			{
				if ( buf_list == NULL )
				{
					if ( arithmetic_statement )
						ll_clear(&help_list, 1);
					else if ( game_statement )
						ll_clear(&game_statement, 0);

					pi_clear(&buf_list, 1);

					fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected CORRECT \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[PRINT_OPERATOR]);
					*next_lexem = NULL;
					return NULL;
				}

				*next_lexem = l->next;
				
				PolizItem* buf_list_first = buf_list; 
				while ( buf_list->next != NULL )
				{
					buf_list = buf_list->next;
				}

				while ( buf_list != NULL )
				{
					pi_insert(&ret_list, buf_list->p);

					buf_list = buf_list->prev;
				}
				buf_list = buf_list_first;
				
				pi_clear(&buf_list, 0);

				pi_insert(&ret_list, new PolizPrint());
				return ret_list;
			}

			if ( !correct )
			{
				if ( arithmetic_statement )
					ll_clear(&help_list, 1);
				else if ( game_statement )
					ll_clear(&game_statement, 0);
				
				pi_clear(&buf_list, 1);

				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected CORRECT \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[PRINT_OPERATOR]);
				*next_lexem = NULL;
				return NULL;
			}
						
			if ( (l->next == NULL) || (l->next->data == NULL) )
			{
				if ( arithmetic_statement )
					ll_clear(&help_list, 1);
				else if ( game_statement )
					ll_clear(&game_statement, 0);

				pi_clear(&buf_list, 1);

				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \";\" or \",\" in \"%s\" operator!\n", list->data->num_str, game_operators[PRINT_OPERATOR]);
				*next_lexem = NULL;
				return NULL;
			}

			if ( (l->next->data->lexem[0] != ';') && (l->next->data->lexem[0] != ',') )
			{
				if ( arithmetic_statement )
					ll_clear(&help_list, 1);
				else if ( game_statement )
					ll_clear(&game_statement, 0);

				pi_clear(&buf_list, 1);

				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \";\" or \",\" in \"%s\" operator!\n", list->data->num_str, game_operators[PRINT_OPERATOR]);
				*next_lexem = NULL;
				return NULL;
			}
			
			if ( rvalue )
			{
				PolizElem* elem = TransformRVal(rvalue);
				if ( !elem )
				{
					fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - variable (%s) is undeclared in program!\n", list->data->num_str, rvalue->data->lexem);
					*next_lexem = NULL;
					return NULL;
				}
				
				PolizInt* rval_int = dynamic_cast<PolizInt*>(elem);
				PolizVarAddr* rval_var = dynamic_cast<PolizVarAddr*>(elem);
				if ( rval_int )
				{
					pi_insert(&ret_list, rval_int);
				}
				else if ( rval_var )
				{
					pi_insert(&ret_list, rval_var);
				}
			}
			else if ( arithmetic_statement )
			{
				PolizItem* ret = TransformArithmetic(arithmetic_statement);
				if ( !ret )
				{
					ll_clear(&help_list, 1);
					fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not an arithmetic expression!\n", list->data->num_str);
					*next_lexem = NULL;
					return NULL;
				}

				if ( buf_list == NULL )
				{
					buf_list = ret;
				}
				else
				{
					PolizItem* buf_first = buf_list;
					while ( buf_list->next != NULL )
					{
						buf_list = buf_list->next;
					}
					
					buf_list->next = ret;
					ret->prev = buf_list;
					buf_list = buf_first;
				}

				ll_clear(&help_list, 1);
			}
			else if ( game_statement )
			{
				PolizItem* ret = TransformGameStatement(game_statement);
				if ( !ret )
				{
					ll_clear(&game_statement, 0);
					fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expression is not a game statement!\n", list->data->num_str);
					*next_lexem = NULL;
					return NULL;
				}
			
				if ( buf_list == NULL )
				{
					buf_list = ret;
				}
				else
				{
					PolizItem* buf_first = buf_list;
					while ( buf_list->next != NULL )
					{
						buf_list = buf_list->next;
					}
					
					buf_list->next = ret;
					ret->prev = buf_list;
					buf_list = buf_first;
				}			

				ll_clear(&game_statement, 0);
			}
			else
			{
				fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[PRINT_OPERATOR]);
				*next_lexem = NULL;
				return NULL;
			}

			l = l->next;
		}
		
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected string, \"rvalue\", arithmetic or game statement in \"%s\" operator!\n", list->data->num_str, game_operators[PRINT_OPERATOR]);
		*next_lexem = NULL;
		return NULL;
	}

	fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected game operator!\n", list->data->num_str);
	*next_lexem = NULL;
	return NULL;
}

PolizItem* SyntaxAnalyzer::Run(LexemList** lex_list_ptr)
{
	if ( (lex_list_ptr == NULL) || (*lex_list_ptr == NULL) )
	{
		fprintf(stderr, "%s", "\n[SyntaxAnalyzer]: Line (1) - expected label, variable, \"if\" or game operator!\n");
		return NULL;
	}

	LexemList* list = *lex_list_ptr;
	int start_lexem = 1;
	int game_op_idx = -1;

	while ( (list != NULL) && (list->data != NULL) )
	{
		int len = strlen(list->data->lexem);
		if ( (list->data->lexem[0] == '@') && ( len > 1 ) )
		{
			PolizItem* label_operator = NULL;
			LexemList* next_lexem = NULL;
			if ( ( label_operator = isCorrectLabelOperator(list, &next_lexem) ) == NULL )
			{
				if ( this->operators_list )
					pi_clear(&this->operators_list, 1);

				return NULL;
			}
			
			PolizItem* op_list = this->operators_list;
			while ( (op_list != NULL) && (op_list->next != NULL) )
				op_list = op_list->next;
			
			if ( op_list == NULL)
				this->operators_list = label_operator;
			else 
				op_list->next = label_operator;
			
			label_operator->prev = op_list;

			list = next_lexem;
			
			if ( start_lexem )
				start_lexem = 0;
			
			/*pi_print(this->operators_list);*/

			continue;
		}
		
		if ( (list->data->lexem[0] == '$') && ( len > 1 ) )
		{
			PolizItem* assign_operator = NULL;
			LexemList* next_lexem = NULL;
			if ( ( assign_operator = isCorrectAssignOperator(list, &next_lexem) ) == NULL )
			{
				if ( this->operators_list )
					pi_clear(&this->operators_list, 1);
				
				return NULL;
			}
			
			PolizItem* op_list = this->operators_list;
			while ( (op_list != NULL) && (op_list->next != NULL) )
				op_list = op_list->next;
			
			if ( op_list == NULL )
				this->operators_list = assign_operator;
			else 
				op_list->next = assign_operator;
			
			assign_operator->prev = op_list;

			list = next_lexem;
			
			if ( start_lexem )
				start_lexem = 0;

			/*pi_print(this->operators_list);*/
			
			continue;
		}
		
		if ( strcmp(list->data->lexem, "if") == 0 )
		{
			PolizItem* if_operator = NULL;
			LexemList* next_lexem = NULL;
			if ( ( if_operator = isCorrectIfOperator(list, &next_lexem) ) == NULL )
			{
				if ( this->operators_list )
					pi_clear(&this->operators_list, 1);
	
				return NULL;
			}
			
			PolizItem* op_list = this->operators_list;
			while ( (op_list != NULL) && (op_list->next != NULL) )
				op_list = op_list->next;
			
			if ( op_list == NULL)
				this->operators_list = if_operator;
			else
				op_list->next = if_operator;
			
			if_operator->prev = op_list;

			list = next_lexem;
			
			if ( start_lexem )
				start_lexem = 0;
			
			/*pi_print(this->operators_list);*/

			continue;
		}
		
		if ( (game_op_idx = isValidGameOperator(list->data->lexem)) > -1 )
		{
			PolizItem* game_operator = NULL;
			LexemList* next_lexem = NULL;
			if ( ( game_operator = isCorrectGameOperator(list, &next_lexem) ) == NULL )
			{
				if ( this->operators_list )
					pi_clear(&this->operators_list, 1);
	
				return NULL;
			}

			PolizItem* op_list = this->operators_list;
			while ( (op_list != NULL) && (op_list->next != NULL) )
				op_list = op_list->next;
			
			if ( op_list == NULL)
				this->operators_list = game_operator;
			else
				op_list->next = game_operator;
			
			game_operator->prev = op_list;

			list = next_lexem;
			
			if ( start_lexem )
				start_lexem = 0;
			
			/*pi_print(this->operators_list);*/

			continue;
		}
		
		if ( strcmp(list->data->lexem, "goto") == 0 )
		{
			PolizItem* goto_operator = NULL;
			LexemList* next_lexem = NULL;
			if ( start_lexem || ( (goto_operator = isCorrectGoOperator(list, &next_lexem)) == NULL ) )
			{
				if ( this->operators_list )
					pi_clear(&this->operators_list, 1);
				
				return NULL;
			}
			
			PolizItem* op_list = this->operators_list;
			while ( (op_list != NULL) && (op_list->next != NULL) )
				op_list = op_list->next;
			
			if ( op_list == NULL )
				this->operators_list = goto_operator;
			else
				op_list->next = goto_operator;
			
			goto_operator->prev = op_list;
			
			list = next_lexem;
			
			if ( start_lexem )
				start_lexem = 0;
			
			/*pi_print(this->operators_list);*/

			continue;
		}
		
		if ( this->operators_list )
			pi_clear(&this->operators_list, 1);
	
		fprintf(stderr, "\n[SyntaxAnalyzer]: Line (%d) - expected label, variable, \"if\" or game operator!\n", list->data->num_str);
		return NULL;
	}
	
	return this->operators_list;
}

#endif
