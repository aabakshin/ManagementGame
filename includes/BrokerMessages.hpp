#ifndef BROKER_MESSAGES_HPP_SENTINEL
#define BROKER_MESSAGES_HPP_SENTINEL


#include "Banker.hpp"
#include <functional>


class BrokerMessages
{
public:

	class BrokerActions
	{
	private:
		std::function<void()>* actions;
		int actions_count;
	public:
		BrokerActions() {}
		void MakeBrokerActions( int );
		std::function<void()>& operator[]( int );
		~BrokerActions();
	private:
		BrokerActions( const BrokerActions& ) = delete;
		BrokerActions( BrokerActions&& ) = delete;
		void operator=( const BrokerActions& ) = delete;
	};

private:
	const Banker& game_session;
public:
	BrokerMessages( const Banker& );
	const Banker& GetGameSession() const { return game_session; }
	virtual void PutMessage( const char**, int ) = 0;
	virtual const char* TakeMessage( int ) = 0;
	virtual void CheckMessageCode( int ) const = 0;
	virtual ~BrokerMessages() {}
private:
	BrokerMessages( const BrokerMessages& ) = delete;
	BrokerMessages( BrokerMessages&& ) = delete;
	void operator=( const BrokerMessages& ) = delete;
};

class BCBrokerMessages : public BrokerMessages
{
public:

	enum
	{
				MARKET_SOURCES_AMOUNT_TOKEN,
				MARKET_SOURCE_MIN_PRICE_TOKEN,
				MARKET_PRODUCTS_AMOUNT_TOKEN,
				MARKET_PRODUCT_MAX_PRICE_TOKEN,
				TARGET_PLAYER_NOT_FOUND_TOKEN,
				TARGET_PLAYER_UID_TOKEN,
				TARGET_PLAYER_MONEY_TOKEN,
				TARGET_PLAYER_INCOME_TOKEN,
				TARGET_PLAYER_SOURCES_TOKEN,
				TARGET_PLAYER_PRODUCTS_TOKEN,
				TARGET_PLAYER_WAIT_FACTORIES_TOKEN,
				TARGET_PLAYER_WORK_FACTORIES_TOKEN,
				TARGET_PLAYER_BUILT_FACTORIES_TOKEN,
				SENDER_PLAYER_IS_BOT_TOKEN,
				TARGET_PLAYER_PRODUCED_TOKEN,
				ALIVE_PLAYERS_TOKEN,
				PLAYER_IS_TURN_TOKEN,
				PROD_CMD_SOURCES_CONDITION_SUCCESS_TOKEN,
				PROD_CMD_MONEY_CONDITION_SUCCESS_TOKEN,
				PROD_CMD_WAIT_FACTORIES_CONDITION_SUCCESS_TOKEN,
				PROD_CMD_UPDATE_GAME_STATE_TOKEN,
				BUILD_CMD_PLAYER_BUILDS_LIST_IS_EMPTY_TOKEN,
				BUILD_CMD_PLAYER_GET_BUILDS_LIST_SIZE_TOKEN,
				BUILD_CMD_PLAYER_GET_BUILDS_LIST_TOKEN,
				BUILD_CMD_MONEY_CONDITION_SUCCESS_TOKEN,
				BUILD_CMD_UPDATE_GAME_STATE_TOKEN,
				BUY_CMD_IS_SENT_SOURCE_REQUEST,
				BUY_CMD_SOURCES_CONDITION_SUCCESS_TOKEN,
				BUY_CMD_PRICE_CONDITION_SUCCESS_TOKEN,
				BUY_CMD_MONEY_CONDITION_SUCCESS_TOKEN,
				BUY_CMD_UPDATE_GAME_STATE_TOKEN,
				SELL_CMD_IS_SENT_PRODUCT_REQUEST,
				SELL_CMD_AMOUNT_CONDITION_SUCCESS_TOKEN,
				SELL_CMD_PRICE_CONDITION_SUCCESS_TOKEN,
				SELL_CMD_UPDATE_GAME_STATE_TOKEN,
				TURN_CMD_UPDATE_GAME_STATE_TOKEN,
				TURN_CMD_GET_WYPA_TOKEN
	};

	enum
	{
				SENDER_PLAYER_ID_PARAM_TOKEN,
				TARGET_PLAYER_ID_PARAM_TOKEN,
				SOURCES_AMOUNT_PARAM_TOKEN,
				SOURCE_PRICE_PARAM_TOKEN,
				PRODUCTS_AMOUNT_PARAM_TOKEN,
				PRODUCT_PRICE_PARAM_TOKEN
	};

	enum
	{
				BROKER_ACTIONS_COUNT	=			37
	};

	enum
	{
				MESSAGE_SIZE			=			300
	};

private:
	int sender_player_id;
	int target_player_id;
	int sources_amount;
	int source_price;
	int products_amount;
	int product_price;

