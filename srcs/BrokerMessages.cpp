#ifndef BROKER_MESSAGES_CPP_SENTINEL
#define BROKER_MESSAGES_CPP_SENTINEL


#include "BrokerMessages.hpp"
#include "MGLib.h"
#include <cstring>


static const char* const true_str = "true";
static const char* const false_str = "false";

extern const char* info_game_messages[];


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


GameMessages::GameMessages( const Banker& banker ) : BrokerMessages( banker )
{
	broker_actions.MakeBrokerActions( GameMessages::BROKER_ACTIONS_COUNT );

	left_player_id			=			0;
	time_to_start			=			0;
	sender_id				=			0;
	produced				=			0;
	total_charges			=			0;

	memset(result_message, 0, MESSAGE_SIZE);

	broker_actions[AUCTION_RESULTS_TOKEN]					=				[this]() {	AuctionResultsMessage();		};
	broker_actions[SUCCESS_CHARGES_PAY_TOKEN]				=				[this]() {	SuccessChargesPayMessage();		};
	broker_actions[PLAYER_BANKROT_TOKEN]					=				[this]() {	PlayerBankrotMessage();			};
	broker_actions[LOST_ALIVE_PLAYER_TOKEN]					=				[this]() {	LostAlivePlayerMessage();		};
	broker_actions[PRODUCED_TOKEN]							=				[this]() {	ProducedMessage();				};
	broker_actions[STARTINSECONDS_TOKEN]					=				[this]() {	StartInSecondsMessage();		};
	broker_actions[GAME_STARTED_TOKEN]						=				[this]() {	GameStartedMessage();			};
	broker_actions[STARTING_GAME_INFORMATION_TOKEN]			=				[this]() {	StartGameInfoMessage();			};
	broker_actions[STARTCANCELLED_TOKEN]					=				[this]() {	StartCancelledMessage();		};
	broker_actions[PAY_FACTORY_SUCCESS_TOKEN]				=				[this]() {	PayFactorySuccessMessage();		};
	broker_actions[FACTORY_BUILT_TOKEN]						=				[this]() {	FactoryBuiltMessage();			};
	broker_actions[VICTORY_MESSAGE_TOKEN]					=				[this]() {	VictoryMessage();				};
	broker_actions[GAME_ALREADY_STARTED_TOKEN]				=				[this]() {	GameAlreadyStartedMessage();	};
	broker_actions[SERVER_FULL_TOKEN]						=				[this]() {	ServerFullMessage();			};
	broker_actions[NEW_PLAYER_CONNECT_TOKEN]				=				[this]() {	NewPlayerConnectMessage();		};
	broker_actions[GAME_NOT_STARTED_TOKEN]					=				[this]() {	GameNotStartedMessage();		};
	broker_actions[LOST_LOBBY_PLAYER_TOKEN]					=				[this]() {	LostLobbyPlayerMessage();		};
	broker_actions[NEW_TURN_TOKEN]							=				[this]() {	NewTurnMessage();				};
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

const char* GameMessages::TakeMessage( int message_code )
{
	CheckMessageCode( message_code );

	broker_actions[message_code]();

	return result_message;
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
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_id );

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

	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_id );

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


