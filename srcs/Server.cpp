#ifndef SERVER_CORE_CPP
#define SERVER_CORE_CPP


#include "Server.hpp"
#include "BrokerMessages.hpp"
#include "MGLib.h"
#include "MessageTokens.hpp"
#include "SessionsPlanner.hpp"
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netdb.h>
#include <ctime>


enum
{
					LISTEN_QUEUE_LEN			=						  5
};


// Описаны в модуле Banker
extern const double amount_multiplier_table[MARKET_LEVEL_NUMBER][2];
extern const int price_table[MARKET_LEVEL_NUMBER][2];
extern const int states_market_chance[MARKET_LEVEL_NUMBER][MARKET_LEVEL_NUMBER];

// Описаны в модуле MGLib
extern const char* info_game_messages[];
extern const char* error_game_messages[];

/* Список сообщений для идентификации бот-клиента */
static const char* const bot_identity_messages[] = {
				"./bot_mg_debug4",
				"./bot_mg_debug4\n",
				"./bot_mg_release4",
				"./bot_mg_release4\n",
				nullptr
};


/* Ф-я-обработчик сигнала SIGALRM */
void alrm_handler( int sig_no )
{
	int save_errno = errno;
	signal(SIGALRM, alrm_handler);

	Server::SetSignalNum( sig_no );
	Server::SetAlrmFlag();
	alarm(0);

	errno = save_errno;
}

/* Ф-я-обработчик сигнала SIGINT */
void exit_handler( int sig_no )
{
	int save_errno = errno;
	signal(SIGINT, exit_handler);

	Server::SetSignalNum( sig_no );
	Server::SetExitFlag();

	errno = save_errno;
}


void Server::SetSignalNum( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	Server::sig_number = value;
}

