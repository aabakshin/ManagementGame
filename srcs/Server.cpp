#ifndef SERVER_CORE_CPP
#define SERVER_CORE_CPP


#include "Server.hpp"
#include "Utility.hpp"
#include "MGExceptions.hpp"
#include "Logger.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <csignal>


enum
{
					LISTEN_QUEUE_LEN					=						  5
};


// Ф-я-обработчик сигнала SIGINT
void exit_handler( int sig_no )
{
	Server::SetSignalNum( sig_no );
	Server::SetExitFlag();
}

// Ф-я обработчик сигнала SIGUSR1
void g_cfg_handler( int sig_no )
{
	Server::SetSignalNum( sig_no );
	Server::SetReloadCfgFlag();
}


void Server::SetListenSocket( int socket_value )
{
	if ( socket_value < -1 )
	{
		srv_msgs_logger->error("[Server::SetListenSocket] ", "Invalid socket value: ", socket_value);
		Stop( 0 );
	}

	ls = socket_value;
}

void Server::SetAddrBuffer( const char* addr, const char* port )
{
	srv_msgs_logger->info("Configuring local address...");

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family			=		AF_INET;
	hints.ai_socktype		=		SOCK_STREAM;
	hints.ai_flags			=		AI_PASSIVE;

	if ( getaddrinfo(addr, port, &hints, &bind_address) != 0 )
	{
		srv_msgs_logger->error("[Server::SetAddrBuffer] ", "An error has occured with \"getaddrinfo\". Message: ", gai_strerror(errno));
		Stop( 0 );
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
		srv_msgs_logger->error("[Server::SetMaxFd] ", "Invalid \"max_fd\": ", max_value);
		Stop( 0 );
	}

	max_fd = max_value;
}

void Server::ListenSocketInit()
{
	srv_msgs_logger->info("Creating listening socket...");

	SetListenSocket( socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol) );
	if ( GetListenSocket() == -1 )
	{
		srv_msgs_logger->error("[Server::ListenSocketInit] ", "socket() failed. Message: ", strerror(errno));
		Stop( 0 );
	}

	srv_msgs_logger->info("Setting socket options...");
	int opt_value = 1;
	setsockopt(GetListenSocket(), SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));

	srv_msgs_logger->info("Binding socket to address...");
	if ( bind(GetListenSocket(), bind_address->ai_addr, bind_address->ai_addrlen) )
	{
		srv_msgs_logger->error("[Server::ListenSocketInit] ", "bind() failed. Message: ", strerror(errno));
		Stop( 0 );
	}

	srv_msgs_logger->info("Enabling listen mode...");
	if ( listen(GetListenSocket(), LISTEN_QUEUE_LEN) < 0 )
	{
		srv_msgs_logger->error("[Server::ListenSocketInit] ", "listen() failed. Message: ", strerror(errno));
		Stop( 0 );
	}
}

void Server::Make( const char* addr, const char* port, Logger* logger, std::shared_ptr<const Config::GameSettings> m_g_sets )
{
	srv_msgs_logger = logger;
	SetSignalNum( 0 );
	SetAddrBuffer( addr, port );
	ListenSocketInit();
	SetMaxFd( 0 );

	m_game_config_settings = std::move(m_g_sets);
	sessions_planner.Make( SessionsPlanner::DEFAULT_START_SESSIONS_COUNT, m_game_config_settings );

	srv_msgs_logger->info("Waiting connections to", port, " port...");
}

Server::~Server()
{
	freeaddrinfo( bind_address );
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
		srv_msgs_logger->info("Lost connection from [", address.c_str(), "]");
}

void Server::Stop( int forcely )
{
	if ( forcely )
	{
		srv_msgs_logger->info("\n\n");
		srv_msgs_logger->info("========== SERVER IS STOPPING WORK FORCELY ==========");
	}

	std::list<std::pair<int, std::string>> players_fds;
	sessions_planner.GetAllPlayersFds( players_fds );

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
		srv_msgs_logger->info("========== SERVER IS STOPPING WORK FORCELY ==========\n");

	exit( 0 );
}

void Server::NewClientHandle()
{
	char new_client_addr[Receiver::ADDRESS_SIZE];
	char new_client_serv[Receiver::SERVICE_SIZE];
	struct sockaddr_storage client_address;
	socklen_t client_address_len = sizeof(client_address);

	int cs = accept( ls, (struct sockaddr*) &client_address, &client_address_len );
	if ( cs == -1 )
	{
		srv_msgs_logger->error("[Server::NewClientHandle]", "accept() failed. Message: ", strerror(errno));
		Stop( 0 );
	}

	getnameinfo(
			(struct sockaddr*) &client_address,
			client_address_len,
			new_client_addr,
			sizeof(new_client_addr),
			new_client_serv,
			sizeof(new_client_serv),
			NI_NUMERICHOST | NI_NUMERICSERV
			);

	ConcatAddrPort( Receiver::SERVICE_SIZE );

	srv_msgs_logger->info("New connection from ", new_client_addr );

	try
	{
		sessions_planner.AddNewClientToSession( cs, new_client_addr );
	}
	catch( ... )
	{
		srv_msgs_logger->error("[Server::NewClientHandle] ", "An error occured in \"AddNewClientToSession\" function!");
		CloseConnection( cs, new_client_addr );
	}
}

