/* Файл реализации модуля serverCore */

#ifndef SERVER_CORE_CPP
#define SERVER_CORE_CPP


#include "Server.hpp"
#include "Banker.hpp"
#include "MGLib.h"
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


static int show_sending_message( const char* send_buf, int mes_len, const char* address, int wc );
static int send_lostlobbyplayer_message( int lobby_players, int cs, const char* address );
static int send_lostaliveplayer_message( int alive_players, int player_number, int cs, const char* address );
static int send_victory_message( int cs, const char* address );
static int send_startinseconds_message( int cs, const char* address );
static int send_gamestarted_message( int cs, const char* address );
static int send_gamealreadystarted_message( int cs, const char* address );
static int send_startgameinfo_message( int alive_players, int turn_number, const Player*, const MarketState& );
static int send_startcancelled_message( int cs, const char* address );
static int send_newplayerconnect_message( int lobby_players, int cs, const char* address );
static int send_gamenotstarted_message( int cs, const char* address );
static int send_auctionreport_message( int ready_players, int turn_number, const Player* const* pl_array, int cs, const char* address );
static int send_newturn_message( int turn_number, bool is_bot, const MarketState&, int cs, const char* address );
static int send_serverfull_message( int cs, const char* address );
static int send_cmdinternalerror_message( int cs, const char* address );
static int send_cmdincorrectargsnum_message( int cs, const char* address );
static int send_unknowncmd_message( int cs, const char* address );
static int send_wfnt_message( int wait_yet_players_amount, int cs, const char* address );

static bool is_correct_identity_msg( const char* identity_msg );
static int concat_addr_port( char* address_buffer, const char* service_buffer );
int send_message( const int fd, const char** message_tokens, const int tokens_amount, const char* address );


/* Список сообщений для идентификации бот-клиента */
static const char* bot_identity_messages[] = {
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
	if (
				( strlen(addr) >= ADDRESS_BUFFER )		||
				( strlen(addr) < 1 )						||
				( port == nullptr )
		)
	{
		memset( address_buffer, 0, ADDRESS_BUFFER );
		return;
		// throw InvalidArgsValuesException();
	}

	printf("%s\n", "Configuring local address...");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ( getaddrinfo(addr, port, &hints, &GetBindAddress()) != 0 )
	{
		fprintf(stderr, "An error has occured with \"getaddrinfo\" (errno code = %d)\n", errno);
		return;
		// throw InvalidGetAddrInfoException();
	}

	char addr_buffer[100];
	char serv_buffer[100];
	getnameinfo(
			GetBindAddress()->ai_addr,
			GetBindAddress()->ai_addrlen,
			addr_buffer,
			sizeof(addr_buffer),
			serv_buffer,
			sizeof(serv_buffer),
			NI_NUMERICHOST | NI_NUMERICSERV);
	concat_addr_port(addr_buffer, serv_buffer);

	strcpy(address_buffer, addr_buffer );
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
	SetListenSocket( socket(GetBindAddress()->ai_family, GetBindAddress()->ai_socktype, GetBindAddress()->ai_protocol) );
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
	if ( bind(GetListenSocket(), GetBindAddress()->ai_addr, GetBindAddress()->ai_addrlen) )
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
	freeaddrinfo(GetBindAddress());
}

static int show_sending_message( const char* send_buf, int mes_len, const char* address, int wc )
{
	if (
						( send_buf == nullptr )					||
						( *send_buf == '\n' )					||
						( *send_buf == '\0' )					||
						( mes_len < 0 )							||
						( address == nullptr )					||
						( *address == '\0' )					||
						( *address == '\n' )					||
						( wc < 0 )
		)
		return 0;


	Server::SetSentMsgsCount( Server::GetSentMsgsCount() + 1 );
	printf("\n==================== (%d) ====================\n", Server::GetSentMsgsCount());

	for ( int i = 0; i < mes_len + 10; ++i )
	{
		printf("%3d ", send_buf[i]);
		if ( ((i+1) % 10) == 0 )
			putchar('\n');
	}
	putchar('\n');

	printf(
					"\nsend_buf = %s\n"
					"Sent to [%s] %d\\%d bytes\n"
					"==================== (%d) ====================\n\n", send_buf, address, wc, mes_len, Server::GetSentMsgsCount());

	return 1;
}

