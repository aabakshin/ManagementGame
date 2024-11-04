/* Модуль для запуска процесса сервера */

#include "../../includes/serverCore.h"

int main(int argc, char** argv)
{
	if ( argc < 2 )
	{
		fprintf(stderr, "%s", "Incorrect arguments num.\n"
							  "Usage: ./<program_name> <port>\n");
		return 1;
	}
	
	printf("%s", "Initialization server banker...\n");
	Banker server_banker;
	if ( !banker_init(&server_banker) )
	{
		fprintf(stderr, "%s", "\"server_banker\" address returns NULL pointer\n");
		return 1;
	}
	
	int ls;
	if ( (ls = server_init(argv[1])) == -1 )
	{
		fprintf(stderr, "%s", "An error has occured during executing server initialization procedure\n");
		return 1;
	}

	return server_run(&server_banker, ls);	
} 
