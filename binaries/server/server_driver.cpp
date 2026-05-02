#include "Server.hpp"
#include <cstdio>


int main( int argc, char** argv )
{
	if ( argc < 3 )
	{
		fprintf(stderr, "%s",
				"Incorrect arguments num.\n"
				"Usage: ./<program_name> <address> <port>\n");
		return 1;
	}

	Server game_server;
	game_server.Make( argv[1], argv[2] );

	return game_server.Run();
}