static int send_lostlobbyplayer_message( int lobby_players, int cs, const char* address )
{
	char lp_buf[10];
	itoa(lobby_players, lp_buf, 9);

	char max_pl_buf[10];
	itoa(MAX_PLAYERS, max_pl_buf, 9);


	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[LOST_LOBBY_PLAYER]),
				lp_buf,
				max_pl_buf,
				nullptr
	};
	send_message(cs, mes_tokens, 3, address);

	return 1;
}

static int send_lostaliveplayer_message( int alive_players, int player_number, int cs, const char* address )
{
	char ap_buf[10];
	itoa(alive_players, ap_buf, 9);

	char left_p_num_buf[10];
	itoa(player_number, left_p_num_buf, 9);


	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[LOST_ALIVE_PLAYER]),
				ap_buf,
				left_p_num_buf,
				nullptr
	};
	send_message(cs, mes_tokens, 3, address);

	return 1;
}

static int send_victory_message( int cs, const char* address )
{
	const char* victory_message[] =
	{
				info_game_messages[VICTORY_MESSAGE],
				nullptr
	};
	send_message(cs, victory_message, 1, address );

	return 1;
}

static int send_startinseconds_message( int cs, const char* address )
{
	char tts[10];
	itoa(TIME_TO_START, tts, 9);


	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[STARTINSECONDS]),
				tts,
				nullptr
	};
	send_message(cs, mes_tokens, 2, address);

	return 1;
}

static int send_gamestarted_message( int cs, const char* address )
{
	const char* message[] =
	{
				info_game_messages[GAME_STARTED],
				nullptr
	};
	send_message(cs, message, 1, address);

	return 1;
}

static int send_startgameinfo_message( int alive_players, int turn_number, const Player* p, const MarketState& market )
{
	char p_num[10];
	itoa(p->GetUID(), p_num, 9);

	char ap[10];
	itoa(alive_players, ap, 9);

	char tn[10];
	itoa(turn_number, tn, 9);

	char p_money[20];
	itoa(p->GetMoney(), p_money, 19);

	char p_sources[10];
	itoa(p->GetSources(), p_sources, 9);

	char p_products[10];
	itoa(p->GetProducts(), p_products, 9);

	char p_wf[10];
	itoa(p->GetWaitFactories(), p_wf, 9);

	char p_wrkf[10];
	itoa(p->GetWorkFactories(), p_wrkf, 9);

	char p_bf[10];
	itoa(p->GetBuiltFactories(), p_bf, 9);

	char sa[10];
	char msp[10];
	char pa[10];
	char mpp[10];

	int tokns_amnt = 10;
	if ( p->IsBot() )
	{
		itoa(market.GetSourcesAmount(), sa, 9);
		itoa(market.GetSourceMinPrice(), msp, 9);
		itoa(market.GetProductsAmount(), pa, 9);
		itoa(market.GetProductMaxPrice(), mpp, 9);
		tokns_amnt = 14;
	}

	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[STARTING_GAME_INFORMATION]),
				p_num,
				ap,
				tn,
				p_money,
				p_sources,
				p_products,
				p_wf,
				p_wrkf,
				p_bf,
				sa,
				msp,
				pa,
				mpp,
				nullptr
	};
	send_message(p->GetFd(), mes_tokens, tokns_amnt, p->GetAddr());

	return 1;
}

static int send_startcancelled_message( int cs, const char* address )
{
	const char* mes_tokens[] =
	{
				info_game_messages[STARTCANCELLED],
				nullptr
	};
	send_message(cs, mes_tokens, 1, address);

	return 1;
}

static int send_newplayerconnect_message( int lobby_players, int cs, const char* address )
{
	char lp_buf[10];
	itoa(lobby_players, lp_buf, 9);

	char max_pl_buf[10];
	itoa(MAX_PLAYERS, max_pl_buf, 9);


	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[NEW_PLAYER_CONNECT]),
				lp_buf,
				max_pl_buf,
				nullptr
	};
	send_message(cs, mes_tokens, 3, address);

	return 1;
}

