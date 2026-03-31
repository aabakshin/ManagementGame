#ifndef BROKER_MESSAGES_CPP_SENTINEL
#define BROKER_MESSAGES_CPP_SENTINEL


#include "BrokerMessages.hpp"
#include "MGLib.h"
#include <cstring>


static const char* const true_str = "true";
static const char* const false_str = "false";

extern const char* info_game_messages[];



template <class T, class U>
void EncapsulatedBrokerMessages<T,U>::MakeBroker( const U& context_object )
{
	brokerPTR = new T( context_object );
}

template <class T, class U>
template <class X, class Y, class Z>
void EncapsulatedBrokerMessages<T,U>::MakeBroker( const U& context_object1, const X& context_object2, const Y& context_object3, const Z& context_object4 )
{
	brokerPTR = new T( context_object1, context_object2, context_object3, context_object4 );
}

template <class T, class U>
const T& EncapsulatedBrokerMessages<T,U>::GetBroker() const
{
	return const_cast<const T&>(*brokerPTR);
}

template <class T, class U>
EncapsulatedBrokerMessages<T,U>::~EncapsulatedBrokerMessages()
{
	delete brokerPTR;
}

void BrokerMessages::BrokerActions::MakeBrokerActions( int a_count )
{
	actions_count = a_count;

	actions = new std::function<void()>[ actions_count ];
}

std::function<void()>& BrokerMessages::BrokerActions::operator[]( int idx )
{
	if ( ( idx < 0 ) || ( idx > actions_count-1 ) )
	{
		return actions[0];
		//throw IncorrectBrockerActionIdxException();
	}

	return actions[idx];
}

BrokerMessages::BrokerActions::~BrokerActions()
{
	if ( actions != nullptr )
		delete[] actions;
}

const char* BrokerMessages::TakeMessage( int message_code )
{
	CheckMessageCode( message_code );

	broker_actions[message_code]();

	return result_message;
}


MulticastActionsExec::MulticastActionsExec( const Banker& banker, const Sender& s, const MessageTokens& mt, const EncapsulatedBrokerMessages<GameMessages, Banker>& egm )
	: game_session( banker ), sender( s ), msg_tokens( mt ), EGameMessages( egm )
{
	BrokerActions& br_acts = const_cast<BrokerActions&>(GetBrokerActions());
	br_acts.MakeBrokerActions( MulticastActionsExec::BROKER_ACTIONS_COUNT );

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
}

void MulticastActionsExec::PutMessage( const char** message_tokens, int tokens_count )
{
	for ( int i = AUCTION_TYPE_PARAM_TOKEN; i < tokens_count; ++i )
	{
		if ( ( message_tokens[i] != nullptr ) && ( strcmp(message_tokens[i], "") != 0 ) )
		{
			switch ( i )
			{
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

	// throw IncorrectMessageCodeExc
}

void MulticastActionsExec::SendReportOnTurn()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			printf("\tPlayer #%d:   money: %dР   produced products: %d\n", p->GetUID(), p->GetMoney(), p->GetProduced());
		}
	}
}

void MulticastActionsExec::AddEmptyAuctionRequest()
{
	List<Item<MarketData>>& requests = ( auction_type == SOURCE_AUCTION ) ? const_cast<Banker&>(game_session).GetSourcesRequests() : const_cast<Banker&>(game_session).GetProductsRequests();

	for ( int i = 0; i < MAX_PLAYERS; ++i )
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
				data.MakeData( p->GetUID(), 0, 0 );
				requests.Insert( data );
			}
		}
	}
}

void MulticastActionsExec::PayCharges()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			if ( !p->IsBankrot() )
			{
				int total_charges = 0;
				total_charges = p->GetSources() * SOURCE_UNIT_CHARGE;
				total_charges += p->GetProducts() * PRODUCT_UNIT_CHARGE;
				total_charges += p->GetWaitFactories() * FACTORY_UNIT_CHARGE;
				total_charges += p->GetWorkFactories() * FACTORY_UNIT_CHARGE;

				itoa( total_charges, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::TOTAL_CHARGES_PARAM_TOKEN]), MESSAGE_TOKEN_SIZE-1 );
				const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::TOTAL_CHARGES_PARAM_TOKEN+1 );

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
		}
	}
}

