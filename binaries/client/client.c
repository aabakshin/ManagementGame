/* 
 * Главный клиентский модуль.
 * Запускает программу, инициирует TCP-соединение с сервером.
 * Обеспечивает взаимодействие с сервером.
 *
 * */

#include "../../includes/clientCore.h"
#include <termios.h>


/* Описан в модуле clientCore */
extern CommandsHistoryList* chl_list;

int main(int argc, char** argv)
{
	if ( argc != 3 )
	{
		fprintf(stderr, "usage: tcp_client <hostname> <port>\n");
		return 1;
	}
	
	if ( !isatty(0) )
	{
		fprintf(stderr, "%s", "stdin is not a terminal!\n");
		return 1;
	}

	printf("PID = %d\n"
		   "Terminal name: %s\n\n", getpid(), ttyname(0));

	int socket_peer = client_init(argv[1], argv[2]);
	if ( socket_peer == -1 )
	{
		fprintf(stderr, "%s", "An error has occured while executing initial procedure\n");
		return 1;
	}

	/* Отправка первичных идентификационных данных на сервер */
	char id_buf[SEND_BUFFER_SIZE] = { 0 };
	char argc_buf[10];
	itoa(argc, argc_buf, 9);

	int i;
	for ( i = 0; argv[0][i]; i++ )
		id_buf[i] = argv[0][i];
	int j;
	for ( j = 0; argc_buf[j]; j++, i++ )
		id_buf[i] = argc_buf[j];
	id_buf[i] = '\n';
	id_buf[i+1] = '\0';

	int wc = write(socket_peer, id_buf, i+1);
	printf("Sent %d\\%d bytes\n", wc, i+1);



	/* Выключение канонического режима терминала */
	struct termios t1, t2;
	tcgetattr(0, &t1);
	memcpy(&t2, &t1, sizeof(t1));

	t1.c_lflag &= ~ICANON;
	t1.c_lflag &= ~ISIG;
	t1.c_lflag &= ~ECHO;
	t1.c_cc[VMIN] = 0;
	t1.c_cc[VTIME] = 0;

	tcsetattr(0, TCSANOW, &t1);
	

	while (1)
	{
		fd_set reads;
		FD_ZERO(&reads);
		FD_SET(socket_peer, &reads);
		FD_SET(0, &reads);

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = TIMEOUT;
		
		int res = -1;
		if ( (res = select(socket_peer+1, &reads, 0, 0, &timeout)) < 0 )
		{
			if ( errno == EINTR )
			{
				fprintf(stderr, "%s", "Got some signal.\n");
				continue;
			}
			
			fprintf(stderr, "select() failed. (errno code = %d)\n", errno);
			continue;
		}
		
		if ( res == 0 )
			continue;

		if ( FD_ISSET(socket_peer, &reads) )
		{
			char read[RECEIVE_BUFFER_SIZE] = { 0 };
			int bytes_received = readline(socket_peer, read, RECEIVE_BUFFER_SIZE);
			if ( bytes_received < 1 )
			{
				fprintf(stderr, "%s\n", "Connection closed by peer.");
				break;
			}
			

			/*--------------------------------------------------------------------------------------*/
			/*for ( i = 0; i < RECEIVE_BUFFER_SIZE; i++ )
			{
				printf("%c ", read[i]);
				if ( ((i+1) % 10) == 0 )
					putchar('\n');
			}
			putchar('\n');
			for ( i = 0; i < RECEIVE_BUFFER_SIZE; i++ )
			{
				printf("%4d ", read[i]);
				if ( ((i+1) % 10) == 0 )
					putchar('\n');
			}
			putchar('\n');*/
			/*--------------------------------------------------------------------------------------*/
			
			int tokens_amount = 0;
			int i;
			for ( i = 0; read[i]; i++ )
				if ( read[i] == '\n' )
					tokens_amount++;
			
			/*printf("\ntokens_amount = %d\n", tokens_amount);*/

			int j = 0;
			char* read_tokens[tokens_amount > 0 ? tokens_amount : 1];
			if ( tokens_amount > 0 )
			{
				char* istr = strtok(read, "\n");

				while ( istr != NULL )
				{
					read_tokens[j] = istr;
					j++;
					istr = strtok(NULL, "\n");
				}
			}
			
			/*printf("\nj = %d\n", j);*/	

			/*
			for ( i = 0; i < bytes_received; i++ )
				if ( read[i] == '\n' )
					break;

			if ( i < RECEIVE_BUFFER_SIZE )
				read[i] = '\0';
			
			printf("\nread = %s\n", read);
			*/
			

			for ( j = 0; j < tokens_amount; j++ )
			{
				char buffer[RECEIVE_BUFFER_SIZE];
				int k;
				for ( k = 0; read_tokens[j][k]; k++ )
					buffer[k] = read_tokens[j][k];
				buffer[k] = '\0';

				if ( !check_server_response(buffer) )
					fprintf(stderr, "%s", "\nUnable to process server response\n");
			} 
		} 

		if ( FD_ISSET(0, &reads) )
		{
			char read[SEND_BUFFER_SIZE] = { 0 };
			
			int mes_len = -1;
			if ( (mes_len = get_string(read, SEND_BUFFER_SIZE)) < 1 )
			{
				if ( mes_len == EXIT_CODE )
					break;
				/*printf("\nmes_len = %d\n", mes_len);*/
				continue;
			}
			
			if ( read[0] == '\n')
				continue;

			/*printf("\nread = %s\nmes_len = %d\n", read, mes_len);*/
			
			/*int k;
			for ( k = 0; k < 20; k++ )
			{
				printf("%3d ", read[k]);
				if ( ((k+1) % 10) == 0 )
					putchar('\n');
			}
			*/

			int size = mes_len+1;
			
			delete_spaces(read, &size);
			mes_len = strlen(read);

			int bytes_sent = send(socket_peer, read, mes_len, 0);
			
			if ( read[mes_len-1] == '\n' )
			{
				read[mes_len-1] = '\0';
				size--;
			}

			chl_insert(&chl_list, read, size);
			int chl_size = chl_get_size(chl_list);

			if ( chl_size > HISTORY_COMMANDS_LIST_SIZE )
			{
				chl_delete(&chl_list, chl_list->number);
			}
			/*chl_print(chl_list);*/
			/*printf("Sent %d bytes.\n", bytes_sent);*/
		}
	}
	
	/* восстановление канонического режима */
	tcsetattr(0, TCSANOW, &t2);
	
	/* очистка буфера отправленных команд */
	chl_clear(&chl_list);

	/*printf("\n--------------------------\n");
	chl_print(chl_list);
	printf("\n--------------------------\n");*/

	printf("%s\n", "Closing socket...");
	close(socket_peer);
	
	printf("%s\n", "Finished.");

	return 0;
}
