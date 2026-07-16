#ifndef CONFIG_HPP_SENTINEL
#define CONFIG_HPP_SENTINEL


#include "nlohmann_json.hpp"
#include <string>


class Config
{
public:

	// default config values
	struct GameSettings
	{
		int min_players_to_start		=			 2;
		int max_players					=			 8;
		int time_to_start				=			10;
		int start_market_level			=			 3;
		int start_money					=		 10000;
		int start_sources				=			 4;
		int start_products				=			 2;
		int start_factories				=			 2;
		int source_unit_charge			=		   300;
		int product_unit_charge			=		   500;
		int factory_unit_charge			=		  1000;
		int new_factory_unit_cost		=		  5000;
		int production_product_cost		=		  2000;
		int turns_to_build_factory		=			 5;
	};

	GameSettings game_settings;

	Config() {}
	bool Load( const std::string& );

private:
	Config( const Config& ) = delete;
	Config( Config&& ) = delete;
	void operator=( const Config& ) = delete;
};

#endif