void Server::SetSentMsgsCount( int msgs_value )
{
	if ( msgs_value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	sent_msgs_count = msgs_value;
}

void Server::SetRecvMsgsCount( int msgs_value )
{
	if ( msgs_value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	recv_msgs_count = msgs_value;
}

void Server::SetListenSocket( int socket_value )
{
	if ( socket_value < -1 )
	{
		return;
		// throw InvalidValueException();
	}

	ls = socket_value;
}

void Server::SetAddrBuffer( const char* addr, const char* port )
{
	printf("%s\n", "Configuring local address...");

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family			=		AF_INET;
	hints.ai_socktype		=		SOCK_STREAM;
	hints.ai_flags			=		AI_PASSIVE;

	if ( getaddrinfo(addr, port, &hints, &bind_address) != 0 )
	{
		fprintf(stderr, "An error has occured with \"getaddrinfo\" (errno code = %d)\n", errno);
		return;
		// throw InvalidGetAddrInfoException();
	}

	getnameinfo(
			bind_address->ai_addr,
			bind_address->ai_addrlen,
			address_buffer,
			sizeof(address_buffer),
			service_buffer,
			sizeof(service_buffer),
			NI_NUMERICHOST | NI_NUMERICSERV );

	ConcatAddrPort();
}

void Server::SetMaxFd( int max_value )
{
	if ( max_value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	max_fd = max_value;
}

void Server::ListenSocketInit()
{
	printf("%s\n", "Creating listening socket...");
	SetListenSocket( socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol) );
	if ( GetListenSocket() == -1 )
	{
		fprintf(stderr, "socket() failed. (errno code = %d)\n", errno);
		return;
		// throw InvalidSocketException();
	}

	printf("%s\n", "Setting socket options...");
	int opt_value = 1;
	setsockopt(GetListenSocket(), SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));

	printf("%s\n", "Binding socket to address...");
	if ( bind(GetListenSocket(), bind_address->ai_addr, bind_address->ai_addrlen) )
	{
		fprintf(stderr, "bind() failed. (errno code = %d)\n", errno);
		return;
		// throw BindSocketException();
	}

	printf("%s\n", "Enabling listen mode...");
	if ( listen(GetListenSocket(), LISTEN_QUEUE_LEN) < 0 )
	{
		fprintf(stderr, "listen() failed. (errno code = %d\n)", errno);
		return;
		// throw ListenSocketException();
	}
}

Server::Server( const char* addr, const char* port )
{
	session_planner.Make( SESSIONS_COUNT );

	EBCbroker.MakeBroker( session_planner );
	EGameMessages.MakeBroker( session_planner );
	EMultiActionsExec.MakeBroker( session_planner, sender, msg_tokens, EGameMessages );

	msg_tokens.MakeMessageTokens( MessageTokens::MESSAGE_TOKENS_COUNT );

	UnsetAlrmFlag();
	UnsetExitFlag();
	UnsetTimerFlag();
	SetSignalNum( 0 );
	SetSentMsgsCount( 0 );
	SetAddrBuffer( addr, port );
	ListenSocketInit();
	SetMaxFd( 0 );

	printf("Waiting connections to %s port...\n", port);
}

Server::~Server()
{
	freeaddrinfo(bind_address);
}

bool Server::IsCorrectIdentityMsg( const char* identity_msg )
{
	for ( int i = 0; bot_identity_messages[i] != nullptr; ++i )
		if ( strcmp(identity_msg, bot_identity_messages[i]) == 0 )
			return true;

	return false;
}

void Server::ConcatAddrPort()
{
	int addr_len = strlen(address_buffer);
	address_buffer[addr_len] = ':';

	int i = addr_len + 1;
	for ( int j = 0; ( j < SERVICE_SIZE-1 ) && service_buffer[j]; ++j, ++i )
		address_buffer[i] = service_buffer[j];
	address_buffer[i] = '\0';
}

void Server::ShowSentMessage() const
{
	printf("\n==================== (%d) ====================\n", GetSentMsgsCount());

	for ( int i = 0; ( i < BUFSIZE ) && ( i < sender.GetMessageLength() ); ++i )
	{
		printf("%3d ", sender.GetMessage()[i]);
		if ( ( (i+1) % 10 ) == 0 )
			putchar('\n');
	}
	putchar('\n');

	printf("\nmessage: <[ %s ]>\n"
			"Sent to [%s] %d\\%d bytes\n"
			"==================== (%d) ====================\n\n", sender.GetMessage(), sender.GetTargetAddress(), sender.GetSentBytes(), sender.GetMessageLength(), GetSentMsgsCount());
}

void Server::ShowReceivedMessage() const
{
	printf("\n==================== (%d) ====================\n", GetRecvMsgsCount());

	for ( int i = 0; ( i < BUFSIZE ) && ( i < receiver.GetMessageLength() ); ++i )
	{
		printf("%3d ", receiver.GetMessage()[i]);
		if ( ( (i+1) % 10 ) == 0 )
			putchar('\n');
	}
	putchar('\n');

	printf("\nmessage: <[ %s ]>\n"
			"Received from [%s] %d\\%d bytes\n"
			"==================== (%d) ====================\n\n", receiver.GetMessage(), receiver.GetTargetAddress(), receiver.GetRecvBytes(), receiver.GetMessageLength(), GetRecvMsgsCount());
}

void Server::CloseConnection( int session_id, int player_number )
{
	const Banker& banker = *session_planner.GetSessionById( session_id );

	const Player* p = banker.GetPlayers().GetPlayerByUID( player_number );
	if ( p != nullptr )
	{
		int p_fd = p->GetFd();

		char ip[100] = { 0 };
		strncpy( ip, p->GetAddr(), ADDRESS_SIZE );

		const_cast<Banker&>(banker).CleanPlayer(player_number);
		close(p_fd);
		FD_CLR(p_fd, &readfds);
		printf("[-] Lost connection from [%s]\n", ip);
		return;
	}

	//throw InvalidPlayerException();
}

void Server::Stop( int forcely )
{
	if ( forcely )
		printf("%s", "\n\n========== SERVER IS STOPPING WORK FORCELY ==========\n");

	for ( int i = SessionsPlanner::DEFAULT_NEXT_SESSION_ID; i <= session_planner.GetSessionsCount(); ++i )
	{
		const Banker& banker = *session_planner.GetSessionById( i );
		for ( int j = 0; j < MAX_PLAYERS; ++j )
			CloseConnection( i, banker.GetPlayers().GetUIDByIdx( j ) );
	}

	if ( forcely )
		printf("%s", "========== SERVER IS STOPPING WORK FORCELY ==========\n\n");

	exit(0);
}

void Server::ShowAuctionInfo( int session_id, const char* auction_type_msg, const Item<MarketData>* node )
{
	const Banker& banker = *session_planner.GetSessionById( session_id );

	printf("\n%s\n", auction_type_msg);

	for ( ; node != nullptr; node = node->GetNext() )
	{
		const Player* p = banker.GetPlayers().GetPlayerByUID(node->GetData().GetPlayerNum());
		printf("Request of Player #%d:\n", p->GetUID());
		printf("\tPrice: %d\n\tAmount: %d\n\tIs proceed: %s\n\n",
				node->GetData().GetPrice(),
				(node->GetData().IsSuccess()) ? p->GetAuctionReport().GetSoldSources() : node->GetData().GetAmount(),
				node->GetData().IsSuccess() ? "yes" : "no");
	}
}

void Server::ReportOnTurn( int session_id )
{
	const Banker& banker = *session_planner.GetSessionById( session_id );

	printf("\n\n\n<<<<<<<<<< Report on Month #%d >>>>>>>>>>\n", banker.GetTurnNumber());
	printf("\n%s\n", "Players statistics:");

	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_REPORT_ON_TURN_TOKEN );

	ShowAuctionInfo( session_id, "Sources auction", const_cast<Banker&>(banker).GetSourcesRequests().GetFirst());
	ShowAuctionInfo( session_id, "Products auction", const_cast<Banker&>(banker).GetProductsRequests().GetFirst());

	printf("\n<<<<<<<<<< Report on Month #%d >>>>>>>>>>\n", banker.GetTurnNumber());
}

