/* Тестовый модуль для проверки контейнера CommandsHistoryList */

#include <stdio.h>
#include "../includes/CommandsHistoryList.h"


int main(int argc, char** argv)
{
	CommandsHistoryList* chl_list = NULL;


	chl_insert(&chl_list, "Hello", 6);
	chl_insert(&chl_list, "World", 6);
	chl_insert(&chl_list, "Pizza", 6);
	chl_insert(&chl_list, "pena", 5);
	chl_insert(&chl_list, "lox", 4);

	
	CommandsHistoryList* chl_l = chl_list;
	
	int size = chl_get_size(chl_list);
	printf("size = %d\n", size);

	int i; 
	for ( i = 1; i <= size; i++ )
	{
		printf("num = %d\nstr = %s\n", chl_l->number, chl_l->command);
		chl_l = chl_l->next;
	}
	
	/*chl_delete(&chl_list, 1);
	chl_delete(&chl_list, 2);
	chl_delete(&chl_list, 3);
	chl_delete(&chl_list, 4);
	chl_delete(&chl_list, 5);*/
	
	/*chl_l = chl_list;
	size = chl_get_size(chl_list);
	printf("size = %d\n", size);
	
	for ( i = 1; i <= size; i++ )
	{
		printf("%d\n", chl_list->number);
		chl_list = chl_list->next;
	}
	*/


	chl_clear(&chl_list);

	size = chl_get_size(chl_list);
	printf("size = %d\n", size);

	return 0;
}
