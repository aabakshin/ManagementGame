/* Файл реализации модуля botCore */

#ifndef BOT_CORE_CPP
#define BOT_CORE_CPP

#include "../includes/botCore.hpp"
#include "../includes/MainInfo.hpp"

/* Определена в модуле bot_mg */
extern MainInfo main_info;

/* Флаг установлен, если ИИ отправил на сервер команду turn */
static int turn_flag = 0;

/* Стандартная процедура установка TCP-соединения с сервером */
int bot_connect(const char* addr, const char* port)
{
	printf("%s\n", "Configuring remote address...");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo* peer_address;

	if ( getaddrinfo(addr, port, &hints, &peer_address) )
	{
		freeaddrinfo(peer_address);
		fprintf(stderr, "getaddrinfo() failed. (%d)\n", errno);
		return -1;
	}

	printf("%s", "Remote address is: ");
	char address_buffer[ADDRESS_BUFFER_SIZE];
	char service_buffer[ADDRESS_BUFFER_SIZE];

	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
	printf("%s:%s\n", address_buffer, service_buffer);
	
	printf("%s\n", "Creating socket...");
	int socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);

	if ( socket_peer < 0 )
	{
		freeaddrinfo(peer_address);
		fprintf(stderr, "socket() failed. (%d)\n", errno);
		return -1;
	}
	
	printf("%s\n", "Connecting...");
	if ( connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen) )
	{
		freeaddrinfo(peer_address);
		fprintf(stderr, "connect() failed. (%d)\n", errno);
		return -1;
	}
	freeaddrinfo(peer_address);
	printf("%s\n", "Connected.");

	return socket_peer;
}

/* Вывод полей данных структуры искусственного игрока */
static int print_debug_main_info(PlayerInfo* pi, const char* command_name)
{
	if ( (pi == NULL) || (command_name == NULL) )
	{
		fprintf(stderr, "%s", "\n[botCore]: In function \"print_debug_main_info\": \"pi\" OR \"command_name\" is NULL\n");
		return 0;
	}
	
	int num = main_info.last_player_num;

	printf("\n\n<<<<< [botCore]: \"%s\" DEBUG INFO >>>>>\n\n", command_name);
	printf(
		   "main_info.pid = %d\n"
		   "main_info.fd = %d\n"
		   "main_info.execute_script = %d\n"
		   "main_info.my_id = %d\n"
		   "main_info.turn = %d\n"
		   "main_info.total_players = %d\n"
		   "main_info.alive_players = %d\n"
		   "main_info.cur_sources_buy = %d\n"
		   "main_info.cur_sources_min_price = %d\n"
		   "main_info.cur_products_sell = %d\n"
		   "main_info.cur_products_max_price = %d\n"
		   "main_info.last_player_num = %d\n"
		   "main_info.p_info[%d]->player_num = %d\n"
		   "main_info.p_info[%d]->money = %d\n"
		   "main_info.p_info[%d]->income = %d\n"
		   "main_info.p_info[%d]->raw = %d\n"
		   "main_info.p_info[%d]->prod = %d\n"
		   "main_info.p_info[%d]->wait_fact = %d\n"
		   "main_info.p_info[%d]->work_fact = %d\n"
		   "main_info.p_info[%d]->build_fact = %d\n"
		   "main_info.p_info[%d]->manufactured = %d\n"
		   "main_info.p_info[%d]->res_raw_sold = %d\n"
		   "main_info.p_info[%d]->res_raw_price = %d\n"
		   "main_info.p_info[%d]->res_prod_bought = %d\n"
		   "main_info.p_info[%d]->res_prod_price = %d\n",
		   main_info.pid,
		   main_info.fd,
		   main_info.execute_script,
		   main_info.my_id,
		   main_info.turn,
		   main_info.total_players,
		   main_info.alive_players,
		   main_info.cur_sources_buy,
		   main_info.cur_sources_min_price,
		   main_info.cur_products_sell,
		   main_info.cur_products_max_price,
		   num,
		   num-1, pi->player_num,
		   num-1, pi->money,
		   num-1, pi->income,
		   num-1, pi->raw,
		   num-1, pi->prod,
		   num-1, pi->wait_fact,
		   num-1, pi->work_fact,
		   num-1, pi->build_fact,
		   num-1, pi->manufactured,
		   num-1, pi->res_raw_sold,
		   num-1, pi->res_raw_price,
		   num-1, pi->res_prod_bought,
		   num-1, pi->res_prod_price
		 );
	printf("\n<<<<< [botCore] \"%s\" DEBUG INFO >>>>>\n\n", command_name);

	return 1;
}