void MulticastActionsExec::CheckBuildingFactories()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			if ( !p->IsBankrot() )
			{
				for ( Item<BuildsData>* node = p->GetBuildsFactories().GetFirst(); node != nullptr; )
				{
					if ( node->GetData().GetTurnsLeft() == 1 )
					{
						int total_charges = NEW_FACTORY_UNIT_COST / 2;

						itoa( total_charges, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::TOTAL_CHARGES_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
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
		}
	}
}

void MulticastActionsExec::PrepareNewTurn()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
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

				itoa( p->GetUID(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::SENDER_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
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
					itoa( p->GetProduced(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::PRODUCED_AMOUNT_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
					const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::PRODUCED_AMOUNT_PARAM_TOKEN+1 );

					const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::PRODUCED_TOKEN ), p->GetFd(), p->GetAddr() );
				}
			}
		}
	}
}

void MulticastActionsExec::PreparePlayersState()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			const_cast<Player*>(p)->SetMoney( START_MONEY );
			const_cast<Player*>(p)->SetOldMoney( p->GetMoney() );
			const_cast<Player*>(p)->SetSources( START_SOURCES );
			const_cast<Player*>(p)->SetProducts( START_PRODUCTS );
			const_cast<Player*>(p)->SetWaitFactories( START_FACTORIES );

			itoa( p->GetUID(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::SENDER_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
			const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::SENDER_ID_PARAM_TOKEN+1 );

			const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::STARTING_GAME_INFORMATION_TOKEN ), p->GetFd(), p->GetAddr() );
		}
	}
}

void MulticastActionsExec::SendAuctionsResults()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::AUCTION_RESULTS_TOKEN ), p->GetFd(), p->GetAddr() );
		}
	}
}

void MulticastActionsExec::SendPlayersBankrot()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			if ( p->IsBankrot() )
			{
				const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::PLAYER_BANKROT_TOKEN ), p->GetFd(), p->GetAddr() );
			}
		}
	}
}

void MulticastActionsExec::SendNewPlayerConnect()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::NEW_PLAYER_CONNECT_TOKEN ), p->GetFd(), p->GetAddr() );
		}
	}
}

void MulticastActionsExec::SendStartTime()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			itoa( TIME_TO_START, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::TIME_TO_START_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
			const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::TIME_TO_START_PARAM_TOKEN+1 );
			const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::STARTINSECONDS_TOKEN ), p->GetFd(), p->GetAddr() );
		}
	}
}

void MulticastActionsExec::SendStartCancelled()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
			const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::STARTCANCELLED_TOKEN ), p->GetFd(), p->GetAddr() );
	}
}

void MulticastActionsExec::SendGameStarted()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = game_session.GetPlayers()[i];
		if ( !p->IsFree() )
			const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::GAME_STARTED_TOKEN ), p->GetFd(), p->GetAddr() );
	}
}

void MulticastActionsExec::QuitPlayer()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
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
						printf("\n\n<<<<< GAME IS FINISHED. PLAYER #%d IS WINNER! >>>>>\n\n", p->GetUID());
					}
					else
					{
						itoa( left_player_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::LEFT_PLAYER_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
						const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::LEFT_PLAYER_ID_PARAM_TOKEN+1 );

						const_cast<Sender&>(sender).SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::LOST_ALIVE_PLAYER_TOKEN ), p->GetFd(), p->GetAddr() );
					}
				}
			}
		}
	}
}


