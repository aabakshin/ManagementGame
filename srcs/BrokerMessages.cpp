#ifndef BROKER_MESSAGES_CPP_SENTINEL
#define BROKER_MESSAGES_CPP_SENTINEL


#include "Banker.hpp"
#include "BrokerMessages.hpp"
#include "SessionsPlanner.hpp"
#include "MGProto.hpp"
#include "Utility.hpp"
#include <cstring>

#ifdef DEBUG_MODE
	#include <iostream>
#endif

#include <stdexcept>


static const char* const true_str = "true";
static const char* const false_str = "false";


template <class T, class U>
void EncapsulatedBrokerMessages<T,U>::Make( const U& context_object, std::shared_ptr<const Config::GameSettings> m_g_sets )
{
	m_game_settings = std::move(m_g_sets);
	brokerPTR = new T( context_object, m_game_settings );
}

template <class T, class U>
template <class X, class Y, class Z>
void EncapsulatedBrokerMessages<T,U>::Make( const U& context_object1, const X& context_object2, const Y& context_object3, const Z& context_object4, std::shared_ptr<const Config::GameSettings> m_g_sets  )
{
	m_game_settings = std::move(m_g_sets);
	brokerPTR = new T( context_object1, context_object2, context_object3, context_object4, m_game_settings );
}

template <class T, class U>
template <class X, class Y>
void EncapsulatedBrokerMessages<T,U>::Make( const U& context_object1, const X& context_object2, const Y& context_object3, std::shared_ptr<const Config::GameSettings> m_g_sets  )
{
	m_game_settings = std::move(m_g_sets);
	brokerPTR = new T( context_object1, context_object2, context_object3, m_game_settings );
}

template <class T, class U>
const T& EncapsulatedBrokerMessages<T,U>::GetBroker() const
{
	return const_cast<const T&>(*brokerPTR);
}

template <class T, class U>
void EncapsulatedBrokerMessages<T,U>::ApplySettings( std::shared_ptr<const Config::GameSettings> m_g_sets )
{
	m_game_settings = std::move(m_g_sets);
}

template <class T, class U>
EncapsulatedBrokerMessages<T,U>::~EncapsulatedBrokerMessages()
{
	delete brokerPTR;
}

void BrokerMessages::BrokerActions::Make( int a_count )
{
	actions_count = a_count;

	actions = new std::function<void()>[ actions_count ];
}

std::function<void()>& BrokerMessages::BrokerActions::operator[]( int idx )
{
	if ( ( idx < 0 ) || ( idx > actions_count-1 ) )
		throw std::range_error("RangeError in \"BrokerMessages::BrokerActions::operator[]\" function!");

	return actions[idx];
}

BrokerMessages::BrokerActions::~BrokerActions()
{
	if ( actions != nullptr )
		delete[] actions;
}

const char* BrokerMessages::TakeMessage( int message_code )
{
	try
	{
		CheckMessageCode( message_code );

		broker_actions[message_code]();
	}
	catch ( const std::runtime_error& ex )
	{
		throw;
	}

	return result_message;
}


GameEvents::GameEvents( const SessionsPlanner& sessions, const MessageTokens& mt, const EncapsulatedBrokerMessages<MulticastActionsExec, SessionsPlanner>& emae, std::shared_ptr<const Config::GameSettings> m_g_sets )
	: BrokerMessages( m_g_sets ), game_sessions( sessions ), msg_tokens( mt ), EMultiActionsExec( emae )
{
	BrokerActions& br_acts = const_cast<BrokerActions&>(GetBrokerActions());
	br_acts.Make( GameEvents::BROKER_ACTIONS_COUNT );

	session_id				=			0;

	memset( result_message, 0, MESSAGE_SIZE );

	br_acts[END_GAME_TURN_EVENT_TOKEN]							=					[this]()	{	EndGameTurnEvent();			};
	br_acts[INIT_START_EVENT_TOKEN]								=					[this]() {	InitStartEvent();			};
	br_acts[CHECK_START_EVENT_TOKEN]							=					[this]() {	CheckStartEvent();			};
	br_acts[REPORT_ON_TURN_EVENT_TOKEN]							=					[this]() {	ReportOnTurnEvent();		};
	br_acts[PREPARE_NEW_TURN_EVENT_TOKEN]						=					[this]() {	PrepareNewTurnEvent();		};
}

void GameEvents::PutMessage( const char** message_tokens, int tokens_count )
{
	for ( int i = SESSION_ID_PARAM_TOKEN; i < tokens_count; ++i )
	{
		if ( ( message_tokens[i] != nullptr ) && ( strcmp(message_tokens[i], "") != 0 ) )
		{
			switch ( i )
			{
				case SESSION_ID_PARAM_TOKEN:
					session_id = atoi(message_tokens[i]);
			}
			break;
		}
	}
}

void GameEvents::CheckMessageCode( int message_code ) const
{
	for ( int i = END_GAME_TURN_EVENT_TOKEN; i <= PREPARE_NEW_TURN_EVENT_TOKEN; ++i )
		if ( message_code == i )
			return;

	throw std::range_error("RangeError in \"GameEvents::CheckMessageCode\" function!");
}

void GameEvents::EndGameTurnEvent()
{
	Utility::itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	Utility::itoa( SOURCE_AUCTION, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::AUCTION_TYPE_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::AUCTION_TYPE_PARAM_TOKEN+1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::START_AUCTION_TOKEN );


	Utility::itoa( PRODUCTION_AUCTION, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::AUCTION_TYPE_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::AUCTION_TYPE_PARAM_TOKEN+1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::START_AUCTION_TOKEN );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_AUCTIONS_RESULTS_TOKEN );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::CHECK_BUILDING_FACTORIES_TOKEN );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::PAY_CHARGES_TOKEN );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_PLAYERS_BANKROT_TOKEN );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::QUIT_BANKROT_PLAYERS_TOKEN );
}

void GameEvents::InitStartEvent()
{
	Utility::itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_START_TIME_TOKEN );
}

void GameEvents::CheckStartEvent()
{
	Utility::itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::CHECK_START_TOKEN );
}

void GameEvents::ReportOnTurnEvent()
{
	Utility::itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_REPORT_ON_TURN_TOKEN );

	Utility::itoa( SOURCE_AUCTION, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::AUCTION_TYPE_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::AUCTION_TYPE_PARAM_TOKEN+1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SHOW_REPORT_ON_TURN_TOKEN );

	Utility::itoa( PRODUCTION_AUCTION, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::AUCTION_TYPE_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::AUCTION_TYPE_PARAM_TOKEN+1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SHOW_REPORT_ON_TURN_TOKEN );
}

void GameEvents::PrepareNewTurnEvent()
{
	Utility::itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::PREPARE_NEW_TURN_TOKEN );
}