void Server::IncomingEventsHandle()
{
	for ( int i = 0; i < max_fd + 1; ++i )
	{
		if ( FD_ISSET( i, &readfds ) )
		{
			if ( i == ls )
			{
				NewClientHandle();
				continue;
			}

			if ( sessions_planner.GetStartTimers().IsTimerFd( i )  )
			{
				try
				{
					int t_idx = sessions_planner.GetStartTimers().GetTimerIdxByFd( i );
					const_cast<SessionsPlanner::StartSessionsTimers&>(sessions_planner.GetStartTimers())[t_idx].StopTimer();
				}
				catch ( const std::runtime_error& ex )
				{
					srv_msgs_logger->error("[Server::IncomingEventsHandle] ", ex.what());
					Stop( 0 );
				}

				continue;
			}

			std::pair<int, int> player_pos { -1, -1 };
			if ( sessions_planner.IsPlayerFd( i, player_pos ) )
			{
				try
				{
					sessions_planner.PlayerEventHandle( player_pos );
				}
				catch ( const std::range_error& ex )
				{
					srv_msgs_logger->error("[Server::IncomingEventsHandle] ", ex.what());
					continue;
				}
				catch ( const QuitCommandException& ex )
				{
					srv_msgs_logger->error("[Server::IncomingEventsHandle] ", ex.what());
					CloseConnection( i, ex.GetAddr() );
					continue;
				}
				catch ( const InternalCmdExecuteException& ex )
				{
					srv_msgs_logger->error("[Server::IncomingEventsHandle] ", ex.what());
					CloseConnection( i, ex.GetAddr() );
					continue;
				}
				catch ( const PlayerLostConnectionException& ex )
				{
					srv_msgs_logger->error("[Server::IncomingEventsHandle] ", ex.what());
					CloseConnection( i, ex.GetAddr() );
					continue;
				}
				catch ( const std::runtime_error& ex )
				{
					srv_msgs_logger->error("[Server::IncomingEventsHandle] ", ex.what());
					Stop( 0 );
				}
			}
		}
	}
}

void Server::RefillReadfds()
{
	FD_ZERO( &readfds );

	FD_SET( ls, &readfds );

	max_fd = ls;

	std::list<int> valid_fds = sessions_planner.GetValidFdsList();

	for ( const auto& fd : valid_fds )
	{
		FD_SET( fd, &readfds );
		if ( fd > max_fd )
			max_fd = fd;
	}

	for ( int i = 0; i < SessionsPlanner::DEFAULT_MAX_SESSIONS_COUNT; ++i )
	{
		int timer_fd = const_cast<SessionsPlanner::StartSessionsTimers&>(sessions_planner.GetStartTimers())[i].GetTimerFd();
		FD_SET( timer_fd, &readfds );
		if ( timer_fd > max_fd )
			max_fd = timer_fd;
	}
}

void Server::ReloadGameConfig()
{
	Config new_config;

	try
	{
		if ( !new_config.Load("config.json") )
			return;

		auto new_game_settings = std::make_shared<const Config::GameSettings>(std::move(new_config.game_settings));

		{
			std::lock_guard<std::mutex> lock(m_game_settings_mutex);
			m_game_config_settings = std::move(new_game_settings);
		}

		sessions_planner.ApplySettings( m_game_config_settings );

		srv_msgs_logger->info("Config reloaded successfully");
		std::cout << "[" << Utility::current_time_str() << "] " << "[INFO] " << "Config reloaded successfully" << std::endl;
	}
	catch ( const nlohmann::json::parse_error& ex )
	{
		throw;
	}
}

int Server::Run()
{
	struct sigaction exit;
	Utility::set_signal_disposition(exit, SIGINT, exit_handler, 0);

	Utility::ignore_unused_signals();

	struct sigaction g_cfg;
	Utility::set_signal_disposition(g_cfg, SIGUSR1, g_cfg_handler, SA_RESTART);

	srand(time(0));


	while ( 1 )
	{
		if ( CheckAndClearExitFlag() )
			Stop( 0 );

		if ( CheckAndClearCfgFlag() )
		{
			try
			{
				ReloadGameConfig();
			}
			catch ( const nlohmann::json::parse_error& ex )
			{
				throw;
			}

			continue;
		}

		RefillReadfds();

		timeval tv { 0, 500000 };
		int res = select( max_fd+1, &readfds, nullptr, nullptr, &tv );
		if ( res == -1 )
		{
			if ( errno == EINTR )
			{
				if ( sig_number == SIGINT )
					Stop( 1 );
			}
			else
			{
				srv_msgs_logger->error("[Server::Run] ", "select() failed. Message: ", strerror(errno) );
				Stop( 0 );
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
			if ( !ex.GetBankrots().empty() )
				for ( const auto& bankrot : ex.GetBankrots() )
					CloseConnection( bankrot.first, bankrot.second );
		}
		catch ( const std::runtime_error& ex )
		{
			srv_msgs_logger->error("[Server::Run] ", ex.what());
			Stop(0);
		}
	}
}

#endif