static int send_gamenotstarted_message( int cs, const char* address )
{
	const char* message[] =
	{
				info_game_messages[GAME_NOT_STARTED],
				nullptr
	};
	send_message(cs, message, 1, address);

	return 1;
}

static int send_auctionreport_message( int ready_players, int turn_number, const Player* const* pl_array, int cs, const char* address )
{
	enum
	{
			PL_REP_FIELDS_NUM		=		6,
			TURN_SIZE				=		16,
			PLAYER_NUM_SIZE			=		16,
			SOLD_SOURCES_SIZE		=		16,
			SOLD_PRICE_SIZE			=		24,
			BOUGHT_PRODS_SIZE		=		16,
			BOUGHT_PRICE_SIZE		=		24

	};
	struct player_report
	{
		char tn[TURN_SIZE];
		char pnum[PLAYER_NUM_SIZE];
		char ssnum[SOLD_SOURCES_SIZE];
		char spnum[SOLD_PRICE_SIZE];
		char bpnum[BOUGHT_PRODS_SIZE];
		char bprnum[BOUGHT_PRICE_SIZE];
	};

	player_report pr[MAX_PLAYERS];
	memset(pr, 0, sizeof(player_report) * MAX_PLAYERS);


	int tokns_amnt = PL_REP_FIELDS_NUM * MAX_PLAYERS + 1;
	char* mes_tokens[tokns_amnt];

	for ( int i = 0; i < tokns_amnt; ++i )
		mes_tokens[i] = nullptr;

	mes_tokens[0] = const_cast<char*>(info_game_messages[AUCTION_RESULTS]);

	int k = 1;
	for ( int i = 0; i < ready_players; ++i )
	{
		itoa(turn_number, pr[i].tn, TURN_SIZE);
		mes_tokens[k] = pr[i].tn;
		k++;

		itoa(pl_array[i]->GetUID(), pr[i].pnum, PLAYER_NUM_SIZE);
		mes_tokens[k] = pr[i].pnum;
		k++;

		itoa(const_cast<Player*>(pl_array[i])->GetAuctionReport().GetSoldSources(), pr[i].ssnum, SOLD_SOURCES_SIZE);
		mes_tokens[k] = pr[i].ssnum;
		k++;

		itoa(const_cast<Player*>(pl_array[i])->GetAuctionReport().GetSoldPrice(), pr[i].spnum, SOLD_PRICE_SIZE);
		mes_tokens[k] = pr[i].spnum;
		k++;

		itoa(const_cast<Player*>(pl_array[i])->GetAuctionReport().GetBoughtProducts(), pr[i].bpnum, BOUGHT_PRODS_SIZE);
		mes_tokens[k] = pr[i].bpnum;
		k++;

		itoa(const_cast<Player*>(pl_array[i])->GetAuctionReport().GetBoughtPrice(), pr[i].bprnum, BOUGHT_PRICE_SIZE);
		mes_tokens[k] = pr[i].bprnum;
		k++;
	}
	tokns_amnt = k;

	send_message(cs, const_cast<const char**>(mes_tokens), tokns_amnt, address);

	return 1;
}

static int send_newturn_message( int turn_number, bool is_bot, const MarketState& market, int cs, const char* address )
{
	char tn[10];
	itoa(turn_number, tn, 9);

	char sa[10];
	char msp[10];
	char pa[10];
	char mpp[10];

	int tokns_amnt = 2;
	if ( is_bot )
	{
		itoa(market.GetSourcesAmount(), sa, 9);
		itoa(market.GetSourceMinPrice(), msp, 9);
		itoa(market.GetProductsAmount(), pa, 9);
		itoa(market.GetProductMaxPrice(), mpp, 9);
		tokns_amnt = 6;
	}

	const char* mes_tokens[] =
	{
				const_cast<char*>(info_game_messages[NEW_TURN]),
				tn,
				sa,
				msp,
				pa,
				mpp,
				nullptr
	};
	send_message(cs, mes_tokens, tokns_amnt, address);

	return 1;
}

