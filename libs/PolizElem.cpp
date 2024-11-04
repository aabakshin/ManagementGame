/* Файл реализации модуля PolizElem */

#ifndef POLIZ_ELEM_CPP
#define POLIZ_ELEM_CPP

#ifndef __CPP_FILE__
#define __CPP_FILE__
#endif

#include "../includes/SystemHeaders.h"

extern "C"
{
	#include "../includes/MGLib.h"
}

#include "../includes/MainInfo.hpp"
#include "../includes/PolizElem.hpp"
#include "../includes/PolizEx.hpp"


/* Описана в модуле SA_Additional */
extern PolizItem* vars_list;

/* Описана в модуле bot_mg */
extern MainInfo main_info;

/* Если результат вычисления выражения/оператора зависит от данных, полученных от сервера после выполения этого оператора, то этот флаг устанавливается, 
 * чтобы не произошло ложного перехода к следующей команде */
static int ret_to_prev_cmd = 0;

/* Системная константа */
enum
{
	DATA_BUFFER_SIZE = 1024
};

void PolizElem::Push(PolizItem** stack, PolizElem* elem)
{
	if ( (stack == NULL) || (elem == NULL) )
	{
		fprintf(stderr, "%s", "\n[PolizElem]: push error\n");
		return;
	}
	
	PolizItem* newPtr = NULL;
	newPtr = new PolizItem;
	if ( !newPtr )
	{
		fprintf(stderr, "%s", "\n[PolizElem]: memory error\n");
		return;
	}
	newPtr->p = elem;
	newPtr->next = NULL;
	newPtr->prev = NULL;

	if ( *stack == NULL )
	{
		*stack = newPtr;
		return;
	}

	newPtr->next = *stack;
	(*stack)->prev = newPtr;
	*stack = newPtr;
}

PolizElem* PolizElem::Pop(PolizItem** stack)
{
	if ( ( stack == NULL ) || ( *stack == NULL ) )
	{
		if ( stack == NULL )
			fprintf(stderr, "%s", "\n[PolizElem]: pop error\n");
		
		return NULL;
	}

	PolizElem* popValue = NULL;
	PolizItem* tempPtr = NULL;

	tempPtr = *stack;
	*stack = (*stack)->next;
	if ( *stack != NULL )
		(*stack)->prev = NULL;
	popValue = tempPtr->p;
	delete tempPtr;

	return popValue;
}

int PolizElem::Size(PolizItem** stack)
{
	if ( (stack == NULL) || (*stack == NULL) )
	{
		return 0;
	}

	int size = 0;
	PolizItem* s = *stack;
	while ( s != NULL )
	{
		size++;
		s = s->next;
	}
	
	return size;
}

int PolizElem::Clear(PolizItem** stack)
{
	if ( (stack == NULL) || (*stack == NULL) )
	{
		return 0;
	}
	
	int size = Size(stack);
	int popped_count = 0;

	PolizElem* popped = NULL;
	do
	{
		popped = Pop(stack);
		if ( popped )
		{
			delete popped;
			popped_count++;
		}
	}
	while ( popped != NULL );

	if ( popped_count == size )
	{
		return 1;
	}

	return 0;
}

void PolizConst::Evaluate(PolizItem** stack, PolizItem** cur_cmd)
{
	Push(stack, Clone());
	*cur_cmd = (*cur_cmd)->next;
}

PolizString::PolizString(char* a, int size)
{
	if ( size < 1 )
	{
		this->value = NULL;
		this->size = 0;
		return;
	}
	
	this->value = new char[size];
	if ( !value )
	{
		this->size = 0;
		fprintf(stderr, "%s", "\n[PolizString]: PolizString() memory error\n");
		return;
	}
	this->size = size;

	int i;
	for ( i = 0; i < size-1; i++ )
		this->value[i] = a[i];
	this->value[i] = '\0';
}

PolizString::PolizString(const PolizString& a)
{
	if ( (a.value == NULL) || (a.size < 1) )
	{
		this->value = NULL;
		this->size = 0;
	}

	this->value = new char[a.size];
	if ( !this->value )
	{
		this->size = 0;
		fprintf(stderr, "%s", "\n[PolizString]: PolizString() memory error\n");
		return;
	}
	this->size = a.size;
	
	int i;
	for ( i = 0; i < a.size-1; i++ )
		this->value[i] = a.value[i];
	this->value[i] = '\0';
}

PolizString& PolizString::operator=(const PolizString& a)
{
	if ( (a.value == NULL) || (a.size < 1) )
		return *this;

	delete[] this->value;
	this->value = new char[a.size];
	if ( !this->value )
	{
		this->size = 0;
		fprintf(stderr, "%s", "\n[PolizString]: PolizString() operator= memory error\n");
		return *this;
	}
	this->size = a.size;
	
	int i;
	for ( i = 0; i < a.size-1; i++ )
		this->value[i] = a.value[i];
	this->value[i] = '\0';

	return *this;
}

PolizString::~PolizString()
{
	if ( value )
		delete[] value;
}

PolizInt::PolizInt(const PolizInt& a)
{
	this->value = a.Get();
}

PolizInt& PolizInt::operator=(const PolizInt& a)
{
	this->value = a.Get();
	return *this;
}

PolizVarAddr::PolizVarAddr(const PolizVarAddr& a)
{
	this->value = a.Get();
}

PolizVarAddr& PolizVarAddr::operator=(const PolizVarAddr& a)
{
	this->value = a.Get();
	return *this;
}

PolizLabel::PolizLabel(const PolizLabel& a)
{
	this->value = a.Get();
}



void PolizOpGo::Evaluate(PolizItem** stack, PolizItem** cur_cmd)
{
	PolizElem* operand1 = Pop(stack);
	
	PolizLabel* lab = dynamic_cast<PolizLabel*>(operand1);
	if ( !lab ) 
		throw PolizExNotLabel(operand1);
	
	PolizItem* addr = lab->Get();
	*cur_cmd = addr;

	delete operand1;
}

