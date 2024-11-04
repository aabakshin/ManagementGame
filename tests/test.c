/* Тестовый модуль для проверки функции delete_spaces */

#include "../includes/MGLib.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
	char buffer[100];
	fgets(buffer, 99, stdin);
	int size = strlen(buffer)+1;

	printf("[1]buffer = %s\nbuffer_len = %d\n", buffer, size-1);
	/*int i;
	for (i = 0; i < 30; i++)
		printf("%d ", buffer[i]);
	putchar('\n');*/

	delete_spaces(buffer, &size);
	
	int i;
	for (i = 0; i < 30; i++ )
		printf("%d ", buffer[i]);
	putchar('\n');

	printf("[2]buffer = %s\nbuffer_size = %d\n", buffer, size);
	/*for (i = 0; i < 30; i++)
		printf("%d ", buffer[i]);
	putchar('\n');*/

	return 0;
}