int send_message( int fd, const char** message_tokens, int tokens_amount, const char* ip )
{
	if (
					( fd < 0 )								||
					( message_tokens == nullptr )			||
					( *message_tokens == nullptr )			||
					( tokens_amount < 1 )					||
					( ip == nullptr )
		)
		return 0;


	char send_buf[BUFSIZE] = { 0 };

	int i = 0;
	for ( int j = 0; j < tokens_amount; j++ )
	{
		for ( int k = 0; message_tokens[j][k]; k++, i++ )
			send_buf[i] = message_tokens[j][k];
		send_buf[i] = '|';
		i++;
	}

	send_buf[i-1] = '\n';
	send_buf[i] = '\0';
	int mes_len = i;

	int wc = write(fd, send_buf, mes_len);
	if ( wc < 0 )
		return 0;

	show_sending_message(send_buf, mes_len, ip, wc);

	return 1;
}

static bool is_correct_identity_msg( const char* identity_msg )
{
	for ( int i = 0; bot_identity_messages[i] != nullptr; ++i )
		if ( strcmp(identity_msg, bot_identity_messages[i]) == 0 )
			return true;

	return false;
}

int Server::CloseConnection( int player_number )
{
	int p_fd = GetBanker().GetPlayerByNum(player_number)->GetFd();
	char ip[100] = { 0 };
	strcpy(ip, GetBanker().GetPlayerByNum(player_number)->GetAddr());


	GetBanker().CleanPlayer(player_number);
	close(p_fd);
	FD_CLR(p_fd, &readfds);
	printf("[+] Lost connection from [%s]\n", ip);

	return 1;
}

void Server::Stop( int forcely )
{
	if ( forcely )
		printf("%s", "\n\n========== SERVER IS STOPPING WORK FORCELY ==========\n");

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		CloseConnection( i+1 );
		GetBanker().SetPlayer(i, nullptr);
	}

	if ( forcely )
		printf("%s", "========== SERVER IS STOPPING WORK FORCELY ==========\n\n");

	exit(0);
}

static int send_gamealreadystarted_message( int cs, const char* address )
{
	const char* message[] =
	{
				info_game_messages[GAME_ALREADY_STARTED],
				nullptr
	};
	send_message(cs, message, 1, address );

	return 1;
}

static int send_serverfull_message( int cs, const char* address )
{
	const char* message[] =
	{
				info_game_messages[SERVER_FULL],
				nullptr
	};
	send_message(cs, message, 1, address);

	return 1;
}

static int send_cmdinternalerror_message( int cs, const char* address )
{
	const char* message[] =
	{
				error_game_messages[COMMAND_INTERNAL_ERROR],
				nullptr
	};
	send_message(cs, message, 1, address);

	return 1;
}

static int send_cmdincorrectargsnum_message( int cs, const char* address )
{
	const char* error_message[] =
	{
				error_game_messages[COMMAND_INCORRECT_ARGUMENTS_NUM],
				nullptr
	};

	send_message(cs, error_message, 1, address);

	return 1;
}

static int send_unknowncmd_message( int cs, const char* address )
{
	const char* unknown_cmd_message[] =
	{
				info_game_messages[UNKNOWN_COMMAND],
				nullptr
	};
	send_message(cs, unknown_cmd_message, 1, address);

	return 1;
}

static int send_wfnt_message( int wait_yet_players_amount, int cs, const char* address )
{
	char w_y_p_a[10];
	itoa(wait_yet_players_amount, w_y_p_a, 9);

	int tokns_amnt = 2;
	const char* mes_tokens[] =
	{
			const_cast<char*>(info_game_messages[WAIT_FOR_NEXT_TURN]),
			w_y_p_a,
			nullptr
	};
	send_message(cs, mes_tokens, tokns_amnt, address);

	return 1;
}

