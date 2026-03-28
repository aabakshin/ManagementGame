#ifndef REGISTERED_PLAYERS_CPP_SENTINEL
#define REGISTERED_PLAYERS_CPP_SENTINEL


#include "RegisteredPlayers.hpp"


RegisteredPlayers::RegisteredPlayers()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = (*this)[i];
		p = new Player( -1, "", GetUIDByIdx(i) );
	}
}

RegisteredPlayers::~RegisteredPlayers()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = (*this)[i];
		if ( p != nullptr )
		{
			delete p;
			p = nullptr;
		}
	}
}

const Player* RegisteredPlayers::operator[]( unsigned int idx ) const
{
	if ( ( idx < 0 ) || ( idx > ( MAX_PLAYERS-1 ) ) )
	{
		return nullptr;
		// throw IndexOutOfRangeException();
	}

	return const_cast<const Player*>(registered_players[idx]);
}

const Player* RegisteredPlayers::GetPlayerByFd( int fd ) const
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = (*this)[i];
		if ( !p->IsFree() && ( p->GetFd() == fd ) )
			return const_cast<Player*>(p);
	}

	return nullptr;
}

const Player* RegisteredPlayers::GetPlayerByUID( int player_id ) const
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = (*this)[i];
		if ( !p->IsFree() && ( p->GetUID() == player_id ) )
			return const_cast<Player*>(p);
	}

	return nullptr;
}

int RegisteredPlayers::GetIdxByUID( int player_id ) const
{
	if ( ( player_id < 1 ) || ( player_id > MAX_PLAYERS ) )
		return -1;

	return player_id - 1;
}

const int RegisteredPlayers::GetUIDByIdx( int idx ) const
{
	if ( ( idx < 0 ) || ( idx >= MAX_PLAYERS ) )
		return -1;

	return idx + 1;
}

#endif
