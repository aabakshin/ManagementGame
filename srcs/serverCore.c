/* Файл реализации модуля serverCore */

#ifndef SERVER_CORE_C
#define SERVER_CORE_C


#include "../includes/serverCore.h"
#include "../includes/MGLib.h"
#include "../includes/CommandsHandler.h"


enum
{
	LISTEN_QUEUE_LEN			=						  5
};


// Описаны в модуле Banker
extern double amount_multiplier_table[MARKET_LEVEL_NUMBER][2];
extern int price_table[MARKET_LEVEL_NUMBER][2];
extern const int states_market_chance[MARKET_LEVEL_NUMBER][MARKET_LEVEL_NUMBER];

// Описаны в модуле MGLib
extern const char* info_game_messages[];
extern const char* error_game_messages[];

// Описаны в модуле CommandsHandler
extern char* command_tokens[3];
extern int cmd_tokens_amount;

// Описаны в модуле Banker
extern int send_victory_message(Banker* b, Player* p);
extern int send_auctionreport_message(Banker* b, Player* p, AuctionReport* ar);
extern int send_lostlobbyplayer_message(Banker* b, Player* p);
extern int send_lostaliveplayer_message(int left_pl_num, Banker* b, Player* p);
extern int send_produced_message(Banker* b, Player* p);
extern int send_startinseconds_message(Banker* b, Player* p);
extern int send_gamestarted_message(Banker* b, Player* p);
extern int send_startgameinfo_message(Banker* b, Player* p);
extern int send_startcancelled_message(Banker* b, Player* p);
extern int send_newplayerconnect_message(Banker* b, Player* p);
extern int send_gamenotstarted_message(Banker* b, Player* p);
extern int send_newturn_message(Banker* b, Player* p);


static int is_correct_identity_msg(const char* identity_msg);
static int server_close_connection(Banker* b, fd_set* readfds, Player* p);
static void server_stop(Banker* banker, fd_set* readfds, int forcely);
static int send_gamealreadystarted_message( int cs, const char* address_buffer );
static int send_serverfull_message( int cs, const char* address_buffer );
static int concat_addr_port(char* address_buffer, const char* service_buffer);
static int show_sending_message(const char* send_buf, int mes_len, const char* ip, int wc);
static int server_fill_readfds(Banker* b, int ls);



int server_quit_player(Banker* b, int i, fd_set* readfds, Player* p);
int send_message(int fd, const char** message_tokens, int tokens_amount, const char* ip);



/* Список сообщений для идентификации бот-клиента */
static const char* bot_identity_messages[] = {
				"./bot_mg_debug4",
				"./bot_mg_debug4\n",
				"./bot_mg_release4",
				"./bot_mg_release4\n",
				NULL
};

/* Флаг срабатывания таймера */
static int alrm_flag = 0;

/* Флаг завершения работы программы */
static int exit_flag = 0;

/* Номер полученного сигнала */
static int sig_number = 0;

/* Флаг установки таймера */
static int timer_set = 0;

/* Счётчик отправленных сообщений сервером */
static int log_info_count = 0;

/* мн-во сокетов, с которых можно считывать данные */
static fd_set readfds;

/* Используется в select как макс. номер прослушиваемых сокетов */
static int max_fd = 0;



/* Ф-я-обработчик сигнала SIGALRM */
void alrm_handler(int sig_no)
{
	int save_errno = errno;
	signal(SIGALRM, alrm_handler);

	sig_number = sig_no;
	alrm_flag = 1;
	alarm(0);

	errno = save_errno;
}

/* Ф-я-обработчик сигнала SIGINT */
void exit_handler(int sig_no)
{
	int save_errno = errno;
	signal(SIGINT, exit_handler);

	sig_number = sig_no;
	exit_flag = 1;

	errno = save_errno;
}

static int show_sending_message(const char* send_buf, int mes_len, const char* ip, int wc)
{
	if (
						( send_buf == NULL )			||
						( *send_buf == '\n' )			||
						( *send_buf == '\0' )			||
						( mes_len < 0 )					||
						( ip == NULL )					||
						( *ip == '\0' )					||
						( *ip == '\n' )					||
						( wc < 0 )
		)
		return 0;

	++log_info_count;

	printf("\n==================== (%d) ====================\n", log_info_count);

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
					"==================== (%d) ====================\n\n", send_buf, ip, wc, mes_len, log_info_count);

	return 1;
}