static int concat_addr_port( char* address_buffer, const char* service_buffer )
{
	if (
				( address_buffer == nullptr )			||
				( service_buffer == nullptr )
		)
		return 0;

	int addr_len = strlen(address_buffer);
	address_buffer[addr_len] = ':';

	int i = addr_len + 1;
	for ( int j = 0; service_buffer[j]; j++, i++ )
		address_buffer[i] = service_buffer[j];
	address_buffer[i] = '\0';

	return 1;
}

int Server::QuitPlayer( int player_number )
{
	CloseConnection(player_number);
	GetBanker().SetPlayer(GetBanker().GetIdxByNum(player_number), nullptr);

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		Player* p = GetBanker().GetPlayer(i);
		if ( p != nullptr )
		{
			if ( !GetBanker().IsGameStarted() )
			{
				send_lostlobbyplayer_message(GetBanker().GetLobbyPlayers(), p->GetFd(), p->GetAddr());
			}
			else
			{
				if ( GetBanker().GetAlivePlayers() == 1 )
				{
					send_victory_message(p->GetFd(), p->GetAddr());
					printf("\n\n<<<<< GAME IS FINISHED. PLAYER #%d IS WINNER! >>>>>\n\n", p->GetUID());
					Server::Stop( 0 );
				}
				else
				{
					send_lostaliveplayer_message(GetBanker().GetAlivePlayers(), player_number, p->GetFd(), p->GetAddr());
				}
			}
		}
	}

	return 1;
}

int Server::FillReadfds()
{
	FD_ZERO(&readfds);

	FD_SET(ls, &readfds);

	max_fd = ls;

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		Player* p = GetBanker().GetPlayer(i);
		if ( p != nullptr )
		{
			int p_fd = p->GetFd();

			FD_SET( p_fd, &readfds );
			if ( p_fd > max_fd )
				max_fd = p_fd;
		}
	}

	return 1;
}