BCBrokerMessages::BCBrokerMessages( const Banker& banker ) : BrokerMessages( banker )
{
	broker_actions.MakeBrokerActions( BCBrokerMessages::BROKER_ACTIONS_COUNT );

	sender_player_id		=			0;
	target_player_id		=			0;
	sources_amount			=			0;
	source_price			=			0;
	product_price			=			0;

	memset(result_message, 0, MESSAGE_SIZE);

	broker_actions[MARKET_SOURCES_AMOUNT_TOKEN]							=			[this]() {	MarketCmdSourcesAmount();			};
	broker_actions[MARKET_SOURCE_MIN_PRICE_TOKEN]						=			[this]() {	MarketCmdSourceMinPrice();			};
	broker_actions[MARKET_PRODUCTS_AMOUNT_TOKEN]						=			[this]() {	MarketCmdProductsAmount();			};
	broker_actions[MARKET_PRODUCT_MAX_PRICE_TOKEN]						=			[this]() {	MarketCmdProductMaxPrice();			};
	broker_actions[TARGET_PLAYER_NOT_FOUND_TOKEN]						=			[this]() { 	PlayerCmdIsTargetNotFound();		};
	broker_actions[TARGET_PLAYER_UID_TOKEN]								=			[this]() {	PlayerCmdGetTargetUID();			};
	broker_actions[TARGET_PLAYER_MONEY_TOKEN]							=			[this]() {	PlayerCmdGetTargetMoney();			};
	broker_actions[TARGET_PLAYER_INCOME_TOKEN]							=			[this]() {	PlayerCmdGetTargetIncome();			};
	broker_actions[TARGET_PLAYER_SOURCES_TOKEN]							=			[this]() {	PlayerCmdGetTargetSources();		};
	broker_actions[TARGET_PLAYER_PRODUCTS_TOKEN]						=			[this]() {	PlayerCmdGetTargetProducts();		};
	broker_actions[TARGET_PLAYER_WAIT_FACTORIES_TOKEN]					=			[this]() {	PlayerCmdGetTargetWaitFactories();	};
	broker_actions[TARGET_PLAYER_WORK_FACTORIES_TOKEN]					=			[this]() {	PlayerCmdGetTargetWorkFactories();	};
	broker_actions[TARGET_PLAYER_BUILT_FACTORIES_TOKEN]					=			[this]() {	PlayerCmdGetTargetBuiltFactories(); };
	broker_actions[SENDER_PLAYER_IS_BOT_TOKEN]							=			[this]() {	PlayerSenderIsBot();				};
	broker_actions[TARGET_PLAYER_PRODUCED_TOKEN]						=			[this]() {	PlayerCmdGetTargetProduced();		};
	broker_actions[ALIVE_PLAYERS_TOKEN]									=			[this]() {	ListCmdGetAlivePlayers();			};
	broker_actions[PLAYER_IS_TURN_TOKEN]								=			[this]() {	PlayerSenderIsTurn();				};
	broker_actions[PROD_CMD_SOURCES_CONDITION_SUCCESS_TOKEN]			=			[this]() {	ProdCmdSourcesCondition();			};
	broker_actions[PROD_CMD_MONEY_CONDITION_SUCCESS_TOKEN]				=			[this]() {	ProdCmdMoneyCondition();			};
	broker_actions[PROD_CMD_WAIT_FACTORIES_CONDITION_SUCCESS_TOKEN]		=			[this]() {	ProdCmdWaitFactoriesCondition();	};
	broker_actions[PROD_CMD_UPDATE_GAME_STATE_TOKEN]					=			[this]() {	ProdCmdUpdateGameState();			};
	broker_actions[BUILD_CMD_PLAYER_BUILDS_LIST_IS_EMPTY_TOKEN]			=			[this]() {	BuildCmdPlayerBuildsListIsEmpty();	};
	broker_actions[BUILD_CMD_PLAYER_GET_BUILDS_LIST_SIZE_TOKEN]			=			[this]() {	BuildCmdPlayerGetBuildsListSize();	};
	broker_actions[BUILD_CMD_PLAYER_GET_BUILDS_LIST_TOKEN]				=			[this]() {	BuildCmdPlayerGetBuildsList();		};
	broker_actions[BUILD_CMD_MONEY_CONDITION_SUCCESS_TOKEN]				=			[this]() {	BuildCmdMoneyCondition();			};
	broker_actions[BUILD_CMD_UPDATE_GAME_STATE_TOKEN]					=			[this]() {	BuildCmdUpdateGameState();			};
	broker_actions[BUY_CMD_IS_SENT_SOURCE_REQUEST]						=			[this]() { 	BuyCmdIsSentSourceRequest();		};
	broker_actions[BUY_CMD_SOURCES_CONDITION_SUCCESS_TOKEN]				=			[this]() {	BuyCmdSourcesCondition();			};
	broker_actions[BUY_CMD_PRICE_CONDITION_SUCCESS_TOKEN]				=			[this]() {	BuyCmdPriceCondition();				};
	broker_actions[BUY_CMD_MONEY_CONDITION_SUCCESS_TOKEN]				=			[this]() {	BuyCmdMoneyCondition();				};
	broker_actions[BUY_CMD_UPDATE_GAME_STATE_TOKEN]						=			[this]() {	BuyCmdUpdateGameState();			};
	broker_actions[SELL_CMD_IS_SENT_PRODUCT_REQUEST]					=			[this]() {	SellCmdIsSentProductRequest();		};
	broker_actions[SELL_CMD_AMOUNT_CONDITION_SUCCESS_TOKEN]				=			[this]() {	SellCmdAmountCondition();			};
	broker_actions[SELL_CMD_PRICE_CONDITION_SUCCESS_TOKEN]				=			[this]() {	SellCmdPriceCondition();			};
	broker_actions[SELL_CMD_UPDATE_GAME_STATE_TOKEN]					=			[this]() {	SellCmdUpdateGameState();			};
	broker_actions[TURN_CMD_UPDATE_GAME_STATE_TOKEN]					=			[this]() {	TurnCmdUpdateGameState();			};
	broker_actions[TURN_CMD_GET_WYPA_TOKEN]								=			[this]() {	TurnCmdGetWypaToken();				};
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

const char* BCBrokerMessages::TakeMessage( int message_code )
{
	CheckMessageCode( message_code );

	broker_actions[message_code]();

	return result_message;
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
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByNum( target_player_id );

	if ( target_p->IsFree() )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::PlayerCmdGetTargetUID()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByNum( target_player_id );

	itoa(target_p->GetUID(), result_message, MESSAGE_SIZE-1);
}

void BCBrokerMessages::PlayerCmdGetTargetMoney()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByNum( target_player_id );

	itoa(target_p->GetMoney(), result_message, MESSAGE_SIZE-1);
}

