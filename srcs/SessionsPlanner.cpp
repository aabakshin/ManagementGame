#ifndef SESSIONS_PLANNER_CPP_SENTINEL
#define SESSIONS_PLANNER_CPP_SENTINEL


#include "SessionsPlanner.hpp"
#include "BrokerMessages.hpp"
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

// Описаны в модуле Banker
extern const double amount_multiplier_table[MARKET_LEVEL_NUMBER][2];
extern const int price_table[MARKET_LEVEL_NUMBER][2];
extern const int states_market_chance[MARKET_LEVEL_NUMBER][MARKET_LEVEL_NUMBER];


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
	EBCbroker.Make( *this );
	EGameMessages.Make( *this );
	EMultiActionsExec.Make( *this, sender, msg_tokens, EGameMessages );
	msg_tokens.Make( MessageTokens::MESSAGE_TOKENS_COUNT );

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
	bool server_full = false;
	bool game_started = false;

	for ( int i = DEFAULT_NEXT_SESSION_ID; i <= GetSessionsCount(); ++i )
	{
		game_started = false;
		server_full = false;

		const Banker& banker = *GetSessionById( i );
		const Player* p = banker.GetFree();

		if ( banker.IsGameStarted() )
		{
			game_started = true;
			continue;
		}

		if ( p == nullptr )
		{
			server_full = true;
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

	if ( server_full )
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

void SessionsPlanner::ShowAuctionInfo( int session_id, const char* auction_type_msg, const Item<MarketData>* node )
{
	const Banker& banker = *GetSessionById( session_id );

	printf("\n%s\n", auction_type_msg);

	for ( ; node != nullptr; node = node->GetNext() )
	{
		const Player* p = banker.GetPlayers().GetPlayerByUID(node->GetData().GetPlayerNum());
		printf("Request of Player #%d:\n", p->GetUID());
		printf("\tPrice: %d\n\tAmount: %d\n\tIs proceed: %s\n\n",
				node->GetData().GetPrice(),
				(node->GetData().IsSuccess()) ? p->GetAuctionReport().GetSoldSources() : node->GetData().GetAmount(),
				node->GetData().IsSuccess() ? "yes" : "no");
	}
}

void SessionsPlanner::ReportOnTurnEvent( int session_id )
{
	const Banker& banker = *GetSessionById( session_id );

	printf("\n\n\n<<<<<<<<<< Report on Month #%d >>>>>>>>>>\n", banker.GetTurnNumber());
	printf("\n%s\n", "Players statistics:");

	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_REPORT_ON_TURN_TOKEN );

	ShowAuctionInfo( session_id, "Sources auction", const_cast<Banker&>(banker).GetSourcesRequests().GetFirst());
	ShowAuctionInfo( session_id, "Products auction", const_cast<Banker&>(banker).GetProductsRequests().GetFirst());

	printf("\n<<<<<<<<<< Report on Month #%d >>>>>>>>>>\n", banker.GetTurnNumber());
}

void SessionsPlanner::ChangeMarketState( int session_id )
{
	const Banker& banker = *GetSessionById( session_id );

	int r = 1 + (int)( 12.0 * rand() / (RAND_MAX + 1.0) );

	int sum = 0;
	int i;
	for ( i = 0; i < MARKET_LEVEL_NUMBER; i++ )
	{
		sum += states_market_chance[banker.GetCurrentMarketLvl()-1][i];
		if ( sum >= r )
			break;
	}

	if ( i < MARKET_LEVEL_NUMBER )
		const_cast<Banker&>(banker).SetCurrentMarketLvl( i+1 );

	const_cast<Banker&>(banker).GetCurrentMarketState().SetSourcesAmount( amount_multiplier_table[banker.GetCurrentMarketLvl()-1][0] * banker.GetAlivePlayers() );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetSourceMinPrice( price_table[banker.GetCurrentMarketLvl()-1][0] );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetProductsAmount( amount_multiplier_table[banker.GetCurrentMarketLvl()-1][1] * banker.GetAlivePlayers() );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetProductMaxPrice( price_table[banker.GetCurrentMarketLvl()-1][1] );
}

bool SessionsPlanner::CheckPlayersReports( int session_id, List<Item<MarketData>>& requests, int auction_type )
{
	// если список заявок на аукцион пуст, то не нужно его проводить
	if ( requests.IsEmpty() )
		return false;

	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	itoa( auction_type, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::AUCTION_TYPE_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::AUCTION_TYPE_PARAM_TOKEN+1 );

	// Если какой-либо игрок не заявился на аукцион, добавить его пустую заявку
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::ADD_EMPTY_AUCTION_REQUEST_TOKEN );

	return true;
}