	char result_message[MESSAGE_SIZE];
	BrokerActions broker_actions;
public:
	BCBrokerMessages( const Banker& );
	virtual void PutMessage( const char**, int ) override;
	virtual const char* TakeMessage( int ) override;
	virtual ~BCBrokerMessages() {}
private:
	virtual void CheckMessageCode( int ) const override;
	BCBrokerMessages( const BCBrokerMessages& ) = delete;
	BCBrokerMessages( BCBrokerMessages&& ) = delete;
	void operator=( const BCBrokerMessages& ) = delete;

	void MarketCmdSourcesAmount();
	void MarketCmdSourceMinPrice();
	void MarketCmdProductsAmount();
	void MarketCmdProductMaxPrice();
	void PlayerCmdIsTargetNotFound();
	void PlayerCmdGetTargetUID();
	void PlayerCmdGetTargetMoney();
	void PlayerCmdGetTargetIncome();
	void PlayerCmdGetTargetSources();
	void PlayerCmdGetTargetProducts();
	void PlayerCmdGetTargetWaitFactories();
	void PlayerCmdGetTargetWorkFactories();
	void PlayerCmdGetTargetBuiltFactories();
	void PlayerSenderIsBot();
	void PlayerCmdGetTargetProduced();
	void ListCmdGetAlivePlayers();
	void PlayerSenderIsTurn();
	void ProdCmdSourcesCondition();
	void ProdCmdMoneyCondition();
	void ProdCmdWaitFactoriesCondition();
	void ProdCmdUpdateGameState();
	void BuildCmdPlayerBuildsListIsEmpty();
	void BuildCmdPlayerGetBuildsListSize();
	void BuildCmdPlayerGetBuildsList();
	void BuildCmdMoneyCondition();
	void BuildCmdUpdateGameState();
	void BuyCmdIsSentSourceRequest();
	void BuyCmdSourcesCondition();
	void BuyCmdPriceCondition();
	void BuyCmdMoneyCondition();
	void BuyCmdUpdateGameState();
	void SellCmdIsSentProductRequest();
	void SellCmdAmountCondition();
	void SellCmdPriceCondition();
	void SellCmdUpdateGameState();
	void TurnCmdUpdateGameState();
	void TurnCmdGetWypaToken();
};

class GameMessages : public BrokerMessages
{
public:

	enum
	{
				AUCTION_RESULTS_TOKEN,
				SUCCESS_CHARGES_PAY_TOKEN,
				PLAYER_BANKROT_TOKEN,
				LOST_ALIVE_PLAYER_TOKEN,
				PRODUCED_TOKEN,
				STARTINSECONDS_TOKEN,
				GAME_STARTED_TOKEN,
				STARTING_GAME_INFORMATION_TOKEN,
				STARTCANCELLED_TOKEN,
				PAY_FACTORY_SUCCESS_TOKEN,
				FACTORY_BUILT_TOKEN,
				VICTORY_MESSAGE_TOKEN,
				GAME_ALREADY_STARTED_TOKEN,
				SERVER_FULL_TOKEN,
				NEW_PLAYER_CONNECT_TOKEN,
				GAME_NOT_STARTED_TOKEN,
				LOST_LOBBY_PLAYER_TOKEN,
				NEW_TURN_TOKEN
	};

	enum
	{
				LEFT_PLAYER_ID_PARAM_TOKEN,
				TIME_TO_START_PARAM_TOKEN,
				SENDER_ID_PARAM_TOKEN,
				PRODUCED_AMOUNT_PARAM_TOKEN,
				TOTAL_CHARGES_PARAM_TOKEN
	};

	enum
	{
				BROKER_ACTIONS_COUNT	=			18
	};

	enum
	{
				MESSAGE_SIZE			=			400
	};

private:
	int left_player_id;
	int time_to_start;
	int sender_id;
	int produced;
	int total_charges;

	char result_message[MESSAGE_SIZE];
	BrokerActions broker_actions;
public:
	GameMessages( const Banker& );
	virtual void PutMessage( const char**, int ) override;
	virtual const char* TakeMessage( int ) override;
	virtual ~GameMessages() {}
private:
	virtual void CheckMessageCode( int ) const override;
	GameMessages( const GameMessages& ) = delete;
	GameMessages( GameMessages&& ) = delete;
	void operator=( const GameMessages& ) = delete;

	void AuctionResultsMessage();
	void SuccessChargesPayMessage();
	void PlayerBankrotMessage();
	void LostAlivePlayerMessage();
	void ProducedMessage();
	void StartInSecondsMessage();
	void GameStartedMessage();
	void StartGameInfoMessage();
	void StartCancelledMessage();
	void PayFactorySuccessMessage();
	void FactoryBuiltMessage();
	void VictoryMessage();
	void GameAlreadyStartedMessage();
	void ServerFullMessage();
	void NewPlayerConnectMessage();
	void GameNotStartedMessage();
	void LostLobbyPlayerMessage();
	void NewTurnMessage();
};

#endif
