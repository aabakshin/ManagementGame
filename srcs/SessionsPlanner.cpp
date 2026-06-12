#ifndef SESSIONS_PLANNER_CPP_SENTINEL
#define SESSIONS_PLANNER_CPP_SENTINEL


#include "SessionsPlanner.hpp"
#include "MGLib.h"
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <list>


/* Список сообщений для идентификации бот-клиента */
static const char* const bot_identity_messages[] = {
				"./bot_mg_debug4",
				"./bot_mg_debug4\n",
				"./bot_mg_release4",
				"./bot_mg_release4\n",
				nullptr
};

// Описаны в модуле MGLib
extern const char* info_game_messages[];
extern const char* error_game_messages[];


SessionsPlanner::StartSessionsTimers::StartSessionTimer::StartSessionTimer()
{
	timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
	if ( timerfd == -1 )
	{
		perror("timerfd_create");
		//throw TimerCreateException();
	}

	timer_settings.it_value.tv_sec			=		0;
	timer_settings.it_value.tv_nsec			=		0;
	timer_settings.it_interval.tv_sec		=		0;
	timer_settings.it_interval.tv_nsec		=		0;

	UnsetLaunched();
	UnsetAlarmed();
}

SessionsPlanner::StartSessionsTimers::StartSessionTimer::~StartSessionTimer()
{
	close(timerfd);
}

void SessionsPlanner::StartSessionsTimers::StartSessionTimer::SetTimerSettings( uint64_t value_sec, uint64_t value_nsec, uint64_t interval_sec, uint64_t interval_nsec )
{
	timer_settings.it_value.tv_sec			=		value_sec;
	timer_settings.it_value.tv_nsec			=		value_nsec;
	timer_settings.it_interval.tv_sec		=		interval_sec;
	timer_settings.it_interval.tv_nsec		=		interval_nsec;
}

void SessionsPlanner::StartSessionsTimers::StartSessionTimer::GetTimerSettings( itimerspec* cur_value )
{
	if ( timerfd_gettime(timerfd, cur_value) == -1 )
	{
		perror("timerfd_gettime");
		close(timerfd);
		timerfd = -1;
		return;
		//throw GetTimerSettingsException();
	}
}

void SessionsPlanner::StartSessionsTimers::StartSessionTimer::StartTimer( uint64_t value_sec, uint64_t value_nsec, uint64_t interval_sec, uint64_t interval_nsec )
{
	SetTimerSettings( value_sec, value_nsec, interval_sec, interval_nsec );

	if ( timerfd_settime(timerfd, 0, &timer_settings, nullptr) == -1 )
	{
		perror("timerfd_settime");
		close(timerfd);
		timerfd = -1;
		return;
		//throw TimerStartException();
	}

	SetLaunched();
	UnsetAlarmed();

	/*printf("\n-----------------\n"
			"StartTimer[%d] has launched.\n"
			"Timer settings:\n"
			"\tValue settings: sec(%lu), nsec(%lu)\n"
			"\tInterval settings: sec(%lu), nsec(%lu)\n"
			"-----------------\n",
			timerfd,
			timer_settings.it_value.tv_sec,
			timer_settings.it_value.tv_nsec,
			timer_settings.it_interval.tv_sec,
			timer_settings.it_interval.tv_nsec);*/
}

void SessionsPlanner::StartSessionsTimers::StartSessionTimer::StopTimer()
{
	StartTimer( 0, 0, 0, 0 );
	UnsetLaunched();
	SetAlarmed();
}


SessionsPlanner::StartSessionsTimers::StartSessionTimer& SessionsPlanner::StartSessionsTimers::operator[]( int idx )
{
	if ( ( idx < 0 ) || ( idx >= DEFAULT_MAX_SESSIONS_COUNT ) )
	{
		return sessions_timers_fds[0];
		// throw IndexOutOfRangeException();
	}

	return sessions_timers_fds[idx];
}

int SessionsPlanner::StartSessionsTimers::GetTimerIdxById( int session_id ) const
{
	if ( ( session_id < 1 ) || ( session_id > DEFAULT_MAX_SESSIONS_COUNT ) )
	{
		return -1;
		// throw IndexOutOfRangeException();
	}

	return session_id - 1;
}