void SessionsPlanner::SortRequestsByPrice( int session_id, const List<Item<MarketData>>& requests, List<Item<MarketData>>& sorted_requests, int auction_type )
{
	const int ready_players = (*GetSessionById( session_id )).GetReadyPlayers();

	Item<MarketData>* arr_reqs[ready_players];
	int prices[ready_players];
	bool reqs_checked[ready_players];


	Item<MarketData>* request = requests.GetFirst();
	for ( int i = 0; request != nullptr; request = request->GetNext(), ++i )
	{
		arr_reqs[i] = request;
		prices[i] = arr_reqs[i]->GetData().GetPrice();
		reqs_checked[i] = false;
	}

	heap_sort(prices, ready_players, ( auction_type == SOURCE_AUCTION ) ? 1 : 0 );

	int j = 0;
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		if ( (arr_reqs[i]->GetData().GetPrice() == prices[j]) && !reqs_checked[i] )
		{
			MarketData data;
			data.MakeData( arr_reqs[i]->GetData().GetPlayerNum(), arr_reqs[i]->GetData().GetAmount(), arr_reqs[i]->GetData().GetPrice() );

			sorted_requests.Insert( data );
			reqs_checked[i] = true;
			i = 0;
			++j;
			if ( j == ready_players )
				break;

			continue;
		}
	}
}

void SessionsPlanner::StartAuction( int session_id, const List<Item<MarketData>>& requests, int auction_type )
{
	if ( !CheckPlayersReports( session_id, const_cast<List<Item<MarketData>>&>(requests), auction_type ) )
		return;

	List<Item<MarketData>> sorted_requests;
	SortRequestsByPrice( session_id, requests, sorted_requests, auction_type );

	const Banker& banker = *GetSessionById( session_id );

	int max_sources = const_cast<Banker&>(banker).GetCurrentMarketState().GetSourcesAmount();
	int max_products = const_cast<Banker&>(banker).GetCurrentMarketState().GetProductsAmount();

	for ( Item<MarketData>* node = sorted_requests.GetFirst(); node != nullptr; node = node->GetNext() )
	{
		if ( node->GetData().GetPrice() < 1 )
			continue;

		const Player* cur_p = banker.GetPlayers().GetPlayerByUID( node->GetData().GetPlayerNum() );

		if ( cur_p->IsFree() )
			continue;

		if ( node->GetData().GetAmount() <= ( ( auction_type == SOURCE_AUCTION ) ? max_sources : max_products ) )
		{
			if ( node->GetData().GetAmount() > 0 )
			{
				if ( auction_type == SOURCE_AUCTION )
				{
					const_cast<Player*>(cur_p)->SetMoney( cur_p->GetMoney() - node->GetData().GetAmount() * node->GetData().GetPrice() );
					const_cast<Player*>(cur_p)->SetSources( cur_p->GetSources() + node->GetData().GetAmount() );
					max_sources -= node->GetData().GetAmount();
				}
				else
				{
					const_cast<Player*>(cur_p)->SetMoney( cur_p->GetMoney() + node->GetData().GetAmount() * node->GetData().GetPrice() );
					const_cast<Player*>(cur_p)->SetProducts( cur_p->GetProducts() - node->GetData().GetAmount() );
					max_products -= node->GetData().GetAmount();
				}

				const_cast<MarketData&>(node->GetData()).SetSuccess();

				if ( auction_type == SOURCE_AUCTION )
				{
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetSoldSources(node->GetData().GetAmount());
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetSoldPrice(node->GetData().GetPrice());
				}
				else
				{
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetBoughtProducts(node->GetData().GetAmount());
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetBoughtPrice(node->GetData().GetPrice());
				}
			}
		}
		else
		{
			int saved_max_sources = 0;
			int saved_max_products = 0;

			if ( ( ( auction_type == SOURCE_AUCTION ) ? max_sources : max_products) > 0 )
			{
				if ( auction_type == SOURCE_AUCTION )
				{
					const_cast<Player*>(cur_p)->SetMoney( cur_p->GetMoney() - max_sources * node->GetData().GetPrice() );
					const_cast<Player*>(cur_p)->SetSources( cur_p->GetSources() + max_sources );
				}
				else
				{
					const_cast<Player*>(cur_p)->SetMoney( cur_p->GetMoney() + max_products * node->GetData().GetPrice() );
					const_cast<Player*>(cur_p)->SetProducts( cur_p->GetProducts() - max_products );
				}

				const_cast<MarketData&>(node->GetData()).SetSuccess();

				if ( auction_type == SOURCE_AUCTION )
				{
					saved_max_sources = max_sources;
					max_sources = 0;
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetSoldSources(saved_max_sources);
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetSoldPrice(node->GetData().GetPrice());
				}
				else
				{
					saved_max_products = max_products;
					max_products = 0;
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetBoughtProducts(saved_max_products);
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetBoughtPrice(node->GetData().GetPrice());
				}
			}
		}
	}
}

