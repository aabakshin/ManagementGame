#ifndef PLAYER_CPP
#define PLAYER_CPP

#include "Player.hpp"
#include <cstdio>
#include <cstring>


Player::BuildsList::BuildsItem::BuildsData::BuildsData( int num_value, int turns_left_value )
{
	SetBuildNumber( num_value );
	SetTurnsLeft( turns_left_value );
}

void Player::BuildsList::BuildsItem::BuildsData::SetBuildNumber( int num_value )
{
	if ( ( num_value < 1 ) || ( num_value > MAX_PLAYERS ) )
	{
		return;
		// throw InvalidValueException();
	}

	build_number = num_value;
}

void Player::BuildsList::BuildsItem::BuildsData::SetTurnsLeft( int turns_left_value )
{
	if ( turns_left_value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	turns_left = turns_left_value;
}


Player::BuildsList::BuildsItem::BuildsItem( int num_value, int turns_left_value )
{
	SetData( num_value, turns_left_value );
	SetNext( nullptr );
	SetPrev( nullptr );
}

void Player::BuildsList::BuildsItem::SetData( int num_value, int turns_left_value )
{
	const_cast<Player::BuildsList::BuildsItem::BuildsData&>(GetData()).SetBuildNumber( num_value );
	const_cast<Player::BuildsList::BuildsItem::BuildsData&>(GetData()).SetTurnsLeft( turns_left_value );
}


int Player::BuildsList::Insert( int num_value, int turns_left_value )
{
	if ( ( num_value < 1 ) || ( num_value > MAX_PLAYERS ) || ( turns_left_value < 0 ) )
		return 0;

	BuildsItem* prev_node = nullptr;
	BuildsItem* cur_node = GetFirst();

	while ( cur_node != nullptr )
	{
		if ( cur_node->GetData().GetBuildNumber() == num_value )
			return 0;

		prev_node = cur_node;
		cur_node = cur_node->GetNext();
	}

	BuildsItem* new_node = new BuildsItem( num_value, turns_left_value );
	new_node->SetNext( nullptr );

	new_node->SetPrev( prev_node );
	if ( !IsEmpty() )
		prev_node->SetNext( new_node );
	else
		SetFirst( new_node );

	SetLast( new_node );

	return 1;
}

int Player::BuildsList::Delete( int build_num )
{
	if ( IsEmpty() )
		return 0;

	BuildsItem* cur_node = GetFirst();
	while ( ( cur_node != nullptr ) && ( cur_node->GetData().GetBuildNumber() != build_num ) )
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
};

int Player::BuildsList::Clear()
{
	if ( IsEmpty() )
		return 0;

	int list_size = GetSize();
	for ( int i = 1; i <= list_size; ++i )
		Delete( GetFirst()->GetData().GetBuildNumber() );

	return 1;
}

int Player::BuildsList::GetSize() const
{
	if ( IsEmpty() )
		return 0;

	int size = 0;
	for ( BuildsItem* node = GetFirst(); node != nullptr; ++size, node = node->GetNext() )
		{}

	return size;
}

void Player::BuildsList::Print() const
{
	if ( IsEmpty() )
	{
		printf("%s", "\"\"\n");
		return;
	}

	for ( BuildsItem* node = GetFirst(); node != nullptr; node = node->GetNext() )
	{
		printf("( %d, %d ), \n", node->GetData().GetBuildNumber(), node->GetData().GetTurnsLeft());
	}
}

int Player::BuildsList::GetMaxNum() const
{
	if ( IsEmpty() )
		return 0;

	int max_number = 0;

	for ( Player::BuildsList::BuildsItem* node = GetFirst(); node != nullptr; node = node->GetNext() )
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
	if ( p_fd < 0 )
	{
		return;
		// throw InvalidSocketException();
	}

	fd = p_fd;
}

void Player::SetAddr( const char* p_addr )
{
	strcpy(addr, p_addr);
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

void Player::SetMessageBuffer( const char* msg, int msg_len )
{
	int i;
	for ( i = 0; ( i < BUFSIZE-1 ) && ( i < msg_len ); ++i )
		message[i] = msg[i];
	message[i] = '\0';
}

void Player::SetMoney( int money_value )
{
	money = money_value;
}

void Player::SetOldMoney( int money_value )
{
	old_money = money_value;
}

void Player::SetIncome( int income_value )
{
	income = income_value;
}

void Player::SetSources( int sources_value )
{
	if ( sources_value < 0 )
	{
		return;
		// throw InvalidSourcesException();
	}

	sources = sources_value;
}

void Player::SetProducts( int products_value )
{
	if ( products_value < 0 )
	{
		return;
		// throw InvalidProductsException();
	}

	products = products_value;
}

void Player::SetWaitFactories( int wait_factories_value )
{
	if ( wait_factories_value < 0 )
	{
		return;
		// throw InvalidFactoriesException();
	}

	wait_factories = wait_factories_value;
}

void Player::SetWorkFactories( int work_factories_value )
{
	if ( work_factories_value < 0 )
	{
		return;
		// throw InvalidFactoriesException();
	}

	work_factories = work_factories_value;
}

void Player::SetBuiltFactories( int built_factories_value )
{
	if ( built_factories_value < 0 )
	{
		return;
		// throw InvalidFactoriesException();
	}

	built_factories = built_factories_value;
}

void Player::SetProduced( int produced_value )
{
	if ( produced_value < 0 )
	{
		return;
		// throw InvalidProducedException();
	}

	produced_on_turn = produced_value;
}

#endif
