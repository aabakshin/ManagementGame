#include "Logger.hpp"
#include "Config.hpp"
#include "Server.hpp"
#include <iostream>


int main( int argc, char** argv )
{
	if ( argc < 3 )
	{
		std::cerr <<
			"Incorrect arguments num\n"
			"Usage: ./" << argv[0] << " <port>\n";

		return 1;
	}

	Logger logger;
	Config game_config;

	try
	{
		logger.SetErrorFile("error.log");
		game_config.Load("config.json");
		auto game_config_settings = std::make_shared<Config::GameSettings>( std::move(game_config.game_settings) );

		Server game_server( nullptr, argv[1], &logger, std::move(game_config_settings) );

		return game_server.Run();
	}
	catch ( const nlohmann::json::parse_error& ex )
	{
		logger.error( ex.what() );
		std::cerr << "Config parse error. See details in log file." << std::endl;
		return 1;
	}
	catch ( const std::runtime_error& ex )
	{
		logger.error( ex.what() );
		std::cerr << "Unable to launch server. See details in log file." << std::endl;
		return 1;
	}
}
