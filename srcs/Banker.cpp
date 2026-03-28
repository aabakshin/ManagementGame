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

void MarketState::SetSourcesAmount( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidAmountException();
	}

	sources_amount = value;
}

void MarketState::SetSourceMinPrice( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidPriceException();
	}

	source_min_price = value;
}

void MarketState::SetProductsAmount( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidAmountException();
	}

	products_amount = value;
}

void MarketState::SetProductMaxPrice( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidPriceException();
	}

	product_max_price = value;
}

void MarketData::MakeData( int p_num, int amnt, int price_value )
{
	SetPlayerNum( p_num );
	SetAmount( amnt );
	SetPrice( price_value );
	UnsetSuccess();
}

MarketData::MarketData( int p_num, int amnt, int price_value )
{
	SetPlayerNum( p_num );
	SetAmount( amnt );
	SetPrice( price_value );
	UnsetSuccess();
}

MarketData::MarketData( const MarketData& data )
{
	SetPlayerNum( data.GetPlayerNum() );
	SetAmount( data.GetAmount() );
	SetPrice( data.GetPrice() );

	success = data.success;
}

MarketData::MarketData( MarketData&& data )
{
	SetPlayerNum( data.GetPlayerNum() );
	SetAmount( data.GetAmount() );
	SetPrice( data.GetPrice() );

	success = data.success;

	data.SetPlayerNum( 0 );
	data.SetAmount( 0 );
	data.SetPrice( 0 );
	data.UnsetSuccess();
}

void MarketData::operator=( const MarketData& data )
{
	SetPlayerNum( data.GetPlayerNum() );
	SetAmount( data.GetAmount() );
	SetPrice( data.GetPrice() );

	success = data.success;
}

void MarketData::SetPlayerNum( int value )
{
	if ( ( value < 1 ) || ( value > MAX_PLAYERS ) )
	{
		return;
		// throw InvalidValueException();
	}

	player_id = value;
}

void MarketData::SetAmount( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	amount = value;
}

void MarketData::SetPrice( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	price = value;
}


void List<Item<MarketData>>::Insert( MarketData data )
{
	Item<MarketData>* prev_node = nullptr;
	Item<MarketData>* cur_node = GetFirst();

	while ( cur_node != nullptr )
	{
		if ( cur_node->GetData().GetPlayerNum() == data.GetPlayerNum() )
			return;

		prev_node = cur_node;
		cur_node = cur_node->GetNext();
	}

	Item<MarketData>* new_node = new Item<MarketData>( data );
	new_node->SetNext( nullptr );

	new_node->SetPrev( prev_node );
	if ( !IsEmpty() )
		prev_node->SetNext( new_node );
	else
		SetFirst( new_node );

	SetLast( new_node );
}

void List<Item<MarketData>>::Delete( int player_id )
{
	if ( IsEmpty() )
		return;

	Item<MarketData>* cur_node = GetFirst();
	while ( ( cur_node != nullptr ) && ( cur_node->GetData().GetPlayerNum() != player_id ) )
	{
		cur_node = cur_node->GetNext();
	}

	if ( cur_node == nullptr )
		return;

	if ( cur_node->GetPrev() == nullptr )
	{
		if ( cur_node->GetNext() == nullptr )
		{
			delete cur_node;
			SetFirst( nullptr );
			SetLast( nullptr );
			return;
		}

		cur_node->GetNext()->SetPrev( nullptr );
		SetFirst( cur_node->GetNext() );
		cur_node->SetNext( nullptr );
		delete cur_node;
		return;
	}

	if ( cur_node->GetNext() == nullptr )
	{
		cur_node->GetPrev()->SetNext( nullptr );
		SetLast( cur_node->GetPrev() );
		cur_node->SetPrev( nullptr );
		delete cur_node;
		return;
	}

	cur_node->GetNext()->SetPrev( cur_node->GetPrev() );
	cur_node->GetPrev()->SetNext( cur_node->GetNext() );
	cur_node->SetNext( nullptr );
	cur_node->SetPrev( nullptr );
	delete cur_node;
}

void List<Item<MarketData>>::Clear()
{
	if ( IsEmpty() )
		return;

	int list_size = GetSize();
	for ( int i = 1; i <= list_size; ++i )
		Delete(GetFirst()->GetData().GetPlayerNum());
}

int List<Item<MarketData>>::GetSize() const
{
	if ( IsEmpty() )
		return 0;

	int size = 0;
	for ( Item<MarketData>* node = GetFirst(); node != nullptr; ++size, node = node->GetNext() )
		{}

	return size;
}

void List<Item<MarketData>>::Print() const
{
	if ( IsEmpty() )
	{
		printf("%s", "\"\"\n");
		return;
	}

	for ( Item<MarketData>* node = GetFirst(); node != nullptr; node = node->GetNext() )
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

void Banker::SetTurnNumber( int value )
{
	if ( value < 0 )
	{
		return;
		//throw InvalidTurnValueException();
	}

	turn_number = value;
}

void Banker::SetAlivePlayers( int value )
{
	if ( ( value < 0 ) || ( value > MAX_PLAYERS ) )
	{
		return;
		// throw InvalidPlayersValueException();
	}

	alive_players = value;
}

void Banker::SetReadyPlayers( int value )
{
	if ( ( value < 0 ) || ( value > MAX_PLAYERS ) )
	{
		return;
		// throw InvalidPlayersValueException();
	}

	ready_players = value;
}

void Banker::SetLobbyPlayers( int value )
{
	if ( ( value < 0 ) || ( value > MAX_PLAYERS ) )
	{
		return;
		// throw InvalidPlayersValueException();
	}

	lobby_players = value;
}

void Banker::SetCurrentMarketLvl( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidMarketLvlException();
	}

	cur_market_lvl = value;
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
		if ( GetPlayers().GetPlayerByUID( player_id )->IsTurn() )
			--ready_players;

		GetSourcesRequests().Delete( player_id );
		GetProductsRequests().Delete( player_id );
	}

	const_cast<Player*>(GetPlayers().GetPlayerByUID(player_id))->SetFree();
	printf( "[+] Player's #%d record now is free\n", player_id );
}

#endif
