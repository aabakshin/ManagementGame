#ifndef REGISTERED_PLAYERS_CPP_SENTINEL
#define REGISTERED_PLAYERS_CPP_SENTINEL


#include "RegisteredPlayers.hpp"
#include <stdexcept>


RegisteredPlayers::RegisteredPlayers( std::shared_ptr<const Config::GameSettings> m_g_sets )
{
	m_game_settings = std::move( m_g_sets );

	registered_players = new Player*[m_game_settings->max_players];

	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = registered_players[i];
		p = new Player( -1, "", GetUIDByIdx(i), m_game_settings );
	}
}

void RegisteredPlayers::ApplySettings( std::shared_ptr<const Config::GameSettings> m_g_sets )
{
	m_game_settings = std::move( m_g_sets );

	for ( int i = 0; i < m_game_settings->max_players; ++i )
		registered_players[i]->ApplySettings( m_game_settings );
}

RegisteredPlayers::~RegisteredPlayers()
{
	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = registered_players[i];
		if ( p != nullptr )
		{
			delete p;
			p = nullptr;
		}
	}

	delete[] registered_players;
}

const Player* RegisteredPlayers::operator[]( unsigned int idx ) const
{
	if ( ( idx < 0 ) || ( idx > ( m_game_settings->max_players-1 ) ) )
		throw std::runtime_error("IndexOutOfRange error in \"RegisteredPlayers::operator[]\" function!");

	return const_cast<const Player*>(registered_players[idx]);
}

const Player* RegisteredPlayers::GetPlayerByFd( int fd ) const
{
	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = registered_players[i];
		if ( !p->IsFree() )
		{
			if ( p->GetFd() == fd )
				return const_cast<Player*>(p);
		}
	}

	return nullptr;
}

const Player* RegisteredPlayers::GetPlayerByUID( int player_id ) const
{
	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = registered_players[i];
		if ( !p->IsFree() )
		{
			if ( p->GetUID() == player_id )
				return const_cast<Player*>(p);
		}
	}

	return nullptr;
}

int RegisteredPlayers::GetIdxByUID( int player_id ) const
{
	if ( ( player_id < 1 ) || ( player_id > m_game_settings->max_players ) )
		return -1;

	return player_id - 1;
}

const int RegisteredPlayers::GetUIDByIdx( int idx ) const
{
	if ( ( idx < 0 ) || ( idx >= m_game_settings->max_players ) )
		return -1;

	return idx + 1;
}

#endif
