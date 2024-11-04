/* Тестовый модуль для проверки контейнера OperStack */

#include <cstdio>
#include "../includes/OperStack.hpp"

int main(int argc, char** argv)
{
	OperStack* stack = NULL;
	op_push(&stack, "Hel", 4);
	op_push(&stack, "l", 2);
	op_push(&stack, "l", 2);
	
	char* popValue = op_pop(&stack);
	delete[] popValue;

	op_push(&stack, "l", 2);
	op_push(&stack, "l", 2);
	op_push(&stack, "o", 2);

	printf("%s", "\nStack: ");
	op_print(stack);
	putchar('\n');
	int size = op_size(stack);
	printf("size = %d\n", size);

	op_clear(&stack);

	size = op_size(stack);
	printf("size = %d\n", size);

	return 0;
}