MulticastActionsExec::MulticastActionsExec( const SessionsPlanner& sessions, const Sender& s, const MessageTokens& mt, const EncapsulatedBrokerMessages<GameMessages, SessionsPlanner>& egm, std::shared_ptr<const Config::GameSettings> m_g_sets )
	: BrokerMessages( m_g_sets ), game_sessions( sessions ), sender( s ), msg_tokens( mt ), EGameMessages( egm )
{
	BrokerActions& br_acts = const_cast<BrokerActions&>(GetBrokerActions());
	br_acts.Make( MulticastActionsExec::BROKER_ACTIONS_COUNT );

	session_id				=			0;
	auction_type			=			0;
	left_player_id			=			0;

	memset( result_message, 0, MESSAGE_SIZE );

	br_acts[SEND_REPORT_ON_TURN_TOKEN]							=					[this]() {	SendReportOnTurn();			};
	br_acts[ADD_EMPTY_AUCTION_REQUEST_TOKEN]					=					[this]() {	AddEmptyAuctionRequest();	};
	br_acts[PAY_CHARGES_TOKEN]									=					[this]() {	PayCharges();				};
	br_acts[CHECK_BUILDING_FACTORIES_TOKEN]						=					[this]() {	CheckBuildingFactories();	};
	br_acts[PREPARE_NEW_TURN_TOKEN]								=					[this]() {	PrepareNewTurn();			};
	br_acts[PREPARE_PLAYERS_STATE_TOKEN]						=					[this]() {	PreparePlayersState();		};
	br_acts[SEND_AUCTIONS_RESULTS_TOKEN]						=					[this]() {	SendAuctionsResults();		};
	br_acts[SEND_PLAYERS_BANKROT_TOKEN]							=					[this]() {	SendPlayersBankrot();		};
	br_acts[SEND_NEW_PLAYER_CONNECT_TOKEN]						=					[this]() {	SendNewPlayerConnect();		};
	br_acts[SEND_START_TIME_TOKEN]								=					[this]() {	SendStartTime();			};
	br_acts[SEND_START_CANCELLED_TOKEN]							=					[this]() {	SendStartCancelled();		};
	br_acts[SEND_GAME_STARTED_TOKEN]							=					[this]() {	SendGameStarted();			};
	br_acts[QUIT_PLAYER_TOKEN]									=					[this]() {	QuitPlayer();				};
	br_acts[QUIT_BANKROT_PLAYERS_TOKEN]							=					[this]() {	QuitBankrotPlayers();		};
	br_acts[START_AUCTION_TOKEN]								=					[this]() {	StartAuction();				};
	br_acts[PREPARE_SESSION_STATE_TOKEN]						=					[this]() {	PrepareSessionState();		};
	br_acts[SHOW_REPORT_ON_TURN_TOKEN]							=					[this]() {	ShowReportOnTurn();			};
	br_acts[CHECK_START_TOKEN]									=					[this]() {	CheckStart();				};
}

void MulticastActionsExec::PutMessage( const char** message_tokens, int tokens_count )
{
	for ( int i = SESSION_ID_PARAM_TOKEN; i < tokens_count; ++i )
	{
		if ( ( message_tokens[i] != nullptr ) && ( strcmp(message_tokens[i], "") != 0 ) )
		{
			switch ( i )
			{
				case SESSION_ID_PARAM_TOKEN:
					session_id = atoi(message_tokens[i]);
					break;
				case AUCTION_TYPE_PARAM_TOKEN:
					auction_type = atoi(message_tokens[i]);
					break;
				case LEFT_PLAYER_ID_PARAM_TOKEN:
					left_player_id = atoi(message_tokens[i]);
					break;
			}
			break;
		}
	}
}

void MulticastActionsExec::CheckMessageCode( int message_code ) const
{
	for ( int i = SEND_REPORT_ON_TURN_TOKEN; i <= QUIT_PLAYER_TOKEN; ++i )
		if ( message_code == i )
			return;

	throw std::range_error("RangeError in \"MulticastActionsExec::CheckMessageCode\" function!");
}

void MulticastActionsExec::SendReportOnTurn()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
#ifdef DEBUG_MODE
			std::cout << "[" << Utility::current_time_str() << "] " << "[DEBUG] "<< "\tPlayer #" << p->GetUID() << ":\t\tmoney: " << p->GetMoney() << "P\t\tproduced products: " << p->GetProduced() << std::endl;
#endif
		}
	}
}

void MulticastActionsExec::AddEmptyAuctionRequest()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	List<Item<MarketData>>& requests = ( auction_type == SOURCE_AUCTION ) ? const_cast<Banker&>(game_session).GetSourcesRequests() : const_cast<Banker&>(game_session).GetProductsRequests();

	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			Item<MarketData>* node;
			for ( node = requests.GetFirst(); node != nullptr; node = node->GetNext() )
				if ( node->GetData().GetPlayerNum() == p->GetUID() )
					break;

			if ( node == nullptr )
			{
				MarketData data;
				data.Make( p->GetUID(), 0, 0, m_game_settings->max_players );
				requests.Insert( data );
			}
		}
	}
}

void MulticastActionsExec::PayCharges()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			if ( !p->IsBankrot() )
			{
				int total_charges = 0;
				total_charges = p->GetSources() * m_game_settings->source_unit_charge;
				total_charges += p->GetProducts() * m_game_settings->product_unit_charge;
				total_charges += p->GetWaitFactories() * m_game_settings->factory_unit_charge;
				total_charges += p->GetWorkFactories() * m_game_settings->factory_unit_charge;

				Utility::itoa( total_charges, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::TOTAL_CHARGES_PARAM_TOKEN]), MESSAGE_TOKEN_SIZE-1 );
				const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::TOTAL_CHARGES_PARAM_TOKEN+1 );

				try
				{
					int remains = p->GetMoney() - total_charges;
					if ( remains >= 0 )
					{
						const_cast<Player*>(p)->SetMoney( remains );
						const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::SUCCESS_CHARGES_PAY_TOKEN ), p->GetFd(), p->GetAddr() );
					}
					else
					{
						const_cast<Player*>(p)->SetBankrot();
					}
				}
				catch ( const std::runtime_error& ex )
				{
					throw;
				}
			}
		}
	}
}

void MulticastActionsExec::CheckBuildingFactories()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			if ( !p->IsBankrot() )
			{
				try
				{
					for ( Item<BuildsData>* node = p->GetBuildsFactories().GetFirst(); node != nullptr; )
					{
						if ( node->GetData().GetTurnsLeft() == 1 )
						{
							int total_charges = m_game_settings->new_factory_unit_cost / 2;

							Utility::itoa(total_charges,const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::TOTAL_CHARGES_PARAM_TOKEN]),MessageTokens::MESSAGE_TOKEN_SIZE-1);
							const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::TOTAL_CHARGES_PARAM_TOKEN+1 );

							int remains = p->GetMoney() - total_charges;
							if ( remains >= 0 )
							{
								const_cast<Player*>(p)->SetMoney( remains );
								const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::PAY_FACTORY_SUCCESS_TOKEN ), p->GetFd(), p->GetAddr() );
							}
							else
							{
								const_cast<Player*>(p)->SetBankrot();
								break;
							}
						}
						else if ( node->GetData().GetTurnsLeft() == 0 )
						{
							const_cast<List<Item<BuildsData>>&>(p->GetBuildsFactories()).Delete( node->GetData().GetBuildNumber() );
							const_cast<Player*>(p)->SetBuiltFactories( p->GetBuiltFactories() - 1 );
							const_cast<Player*>(p)->SetWaitFactories( p->GetWaitFactories() + 1 );

							const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::FACTORY_BUILT_TOKEN ), p->GetFd(), p->GetAddr() );
							node = p->GetBuildsFactories().GetFirst();
							continue;
						}

						const_cast<BuildsData&>(node->GetData()).SetTurnsLeft( node->GetData().GetTurnsLeft() - 1 );
						node = node->GetNext();
					}
				}
				catch ( const std::runtime_error& ex )
				{
					throw;
				}
			}
		}
	}
}

void MulticastActionsExec::ShowReportOnTurn()
{
	const Banker& banker = *game_sessions.GetSessionById( session_id );
#ifdef DEBUG_MODE
	std::cout << "\n\n\n" << "[" << Utility::current_time_str() << "]" << " [DEBUG] <<<<<<<<<< Report on Month #" << banker.GetTurnNumber() << " >>>>>>>>>>\n";
	std::cout << "\n" << "[" << Utility::current_time_str() << "] " << "[DEBUG] Players statistics:" << "\n";

	ShowAuctionInfo();

	std::cout << "\n" << "[" << Utility::current_time_str() << "] " << "[DEBUG] <<<<<<<<<< Report on Month #" << banker.GetTurnNumber() << " >>>>>>>>>>\n";
#endif
}

void MulticastActionsExec::CheckStart()
{
	const Banker& banker = *game_sessions.GetSessionById( session_id );

	if (  banker.GetLobbyPlayers() < m_game_settings->min_players_to_start )
	{
		SendStartCancelled();
	}
	else
	{
		const_cast<Banker&>(banker).SetGameStarted();
		SendGameStarted();

		if ( !banker.IsGameStatePrepared() )
		{
			try
			{
				PreparePlayersState();
				PrepareSessionState();
			}
			catch ( const std::runtime_error& ex )
			{
				throw;
			}
		}
	}
}