void Server::ChangeMarketState( int session_id )
{
	const Banker& banker = *session_planner.GetSessionById( session_id );

	int r = 1 + (int)( 12.0 * rand() / (RAND_MAX + 1.0) );

	int sum = 0;
	int i;
	for ( i = 0; i < MARKET_LEVEL_NUMBER; i++ )
	{
		sum += states_market_chance[banker.GetCurrentMarketLvl()-1][i];
		if ( sum >= r )
			break;
	}

	if ( i < MARKET_LEVEL_NUMBER )
		const_cast<Banker&>(banker).SetCurrentMarketLvl( i+1 );

	const_cast<Banker&>(banker).GetCurrentMarketState().SetSourcesAmount( amount_multiplier_table[banker.GetCurrentMarketLvl()-1][0] * banker.GetAlivePlayers() );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetSourceMinPrice( price_table[banker.GetCurrentMarketLvl()-1][0] );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetProductsAmount( amount_multiplier_table[banker.GetCurrentMarketLvl()-1][1] * banker.GetAlivePlayers() );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetProductMaxPrice( price_table[banker.GetCurrentMarketLvl()-1][1] );
}

bool Server::CheckPlayersReports( int session_id, List<Item<MarketData>>& requests, int auction_type )
{
	// если список заявок на аукцион пуст, то не нужно его проводить
	if ( requests.IsEmpty() )
		return false;

	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	itoa( auction_type, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::AUCTION_TYPE_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::AUCTION_TYPE_PARAM_TOKEN+1 );

	// Если какой-либо игрок не заявился на аукцион, добавить его пустую заявку
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::ADD_EMPTY_AUCTION_REQUEST_TOKEN );

	return true;
}