int SessionsPlanner::StartSessionsTimers::GetTimerIdxByFd( int fd ) const
{
	if ( fd < 0 )
		return -1;

	for ( int i = 0; i < DEFAULT_MAX_SESSIONS_COUNT; ++i )
		if ( sessions_timers_fds[i].GetTimerFd() == fd )
			return i;

	return -1;
}

bool SessionsPlanner::StartSessionsTimers::IsTimerFd( int fd ) const
{
	if ( fd < 0 )
		return false;

	for ( int i = 0; i < DEFAULT_MAX_SESSIONS_COUNT; ++i )
		if ( sessions_timers_fds[i].GetTimerFd() == fd )
			return true;

	return false;
}

void SessionsPlanner::StartSessionsTimers::ResetTimerFd( int session_id )
{
	int idx = GetTimerIdxById( session_id );

	if ( idx == -1 )
	{
		return;
		// throw IndexOutOfRangeException();
	}

	sessions_timers_fds[idx].StopTimer();
}

void SessionsPlanner::StartSessionsTimers::ResetTimers()
{
	for ( int i = 0; i < DEFAULT_MAX_SESSIONS_COUNT; ++i )
		sessions_timers_fds[i].StopTimer();
}


SessionsPlanner::SessionsPlanner()
{
	next_session_id = DEFAULT_NEXT_SESSION_ID;

	if ( next_session_id < 1 )
		next_session_id = 1;
}

void SessionsPlanner::Make( int sessions_count )
{
	msg_tokens.Make( MessageTokens::MESSAGE_TOKENS_COUNT );

	EBCbroker.Make( *this );
	EGameMessages.Make( *this );
	EMultiActionsExec.Make( *this, sender, msg_tokens, EGameMessages );
	EGameEvents.Make( *this, msg_tokens, EMultiActionsExec );

	if ( sessions_count < 1 )
		current_sessions_count = DEFAULT_START_SESSIONS_COUNT;
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
	if ( current_sessions_count + DEFAULT_ADDITIONAL_SESSIONS_COUNT > DEFAULT_MAX_SESSIONS_COUNT )
	{
		return;
		// throw ReachedMaxSessionsException();
	}

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

void SessionsPlanner::AddNewClientToSession( int cs, const char* new_client_addr )
{
	bool session_full = false;
	bool game_started = false;

	for ( int i = DEFAULT_NEXT_SESSION_ID; i <= GetSessionsCount(); ++i )
	{
		game_started = false;
		session_full = false;

		const Banker& banker = *GetSessionById( i );
		const Player* p = banker.GetFree();

		if ( banker.IsGameStarted() )
		{
			game_started = true;
			continue;
		}

		if ( p == nullptr )
		{
			session_full = true;
			continue;
		}

		try
		{
			const_cast<Player*>(p)->SetNewPlayer( cs, new_client_addr );
		}
		catch ( ... )
		{
			sender.SendMessage( error_game_messages[INTERNAL_SERVER_ERROR], cs, new_client_addr );
			sender.SetSentMsgsCount( sender.GetSentMsgsCount() + 1 );
			sender.ShowSentMessage();
			throw;
		}

		const_cast<Banker&>(banker).SetLobbyPlayers( banker.GetLobbyPlayers() + 1 );

		itoa( banker.GetId(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
		const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

		const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_NEW_PLAYER_CONNECT_TOKEN );
		break;
	}

	if ( game_started )
	{
		sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::GAME_ALREADY_STARTED_TOKEN ), cs, new_client_addr );
		sender.SetSentMsgsCount( sender.GetSentMsgsCount() + 1 );
		sender.ShowSentMessage();
		throw ErrorNewClientGameAlreadyStartedException();
	}

	if ( session_full )
	{
		sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::SERVER_FULL_TOKEN ), cs, new_client_addr );
		sender.SetSentMsgsCount( sender.GetSentMsgsCount() + 1 );
		sender.ShowSentMessage();
		throw ErrorNewClientServerFullException();
	}
}

