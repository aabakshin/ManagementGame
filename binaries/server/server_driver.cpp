#include "Server.hpp"
//#include <iostream>


int main( int argc, char** argv )
{
	if ( argc < 3 )
	{
		//std::cerr <<
		//	"Incorrect arguments num.\n"
		//	"Usage: ./<program_name> <address> <port>\n";

		return 1;
	}

	try
	{
		Server game_server;

		game_server.Make( argv[1], argv[2] );

		return game_server.Run();
	}
	catch ( const std::runtime_error& ex )
	{
		// логгирование ошибки
		//std::cerr << "Не удалось запустить сервер: " << ex.what() << std::endl;
		return 1;
	}
}