void BCBrokerMessages::PlayerCmdGetTargetIncome()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByNum( target_player_id );

	itoa(target_p->GetIncome(), result_message, MESSAGE_SIZE-1);
}

void BCBrokerMessages::PlayerCmdGetTargetSources()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByNum( target_player_id );

	itoa(target_p->GetSources(), result_message, MESSAGE_SIZE-1);
}

void BCBrokerMessages::PlayerCmdGetTargetProducts()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByNum( target_player_id );

	itoa(target_p->GetProducts(), result_message, MESSAGE_SIZE-1);
}

void BCBrokerMessages::PlayerCmdGetTargetWaitFactories()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByNum( target_player_id );

	itoa(target_p->GetWaitFactories(), result_message, MESSAGE_SIZE-1);
}

void BCBrokerMessages::PlayerCmdGetTargetWorkFactories()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByNum( target_player_id );

	itoa(target_p->GetWorkFactories(), result_message, MESSAGE_SIZE-1);
}

void BCBrokerMessages::PlayerCmdGetTargetBuiltFactories()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByNum( target_player_id );

	itoa(target_p->GetBuiltFactories(), result_message, MESSAGE_SIZE-1);
}

void BCBrokerMessages::PlayerSenderIsBot()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	if ( sender_p->IsBot() )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message,  false_str);
}

void BCBrokerMessages::PlayerCmdGetTargetProduced()
{
	const Player* target_p = GetGameSession().GetPlayers().GetPlayerByNum( target_player_id );

	itoa(target_p->GetProduced(), result_message, MESSAGE_SIZE-1);
}

void BCBrokerMessages::ListCmdGetAlivePlayers()
{
	itoa( GetGameSession().GetAlivePlayers(), result_message, MESSAGE_SIZE-1 );
}

void BCBrokerMessages::PlayerSenderIsTurn()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	if ( sender_p->IsTurn() )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::ProdCmdSourcesCondition()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	if ( sender_p->GetSources() >= 1 )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::ProdCmdMoneyCondition()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	if ( sender_p->GetMoney() >= PRODUCTION_PRODUCT_COST )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::ProdCmdWaitFactoriesCondition()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	if ( sender_p->GetWaitFactories() > 0 )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::ProdCmdUpdateGameState()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	const_cast<Player*>(sender_p)->SetWaitFactories( sender_p->GetWaitFactories() - 1 );
	const_cast<Player*>(sender_p)->SetWorkFactories( sender_p->GetWorkFactories() + 1 );
	const_cast<Player*>(sender_p)->SetSources( sender_p->GetSources() - 1 );
	const_cast<Player*>(sender_p)->SetMoney( sender_p->GetMoney() - PRODUCTION_PRODUCT_COST );
}