GameMessages::GameMessages( const Banker& banker ) : game_session( banker )
{
	BrokerActions& br_acts = const_cast<BrokerActions&>(GetBrokerActions());
	br_acts.MakeBrokerActions( GameMessages::BROKER_ACTIONS_COUNT );

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
	for ( int i = LEFT_PLAYER_ID_PARAM_TOKEN; i < tokens_count; ++i )
	{
		if ( ( message_tokens[i] != nullptr ) && ( strcmp(message_tokens[i], "") != 0 ) )
		{
			switch ( i )
			{
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

	// throw IncorrectMessageCodeException();
}

void GameMessages::LostLobbyPlayerMessage()
{
	char lp_buf[10];
	itoa(GetGameSession().GetLobbyPlayers(), lp_buf, 9);

	char max_pl_buf[10];
	itoa(MAX_PLAYERS, max_pl_buf, 9);


	const char* message_tokens[] =
	{
				info_game_messages[LOST_LOBBY_PLAYER],
				lp_buf,
				max_pl_buf,
				nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 3 );
}

void GameMessages::LostAlivePlayerMessage()
{
	char ap_buf[10];
	itoa( GetGameSession().GetAlivePlayers(), ap_buf, 9 );

	char left_p_num_buf[10];
	itoa( left_player_id, left_p_num_buf, 9 );


	const char* message_tokens[] =
	{
				info_game_messages[LOST_ALIVE_PLAYER],
				ap_buf,
				left_p_num_buf,
				nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 3 );
}

void GameMessages::VictoryMessage()
{
	const char* message_tokens[] =
	{
				info_game_messages[VICTORY_MESSAGE],
				nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}

void GameMessages::StartInSecondsMessage()
{
	char tts[10];
	itoa(time_to_start, tts, 9);


	const char* message_tokens[] =
	{
				info_game_messages[STARTINSECONDS],
				tts,
				nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 2 );
}

void GameMessages::GameStartedMessage()
{
	const char* message_tokens[] =
	{
				info_game_messages[GAME_STARTED],
				nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}

void GameMessages::GameAlreadyStartedMessage()
{
	const char* message_tokens[] =
	{
				info_game_messages[GAME_ALREADY_STARTED],
				nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}

void GameMessages::StartGameInfoMessage()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_id );
	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			//throw PlayerRecordIsFreeException();
		}
	}
	else
	{
		return;
		//throw NullPointerException();
	}

	char p_num[10];
	itoa(sender_p->GetUID(), p_num, 9);

	char ap[10];
	itoa(GetGameSession().GetAlivePlayers(), ap, 9);

	char tn[10];
	itoa(GetGameSession().GetTurnNumber(), tn, 9);

	char p_money[20];
	itoa(sender_p->GetMoney(), p_money, 19);

	char p_sources[10];
	itoa(sender_p->GetSources(), p_sources, 9);

	char p_products[10];
	itoa(sender_p->GetProducts(), p_products, 9);

	char p_wf[10];
	itoa(sender_p->GetWaitFactories(), p_wf, 9);

	char p_wrkf[10];
	itoa(sender_p->GetWorkFactories(), p_wrkf, 9);

	char p_bf[10];
	itoa(sender_p->GetBuiltFactories(), p_bf, 9);

	char sa[10];
	itoa(const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetSourcesAmount(), sa, 9);

	char smp[10];
	itoa(const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetSourceMinPrice(), smp, 9);

	char pa[10];
	itoa(const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetProductsAmount(), pa, 9);

	char pmp[10];
	itoa(const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetProductMaxPrice(), pmp, 9);


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

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, sender_p->IsBot() ? 14 : 10 );
}

void GameMessages::StartCancelledMessage()
{
	const char* message_tokens[] =
	{
				info_game_messages[STARTCANCELLED],
				nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}

void GameMessages::NewPlayerConnectMessage()
{
	char lp_buf[10];
	itoa(GetGameSession().GetLobbyPlayers(), lp_buf, 9);

	char max_pl_buf[10];
	itoa(MAX_PLAYERS, max_pl_buf, 9);


	const char* message_tokens[] =
	{
				info_game_messages[NEW_PLAYER_CONNECT],
				lp_buf,
				max_pl_buf,
				nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 3 );
}

void GameMessages::GameNotStartedMessage()
{
	const char* message_tokens[] =
	{
				info_game_messages[GAME_NOT_STARTED],
				nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
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

	player_report pr[MAX_PLAYERS];
	memset(pr, 0, sizeof(player_report) * MAX_PLAYERS);


	const char* message_tokens[ PL_REP_FIELDS_NUM * MAX_PLAYERS + 1 ] { nullptr };


	message_tokens[0] = info_game_messages[AUCTION_RESULTS];

	int tokens_amount = 1;
	for ( int i = 0; i < GetGameSession().GetReadyPlayers(); ++i )
	{
		itoa(GetGameSession().GetTurnNumber(), pr[i].tn, TURN_SIZE);
		message_tokens[tokens_amount] = pr[i].tn;
		++tokens_amount;

		itoa(GetGameSession().GetPlayers()[i]->GetUID(), pr[i].pnum, PLAYER_NUM_SIZE);
		message_tokens[tokens_amount] = pr[i].pnum;
		++tokens_amount;

		itoa(GetGameSession().GetPlayers()[i]->GetAuctionReport().GetSoldSources(), pr[i].ssnum, SOLD_SOURCES_SIZE);
		message_tokens[tokens_amount] = pr[i].ssnum;
		++tokens_amount;

		itoa(GetGameSession().GetPlayers()[i]->GetAuctionReport().GetSoldPrice(), pr[i].spnum, SOLD_PRICE_SIZE);
		message_tokens[tokens_amount] = pr[i].spnum;
		++tokens_amount;

		itoa(GetGameSession().GetPlayers()[i]->GetAuctionReport().GetBoughtProducts(), pr[i].bpnum, BOUGHT_PRODS_SIZE);
		message_tokens[tokens_amount] = pr[i].bpnum;
		++tokens_amount;

		itoa(GetGameSession().GetPlayers()[i]->GetAuctionReport().GetBoughtPrice(), pr[i].bprnum, BOUGHT_PRICE_SIZE);
		message_tokens[tokens_amount] = pr[i].bprnum;
		++tokens_amount;
	}

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, tokens_amount );
}

void GameMessages::NewTurnMessage()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_id );
	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			//throw PlayerRecordIsFreeException();
		}
	}
	else
	{
		return;
		//throw NullPointerException();
	}

	char tn[10];
	itoa(GetGameSession().GetTurnNumber(), tn, 9);

	char sa[10];
	itoa(const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetSourcesAmount(), sa, 9);

	char smp[10];
	itoa(const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetSourceMinPrice(), smp, 9);

	char pa[10];
	itoa(const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetProductsAmount(), pa, 9);

	char pmp[10];
	itoa(const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetProductMaxPrice(), pmp, 9);


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

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, sender_p->IsBot() ? 6 : 2 );
}