void PolizOpIfThen::Evaluate(PolizItem** stack, PolizItem** cur_cmd)
{
	PolizElem* operand1 = Pop(stack);
	PolizInt* int1 = dynamic_cast<PolizInt*>(operand1);
	if ( !int1 )
		throw PolizExNotInt(operand1);

	if ( int1->Get() != 0 )
	{
		*cur_cmd = (*cur_cmd)->next;
		delete operand1;
		return;
	}
	
	while ( (*cur_cmd != NULL) && ((*cur_cmd)->p != NULL) )
	{
		if ( 
				( dynamic_cast<PolizOpGo*>((*cur_cmd)->p)		!=	NULL )		||
				( dynamic_cast<PolizAssign*>((*cur_cmd)->p)		!=	NULL )		||
				( dynamic_cast<PolizPrint*>((*cur_cmd)->p)		!=	NULL )		||
				( dynamic_cast<PolizPrint*>((*cur_cmd)->p)		!=	NULL )		||
				( dynamic_cast<PolizBuy*>((*cur_cmd)->p)		!=	NULL )		||
				( dynamic_cast<PolizSell*>((*cur_cmd)->p)		!=	NULL )		||
				( dynamic_cast<PolizProd*>((*cur_cmd)->p)		!=	NULL )		||
				( dynamic_cast<PolizBuild*>((*cur_cmd)->p)		!=	NULL )		||
				( dynamic_cast<PolizEndturn*>((*cur_cmd)->p)	!=	NULL )
		   )
		{
			break;
		}

		*cur_cmd = (*cur_cmd)->next;
	}

	if ( *cur_cmd != NULL )
		*cur_cmd = (*cur_cmd)->next;

	delete operand1;
}


void PolizFunction::Evaluate(PolizItem** stack, PolizItem** cur_cmd)
{
	PolizElem* res = EvaluateFun(stack);
	if ( res ) 
		Push(stack, res);

	if ( !ret_to_prev_cmd )
		*cur_cmd = (*cur_cmd)->next;
}

PolizVar::PolizVar(const char* var_name, int var_name_size, PolizInt* var_value )
{
	if ( (var_name == NULL) || (var_name_size < 1) || (*var_name == '\0') || (var_value == NULL) ) 
	{
		fprintf(stderr, "%s", "\n[PolizVar]: PolizVar error\n");
		return;
	}
	
	this->var_name = new char[var_name_size];
	if ( !this->var_name )
	{
		fprintf(stderr, "%s", "\n[PolizVar]: PolizVar memory error\n");
		return;
	}

	int i;
	for ( i = 0; i < var_name_size-1; i++ )
		this->var_name[i] = var_name[i];
	this->var_name[i] = '\0';

	this->var_name_size = var_name_size;
	this->var_value = var_value->Get();
	this->var_addr = 0;

	pi_insert(&vars_list, this);
	PolizItem* res = pi_find(vars_list, this);
	this->var_addr = new PolizVarAddr(res->elem_number);
}

PolizVar::~PolizVar()
{
	if ( this->var_name )
		delete[] this->var_name;
	
	pi_delete(&vars_list, this);

	delete this->var_addr;
}

void PolizVar::SetVarName(const char* var_name, int var_name_size)
{
	if ( (var_name == NULL) || (var_name_size < 1) )
	{
		fprintf(stderr, "%s", "\n[PolizVar]: SetVarName error\n");
		return;
	}

	if ( this->var_name )
		delete[] this->var_name;
	this->var_name = NULL;

	this->var_name = new char[var_name_size];
	if ( !this->var_name )
	{
		fprintf(stderr, "%s", "\n[PolizVar]: SetVarName memory error\n");
		return;
	}
	this->var_name_size = var_name_size;

	int i;
	for ( i = 0; i < var_name_size-1; i++ )
		this->var_name[i] = var_name[i];
	this->var_name[i] = '\0';
}

PolizElem* PolizVar::EvaluateFun(PolizItem** stack)
{
	PolizItem* res = pi_find(vars_list, this);
	if ( !res )
	{
		fprintf(stderr, "%s", "\n[PolizVar]: EvaluateRun pi_find error\n");
		return NULL;
	};

	PolizVar* var = dynamic_cast<PolizVar*>(res->p);
	if ( !var )
		throw PolizExNotVar(res->p);
	
	return new PolizInt(var->GetVarValue());
}

PolizElem* PolizAssign::EvaluateFun(PolizItem** stack)
{
	PolizElem* operand1 = Pop(stack);
	PolizVarAddr* var_addr = dynamic_cast<PolizVarAddr*>(operand1);
	if ( !var_addr )
		throw PolizExNotVar(operand1);

	PolizElem* operand2 = Pop(stack);
	PolizInt* integer = dynamic_cast<PolizInt*>(operand2);
	if ( !integer )
		throw PolizExNotInt(operand2);

	PolizItem* vl = vars_list;
	while ( (vl != NULL) && ( vl->p != NULL) )
	{
		PolizVar* var = dynamic_cast<PolizVar*>(vl->p);
		if ( var != NULL )
		{
			if ( var->GetVarAddr()->Get() == var_addr->Get() )
			{
				var->SetVarValue(integer->Get());
				break;
			}
		}

		vl = vl->next;
	}

	delete operand1;
	delete operand2;

	return NULL;
}

PolizElem* PolizGFMyId::EvaluateFun(PolizItem** stack)
{
	//printf("\n[PolizGFMyId]: main_info.my_id = %d\n", main_info.my_id);
	return new PolizInt(main_info.my_id);
}

PolizElem* PolizGFTurn::EvaluateFun(PolizItem** stack)
{
	//printf("\n[PolizGFTurn]: main_info.turn = %d\n", main_info.turn);
	return new PolizInt(main_info.turn);
}

PolizElem* PolizGFPlayers::EvaluateFun(PolizItem** stack)
{
	//printf("\n[PolizGFPlayers]: main_info.total_players = %d\n", main_info.total_players);
	return new PolizInt(main_info.total_players);
}