/* Ф-я-обработчк пришедших с сервера ответов на запросы игрока */
int bot_check_server_response(char* buffer)
{
	if ( buffer == NULL )
		return 0;

	if ( (*buffer == '\n') || (*buffer == '\0') )
		return 0;

	char* tokens[100];
	int i;
	for ( i = 0; i < 100; i++ )
		tokens[i] = NULL;

	i = 0;
	char* istr = strtok(buffer, "|");
	while ( istr != NULL )
	{
		tokens[i] = istr;
		i++;
		istr = strtok(NULL, "|");
	}
	int command_tokens_amount = i;


	if ( strcmp(tokens[0], "*INFO_MESSAGE") == 0 )
	{
		if ( strcmp(tokens[1], "PLAYER_COMMAND_NOT_FOUND") == 0 )
		{
			printf("%s", "\n\t[!] Player with that specific number is not found!\n");

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "PLAYER_COMMAND_NOT_FOUND");

		}
		else if ( strcmp(tokens[1], "PLAYER_COMMAND") == 0 )
		{
			/*
			const char* specs[] = {
								"money: ",
								"sources: ",
								"products: ",
								"wait factories: ",
								"work factories: ",
								"build factories: ",
								NULL
			};
			int specs_amount = 6;

			printf("\nMain information about Player #%s:\n", tokens[2]);

			int i, j;
			for ( j = 3, i = 3; j < 3+specs_amount; j++, i++ )
			{
				if ( strcmp(specs[j-3], "money: ") == 0 )
				{
					printf(" <%s %s> ( ", specs[j-3], tokens[i]);
					if ( tokens[i+1][0] != '-' )
						putchar('+');
					printf("%s )\n", tokens[i+1]);
					i++;
				}
				else
					printf(" <%s %s>\n", specs[j-3], tokens[i]);
			}
			putchar('\n');
			*/

			main_info.last_player_num = atoi(tokens[2]);
			int num = main_info.last_player_num;
			
			PlayerInfo* pi = main_info.p_info[num-1];
			if ( pi == NULL )
			{
				main_info.p_info[num-1] = new PlayerInfo;
				pi = main_info.p_info[num-1];
				if ( !pi )
				{
					fprintf(stderr, "%s", "\n[botCore]: In function \"bot_check_response\": in token PLAYER_COMMAND: memory error\n");
					return 0;
				}
				pi->res_raw_sold = 0;
				pi->res_raw_price = 0;
				pi->res_prod_bought = 0;
				pi->res_prod_price = 0;
			}

			pi->player_num = num;
			pi->money = atoi(tokens[3]);
			pi->income = atoi(tokens[4]);
			pi->raw = atoi(tokens[5]);
			pi->prod = atoi(tokens[6]);
			pi->wait_fact = atoi(tokens[7]);
			pi->work_fact = atoi(tokens[8]);
			pi->build_fact = atoi(tokens[9]);
			pi->manufactured = atoi(tokens[10]);
			
			print_debug_main_info(pi, "PLAYER_COMMAND");
		}
		else if ( strcmp(tokens[1], "PROD_COMMAND_SUCCESS") == 0 )
		{
			printf("%s", "\nYou are producing a product\n"
						 "Result will be on next turn\n\n");
	
			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "PROD_COMMAND_SUCCESS");
		}
		else if ( strcmp(tokens[1], "PROD_COMMAND_NO_FACTORIES") == 0 )
		{
			printf("%s", "\n[!] You don't have on current turn any waiting factories!\n\n");
	
			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "PROD_COMMAND_NO_FACTORIES");
		}
		else if ( strcmp(tokens[1], "PROD_COMMAND_NO_MONEY") == 0 )
		{
			printf("%s", "\n[!] You don't have enough money to pay for producing a product!\n\n");
	
			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "PROD_COMMAND_NO_MONEY");
		}
		else if ( strcmp(tokens[1], "PROD_COMMAND_NO_SOURCE") == 0 )
		{
			printf("%s", "\n[!] You don't have enough sources to produce a product!\n\n");

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "PROD_COMMAND_NO_SOURCE");
		}
		else if ( strcmp(tokens[1], "BUILD_COMMAND_SUCCESS") == 0 )
		{
			printf("%s", "\nYou have successfully started to build a new factory.\n\n");

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "BUILD_COMMAND_SUCCESS");
		}
		else if ( strcmp(tokens[1], "BUILD_COMMAND_NO_MONEY") == 0 )
		{
			printf("%s", "\n[!] You don't have enough money to start building a new factory!\n\n");
	
			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "BUILD_COMMAND_NO_MONEY");
		}
		else if ( strcmp(tokens[1], "BUY_COMMAND_NO_MONEY") == 0 )
		{
			printf("%s", "\n[!] You don't have enough money to do request to source auction!\n\n");	

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "BUY_COMMAND_NO_MONEY");
		}
		else if ( strcmp(tokens[1], "BUY_COMMAND_ALREADY_SENT") == 0 )
		{
			printf("%s", "\n[!] You have already done the request to source auction!\n\n");

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "BUY_COMMAND_ALREADY_SENT");
		}
		else if ( strcmp(tokens[1], "BUY_COMMAND_SUCCESS") == 0 )
		{
			printf("\nYou have made a request to source auction:\n"
				   "\tSource amount: %s\n"
				   "\tSource price: %sP\n\n", tokens[2], tokens[3]);

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "BUY_COMMAND_SUCCESS");
		}
		else if ( strcmp(tokens[1], "BUY_COMMAND_INCORRECT_PRICE") == 0 )
		{
			printf("%s", "\n[!] Your price to buy source is less than minimal price for this turn.\n\n");
			
			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "BUY_COMMAND_INCORRECT_PRICE");
		}
		else if ( strcmp(tokens[1], "BUY_COMMAND_INCORRECT_AMOUNT") == 0 )
		{
			printf("%s", "\n[!] Your amount of source is incorrect.\n\n");

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "BUY_COMMAND_INCORRECT_AMOUNT");
		}
		else if ( strcmp(tokens[1], "SELL_COMMAND_ALREADY_SENT") == 0 )
		{
			printf("%s", "\n[!] You have already done the request to production auction!\n\n");
	
			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "SELL_COMMAND_ALREADY_SENT");
		}
		else if ( strcmp(tokens[1], "SELL_COMMAND_SUCCESS") == 0 )
		{
			printf("\nYou have made a request to product auction:\n"
				   "\tProduct amount: %s\n"
				   "\tProduct price: %sP\n\n", tokens[2], tokens[3]);

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "SELL_COMMAND_SUCCESS");
		}
		else if ( strcmp(tokens[1], "SELL_COMMAND_INCORRECT_PRICE") == 0 )
		{
			printf("%s", "\n[!] Your price to sell product is greater than maximal price for this turn.\n\n");
	
			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "SELL_COMMAND_INCORRECT_PRICE");
		}
		else if ( strcmp(tokens[1], "SELL_COMMAND_INCORRECT_AMOUNT") == 0 )
		{
			printf("%s", "\n[!] Your amount of product is incorrect.\n\n");

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "SELL_COMMAND_INCORRECT_AMOUNT");
		}
		else if ( strcmp(tokens[1], "TURN_COMMAND_SUCCESS") == 0 )
		{
			int players_amount = atoi(tokens[2]);
			if ( players_amount > 0 )
			{
				if ( !turn_flag )
				{
					printf("%s", "\nYou finished your deals in this month\n");
					turn_flag = 1;
				}
				printf("Waiting other players(%s)...\n\n", tokens[2]);
			}
			else
			{
				printf("%s", "You finished your deals in this month\n");
			}

			main_info.execute_script = 0;

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "TURN_COMMAND_SUCCESS");
		}
		else if ( strcmp(tokens[1], "VICTORY_MESSAGE") == 0 )
		{
			printf("%s","\n\t\t\t\tCONGRATULATIONS!!!\n"
						"\t\t\t\tYou won this game!\n\n");
			
			main_info.execute_script = 0;

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "VICTORY_MESSAGE");
		}
		else if ( strcmp(tokens[1], "LOST_ALIVE_PLAYER") == 0 )
		{
			printf("\n\t\tOne of players left the game\n"
				   "\t\tTotal alive players: %s\n\n", tokens[2]);
			
			int player_num = atoi(tokens[3]);
			PlayerInfo* pi = main_info.p_info[player_num-1];
			if ( pi )
			{
				delete pi;
				pi = NULL;
			}

			main_info.alive_players--;

			int num = main_info.my_id;
			pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "LOST_ALIVE_PLAYER");
		}
		else if ( strcmp(tokens[1], "PRODUCED") == 0 )
		{
			printf("\nYou have produced %s amount of product on this turn.\n\n", tokens[2]);
	
			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "PRODUCED");
		}
		else if ( strcmp(tokens[1], "GAME_ALREADY_STARTED") == 0 )
		{
			printf("%s", "\n[!] This game is already started. Try to connect later ;(\n\n");

			main_info.execute_script = 0;
		}
		else if ( strcmp(tokens[1], "SERVER_FULL") == 0 )
		{
			printf("%s", "\n[!] Server is full.\n\n");

			main_info.execute_script = 0;
		}
		else if ( strcmp(tokens[1], "OUT_OF_MEMORY") == 0 )
		{
			printf("%s", "\n[!] An internal error has occured while proceeding player record\n\n");

			main_info.execute_script = 0;
		}
		else if ( strcmp(tokens[1], "NEW_PLAYER_CONNECT") == 0 )
		{
			printf("\n\t\tNew player connected. There are %s/%s players in lobby\n\n", tokens[2], tokens[3]);

			main_info.execute_script = 0;
		}
		else if ( strcmp(tokens[1], "STARTINSECONDS") == 0 )
		{
			printf("\n[!] The game will start in %s seconds!\n\n", tokens[2]);

			main_info.execute_script = 0;
		}
		else if ( strcmp(tokens[1], "STARTCANCELLED") == 0 )
		{
			printf("%s", "\n[!] Starting the game is cancelled!\n\n");

			main_info.execute_script = 0;
		}
		else if ( strcmp(tokens[1], "GAME_STARTED") == 0 )
		{
			printf("%s", "\n\t\t\t\t\tTHE GAME HAS STARTED!\n\n");

			main_info.execute_script = 0;
		}
		else if ( strcmp(tokens[1], "STARTING_GAME_INFORMATION") == 0  )
		{
			/*
			const char* specs[] = {
								"Alive players: ",
								"Month: ",
								"Money: ",
								"Sources: ",
								"Products: ",
								"Wait factories: ",
								"Work factories: ",
								"Build factories: ",
								NULL
			};

			printf("\n==== Player #%s ====\n", tokens[2]);
			int i;
			for ( i = 7; tokens[i] != NULL; i++ )
			{
				if ( strcmp(specs[i-7], "Money: ") == 0 )
					printf(" <%s %s> ( +0 )\n", specs[i-7], tokens[i-4]);
				else
					printf(" <%s %s>\n", specs[i-7], tokens[i-4]);
			}
			putchar('\n');
			*/	

			main_info.my_id = atoi(tokens[2]);
			main_info.turn = atoi(tokens[4]);
			main_info.alive_players = atoi(tokens[3]);
			main_info.total_players = main_info.alive_players;
			main_info.cur_sources_buy = atoi(tokens[11]);
			main_info.cur_sources_min_price = atoi(tokens[12]);
			main_info.cur_products_sell = atoi(tokens[13]);
			main_info.cur_products_max_price = atoi(tokens[14]);

			
			int id = main_info.my_id;
			if ( (id < 1) || (id > MAX_PLAYERS) )
			{
				fprintf(stderr, "%s", "\n[bot_check_server_response]: token STARTING_GAME_INFORMATION: \"id\" has incorrect value!\n");
				return 0;
			}
			
			if ( main_info.p_info[id-1] == NULL )
			{
				main_info.p_info[id-1] = new PlayerInfo;
				PlayerInfo* pi = main_info.p_info[id-1];
				if ( !pi )
				{
					fprintf(stderr, "%s", "\n[bot_check_server_response]: token STARTING_GAME_INFORMATION: memory error\n");
					return 0;
				}

				pi->player_num = id;
				pi->money = atoi(tokens[5]);
				pi->raw = atoi(tokens[6]);
				pi->prod = atoi(tokens[7]);
				pi->wait_fact = atoi(tokens[8]);
				pi->work_fact = atoi(tokens[9]);
				pi->build_fact = atoi(tokens[10]);
				pi->income = 0;
				pi->manufactured = 0;
				pi->res_raw_sold = 0;
				pi->res_raw_price = 0;
				pi->res_prod_bought = 0;
				pi->res_prod_price = 0;
			}
			main_info.last_player_num = id;

			main_info.execute_script = 1;

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "STARTING_GAME_INFORMATION");
		}
		else if ( strcmp(tokens[1], "GAME_NOT_STARTED") == 0 )
		{
			printf("%s", "\n[!] Game is not started yet!\n\n");

			main_info.execute_script = 0;
		}
		else if ( strcmp(tokens[1], "WAIT_FOR_NEXT_TURN") == 0 )
		{
			int players_amount = atoi(tokens[2]);
			if ( players_amount > 0 )
				printf("\n[!] You need to wait other %s players now\n\n", tokens[2]);

			main_info.execute_script = 0;

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "WAIT_FOR_NEXT_TURN");
		}
		else if ( strcmp(tokens[1], "UNKNOWN_COMMAND") == 0 )
		{
			printf("\n[!] Unknown command. Type \"help\" to get help.\n\n");

			main_info.execute_script = 0;

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "UNKNOWN_COMMAND");
		}
		else if ( strcmp(tokens[1], "LOST_LOBBY_PLAYER") == 0 )
		{
			printf("\n\t\tOne of players left the lobby\n"
				   "\t\tTotal lobby players: %s/%s\n\n", tokens[2], tokens[3]);
			
			main_info.execute_script = 0;
		}
		else if ( strcmp(tokens[1], "SUCCESS_CHARGES_PAY") == 0 )
		{
			printf("\n[!] You have successfully paid %sP as charges for this month\n\n", tokens[2]);

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "SUCCESS_CHARGES_PAY");
		}
		else if ( strcmp(tokens[1], "PLAYER_BANKROT") == 0 )
		{
			printf("\n[!] You couldn't to pay for charges( %sP ) in this month!\n"
				   "[!] Unfortunately, you are a bankrot. Goodbye ;(\n\n", tokens[2]);

			main_info.execute_script = 0;

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "PLAYER_BANKROT");
		}
		else if ( strcmp(tokens[1], "PAY_FACTORY_SUCCESS") == 0 )
		{
			printf("%s", "\n[!] You have successfully paid for your new factory!\n"
						 "[!] It will occur in the next month\n\n");

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "PAY_FACTORY_SUCCESS");
		}
		else if ( strcmp(tokens[1], "FACTORY_BUILT") == 0 )
		{
			printf("%s", "\n[!] You have a new factory which is read for work!\n\n");
	
			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "FACTORY_BUILT");
		}
		else if ( strcmp(tokens[1], "NEW_TURN") == 0 )
		{
			turn_flag = 0;
			printf("\n%s", "<");
			int i;
			for ( i = 1; i <= 38; i++ )
				putchar('%');
			for ( i = 1; i <= 9; i++ )
				putchar('=');
			for ( i = 1; i <= 38; i++ )
				putchar('%');
			printf("%s\n", ">");

			printf("\n\n\n\n\n\t\t\t\t\tMONTH #%s\n\n\n\n\n", tokens[2]);
			
			printf("\n%s", "<");
			for ( i = 1; i <= 38; i++ )
				putchar('%');
			for ( i = 1; i <= 9; i++ )
				putchar('=');
			for ( i = 1; i <= 38; i++ )
				putchar('%');
			printf("%s\n", ">\n");

			main_info.turn = atoi(tokens[2]);
			main_info.cur_sources_buy = atoi(tokens[3]);
			main_info.cur_sources_min_price = atoi(tokens[4]);
			main_info.cur_products_sell = atoi(tokens[5]);
			main_info.cur_products_max_price = atoi(tokens[6]);
			main_info.execute_script = 1;
			
			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "NEW_TURN");
		}
		else if ( strcmp(tokens[1], "AUCTION_RESULTS") == 0 )
		{
			printf("\n\n\n========== Auctions results in Month #%s ==========\n\n", tokens[2]);
			int i;
			int j = 0;
			for ( i = 3; tokens[i] != NULL; i += 6 )
			{
				printf("Player #%s: <bought sources: %s> <bought price: %s> <sold products: %s> <sold price: %s>\n", tokens[i], tokens[i+1], tokens[i+2], tokens[i+3], tokens[i+4] );
				PlayerInfo* pi = main_info.p_info[j];
				if ( pi != NULL )
				{
					pi->res_raw_sold = atoi(tokens[i+1]);
					pi->res_raw_price = atoi(tokens[i+2]);
					pi->res_prod_bought = atoi(tokens[i+3]);
					pi->res_prod_price = atoi(tokens[i+4]);
				}
				j++;
			}
			printf("\n========== Auctions results in Month #%s ==========\n\n\n", tokens[2]);

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "AUCTION_RESULTS");
		}
		else if ( strcmp(tokens[1], "BUILDING_FACTORIES_LIST") == 0 )
		{
			/*printf("command_tokens_amount = %d\n", command_tokens_amount);*/
			printf("%s", "\nBuilding factories list:\n");
			
			int i;
			for ( i = 3; i < command_tokens_amount; i += 2 )
			{
				printf("====== (%s) ======\n", tokens[i]);
				printf("  Turns left: %s\n", tokens[i+1]);
				printf("====== (%s) ======\n", tokens[i]);
			}
			putchar('\n');

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "BUILDING_FACTORIES_LIST");
		}
		else if ( strcmp(tokens[1], "BUILDING_FACTORIES_LIST_EMPTY") == 0 )
		{
			printf("%s", "\n[!] You have no any building factories now\n");

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "BUILDING_FACTORIES_LIST_EMPTY");
		}
	}
	else if ( strcmp(tokens[0], "*ERROR_MESSAGE") == 0 )
	{
		if ( strcmp(tokens[1], "COMMAND_INTERNAL_ERROR") == 0 )
		{
			printf("%s", "\n[!] An internal error has occured while executing this command!\n");

			main_info.execute_script = 0;

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "COMMAND_INTERNAL_ERROR");
		}
		else if ( strcmp(tokens[1], "COMMAND_INCORRECT_ARGUMENTS_NUM") == 0 )
		{
			printf("%s", "\n[!] Incorrect number of arguments! Try type \"help\" for help.\n");

			main_info.execute_script = 0;

			int num = main_info.my_id;
			PlayerInfo* pi = main_info.p_info[num-1];
			print_debug_main_info(pi, "COMMAND_INCORRECT_ARGUMENTS_NUM");
		}
	}

	return 1;
}

#endif
