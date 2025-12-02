#ifndef BANKER_HPP
#define BANKER_HPP


#include "Player.hpp"


enum
{
	BUFSIZE							=							1024,
	ADDRESS_BUFFER					=							 100
};

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
	void SetSourcesAmount( int amount );
	void SetSourceMinPrice( int min_price );
	void SetProductsAmount( int amount );
	void SetProductMaxPrice( int max_price );
private:
	MarketState( const MarketState& ) {}
	void operator=( const MarketState& ) {}
};


class MarketRequestList
{
public:
	class MarketRequest
	{
	public:
		class MarketData
		{
			int player_num;
			int amount;
			int price;
			bool success;
		public:
			MarketData( int p_num = 0, int amnt = 0, int price_value = 0 );
			int GetPlayerNum() const { return player_num; }
			int GetAmount() const { return amount; }
			int GetPrice() const { return price; }
			void SetPlayerNum( int num_value );
			void SetAmount( int amount_value );
			void SetPrice( int price_value );
			bool IsSuccess() const { return success; }
			void SetSuccess() { success = true; }
			void UnsetSuccess() { success = false; }
		private:
			MarketData( const MarketData& ) {}
			void operator=( const MarketData& ) {}
		};
	private:
		MarketData data;
		MarketRequest* next;
		MarketRequest* prev;
	public:
		MarketRequest( int num_value, int amount_value, int price_value );
		void SetData( int num_value, int amount_value, int price_value );
		const MarketData& GetData() const { return data; }
		MarketRequest* GetNext() const { return next; }
		MarketRequest* GetPrev() const { return prev; }
		void SetNext( MarketRequest* next_value ) { next = next_value; }
		void SetPrev( MarketRequest* prev_value ) { prev = prev_value; }
	private:
		MarketRequest( const MarketRequest& ) {}
		void operator=( const MarketRequest& ) {}
	};
private:
	MarketRequest* first;
	MarketRequest* last;
public:
	MarketRequestList() { first = nullptr; last = nullptr; }
	~MarketRequestList() { Clear(); }
	MarketRequest* GetFirst() const { return first; }
	MarketRequest* GetLast() const { return last; }
	bool IsEmpty() const { if ( !first && !last ) return true; return false; }
	int Insert( int num_value, int amount_value, int price_value );
	int Delete( int num_value );
	int Clear();
	int GetSize() const;
	void Print() const;
private:
	MarketRequestList( const MarketRequestList& ) {}
	void operator=( const MarketRequestList& ) {}
	void SetFirst( MarketRequest* first_value ) { first = first_value; }
	void SetLast( MarketRequest* last_value ) { last = last_value; }
};


class Banker
{
private:
	bool players_prepared;
	bool game_started;
	int turn_number;
	int alive_players;
	int ready_players;
	int lobby_players;
	int cur_market_lvl;
	Player* pl_array[MAX_PLAYERS];
	MarketState cur_market_state;
	MarketRequestList sources_requests;
	MarketRequestList products_requests;
public:
	Banker();
	bool IsPlayersPrepared() const { return players_prepared; }
	void SetPlayersPrepared() { players_prepared = true; }
	void UnsetPlayersPrepared() { players_prepared = false; }
	bool IsGameStarted() const { return game_started; }
	void SetGameStarted() { game_started = true; }
	void UnsetGameStarted() { game_started = false; }

	int GetTurnNumber() const { return turn_number; }
	int GetAlivePlayers() const { return alive_players; }
	int GetReadyPlayers() const { return ready_players; }
	int GetLobbyPlayers() const { return lobby_players; }
	int GetCurrentMarketLvl() const { return cur_market_lvl; }
	Player* const* GetPlayers() const { return pl_array; }
	Player* GetPlayer( int idx ) const { return ((idx < 0) || (idx >= MAX_PLAYERS)) ? nullptr : pl_array[idx]; }
	Player* GetPlayerByFd( int fd ) const;
	Player* GetPlayerByNum( int player_number ) const;
	int GetIdxByNum( int player_number ) const;
	int GetNumByIdx( int idx ) const;
	MarketState& GetCurrentMarketState() { return cur_market_state; }
	MarketRequestList& GetSourcesRequests() { return sources_requests; }
	MarketRequestList& GetProductsRequests() { return products_requests; }

	void SetTurnNumber( int number );
	void SetAlivePlayers( int players_value );
	void SetReadyPlayers( int players_value );
	void SetLobbyPlayers( int players_value );
	void SetCurrentMarketLvl( int lvl_value );
	void SetPlayer( int idx, Player* p );
	int CleanPlayer( int player_number );

	int check_producing_on_turn();
	int pay_charges();
	int report_on_turn();
	int change_market_state();
	int check_players_reports( MarketRequestList& );
	int sort_requests_by_price( const MarketRequestList&, MarketRequestList& , int auction_type);
	int start_auction( const MarketRequestList&, int auction_type );
	int check_building_factories();
private:
	void operator=( const Banker& ) {}
	Banker( const Banker& ) {}
	int show_auction_info( const char* auction_type_msg, const MarketRequestList::MarketRequest* );
};

#endif