PolizElem* PolizGFAPlayers::EvaluateFun(PolizItem** stack)
{
	//printf("\n[PolizGFAPlayers]: main_info.alive_players = %d\n", main_info.alive_players);
	return new PolizInt(main_info.alive_players);
}

PolizElem* PolizGFSupply::EvaluateFun(PolizItem** stack)
{
	//printf("\n[PolizGFSupply]: main_info.cur_sources_buy = %d\n", main_info.cur_sources_buy);
	return new PolizInt(main_info.cur_sources_buy);
}

PolizElem* PolizGFRawPrice::EvaluateFun(PolizItem** stack)
{
	//printf("\n[PolizGFRawPrice]: main_info.cur_sources_min_price = %d\n", main_info.cur_sources_min_price);
	return new PolizInt(main_info.cur_sources_min_price);
}

PolizElem* PolizGFDemand::EvaluateFun(PolizItem** stack)
{
	//printf("\n[PolizGFDemand]: main_info.cur_products_sell = %d\n", main_info.cur_products_sell);
	return new PolizInt(main_info.cur_products_sell);
}

PolizElem* PolizGFProdPrice::EvaluateFun(PolizItem** stack)
{
	//printf("\n[PolizGFProdPrice]: main_info.cur_products_max_price = %d\n", main_info.cur_products_max_price);
	return new PolizInt(main_info.cur_products_max_price);
}

PolizElem* PolizFunction::EvalGameFunction(PolizItem** stack, const char* function_name)
{
	if ( (!ret_to_prev_cmd) && (*stack == NULL) )
	{
		fprintf(stderr, "\n[PolizElem]: In function: %s::EvaluateFun() stack is empty!\n", function_name);
		return NULL;
	}
	
	if ( ret_to_prev_cmd )
	{
		ret_to_prev_cmd = 0;
		
		int num = main_info.last_player_num;
		
		if ( strcmp(function_name, "PolizGFMoney") == 0 )
		{
			//printf("\n[PolizGFMoney]: main_info.p_info[%d]->money = %d\n", num-1, main_info.p_info[num-1]->money);
			return new PolizInt(main_info.p_info[num-1]->money);
		}

		if ( strcmp(function_name, "PolizGFRaw") == 0 )
		{
			//printf("\n[PolizGFRaw]: main_info.p_info[%d]->raw = %d\n", num-1, main_info.p_info[num-1]->raw);
			return new PolizInt(main_info.p_info[num-1]->raw);
		}

		if ( strcmp(function_name, "PolizGFProduction") == 0 )
		{
			//printf("\n[PolizGFProduction]: main_info.p_info[%d]->prod = %d\n", num-1, main_info.p_info[num-1]->prod);
			return new PolizInt(main_info.p_info[num-1]->prod);
		}

		if ( strcmp(function_name, "PolizGFFactories") == 0 )
		{
			//printf("\n[PolizGFFactories]: main_info.p_info[%d]->wait_fact (%d) + main_info.p_info[%d]->work_fact (%d) + main_info.p_info[%d]->build_fact (%d) = %d\n", num-1, main_info.p_info[num-1]->wait_fact, num-1, main_info.p_info[num-1]->work_fact, num-1, main_info.p_info[num-1]->build_fact, main_info.p_info[num-1]->wait_fact + main_info.p_info[num-1]->work_fact + main_info.p_info[num-1]->build_fact);
			return new PolizInt(main_info.p_info[num-1]->wait_fact + main_info.p_info[num-1]->work_fact + main_info.p_info[num-1]->build_fact);
		}
		if ( strcmp(function_name, "PolizGFManufactured") == 0 )
		{
			//printf("\n[PolizGFManufactured]: main_info.p_info[%d]->manufactured = %d\n", num-1, main_info.p_info[num-1]->manufactured);
			return new PolizInt(main_info.p_info[num-1]->manufactured);
		}

		if ( strcmp(function_name, "PolizGFResRawSold") == 0 )
		{
			//printf("\n[PolizGFResRawSold]: main_info.p_info[%d]->res_raw_sold = %d\n", num-1, main_info.p_info[num-1]->res_raw_sold);
			return new PolizInt(main_info.p_info[num-1]->res_raw_sold);
		}

		if ( strcmp(function_name, "PolizGFResRawPrice") == 0 )
		{
			//printf("\n[PolizGFResRawPrice]: main_info.p_info[%d]->res_raw_price = %d\n", num-1, main_info.p_info[num-1]->res_raw_price);
			return new PolizInt(main_info.p_info[num-1]->res_raw_price);
		}

		if ( strcmp(function_name, "PolizGFResProdBought") == 0 )
		{
			//printf("\n[PolizGFResProdBought]: main_info.p_info[%d]->res_prod_bought = %d\n", num-1, main_info.p_info[num-1]->res_prod_bought);
			return new PolizInt(main_info.p_info[num-1]->res_prod_bought);
		}

		if ( strcmp(function_name, "PolizGFResProdPrice") == 0 )
		{
			//printf("\n[PolizGFResProdPrice]: main_info.p_info[%d]->res_prod_price = %d\n", num-1, main_info.p_info[num-1]->res_prod_price);
			return new PolizInt(main_info.p_info[num-1]->res_prod_price);
		}

		fprintf(stderr, "\n[PolizElem]: In function EvalGameFunction() there is invalid game function!\n");
		return NULL;
	}

	PolizElem* popped = Pop(stack);
	PolizInt* int1 = dynamic_cast<PolizInt*>(popped);
	if ( !int1 )
	{
		fprintf(stderr, "\n[PolizElem]: In function: %s::EvaluateFun() popped element is not integer!\n", function_name);
		return NULL;
	}

	int player_number = int1->Get();
	char pl_num[10];
	itoa(player_number, pl_num, 9);

	const char* player_command = "player ";
	
	char send_buf[DATA_BUFFER_SIZE] = { 0 };
	int i, j;
	for ( i = 0; player_command[i]; i++ )
		send_buf[i] = player_command[i];
	for ( j = 0; pl_num[j]; j++, i++ )
		send_buf[i] = pl_num[j];
	send_buf[i] = '\n';
	send_buf[i+1] = '\0';

	int wc = write(main_info.fd, send_buf, i+1);
	printf("\nSent %d\\%d bytes\n", wc, i+1);
	
	for ( i = 0; send_buf[i]; i++ )
	{
		printf("%c ", send_buf[i]);
		if ( ((i+1) % 20) == 0 )
			putchar('\n');
	}
	putchar('\n');

	for ( i = 0; send_buf[i]; i++ )
	{
		printf("%d ", send_buf[i]);
		if ( ((i+1) % 20) == 0 )
			putchar('\n');
	}
	putchar('\n');


	ret_to_prev_cmd = 1;
	
	delete popped;

	return NULL;
}

