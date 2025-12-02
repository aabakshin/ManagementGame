#ifndef BANKER_CPP
#define BANKER_CPP

#include "Banker.hpp"
#include "MGLib.h"
#include "Player.hpp"
#include <cstdlib>
#include <cstring>
#include <cstdio>

// Описаны в модуле MGLib
extern const char* info_game_messages[];
extern const char* error_game_messages[];

// Описана в модуле Server
extern int send_message( const int fd, const char** message_tokens, const int tokens_amount, const char* address );


static int send_successchargespay_message( int total_charges, int cs, const char* address );
static int send_playerbankrot_message( int total_charges, int cs, const char* address );
static int send_payfactorysuccess_message( int cs, const char* address );
static int send_factorybuilt_message( int cs, const char* address );
static int send_produced_message( int produced, int cs, const char* address );


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


static int send_successchargespay_message( int total_charges, int cs, const char* address )
{
	char charges[20];
	itoa(total_charges, charges, 19);


	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[SUCCESS_CHARGES_PAY]),
				charges,
				nullptr
	};
	send_message(cs, mes_tokens, 2, address);

	return 1;
}

static int send_playerbankrot_message( int total_charges, int cs, const char* address )
{
	char charges[20];
	itoa(total_charges, charges, 19);


	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[PLAYER_BANKROT]),
				charges,
				nullptr
	};
	send_message(cs, mes_tokens, 2, address);

	return 1;
}

static int send_payfactorysuccess_message( int cs, const char* address )
{
	const char* success_pay_mes[] =
	{
			info_game_messages[PAY_FACTORY_SUCCESS],
			nullptr
	};
	send_message(cs, success_pay_mes, 1, address);

	return 1;
}

static int send_factorybuilt_message( int cs, const char* address )
{
	const char* factory_built_mes[] =
	{
			info_game_messages[FACTORY_BUILT],
			nullptr
	};
	send_message(cs, factory_built_mes, 1, address);

	return 1;
}

static int send_produced_message( int produced, int cs, const char* address )
{
	char am_prd[10];
	itoa(produced, am_prd, 9);


	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[PRODUCED]),
				am_prd,
				nullptr
	};
	send_message(cs, mes_tokens, 2, address);

	return 1;
}


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

	for ( int i = 0; i < MAX_PLAYERS; ++i )
		SetPlayer(i, nullptr);
}

Player* Banker::GetPlayerByNum( int player_number ) const
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		Player* p = GetPlayer(i);
		if ( p != nullptr )
			if ( p->GetUID() == player_number )
				return pl_array[i];
	}

	return nullptr;
}

Player* Banker::GetPlayerByFd( int fd ) const
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		Player* p = GetPlayer(i);
		if ( p != nullptr )
			if ( p->GetFd() == fd )
				return pl_array[i];
	}

	return nullptr;
}

int Banker::GetIdxByNum( int player_number ) const
{
	if ( (player_number < 1) || (player_number > MAX_PLAYERS) )
		return -1;

	return player_number - 1;
}

int Banker::GetNumByIdx( int idx ) const
{
	if ( (idx < 0) || (idx >= MAX_PLAYERS) )
		return -1;

	return idx + 1;
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

void Banker::SetPlayer( int idx, Player* p )
{
	if ( ( idx < 0 ) || ( idx >= MAX_PLAYERS ) )
	{
		return;
		// throw InvalidIdxValueException();
	}

	pl_array[idx] = p;
}

int Banker::CleanPlayer( int player_number )
{
	if ( !IsGameStarted() )
	{
		--lobby_players;
	}
	else
	{
		--alive_players;
		if ( GetPlayerByNum(player_number)->IsTurn() )
			--ready_players;

		GetSourcesRequests().Delete( player_number );
		GetProductsRequests().Delete( player_number );
	}
	delete GetPlayerByNum(player_number);

	printf("[+] Player #%d has been cleared\n", player_number);

	return 1;
}

int Banker::check_producing_on_turn()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		Player* p = GetPlayer(i);
		if ( p != nullptr )
		{
			if ( !p->IsProd() )
			{
				while ( p->GetWorkFactories() > 0 )
				{
					p->SetProduced( p->GetProduced() + 1 );
					p->SetProducts( p->GetProducts() + 1 );
					p->SetWorkFactories( p->GetWorkFactories() - 1 );
					p->SetWaitFactories( p->GetWaitFactories() + 1 );
				}

				if ( p->GetProduced() > 0 )
					send_produced_message(p->GetProduced(), p->GetFd(), p->GetAddr());
			}
		}
	}

	return 1;
}