void BCBrokerMessages::BuildCmdPlayerBuildsListIsEmpty()
{
	const Player::BuildsList& player_builds = GetGameSession().GetPlayers().GetPlayerByNum(sender_player_id)->GetBuildsFactories();

	if ( player_builds.IsEmpty() )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::BuildCmdPlayerGetBuildsListSize()
{
	const Player::BuildsList& player_builds = GetGameSession().GetPlayers().GetPlayerByNum(sender_player_id)->GetBuildsFactories();

	itoa( player_builds.GetSize(), result_message, MESSAGE_SIZE-1 );
}

void BCBrokerMessages::BuildCmdPlayerGetBuildsList()
{
	const Player::BuildsList& player_builds = GetGameSession().GetPlayers().GetPlayerByNum(sender_player_id)->GetBuildsFactories();
	char build_number[10]	{	0x00	};
	char turns_left[10]		{	0x00	};


	int offset = 0;

	for ( Player::BuildsList::BuildsItem* node = player_builds.GetFirst(); node != nullptr; node = node->GetNext() )
	{
		concat_to_str(node->GetData().GetBuildNumber(), build_number, 9, result_message, &offset);
		concat_to_str(node->GetData().GetTurnsLeft(), turns_left, 9, result_message, &offset);
	}

	result_message[offset-1] = '\0';
}

void BCBrokerMessages::BuildCmdMoneyCondition()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	if ( sender_p->GetMoney() >= NEW_FACTORY_UNIT_COST/2 )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::BuildCmdUpdateGameState()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	const_cast<Player::BuildsList&>(sender_p->GetBuildsFactories()).Insert( sender_p->GetBuildsFactories().GetValidNum(), TURNS_TO_BUILD );
	const_cast<Player*>(sender_p)->SetMoney( sender_p->GetMoney() - NEW_FACTORY_UNIT_COST/2 );
	const_cast<Player*>(sender_p)->SetBuiltFactories( sender_p->GetBuiltFactories() + 1 );
}

void BCBrokerMessages::BuyCmdIsSentSourceRequest()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	if ( sender_p->IsSentSourceRequest() )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
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
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	if ( sender_p->GetMoney() < source_price * sources_amount )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::BuyCmdUpdateGameState()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	const_cast<Banker&>(GetGameSession()).GetSourcesRequests().Insert( sender_player_id, sources_amount, source_price );
	const_cast<Player*>(sender_p)->SetSentSourceRequest();
}

void BCBrokerMessages::SellCmdIsSentProductRequest()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	if ( sender_p->IsSentProductsRequest() )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::SellCmdAmountCondition()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	if ( ( products_amount > 0 ) && ( products_amount <= sender_p->GetProducts() ) )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::SellCmdPriceCondition()
{
	if ( ( product_price > 0 ) && ( product_price <= const_cast<Banker&>(GetGameSession()).GetCurrentMarketState().GetProductMaxPrice() ) )
	{
		strcpy(result_message, true_str);
		return;
	}

	strcpy(result_message, false_str);
}

void BCBrokerMessages::SellCmdUpdateGameState()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	const_cast<Banker&>(GetGameSession()).GetProductsRequests().Insert( sender_player_id, products_amount, product_price );
	const_cast<Player*>(sender_p)->SetSentProductsRequest();
}

void BCBrokerMessages::TurnCmdUpdateGameState()
{
	const Player* sender_p = GetGameSession().GetPlayers().GetPlayerByNum( sender_player_id );

	const_cast<Player*>(sender_p)->SetTurn();
	const_cast<Banker&>(GetGameSession()).SetReadyPlayers( GetGameSession().GetReadyPlayers() + 1 );
}
void BCBrokerMessages::TurnCmdGetWypaToken()
{
	itoa( GetGameSession().GetAlivePlayers() - GetGameSession().GetReadyPlayers(), result_message, MESSAGE_SIZE-1 );
}

#endif