void Server::SortRequestsByPrice( int session_id, const List<Item<MarketData>>& requests, List<Item<MarketData>>& sorted_requests, int auction_type )
{
	const int ready_players = (*session_planner.GetSessionById( session_id )).GetReadyPlayers();

	Item<MarketData>* arr_reqs[ready_players];
	int prices[ready_players];
	bool reqs_checked[ready_players];


	Item<MarketData>* request = requests.GetFirst();
	for ( int i = 0; request != nullptr; request = request->GetNext(), ++i )
	{
		arr_reqs[i] = request;
		prices[i] = arr_reqs[i]->GetData().GetPrice();
		reqs_checked[i] = false;
	}

	heap_sort(prices, ready_players, ( auction_type == SOURCE_AUCTION ) ? 1 : 0 );

	int j = 0;
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		if ( (arr_reqs[i]->GetData().GetPrice() == prices[j]) && !reqs_checked[i] )
		{
			MarketData data;
			data.MakeData( arr_reqs[i]->GetData().GetPlayerNum(), arr_reqs[i]->GetData().GetAmount(), arr_reqs[i]->GetData().GetPrice() );

			sorted_requests.Insert( data );
			reqs_checked[i] = true;
			i = 0;
			++j;
			if ( j == ready_players )
				break;

			continue;
		}
	}
}

void Server::StartAuction( int session_id, const List<Item<MarketData>>& requests, int auction_type )
{
	if ( !CheckPlayersReports( session_id, const_cast<List<Item<MarketData>>&>(requests), auction_type ) )
		return;

	List<Item<MarketData>> sorted_requests;
	SortRequestsByPrice( session_id, requests, sorted_requests, auction_type );

	const Banker& banker = *session_planner.GetSessionById( session_id );

	int max_sources = const_cast<Banker&>(banker).GetCurrentMarketState().GetSourcesAmount();
	int max_products = const_cast<Banker&>(banker).GetCurrentMarketState().GetProductsAmount();

	for ( Item<MarketData>* node = sorted_requests.GetFirst(); node != nullptr; node = node->GetNext() )
	{
		if ( node->GetData().GetPrice() < 1 )
			continue;

		const Player* cur_p = banker.GetPlayers().GetPlayerByUID( node->GetData().GetPlayerNum() );

		if ( cur_p->IsFree() )
			continue;

		if ( node->GetData().GetAmount() <= ( ( auction_type == SOURCE_AUCTION ) ? max_sources : max_products ) )
		{
			if ( node->GetData().GetAmount() > 0 )
			{
				if ( auction_type == SOURCE_AUCTION )
				{
					const_cast<Player*>(cur_p)->SetMoney( cur_p->GetMoney() - node->GetData().GetAmount() * node->GetData().GetPrice() );
					const_cast<Player*>(cur_p)->SetSources( cur_p->GetSources() + node->GetData().GetAmount() );
					max_sources -= node->GetData().GetAmount();
				}
				else
				{
					const_cast<Player*>(cur_p)->SetMoney( cur_p->GetMoney() + node->GetData().GetAmount() * node->GetData().GetPrice() );
					const_cast<Player*>(cur_p)->SetProducts( cur_p->GetProducts() - node->GetData().GetAmount() );
					max_products -= node->GetData().GetAmount();
				}

				const_cast<MarketData&>(node->GetData()).SetSuccess();

				if ( auction_type == SOURCE_AUCTION )
				{
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetSoldSources(node->GetData().GetAmount());
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetSoldPrice(node->GetData().GetPrice());
				}
				else
				{
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetBoughtProducts(node->GetData().GetAmount());
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetBoughtPrice(node->GetData().GetPrice());
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
					const_cast<Player*>(cur_p)->SetMoney( cur_p->GetMoney() - max_sources * node->GetData().GetPrice() );
					const_cast<Player*>(cur_p)->SetSources( cur_p->GetSources() + max_sources );
				}
				else
				{
					const_cast<Player*>(cur_p)->SetMoney( cur_p->GetMoney() + max_products * node->GetData().GetPrice() );
					const_cast<Player*>(cur_p)->SetProducts( cur_p->GetProducts() - max_products );
				}

				const_cast<MarketData&>(node->GetData()).SetSuccess();

				if ( auction_type == SOURCE_AUCTION )
				{
					saved_max_sources = max_sources;
					max_sources = 0;
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetSoldSources(saved_max_sources);
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetSoldPrice(node->GetData().GetPrice());
				}
				else
				{
					saved_max_products = max_products;
					max_products = 0;
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetBoughtProducts(saved_max_products);
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetBoughtPrice(node->GetData().GetPrice());
				}
			}
		}
	}
}

