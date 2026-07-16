#ifndef PLAYER_CPP
#define PLAYER_CPP


#include "Player.hpp"

#ifdef DEBUG_MODE
	#include <iostream>
#endif

#include <cstring>
#include <stdexcept>


BuildsData::BuildsData( int num_value, int turns_left_value, int max_p )
{
	SetMaxPlayers( max_p );
	SetBuildNumber( num_value );
	SetTurnsLeft( turns_left_value );
}

BuildsData::BuildsData( const BuildsData& data )
{
	SetMaxPlayers( data.GetMaxPlayers() );
	SetBuildNumber( data.GetBuildNumber() );
	SetTurnsLeft( data.GetTurnsLeft() );
}

BuildsData::BuildsData( BuildsData&& data )
{
	SetMaxPlayers( data.GetMaxPlayers() );
	SetBuildNumber( data.GetBuildNumber() );
	SetTurnsLeft( data.GetTurnsLeft() );

	data.SetBuildNumber( 0 );
	data.SetTurnsLeft( 0 );
	data.SetMaxPlayers( 0 );
}

void BuildsData::operator=( const BuildsData& data )
{
	SetMaxPlayers( data.GetMaxPlayers() );
	SetBuildNumber( data.GetBuildNumber() );
	SetTurnsLeft( data.GetTurnsLeft() );
}

void BuildsData::Make( int num_value, int turns_left_value, int max_p )
{
	SetMaxPlayers( max_p );
	SetBuildNumber( num_value );
	SetTurnsLeft( turns_left_value );
}

void BuildsData::SetBuildNumber( int num_value )
{
	if ( ( num_value < 1 ) || ( num_value > GetMaxPlayers() ) )
		throw std::runtime_error("Invalid 'build number' error in \"BuildsData::SetBuildNumber\" function!");

	build_number = num_value;
}

void BuildsData::SetTurnsLeft( int turns_left_value )
{
	if ( turns_left_value < 0 )
		throw std::runtime_error("Invalid 'turns left' error in \"BuildsData::SetTurnsLeft\" function!");

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
#ifdef DEBUG_MODE
	if ( IsEmpty() )
	{
		std::cout << "[" << Utility::current_time_str() << "] " << "[DEBUG] " << "\"\"" << std::endl;
		return;
	}

	for ( Item<BuildsData>* node = GetFirst(); node != nullptr; node = node->GetNext() )
		std::cout << "[" << Utility::current_time_str() << "] " << "[DEBUG] " << "( " << node->GetData().GetBuildNumber() << ", " << node->GetData().GetTurnsLeft() << ")," << std::endl;
#endif
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
		throw std::runtime_error("Invalid 'sold sources' error in \"Player::AuctionReport::SetSoldSources\" function!");

	sold_sources = src_value;
}

void Player::AuctionReport::SetSoldPrice( int price_value )
{
	if ( price_value < 0 )
		throw std::runtime_error("Invalid 'sold price' error in \"Player::AuctionReport::SetSoldPrice\" function!");

	sold_price = price_value;
}

void Player::AuctionReport::SetBoughtProducts( int prod_value )
{
	if ( prod_value < 0 )
		throw std::runtime_error("Invalid 'bought products' error in \"Player::AuctionReport::SetBoughtProducts\" function!");

	bought_products = prod_value;
}

void Player::AuctionReport::SetBoughtPrice( int price_value )
{
	if ( price_value < 0 )
		throw std::runtime_error("Invalid 'bought price' error in \"Player::AuctionReport::SetBoughtPrice\" function!");

	bought_price = price_value;
}


void Player::Reset()
{
	SetFd( -1 );
	SetAddr( "0.0.0.0" );

	SetMoney( 0 );
	SetOldMoney( 0 );
	SetIncome( 0 );
	SetSources( 0 );
	SetProducts( 0 );
	SetWaitFactories( 0 );
	SetWorkFactories( 0 );
	SetBuiltFactories( 0 );
	SetProduced( 0 );
	SetFree();

	UnsetBot();
	UnsetIdentMsgRecv();
	UnsetTurn();
	UnsetBankrot();
	UnsetSentSourceRequest();
	UnsetSentProductsRequest();
}

Player::Player( int p_fd, const char* p_addr, int p_uid, std::shared_ptr<const Config::GameSettings> m_g_sets )
{
	m_game_settings = std::move( m_g_sets );

	Reset();

	SetUID( p_uid );
	SetFd( p_fd );
	SetAddr( p_addr );
}

void Player::ApplySettings( std::shared_ptr<const Config::GameSettings> m_g_sets )
{
	m_game_settings = std::move( m_g_sets );
}

void Player::SetFd( int p_fd )
{
	if ( p_fd < -1 )
		throw std::runtime_error("Invalid 'socket fd' error in \"Player::SetFd\" function!");

	fd = p_fd;
}

void Player::SetAddr( const char* p_addr )
{
	if ( p_addr == nullptr )
		throw std::runtime_error("Address pointer is nullptr in \"Player::SetAddr\" function!");

	strncpy(addr, p_addr, ADDRESS_SIZE-1);
}

void Player::SetUID( int p_uid )
{
	if ( p_uid < 0 )
		throw std::runtime_error("Invalid 'uid' error in \"Player::SetUID\" function!");

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
		throw std::runtime_error("Invalid 'sources' error in \"Player::SetSources\" function!");

	sources = value;
}

void Player::SetProducts( int value )
{
	if ( value < 0 )
		throw std::runtime_error("Invalid 'products' error in \"Player::SetProducts\" function!");

	products = value;
}

void Player::SetWaitFactories( int value )
{
	if ( value < 0 )
		throw std::runtime_error("Invalid 'wait factories' error in \"Player::SetWaitFactories\" function!");

	wait_factories = value;
}

void Player::SetWorkFactories( int value )
{
	if ( value < 0 )
		throw std::runtime_error("Invalid 'work factories' error in \"Player::SetWorkFactories\" function!");

	work_factories = value;
}

void Player::SetBuiltFactories( int value )
{
	if ( value < 0 )
		throw std::runtime_error("Invalid 'built factories' error in \"Player::SetBuiltFactories\" function!");

	built_factories = value;
}

void Player::SetProduced( int value )
{
	if ( value < 0 )
		throw std::runtime_error("Invalid 'produced' error in \"Player::SetProduced\" function!");

	produced_on_turn = value;
}

#endif
