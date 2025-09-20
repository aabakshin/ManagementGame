/* Файл реализации модуля clientCore */

#ifndef CLIENTCORE_C
#define CLIENTCORE_C

#include "../includes/clientCore.h"
#include "../includes/CommandsHistoryList.h"

/* Буфер предыдущих отпраленных команд */
CommandsHistoryList* chl_list = NULL;

/* Игровой флаг. Определяет, отправлял ли игрок команду turn */
static int turn_flag = 0;

/* Стандартная процедура инициализации клиентского TCP-сокета */
int client_init(const char* addr, const char* port)
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

/* Обработка полученной информации от сервера в соответствии с протоколом общения */
int check_server_response(char* buffer)
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
		if ( strcmp(tokens[1], "HELP_COMMAND") == 0 )
		{
			const char* commands_desc[] = {
						"- print all avaliable commands",
						"- show current state of market",
						"<player_number> - give base information about player with specific number",
						"- show all players who are not a bankrot",
						"- start to produce production from one of your waiting factories",
						"- start to build one more factory. It will take 5 months",
						"<amount> <price> - make a request to buy some source from sources auction",
						"<amount> <price> - make a request to sell some products to products auction",
						"- finish the turn",
						"- force quit the game",
						NULL
			};
			printf("%s", "\nList of all avaliable commands:\n");
			int i;
			for ( i = 2; tokens[i] != NULL; i++ )
				printf("\t%s %s\n", tokens[i], commands_desc[i-2]);
			putchar('\n');
		}
		else if ( strcmp(tokens[1], "MARKET_COMMAND") == 0 )
		{
			const char* specs[] = {
							"\tmaximum sources amount to buy: ",
							"\tminimal price for source to buy: ",
							"\tmaximum product amount to sell: ",
							"\tmaximum product price to sell: ",
							NULL
			};
			printf("%s", "\nCurrent state of market:\n");
			int i;
			for ( i = 2; tokens[i] != NULL; i++ )
				printf("%s %s\n", specs[i-2], tokens[i]);
			putchar('\n');
		}
		else if ( strcmp(tokens[1], "PLAYER_COMMAND_NOT_FOUND") == 0 )
		{
			printf("%s", "\n\t[!] Player with that specific number is not found!\n");
		}
		else if ( strcmp(tokens[1], "PLAYER_COMMAND") == 0 )
		{
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
		}
		else if ( strcmp(tokens[1], "LIST_COMMAND") == 0 )
		{
			printf("\nThere is %s alive players left.\n\n", tokens[2]);
		}
		else if ( strcmp(tokens[1], "PROD_COMMAND_SUCCESS") == 0 )
		{
			printf("%s", "\nYou are producing a product\n"
					"Result will be on next turn\n\n");
		}
		else if ( strcmp(tokens[1], "PROD_COMMAND_NO_FACTORIES") == 0 )
		{
			printf("%s", "\n[!] You don't have on current turn any waiting factories!\n\n");
		}
		else if ( strcmp(tokens[1], "PROD_COMMAND_NO_MONEY") == 0 )
		{
			printf("%s", "\n[!] You don't have enough money to pay for producing a product!\n\n");
		}
		else if ( strcmp(tokens[1], "PROD_COMMAND_NO_SOURCE") == 0 )
		{
			printf("%s", "\n[!] You don't have enough sources to produce a product!\n\n");
		}
		else if ( strcmp(tokens[1], "BUILD_COMMAND_SUCCESS") == 0 )
		{
			printf("%s", "\nYou have successfully started to build a new factory.\n\n");
		}
		else if ( strcmp(tokens[1], "BUILD_COMMAND_NO_MONEY") == 0 )
		{
			printf("%s", "\n[!] You don't have enough money to start building a new factory!\n\n");
		}
		else if ( strcmp(tokens[1], "BUY_COMMAND_NO_MONEY") == 0 )
		{
			printf("%s", "\n[!] You don't have enough money to do request to source auction!\n\n");	
		}
		else if ( strcmp(tokens[1], "BUY_COMMAND_ALREADY_SENT") == 0 )
		{
			printf("%s", "\n[!] You have already done the request to source auction!\n\n");
		}
		else if ( strcmp(tokens[1], "BUY_COMMAND_SUCCESS") == 0 )
		{
			printf("\nYou have made a request to source auction:\n"
				   "\tSource amount: %s\n"
				   "\tSource price: %sP\n\n", tokens[2], tokens[3]);
		}
		else if ( strcmp(tokens[1], "BUY_COMMAND_INCORRECT_PRICE") == 0 )
		{
			printf("%s", "\n[!] Your price to buy source is less than minimal price for this turn.\n"
						 "[!] To check information about market enter command \"market\"\n\n");
		}
		else if ( strcmp(tokens[1], "BUY_COMMAND_INCORRECT_AMOUNT") == 0 )
		{
			printf("%s", "\n[!] Your amount of source is incorrect.\n\n");
		}
		else if ( strcmp(tokens[1], "SELL_COMMAND_ALREADY_SENT") == 0 )
		{
			printf("%s", "\n[!] You have already done the request to production auction!\n\n");
		}
		else if ( strcmp(tokens[1], "SELL_COMMAND_SUCCESS") == 0 )
		{
			printf("\nYou have made a request to product auction:\n"
				   "\tProduct amount: %s\n"
				   "\tProduct price: %sP\n\n", tokens[2], tokens[3]);
		}
		else if ( strcmp(tokens[1], "SELL_COMMAND_INCORRECT_PRICE") == 0 )
		{
			printf("%s", "\n[!] Your price to sell product is greater than maximal price for this turn.\n"
						 "[!] To check information about market enter command \"market\"\n\n");
		}
		else if ( strcmp(tokens[1], "SELL_COMMAND_INCORRECT_AMOUNT") == 0 )
		{
			printf("%s", "\n[!] Your amount of product is incorrect.\n\n");
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
		}
		else if ( strcmp(tokens[1], "VICTORY_MESSAGE") == 0 )
		{
			printf("%s","\n\t\t\t\tCONGRATULATIONS!!!\n"
						"\t\t\t\tYou won this game!\n\n");
		}
		else if ( strcmp(tokens[1], "LOST_ALIVE_PLAYER") == 0 )
		{
			printf("\n\t\tOne of players left the game\n"
				   "\t\tTotal alive players: %s\n\n", tokens[2]);
		}
		else if ( strcmp(tokens[1], "PRODUCED") == 0 )
		{
			printf("\nYou have produced %s amount of product on this turn.\n\n", tokens[2]);
		}
		else if ( strcmp(tokens[1], "GAME_ALREADY_STARTED") == 0 )
		{
			printf("%s", "\n[!] This game is already started. Try to connect later ;(\n\n");
		}
		else if ( strcmp(tokens[1], "SERVER_FULL") == 0 )
		{
			printf("%s", "\n[!] Server is full.\n\n");
		}
		else if ( strcmp(tokens[1], "OUT_OF_MEMORY") == 0 )
		{
			printf("%s", "\n[!] An internal error has occured while proceeding player record\n\n");
		}
		else if ( strcmp(tokens[1], "NEW_PLAYER_CONNECT") == 0 )
		{
			printf("\n\t\tNew player connected. There are %s/%s players in lobby\n\n", tokens[2], tokens[3]);
		}
		else if ( strcmp(tokens[1], "STARTINSECONDS") == 0 )
		{
			printf("\n[!] The game will start in %s seconds!\n\n", tokens[2]);
		}
		else if ( strcmp(tokens[1], "STARTCANCELLED") == 0 )
		{
			printf("%s", "\n[!] Starting the game is cancelled!\n\n");
		}
		else if ( strcmp(tokens[1], "GAME_STARTED") == 0 )
		{
			printf("%s", "\n\t\t\t\t\tTHE GAME HAS STARTED!\n\n");
		}
		else if ( strcmp(tokens[1], "STARTING_GAME_INFORMATION") == 0  )
		{
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
			for ( i = 3; tokens[i] != NULL; i++ )
			{
				if ( strcmp(specs[i-3], "Money: ") == 0 )
					printf(" <%s %s> ( +0 )\n", specs[i-3], tokens[i]);
				else
					printf(" <%s %s>\n", specs[i-3], tokens[i]);
			}
			putchar('\n');
		}
		else if ( strcmp(tokens[1], "GAME_NOT_STARTED") == 0 )
		{
			printf("%s", "\n[!] Game is not started yet!\n\n");
		}
		else if ( strcmp(tokens[1], "WAIT_FOR_NEXT_TURN") == 0 )
		{
			int players_amount = atoi(tokens[2]);
			if ( players_amount > 0 )
				printf("\n[!] You need to wait other %s players now\n\n", tokens[2]);
		}
		else if ( strcmp(tokens[1], "UNKNOWN_COMMAND") == 0 )
		{
			printf("\n[!] Unknown command. Type \"help\" to get help.\n\n");
		}
		else if ( strcmp(tokens[1], "LOST_LOBBY_PLAYER") == 0 )
		{
			printf("\n\t\tOne of players left the lobby\n"
				   "\t\tTotal lobby players: %s/%s\n\n", tokens[2], tokens[3]);

		}
		else if ( strcmp(tokens[1], "SUCCESS_CHARGES_PAY") == 0 )
		{
			printf("\n[!] You have successfully paid %sP as charges for this month\n\n", tokens[2]);
		}
		else if ( strcmp(tokens[1], "PLAYER_BANKROT") == 0 )
		{
			printf("\n[!] You couldn't to pay for charges( %sP ) in this month!\n"
				   "[!] Unfortunately, you are a bankrot. Goodbye ;(\n\n", tokens[2]);
		}
		else if ( strcmp(tokens[1], "PAY_FACTORY_SUCCESS") == 0 )
		{
			printf("%s", "\n[!] You have successfully paid for your new factory!\n"
						 "[!] It will occur in the next month\n\n");
		}
		else if ( strcmp(tokens[1], "FACTORY_BUILT") == 0 )
		{
			printf("%s", "\n[!] You have a new factory which is read for work!\n\n");
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
		}
		else if ( strcmp(tokens[1], "AUCTION_RESULTS") == 0 )
		{
			printf("\n\n\n========== Auctions results in Month #%s ==========\n\n", tokens[2]);
			int i;
			for ( i = 3; tokens[i] != NULL; i += 6 )
			{
				printf("Player #%s: <bought sources: %s> <bought price: %s> <sold products: %s> <sold price: %s>\n", tokens[i], tokens[i+1], tokens[i+2], tokens[i+3], tokens[i+4] );
			}
			printf("\n========== Auctions results in Month #%s ==========\n\n\n", tokens[2]);
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
		}
		else if ( strcmp(tokens[1], "BUILDING_FACTORIES_LIST_EMPTY") == 0 )
		{
			printf("%s", "\n[!] You have no any building factories now\n");
		}
	}
	else if ( strcmp(tokens[0], "*ERROR_MESSAGE") == 0 )
	{
		if ( strcmp(tokens[1], "COMMAND_INTERNAL_ERROR") == 0 )
		{
			printf("%s", "\n[!] An internal error has occured while executing this command!\n");
		}
		else if ( strcmp(tokens[1], "COMMAND_INCORRECT_ARGUMENTS_NUM") == 0 )
		{
			printf("%s", "\n[!] Incorrect number of arguments! Try type \"help\" for help.\n");
		}
	}

	return 1;
}

#endif