void MulticastActionsExec::ChangeMarketState()
{
	const Banker& banker = *game_sessions.GetSessionById( session_id );

	int r = 1 + (int)( 12.0 * rand() / (RAND_MAX + 1.0) );

	int sum = 0;
	int i;
	for ( i = 0; i < MARKET_LEVEL_NUMBER; i++ )
	{
		sum += states_market_chance[banker.GetCurrentMarketLvl()-1][i];
		if ( sum >= r )
			break;
	}

	try
	{
		if ( i < MARKET_LEVEL_NUMBER )
			const_cast<Banker&>(banker).SetCurrentMarketLvl( i+1 );

		const_cast<Banker&>(banker).GetCurrentMarketState().SetSourcesAmount( amount_multiplier_table[banker.GetCurrentMarketLvl()-1][0] * banker.GetAlivePlayers() );
		const_cast<Banker&>(banker).GetCurrentMarketState().SetSourceMinPrice( price_table[banker.GetCurrentMarketLvl()-1][0] );
		const_cast<Banker&>(banker).GetCurrentMarketState().SetProductsAmount( amount_multiplier_table[banker.GetCurrentMarketLvl()-1][1] * banker.GetAlivePlayers() );
		const_cast<Banker&>(banker).GetCurrentMarketState().SetProductMaxPrice( price_table[banker.GetCurrentMarketLvl()-1][1] );
	}
	catch ( const std::runtime_error& ex )
	{
		throw;
	}
}

void MulticastActionsExec::ShowAuctionInfo()
{
	const Banker& banker = *game_sessions.GetSessionById( session_id );

#ifdef DEBUG_MODE
	std::cout << "\n" << "[" << Utility::current_time_str() << "] " << "[DEBUG] " << (( auction_type == SOURCE_AUCTION ) ? "Sources auction" : "Products auction") << "\n";
#endif

	Item<MarketData>* node = ( auction_type == SOURCE_AUCTION ) ? const_cast<Banker&>(banker).GetSourcesRequests().GetFirst() : const_cast<Banker&>(banker).GetProductsRequests().GetFirst();

	for ( ; node != nullptr; node = node->GetNext() )
	{
		const Player* p = banker.GetPlayers().GetPlayerByUID(node->GetData().GetPlayerNum());
#ifdef DEBUG_MODE
		std::cout	<< "[" << Utility::current_time_str() << "] " << "[DEBUG] " << "Request of Player #" << p->GetUID() << ":" << std::endl;
		std::cout	<< "[" << Utility::current_time_str() << "] " << "[DEBUG] " << "\tPrice: " << node->GetData().GetPrice() << "\n";
					<< "[" << Utility::current_time_str() << "] " << "[DEBUG] " << "\tAmount: " << ((node->GetData().IsSuccess()) ? p->GetAuctionReport().GetSoldSources() : node->GetData().GetAmount()) << "\n"
					<< "[" << Utility::current_time_str() << "] " << "[DEBUG] " << "\tIs proceed: " << (node->GetData().IsSuccess() ? "yes" : "no")
					<< std::endl;
#endif
	}
}

void MulticastActionsExec::PrepareNewTurn()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	try
	{
		ChangeMarketState();
		const_cast<Banker&>( game_session ).SetTurnNumber( game_session.GetTurnNumber() + 1 );
		const_cast<Banker&>( game_session ).SetReadyPlayers( 0 );

		for ( int i = 0; i < m_game_settings->max_players; ++i )
		{
			const Player* p = game_session.GetPlayers()[i];
			if ( !p->IsFree() )
			{
				if ( !p->IsBankrot() )
				{
					const_cast<Player*>(p)->UnsetSentSourceRequest();
					const_cast<Player*>(p)->UnsetSentProductsRequest();
					const_cast<Player*>(p)->SetProduced( 0 );
					const_cast<Player*>(p)->UnsetTurn();
					const_cast<Player*>(p)->SetIncome( p->GetMoney() - p->GetOldMoney() );
					const_cast<Player*>(p)->SetOldMoney( p->GetMoney() );

					Utility::itoa( p->GetUID(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::SENDER_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
					const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::SENDER_ID_PARAM_TOKEN+1 );

					const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::NEW_TURN_TOKEN ), p->GetFd(), p->GetAddr() );

					while ( p->GetWorkFactories() > 0 )
					{
						const_cast<Player*>(p)->SetProduced( p->GetProduced() + 1 );
						const_cast<Player*>(p)->SetProducts( p->GetProducts() + 1 );
						const_cast<Player*>(p)->SetWorkFactories( p->GetWorkFactories() - 1 );
						const_cast<Player*>(p)->SetWaitFactories( p->GetWaitFactories() + 1 );
					}

					if ( p->GetProduced() > 0 )
					{
						Utility::itoa(p->GetProduced(),const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::PRODUCED_AMOUNT_PARAM_TOKEN]),MessageTokens::MESSAGE_TOKEN_SIZE-1 );
						const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::PRODUCED_AMOUNT_PARAM_TOKEN+1 );

						const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::PRODUCED_TOKEN ), p->GetFd(), p->GetAddr() );
					}
				}
			}
		}
	}
	catch ( const std::runtime_error& ex )
	{
		throw;
	}
}

void MulticastActionsExec::PrepareSessionState()
{
	const Banker& banker = *game_sessions.GetSessionById( session_id );

	try
	{
		const_cast<Banker&>(banker).SetAlivePlayers( banker.GetLobbyPlayers() );
		const_cast<Banker&>(banker).SetLobbyPlayers( 0 );
		const_cast<Banker&>(banker).SetTurnNumber( 1 );
		const_cast<Banker&>(banker).SetCurrentMarketLvl( m_game_settings->start_market_level );
		const_cast<Banker&>(banker).GetCurrentMarketState().SetSourcesAmount( amount_multiplier_table[m_game_settings->start_market_level-1][0] * banker.GetAlivePlayers() );
		const_cast<Banker&>(banker).GetCurrentMarketState().SetSourceMinPrice( price_table[m_game_settings->start_market_level-1][0] );
		const_cast<Banker&>(banker).GetCurrentMarketState().SetProductsAmount( amount_multiplier_table[m_game_settings->start_market_level-1][1] * banker.GetAlivePlayers() );
		const_cast<Banker&>(banker).GetCurrentMarketState().SetProductMaxPrice( price_table[m_game_settings->start_market_level-1][1] );
		const_cast<Banker&>(banker).SetGameStatePrepared();
	}
	catch ( const std::runtime_error& ex )
	{
		throw;
	}
}

void MulticastActionsExec::PreparePlayersState()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			try
			{
				const_cast<Player*>(p)->SetMoney( m_game_settings->start_money );
				const_cast<Player*>(p)->SetOldMoney( m_game_settings->start_money );
				const_cast<Player*>(p)->SetSources( m_game_settings->start_sources );
				const_cast<Player*>(p)->SetProducts( m_game_settings->start_products );
				const_cast<Player*>(p)->SetWaitFactories( m_game_settings->start_factories );

				Utility::itoa( p->GetUID(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::SENDER_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
				const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::SENDER_ID_PARAM_TOKEN+1 );

				const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::STARTING_GAME_INFORMATION_TOKEN ), p->GetFd(), p->GetAddr() );
			}
			catch ( const std::runtime_error& ex )
			{
				throw;
			}
		}
	}
}

void MulticastActionsExec::SendAuctionsResults()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			try
			{
				const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::AUCTION_RESULTS_TOKEN ), p->GetFd(), p->GetAddr() );
			}
			catch ( const std::runtime_error& ex )
			{
				throw;
			}
		}
	}
}