PolizElem* PolizGFMoney::EvaluateFun(PolizItem** stack)
{
	return EvalGameFunction(stack, "PolizGFMoney");
}

PolizElem* PolizGFRaw::EvaluateFun(PolizItem** stack)
{
	return EvalGameFunction(stack, "PolizGFRaw");
}

PolizElem* PolizGFProduction::EvaluateFun(PolizItem** stack)
{
	return EvalGameFunction(stack, "PolizGFProduction");
}

PolizElem* PolizGFFactories::EvaluateFun(PolizItem** stack)
{
	return EvalGameFunction(stack, "PolizGFFactories");
}

PolizElem* PolizGFManufactured::EvaluateFun(PolizItem** stack)
{
	return EvalGameFunction(stack, "PolizGFManufactured");
}

PolizElem* PolizGFResRawSold::EvaluateFun(PolizItem** stack)
{
	return EvalGameFunction(stack, "PolizGFResRawSold");
}

PolizElem* PolizGFResRawPrice::EvaluateFun(PolizItem** stack)
{
	return EvalGameFunction(stack, "PolizGFResRawPrice");
}

PolizElem* PolizGFResProdBought::EvaluateFun(PolizItem** stack)
{
	return EvalGameFunction(stack, "PolizGFResProdBought");
}

PolizElem* PolizGFResProdPrice::EvaluateFun(PolizItem** stack)
{
	return EvalGameFunction(stack, "PolizGFResProdPrice");
}

PolizElem* PolizPrint::EvaluateFun(PolizItem** stack)
{
	if ( *stack == NULL )
	{
		fprintf(stderr, "%s", "\n[PolizElem]: In function: PolizPrint::EvaluateFun() stack is empty!\n");
		return NULL;
	}

	PolizItem* list = NULL;

	PolizElem* elem = NULL;
	while ( (elem = Pop(stack)) != NULL )
	{
		pi_insert(&list, elem);
	}
	
	putchar('\n');
	PolizItem* list_c = list;
	while ( list_c != NULL )
	{
		PolizInt* integer = dynamic_cast<PolizInt*>(list_c->p); 
		if ( integer != NULL )
		{
			printf("%d ", integer->Get());
			list_c = list_c->next;
			continue;
		}
		
		PolizString* str = dynamic_cast<PolizString*>(list_c->p);
		if ( str != NULL )
		{
			printf("%s ", str->Get());
			list_c = list_c->next;
			continue;
		}
		
		PolizVarAddr* var_addr = dynamic_cast<PolizVarAddr*>(list_c->p);
		if ( var_addr != NULL )
		{
			printf("[%d] ", var_addr->Get());
			list_c = list_c->next;
			continue;
		}

		PolizLabel* label = dynamic_cast<PolizLabel*>(list_c->p);
		if ( label != NULL )
		{
			printf("[%p] ", label->Get());
			list_c = list_c->next;
			continue;
		}

		printf("%s", "\"not <PolizConst> object\"");
		list_c = list_c->next;
	}
	putchar('\n');

	pi_clear(&list, 1);

	return NULL;
}

