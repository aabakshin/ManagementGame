/* Файл реализации модуля PolizItem */

#ifndef POLIZ_ITEM_CPP
#define POLIZ_ITEM_CPP

#ifndef __CPP_FILE__
#define __CPP_FILE__
#endif

#include "../includes/PolizItem.hpp"
#include "../includes/PolizElem.hpp"
#include <stdlib.h>
#include <stdio.h>

void pi_insert(PolizItem** list_ptr, PolizElem* elem)
{
	if ( (list_ptr == NULL) || (elem == NULL) )
	{
		fprintf(stderr, "%s", "\n[PolizItem]: pi_insert error\n");
		return;
	}

	PolizItem* newPtr = NULL;
	newPtr = new PolizItem;
	if ( !newPtr )
	{
		fprintf(stderr, "%s", "\n[PolizItem] pi_insert memory error\n");
		return;
	}
	newPtr->p = elem;
	newPtr->next = NULL;
	newPtr->prev = NULL;

	if ( *list_ptr == NULL )
	{
		*list_ptr = newPtr;
		(*list_ptr)->elem_number = 1;
		return;
	}

	PolizItem* prevPtr = NULL;
	PolizItem* curPtr = *list_ptr;

	while ( curPtr != NULL )
	{
		prevPtr = curPtr;
		curPtr = curPtr->next;
	}
	newPtr->elem_number = prevPtr->elem_number+1;
	prevPtr->next = newPtr;
	newPtr->prev = prevPtr;
}

PolizElem* pi_delete(PolizItem** list_ptr, PolizElem* elem)
{
	if ( (list_ptr == NULL) || (*list_ptr == NULL) || (elem == NULL) )
	{
		if ( list_ptr == NULL )
			fprintf(stderr, "%s", "\n[PolizItem] pi_delete error(list_ptr) is NULL)\n");
		else if ( elem == NULL )
			fprintf(stderr, "%s", "\n[PolizItem] pi_delete error(elem) is NULL)\n");

		return NULL;
	}
	
	PolizItem* tempPtr = NULL;

	if ( (*list_ptr)->p == elem )
	{
		tempPtr = *list_ptr;
		if ( (*list_ptr)->next != NULL )
		{
			*list_ptr = (*list_ptr)->next;
			(*list_ptr)->prev = NULL;
		}
		else
			*list_ptr = NULL;
		delete tempPtr;
		return elem;
	}

	PolizItem* prevPtr = *list_ptr;
	PolizItem* curPtr = (*list_ptr)->next;

	while ( (curPtr != NULL) && (curPtr->p != elem) )
	{
		prevPtr = curPtr;
		curPtr = curPtr->next;
	}
	
	if ( curPtr != NULL )
	{
		tempPtr = curPtr;

		if ( curPtr->next != NULL )
		{
			prevPtr->next = curPtr->next;
			curPtr->next->prev = prevPtr;
		}
		else
			prevPtr->next = NULL;

		delete tempPtr;
		return elem;
	}

	return NULL;
}

PolizItem* pi_find(PolizItem* list, PolizElem* elem)
{
	if ( (list == NULL) || (elem == NULL) )
	{
		fprintf(stderr, "%s", "\n[PolizItem] pi_find error\n");
		return NULL;
	}

	while ( list != NULL )
	{
		if ( list->p == elem )
			return list;

		list = list->next;
	}
	
	return NULL;
}

int pi_clear(PolizItem** list_ptr, int clear)
{
	if ( list_ptr == NULL )
	{
		fprintf(stderr, "%s", "\n[PolizItem] pi_clear error\n");
		return 0;
	}

	if ( *list_ptr == NULL )
		return 1;

	int size = pi_size(*list_ptr);

	for ( int i = 1; i <= size; i++ )
	{
		PolizElem* pe = (*list_ptr)->p;
		PolizElem* del = pi_delete(list_ptr, pe);
		if ( !del )
		{
			fprintf(stderr, "%s", "\n[PolizItem] pi_clear delete error\n");
			return 0;
		}

		if ( clear )
			if ( pe )
				delete pe;
	}

	return 1;
}