void Server::PrepareNewTurn( int session_id )
{
	const Banker& banker = *session_planner.GetSessionById( session_id );

	ChangeMarketState( session_id );
	const_cast<Banker&>(banker).SetTurnNumber( banker.GetTurnNumber() + 1 );
	const_cast<Banker&>(banker).SetReadyPlayers( 0 );

	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::PREPARE_NEW_TURN_TOKEN );
}

void Server::PrepareGameState( int session_id )
{
	const Banker& banker = *session_planner.GetSessionById( session_id );

	const_cast<Banker&>(banker).SetAlivePlayers( banker.GetLobbyPlayers() );
	const_cast<Banker&>(banker).SetLobbyPlayers( 0 );
	const_cast<Banker&>(banker).SetTurnNumber( 1 );
	const_cast<Banker&>(banker).SetCurrentMarketLvl( START_MARKET_LEVEL );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetSourcesAmount( amount_multiplier_table[START_MARKET_LEVEL-1][0] * banker.GetAlivePlayers() );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetSourceMinPrice( price_table[START_MARKET_LEVEL-1][0] );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetProductsAmount( amount_multiplier_table[START_MARKET_LEVEL-1][1] * banker.GetAlivePlayers() );
	const_cast<Banker&>(banker).GetCurrentMarketState().SetProductMaxPrice( price_table[START_MARKET_LEVEL-1][1] );

	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::PREPARE_PLAYERS_STATE_TOKEN );

	const_cast<Banker&>(banker).SetGameStatePrepared();
}

void Server::EndGameTurn( int session_id )
{
	const Banker& banker = *session_planner.GetSessionById( session_id );

	StartAuction( session_id, const_cast<Banker&>(banker).GetSourcesRequests(), SOURCE_AUCTION );
	StartAuction( session_id, const_cast<Banker&>(banker).GetProductsRequests(), PRODUCTION_AUCTION );

	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_AUCTIONS_RESULTS_TOKEN );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::CHECK_BUILDING_FACTORIES_TOKEN );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::PAY_CHARGES_TOKEN );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_PLAYERS_BANKROT_TOKEN );

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = banker.GetPlayers()[i];
		if ( !p->IsFree() )
			if ( p->IsBankrot() )
				if ( QuitPlayer( session_id, p->GetUID() ) )
					return;
	}

	//ReportOnTurn( session_id );
	PrepareNewTurn( session_id );
}

bool Server::QuitPlayer( int session_id, int player_number )
{
	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	itoa( player_number, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::LEFT_PLAYER_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::LEFT_PLAYER_ID_PARAM_TOKEN+1 );

	CloseConnection( session_id, player_number );

	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::QUIT_PLAYER_TOKEN );

	if ( session_planner.GetSessionById( session_id )->GetAlivePlayers() <= 1 )
	{
		SetExitFlag();
		return true;
	}

	return false;
}

void Server::ErrorEvent( int cs, const char* new_client_addr, int event_code )
{
	if ( event_code == INTERNAL_SERVER_ERROR )
	{
		sender.SendMessage( error_game_messages[INTERNAL_SERVER_ERROR], cs, new_client_addr );
	}
	else
	{
		sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( event_code ), cs, new_client_addr );
	}

	SetSentMsgsCount( GetSentMsgsCount() + 1 );
	ShowSentMessage();

	close(cs);
	printf("Lost connection from (%s)\n", new_client_addr);
}

