/* Тестовый модуль для проверки контейнера BuildList */

#include <stdio.h>

#include "../includes/BuildList.h"

int main(int argc, char** argv)
{
	printf("BuildList item size = %lu\n", sizeof(struct BuildList));
	BuildList* build_queue = NULL;

	printf("size = %d\n", bl_get_size(build_queue));
	bl_insert(&build_queue);
	printf("size = %d\n", bl_get_size(build_queue));
	bl_insert(&build_queue);
	printf("size = %d\n", bl_get_size(build_queue));
	bl_insert(&build_queue);
	printf("size = %d\n", bl_get_size(build_queue));
	bl_insert(&build_queue);
	printf("size = %d\n", bl_get_size(build_queue));
	bl_insert(&build_queue);
	printf("size = %d\n", bl_get_size(build_queue));
	
	BuildList* listPtr = build_queue;
	while (listPtr != NULL)
	{
		printf("Node #%d\n"
			   "Turns left: %d\n", listPtr->build_number, listPtr->turns_left);
		
		if (listPtr->next == NULL)
			break;

		listPtr = listPtr->next;
	}
	
	listPtr = build_queue;
	while ( listPtr != NULL )
	{
		if (listPtr->build_number == 3 )
		{
			listPtr->turns_left = 0;
			printf("Deleted node #%d\n", bl_delete(&build_queue, 0));
			listPtr = build_queue;
			break;
		}
		listPtr = listPtr->next;
	}

	printf("size = %d\n", bl_get_size(listPtr));
	while ( listPtr != NULL )
	{
		printf("Node #%d\n"
			   "Turns left: %d\n", listPtr->build_number, listPtr->turns_left);

		if ( listPtr->next == NULL )
			break;

		listPtr = listPtr->next;
	}

	putchar('\n');
	putchar('\n');	
	putchar('\n');
	
	bl_clear(&build_queue);
	if ( build_queue == NULL )
		printf("ok\n");
	else
		printf("trouble\n");

	printf("size = %d\n", bl_get_size(build_queue));

	return 0;
}
