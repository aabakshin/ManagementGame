#ifndef REGISTERED_PLAYERS_HPP_SENTINEL
#define REGISTERED_PLAYERS_HPP_SENTINEL


#include "Player.hpp"


class RegisteredPlayers
{
private:
	Player* registered_players[MAX_PLAYERS];
public:
	RegisteredPlayers();
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