void MulticastActionsExec::SendPlayersBankrot()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			if ( p->IsBankrot() )
			{
				try
				{
					const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::PLAYER_BANKROT_TOKEN ), p->GetFd(), p->GetAddr() );
				}
				catch ( const std::runtime_error& ex )
				{
					throw;
				}
			}
		}
	}
}

void MulticastActionsExec::SendNewPlayerConnect()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			try
			{
				const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::NEW_PLAYER_CONNECT_TOKEN ), p->GetFd(), p->GetAddr() );
			}
			catch ( const std::runtime_error& ex )
			{
				throw;
			}
		}
	}
}

void MulticastActionsExec::SendStartTime()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			try
			{
				Utility::itoa( m_game_settings->time_to_start, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::TIME_TO_START_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
				const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::TIME_TO_START_PARAM_TOKEN+1 );
				const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::STARTINSECONDS_TOKEN ), p->GetFd(), p->GetAddr() );
			}
			catch ( const std::runtime_error& ex )
			{
				throw;
			}
		}
	}
}

void MulticastActionsExec::SendStartCancelled()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			try
			{
				const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::STARTCANCELLED_TOKEN ), p->GetFd(), p->GetAddr() );
			}
			catch ( const std::runtime_error& ex )
			{
				throw;
			}
		}
	}
}

void MulticastActionsExec::SendGameStarted()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			try
			{
				const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::GAME_STARTED_TOKEN ), p->GetFd(), p->GetAddr() );
			}
			catch ( const std::runtime_error& ex )
			{
				throw;
			}
		}
	}
}

void MulticastActionsExec::QuitBankrotPlayers()
{
	Banker& game_session = const_cast<Banker&>(*game_sessions.GetSessionById( session_id ));

	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			if ( p->IsBankrot() )
			{
				std::pair<int, std::string> bankrot_record { p->GetFd(), p->GetAddr() };
				left_player_id = p->GetUID();

				try
				{
					QuitPlayer();
				}
				catch ( const std::runtime_error& ex )
				{
					throw;
				}

				const_cast<Banker::BankrotsList&>(game_session.GetBankrotsList()).push_back(bankrot_record);
			}
		}
	}

	if ( !game_session.GetBankrotsList().empty() )
	{
		SessionsPlanner::BankrotsList sessions_bankrots = game_sessions.GetBankrotsList();
		sessions_bankrots.splice( sessions_bankrots.end(), const_cast<Banker::BankrotsList&>( game_session.GetBankrotsList() ) );
	}
}

void MulticastActionsExec::QuitPlayer()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );
	const Player* left_player = game_session.GetPlayers().GetPlayerByUID( left_player_id );

	try
	{
		const_cast<Player*>(left_player)->SetFree();
		const_cast<Banker&>(game_session).SetAlivePlayers( game_session.GetAlivePlayers() - 1 );

		for ( int i = 0; i < m_game_settings->max_players; ++i )
		{
			const Player* p = game_session.GetPlayers()[i];
			if ( !p->IsFree() )
			{
				if ( !p->IsBankrot() )
				{
					if ( !game_session.IsGameStarted() )
					{
						const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::LOST_LOBBY_PLAYER_TOKEN ), p->GetFd(), p->GetAddr() );
					}
					else
					{
						if ( game_session.GetAlivePlayers() <= 1 )
						{
							const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::VICTORY_MESSAGE_TOKEN ), p->GetFd(), p->GetAddr() );
#ifdef DEBUG_MODE
							std::cout << "[" << Utility::current_time_str() << "] " << "[DEBUG] " << "\n\n" << "<<<<< GAME IS FINISHED. PLAYER #" << p->GetUID() << " IS WINNER! >>>>>" << "\n\n";
#endif
							return;
						}

						Utility::itoa(left_player_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::LEFT_PLAYER_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1);
						const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::LEFT_PLAYER_ID_PARAM_TOKEN+1 );

						const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::LOST_ALIVE_PLAYER_TOKEN ), p->GetFd(), p->GetAddr() );
					}
				}
			}
		}
	}
	catch ( const std::runtime_error& ex )
	{
		throw;
	}
}