int send_message(int fd, const char** message_tokens, int tokens_amount, const char* ip)
{
	if (
					( fd < 0 )							||
					( message_tokens == NULL )			||
					( *message_tokens == NULL )			||
					( tokens_amount < 1 )				||
					( ip == NULL )
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

static int is_correct_identity_msg(const char* identity_msg)
{
	for ( int i = 0; bot_identity_messages[i] != NULL; ++i )
		if ( strcmp(identity_msg, bot_identity_messages[i]) == 0 )
			return 1;

	return 0;
}

static int server_close_connection(Banker* b, fd_set* readfds, Player* p)
{
	if (
				( b == NULL )			||
				( p == NULL )
		)
		return 0;


	int p_fd = p->fd;
	char ip[100] = { 0 };
	strcpy(ip, p->ip);


	clean_player_record(b, p);
	close(p_fd);
	FD_CLR(p_fd, readfds);
	printf("[+] Lost connection from [%s]\n", ip);

	return 1;
}

static void server_stop(Banker* banker, fd_set* readfds, int forcely)
{
	if (
			( banker == NULL )			||
			( readfds == NULL )
		)
		exit(1);

	if ( forcely )
		printf("%s", "\n\n========== SERVER IS STOPPING WORK FORCELY ==========\n");

	mr_clear(&banker->products_requests);
	banker->products_requests = NULL;
	printf("%s","[+] Products request list has cleared\n");

	mr_clear(&banker->sources_requests);
	banker->sources_requests = NULL;
	printf("%s", "[+] Sources request list has cleared\n");

	int i;
	for ( i = 0; i < MAX_PLAYERS; i++ )
	{
		Player* p = banker->pl_array[i];
		if ( p != NULL )
		{
			server_close_connection(banker, readfds, p);
			banker->pl_array[i] = NULL;
		}
	}

	if ( forcely )
		printf("%s", "========== SERVER IS STOPPING WORK FORCELY ==========\n\n");

	exit(0);
}

/* инициализация работы сервера */
int server_init(char* port)
{
	printf("%s\n", "Configuring local address...");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo* bind_address;
	if ( getaddrinfo(NULL, port, &hints, &bind_address) != 0 )
	{
		fprintf(stderr, "An error has occured with \"getaddrinfo\" (errno code = %d)\n", errno);
		return -1;
	}

	printf("%s\n", "Creating listening socket...");
	int ls = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	if ( ls == -1 )
	{
		fprintf(stderr, "socket() failed. (errno code = %d)\n", errno);
		return -1;
	}

	int opt = 1;
	setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	printf("%s\n", "Binding socket to address...");
	if ( bind(ls, bind_address->ai_addr, bind_address->ai_addrlen) )
	{
		fprintf(stderr, "bind() failed. (errno code = %d)\n", errno);
		return -1;
	}
	freeaddrinfo(bind_address);

	printf("%s\n", "Listening...");
	if ( listen(ls, LISTEN_QUEUE_LEN) < 0 )
	{
		fprintf(stderr, "listen() failed. (errno code = %d\n)", errno);
		return -1;
	}
	printf("Waiting connections to %s port...\n", port);

	return ls;
}

static int send_gamealreadystarted_message(int cs, const char* address_buffer)
{
	if (
					( cs < 0 )							||
					( address_buffer == NULL )
		)
		return 0;

	const char* message[] =
	{
				info_game_messages[GAME_ALREADY_STARTED],
				NULL
	};
	send_message(cs, message, 1, address_buffer);

	return 1;
}

static int send_serverfull_message(int cs, const char* address_buffer)
{
	if (
					( cs < 0 )							||
					( address_buffer == NULL )
		)
		return 0;

	const char* message[] =
	{
				info_game_messages[SERVER_FULL],
				NULL
	};
	send_message(cs, message, 1, address_buffer);

	return 1;
}

static int send_cmdinternalerror_message( int cs, const char* address_buffer )
{
	if (
					( cs < 0 )							||
					( address_buffer == NULL )
		)
		return 0;

	const char* message[] =
	{
				error_game_messages[COMMAND_INTERNAL_ERROR],
				NULL
	};
	send_message(cs, message, 1, address_buffer);

	return 1;
}

static int send_cmdincorrectargsnum_message(int fd, const char* ip)
{
	if (
						( fd < 0 )			||
						( ip == NULL )		||
						( *ip == '\0' )
		)
		return 0;

	const char* error_message[] =
	{
				error_game_messages[COMMAND_INCORRECT_ARGUMENTS_NUM],
				NULL
	};

	send_message(fd, error_message, 1, ip);

	return 1;
}

static int send_unknowncmd_message(int fd, const char* ip)
{
	if (
						( fd < 0 )			||
						( ip == NULL )		||
						( *ip == '\0' )
		)
		return 0;

	const char* unknown_cmd_message[] =
	{
				info_game_messages[UNKNOWN_COMMAND],
				NULL
	};
	send_message(fd, unknown_cmd_message, 1, ip);

	return 1;
}

static int send_wfnt_message(Banker* b, Player* p)
{
	if (
				( b == NULL )				||
				( p == NULL )
		)
		return 0;

	char w_y_p_a[10];
	int wait_yet_players_amount = b->alive_players - b->ready_players;
	itoa(wait_yet_players_amount, w_y_p_a, 9);

	int tokns_amnt = 2;
	char* mes_tokens[] =
	{
			(char*)info_game_messages[WAIT_FOR_NEXT_TURN],
			w_y_p_a,
			NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int concat_addr_port(char* address_buffer, const char* service_buffer)
{
	if (
				( address_buffer == NULL )			||
				( service_buffer == NULL )
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

int server_quit_player(Banker* b, int i, fd_set* readfds, Player* p)
{
	if (
					( b == NULL )							||
					( (i < 0) && (i >= MAX_PLAYERS) )		||
					( readfds == NULL )						||
					( p == NULL )
		)
		return 0;

	int left_pl_num = p->number;
	server_close_connection(b, readfds, p);
	b->pl_array[i] = NULL;

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		Player* p = b->pl_array[i];
		if ( p != NULL )
		{
			if ( !b->game_started )
			{
				send_lostlobbyplayer_message(b, p);
			}
			else
			{
				if ( b->alive_players == 1 )
				{
					send_victory_message(b, p);
					printf("\n\n<<<<< GAME IS FINISHED. PLAYER #%d IS WINNER! >>>>>\n\n", p->number);
					server_stop(b, readfds, 0);
				}
				else
				{
					send_lostaliveplayer_message(left_pl_num, b, p);
				}
			}
		}
	}

	return 1;
}

static int server_fill_readfds(Banker* b, int ls)
{
	if (
					( b == NULL )		||
					( ls < 0 )
		)
		return 0;

	FD_ZERO(&readfds);

	FD_SET(ls, &readfds);

	max_fd = ls;

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		Player* p = b->pl_array[i];
		if ( p != NULL )
		{
			FD_SET( p->fd, &readfds );
			if ( p->fd > max_fd )
				max_fd = p->fd;
		}
	}

	return 1;
}

/* запуск главного игрового цикла */
int server_run(Banker* banker, int ls)
{
	signal(SIGINT, exit_handler);
	signal(SIGALRM, alrm_handler);

	srand(time(0));


	while ( 1 )
	{
		if ( exit_flag )
			server_stop(banker, &readfds, 1);

		server_fill_readfds(banker, ls);


		if ( !banker->game_started )
		{
			if ( !timer_set && (banker->lobby_players >= MIN_PLAYERS_TO_START) && (banker->lobby_players < MAX_PLAYERS) )
			{
				alarm(TIME_TO_START);
				timer_set = 1;

				for ( int i = 0; i < MAX_PLAYERS; i++ )
				{
					Player* p = banker->pl_array[i];
					if ( p != NULL )
						send_startinseconds_message(banker, p);
				}
			}
			else if ( banker->lobby_players == MAX_PLAYERS )
			{
				alarm(0);
				alrm_flag = 0;
				timer_set = 0;

				/*FORCE START THE GAME*/
				banker->game_started = 1;
				/*FORCE START THE GAME*/

				for ( int i = 0; i < MAX_PLAYERS; i++ )
				{
					Player* p = banker->pl_array[i];
					if ( p != NULL )
						send_gamestarted_message(banker, p);
				}

				continue;
			}
		}
		else
		{
			if ( !banker->players_prepared )
			{
				banker->players_prepared = 1;

				banker->alive_players = banker->lobby_players;
				banker->lobby_players = 0;
				banker->turn_number = 1;

				banker->cur_market_lvl = START_MARKET_LEVEL;
				banker->cur_market_state->source_amount = amount_multiplier_table[START_MARKET_LEVEL-1][0]*banker->alive_players;
				banker->cur_market_state->min_source_price = price_table[START_MARKET_LEVEL-1][0];
				banker->cur_market_state->product_amount = amount_multiplier_table[START_MARKET_LEVEL-1][1]*banker->alive_players;
				banker->cur_market_state->max_product_price = price_table[START_MARKET_LEVEL-1][1];

				int i;
				for ( i = 0; i < MAX_PLAYERS; i++ )
				{
					Player* p = banker->pl_array[i];
					if ( p != NULL )
					{
						p->money = START_MONEY;
						p->sources = START_SOURCES;
						p->products = START_PRODUCTS;
						p->wait_factories = START_FACTORIES;
						p->old_money = p->money;

						//printf("[%s]: p->is_bot: %s\n", p->ip, (p->is_bot) ? "TRUE" : "FALSE");

						send_startgameinfo_message(banker, p);
					}
				}
			}

			check_producing_on_turn(banker);
		}


		int res = select(max_fd+1, &readfds, NULL, NULL, NULL);
		if ( res == -1 )
		{
			if ( errno == EINTR )
			{
				if ( (sig_number == SIGALRM) && alrm_flag && (!banker->game_started) )
				{
					alrm_flag = 0;
					timer_set = 0;

					if ( banker->lobby_players < MIN_PLAYERS_TO_START )
					{
						for ( int i = 0; i < MAX_PLAYERS; i++ )
						{
							Player* p = banker->pl_array[i];
							if ( p != NULL )
								send_startcancelled_message(banker, p);
						}
					}
					else
					{
						if ( banker->lobby_players < MAX_PLAYERS )
						{
							/* START THE GAME */
							banker->game_started = 1;
							/* START THE GAME */

							for ( int i = 0; i < MAX_PLAYERS; i++ )
							{
								Player* p = banker->pl_array[i];
								if ( p != NULL )
									send_gamestarted_message(banker, p);
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
		/* end if (res == -1) */


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


			if ( banker->game_started )
			{
				send_gamealreadystarted_message(cs, address_buffer);
				close(cs);
				printf("Lost connection from (%s)\n", address_buffer);
			}
			else
			{
				int i;
				for ( i = 0; i < MAX_PLAYERS; i++ )
				{
					if ( banker->pl_array[i] == NULL )
						break;
				}

				if ( i >= MAX_PLAYERS )
				{
					send_serverfull_message(cs, address_buffer);
					close(cs);
					printf("Lost connection from (%s)\n", address_buffer);
				}
				else
				{
					Player* p = malloc(sizeof(struct Player));
					if ( p == NULL )
					{
						send_cmdinternalerror_message(cs, address_buffer);
						close(cs);
						printf("Lost connection from (%s)\n", address_buffer);
					}
					else
					{
						p->fd = cs;
						p->number = i+1;

						int j, k = 0;
						for ( j = 0; address_buffer[j]; j++, k++ )
							p->ip[k] = address_buffer[j];
						p->ip[k] = '\0';

						p->money = 0;
						p->old_money = 0;
						p->income = 0;
						p->sources = 0;
						p->products = 0;
						p->wait_factories = 0;
						p->work_factories = 0;
						p->build_factories = 0;
						p->build_list = NULL;
						p->is_turn = 0;
						p->is_prod = 0;
						p->is_ident_message_recv = 0;
						p->produced_on_turn = 0;
						p->sent_source_request = 0;
						p->sent_products_request = 0;

						banker->pl_array[i] = p;
						banker->lobby_players++;


						for ( i = 0; i < MAX_PLAYERS; i++ )
						{
							Player* p = banker->pl_array[i];
							if ( p != NULL )
								send_newplayerconnect_message(banker, p);
						}
					}
				}
			}
		}
		/* end if FD_ISSET */


		char read_buf[BUFSIZE];
		for ( int i = 0; i < MAX_PLAYERS; i++ )
		{
			Player* p = banker->pl_array[i];
			if ( p != NULL )
			{
				if ( FD_ISSET(p->fd, &readfds) )
				{
					int rc = readline(p->fd, read_buf, BUFSIZE-1);
					if ( rc > 0 )
					{
						printf("Received %d bytes from [%s]\n", rc, p->ip);

						read_buf[rc] = '\0';
						cut_str(read_buf, rc, '\n');

						int size = strlen(read_buf)+1;
						delete_spaces(read_buf, &size);

						if ( !banker->game_started )
						{
							if ( !p->is_ident_message_recv )
							{
								if ( is_correct_identity_msg(read_buf) )
									p->is_bot = 1;
								else
									p->is_bot = 0;

								p->is_ident_message_recv = 1;
							}
							else
							{
								send_gamenotstarted_message(banker, p);
							}
							continue;
						}


						// Обработка данных от игрока, когда игра началась
						make_cmd_tokens(read_buf, command_tokens, &cmd_tokens_amount, 3);
						if ( cmd_tokens_amount < 1 )
							continue;

						int result_code = process_command(banker, p, (const char**)command_tokens, cmd_tokens_amount);
						switch ( result_code )
						{
							case QUIT_COMMAND_NUM:
								server_quit_player(banker, i, &readfds, p);
								break;
							case INTERNAL_COMMAND_ERROR:
								send_cmdinternalerror_message(p->fd, p->ip);
								break;
							case INCORRECT_ARGS_COMMAND_ERROR:
								send_cmdincorrectargsnum_message(p->fd, p->ip);
								break;
							case UNKNOWN_COMMAND_ERROR:
								send_unknowncmd_message(p->fd, p->ip);
								break;
							case WFNT_COMMAND_ERROR:
								send_wfnt_message(banker, p);
						}
					} /* end of if (rc > 0) */
					else
					{
						server_quit_player(banker, i, &readfds, p);
					}
				} /* end if FD_ISSET */
			} /* end server_banker.pl_array[i] != NULL */
		} /* end for */


		// Действия, происходящие в конце игрового месяца
		if ( banker->game_started )
		{
			if ( banker->ready_players == banker->alive_players )
			{
				check_building_factories(banker, &readfds);

				AuctionReport ar[banker->ready_players];

				for ( int j = 0, i = 0; i < MAX_PLAYERS; i++ )
				{
					Player* p = banker->pl_array[i];
					if ( p != NULL )
					{
						ar[j].turn = banker->turn_number;
						ar[j].p = p;
						ar[j].sold_price = 0;
						ar[j].sold_sources = 0;
						ar[j].bought_price = 0;
						ar[j].bought_prods = 0;
						j++;
						if ( j >= banker->ready_players )
							break;
					}
				}

				MarketRequest* new_source_request = start_auction(banker, ar, 0);			/* 0 - аукцион сырья */
				MarketRequest* new_prod_request = start_auction(banker, ar, 1);			/* 1 - аукцион продукции */

				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					Player* p = banker->pl_array[i];
					if ( p != NULL )
						send_auctionreport_message(banker, p, ar);
				}

				pay_charges(banker, &readfds, ar, &new_source_request, &new_prod_request);
				report_on_turn(banker, ar, new_source_request, new_prod_request);

				if ( new_source_request != NULL)
					mr_clear(&new_source_request);
				if ( new_prod_request != NULL )
					mr_clear(&new_prod_request);

				change_market_state(banker);

				banker->turn_number++;
				banker->ready_players = 0;

				for ( int i = 0; i < MAX_PLAYERS; i++ )
				{
					Player* p = banker->pl_array[i];
					if ( p != NULL )
					{
						p->sent_source_request = 0;
						p->sent_products_request = 0;
						p->produced_on_turn = 0;
						p->is_turn = 0;
						p->is_prod = 0;
						p->income = p->money - p->old_money;
						p->old_money = p->money;

						send_newturn_message(banker, p);
					}
				}
			} /* end if server_banker.ready_players == alive_players */
		} /* end if server_banker.game_started */
	} /* end while ( 1 ) */
}

#endif