int Banker::pay_charges()
{
	int total_charges = 0;

	for ( int i = 0; i < ready_players; ++i )
	{
		Player* p = GetPlayer(i);
		if ( p == nullptr )
			return 0;

		total_charges = p->GetSources() * SOURCE_UNIT_CHARGE;
		total_charges += p->GetProducts() * PRODUCT_UNIT_CHARGE;
		total_charges += p->GetWaitFactories() * FACTORY_UNIT_CHARGE;
		total_charges += p->GetWorkFactories() * FACTORY_UNIT_CHARGE;

		int remains = p->GetMoney() - total_charges;
		if ( remains >= 0 )
		{
			p->SetMoney( remains );
			send_successchargespay_message(total_charges, p->GetFd(), p->GetAddr());
		}
		else
		{
			send_playerbankrot_message(total_charges, p->GetFd(), p->GetAddr());
			p->SetBankrot();
		}
	}

	return 1;
}

int Banker::show_auction_info( const char* auction_type_msg, const MarketRequestList::MarketRequest* node )
{
	printf("\n%s\n", auction_type_msg);

	for ( ; node != nullptr; node = node->GetNext() )
	{
		Player* p = GetPlayerByNum(node->GetData().GetPlayerNum());
		printf("Request of Player #%d:\n", p->GetUID());
		printf("\tPrice: %d\n\tAmount: %d\n\tIs proceed: %s\n\n",
				node->GetData().GetPrice(),
				(node->GetData().IsSuccess()) ? p->GetAuctionReport().GetSoldSources() : node->GetData().GetAmount(),
				node->GetData().IsSuccess() ? "yes" : "no");
	}

	return 1;
}

int Banker::report_on_turn()
{
	printf("\n\n\n<<<<<<<<<< Report on Month #%d >>>>>>>>>>\n", turn_number);
	printf("\n%s\n", "Players statistics:");
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		Player* p = GetPlayer(i);
		if ( p != nullptr )
			printf("\tPlayer #%d:   money: %dР   produced products: %d\n", p->GetUID(), p->GetMoney(), p->GetProduced());
	}

	show_auction_info("Sources auction", GetSourcesRequests().GetFirst());
	show_auction_info("Products auction", GetProductsRequests().GetFirst());

	printf("\n<<<<<<<<<< Report on Month #%d >>>>>>>>>>\n", turn_number);
	return 1;
}

int Banker::change_market_state()
{
	int r = 1 + (int)(12.0 * rand() / (RAND_MAX + 1.0));

	int sum = 0;
	int i;
	for ( i = 0; i < MARKET_LEVEL_NUMBER; i++ )
	{
		sum += states_market_chance[cur_market_lvl-1][i];
		if ( sum >= r )
			break;
	}

	if ( i < MARKET_LEVEL_NUMBER )
		cur_market_lvl = i+1;

	GetCurrentMarketState().SetSourcesAmount( amount_multiplier_table[cur_market_lvl-1][0] * alive_players );
	GetCurrentMarketState().SetSourceMinPrice( price_table[cur_market_lvl-1][0] );
	GetCurrentMarketState().SetProductsAmount( amount_multiplier_table[cur_market_lvl-1][1]* alive_players );
	GetCurrentMarketState().SetProductMaxPrice( price_table[cur_market_lvl-1][1] );

	return 1;
}

int Banker::check_players_reports( MarketRequestList& requests )
{
	// Если игрок не заявился на аукцион, добавить его пустую заявку
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		Player* p = GetPlayer(i);
		if ( p != nullptr )
		{
			// если список заявок на аукцион пуст, то не нужно его проводить
			MarketRequestList::MarketRequest* node = requests.GetFirst();
			if ( node == nullptr )
				return 0;

			for ( ; node != nullptr; node = node->GetNext() )
				if ( node->GetData().GetPlayerNum() == p->GetUID() )
					break;

			if ( node == nullptr )
				requests.Insert( p->GetUID(), 0, 0 );
		}
	}

	return 1;
}

int Banker::sort_requests_by_price( const MarketRequestList& requests, MarketRequestList& sorted_requests, int auction_type )
{
	MarketRequestList::MarketRequest* arr_reqs[ready_players];
	int prices[ready_players];
	bool reqs_checked[ready_players];


	MarketRequestList::MarketRequest* request = requests.GetFirst();
	for ( int i = 0; request != nullptr; request = request->GetNext(), ++i )
	{
		arr_reqs[i] = request;
		prices[i] = arr_reqs[i]->GetData().GetPrice();
		reqs_checked[i] = false;
	}

	heap_sort(prices, ready_players, ( auction_type == SOURCE_AUCTION ) ? 1 : 0 );

	for ( int i = 0, j = 0; i < ready_players; ++i )
	{
		if ( (arr_reqs[i]->GetData().GetPrice() == prices[j]) && !reqs_checked[i] )
		{
			sorted_requests.Insert( arr_reqs[i]->GetData().GetPlayerNum(), arr_reqs[i]->GetData().GetAmount(), arr_reqs[i]->GetData().GetPrice() );
			reqs_checked[i] = true;
			i = 0;
			++j;
			if ( j == ready_players )
				break;

			continue;
		}
	}

	return 1;
}

