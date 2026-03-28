#ifndef PLAYER_CPP
#define PLAYER_CPP


#include "Player.hpp"
#include <cstdio>
#include <cstring>


BuildsData::BuildsData( int num_value, int turns_left_value )
{
	SetBuildNumber( num_value );
	SetTurnsLeft( turns_left_value );
}

BuildsData::BuildsData( const BuildsData& data )
{
	SetBuildNumber( data.GetBuildNumber() );
	SetTurnsLeft( data.GetTurnsLeft() );
}

BuildsData::BuildsData( BuildsData&& data )
{
	SetBuildNumber( data.GetBuildNumber() );
	SetTurnsLeft( data.GetTurnsLeft() );

	data.SetBuildNumber( 0 );
	data.SetTurnsLeft( 0 );
}

void BuildsData::operator=( const BuildsData& data )
{
	SetBuildNumber( data.GetBuildNumber() );
	SetTurnsLeft( data.GetTurnsLeft() );
}

void BuildsData::MakeData( int num_value, int turns_left_value )
{
	SetBuildNumber( num_value );
	SetTurnsLeft( turns_left_value );
}

void BuildsData::SetBuildNumber( int num_value )
{
	if ( ( num_value < 1 ) || ( num_value > MAX_PLAYERS ) )
	{
		return;
		// throw InvalidValueException();
	}

	build_number = num_value;
}

void BuildsData::SetTurnsLeft( int turns_left_value )
{
	if ( turns_left_value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	turns_left = turns_left_value;
}


void List<Item<BuildsData>>::Insert( BuildsData data )
{
	Item<BuildsData>* prev_node = nullptr;
	Item<BuildsData>* cur_node = GetFirst();

	while ( cur_node != nullptr )
	{
		if ( cur_node->GetData().GetBuildNumber() == data.GetBuildNumber() )
			return;

		prev_node = cur_node;
		cur_node = cur_node->GetNext();
	}

	Item<BuildsData>* new_node = new Item<BuildsData>( data );
	new_node->SetNext( nullptr );

	new_node->SetPrev( prev_node );
	if ( !IsEmpty() )
		prev_node->SetNext( new_node );
	else
		SetFirst( new_node );

	SetLast( new_node );
}

void List<Item<BuildsData>>::Delete( int build_num )
{
	if ( IsEmpty() )
		return;

	Item<BuildsData>* cur_node = GetFirst();
	while ( ( cur_node != nullptr ) && ( cur_node->GetData().GetBuildNumber() != build_num ) )
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
};

void List<Item<BuildsData>>::Clear()
{
	if ( IsEmpty() )
		return;

	int list_size = GetSize();
	for ( int i = 1; i <= list_size; ++i )
		Delete( GetFirst()->GetData().GetBuildNumber() );
}

int List<Item<BuildsData>>::GetSize() const
{
	if ( IsEmpty() )
		return 0;

	int size = 0;
	for ( Item<BuildsData>* node = GetFirst(); node != nullptr; ++size, node = node->GetNext() )
		{}

	return size;
}

void List<Item<BuildsData>>::Print() const
{
	if ( IsEmpty() )
	{
		printf("%s", "\"\"\n");
		return;
	}

	for ( Item<BuildsData>* node = GetFirst(); node != nullptr; node = node->GetNext() )
	{
		printf("( %d, %d ), \n", node->GetData().GetBuildNumber(), node->GetData().GetTurnsLeft());
	}
}

int List<Item<BuildsData>>::GetMaxNum() const
{
	if ( IsEmpty() )
		return 0;

	int max_number = 0;

	for ( Item<BuildsData>* node = GetFirst(); node != nullptr; node = node->GetNext() )
		if ( node->GetData().GetBuildNumber() > max_number )
			max_number = node->GetData().GetBuildNumber();

	return max_number;
}


Player::AuctionReport::AuctionReport()
{
	SetSoldSources( 0 );
	SetSoldPrice( 0 );
	SetBoughtProducts( 0 );
	SetBoughtPrice( 0 );
}

void Player::AuctionReport::SetSoldSources( int src_value )
{
	if ( src_value < 0 )
	{
		return;
		// throw InvalidSourcesException();
	}

	sold_sources = src_value;
}

void Player::AuctionReport::SetSoldPrice( int price_value )
{
	if ( price_value < 0 )
	{
		return;
		// throw InvalidPriceException();
	}

	sold_price = price_value;
}

void Player::AuctionReport::SetBoughtProducts( int prod_value )
{
	if ( prod_value < 0 )
	{
		return;
		// throw InvalidProductsException();
	}

	bought_products = prod_value;
}

void Player::AuctionReport::SetBoughtPrice( int price_value )
{
	if ( price_value < 0 )
	{
		return;
		// throw InvalidPriceException();
	}

	bought_price = price_value;
}


Player::Player( int p_fd, const char* p_addr, int p_uid )
{
	SetFd( p_fd );
	SetAddr( p_addr );
	SetUID( p_uid );

	SetMoney(0);
	SetOldMoney(0);
	SetIncome(0);
	SetSources(0);
	SetProducts(0);
	SetWaitFactories(0);
	SetWorkFactories(0);
	SetBuiltFactories(0);
	SetProduced(0);
	SetFree();

	UnsetBot();
	UnsetIdentMsgRecv();
	UnsetTurn();
	//UnsetProd();
	UnsetBankrot();
	UnsetSentSourceRequest();
	UnsetSentProductsRequest();
}

void Player::SetFd( int p_fd )
{
	if ( p_fd < -1 )
	{
		return;
		// throw InvalidSocketException();
	}

	fd = p_fd;
}

void Player::SetAddr( const char* p_addr )
{
	strncpy(addr, p_addr, ADDRESS_SIZE-1);
}

void Player::SetUID( int p_uid )
{
	if ( p_uid < 0 )
	{
		return;
		// throw InvalidUIDException();
	}

	uid = p_uid;
}

void Player::SetNewPlayer( int cs, const char* address_buffer )
{
	SetFd( cs );
	SetAddr( address_buffer );
	UnsetFree();
}

void Player::SetMessageBuffer( const char* msg, int msg_len )
{
	int i;
	for ( i = 0; ( i < BUFSIZE-1 ) && ( i < msg_len ); ++i )
		message[i] = msg[i];
	message[i] = '\0';
}

void Player::SetMoney( int value )
{
	money = value;
}

void Player::SetOldMoney( int value )
{
	old_money = value;
}

void Player::SetIncome( int value )
{
	income = value;
}

void Player::SetSources( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidSourcesException();
	}

	sources = value;
}

void Player::SetProducts( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidProductsException();
	}

	products = value;
}

void Player::SetWaitFactories( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidFactoriesException();
	}

	wait_factories = value;
}

void Player::SetWorkFactories( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidFactoriesException();
	}

	work_factories = value;
}

void Player::SetBuiltFactories( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidFactoriesException();
	}

	built_factories = value;
}

void Player::SetProduced( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidProducedException();
	}

	produced_on_turn = value;
}

#endif