PolizElem* PolizBuy::EvaluateFun(PolizItem** stack)
{
	if ( *stack == NULL )
	{
		fprintf(stderr, "%s", "\n[PolizElem]: In function: PolizBuy::EvaluateFun() stack is empty!\n");
		return NULL;
	}
	
	int size = Size(stack);
	if ( size != 2 )
	{
		int clear = Clear(stack);
		if ( !clear )
			fprintf(stderr, "\n[PolizElem]: In function: PolizBuy::EvaluateFun() unable to clear stack!\n");
		fprintf(stderr, "\n[PolizElem]: In function: PolizBuy::EvaluateFun() stack has more or less than it needs operands!\n");
		
		return NULL;
	}

	PolizElem* elem = Pop(stack);

	int op1 = 0;
	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(elem);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete elem;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(elem);
	if ( !va1 && !i1 )
		throw PolizExNotInt(elem);
	if ( i1 )
	{
		op1 = i1->Get();
	}


	PolizElem* elem2 = Pop(stack);

	int op2 = 0;
	PolizVarAddr* va2 = dynamic_cast<PolizVarAddr*>(elem2);
	if ( va2 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var2 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var2 = dynamic_cast<PolizVar*>(vl->p);
			if ( var2 && (var2->GetVarAddr()->Get() == va2->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op2 = var2->GetVarValue();
		}
		else
		{
			delete elem;
			delete elem2;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i2 = dynamic_cast<PolizInt*>(elem2);
	if ( !va2 && !i2 )
		throw PolizExNotInt(elem2);
	if ( i2 )
	{
		op2 = i2->Get();
	}

	int buy_amount = op1;
	char buy_amnt[10];
	itoa(buy_amount, buy_amnt, 9);
	

	int buy_price = op2;
	char buy_prc[10];
	itoa(buy_price, buy_prc, 9);

	const char* buy_command = "buy ";

	char send_buf[DATA_BUFFER_SIZE] = { 0 };
	int i, j;
	for ( i = 0; buy_command[i]; i++ )
		send_buf[i] = buy_command[i];
	for ( j = 0; buy_amnt[j]; j++, i++ )
		send_buf[i] = buy_amnt[j];
	send_buf[i] = ' ';
	i++;
	for ( j = 0; buy_prc[j]; j++, i++ )
		send_buf[i] = buy_prc[j];
	send_buf[i] = '\n';
	send_buf[i+1] = '\0';

	int wc = write(main_info.fd, send_buf, i+1);
	printf("\nSent %d\\%d bytes\n", wc, i+1);
	
	for ( i = 0; send_buf[i]; i++ )
	{
		printf("%c ", send_buf[i]);
		if ( ((i+1) % 20) == 0 )
			putchar('\n');
	}
	putchar('\n');

	for ( i = 0; send_buf[i]; i++ )
	{
		printf("%d ", send_buf[i]);
		if ( ((i+1) % 20) == 0 )
			putchar('\n');
	}
	putchar('\n');


	delete elem;
	delete elem2;

	return NULL;
}

PolizElem* PolizSell::EvaluateFun(PolizItem** stack)
{
	if ( *stack == NULL )
	{
		fprintf(stderr, "%s", "\n[PolizElem]: In function: PolizSell::EvaluateFun() stack is empty!\n");
		return NULL;
	}
	
	int size = Size(stack);
	if ( size != 2 )
	{
		int clear = Clear(stack);
		if ( !clear )
			fprintf(stderr, "\n[PolizElem]: In function: PolizSell::EvaluateFun() unable to clear stack!\n");
		fprintf(stderr, "\n[PolizElem]: In function: PolizSell::EvaluateFun() stack has more or less than it needs operands!\n");
		
		return NULL;
	}

	PolizElem* elem = Pop(stack);

	int op1 = 0;
	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(elem);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete elem;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(elem);
	if ( !va1 && !i1 )
		throw PolizExNotInt(elem);
	if ( i1 )
	{
		op1 = i1->Get();
	}


	PolizElem* elem2 = Pop(stack);

	int op2 = 0;
	PolizVarAddr* va2 = dynamic_cast<PolizVarAddr*>(elem2);
	if ( va2 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var2 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var2 = dynamic_cast<PolizVar*>(vl->p);
			if ( var2 && (var2->GetVarAddr()->Get() == va2->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op2 = var2->GetVarValue();
		}
		else
		{
			delete elem;
			delete elem2;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i2 = dynamic_cast<PolizInt*>(elem2);
	if ( !va2 && !i2 )
		throw PolizExNotInt(elem2);
	if ( i2 )
	{
		op2 = i2->Get();
	}


	int sell_amount = op1;
	char sell_amnt[10];
	itoa(sell_amount, sell_amnt, 9);
	
	int sell_price = op2;
	char sell_prc[10];
	itoa(sell_price, sell_prc, 9);

	const char* sell_command = "sell ";

	char send_buf[DATA_BUFFER_SIZE] = { 0 };
	int i, j;
	for ( i = 0; sell_command[i]; i++ )
		send_buf[i] = sell_command[i];
	for ( j = 0; sell_amnt[j]; j++, i++ )
		send_buf[i] = sell_amnt[j];
	send_buf[i] = ' ';
	i++;
	for ( j = 0; sell_prc[j]; j++, i++ )
		send_buf[i] = sell_prc[j];
	send_buf[i] = '\n';
	send_buf[i+1] = '\0';

	int wc = write(main_info.fd, send_buf, i+1);
	printf("\nSent %d\\%d bytes\n", wc, i+1);
	
	for ( i = 0; send_buf[i]; i++ )
	{
		printf("%c ", send_buf[i]);
		if ( ((i+1) % 20) == 0 )
			putchar('\n');
	}
	putchar('\n');

	for ( i = 0; send_buf[i]; i++ )
	{
		printf("%d ", send_buf[i]);
		if ( ((i+1) % 20) == 0 )
			putchar('\n');
	}
	putchar('\n');


	delete elem;
	delete elem2;

	return NULL;
}

PolizElem* PolizProd::EvaluateFun(PolizItem** stack)
{
	if ( *stack == NULL )
	{
		fprintf(stderr, "%s", "\n[PolizElem]: In function: PolizProd::EvaluateFun() stack is empty!\n");
		return NULL;
	}
	
	int size = Size(stack);
	if ( size > 1 )
	{
		int clear = Clear(stack);
		if ( !clear )
			fprintf(stderr, "\n[PolizElem]: In function: PolizProd::EvaluateFun() unable to clear stack!\n");
		fprintf(stderr, "\n[PolizElem]: In function: PolizProd::EvaluateFun() stack has more than it needs operands!\n");
		
		return NULL;
	}

	PolizElem* elem = Pop(stack);
	
	int op1 = 0;
	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(elem);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete elem;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(elem);
	if ( !va1 && !i1 )
		throw PolizExNotInt(elem);
	if ( i1 )
	{
		op1 = i1->Get();
	}

	int prod_amount = op1;

	while ( prod_amount > 0 )
	{
		const char* prod_command = "prod\n";
		int wc = write(main_info.fd, prod_command, 5);
		printf("\nSent %d\\%d bytes\n", wc, 5);
		
		int i;
		for ( i = 0; prod_command[i]; i++ )
		{
			printf("%c ", prod_command[i]);
			if ( ((i+1) % 20) == 0 )
				putchar('\n');
		}
		putchar('\n');

		for ( i = 0; prod_command[i]; i++ )
		{
			printf("%d ", prod_command[i]);
			if ( ((i+1) % 20) == 0 )
				putchar('\n');
		}
		putchar('\n');
		
		
		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 500000;
		select(0, NULL, NULL, NULL, &tv);
		

		prod_amount--;
	}

	delete elem;

	return NULL;
}

PolizElem* PolizBuild::EvaluateFun(PolizItem** stack)
{
	if ( *stack == NULL )
	{
		fprintf(stderr, "%s", "\n[PolizElem]: In function: PolizBuild::EvaluateFun() stack is empty!\n");
		return NULL;
	}
	
	int size = Size(stack);
	if ( size > 1 )
	{
		int clear = Clear(stack);
		if ( !clear )
			fprintf(stderr, "\n[PolizElem]: In function: PolizBuild::EvaluateFun() unable to clear stack!\n");
		fprintf(stderr, "\n[PolizElem]: In function: PolizBuild::EvaluateFun() stack has more than it needs operands!\n");
		
		return NULL;
	}

	PolizElem* elem = Pop(stack);

	int op1 = 0;
	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(elem);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete elem;
			return new PolizInt(0);
		}
	}

	PolizInt* i1 = dynamic_cast<PolizInt*>(elem);
	if ( !va1 && !i1 )
		throw PolizExNotInt(elem);
	if ( i1 )
	{
		op1 = i1->Get();
	}

	int build_amount = op1;

	while ( build_amount > 0 )
	{
		const char* build_command = "build\n";
		int wc = write(main_info.fd, build_command, 6);
		printf("\nSent %d\\%d bytes\n", wc, 6);
		
		int i;
		for ( i = 0; build_command[i]; i++ )
		{
			printf("%c ", build_command[i]);
			if ( ((i+1) % 20) == 0 )
				putchar('\n');
		}
		putchar('\n');

		for ( i = 0; build_command[i]; i++ )
		{
			printf("%d ", build_command[i]);
			if ( ((i+1) % 20) == 0 )
				putchar('\n');
		}
		putchar('\n');
		
		
		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 500000;
		select(0, NULL, NULL, NULL, &tv);


		build_amount--;
	}


	delete elem;

	return NULL;
}

PolizElem* PolizEndturn::EvaluateFun(PolizItem** stack)
{
	if ( *stack != NULL )
	{
		int clear = Clear(stack);
		if ( !clear )
			fprintf(stderr, "%s", "\n[PolizElem]: In function: PolizEndturn::EvaluateFun() unable to clear stack!\n");
		fprintf(stderr, "%s", "\n[PolizElem]: In function: PolizEndturn::EvaluateFun() stack is not empty!\n");
		return NULL;
	}
	
	const char* turn_command = "turn\n";
	int len = strlen(turn_command);
	int wc = write(main_info.fd, turn_command, len);
	printf("\nSent %d\\%d bytes\n", wc, len);

	int i;
	for ( i = 0; turn_command[i]; i++ )
	{
		printf("%c ", turn_command[i]);
		if ( ((i+1) % 20) == 0 )
			putchar('\n');
	}
	putchar('\n');

	for ( i = 0; turn_command[i]; i++ )
	{
		printf("%d ", turn_command[i]);
		if ( ((i+1) % 20) == 0 )
			putchar('\n');
	}
	putchar('\n');

	main_info.execute_script = 0;
	
	return NULL;
}

PolizElem* PolizFunLess::EvaluateFun(PolizItem** stack)
{
	PolizElem* operand1 = Pop(stack);
	int op1 = 0;
	int op2 = 0;

	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(operand1);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete operand1;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(operand1);
	if ( !va1 && !i1 )
		throw PolizExNotInt(operand1);
	if ( i1 )
	{
		op1 = i1->Get();
	}

	PolizElem* operand2 = Pop(stack);
	PolizVarAddr* va2 = dynamic_cast<PolizVarAddr*>(operand2);
	if ( va2 )
	{
		PolizItem* vl = vars_list;
		PolizVar* var2 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var2 = dynamic_cast<PolizVar*>(vl->p);
			if ( var2 && (var2->GetVarAddr()->Get() == va2->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op2 = var2->GetVarValue();
		}
		else
		{
			delete operand1;
			delete operand2;
			return new PolizInt(0);
		}
	}

	PolizInt* i2 = dynamic_cast<PolizInt*>(operand2);
	if ( !va2 && !i2 )
		throw PolizExNotInt(operand2);
	if ( i2 )
	{
		op2 = i2->Get();
	}

	int res = (op1 < op2) ? 1 : 0;
	delete operand1;
	delete operand2;

	return new PolizInt(res);
}

PolizElem* PolizFunGreater::EvaluateFun(PolizItem** stack)
{
	PolizElem* operand1 = Pop(stack);
	int op1 = 0;
	int op2 = 0;

	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(operand1);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete operand1;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(operand1);
	if ( !va1 && !i1 )
		throw PolizExNotInt(operand1);
	if ( i1 )
	{
		op1 = i1->Get();
	}

	PolizElem* operand2 = Pop(stack);
	PolizVarAddr* va2 = dynamic_cast<PolizVarAddr*>(operand2);
	if ( va2 )
	{
		PolizItem* vl = vars_list;
		PolizVar* var2 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var2 = dynamic_cast<PolizVar*>(vl->p);
			if ( var2 && (var2->GetVarAddr()->Get() == va2->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op2 = var2->GetVarValue();
		}
		else
		{
			delete operand1;
			delete operand2;
			return new PolizInt(0);
		}
	}

	PolizInt* i2 = dynamic_cast<PolizInt*>(operand2);
	if ( !va2 && !i2 )
		throw PolizExNotInt(operand2);
	if ( i2 )
	{
		op2 = i2->Get();
	}

	int res = (op1 > op2) ? 1 : 0;
	delete operand1;
	delete operand2;

	return new PolizInt(res);
}

PolizElem* PolizFunEqual::EvaluateFun(PolizItem** stack)
{
	PolizElem* operand1 = Pop(stack);
	int op1 = 0;
	int op2 = 0;

	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(operand1);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete operand1;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(operand1);
	if ( !va1 && !i1 )
		throw PolizExNotInt(operand1);
	if ( i1 )
	{
		op1 = i1->Get();
	}

	PolizElem* operand2 = Pop(stack);
	PolizVarAddr* va2 = dynamic_cast<PolizVarAddr*>(operand2);
	if ( va2 )
	{
		PolizItem* vl = vars_list;
		PolizVar* var2 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var2 = dynamic_cast<PolizVar*>(vl->p);
			if ( var2 && (var2->GetVarAddr()->Get() == va2->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op2 = var2->GetVarValue();
		}
		else
		{
			delete operand1;
			delete operand2;
			return new PolizInt(0);
		}
	}

	PolizInt* i2 = dynamic_cast<PolizInt*>(operand2);
	if ( !va2 && !i2 )
		throw PolizExNotInt(operand2);
	if ( i2 )
	{
		op2 = i2->Get();
	}

	int res = (op1 == op2) ? 1 : 0;
	delete operand1;
	delete operand2;

	return new PolizInt(res);
}

PolizElem* PolizFunNotEqual::EvaluateFun(PolizItem** stack)
{
	PolizElem* operand1 = Pop(stack);
	int op1 = 0;
	int op2 = 0;

	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(operand1);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete operand1;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(operand1);
	if ( !va1 && !i1 )
		throw PolizExNotInt(operand1);
	if ( i1 )
	{
		op1 = i1->Get();
	}

	PolizElem* operand2 = Pop(stack);
	PolizVarAddr* va2 = dynamic_cast<PolizVarAddr*>(operand2);
	if ( va2 )
	{
		PolizItem* vl = vars_list;
		PolizVar* var2 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var2 = dynamic_cast<PolizVar*>(vl->p);
			if ( var2 && (var2->GetVarAddr()->Get() == va2->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op2 = var2->GetVarValue();
		}
		else
		{
			delete operand1;
			delete operand2;
			return new PolizInt(0);
		}
	}

	PolizInt* i2 = dynamic_cast<PolizInt*>(operand2);
	if ( !va2 && !i2 )
		throw PolizExNotInt(operand2);
	if ( i2 )
	{
		op2 = i2->Get();
	}
	
	int res = (op1 != op2) ? 1 : 0;
	delete operand1;
	delete operand2;

	return new PolizInt(res);
}

PolizElem* PolizFunOr::EvaluateFun(PolizItem** stack)
{
	PolizElem* operand1 = Pop(stack);
	int op1 = 0;
	int op2 = 0;

	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(operand1);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete operand1;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(operand1);
	if ( !va1 && !i1 )
		throw PolizExNotInt(operand1);
	if ( i1 )
	{
		op1 = i1->Get();
	}


	PolizElem* operand2 = Pop(stack);
	PolizVarAddr* va2 = dynamic_cast<PolizVarAddr*>(operand2);
	if ( va2 )
	{
		PolizItem* vl = vars_list;
		PolizVar* var2 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var2 = dynamic_cast<PolizVar*>(vl->p);
			if ( var2 && (var2->GetVarAddr()->Get() == va2->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op2 = var2->GetVarValue();
		}
		else
		{
			delete operand1;
			delete operand2;
			return new PolizInt(0);
		}
	}

	PolizInt* i2 = dynamic_cast<PolizInt*>(operand2);
	if ( !va2 && !i2 )
		throw PolizExNotInt(operand2);
	if ( i2 )
	{
		op2 = i2->Get();
	}
	
	int res = (op1 || op2) ? 1 : 0;
	delete operand1;
	delete operand2;

	return new PolizInt(res);
}

PolizElem* PolizFunAnd::EvaluateFun(PolizItem** stack)
{
	PolizElem* operand1 = Pop(stack);
	int op1 = 0;
	int op2 = 0;

	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(operand1);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete operand1;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(operand1);
	if ( !va1 && !i1 )
		throw PolizExNotInt(operand1);
	if ( i1 )
	{
		op1 = i1->Get();
	}


	PolizElem* operand2 = Pop(stack);
	PolizVarAddr* va2 = dynamic_cast<PolizVarAddr*>(operand2);
	if ( va2 )
	{
		PolizItem* vl = vars_list;
		PolizVar* var2 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var2 = dynamic_cast<PolizVar*>(vl->p);
			if ( var2 && (var2->GetVarAddr()->Get() == va2->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op2 = var2->GetVarValue();
		}
		else
		{
			delete operand1;
			delete operand2;
			return new PolizInt(0);
		}
	}

	PolizInt* i2 = dynamic_cast<PolizInt*>(operand2);
	if ( !va2 && !i2 )
		throw PolizExNotInt(operand2);
	if ( i2 )
	{
		op2 = i2->Get();
	}

	
	int res = (op1 && op2) ? 1 : 0;
	delete operand1;
	delete operand2;

	return new PolizInt(res);
}

PolizElem* PolizFunNot::EvaluateFun(PolizItem** stack) 
{
	PolizElem* operand1 = Pop(stack);
	int op1 = 0;

	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(operand1);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete operand1;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(operand1);
	if ( !va1 && !i1 )
		throw PolizExNotInt(operand1);
	if ( i1 )
	{
		op1 = i1->Get();
	}


	int res = ( !op1 ) ? 1 : 0;
	delete operand1;

	return new PolizInt(res);
}

PolizElem* PolizFunUnMinus::EvaluateFun(PolizItem** stack)
{
	PolizElem* operand1 = Pop(stack);
	int op1 = 0;

	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(operand1);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete operand1;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(operand1);
	if ( !va1 && !i1 )
		throw PolizExNotInt(operand1);
	if ( i1 )
	{
		op1 = i1->Get();
	}


	int res = -1 * op1;
	delete operand1;

	return new PolizInt(res);
}

PolizElem* PolizFunPlus::EvaluateFun(PolizItem** stack)
{
	PolizElem* operand1 = Pop(stack);
	int op1 = 0;
	int op2 = 0;

	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(operand1);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete operand1;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(operand1);
	if ( !va1 && !i1 )
		throw PolizExNotInt(operand1);
	if ( i1 )
	{
		op1 = i1->Get();
	}


	PolizElem* operand2 = Pop(stack);
	PolizVarAddr* va2 = dynamic_cast<PolizVarAddr*>(operand2);
	if ( va2 )
	{
		PolizItem* vl = vars_list;
		PolizVar* var2 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var2 = dynamic_cast<PolizVar*>(vl->p);
			if ( var2 && (var2->GetVarAddr()->Get() == va2->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op2 = var2->GetVarValue();
		}
		else
		{
			delete operand1;
			delete operand2;
			return new PolizInt(0);
		}
	}

	PolizInt* i2 = dynamic_cast<PolizInt*>(operand2);
	if ( !va2 && !i2 )
		throw PolizExNotInt(operand2);
	if ( i2 )
	{
		op2 = i2->Get();
	}
	
	int res = op1 + op2;
	delete operand1;
	delete operand2;

	return new PolizInt(res);
}

PolizElem* PolizFunMinus::EvaluateFun(PolizItem** stack)
{
	PolizElem* operand1 = Pop(stack);
	int op1 = 0;
	int op2 = 0;

	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(operand1);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete operand1;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(operand1);
	if ( !va1 && !i1 )
		throw PolizExNotInt(operand1);
	if ( i1 )
	{
		op1 = i1->Get();
	}


	PolizElem* operand2 = Pop(stack);
	PolizVarAddr* va2 = dynamic_cast<PolizVarAddr*>(operand2);
	if ( va2 )
	{
		PolizItem* vl = vars_list;
		PolizVar* var2 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var2 = dynamic_cast<PolizVar*>(vl->p);
			if ( var2 && (var2->GetVarAddr()->Get() == va2->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op2 = var2->GetVarValue();
		}
		else
		{
			delete operand1;
			delete operand2;
			return new PolizInt(0);
		}
	}

	PolizInt* i2 = dynamic_cast<PolizInt*>(operand2);
	if ( !va2 && !i2 )
		throw PolizExNotInt(operand2);
	if ( i2 )
	{
		op2 = i2->Get();
	}


	int res = op1 - op2;
	delete operand1;
	delete operand2;

	return new PolizInt(res);
}

PolizElem* PolizFunMul::EvaluateFun(PolizItem** stack)
{
	PolizElem* operand1 = Pop(stack);
	int op1 = 0;
	int op2 = 0;

	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(operand1);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete operand1;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(operand1);
	if ( !va1 && !i1 )
		throw PolizExNotInt(operand1);
	if ( i1 )
	{
		op1 = i1->Get();
	}


	PolizElem* operand2 = Pop(stack);
	PolizVarAddr* va2 = dynamic_cast<PolizVarAddr*>(operand2);
	if ( va2 )
	{
		PolizItem* vl = vars_list;
		PolizVar* var2 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var2 = dynamic_cast<PolizVar*>(vl->p);
			if ( var2 && (var2->GetVarAddr()->Get() == va2->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op2 = var2->GetVarValue();
		}
		else
		{
			delete operand1;
			delete operand2;
			return new PolizInt(0);
		}
	}

	PolizInt* i2 = dynamic_cast<PolizInt*>(operand2);
	if ( !va2 && !i2 )
		throw PolizExNotInt(operand2);
	if ( i2 )
	{
		op2 = i2->Get();
	}

	
	int res = op1 * op2;
	delete operand1;
	delete operand2;

	return new PolizInt(res);
}

PolizElem* PolizFunDiv::EvaluateFun(PolizItem** stack)
{
	PolizElem* operand1 = Pop(stack);
	int op1 = 0;
	int op2 = 0;

	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(operand1);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete operand1;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(operand1);
	if ( !va1 && !i1 )
		throw PolizExNotInt(operand1);
	if ( i1 )
	{
		op1 = i1->Get();
	}


	PolizElem* operand2 = Pop(stack);
	PolizVarAddr* va2 = dynamic_cast<PolizVarAddr*>(operand2);
	if ( va2 )
	{
		PolizItem* vl = vars_list;
		PolizVar* var2 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var2 = dynamic_cast<PolizVar*>(vl->p);
			if ( var2 && (var2->GetVarAddr()->Get() == va2->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op2 = var2->GetVarValue();
		}
		else
		{
			delete operand1;
			delete operand2;
			return new PolizInt(0);
		}
	}

	PolizInt* i2 = dynamic_cast<PolizInt*>(operand2);
	if ( !va2 && !i2 )
		throw PolizExNotInt(operand2);
	if ( i2 )
	{
		op2 = i2->Get();
	}

	if ( op2 == 0 )
		throw PolizExDivisionZero(operand2);
	
	int res = op1 / op2;
	delete operand1;
	delete operand2;

	return new PolizInt(res);
}

PolizElem* PolizFunRem::EvaluateFun(PolizItem** stack)
{
	PolizElem* operand1 = Pop(stack);
	int op1 = 0;
	int op2 = 0;

	PolizVarAddr* va1 = dynamic_cast<PolizVarAddr*>(operand1);
	if ( va1 )
	{		
		PolizItem* vl = vars_list;
		PolizVar* var1 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var1 = dynamic_cast<PolizVar*>(vl->p);
			if ( var1 && (var1->GetVarAddr()->Get() == va1->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op1 = var1->GetVarValue();
		}
		else
		{
			delete operand1;
			return new PolizInt(0);
		}
	}
	
	PolizInt* i1 = dynamic_cast<PolizInt*>(operand1);
	if ( !va1 && !i1 )
		throw PolizExNotInt(operand1);
	if ( i1 )
	{
		op1 = i1->Get();
	}


	PolizElem* operand2 = Pop(stack);
	PolizVarAddr* va2 = dynamic_cast<PolizVarAddr*>(operand2);
	if ( va2 )
	{
		PolizItem* vl = vars_list;
		PolizVar* var2 = NULL;
		while ( (vl != NULL) && (vl->p != NULL) )
		{
			var2 = dynamic_cast<PolizVar*>(vl->p);
			if ( var2 && (var2->GetVarAddr()->Get() == va2->Get() ) )
				break;

			vl = vl->next;
		}

		if ( vl != NULL )
		{
			op2 = var2->GetVarValue();
		}
		else
		{
			delete operand1;
			delete operand2;
			return new PolizInt(0);
		}
	}

	PolizInt* i2 = dynamic_cast<PolizInt*>(operand2);
	if ( !va2 && !i2 )
		throw PolizExNotInt(operand2);
	if ( i2 )
	{
		op2 = i2->Get();
	}

	if ( op2 == 0 )
		throw PolizExDivisionZero(operand2);
	
	int res = op1 % op2;
	delete operand1;
	delete operand2;

	return new PolizInt(res);
}
#endif
