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

	Server* game_server = new Server( const_cast<const char*>(argv[1]), const_cast<const char*>(argv[2]) );

	game_server->Run();

	if ( game_server != nullptr )
		delete game_server;

	return 0;
}
