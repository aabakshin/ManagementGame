#include "Server.hpp"
#include "Logger.hpp"
#include <iostream>


int main( int argc, char** argv )
{
	if ( argc < 3 )
	{
		std::cerr <<
			"Incorrect arguments num\n"
			"Usage: ./" << argv[0] << " <address> <port>\n";

		return 1;
	}

	Logger logger;
	logger.SetErrorFile("error.log");

	try
	{
		Server game_server;

		game_server.Make( argv[1], argv[2], &logger );

		return game_server.Run();
	}
	catch ( const std::runtime_error& ex )
	{
		logger.error( ex.what() );
		std::cerr << "Unable to launch server. See error log file." << std::endl;
		return 1;
	}
}
