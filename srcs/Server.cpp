#ifndef SERVER_CORE_CPP
#define SERVER_CORE_CPP


#include "Server.hpp"
#include "BrokerMessages.hpp"
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netdb.h>
#include <ctime>
#include <list>


enum
{
					LISTEN_QUEUE_LEN			=						  5
};


// Описаны в модуле Banker
extern const double amount_multiplier_table[MARKET_LEVEL_NUMBER][2];
extern const int price_table[MARKET_LEVEL_NUMBER][2];
extern const int states_market_chance[MARKET_LEVEL_NUMBER][MARKET_LEVEL_NUMBER];


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

	ConcatAddrPort( Sender::SERVICE_SIZE );
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

void Server::Make( const char* addr, const char* port )
{
	sessions_planner.Make( SessionsPlanner::DEFAULT_START_SESSIONS_COUNT );

	UnsetExitFlag();
	SetSignalNum( 0 );
	SetAddrBuffer( addr, port );
	ListenSocketInit();
	SetMaxFd( 0 );

	printf("Waiting connections to %s port...\n", port);
}

Server::~Server()
{
	freeaddrinfo(bind_address);
}

void Server::ConcatAddrPort( int service_size )
{
	int addr_len = strlen(address_buffer);
	address_buffer[addr_len] = ':';

	int i = addr_len + 1;
	for ( int j = 0; ( j < service_size-1 ) && service_buffer[j]; ++j, ++i )
		address_buffer[i] = service_buffer[j];
	address_buffer[i] = '\0';
}

void Server::CloseConnection( int session_id, int player_number )
{
	const Banker& banker = *sessions_planner.GetSessionById( session_id );

	const Player* p = banker.GetPlayers().GetPlayerByUID( player_number );
	if ( p != nullptr )
	{
		int p_fd = p->GetFd();

		char ip[100] = { 0 };
		strncpy( ip, p->GetAddr(),  Receiver::ADDRESS_SIZE );

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

	for ( int i = SessionsPlanner::DEFAULT_NEXT_SESSION_ID; i <= sessions_planner.GetSessionsCount(); ++i )
	{
		const Banker& banker = *sessions_planner.GetSessionById( i );
		for ( int j = 0; j < MAX_PLAYERS; ++j )
			CloseConnection( i, banker.GetPlayers().GetUIDByIdx( j ) );
	}

	if ( forcely )
		printf("%s", "========== SERVER IS STOPPING WORK FORCELY ==========\n\n");

	exit(0);
}

bool Server::QuitPlayer( int session_id, int player_number )
{
	itoa( session_id, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::SESSION_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	itoa( player_number, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[MulticastActionsExec::LEFT_PLAYER_ID_PARAM_TOKEN]), MessageTokens::MESSAGE_TOKEN_SIZE-1 );
	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).PutMessage( msg_tokens.GetValue(), MulticastActionsExec::LEFT_PLAYER_ID_PARAM_TOKEN+1 );

	CloseConnection( session_id, player_number );

	const_cast<MulticastActionsExec&>(EMultiActionsExec.GetBroker()).TakeMessage( MulticastActionsExec::QUIT_PLAYER_TOKEN );

	if ( sessions_planner.GetSessionById( session_id )->GetAlivePlayers() <= 1 )
	{
		SetExitFlag();
		return true;
	}

	return false;
}

void Server::NewClientHandle()
{
	char new_client_addr[Receiver::ADDRESS_SIZE];
	char new_client_serv[Receiver::SERVICE_SIZE];
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

	ConcatAddrPort( Receiver::SERVICE_SIZE );

	printf("New connection from (%s)\n", new_client_addr);

	try
	{
		sessions_planner.AddNewClientToSession( cs, new_client_addr );
	}
	catch( ... )
	{
		close(cs);
		printf("Lost connection from (%s)\n", new_client_addr);
		return;
	}
}

void Server::IncomingEventsHandle()
{
	for ( int i = 0; i < max_fd + 1; ++i )
	{
		if ( FD_ISSET(i, &readfds) )
		{
			if ( i == ls )
			{
				NewClientHandle();
				continue;
			}

			if ( sessions_planner.GetStartTimers().IsTimerFd(i)  )
			{
				// обработка таймера
				//
				//

				continue;
			}

			std::pair<int, int> player_pos { -1, -1 };
			if ( sessions_planner.IsPlayerFd(i, player_pos) )
			{
				try
				{
					sessions_planner.PlayerEventHandle(player_pos);
				}
				catch ( const QuitCommandSuccessException& ex )
				{
					QuitPlayer( ex.GetSessionID(), ex.GetUID() );
					continue;
				}
				catch ( const InternalServerErrorException& ex )
				{
					QuitPlayer( ex.GetSessionID(), ex.GetUID() );
					continue;
				}
				catch ( const LostConnectionException& ex )
				{
					QuitPlayer( ex.GetSessionID(), ex.GetUID() );
				}
			}
		}
	}
}

void Server::RefillReadfds()
{
	FD_ZERO(&readfds);

	FD_SET(ls, &readfds);

	max_fd = ls;

	std::list<int> valid_fds = sessions_planner.GetValidFdsList();

	for ( const auto& fd : valid_fds )
	{
		FD_SET( fd, &readfds );
		if ( fd > max_fd )
			max_fd = fd;
	}

	/*for ( int i = SessionsPlanner::DEFAULT_NEXT_SESSION_ID; i <= session_planner.GetSessionsCount(); ++i )
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
	}*/
}

int Server::Run()
{
	signal(SIGINT, exit_handler);

	srand(time(0));

	while ( 1 )
	{
		if ( exit_flag )
			Stop( 0 );

		RefillReadfds();

		timeval tv { 0, 500000 };
		int res = select(max_fd+1, &readfds, nullptr, nullptr, &tv);
		if ( res == -1 )
		{
			if ( errno == EINTR )
			{
				if ( sig_number == SIGINT )
					Stop( 1 );
			}
			else
			{
				fprintf(stderr, "\nselect() failed. (errno code = %d)\n", errno);
			}
		}
		else if ( res > 0 )
		{
			IncomingEventsHandle();
		}

		sessions_planner.GameEventsHandle();
	}
}

#endif
