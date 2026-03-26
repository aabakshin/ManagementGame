#ifndef REGISTERED_PLAYERS_HPP_SENTINEL
#define REGISTERED_PLAYERS_HPP_SENTINEL

#include "Player.hpp"

class RegisteredPlayers
{
private:
	Player* registered_players[MAX_PLAYERS];
	int players_count;
public:
	RegisteredPlayers();
	~RegisteredPlayers();
	const Player* operator[]( unsigned int idx ) const;
	int GetPlayersCount() const { return players_count; }
	const Player* GetPlayerByFd( int ) const;
	const Player* GetPlayerByNum( int ) const;
	int GetIdxByNum( int ) const;
	int GetNumByIdx( int ) const;
private:
	RegisteredPlayers( const RegisteredPlayers& ) {}
	RegisteredPlayers( RegisteredPlayers&& ) {}
	void operator=( const RegisteredPlayers& ) {}
};

#endif