void MulticastActionsExec::SortRequestsByPrice( const List<Item<MarketData>>& requests, List<Item<MarketData>>& sorted_requests )
{
	const int ready_players = (*game_sessions.GetSessionById( session_id )).GetReadyPlayers();

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

	Utility::heap_sort(prices, ready_players, ( auction_type == SOURCE_AUCTION ) ? 1 : 0 );

	int j = 0;
	for ( int i = 0; i < m_game_settings->max_players; ++i )
	{
		if ( (arr_reqs[i]->GetData().GetPrice() == prices[j]) && !reqs_checked[i] )
		{
			MarketData data;
			data.Make( arr_reqs[i]->GetData().GetPlayerNum(), arr_reqs[i]->GetData().GetAmount(), arr_reqs[i]->GetData().GetPrice(), m_game_settings->max_players );

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

void MulticastActionsExec::StartAuction()
{
	const Banker& banker = *game_sessions.GetSessionById( session_id );

	// если список заявок на аукцион пуст, то не нужно его проводить
	List<Item<MarketData>>& requests = ( auction_type == SOURCE_AUCTION ) ? const_cast<Banker&>(banker).GetSourcesRequests() : const_cast<Banker&>(banker).GetProductsRequests();
	if ( requests.IsEmpty() )
		return;

	// Если какой-либо игрок не заявился на аукцион, добавить его пустую заявку
	AddEmptyAuctionRequest();

	List<Item<MarketData>> sorted_requests;
	SortRequestsByPrice( requests, sorted_requests );

	int max_sources = const_cast<Banker&>(banker).GetCurrentMarketState().GetSourcesAmount();
	int max_products = const_cast<Banker&>(banker).GetCurrentMarketState().GetProductsAmount();

	for ( Item<MarketData>* node = sorted_requests.GetFirst(); node != nullptr; node = node->GetNext() )
	{
		if ( node->GetData().GetPrice() < 1 )
			continue;

		const Player* cur_p = banker.GetPlayers().GetPlayerByUID( node->GetData().GetPlayerNum() );

		if ( cur_p->IsFree() )
			continue;

		try
		{
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
		catch ( const std::runtime_error& ex )
		{
			throw;
		}
	}
}


GameMessages::GameMessages( const SessionsPlanner& sessions, std::shared_ptr<const Config::GameSettings> m_g_sets ) : BrokerMessages( m_g_sets ), game_sessions( sessions )
{
	BrokerActions& br_acts = const_cast<BrokerActions&>(GetBrokerActions());
	br_acts.Make( GameMessages::BROKER_ACTIONS_COUNT );

	sender_id				=			0;
	left_player_id			=			0;
	time_to_start			=			0;
	sender_id				=			0;
	produced				=			0;
	total_charges			=			0;

	memset(result_message, 0, MESSAGE_SIZE);

	br_acts[AUCTION_RESULTS_TOKEN]							=				[this]() {	AuctionResultsMessage();		};
	br_acts[SUCCESS_CHARGES_PAY_TOKEN]						=				[this]() {	SuccessChargesPayMessage();		};
	br_acts[PLAYER_BANKROT_TOKEN]							=				[this]() {	PlayerBankrotMessage();			};
	br_acts[LOST_ALIVE_PLAYER_TOKEN]						=				[this]() {	LostAlivePlayerMessage();		};
	br_acts[PRODUCED_TOKEN]									=				[this]() {	ProducedMessage();				};
	br_acts[STARTINSECONDS_TOKEN]							=				[this]() {	StartInSecondsMessage();		};
	br_acts[GAME_STARTED_TOKEN]								=				[this]() {	GameStartedMessage();			};
	br_acts[STARTING_GAME_INFORMATION_TOKEN]				=				[this]() {	StartGameInfoMessage();			};
	br_acts[STARTCANCELLED_TOKEN]							=				[this]() {	StartCancelledMessage();		};
	br_acts[PAY_FACTORY_SUCCESS_TOKEN]						=				[this]() {	PayFactorySuccessMessage();		};
	br_acts[FACTORY_BUILT_TOKEN]							=				[this]() {	FactoryBuiltMessage();			};
	br_acts[VICTORY_MESSAGE_TOKEN]							=				[this]() {	VictoryMessage();				};
	br_acts[GAME_ALREADY_STARTED_TOKEN]						=				[this]() {	GameAlreadyStartedMessage();	};
	br_acts[SERVER_FULL_TOKEN]								=				[this]() {	ServerFullMessage();			};
	br_acts[NEW_PLAYER_CONNECT_TOKEN]						=				[this]() {	NewPlayerConnectMessage();		};
	br_acts[GAME_NOT_STARTED_TOKEN]							=				[this]() {	GameNotStartedMessage();		};
	br_acts[LOST_LOBBY_PLAYER_TOKEN]						=				[this]() {	LostLobbyPlayerMessage();		};
	br_acts[NEW_TURN_TOKEN]									=				[this]() {	NewTurnMessage();				};
}

void GameMessages::PutMessage( const char** message_tokens, int tokens_count )
{
	for ( int i = SESSION_ID_PARAM_TOKEN; i < tokens_count; ++i )
	{
		if ( ( message_tokens[i] != nullptr ) && ( strcmp(message_tokens[i], "") != 0 ) )
		{
			switch ( i )
			{
				case SESSION_ID_PARAM_TOKEN:
					session_id = atoi(message_tokens[i]);
					break;
				case LEFT_PLAYER_ID_PARAM_TOKEN:
					left_player_id = atoi(message_tokens[i]);
					break;
				case TIME_TO_START_PARAM_TOKEN:
					time_to_start = atoi(message_tokens[i]);
					break;
				case SENDER_ID_PARAM_TOKEN:
					sender_id = atoi(message_tokens[i]);
					break;
				case PRODUCED_AMOUNT_PARAM_TOKEN:
					produced = atoi(message_tokens[i]);
					break;
				case TOTAL_CHARGES_PARAM_TOKEN:
					total_charges = atoi(message_tokens[i]);
					break;
			}
			break;
		}
	}
}

void GameMessages::CheckMessageCode( int message_code ) const
{
	for ( int i = AUCTION_RESULTS_TOKEN; i <= NEW_TURN_TOKEN; ++i )
		if ( message_code == i )
			return;

	throw std::range_error("RangeError in \"GameMessages::CheckMessageCode\" function!");
}

void GameMessages::LostLobbyPlayerMessage()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	char lp_buf[10];
	Utility::itoa(game_session.GetLobbyPlayers(), lp_buf, 9);

	char max_pl_buf[10];
	Utility::itoa(m_game_settings->max_players, max_pl_buf, 9);


	const char* message_tokens[] =
	{
				info_game_messages[LOST_LOBBY_PLAYER],
				lp_buf,
				max_pl_buf,
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 3 );
}

void GameMessages::LostAlivePlayerMessage()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	char ap_buf[10];
	Utility::itoa( game_session.GetAlivePlayers(), ap_buf, 9 );

	char left_p_num_buf[10];
	Utility::itoa( left_player_id, left_p_num_buf, 9 );


	const char* message_tokens[] =
	{
				info_game_messages[LOST_ALIVE_PLAYER],
				ap_buf,
				left_p_num_buf,
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 3 );
}

void GameMessages::VictoryMessage()
{
	const char* message_tokens[] =
	{
				info_game_messages[VICTORY_MESSAGE],
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}

void GameMessages::StartInSecondsMessage()
{
	char tts[10];
	Utility::itoa(time_to_start, tts, 9);


	const char* message_tokens[] =
	{
				info_game_messages[STARTINSECONDS],
				tts,
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 2 );
}

void GameMessages::GameStartedMessage()
{
	const char* message_tokens[] =
	{
				info_game_messages[GAME_STARTED],
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}

void GameMessages::GameAlreadyStartedMessage()
{
	const char* message_tokens[] =
	{
				info_game_messages[GAME_ALREADY_STARTED],
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}

void GameMessages::StartGameInfoMessage()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	const Player* sender_p = game_session.GetPlayers().GetPlayerByUID( sender_id );
	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;
	}
	else
	{
		throw std::runtime_error("Got null pointer of Player object in \"GameMessages::StartGameInfoMessage\" function!");
	}

	char p_num[10];
	Utility::itoa(sender_p->GetUID(), p_num, 9);

	char ap[10];
	Utility::itoa(game_session.GetAlivePlayers(), ap, 9);

	char tn[10];
	Utility::itoa(game_session.GetTurnNumber(), tn, 9);

	char p_money[20];
	Utility::itoa(sender_p->GetMoney(), p_money, 19);

	char p_sources[10];
	Utility::itoa(sender_p->GetSources(), p_sources, 9);

	char p_products[10];
	Utility::itoa(sender_p->GetProducts(), p_products, 9);

	char p_wf[10];
	Utility::itoa(sender_p->GetWaitFactories(), p_wf, 9);

	char p_wrkf[10];
	Utility::itoa(sender_p->GetWorkFactories(), p_wrkf, 9);

	char p_bf[10];
	Utility::itoa(sender_p->GetBuiltFactories(), p_bf, 9);

	char sa[10];
	Utility::itoa(const_cast<Banker&>(game_session).GetCurrentMarketState().GetSourcesAmount(), sa, 9);

	char smp[10];
	Utility::itoa(const_cast<Banker&>(game_session).GetCurrentMarketState().GetSourceMinPrice(), smp, 9);

	char pa[10];
	Utility::itoa(const_cast<Banker&>(game_session).GetCurrentMarketState().GetProductsAmount(), pa, 9);

	char pmp[10];
	Utility::itoa(const_cast<Banker&>(game_session).GetCurrentMarketState().GetProductMaxPrice(), pmp, 9);


	const char* message_tokens[] =
	{
				info_game_messages[STARTING_GAME_INFORMATION],
				p_num,
				ap,
				tn,
				p_money,
				p_sources,
				p_products,
				p_wf,
				p_wrkf,
				p_bf,
				sa,
				smp,
				pa,
				pmp,
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, sender_p->IsBot() ? 14 : 10 );
}

void GameMessages::StartCancelledMessage()
{
	const char* message_tokens[] =
	{
				info_game_messages[STARTCANCELLED],
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}

void GameMessages::NewPlayerConnectMessage()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	char lp_buf[10];
	Utility::itoa(game_session.GetLobbyPlayers(), lp_buf, 9);

	char max_pl_buf[10];
	Utility::itoa(m_game_settings->max_players, max_pl_buf, 9);


	const char* message_tokens[] =
	{
				info_game_messages[NEW_PLAYER_CONNECT],
				lp_buf,
				max_pl_buf,
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 3 );
}

void GameMessages::GameNotStartedMessage()
{
	const char* message_tokens[] =
	{
				info_game_messages[GAME_NOT_STARTED],
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}

void GameMessages::AuctionResultsMessage()
{
	enum
	{
			PL_REP_FIELDS_NUM		=		6,
			TURN_SIZE				=		5,
			PLAYER_NUM_SIZE			=		4,
			SOLD_SOURCES_SIZE		=		4,
			SOLD_PRICE_SIZE			=		11,
			BOUGHT_PRODS_SIZE		=		4,
			BOUGHT_PRICE_SIZE		=		11

	};
	struct player_report
	{
		char tn[TURN_SIZE];
		char pnum[PLAYER_NUM_SIZE];
		char ssnum[SOLD_SOURCES_SIZE];
		char spnum[SOLD_PRICE_SIZE];
		char bpnum[BOUGHT_PRODS_SIZE];
		char bprnum[BOUGHT_PRICE_SIZE];
	};

	player_report pr[m_game_settings->max_players];
	memset(pr, 0, sizeof(player_report) * m_game_settings->max_players);

	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	const int msg_tokens_size = PL_REP_FIELDS_NUM * m_game_settings->max_players + 1;
	const char* message_tokens[ msg_tokens_size ];

	for ( int i = 0; i < msg_tokens_size; ++i )
		message_tokens[i] = nullptr;


	message_tokens[0] = info_game_messages[AUCTION_RESULTS];

	int tokens_amount = 1;
	for ( int i = 0; i < game_session.GetReadyPlayers(); ++i )
	{
		Utility::itoa(game_session.GetTurnNumber(), pr[i].tn, TURN_SIZE);
		message_tokens[tokens_amount] = pr[i].tn;
		++tokens_amount;

		Utility::itoa(game_session.GetPlayers()[i]->GetUID(), pr[i].pnum, PLAYER_NUM_SIZE);
		message_tokens[tokens_amount] = pr[i].pnum;
		++tokens_amount;

		Utility::itoa(game_session.GetPlayers()[i]->GetAuctionReport().GetSoldSources(), pr[i].ssnum, SOLD_SOURCES_SIZE);
		message_tokens[tokens_amount] = pr[i].ssnum;
		++tokens_amount;

		Utility::itoa(game_session.GetPlayers()[i]->GetAuctionReport().GetSoldPrice(), pr[i].spnum, SOLD_PRICE_SIZE);
		message_tokens[tokens_amount] = pr[i].spnum;
		++tokens_amount;

		Utility::itoa(game_session.GetPlayers()[i]->GetAuctionReport().GetBoughtProducts(), pr[i].bpnum, BOUGHT_PRODS_SIZE);
		message_tokens[tokens_amount] = pr[i].bpnum;
		++tokens_amount;

		Utility::itoa(game_session.GetPlayers()[i]->GetAuctionReport().GetBoughtPrice(), pr[i].bprnum, BOUGHT_PRICE_SIZE);
		message_tokens[tokens_amount] = pr[i].bprnum;
		++tokens_amount;
	}

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, tokens_amount );
}

void GameMessages::NewTurnMessage()
{
	const Banker& game_session = *game_sessions.GetSessionById( session_id );

	const Player* sender_p = game_session.GetPlayers().GetPlayerByUID( sender_id );
	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;
	}
	else
	{
		throw std::runtime_error("Got null pointer of Player object in \"GameMessages::NewTurnMessage\" function!");
	}

	char tn[10];
	Utility::itoa(game_session.GetTurnNumber(), tn, 9);

	char sa[10];
	Utility::itoa(const_cast<Banker&>(game_session).GetCurrentMarketState().GetSourcesAmount(), sa, 9);

	char smp[10];
	Utility::itoa(const_cast<Banker&>(game_session).GetCurrentMarketState().GetSourceMinPrice(), smp, 9);

	char pa[10];
	Utility::itoa(const_cast<Banker&>(game_session).GetCurrentMarketState().GetProductsAmount(), pa, 9);

	char pmp[10];
	Utility::itoa(const_cast<Banker&>(game_session).GetCurrentMarketState().GetProductMaxPrice(), pmp, 9);


	const char* message_tokens[] =
	{
				info_game_messages[NEW_TURN],
				tn,
				sa,
				smp,
				pa,
				pmp,
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, sender_p->IsBot() ? 6 : 2 );
}

void GameMessages::ProducedMessage()
{
	char am_prd[10];
	Utility::itoa(produced, am_prd, 9);


	const char* message_tokens[] =
	{
				info_game_messages[PRODUCED],
				am_prd,
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 2 );
}

void GameMessages::SuccessChargesPayMessage()
{
	char charges[20];
	Utility::itoa(total_charges, charges, 19);


	const char* message_tokens[] =
	{
				info_game_messages[SUCCESS_CHARGES_PAY],
				charges,
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 2 );
}

void GameMessages::PlayerBankrotMessage()
{
	char charges[20];
	Utility::itoa(total_charges, charges, 19);


	const char* message_tokens[] =
	{
				info_game_messages[PLAYER_BANKROT],
				charges,
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 2 );
}

void GameMessages::PayFactorySuccessMessage()
{
	const char* message_tokens[] =
	{
			info_game_messages[PAY_FACTORY_SUCCESS],
			nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}

void GameMessages::FactoryBuiltMessage()
{
	const char* message_tokens[] =
	{
			info_game_messages[FACTORY_BUILT],
			nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}

void GameMessages::ServerFullMessage()
{
	const char* message_tokens[] =
	{
				info_game_messages[SERVER_FULL],
				nullptr
	};

	Utility::concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}


BCBrokerMessages::BCBrokerMessages( const SessionsPlanner& sessions, std::shared_ptr<const Config::GameSettings> m_g_sets ) : BrokerMessages( m_g_sets ), game_sessions( sessions )
{
	BrokerActions& br_acts = const_cast<BrokerActions&>(GetBrokerActions());
	br_acts.Make( BCBrokerMessages::BROKER_ACTIONS_COUNT );

	session_id				=			0;
	sender_player_id		=			0;
	target_player_id		=			0;
	sources_amount			=			0;
	source_price			=			0;
	product_price			=			0;

	memset(result_message, 0, MESSAGE_SIZE);

	br_acts[MARKET_SOURCES_AMOUNT_TOKEN]							=			[this]() {	MarketCmdSourcesAmount();			};
	br_acts[MARKET_SOURCE_MIN_PRICE_TOKEN]							=			[this]() {	MarketCmdSourceMinPrice();			};
	br_acts[MARKET_PRODUCTS_AMOUNT_TOKEN]							=			[this]() {	MarketCmdProductsAmount();			};
	br_acts[MARKET_PRODUCT_MAX_PRICE_TOKEN]							=			[this]() {	MarketCmdProductMaxPrice();			};
	br_acts[TARGET_PLAYER_NOT_FOUND_TOKEN]							=			[this]() { 	PlayerCmdIsTargetNotFound();		};
	br_acts[TARGET_PLAYER_UID_TOKEN]								=			[this]() {	PlayerCmdGetTargetUID();			};
	br_acts[TARGET_PLAYER_MONEY_TOKEN]								=			[this]() {	PlayerCmdGetTargetMoney();			};
	br_acts[TARGET_PLAYER_INCOME_TOKEN]								=			[this]() {	PlayerCmdGetTargetIncome();			};
	br_acts[TARGET_PLAYER_SOURCES_TOKEN]							=			[this]() {	PlayerCmdGetTargetSources();		};
	br_acts[TARGET_PLAYER_PRODUCTS_TOKEN]							=			[this]() {	PlayerCmdGetTargetProducts();		};
	br_acts[TARGET_PLAYER_WAIT_FACTORIES_TOKEN]						=			[this]() {	PlayerCmdGetTargetWaitFactories();	};
	br_acts[TARGET_PLAYER_WORK_FACTORIES_TOKEN]						=			[this]() {	PlayerCmdGetTargetWorkFactories();	};
	br_acts[TARGET_PLAYER_BUILT_FACTORIES_TOKEN]					=			[this]() {	PlayerCmdGetTargetBuiltFactories(); };
	br_acts[SENDER_PLAYER_IS_BOT_TOKEN]								=			[this]() {	PlayerSenderIsBot();				};
	br_acts[TARGET_PLAYER_PRODUCED_TOKEN]							=			[this]() {	PlayerCmdGetTargetProduced();		};
	br_acts[ALIVE_PLAYERS_TOKEN]									=			[this]() {	ListCmdGetAlivePlayers();			};
	br_acts[PLAYER_IS_TURN_TOKEN]									=			[this]() {	PlayerSenderIsTurn();				};
	br_acts[PROD_CMD_SOURCES_CONDITION_SUCCESS_TOKEN]				=			[this]() {	ProdCmdSourcesCondition();			};
	br_acts[PROD_CMD_MONEY_CONDITION_SUCCESS_TOKEN]					=			[this]() {	ProdCmdMoneyCondition();			};
	br_acts[PROD_CMD_WAIT_FACTORIES_CONDITION_SUCCESS_TOKEN]		=			[this]() {	ProdCmdWaitFactoriesCondition();	};
	br_acts[PROD_CMD_UPDATE_GAME_STATE_TOKEN]						=			[this]() {	ProdCmdUpdateGameState();			};
	br_acts[BUILD_CMD_PLAYER_BUILDS_LIST_IS_EMPTY_TOKEN]			=			[this]() {	BuildCmdPlayerBuildsListIsEmpty();	};
	br_acts[BUILD_CMD_PLAYER_GET_BUILDS_LIST_SIZE_TOKEN]			=			[this]() {	BuildCmdPlayerGetBuildsListSize();	};
	br_acts[BUILD_CMD_PLAYER_GET_BUILDS_LIST_TOKEN]					=			[this]() {	BuildCmdPlayerGetBuildsList();		};
	br_acts[BUILD_CMD_MONEY_CONDITION_SUCCESS_TOKEN]				=			[this]() {	BuildCmdMoneyCondition();			};
	br_acts[BUILD_CMD_UPDATE_GAME_STATE_TOKEN]						=			[this]() {	BuildCmdUpdateGameState();			};
	br_acts[BUY_CMD_IS_SENT_SOURCE_REQUEST]							=			[this]() { 	BuyCmdIsSentSourceRequest();		};
	br_acts[BUY_CMD_SOURCES_CONDITION_SUCCESS_TOKEN]				=			[this]() {	BuyCmdSourcesCondition();			};
	br_acts[BUY_CMD_PRICE_CONDITION_SUCCESS_TOKEN]					=			[this]() {	BuyCmdPriceCondition();				};
	br_acts[BUY_CMD_MONEY_CONDITION_SUCCESS_TOKEN]					=			[this]() {	BuyCmdMoneyCondition();				};
	br_acts[BUY_CMD_UPDATE_GAME_STATE_TOKEN]						=			[this]() {	BuyCmdUpdateGameState();			};
	br_acts[SELL_CMD_IS_SENT_PRODUCT_REQUEST]						=			[this]() {	SellCmdIsSentProductRequest();		};
	br_acts[SELL_CMD_AMOUNT_CONDITION_SUCCESS_TOKEN]				=			[this]() {	SellCmdAmountCondition();			};
	br_acts[SELL_CMD_PRICE_CONDITION_SUCCESS_TOKEN]					=			[this]() {	SellCmdPriceCondition();			};
	br_acts[SELL_CMD_UPDATE_GAME_STATE_TOKEN]						=			[this]() {	SellCmdUpdateGameState();			};
	br_acts[TURN_CMD_UPDATE_GAME_STATE_TOKEN]						=			[this]() {	TurnCmdUpdateGameState();			};
	br_acts[TURN_CMD_GET_WYPA_TOKEN]								=			[this]() {	TurnCmdGetWypaToken();				};
}

void BCBrokerMessages::PutMessage( const char** message_tokens, int tokens_count )
{
	for ( int i = SESSION_ID_PARAM_TOKEN; i < tokens_count; ++i )
	{
		if ( ( message_tokens[i] != nullptr ) && ( strcmp(message_tokens[i], "") != 0 ) )
		{
			switch ( i )
			{
				case SESSION_ID_PARAM_TOKEN:
					session_id = atoi(message_tokens[i]);
					break;
				case SENDER_PLAYER_ID_PARAM_TOKEN:
					sender_player_id = atoi(message_tokens[i]);
					break;
				case TARGET_PLAYER_ID_PARAM_TOKEN:
					target_player_id = atoi(message_tokens[i]);
					break;
				case SOURCES_AMOUNT_PARAM_TOKEN:
					sources_amount = atoi(message_tokens[i]);
					break;
				case SOURCE_PRICE_PARAM_TOKEN:
					source_price = atoi(message_tokens[i]);
					break;
				case PRODUCTS_AMOUNT_PARAM_TOKEN:
					products_amount = atoi(message_tokens[i]);
					break;
				case PRODUCT_PRICE_PARAM_TOKEN:
					product_price = atoi(message_tokens[i]);
					break;
			}
			break;
		}
	}
}

void BCBrokerMessages::CheckMessageCode( int message_code ) const
{
	for ( int i = MARKET_SOURCES_AMOUNT_TOKEN; i <= TURN_CMD_GET_WYPA_TOKEN; ++i )
		if ( message_code == i )
			return;

	throw std::range_error("RangeError in \"BCBrokerMessages::CheckMessageCode\" function!");
}

void BCBrokerMessages::MarketCmdSourcesAmount()
{
	Utility::itoa( const_cast<Banker&>(*game_sessions.GetSessionById(session_id)).GetCurrentMarketState().GetSourcesAmount(), result_message, MESSAGE_SIZE-1 );
}

void BCBrokerMessages::MarketCmdSourceMinPrice()
{
	Utility::itoa( const_cast<Banker&>(*game_sessions.GetSessionById(session_id)).GetCurrentMarketState().GetSourceMinPrice(), result_message, MESSAGE_SIZE-1 );
}

void BCBrokerMessages::MarketCmdProductsAmount()
{
	Utility::itoa( const_cast<Banker&>(*game_sessions.GetSessionById(session_id)).GetCurrentMarketState().GetProductsAmount(), result_message, MESSAGE_SIZE-1 );
}

void BCBrokerMessages::MarketCmdProductMaxPrice()
{
	Utility::itoa( const_cast<Banker&>(*game_sessions.GetSessionById(session_id)).GetCurrentMarketState().GetProductMaxPrice(), result_message, MESSAGE_SIZE-1 );
}

void BCBrokerMessages::PlayerCmdIsTargetNotFound()
{
	const Player* target_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::PlayerCmdIsTargetNotFound\" function!");
}

void BCBrokerMessages::PlayerCmdGetTargetUID()
{
	const Player* target_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
			return;

		Utility::itoa(target_p->GetUID(), result_message, MESSAGE_SIZE-1);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::PlayerCmdGetTargetUID\" function!");
}

void BCBrokerMessages::PlayerCmdGetTargetMoney()
{
	const Player* target_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
			return;

		Utility::itoa(target_p->GetMoney(), result_message, MESSAGE_SIZE-1);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::PlayerCmdGetTargetMoney\" function!");
}

void BCBrokerMessages::PlayerCmdGetTargetIncome()
{
	const Player* target_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
			return;

		Utility::itoa(target_p->GetIncome(), result_message, MESSAGE_SIZE-1);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::PlayerCmdGetTargetIncome\" function!");
}