int Banker::start_auction( const MarketRequestList& requests, int auction_type )
{
	if ( !check_players_reports( const_cast<MarketRequestList&>(requests) ) )
		return 0;

	MarketRequestList sorted_requests;
	sort_requests_by_price( requests, sorted_requests, auction_type );

	int max_sources = GetCurrentMarketState().GetSourcesAmount();
	int max_products = GetCurrentMarketState().GetProductsAmount();

	for ( MarketRequestList::MarketRequest* node = sorted_requests.GetFirst(); node != nullptr; node = node->GetNext() )
	{
		if ( node->GetData().GetPrice() < 1 )
			continue;

		Player* cur_p = GetPlayerByNum( node->GetData().GetPlayerNum() );

		if ( node->GetData().GetAmount() <= ( ( auction_type == SOURCE_AUCTION ) ? max_sources : max_products ) )
		{
			if ( node->GetData().GetAmount() > 0 )
			{
				if ( auction_type == SOURCE_AUCTION )
				{
					cur_p->SetMoney( cur_p->GetMoney() - node->GetData().GetAmount() * node->GetData().GetPrice() );
					cur_p->SetSources( cur_p->GetSources() + node->GetData().GetAmount() );
					max_sources -= node->GetData().GetAmount();
				}
				else
				{
					cur_p->SetMoney( cur_p->GetMoney() + node->GetData().GetAmount() * node->GetData().GetPrice() );
					cur_p->SetProducts( cur_p->GetProducts() - node->GetData().GetAmount() );
					max_products -= node->GetData().GetAmount();
				}

				const_cast<MarketRequestList::MarketRequest::MarketData&>(node->GetData()).SetSuccess();

				if ( auction_type == SOURCE_AUCTION )
				{
					cur_p->GetAuctionReport().SetSoldSources(node->GetData().GetAmount());
					cur_p->GetAuctionReport().SetSoldPrice(node->GetData().GetPrice());
				}
				else
				{
					cur_p->GetAuctionReport().SetBoughtProducts(node->GetData().GetAmount());
					cur_p->GetAuctionReport().SetBoughtPrice(node->GetData().GetPrice());
				}
			}
		}
		else
		{
			int saved_max_sources = 0;
			int saved_max_products = 0;

			if ( ( ( auction_type == SOURCE_AUCTION ) ? max_sources : max_products) > 0 )
			{
				if ( auction_type == SOURCE_AUCTION )
				{
					cur_p->SetMoney( cur_p->GetMoney() - max_sources * node->GetData().GetPrice() );
					cur_p->SetSources( cur_p->GetSources() + max_sources );
				}
				else
				{
					cur_p->SetMoney( cur_p->GetMoney() + max_products * node->GetData().GetPrice() );
					cur_p->SetProducts( cur_p->GetProducts() - max_products );
				}

				const_cast<MarketRequestList::MarketRequest::MarketData&>(node->GetData()).SetSuccess();

				if ( auction_type == SOURCE_AUCTION )
				{
					saved_max_sources = max_sources;
					max_sources = 0;
					cur_p->GetAuctionReport().SetSoldSources(saved_max_sources);
					cur_p->GetAuctionReport().SetSoldPrice(node->GetData().GetPrice());
				}
				else
				{
					saved_max_products = max_products;
					max_products = 0;
					cur_p->GetAuctionReport().SetBoughtProducts(saved_max_products);
					cur_p->GetAuctionReport().SetBoughtPrice(node->GetData().GetPrice());
				}
			}
		}
	}

	return 1;
}

int Banker::check_building_factories()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		Player* p = GetPlayer(i);
		if ( p != nullptr )
		{
			for ( Player::BuildsList::BuildsItem* node = p->GetBuildsFactories().GetFirst(); node != nullptr; )
			{
				if ( node->GetData().GetTurnsLeft() == 1 )
				{
					int total_charges = NEW_FACTORY_UNIT_COST / 2;
					p->SetMoney( p->GetMoney() - total_charges );
					if ( p->GetMoney() >= 0 )
					{
						send_payfactorysuccess_message(p->GetFd(), p->GetAddr());
					}
					else
					{
						send_playerbankrot_message(total_charges, p->GetFd(), p->GetAddr());
						p->SetBankrot();
						break;
					}
				}
				else if ( node->GetData().GetTurnsLeft() == 0 )
				{
					p->GetBuildsFactories().Delete( node->GetData().GetBuildNumber() );
					p->SetBuiltFactories( p->GetBuiltFactories() - 1 );
					p->SetWaitFactories( p->GetWaitFactories() + 1 );

					send_factorybuilt_message(p->GetFd(), p->GetAddr());

					node = p->GetBuildsFactories().GetFirst();
					continue;
				}

				const_cast<Player::BuildsList::BuildsItem::BuildsData&>(node->GetData()).SetTurnsLeft( node->GetData().GetTurnsLeft() - 1 );
				node = node->GetNext();
			}
		}
	}

	return 1;
}

#endif