int Server::Run()
{
	signal(SIGINT, exit_handler);
	signal(SIGALRM, alrm_handler);

	srand(time(0));


	while ( 1 )
	{
		if ( exit_flag )
			Server::Stop( 1 );

		Server::FillReadfds();

		if ( !GetBanker().IsGameStarted() )
		{
			if ( !IsTimerFlag() && (GetBanker().GetLobbyPlayers() >= MIN_PLAYERS_TO_START) && (GetBanker().GetLobbyPlayers() < MAX_PLAYERS) )
			{
				alarm(TIME_TO_START);

				SetTimerFlag();

				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					Player* p = GetBanker().GetPlayer(i);
					if ( p != nullptr )
						send_startinseconds_message(p->GetFd(), p->GetAddr());
				}
			}
			else if ( GetBanker().GetLobbyPlayers() == MAX_PLAYERS )
			{
				alarm(0);
				UnsetAlrmFlag();
				UnsetTimerFlag();

				/*FORCE START THE GAME*/
				GetBanker().SetGameStarted();
				/*FORCE START THE GAME*/

				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					Player* p = GetBanker().GetPlayer(i);
					if ( p != nullptr )
						send_gamestarted_message(p->GetFd(), p->GetAddr());
				}

				continue;
			}
		}
		else
		{
			if ( !GetBanker().IsPlayersPrepared() )
			{
				GetBanker().SetPlayersPrepared();
				GetBanker().SetAlivePlayers(GetBanker().GetLobbyPlayers());
				GetBanker().SetLobbyPlayers( 0 );
				GetBanker().SetTurnNumber( 1 );
				GetBanker().SetCurrentMarketLvl( START_MARKET_LEVEL );
				GetBanker().GetCurrentMarketState().SetSourcesAmount( amount_multiplier_table[START_MARKET_LEVEL-1][0] * GetBanker().GetAlivePlayers() );
				GetBanker().GetCurrentMarketState().SetSourceMinPrice( price_table[START_MARKET_LEVEL-1][0] );
				GetBanker().GetCurrentMarketState().SetProductsAmount( amount_multiplier_table[START_MARKET_LEVEL-1][1] * GetBanker().GetAlivePlayers() );
				GetBanker().GetCurrentMarketState().SetProductMaxPrice( price_table[START_MARKET_LEVEL-1][1] );

				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					Player* p = GetBanker().GetPlayer(i);
					if ( p != nullptr )
					{
						p->SetMoney( START_MONEY );
						p->SetOldMoney( p->GetMoney() );
						p->SetSources( START_SOURCES );
						p->SetProducts( START_PRODUCTS );
						p->SetWaitFactories( START_FACTORIES );

						send_startgameinfo_message(GetBanker().GetAlivePlayers(), GetBanker().GetTurnNumber(), p, GetBanker().GetCurrentMarketState());
					}
				}
			}

			GetBanker().check_producing_on_turn();
		}


		int res = select(max_fd+1, &readfds, nullptr, nullptr, nullptr);
		if ( res == -1 )
		{
			if ( errno == EINTR )
			{
				if ( ( sig_number == SIGALRM ) && alrm_flag && ( !GetBanker().IsGameStarted() ) )
				{
					UnsetAlrmFlag();
					UnsetTimerFlag();

					if ( GetBanker().GetLobbyPlayers() < MIN_PLAYERS_TO_START )
					{
						for ( int i = 0; i < MAX_PLAYERS; ++i )
						{
							Player* p = GetBanker().GetPlayer(i);
							if ( p != nullptr )
								send_startcancelled_message(p->GetFd(), p->GetAddr());
						}
					}
					else
					{
						if ( GetBanker().GetLobbyPlayers() < MAX_PLAYERS )
						{
							/* START THE GAME */
							GetBanker().SetGameStarted();
							/* START THE GAME */

							for ( int i = 0; i < MAX_PLAYERS; ++i )
							{
								Player* p = GetBanker().GetPlayer(i);
								if ( p != nullptr )
									send_gamestarted_message(p->GetFd(), p->GetAddr());
							}
						}
						continue;
					}
				}
				fprintf(stderr, "\nGot some signal (#%d).\n", sig_number);
			}
			else
			{
				fprintf(stderr, "\nselect() failed. (errno code = %d)\n", errno);
			}
			continue;
		}


		if ( FD_ISSET(ls, &readfds) )
		{
			char address_buffer[ADDRESS_BUFFER];
			char service_buffer[ADDRESS_BUFFER];
			struct sockaddr_storage client_address;
			socklen_t client_address_len = sizeof(client_address);

			int cs = accept(ls, (struct sockaddr*) &client_address, &client_address_len);
			if ( cs == -1 )
			{
				fprintf(stderr, "accept() failed. (errno code = %d)\n", errno);
				return 1;
			}

			getnameinfo(
					(struct sockaddr*) &client_address,
					client_address_len,
					address_buffer,
					sizeof(address_buffer),
					service_buffer,
					sizeof(service_buffer),
					NI_NUMERICHOST | NI_NUMERICSERV);

			concat_addr_port(address_buffer, service_buffer);
			printf("New client (%s) has connected.\n", address_buffer);

			if ( GetBanker().IsGameStarted() )
			{
				send_gamealreadystarted_message(cs, address_buffer);
				close(cs);
				printf("Lost connection from (%s)\n", address_buffer);
			}
			else
			{
				int i;
				for ( i = 0; i < MAX_PLAYERS; ++i )
					if ( GetBanker().GetPlayer(i) == nullptr )
						break;

				if ( i >= MAX_PLAYERS )
				{
					send_serverfull_message(cs, address_buffer);
					close(cs);
					printf("Lost connection from (%s)\n", address_buffer);
				}
				else
				{
					try
					{
						GetBanker().SetPlayer(i, new Player(cs, address_buffer, i+1));
					}
					catch ( ... )
					{
						send_cmdinternalerror_message(cs, address_buffer);
						close(cs);
						printf("Lost connection from [%s]\n", address_buffer);
					}

					GetBanker().SetLobbyPlayers( GetBanker().GetLobbyPlayers() + 1 );

					for ( i = 0; i < MAX_PLAYERS; ++i )
					{
						Player* p = GetBanker().GetPlayer(i);
						if ( p != nullptr )
							send_newplayerconnect_message(GetBanker().GetLobbyPlayers(), p->GetFd(), p->GetAddr());
					}
				}
			}
		}

		char read_buf[BUFSIZE];
		for ( int i = 0; i < MAX_PLAYERS; ++i )
		{
			Player* p = GetBanker().GetPlayer(i);
			if ( p != nullptr )
			{
				int p_fd = p->GetFd();
				const char* p_addr = p->GetAddr();

				if ( FD_ISSET(p_fd, &readfds) )
				{
					int rc = readline(p_fd, read_buf, BUFSIZE-1);
					if ( rc > 0 )
					{
						printf("Received %d bytes from [%s]\n", rc, p_addr);

						read_buf[rc] = '\0';
						cut_str(read_buf, rc, '\n');

						int size = strlen(read_buf)+1;
						delete_spaces(read_buf, &size);

						if ( !GetBanker().IsGameStarted() )
						{
							if ( !p->IsIdentMsgRecv() )
							{
								if ( is_correct_identity_msg(read_buf) )
									p->SetBot();
								else
									p->UnsetBot();

								p->SetIdentMsgRecv();
							}
							else
							{
								send_gamenotstarted_message(p_fd, p_addr);
							}
							continue;
						}

						// Обработка данных от игрока, когда игра началась
						GetCmdsHndl().make_cmd_tokens( read_buf );
						switch ( GetCmdsHndl().process_command(&GetBanker(), GetBanker().GetNumByIdx(i)) )
						{
							case QUIT_COMMAND_NUM:
								Server::QuitPlayer(GetBanker().GetNumByIdx(i));
								break;
							case INTERNAL_COMMAND_ERROR:
								send_cmdinternalerror_message(p_fd, p_addr);
								break;
							case INCORRECT_ARGS_COMMAND_ERROR:
								send_cmdincorrectargsnum_message(p_fd, p_addr);
								break;
							case UNKNOWN_COMMAND_ERROR:
								send_unknowncmd_message(p_fd, p_addr);
								break;
							case WFNT_COMMAND_ERROR:
								int wfnt = GetBanker().GetAlivePlayers() - GetBanker().GetReadyPlayers();
								send_wfnt_message(wfnt, p_fd, p_addr);
						}
					}
					else
					{
						Server::QuitPlayer(GetBanker().GetNumByIdx(i));
					}
				}
			}
		}

		// Действия, происходящие в конце игрового месяца
		if ( GetBanker().IsGameStarted() )
		{
			if ( GetBanker().GetReadyPlayers() == GetBanker().GetAlivePlayers() )
			{
				GetBanker().check_building_factories();
				GetBanker().start_auction( GetBanker().GetSourcesRequests(), SOURCE_AUCTION );
				GetBanker().start_auction( GetBanker().GetProductsRequests(), PRODUCTION_AUCTION );

				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					Player* p = GetBanker().GetPlayer(i);
					if ( p != nullptr )
						send_auctionreport_message(GetBanker().GetReadyPlayers(), GetBanker().GetTurnNumber(), GetBanker().GetPlayers(), p->GetFd(), p->GetAddr());
				}

				GetBanker().pay_charges();
				GetBanker().report_on_turn();
				GetBanker().change_market_state();
				GetBanker().SetTurnNumber( GetBanker().GetTurnNumber() + 1 );
				GetBanker().SetReadyPlayers( 0 );

				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					Player* p = GetBanker().GetPlayer( i );
					if ( p != nullptr )
					{
						p->UnsetSentSourceRequest();
						p->UnsetSentProductsRequest();
						p->SetProduced( 0 );
						p->UnsetTurn();
						p->UnsetProd();
						p->SetIncome( p->GetMoney() - p->GetOldMoney() );
						p->SetOldMoney( p->GetMoney() );

						send_newturn_message(GetBanker().GetTurnNumber(), p->IsBot(), GetBanker().GetCurrentMarketState(), p->GetFd(), p->GetAddr());
					}
				}
			}
		}
	} /* end while ( 1 ) */
}

#endif
