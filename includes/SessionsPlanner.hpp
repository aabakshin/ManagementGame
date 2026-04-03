#ifndef SESSIONS_PLANNER_HPP_SENTINEL
#define SESSIONS_PLANNER_HPP_SENTINEL


#include "Banker.hpp"


class SessionsPlanner
{
public:

	enum
	{
					DEFAULT_SESSIONS_COUNT					=			2,
					DEFAULT_NEXT_SESSION_ID					=			1,
					DEFAULT_ADDITIONAL_SESSIONS_COUNT		=			2
	};

private:
	static int next_session_id;
	Banker** game_sessions { nullptr };
	int current_sessions_count { 0 };
public:
	SessionsPlanner();
	void Make( int );
	const Banker* operator[]( int );
	const Banker* GetSessionById( int ) const;
	int GetSessionsCount() const { return current_sessions_count; }
	void AddSessions();
	~SessionsPlanner();
private:
	SessionsPlanner( const SessionsPlanner& ) = delete;
	SessionsPlanner( SessionsPlanner&& ) = delete;
	void operator=( const SessionsPlanner& ) = delete;
	int GetSessionIdxById( int ) const;
};

#endif