void pi_print(PolizItem* list)
{
	if ( list == NULL )
	{
		fprintf(stderr, "%s", "\n[PolizItem] pi_print error\n");
		return;
	}
	
	putchar('\n');
	int count = 0;
	while ( list != NULL )
	{
		count++;

		if ( dynamic_cast<PolizInt*>(list->p) != NULL )
			printf("<PolizInt*>(%d) --> ", dynamic_cast<PolizInt*>(list->p)->Get() );
		else if ( dynamic_cast<PolizString*>(list->p) != NULL )
			printf("<PolizString*>(%s) --> ", dynamic_cast<PolizString*>(list->p)->Get() );
		else if ( dynamic_cast<PolizVarAddr*>(list->p) != NULL )
			printf("<PolizVarAddr*>(%d) --> ", dynamic_cast<PolizVarAddr*>(list->p)->Get() );
		else if ( dynamic_cast<PolizLabel*>(list->p) != NULL )
			printf("<PolizLabel*>(%p) --> ", dynamic_cast<PolizLabel*>(list->p)->Get() );
		else if ( dynamic_cast<PolizOpGo*>(list->p) != NULL )
			printf("%s", "<PolizOpGo*> --> ");
		else if ( dynamic_cast<PolizOpIfThen*>(list->p) != NULL )
			printf("%s", "<PolizOpIfThen*> --> ");
		else if ( dynamic_cast<PolizVar*>(list->p) != NULL )
			printf("%s", "<PolizVar*> --> ");
		else if ( dynamic_cast<PolizAssign*>(list->p) != NULL )
			printf("%s", "<PolizAssign*> --> ");
		else if ( dynamic_cast<PolizGFMyId*>(list->p) != NULL )
			printf("%s", "<PolizGFMyId*> --> ");
		else if ( dynamic_cast<PolizGFTurn*>(list->p) != NULL )
			printf("<PolizGFTurn*>(%p) --> ", list);
		else if ( dynamic_cast<PolizGFPlayers*>(list->p) != NULL )
			printf("%s", "<PolizGFPlayers*> --> ");
		else if ( dynamic_cast<PolizGFAPlayers*>(list->p) != NULL )
			printf("%s", "<PolizGFAPlayers*> --> ");
		else if ( dynamic_cast<PolizGFSupply*>(list->p) != NULL )
			printf("%s", "<PolizGFSupply*> --> ");
		else if ( dynamic_cast<PolizGFRawPrice*>(list->p) != NULL )
			printf("%s", "<PolizGFRawPrice*> --> ");
		else if ( dynamic_cast<PolizGFDemand*>(list->p) != NULL )
			printf("%s", "<PolizGFDemand*> --> ");
		else if ( dynamic_cast<PolizGFProdPrice*>(list->p) != NULL )
			printf("%s", "<PolizGFProdPrice*> --> ");
		else if ( dynamic_cast<PolizGFMoney*>(list->p) != NULL )
			printf("%s", "<PolizGFMoney*> --> ");
		else if ( dynamic_cast<PolizGFRaw*>(list->p) != NULL )
			printf("%s", "<PolizGFRaw*> --> ");
		else if ( dynamic_cast<PolizGFProduction*>(list->p) != NULL )
			printf("%s", "<PolizGFProduction*> --> ");
		else if ( dynamic_cast<PolizGFFactories*>(list->p) != NULL )
			printf("%s", "<PolizGFFactories*> --> ");
		else if ( dynamic_cast<PolizGFManufactured*>(list->p) != NULL )
			printf("%s", "<PolizGFManufactured*> --> ");
		else if ( dynamic_cast<PolizGFResRawSold*>(list->p) != NULL )
			printf("%s", "<PolizGFResRawSold*> --> ");
		else if ( dynamic_cast<PolizGFResRawPrice*>(list->p) != NULL )
			printf("%s", "<PolizGFResRawPrice*> --> ");
		else if ( dynamic_cast<PolizGFResProdBought*>(list->p) != NULL )
			printf("%s", "<PolizGFResProdBought*> --> ");
		else if ( dynamic_cast<PolizGFResProdPrice*>(list->p) != NULL )
			printf("%s", "<PolizGFResProdPrice*> --> ");
		else if ( dynamic_cast<PolizPrint*>(list->p) != NULL )
			printf("%s", "<PolizPrint*> --> ");
		else if ( dynamic_cast<PolizBuy*>(list->p) != NULL )
			printf("%s", "<PolizBuy*> --> ");
		else if ( dynamic_cast<PolizSell*>(list->p) != NULL )
			printf("%s", "<PolizSell*> --> ");
		else if ( dynamic_cast<PolizProd*>(list->p) != NULL )
			printf("%s", "<PolizProd*> --> ");
		else if ( dynamic_cast<PolizBuild*>(list->p) != NULL )
			printf("%s", "<PolizBuild*> --> ");
		else if ( dynamic_cast<PolizEndturn*>(list->p) != NULL )
			printf("%s", "<PolizEndturn*> --> ");
		else if ( dynamic_cast<PolizFunLess*>(list->p) != NULL )
			printf("%s", "<PolizFunLess*> --> ");
		else if ( dynamic_cast<PolizFunGreater*>(list->p) != NULL )
			printf("%s", "<PolizFunGreater*> --> ");
		else if ( dynamic_cast<PolizFunEqual*>(list->p) != NULL )
			printf("%s", "<PolizFunEqual*> --> ");
		else if ( dynamic_cast<PolizFunNotEqual*>(list->p) != NULL )
			printf("%s", "<PolizFunNotEqual*> --> ");
		else if ( dynamic_cast<PolizFunOr*>(list->p) != NULL )
			printf("%s", "<PolizFunOr*> --> ");
		else if ( dynamic_cast<PolizFunAnd*>(list->p) != NULL )
			printf("%s", "<PolizFunAnd*> --> ");
		else if ( dynamic_cast<PolizFunNot*>(list->p) != NULL )
			printf("%s", "<PolizFunNot*> --> ");
		else if ( dynamic_cast<PolizFunUnMinus*>(list->p) != NULL )
			printf("%s", "<PolizFunUnMinus*> --> ");
		else if ( dynamic_cast<PolizFunPlus*>(list->p) != NULL )
			printf("%s", "<PolizFunPlus*> --> ");
		else if ( dynamic_cast<PolizFunMinus*>(list->p) != NULL )
			printf("%s", "<PolizFunMinus*> --> ");
		else if ( dynamic_cast<PolizFunDiv*>(list->p) != NULL )
			printf("%s", "<PolizFunDiv*> --> ");
		else if ( dynamic_cast<PolizFunMul*>(list->p) != NULL )
			printf("%s", "<PolizFunMul*> --> ");
		else if ( dynamic_cast<PolizFunRem*>(list->p) != NULL )
			printf("%s", "<PolizFunRem*> --> ");


		if ( (count % 5) == 0 )
			putchar('\n');
		

		list = list->next;
	}
	printf("%s", "NULL\n\n");

}

int pi_size(PolizItem* list)
{
	if ( list == NULL )
		return 0;

	int size = 0;
	while ( list != NULL )
	{
		size++;
		list = list->next;
	}

	return size;
}

#endif
