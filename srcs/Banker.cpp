#ifndef BANKER_CPP
#define BANKER_CPP

#include "Banker.hpp"
#include <cstdlib>
#include <cstring>
#include <cstdio>


/* Таблица множителей для формулы вычисления нового состояния рынка в соответствии с текущим уровнем */
const double amount_multiplier_table[MARKET_LEVEL_NUMBER][2] = {
					{ 1.0, 3.0 },
					{ 1.5, 2.5 },
					{ 2.0, 2.0 },
					{ 2.5, 2.5 },
					{ 3.0, 1.0 }
};

/* Таблица цен для формулы вычисления нового состояния рынка в соответствии с текущим уровнем */
const int price_table[MARKET_LEVEL_NUMBER][2] = {
					{ 800, 6500 },
					{ 650, 6000 },
					{ 500, 5500 },
					{ 400, 5000 },
					{ 300, 4500 }
};

/* Таблица вероятностных переходов для формулы вычисления шанса перехода на другой уровень рынка */
const int states_market_chance[MARKET_LEVEL_NUMBER][MARKET_LEVEL_NUMBER] = {
				{ 4, 4, 2, 1, 1 },
				{ 3, 4, 3, 1, 1 },
				{ 1, 3, 4, 3, 1 },
				{ 1, 1, 3, 4, 3 },
				{ 1, 1, 2, 4, 4 }
};


MarketState::MarketState()
{
	SetSourcesAmount(0);
	SetSourceMinPrice(0);
	SetProductsAmount(0);
	SetProductMaxPrice(0);
}

void MarketState::SetSourcesAmount( int amount )
{
	if ( amount < 0 )
	{
		return;
		// throw InvalidAmountException();
	}

	sources_amount = amount;
}

void MarketState::SetSourceMinPrice( int min_price )
{
	if ( min_price < 0 )
	{
		return;
		// throw InvalidPriceException();
	}

	source_min_price = min_price;
}

void MarketState::SetProductsAmount( int amount )
{
	if ( amount < 0 )
	{
		return;
		// throw InvalidAmountException();
	}

	products_amount = amount;
}

void MarketState::SetProductMaxPrice( int max_price )
{
	if ( max_price < 0 )
	{
		return;
		// throw InvalidPriceException();
	}

	product_max_price = max_price;
}


MarketRequestList::MarketRequest::MarketData::MarketData( int p_num, int amnt, int price_value )
{
	SetPlayerNum( p_num );
	SetAmount( amnt );
	SetPrice( price_value );
	UnsetSuccess();
}

void MarketRequestList::MarketRequest::MarketData::SetPlayerNum( int num_value )
{
	if ( ( num_value < 1 ) || ( num_value > MAX_PLAYERS ) )
	{
		return;
		// throw InvalidValueException();
	}

	player_num = num_value;
}