void GameMessages::ProducedMessage()
{
	char am_prd[10];
	itoa(produced, am_prd, 9);


	const char* message_tokens[] =
	{
				info_game_messages[PRODUCED],
				am_prd,
				nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 2 );
}

void GameMessages::SuccessChargesPayMessage()
{
	char charges[20];
	itoa(total_charges, charges, 19);


	const char* message_tokens[] =
	{
				info_game_messages[SUCCESS_CHARGES_PAY],
				charges,
				nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 2 );
}

void GameMessages::PlayerBankrotMessage()
{
	char charges[20];
	itoa(total_charges, charges, 19);


	const char* message_tokens[] =
	{
				info_game_messages[PLAYER_BANKROT],
				charges,
				nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 2 );
}

void GameMessages::PayFactorySuccessMessage()
{
	const char* message_tokens[] =
	{
			info_game_messages[PAY_FACTORY_SUCCESS],
			nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}

void GameMessages::FactoryBuiltMessage()
{
	const char* message_tokens[] =
	{
			info_game_messages[FACTORY_BUILT],
			nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}

void GameMessages::ServerFullMessage()
{
	const char* message_tokens[] =
	{
				info_game_messages[SERVER_FULL],
				nullptr
	};

	concat_tokens( result_message, MESSAGE_SIZE, message_tokens, 1 );
}


BCBrokerMessages::BCBrokerMessages( const Banker& banker ) : game_session( banker )
{
	BrokerActions& br_acts = const_cast<BrokerActions&>(GetBrokerActions());
	br_acts.MakeBrokerActions( BCBrokerMessages::BROKER_ACTIONS_COUNT );

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
	for ( int i = SENDER_PLAYER_ID_PARAM_TOKEN; i < tokens_count; ++i )
	{
		if ( ( message_tokens[i] != nullptr ) && ( strcmp(message_tokens[i], "") != 0 ) )
		{
			switch ( i )
			{
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

	// throw IncorrectMessageCodeException();
}

void BCBrokerMessages::MarketCmdSourcesAmount()
{
	itoa( const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetSourcesAmount(), result_message, MESSAGE_SIZE-1 );
}

void BCBrokerMessages::MarketCmdSourceMinPrice()
{
	itoa( const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetSourceMinPrice(), result_message, MESSAGE_SIZE-1 );
}

void BCBrokerMessages::MarketCmdProductsAmount()
{
	itoa( const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetProductsAmount(), result_message, MESSAGE_SIZE-1 );
}

void BCBrokerMessages::MarketCmdProductMaxPrice()
{
	itoa( const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetProductMaxPrice(), result_message, MESSAGE_SIZE-1 );
}

void BCBrokerMessages::PlayerCmdIsTargetNotFound()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByUID( target_player_id );

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
	//throw NullPointerException();
}

void BCBrokerMessages::PlayerCmdGetTargetUID()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		itoa(target_p->GetUID(), result_message, MESSAGE_SIZE-1);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::PlayerCmdGetTargetMoney()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		itoa(target_p->GetMoney(), result_message, MESSAGE_SIZE-1);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::PlayerCmdGetTargetIncome()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		itoa(target_p->GetIncome(), result_message, MESSAGE_SIZE-1);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::PlayerCmdGetTargetSources()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		itoa(target_p->GetSources(), result_message, MESSAGE_SIZE-1);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::PlayerCmdGetTargetProducts()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		itoa(target_p->GetProducts(), result_message, MESSAGE_SIZE-1);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::PlayerCmdGetTargetWaitFactories()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		itoa(target_p->GetWaitFactories(), result_message, MESSAGE_SIZE-1);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::PlayerCmdGetTargetWorkFactories()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		itoa(target_p->GetWorkFactories(), result_message, MESSAGE_SIZE-1);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::PlayerCmdGetTargetBuiltFactories()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		itoa(target_p->GetBuiltFactories(), result_message, MESSAGE_SIZE-1);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::PlayerSenderIsBot()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		if ( sender_p->IsBot() )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message,  false_str);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::PlayerCmdGetTargetProduced()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByUID( target_player_id );

	if ( target_p != nullptr )
	{
		if ( target_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		itoa(target_p->GetProduced(), result_message, MESSAGE_SIZE-1);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::ListCmdGetAlivePlayers()
{
	itoa( GetGameSession().GetAlivePlayers(), result_message, MESSAGE_SIZE-1 );
}

void BCBrokerMessages::PlayerSenderIsTurn()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		if ( sender_p->IsTurn() )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::ProdCmdSourcesCondition()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		if ( sender_p->GetSources() >= 1 )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::ProdCmdMoneyCondition()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		if ( sender_p->GetMoney() >= PRODUCTION_PRODUCT_COST )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::ProdCmdWaitFactoriesCondition()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		if ( sender_p->GetWaitFactories() > 0 )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::ProdCmdUpdateGameState()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		const_cast<Player*>(sender_p)->SetWaitFactories( sender_p->GetWaitFactories() - 1 );
		const_cast<Player*>(sender_p)->SetWorkFactories( sender_p->GetWorkFactories() + 1 );
		const_cast<Player*>(sender_p)->SetSources( sender_p->GetSources() - 1 );
		const_cast<Player*>(sender_p)->SetMoney( sender_p->GetMoney() - PRODUCTION_PRODUCT_COST );
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::BuildCmdPlayerBuildsListIsEmpty()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID(sender_player_id);

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		const List<Item<BuildsData>>& player_builds = sender_p->GetBuildsFactories();

		if ( player_builds.IsEmpty() )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::BuildCmdPlayerGetBuildsListSize()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID(sender_player_id);

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		const List<Item<BuildsData>>& player_builds = sender_p->GetBuildsFactories();
		itoa( player_builds.GetSize(), result_message, MESSAGE_SIZE-1 );
	}

	//throw NullPointerException();
}

void BCBrokerMessages::BuildCmdPlayerGetBuildsList()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID(sender_player_id);

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		const List<Item<BuildsData>>& player_builds = sender_p->GetBuildsFactories();

		char build_number[10]	{	0x00	};
		char turns_left[10]		{	0x00	};

		int offset = 0;

		for ( Item<BuildsData>* node = player_builds.GetFirst(); node != nullptr; node = node->GetNext() )
		{
			concat_to_str(node->GetData().GetBuildNumber(), build_number, 9, result_message, &offset);
			concat_to_str(node->GetData().GetTurnsLeft(), turns_left, 9, result_message, &offset);
		}

		result_message[offset-1] = '\0';
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::BuildCmdMoneyCondition()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		if ( sender_p->GetMoney() >= NEW_FACTORY_UNIT_COST/2 )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::BuildCmdUpdateGameState()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		BuildsData data;
		data.MakeData( sender_p->GetBuildsFactories().GetValidNum(), TURNS_TO_BUILD );

		const_cast<List<Item<BuildsData>>&>(sender_p->GetBuildsFactories()).Insert( data );
		const_cast<Player*>(sender_p)->SetMoney( sender_p->GetMoney() - NEW_FACTORY_UNIT_COST/2 );
		const_cast<Player*>(sender_p)->SetBuiltFactories( sender_p->GetBuiltFactories() + 1 );
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::BuyCmdIsSentSourceRequest()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		if ( sender_p->IsSentSourceRequest() )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::BuyCmdSourcesCondition()
{
	if ( ( sources_amount > 0 ) && ( sources_amount <= const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetSourcesAmount() ) )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::BuyCmdPriceCondition()
{
	if ( ( source_price >= const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetSourceMinPrice() ) )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::BuyCmdMoneyCondition()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		if ( sender_p->GetMoney() < source_price * sources_amount )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::BuyCmdUpdateGameState()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		MarketData data;
		data.MakeData( sender_player_id, sources_amount, source_price );

		const_cast<Banker&>(GetGameSession()).GetSourcesRequests().Insert( data );
		const_cast<Player*>(sender_p)->SetSentSourceRequest();
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::SellCmdIsSentProductRequest()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		if ( sender_p->IsSentProductsRequest() )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::SellCmdAmountCondition()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		if ( ( products_amount > 0 ) && ( products_amount <= sender_p->GetProducts() ) )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::SellCmdPriceCondition()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		if ( ( product_price > 0 ) && ( product_price <= const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetProductMaxPrice() ) )
		{
			strcpy(result_message, true_str);
			return;
		}

		strcpy(result_message, false_str);
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::SellCmdUpdateGameState()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		MarketData data;
		data.MakeData( sender_player_id, products_amount, product_price );

		const_cast<Banker&>(GetGameSession()).GetProductsRequests().Insert( data );
		const_cast<Player*>(sender_p)->SetSentProductsRequest();
		return;
	}

	//throw NullPointerException();
}

void BCBrokerMessages::TurnCmdUpdateGameState()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByUID( sender_player_id );

	if ( sender_p != nullptr )
	{
		if ( sender_p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		const_cast<Player*>(sender_p)->SetTurn();
		const_cast<Banker&>(GetGameSession()).SetReadyPlayers( GetGameSession().GetReadyPlayers() + 1 );
		return;
	}

	//throw NullPointerException();
}
void BCBrokerMessages::TurnCmdGetWypaToken()
{
	itoa( GetGameSession().GetAlivePlayers() - GetGameSession().GetReadyPlayers(), result_message, MESSAGE_SIZE-1 );
}

#endif