void SessionsPlanner::PrepareNewTurnEvent( int session_id )
{
	const Banker& banker = *GetSessionById( session_id );

	ChangeMarketState( session_id );
	const_cast<Banker&>(banker).SetTurnNumber( banker.GetTurnNumber() + 1 );
	const_cast<Banker&>(banker).SetReadyPlayers( 0 );

	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::PREPARE_NEW_TURN_TOKEN );
}

void SessionsPlanner::InitStartEvent( int session_id )
{
	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_START_TIME_TOKEN );
}

void SessionsPlanner::CheckStartEvent( int session_id )
{
	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

	const Banker& banker = *GetSessionById(session_id);

	if (  banker.GetLobbyPlayers() < MIN_PLAYERS_TO_START )
	{
		const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_START_CANCELLED_TOKEN );
	}
	else
	{
		const_cast<Banker&>(banker).SetGameStarted();
		const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_GAME_STARTED_TOKEN );
	}
}

void SessionsPlanner::PrepareGameStateEvent( int session_id )
{
	const Banker& banker = *GetSessionById( session_id );

	const_cast<Banker&>(banker).SetAlivePlayers( banker.GetLobbyPlayers() );
	const_cast<Banker&>(banker).SetLobbyPlayers( 0 );
	const_cast<Banker&>(banker).SetTurnNumber( 1 );
	const_cast<Banker&>(banker).SetCurrentMarketLvl( START_MARKET_LEVEL );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetSourcesAmount( amount_multiplier_table[START_MARKET_LEVEL-1][0] * banker.GetAlivePlayers() );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetSourceMinPrice( price_table[START_MARKET_LEVEL-1][0] );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetProductsAmount( amount_multiplier_table[START_MARKET_LEVEL-1][1] * banker.GetAlivePlayers() );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetProductMaxPrice( price_table[START_MARKET_LEVEL-1][1] );

	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::PREPARE_PLAYERS_STATE_TOKEN );

	const_cast<Banker&>(banker).SetGameStatePrepared();
}

void SessionsPlanner::EndGameTurnEvent( int session_id, std::list<std::pair<int,std::string>>& b_list )
{
	const Banker& banker = *GetSessionById( session_id );

	StartAuction( session_id, const_cast<Banker&>(banker).GetSourcesRequests(), SOURCE_AUCTION );
	StartAuction( session_id, const_cast<Banker&>(banker).GetProductsRequests(), PRODUCTION_AUCTION );

	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_AUCTIONS_RESULTS_TOKEN );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::CHECK_BUILDING_FACTORIES_TOKEN );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::PAY_CHARGES_TOKEN );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_PLAYERS_BANKROT_TOKEN );
	//const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::QUIT_BANKROT_PLAYERS_TOKEN );


	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = banker.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			if ( p->IsBankrot() )
			{
				QuitPlayer( session_id, p->GetUID() );
				std::pair<int, std::string> bankrot_player { p->GetFd(), p->GetAddr() };
				b_list.push_back(bankrot_player);
			}
		}
	}
}

void SessionsPlanner::GameEventsHandle( std::list<std::pair<int,std::string>>& b_list )
{
	for ( int i = DEFAULT_NEXT_SESSION_ID; i <= GetSessionsCount(); ++i )
	{
		const Banker& banker = *GetSessionById( i );

		if ( !banker.IsGameStarted() )
		{
			if ( !IsTimerFlag() && ( banker.GetLobbyPlayers() >= MIN_PLAYERS_TO_START ) && ( banker.GetLobbyPlayers() <= MAX_PLAYERS ) )
			{
				InitStartEvent( banker.GetId() );
			}

			if ( IsAlrmFlag() )
			{
				CheckStartEvent( banker.GetId() );
			}
		}
		else
		{
			if ( !banker.IsGameStatePrepared() )
			{
				PrepareGameStateEvent( banker.GetId() );
			}

			if ( banker.GetReadyPlayers() == banker.GetAlivePlayers() )
			{
				EndGameTurnEvent( banker.GetId(), b_list );
				//ReportOnTurnEvent( banker.GetId() );
				PrepareNewTurnEvent( banker.GetId() );
			}
		}
	}
}

bool SessionsPlanner::IsCorrectIdentityMsg( const char* identity_msg )
{
	for ( int i = 0; bot_identity_messages[i] != nullptr; ++i )
		if ( strcmp(identity_msg, bot_identity_messages[i]) == 0 )
			return true;

	return false;
}

void SessionsPlanner::QuitPlayer( int session_id, int player_number )
{
	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	itoa( player_number, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::LEFT_PLAYER_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::LEFT_PLAYER_ID_PARAM_TOKEN+1 );

	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::QUIT_PLAYER_TOKEN );
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
	const Player* p = banker.GetPlayers().GetPlayerByUID(uid);

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
	{
		if ( game_sessions[i] != nullptr )
			delete game_sessions[i];
	}

	delete[] game_sessions;
	game_sessions = nullptr;
}

#endif