void Server::AddNewClientToSession( int cs, const char* new_client_addr )
{
	int i;
	for ( i = session_planner.DEFAULT_NEXT_SESSION_ID; i <= session_planner.GetSessionsCount(); ++i )
	{
		const Banker& banker = *session_planner.GetSessionById( i );
		if ( banker.IsGameStarted() )
		{
			continue;
		}

		const Player* p = banker.GetFree();
		if ( p == nullptr )
		{
			ErrorEvent( cs, new_client_addr, GameMessages::SERVER_FULL_TOKEN );
		}
		else
		{
			try
			{
				const_cast<Player*>(p)->SetNewPlayer( cs, address_buffer );
			}
			catch ( ... )
			{
				ErrorEvent( cs, new_client_addr, INTERNAL_SERVER_ERROR );
				return;
			}

			const_cast<Banker&>(banker).SetLobbyPlayers( banker.GetLobbyPlayers() + 1 );

			itoa( banker.GetId(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
			const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

			const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_NEW_PLAYER_CONNECT_TOKEN );
		}
	}

	if ( i > session_planner.GetSessionsCount() )
	{
		ErrorEvent( cs, new_client_addr, GameMessages::GAME_ALREADY_STARTED_TOKEN );
	}
}

void Server::NewClientHandle()
{
	if ( FD_ISSET(ls, &readfds) )
	{
		char new_client_addr[Server::ADDRESS_SIZE];
		char new_client_serv[Server::SERVICE_SIZE];
		struct sockaddr_storage client_address;
		socklen_t client_address_len = sizeof(client_address);

		int cs = accept(ls, (struct sockaddr*) &client_address, &client_address_len);
		if ( cs == -1 )
		{
			fprintf(stderr, "accept() failed. (errno code = %d)\n", errno);
			return;
		}

		getnameinfo(
				(struct sockaddr*) &client_address,
				client_address_len,
				new_client_addr,
				sizeof(new_client_addr),
				new_client_serv,
				sizeof(new_client_serv),
				NI_NUMERICHOST | NI_NUMERICSERV);

		ConcatAddrPort();

		printf("New connection from (%s)\n", new_client_addr);

		AddNewClientToSession( cs, new_client_addr );
	}
}

void Server::ClientsInputHandle()
{
	for ( int i = session_planner.DEFAULT_NEXT_SESSION_ID; i <= session_planner.GetSessionsCount(); ++i )
	{
		const Banker& banker = *session_planner.GetSessionById( i );

		for ( int j = 0; j < MAX_PLAYERS; ++j )
		{
			const Player* p = banker.GetPlayers()[j];
			if ( !p->IsFree() )
			{
				int sender_p_fd = p->GetFd();
				const char* sender_p_addr = p->GetAddr();

				if ( FD_ISSET(sender_p_fd, &readfds) )
				{
					receiver.RecvMessage( sender_p_fd, sender_p_addr );
					SetRecvMsgsCount( GetRecvMsgsCount() + 1 );
					ShowReceivedMessage();

					if ( receiver.GetRecvBytes() > 0 )
					{
						const_cast<Player*>(p)->SetMessageBuffer( receiver.GetMessage(), receiver.GetMessageLength() );

						if ( !banker.IsGameStarted() )
						{
							if ( !p->IsIdentMsgRecv() )
							{
								if ( IsCorrectIdentityMsg( p->GetMessageBuffer() ) )
									const_cast<Player*>(p)->SetBot();
								else
									const_cast<Player*>(p)->UnsetBot();

								const_cast<Player*>(p)->SetIdentMsgRecv();
							}
							else
							{
								itoa( banker.GetId(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
								const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

								sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::GAME_NOT_STARTED_TOKEN ), sender_p_fd, sender_p_addr );
								SetSentMsgsCount( GetSentMsgsCount() + 1 );
								ShowSentMessage();
							}
						}
						else
						{
							// Обработка данных от игрока, когда игра началась
							cmds_exec.ProcessCommand( banker.GetId(), p->GetMessageBuffer(), p->GetUID(), EBCbroker.GetBroker() );
							sender.SendMessage( cmds_exec.GetCmdResultTokens(), cmds_exec.GetCmdResultTokensAmount(), sender_p_fd, sender_p_addr );
							SetSentMsgsCount( GetSentMsgsCount() + 1 );
							ShowSentMessage();

							const char* info_token = cmds_exec.GetCmdToken( 0 );
							if ( strcmp(info_token, info_game_messages[QUIT_COMMAND_SUCCESS]) == 0 )
							{
								QuitPlayer( banker.GetId(), p->GetUID() );
							}
							else if ( strcmp(info_token, error_game_messages[INTERNAL_SERVER_ERROR]) == 0 )
							{
								QuitPlayer( banker.GetId(), p->GetUID() );
							}
						}
					}
					else
					{
						QuitPlayer( banker.GetId(), p->GetUID() );
					}
				}
			}
		}
	}
}

void Server::GameEventsHandle()
{
	for ( int i = session_planner.DEFAULT_NEXT_SESSION_ID; i <= session_planner.GetSessionsCount(); ++i )
	{
		const Banker& banker = *session_planner.GetSessionById( i );

		itoa( banker.GetId(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
		const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::SESSION_ID_PARAM_TOKEN+1 );

		if ( !banker.IsGameStarted() )
		{
			if ( !IsTimerFlag() && ( banker.GetLobbyPlayers() >= MIN_PLAYERS_TO_START ) && ( banker.GetLobbyPlayers() <= MAX_PLAYERS ) )
			{
				alarm(TIME_TO_START);
				SetTimerFlag();
				const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_START_TIME_TOKEN );
			}

			if ( IsAlrmFlag() )
			{
				UnsetAlrmFlag();
				UnsetTimerFlag();

				if ( banker.GetLobbyPlayers() < MIN_PLAYERS_TO_START )
				{
					const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_START_CANCELLED_TOKEN );
				}
				else
				{
					const_cast<Banker&>(banker).SetGameStarted();
					const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::SEND_GAME_STARTED_TOKEN );
				}
			}
		}
		else
		{
			if ( !banker.IsGameStatePrepared() )
			{
				PrepareGameState( banker.GetId() );
			}

			if ( banker.GetReadyPlayers() == banker.GetAlivePlayers() )
			{
				EndGameTurn( banker.GetId() );
			}
		}
	}
}

