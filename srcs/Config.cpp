#ifndef CONFIG_CPP_SENTINEL
#define CONFIG_CPP_SENTINEL


#include "Config.hpp"
#include "Utility.hpp"
#include <fstream>
#include <iostream>


enum
{
				MIN_PLAYERS_TO_START,
				MAX_PLAYERS,
				TIME_TO_START,
				START_MARKET_LEVEL,
				START_MONEY,
				START_SOURCES,
				START_PRODUCTS,
				START_FACTORIES,
				SOURCE_UNIT_CHARGE,
				PRODUCT_UNIT_CHARGE,
				FACTORY_UNIT_CHARGE,
				NEW_FACTORY_UNIT_COST,
				PRODUCTION_PRODUCT_COST,
				TURNS_TO_BUILD_FACTORY
};

static const char* settings_names[] =
{
				"min_players_to_start",
				"max_players",
				"time_to_start",
				"start_market_level",
				"start_money",
				"start_sources",
				"start_products",
				"start_factories",
				"source_unit_charge",
				"product_unit_charge",
				"factory_unit_charge",
				"new_factory_unit_cost",
				"production_product_cost",
				"turns_to_build_factory",
				nullptr
};

bool Config::Load( const std::string& filepath )
{
	std::vector<std::reference_wrapper<int>> game_param_refs =
	{
				game_settings.min_players_to_start,
				game_settings.max_players,
				game_settings.time_to_start,
				game_settings.start_market_level,
				game_settings.start_money,
				game_settings.start_sources,
				game_settings.start_products,
				game_settings.start_factories,
				game_settings.source_unit_charge,
				game_settings.product_unit_charge,
				game_settings.factory_unit_charge,
				game_settings.new_factory_unit_cost,
				game_settings.production_product_cost,
				game_settings.turns_to_build_factory
	};

	std::vector<std::function<bool(int)>> game_params_predicats =
	{
		[]( int value ) { return ( value >= 1 && value <= 8 ); },
		[]( int value ) { return ( value >= 1 && value <= 8 ); },
		[]( int value ) { return ( value >= 5 && value <= 300 ); },
		[]( int value ) { return ( value >= 1 && value <= 5 ); },
		[]( int value ) { return ( value >= 1 && value <= 999999999 ); },
		[]( int value ) { return ( value >= 1 && value <= 10 ); },
		[]( int value ) { return ( value >= 1 && value <= 10 ); },
		[]( int value ) { return ( value >= 1 && value <= 10 ); },
		[]( int value ) { return ( value >= 1 && value <= 999999 ); },
		[]( int value ) { return ( value >= 1 && value <= 999999 ); },
		[]( int value ) { return ( value >= 1 && value <= 999999 ); },
		[]( int value ) { return ( value >= 1 && value <= 999999 ); },
		[]( int value ) { return ( value >= 1 && value <= 999999 ); },
		[]( int value ) { return ( value >= 1 && value <= 100 ); },
	};


	std::ifstream config_file(filepath);

	if ( !config_file.is_open() )
	{
		std::cerr << "[" << Utility::current_time_str() << "] " << "[WARN] "  << "Config file " << "\"" << filepath << "\"" << " not found, using default settings" << std::endl;
		return false;
	}

	nlohmann::json config_obj;

	try
	{
		config_file >> config_obj;
	}
	catch ( const nlohmann::json::parse_error& ex )
	{
		throw;
	}

	for ( int i = MIN_PLAYERS_TO_START; i <= TURNS_TO_BUILD_FACTORY; ++i )
	{
		if ( config_obj.contains(settings_names[i]) )
		{
			int value = config_obj[settings_names[i]].get<int>();
			if ( game_params_predicats[i](value) )
				game_param_refs[i].get() = value;
			else
				std::cerr << "[" << Utility::current_time_str() << "] " << "[ERROR] " << "Invalid " << "\"" << settings_names[i] << "\", using defaults" << std::endl;
		}
	}

	return true;
}

#endif