void BCBrokerMessages::PlayerCmdGetTargetSources()
{
	const Player* target_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
			return;

		Utility::itoa(target_p->GetSources(), result_message, MESSAGE_SIZE-1);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::PlayerCmdGetTargetSources\" function!");
}

void BCBrokerMessages::PlayerCmdGetTargetProducts()
{
	const Player* target_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
			return;

		Utility::itoa(target_p->GetProducts(), result_message, MESSAGE_SIZE-1);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::PlayerCmdGetTargetProducts\" function!");
}

void BCBrokerMessages::PlayerCmdGetTargetWaitFactories()
{
	const Player* target_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
			return;

		Utility::itoa(target_p->GetWaitFactories(), result_message, MESSAGE_SIZE-1);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::PlayerCmdGetTargetWaitFactories\" function!");
}

void BCBrokerMessages::PlayerCmdGetTargetWorkFactories()
{
	const Player* target_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
			return;

		Utility::itoa(target_p->GetWorkFactories(), result_message, MESSAGE_SIZE-1);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::PlayerCmdGetTargetWorkFactories\" function!");
}

void BCBrokerMessages::PlayerCmdGetTargetBuiltFactories()
{
	const Player* target_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
			return;

		Utility::itoa(target_p->GetBuiltFactories(), result_message, MESSAGE_SIZE-1);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::PlayerCmdGetTargetBuiltFactories\" function!");
}

