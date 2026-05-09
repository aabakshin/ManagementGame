#ifndef SESSIONS_PLANNER_HPP_SENTINEL
#define SESSIONS_PLANNER_HPP_SENTINEL


#include "Banker.hpp"
#include "CommandExecutor.hpp"
#include "BrokerMessages.hpp"
#include "Sender.hpp"
#include "Receiver.hpp"
#include <sys/timerfd.h>
#include <cstdint>
#include <utility>
#include <list>


class MessageTokens;
class MulticastActionsExec;


class SessionsPlanner
{
public:

	enum
	{
					DEFAULT_START_SESSIONS_COUNT					=			2,
					DEFAULT_NEXT_SESSION_ID							=			1,
					DEFAULT_ADDITIONAL_SESSIONS_COUNT				=			2,
					DEFAULT_MAX_SESSIONS_COUNT						=			8
	};

	typedef std::list<std::pair<int, std::string>> BankrotsList;

	class StartSessionsTimers
	{
	private:

		class StartSessionTimer
		{
		private:
			int timerfd;
			itimerspec timer_settings;
		public:
			StartSessionTimer();
			~StartSessionTimer();
			void StartTimer( uint64_t, uint64_t, uint64_t, uint64_t );
			void StopTimer();
			int GetTimerFd() const { return timerfd; }
		private:
			StartSessionTimer( const StartSessionTimer& ) = delete;
			StartSessionTimer( StartSessionTimer&& ) = delete;
			void operator=( const StartSessionTimer& ) = delete;
			void SetTimerSettings( uint64_t, uint64_t, uint64_t, uint64_t );
			void GetTimerSettings( itimerspec* );
		};

		StartSessionTimer sessions_timers_fds[DEFAULT_MAX_SESSIONS_COUNT];
	public:
		StartSessionsTimers() {}
		StartSessionTimer& operator[]( int );
		bool IsTimerFd( int ) const;
		void ResetTimerFd( int );
		void ResetTimers();
	private:
		StartSessionsTimers( const StartSessionsTimers& ) = delete;
		StartSessionsTimers( StartSessionsTimers&& ) = delete;
		void operator=( const StartSessionsTimers& ) = delete;
		int GetTimerIdxById( int ) const;
	};

private:
	static int next_session_id;
	Banker** game_sessions { nullptr };
	int current_sessions_count { };
	StartSessionsTimers start_timers;
	CommandExecutor cmds_exec;
	EncapsulatedBrokerMessages<BCBrokerMessages,SessionsPlanner> EBCbroker;
	EncapsulatedBrokerMessages<GameMessages,SessionsPlanner> EGameMessages;
	EncapsulatedBrokerMessages<MulticastActionsExec,SessionsPlanner> EMultiActionsExec;
	Sender sender;
	Receiver receiver;
	MessageTokens msg_tokens;
	BankrotsList sessions_bankrots;
public:
	SessionsPlanner();
	~SessionsPlanner();
	void Make( int );
	const Banker* operator[]( int );
	const Banker* GetSessionById( int ) const;
	int GetSessionsCount() const { return current_sessions_count; }
	const StartSessionsTimers& GetStartTimers() const { return start_timers; }
	std::list<int> GetValidFdsList() const;
	const BankrotsList& GetBankrotsList() const { return sessions_bankrots; }
	void AddSessions();
	void AddNewClientToSession( int, const char* );
	bool IsCorrectIdentityMsg( const char* );
	bool IsPlayerFd( int, std::pair<int,int>& ) const;
	void PlayerEventHandle( const std::pair<int,int>& );
	void QuitPlayer( int, int );
	void QuitAllPlayers( std::list<std::pair<int, std::string>>& );
	void ShowAuctionInfo( int, const char*, const Item<MarketData>* );

	void EndGameTurnEvent( int );
	void PrepareGameStateEvent( int );
	void InitStartEvent( int );
	void CheckStartEvent( int );
	void ReportOnTurnEvent( int );
	void PrepareNewTurnEvent( int );
	void GameEventsHandle();
private:
	SessionsPlanner( const SessionsPlanner& ) = delete;
	SessionsPlanner( SessionsPlanner&& ) = delete;
	void operator=( const SessionsPlanner& ) = delete;
	int GetSessionIdxById( int ) const;
};

#endif
