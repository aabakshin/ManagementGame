/* Файл реализации модуля serverCore */

#ifndef SERVER_CORE_C
#define SERVER_CORE_C


#include "../includes/serverCore.h"
#include "../includes/MGLib.h"



enum
{
	LISTEN_QUEUE_LEN			=						  5
};


/* Описаны в модуле Banker */
extern double amount_multiplier_table[MARKET_LEVEL_NUMBER][2];
extern int price_table[MARKET_LEVEL_NUMBER][2];
extern const int states_market_chance[MARKET_LEVEL_NUMBER][MARKET_LEVEL_NUMBER];

// Описаны в модуле MGLib
extern const char* info_game_messages[];
extern const char* error_game_messages[];


// проверяет, является ли отправитель ботом, а не игроком
static int is_correct_identity_msg(const char* identity_msg);

// закрыть соединение с определённым игроком
static int server_close_connection(Banker* b, fd_set* readfds, Player* p);

// остановка сервера
static void server_stop(Banker* banker, fd_set* readfds, int forcely);


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

/* Флаг о готовности всех игроков к игре */
static int players_prepared = 0;

/* Флаг установки таймера */
static int timer_set = 0;

/* Счётчик событий в логе сервера */
//static int log_info_count = 0;

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

// Banker module
static int send_produced_message(Banker* b, Player* p)
{
	if (
			( b == NULL )			||
			( p == NULL )
		)
		return 0;

	char am_prd[10];
	itoa(p->produced_on_turn, am_prd, 9);

	int tokns_amnt = 2;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[PRODUCED],
				am_prd,
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

// Banker module
static int send_startinseconds_message(Banker* b, Player* p)
{
	if (
			( b == NULL )			||
			( p == NULL )
		)
		return 0;

	char tts[10];
	itoa(TIME_TO_START, tts, 9);

	int tokns_amnt = 2;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[STARTINSECONDS],
				tts,
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

// Banker module
static int send_gamestarted_message(Banker* b, Player* p)
{
	if (
			( b == NULL )					||
			( p == NULL )
		)
		return 0;

	const char* message[] =
	{
				info_game_messages[GAME_STARTED],
				NULL
	};

	send_message(p->fd, message, 1, p->ip);

	return 1;
}

// Banker module
static int send_startgameinfo_message(Banker* b, Player* p)
{
	if (
			( b == NULL )					||
			( p == NULL )
		)
		return 0;

	char p_num[10];
	itoa(p->number, p_num, 9);

	char ap[10];
	itoa(b->alive_players, ap, 9);

	char tn[10];
	itoa(b->turn_number, tn, 9);

	char p_money[20];
	itoa(p->money, p_money, 19);

	char p_sources[10];
	itoa(p->sources, p_sources, 9);

	char p_products[10];
	itoa(p->products, p_products, 9);

	char p_wf[10];
	itoa(p->wait_factories, p_wf, 9);

	char p_wrkf[10];
	itoa(p->work_factories, p_wrkf, 9);

	char p_bf[10];
	itoa(p->build_factories, p_bf, 9);

	char sa[10];
	char msp[10];
	char pa[10];
	char mpp[10];

	if ( p->is_bot )
	{
		itoa(b->cur_market_state->source_amount, sa, 9);
		itoa(b->cur_market_state->min_source_price, msp, 9);
		itoa(b->cur_market_state->product_amount, pa, 9);
		itoa(b->cur_market_state->max_product_price, mpp, 9);
	}

	int tokns_amnt = (p->is_bot) ? 14 : 10;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[STARTING_GAME_INFORMATION],
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
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

// Banker module
static int send_startcancelled_message(Banker* b, Player* p)
{
	if (
					( b == NULL )			||
					( p == NULL )
		)
		return 0;


	const char* mes_tokens[] =
	{
				info_game_messages[STARTCANCELLED],
				NULL
	};
	send_message(p->fd, mes_tokens, 1, p->ip);

	return 1;
}

// Server module
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

// Server module
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

// Server module
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

static int send_newplayerconnect_message(Banker* b, Player* p)
{
	if (
					( b == NULL )			||
					( p == NULL )
		)
		return 0;

	char lp_buf[10];
	itoa(b->lobby_players, lp_buf, 9);

	char max_pl_buf[10];
	itoa(MAX_PLAYERS, max_pl_buf, 9);

	int tokns_amnt = 3;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[NEW_PLAYER_CONNECT],
				lp_buf,
				max_pl_buf,
				NULL
	};
	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int send_gamenotstarted_message(Banker* b, Player* p)
{
	if (
					( b == NULL )			||
					( p == NULL )
		)
		return 0;

	const char* message[] =
	{
				info_game_messages[GAME_NOT_STARTED],
				NULL
	};
	send_message(p->fd, message, 1, p->ip);

	return 1;
}

static int send_lostlobbyplayer_message(Banker* b, Player* p)
{
	if (
					( b == NULL )			||
					( p == NULL )
		)
		return 0;

	char lp_buf[10];
	itoa(b->lobby_players, lp_buf, 9);

	char max_pl_buf[10];
	itoa(MAX_PLAYERS, max_pl_buf, 9);

	int tokns_amnt = 3;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[LOST_LOBBY_PLAYER],
				lp_buf,
				max_pl_buf,
				NULL
	};

	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int send_lostaliveplayer_message(int left_pl_num, Banker* b, Player* p)
{
	if (
					( b == NULL )			||
					( p == NULL )
		)
		return 0;


	char ap_buf[10];
	itoa(b->alive_players, ap_buf, 9);

	char left_p_num_buf[10];
	itoa(left_pl_num, left_p_num_buf, 9);

	int tokns_amnt = 3;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[LOST_ALIVE_PLAYER],
				ap_buf,
				left_p_num_buf,
				NULL
	};


	send_message(p->fd, (const char**)mes_tokens, tokns_amnt, p->ip);

	return 1;
}

static int send_newturn_message(Banker* b, Player* p)
{
	if (
					( b == NULL )			||
					( p == NULL )
		)
		return 0;

	char nt[10];
	itoa(b->turn_number, nt, 9);

	char sa[10];
	char msp[10];
	char pa[10];
	char mpp[10];

	if ( p->is_bot )
	{
		itoa(b->cur_market_state->source_amount, sa, 9);
		itoa(b->cur_market_state->min_source_price, msp, 9);
		itoa(b->cur_market_state->product_amount, pa, 9);
		itoa(b->cur_market_state->max_product_price, mpp, 9);
	}

	int tokns_amnt = (p->is_bot) ? 6 : 2;
	char* mes_tokens[] =
	{
				(char*)info_game_messages[NEW_TURN],
				nt,
				sa,
				msp,
				pa,
				mpp,
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

static int server_quit_player(Banker* b, int i, fd_set* readfds, Player* p)
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
					int winner_number = last_man_stand(b);
					printf("\n\n<<<<< GAME IS FINISHED. PLAYER #%d IS WINNER! >>>>>\n\n", winner_number);
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

/* запуск главного игрового цикла */
int server_run(Banker* banker, int ls)
{
	signal(SIGINT, exit_handler);
	signal(SIGALRM, alrm_handler);

	srand(time(0));


	while ( 1 )
	{
		fd_set readfds;

		if ( exit_flag )
			server_stop(banker, &readfds, 1);

		FD_ZERO(&readfds);
		FD_SET(ls, &readfds);
		int max_fd = ls;

		int i;
		for ( i = 0; i < MAX_PLAYERS; i++ )
		{
			Player* p = banker->pl_array[i];
			if ( p != NULL )
			{
				if ( banker->game_started && (!p->is_prod) )
				{
					int amount_products_prev = p->products;
					int w_f = p->work_factories;

					while ( p->work_factories > 0 )
					{
						p->work_factories--;
						p->products++;
						p->wait_factories++;
					}

					if ( w_f > 0 )
					{
						p->produced_on_turn = p->products - amount_products_prev;
						send_produced_message(banker, p);
					}
				}

				int fd = p->fd;
				FD_SET( fd, &readfds );
				if ( fd > max_fd )
					max_fd = fd;
			}
		}
		/* end for */

		if ( !banker->game_started )
		{
			if ( !timer_set && (banker->lobby_players >= MIN_PLAYERS_TO_START) && (banker->lobby_players < MAX_PLAYERS) )
			{
				alarm(TIME_TO_START);
				timer_set = 1;

				for ( i = 0; i < MAX_PLAYERS; i++ )
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
			if ( !players_prepared )
			{
				players_prepared = 1;

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
			} /* end if (!players_prepared) */
		} /* end server_banker.game_started */


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
		for ( i = 0; i < MAX_PLAYERS; i++ )
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
						char* command_tokens[3] = {
								NULL,
								NULL,
								NULL
						};
						int tokens_amnt = 0;
						char* istr = strtok(read_buf, " ");
						int j = 0;
						while ( (istr != NULL) && ( j < 3 ) )
						{
							command_tokens[j] = istr;
							j++;
							istr = strtok(NULL, " ");
						}
						tokens_amnt = j;

						if ( tokens_amnt < 1 )
							continue;


						int result_code = process_command(banker, p, (const char**)command_tokens, tokens_amnt);
						switch ( result_code )
						{
							case QUIT_COMMAND_NUM:
								server_quit_player(banker, i, &readfds, p);
								break;
							case ERROR_COMMAND_NUM:
								send_cmdinternalerror_message(p->fd, p->ip);
						}
					} /* end of if (rc > 0) */
					else
					{
						server_quit_player(banker, i, &readfds, p);
					}
				} /* end if FD_ISSET */
			} /* end server_banker.pl_array[i] != NULL */
		} /* end for */

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

				make_auction_report(banker, ar);
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