bool SessionsPlanner::IsPlayerFd( int fd, std::pair<int,int>& player_pos ) const
{
	for ( int i = DEFAULT_NEXT_SESSION_ID; i <= GetSessionsCount(); ++i )
	{
		const Banker& banker = *GetSessionById( i );

		for ( int j = 0; j < MAX_PLAYERS; ++j )
		{
			const Player* p = banker.GetPlayers()[j];
			if ( !p->IsFree() )
			{
				player_pos.first = i;
				player_pos.second = j;
				return true;
			}
		}
	}

	player_pos.first = -1;
	player_pos.second = -1;

	return false;
}

std::list<int> SessionsPlanner::GetValidFdsList() const
{
	std::list<int> valid_fds;

	for ( int i = DEFAULT_NEXT_SESSION_ID; i <= GetSessionsCount(); ++i )
	{
		for ( int j = 0; j < MAX_PLAYERS; ++j )
		{
			const Player* p = GetSessionById( i )->GetPlayers()[j];
			if ( !p->IsFree() )
				valid_fds.push_back(p->GetFd());
		}
	}

	return valid_fds;
}

void SessionsPlanner::GameEventsHandle()
{
	for ( int i = DEFAULT_NEXT_SESSION_ID; i <= GetSessionsCount(); ++i )
	{
		const Banker& banker = *GetSessionById( i );

		if ( !banker.IsGameStarted() )
		{
			int t_idx = GetStartTimers().GetTimerIdxById( i );
			bool is_launched_flag = const_cast<StartSessionsTimers&>(GetStartTimers())[t_idx].IsLaunched();
			bool is_alarmed_flag = const_cast<StartSessionsTimers&>(GetStartTimers())[t_idx].IsAlarmed();

			if ( ( banker.GetLobbyPlayers() >= MIN_PLAYERS_TO_START ) && ( banker.GetLobbyPlayers() <= MAX_PLAYERS ) )
			{
				if ( !is_launched_flag )
				{
					const_cast<StartSessionsTimers&>(GetStartTimers())[t_idx].StartTimer( TIME_TO_START, 0, 0, 0 );

					itoa( banker.GetId(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameEvents::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
					const_cast<GameEvents&>(EGameEvents.GetBroker()).PutMessage( msg_tokens.GetValue(), GameEvents::SESSION_ID_PARAM_TOKEN+1 );
					const_cast<GameEvents&>(EGameEvents.GetBroker()).TakeMessage( GameEvents::INIT_START_EVENT_TOKEN );
				}
			}

			if ( is_alarmed_flag )
			{
				const_cast<StartSessionsTimers&>(GetStartTimers())[t_idx].UnsetAlarmed();

				itoa( banker.GetId(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameEvents::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
				const_cast<GameEvents&>(EGameEvents.GetBroker()).PutMessage( msg_tokens.GetValue(), GameEvents::SESSION_ID_PARAM_TOKEN+1 );
				const_cast<GameEvents&>(EGameEvents.GetBroker()).TakeMessage( GameEvents::CHECK_START_EVENT_TOKEN );
			}
		}
		else
		{
			if ( banker.GetReadyPlayers() == banker.GetAlivePlayers() )
			{
				itoa( banker.GetId(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameEvents::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
				const_cast<GameEvents&>(EGameEvents.GetBroker()).PutMessage( msg_tokens.GetValue(), GameEvents::SESSION_ID_PARAM_TOKEN+1 );
				const_cast<GameEvents&>(EGameEvents.GetBroker()).TakeMessage( GameEvents::END_GAME_TURN_EVENT_TOKEN );
				const_cast<GameEvents&>(EGameEvents.GetBroker()).TakeMessage( GameEvents::REPORT_ON_TURN_EVENT_TOKEN );
				const_cast<GameEvents&>(EGameEvents.GetBroker()).TakeMessage( GameEvents::PREPARE_NEW_TURN_EVENT_TOKEN );
			}
		}
	}

	if ( !sessions_bankrots.empty() )
	{
		SessionsPlanner::BankrotsList bankrots_copy = sessions_bankrots;
		sessions_bankrots.clear();
		throw KickBankrotsException( bankrots_copy );
	}
}

bool SessionsPlanner::IsCorrectIdentityMsg( const char* identity_msg )
{
	for ( int i = 0; bot_identity_messages[i] != nullptr; ++i )
		if ( strcmp(identity_msg, bot_identity_messages[i]) == 0 )
			return true;

	return false;
}

void SessionsPlanner::QuitAllPlayers( std::list<std::pair<int,std::string>>& players_fds )
{
	for ( int i = DEFAULT_NEXT_SESSION_ID; i <= GetSessionsCount(); ++i )
	{
		const Banker& banker = *GetSessionById( i );

		for ( int j = 0; j < MAX_PLAYERS; ++j )
		{
			const Player* p = banker.GetPlayers()[j];
			if ( !p->IsFree() )
			{
				std::pair<int,std::string> p_pair { p->GetFd(), p->GetAddr() };
				players_fds.push_back( p_pair );
			}
		}
	}
}

void SessionsPlanner::QuitPlayer( int session_id, int player_id )
{
	itoa( session_id, const_cast<char*>( const_cast<MessageTokens&>( msg_tokens ).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN] ), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	itoa( player_id, const_cast<char*>( const_cast<MessageTokens&>( msg_tokens ).GetValue()[MulticastActionsExec::LEFT_PLAYER_ID_PARAM_TOKEN] ), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>( EMultiActionsExec.GetBroker() ).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::LEFT_PLAYER_ID_PARAM_TOKEN+1 );

	const_cast<MulticastActionsExec&>( EMultiActionsExec.GetBroker() ).TakeMessage( MulticastActionsExec::QUIT_PLAYER_TOKEN );
}

void SessionsPlanner::PlayerEventHandle( const std::pair<int,int>& player_pos )
{
	int sid = player_pos.first;
	int uid = player_pos.second;

	if ( ( sid < 0 ) || ( uid < 0 ) )
	{
		return;
		//throw InvalidIdException();
	}

	const Banker& banker = *GetSessionById( sid );
	const Player* p = banker.GetPlayers().GetPlayerByUID( uid );

	int p_fd = p->GetFd();
	const char* p_addr = p->GetAddr();

	receiver.RecvMessage( p_fd, p_addr );
	receiver.SetRecvMsgsCount( receiver.GetRecvMsgsCount() + 1 );
	receiver.ShowReceivedMessage();

	if ( receiver.GetRecvBytes() > 0 )
	{
		const_cast<Player*>(p)->SetMessageBuffer( receiver.GetMessage(), receiver.GetMessageLength() );

		if ( !banker.IsGameStarted() )
		{
			if ( !p->IsIdentMsgRecv() )
			{
				if ( IsCorrectIdentityMsg( p->GetMessageBuffer() ) )
					const_cast<Player*>(p)->SetBot();
				else
					const_cast<Player*>(p)->UnsetBot();

				const_cast<Player*>(p)->SetIdentMsgRecv();
			}
			else
			{
				itoa( banker.GetId(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
				const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

				sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::GAME_NOT_STARTED_TOKEN ), p_fd, p_addr );
				sender.SetSentMsgsCount( sender.GetSentMsgsCount() + 1 );
				sender.ShowSentMessage();
			}
		}
		else
		{
			// Обработка данных от игрока, когда игра началась
			cmds_exec.ProcessCommand( banker.GetId(), p->GetMessageBuffer(), p->GetUID(), EBCbroker.GetBroker() );
			sender.SendMessage( cmds_exec.GetCmdResultTokens(), cmds_exec.GetCmdResultTokensAmount(), p_fd, p_addr );
			sender.SetSentMsgsCount( sender.GetSentMsgsCount() + 1 );
			sender.ShowSentMessage();

			const char* info_token = cmds_exec.GetCmdToken( 0 );
			if ( strcmp(info_token, info_game_messages[QUIT_COMMAND_SUCCESS]) == 0 )
			{
				QuitPlayer( sid, uid );
				throw QuitCommandSuccessException( p_fd, p_addr );
			}
			else if ( strcmp(info_token, error_game_messages[INTERNAL_SERVER_ERROR]) == 0 )
			{
				QuitPlayer( sid, uid );
				throw InternalServerErrorException( p_fd, p_addr );
			}
		}
		return;
	}

	QuitPlayer( sid, uid );
	throw LostConnectionException( p_fd, p_addr );
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