void Server::FillReadfds()
{
	FD_ZERO(&readfds);

	FD_SET(ls, &readfds);

	max_fd = ls;

	for ( int i = SessionsPlanner::DEFAULT_NEXT_SESSION_ID; i <= session_planner.GetSessionsCount(); ++i )
	{
		for ( int j = 0; j < MAX_PLAYERS; ++j )
		{
			const Player* p = session_planner.GetSessionById( i )->GetPlayers()[j];
			if ( !p->IsFree() )
			{
				int p_fd = p->GetFd();

				FD_SET( p_fd, &readfds );
				if ( p_fd > max_fd )
					max_fd = p_fd;
			}
		}
	}
}

int Server::Run()
{
	signal(SIGINT, exit_handler);
	signal(SIGALRM, alrm_handler);

	srand(time(0));

	while ( 1 )
	{
		if ( exit_flag )
			Stop( 0 );

		FillReadfds();

		timeval tv { 0, 300000 };
		int res = select(max_fd+1, &readfds, nullptr, nullptr, &tv);
		if ( res == -1 )
		{
			if ( errno != EINTR )
			{
				fprintf(stderr, "\nselect() failed. (errno code = %d)\n", errno);
			}
			else
			{
				fprintf(stderr, "\nGot some signal (#%d).\n", sig_number);
			}
		}
		else if ( res > 0 )
		{
			NewClientHandle();
			ClientsInputHandle();
		}

		GameEventsHandle();
	}
}

#endif
