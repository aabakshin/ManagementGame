#ifndef SESSIONS_PLANNER_CPP_SENTINEL
#define SESSIONS_PLANNER_CPP_SENTINEL


#include "SessionsPlanner.hpp"


SessionsPlanner::SessionsPlanner()
{
	next_session_id = DEFAULT_NEXT_SESSION_ID;

	if ( next_session_id < 1 )
		next_session_id = 1;
}

void SessionsPlanner::Make( int sessions_count )
{
	if ( sessions_count < 1 )
		current_sessions_count = DEFAULT_SESSIONS_COUNT;
	else
		current_sessions_count = sessions_count;

	game_sessions = new Banker*[ current_sessions_count ];

	for ( int i = 0; i < current_sessions_count; ++i )
	{
		game_sessions[i] = new Banker( next_session_id );
		++next_session_id;
	}
}

void SessionsPlanner::AddSessions()
{
	int prev_sessions_count = current_sessions_count;
	current_sessions_count += DEFAULT_ADDITIONAL_SESSIONS_COUNT;

	Banker** temp = new Banker*[ current_sessions_count ];

	for ( int i = 0; i < current_sessions_count; ++i )
	{
		if ( i < prev_sessions_count )
		{
			temp[i] = game_sessions[i];
		}
		else
		{
			temp[i] = new Banker( next_session_id );
			++next_session_id;
		}
	}

	delete[] game_sessions;
	game_sessions = temp;
}

const Banker* SessionsPlanner::operator[]( int index )
{
	if ( ( index < 0 ) || ( index >= current_sessions_count ) )
	{
		return nullptr;
		// throw InvalidIndexException();
	}

	return game_sessions[index];
}

const Banker* SessionsPlanner::GetSessionById( int sid ) const
{
	if ( ( sid < 1 ) || ( sid > current_sessions_count ) )
		return nullptr;

	int idx = GetSessionIdxById( sid );

	return game_sessions[idx];
}

int SessionsPlanner::GetSessionIdxById( int sid ) const
{
	if ( ( sid < 1 ) || ( sid > current_sessions_count ) )
		return -1;

	return sid - 1;
}

SessionsPlanner::~SessionsPlanner()
{
	if ( game_sessions == nullptr )
		return;

	for ( int i = 0; i < current_sessions_count; ++i )
		if ( game_sessions[i] != nullptr )
			delete game_sessions[i];

	delete[] game_sessions;
	game_sessions = nullptr;
}

#endif