void BCBrokerMessages::PlayerSenderIsBot()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		if ( sender_p->IsBot() )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message,  false_str);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::PlayerSenderIsBot\" function!");
}

void BCBrokerMessages::PlayerCmdGetTargetProduced()
{
	const Player* target_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
			return;

		Utility::itoa(target_p->GetProduced(), result_message, MESSAGE_SIZE-1);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::PlayerCmdGetTargetProduced\" function!");
}

void BCBrokerMessages::ListCmdGetAlivePlayers()
{
	Utility::itoa( (*game_sessions.GetSessionById(session_id)).GetAlivePlayers(), result_message, MESSAGE_SIZE-1 );
}

void BCBrokerMessages::PlayerSenderIsTurn()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		if ( sender_p->IsTurn() )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::PlayerSenderIsTurn\" function!");
}

void BCBrokerMessages::ProdCmdSourcesCondition()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		if ( sender_p->GetSources() >= 1 )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::ProdCmdSourcesCondition\" function!");
}

void BCBrokerMessages::ProdCmdMoneyCondition()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		if ( sender_p->GetMoney() >= m_game_settings->production_product_cost )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::ProdCmdMoneyCondition\" function!");
}

void BCBrokerMessages::ProdCmdWaitFactoriesCondition()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		if ( sender_p->GetWaitFactories() > 0 )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::ProdCmdWaitFactoriesCondition\" function!");
}

