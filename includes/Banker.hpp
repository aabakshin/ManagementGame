#ifndef BANKER_HPP
#define BANKER_HPP


#include "RegisteredPlayers.hpp"
#include "List.hpp"


enum
{
	MIN_PLAYERS_TO_START			=							   2,
	TIME_TO_START					=							  10,
	MARKET_LEVEL_NUMBER				=							   5,
	START_MARKET_LEVEL				=							   3
};

enum
{
	START_MONEY						=						  100000,
	START_SOURCES					=						   	   4,
	START_PRODUCTS					=							   2,
	START_FACTORIES					=					           2,
	SOURCE_UNIT_CHARGE				=						     300,
	PRODUCT_UNIT_CHARGE				=						     500,
	FACTORY_UNIT_CHARGE				=						    1000,
	NEW_FACTORY_UNIT_COST			=						    5000,
	PRODUCTION_PRODUCT_COST			=						    2000
};

enum
{
	SOURCE_AUCTION					=							   0,
	PRODUCTION_AUCTION				=							   1
};


class MarketState
{
private:
	int sources_amount;
	int source_min_price;
	int products_amount;
	int product_max_price;
public:
	MarketState();
	int GetSourcesAmount() const { return sources_amount; }
	int GetSourceMinPrice() const { return source_min_price; }
	int GetProductsAmount() const { return products_amount; }
	int GetProductMaxPrice() const { return product_max_price; }
	void SetSourcesAmount( int );
	void SetSourceMinPrice( int );
	void SetProductsAmount( int );
	void SetProductMaxPrice( int );
private:
	MarketState( const MarketState& ) = delete;
	MarketState( MarketState&& ) = delete;
	void operator=( const MarketState& ) = delete;
};

class MarketData
{
	int player_id;
	int amount;
	int price;
	bool success;
public:
	MarketData( int p_num = 0, int amnt = 0, int pr = 0 );
	MarketData( const MarketData& );
	MarketData( MarketData&& );
	void operator=( const MarketData& );
	void MakeData( int p_num, int amnt, int pr );
	int GetPlayerNum() const { return player_id; }
	int GetAmount() const { return amount; }
	int GetPrice() const { return price; }
	void SetPlayerNum( int value );
	void SetAmount( int value );
	void SetPrice( int value );
	bool IsSuccess() const { return success; }
	void SetSuccess() { success = true; }
	void UnsetSuccess() { success = false; }
};

template<>
class List<Item<MarketData>>
{
private:
	Item<MarketData>* first;
	Item<MarketData>* last;
public:
	List<Item<MarketData>>() { first = nullptr; last = nullptr; }
	~List<Item<MarketData>>() { Clear(); }
	Item<MarketData>* GetFirst() const { return first; }
	Item<MarketData>* GetLast() const { return last; }
	bool IsEmpty() const { if ( !first && !last ) return true; return false; }
	void Insert( MarketData data );
	void Delete( int num_value );
	void Clear();
	int GetSize() const;
	void Print() const;
private:
	List<Item<MarketData>>( const List<Item<MarketData>>& ) = delete;
	List<Item<MarketData>>( List<Item<MarketData>>&& ) = delete;
	void operator=( const List<Item<MarketData>>& ) = delete;
	void SetFirst( Item<MarketData>* first_value ) { first = first_value; }
	void SetLast( Item<MarketData>* last_value ) { last = last_value; }
};

class Banker
{
private:
	bool game_state_prepared;
	bool game_started;
	int turn_number;
	int alive_players;
	int ready_players;
	int lobby_players;
	int cur_market_lvl;
	RegisteredPlayers registered_players;
	MarketState cur_market_state;
	List<Item<MarketData>> sources_requests;
	List<Item<MarketData>> products_requests;
public:
	Banker();
	bool IsGameStatePrepared() const { return game_state_prepared; }
	void SetGameStatePrepared() { game_state_prepared = true; }
	void UnsetPlayersPrepared() { game_state_prepared = false; }
	bool IsGameStarted() const { return game_started; }
	void SetGameStarted() { game_started = true; }
	void UnsetGameStarted() { game_started = false; }

	int GetTurnNumber() const { return turn_number; }
	int GetAlivePlayers() const { return alive_players; }
	int GetReadyPlayers() const { return ready_players; }
	int GetLobbyPlayers() const { return lobby_players; }
	int GetCurrentMarketLvl() const { return cur_market_lvl; }
	const RegisteredPlayers& GetPlayers() const { return registered_players; }
	MarketState& GetCurrentMarketState() { return cur_market_state; }
	List<Item<MarketData>>& GetSourcesRequests() { return sources_requests; }
	List<Item<MarketData>>& GetProductsRequests() { return products_requests; }

	void SetTurnNumber( int );
	void SetAlivePlayers( int );
	void SetReadyPlayers( int );
	void SetLobbyPlayers( int );
	void SetCurrentMarketLvl( int );

	void CleanPlayer( int );
private:
	Banker( const Banker& ) = delete;
	Banker( Banker&& ) = delete;
	void operator=( const Banker& ) = delete;
};

#endif