void MarketRequestList::MarketRequest::MarketData::SetAmount( int amount_value )
{
	if ( amount_value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	amount = amount_value;
}

void MarketRequestList::MarketRequest::MarketData::SetPrice( int price_value )
{
	if ( price_value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	price = price_value;
}


MarketRequestList::MarketRequest::MarketRequest( int num_value, int amount_value, int price_value )
{
	SetData( num_value, amount_value, price_value );
	SetNext( nullptr );
	SetPrev( nullptr );
}

void MarketRequestList::MarketRequest::SetData( int num_value, int amount_value, int price_value )
{
	const_cast<MarketData&>(GetData()).SetPlayerNum(num_value);
	const_cast<MarketData&>(GetData()).SetAmount(amount_value);
	const_cast<MarketData&>(GetData()).SetPrice(price_value);
}


int MarketRequestList::Insert( int num_value, int amount_value, int price_value )
{
	if ( ( num_value < 1 ) || ( num_value > MAX_PLAYERS ) || ( amount_value < 0 ) || ( price_value < 0 ) )
		return 0;

	MarketRequest* prev_node = nullptr;
	MarketRequest* cur_node = GetFirst();

	while ( cur_node != nullptr )
	{
		if ( cur_node->GetData().GetPlayerNum() == num_value )
			return 0;

		prev_node = cur_node;
		cur_node = cur_node->GetNext();
	}

	MarketRequest* new_node = new MarketRequest( num_value, amount_value, price_value );
	new_node->SetNext( nullptr );

	new_node->SetPrev( prev_node );
	if ( !IsEmpty() )
		prev_node->SetNext( new_node );
	else
		SetFirst( new_node );

	SetLast( new_node );

	return 1;
}

int MarketRequestList::Delete( int player_num )
{
	if ( IsEmpty() )
		return 0;

	MarketRequest* cur_node = GetFirst();
	while ( ( cur_node != nullptr ) && ( cur_node->GetData().GetPlayerNum() != player_num ) )
	{
		cur_node = cur_node->GetNext();
	}

	if ( cur_node == nullptr )
		return 0;

	if ( cur_node->GetPrev() == nullptr )
	{
		if ( cur_node->GetNext() == nullptr )
		{
			delete cur_node;
			SetFirst( nullptr );
			SetLast( nullptr );

			return 1;
		}

		cur_node->GetNext()->SetPrev( nullptr );
		SetFirst( cur_node->GetNext() );
		cur_node->SetNext( nullptr );
		delete cur_node;

		return 1;
	}

	if ( cur_node->GetNext() == nullptr )
	{
		cur_node->GetPrev()->SetNext( nullptr );
		SetLast( cur_node->GetPrev() );
		cur_node->SetPrev( nullptr );
		delete cur_node;

		return 1;
	}

	cur_node->GetNext()->SetPrev( cur_node->GetPrev() );
	cur_node->GetPrev()->SetNext( cur_node->GetNext() );
	cur_node->SetNext( nullptr );
	cur_node->SetPrev( nullptr );
	delete cur_node;

	return 1;
}

int MarketRequestList::Clear()
{
	if ( IsEmpty() )
		return 0;

	int list_size = GetSize();
	for ( int i = 1; i <= list_size; ++i )
		Delete(GetFirst()->GetData().GetPlayerNum());

	return 1;
}

int MarketRequestList::GetSize() const
{
	if ( IsEmpty() )
		return 0;

	int size = 0;
	for ( MarketRequest* node = GetFirst(); node != nullptr; ++size, node = node->GetNext() )
		{}

	return size;
}

void MarketRequestList::Print() const
{
	if ( IsEmpty() )
	{
		printf("%s", "\"\"\n");
		return;
	}

	for ( MarketRequest* node = GetFirst(); node != nullptr; node = node->GetNext() )
	{
		printf("( %d, %d, %d ),\n", node->GetData().GetPlayerNum(), node->GetData().GetAmount(), node->GetData().GetPrice());
	}
}


Banker::Banker()
{
	UnsetPlayersPrepared();
	UnsetGameStarted();
	SetLobbyPlayers(0);
	SetAlivePlayers(0);
	SetReadyPlayers(0);
	SetTurnNumber(0);
	SetCurrentMarketLvl(0);
}

void Banker::SetTurnNumber( int number )
{
	if ( number < 0 )
	{
		return;
		//throw InvalidTurnValueException();
	}

	turn_number = number;
}

void Banker::SetAlivePlayers( int players_value )
{
	if ( ( players_value < 0 ) || ( players_value > MAX_PLAYERS ) )
	{
		return;
		// throw InvalidPlayersValueException();
	}

	alive_players = players_value;
}

void Banker::SetReadyPlayers( int players_value )
{
	if ( ( players_value < 0 ) || ( players_value > MAX_PLAYERS ) )
	{
		return;
		// throw InvalidPlayersValueException();
	}

	ready_players = players_value;
}

void Banker::SetLobbyPlayers( int players_value )
{
	if ( ( players_value < 0 ) || ( players_value > MAX_PLAYERS ) )
	{
		return;
		// throw InvalidPlayersValueException();
	}

	lobby_players = players_value;
}

void Banker::SetCurrentMarketLvl( int lvl_value )
{
	if ( lvl_value < 0 )
	{
		return;
		// throw InvalidMarketLvlException();
	}

	cur_market_lvl = lvl_value;
}

void Banker::CleanPlayer( int player_id )
{
	if ( !IsGameStarted() )
	{
		--lobby_players;
	}
	else
	{
		--alive_players;
		if ( GetPlayers().GetPlayerByNum( player_id )->IsTurn() )
			--ready_players;

		GetSourcesRequests().Delete( player_id );
		GetProductsRequests().Delete( player_id );
	}

	const_cast<Player*>(GetPlayers().GetPlayerByNum(player_id))->SetFree();
	printf( "[+] Player's #%d record now is free\n", player_id );
}

#endif
