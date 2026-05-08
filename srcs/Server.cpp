#ifndef SERVER_CORE_CPP
#define SERVER_CORE_CPP


#include "Server.hpp"
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
#include <list>
#include <string>


enum
{
					LISTEN_QUEUE_LEN			=						  5
};


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
	UnsetExitFlag();
	SetSignalNum( 0 );
	SetAddrBuffer( addr, port );
	ListenSocketInit();
	SetMaxFd( 0 );

	sessions_planner.Make( SessionsPlanner::DEFAULT_START_SESSIONS_COUNT );

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

void Server::CloseConnection( int fd, std::string address )
{
	close(fd);
	FD_CLR(fd, &readfds);

	if (
			( strcmp( address.c_str(), "start_timers" ) != 0 )	&&
			( strcmp( address.c_str(), "ls" ) != 0 )
		)
		printf("[-] Lost connection from [%s]\n", address.c_str());
}

void Server::Stop( int forcely )
{
	if ( forcely )
		printf("%s", "\n\n========== SERVER IS STOPPING WORK FORCELY ==========\n");

	std::list<std::pair<int, std::string>> players_fds;
	sessions_planner.QuitAllPlayers( players_fds );

	if ( !players_fds.empty() )
		for ( const auto& close_pair : players_fds )
			CloseConnection( close_pair.first, close_pair.second );

	for ( int i = 0; i < SessionsPlanner::DEFAULT_MAX_SESSIONS_COUNT; ++i )
	{
		int fd = const_cast<SessionsPlanner::StartSessionsTimers&>(sessions_planner.GetStartTimers())[i].GetTimerFd();
		CloseConnection( fd, "start_timers" );
	}

	CloseConnection( ls, "ls" );

	if ( forcely )
		printf("%s", "========== SERVER IS STOPPING WORK FORCELY ==========\n\n");

	exit(0);
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
		CloseConnection(cs, new_client_addr);
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
			if ( sessions_planner.IsPlayerFd( i, player_pos ) )
			{
				try
				{
					sessions_planner.PlayerEventHandle(player_pos);
				}
				catch ( const QuitCommandSuccessException& ex )
				{
					CloseConnection( ex.GetFd(), ex.GetAddress() );
					continue;
				}
				catch ( const InternalServerErrorException& ex )
				{
					CloseConnection( ex.GetFd(), ex.GetAddress() );
					continue;
				}
				catch ( const LostConnectionException& ex )
				{
					CloseConnection( ex.GetFd(), ex.GetAddress() );
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

		try
		{
			sessions_planner.GameEventsHandle();
		}
		catch ( const KickBankrotsException& ex )
		{
			if ( !ex.bankrots.empty() )
				for ( const auto& bankrot : ex.bankrots )
					CloseConnection( bankrot.first, bankrot.second );
		}
	}
}

#endif