void BCBrokerMessages::ProdCmdUpdateGameState()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;
		try
		{
			const_cast<Player*>(sender_p)->SetWaitFactories( sender_p->GetWaitFactories() - 1 );
			const_cast<Player*>(sender_p)->SetWorkFactories( sender_p->GetWorkFactories() + 1 );
			const_cast<Player*>(sender_p)->SetSources( sender_p->GetSources() - 1 );
			const_cast<Player*>(sender_p)->SetMoney( sender_p->GetMoney() - m_game_settings->production_product_cost );
		}
		catch ( const std::runtime_error& ex )
		{
			throw;
		}

		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::ProdCmdUpdateGameState\" function!");
}

void BCBrokerMessages::BuildCmdPlayerBuildsListIsEmpty()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID(sender_player_id);

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		const List<Item<BuildsData>>& player_builds = sender_p->GetBuildsFactories();

		if ( player_builds.IsEmpty() )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::BuildCmdPlayerBuildsListIsEmpty\" function!");
}

void BCBrokerMessages::BuildCmdPlayerGetBuildsListSize()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID(sender_player_id);

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		const List<Item<BuildsData>>& player_builds = sender_p->GetBuildsFactories();
		Utility::itoa( player_builds.GetSize(), result_message, MESSAGE_SIZE-1 );
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::BuildCmdPlayerGetBuildListSize\" function!");
}

void BCBrokerMessages::BuildCmdPlayerGetBuildsList()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID(sender_player_id);

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		const List<Item<BuildsData>>& player_builds = sender_p->GetBuildsFactories();

		char build_number[10]	{	0x00	};
		char turns_left[10]		{	0x00	};

		int offset = 0;

		for ( Item<BuildsData>* node = player_builds.GetFirst(); node != nullptr; node = node->GetNext() )
		{
			Utility::concat_to_str(node->GetData().GetBuildNumber(), build_number, 9, result_message, &offset);
			Utility::concat_to_str(node->GetData().GetTurnsLeft(), turns_left, 9, result_message, &offset);
		}

		result_message[offset-1] = '\0';
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::BuildCmdPlayerGetBuildsList\" function!");
}

void BCBrokerMessages::BuildCmdMoneyCondition()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		if ( sender_p->GetMoney() >= m_game_settings->new_factory_unit_cost/2 )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::BuildCmdMoneyCondition\" function!");
}

void BCBrokerMessages::BuildCmdUpdateGameState()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		BuildsData data;
		data.Make( sender_p->GetBuildsFactories().GetValidNum(), m_game_settings->turns_to_build_factory, m_game_settings->max_players );

		try
		{
			const_cast<List<Item<BuildsData>>&>(sender_p->GetBuildsFactories()).Insert( data );
			const_cast<Player*>(sender_p)->SetMoney( sender_p->GetMoney() - m_game_settings->new_factory_unit_cost/2 );
			const_cast<Player*>(sender_p)->SetBuiltFactories( sender_p->GetBuiltFactories() + 1 );
		}
		catch ( const std::runtime_error& ex )
		{
			throw;
		}

		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::BuildCmdUpdateGameState\" function!");
}

void BCBrokerMessages::BuyCmdIsSentSourceRequest()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		if ( sender_p->IsSentSourceRequest() )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::BuyCmdIsSentSourceRequest\" function!");
}

void BCBrokerMessages::BuyCmdSourcesCondition()
{
	if ( ( sources_amount > 0 ) && ( sources_amount <= const_cast<Banker&>((*game_sessions.GetSessionById(session_id))).GetCurrentMarketState().GetSourcesAmount() ) )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::BuyCmdPriceCondition()
{
	if ( ( source_price >= const_cast<Banker&>((*game_sessions.GetSessionById(session_id))).GetCurrentMarketState().GetSourceMinPrice() ) )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::BuyCmdMoneyCondition()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		if ( sender_p->GetMoney() < source_price * sources_amount )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::BuyCmdMoneyCondition\" function!");
}

void BCBrokerMessages::BuyCmdUpdateGameState()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		MarketData data;
		data.Make( sender_player_id, sources_amount, source_price, m_game_settings->max_players );

		const_cast<Banker&>(*game_sessions.GetSessionById(session_id)).GetSourcesRequests().Insert( data );

		try
		{
			const_cast<Player*>(sender_p)->SetSentSourceRequest();
		}
		catch ( const std::runtime_error& ex )
		{
			throw;
		}

		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::BuyCmdUpdateGameState\" function!");
}

void BCBrokerMessages::SellCmdIsSentProductRequest()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		if ( sender_p->IsSentProductsRequest() )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::SellCmdIsSentProductRequest\" function!");
}

void BCBrokerMessages::SellCmdAmountCondition()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		if ( ( products_amount > 0 ) && ( products_amount <= sender_p->GetProducts() ) )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::SellCmdAmountCondition\" function!");
}

void BCBrokerMessages::SellCmdPriceCondition()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		if ( ( product_price > 0 ) && ( product_price <= const_cast<Banker&>((*game_sessions.GetSessionById(session_id))).GetCurrentMarketState().GetProductMaxPrice() ) )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::SellCmdPriceCondition\" function!");
}

void BCBrokerMessages::SellCmdUpdateGameState()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		MarketData data;
		data.Make( sender_player_id, products_amount, product_price, m_game_settings->max_players );

		const_cast<Banker&>((*game_sessions.GetSessionById(session_id))).GetProductsRequests().Insert( data );

		try
		{
			const_cast<Player*>(sender_p)->SetSentProductsRequest();
		}
		catch ( const std::runtime_error& ex )
		{
			throw;
		}

		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::SellCmdUpdateGameState\" function!");
}

void BCBrokerMessages::TurnCmdUpdateGameState()
{
	const Player* sender_p = (*game_sessions.GetSessionById(session_id)).GetPlayers().GetPlayerByUID( sender_player_id );
	const Banker& game_session = *game_sessions.GetSessionById(session_id);

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
			return;

		try
		{
			const_cast<Player*>(sender_p)->SetTurn();
			const_cast<Banker&>(game_session).SetReadyPlayers( game_session.GetReadyPlayers() + 1 );
		}
		catch ( const std::runtime_error& ex )
		{
			throw;
		}

		return;
	}

	throw std::runtime_error("Got null pointer of Player object in \"BCBrokerMessages::TurnCmdUpdateGameState\" function!");
}
void BCBrokerMessages::TurnCmdGetWypaToken()
{
	const Banker& game_session = *game_sessions.GetSessionById(session_id);

	Utility::itoa( game_session.GetAlivePlayers() - game_session.GetReadyPlayers(), result_message, MESSAGE_SIZE-1 );
}

#endif
