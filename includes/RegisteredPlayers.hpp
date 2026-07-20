#ifndef REGISTERED_PLAYERS_HPP_SENTINEL
#define REGISTERED_PLAYERS_HPP_SENTINEL


#include "Config.hpp"
#include "Player.hpp"
#include <memory>


class RegisteredPlayers
{
private:
	std::shared_ptr<const Config::GameSettings> m_game_settings;
	Player** registered_players;
public:
	RegisteredPlayers( std::shared_ptr<const Config::GameSettings> );
	void ApplySettings( std::shared_ptr<const Config::GameSettings> );
	~RegisteredPlayers();
	const Player* operator[]( unsigned int idx ) const;
	const Player* GetPlayerByFd( int ) const;
	const Player* GetPlayerByUID( int ) const;
	int GetIdxByUID( int ) const;
	const int GetUIDByIdx( int ) const;
private:
	RegisteredPlayers( const RegisteredPlayers& ) = delete;
	RegisteredPlayers( RegisteredPlayers&& ) = delete;
	void operator=( const RegisteredPlayers& ) = delete;
};

#endif
