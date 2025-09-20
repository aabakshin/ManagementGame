/*
 * Модуль bot_mg отвечает за создание главного процесса искусственного игрока,
 * установку соединения с сервером, парсинг игровых скриптов, задающих игровую
 * стратегию игрока, а также интерпретацию игровых команд.
 * */

#include "../../includes/botCore.hpp"
#include <signal.h>

/* Описаны в модуле SA_Additional */
extern PolizItem* vars_list;
extern LabelsList* labels_list;

/* Глобальная структура, хранящая текущее состояние искусственного игрока */
MainInfo main_info;

/* Установка флага завершает выполнение программы */
int break_flag = 0;

/* Ф-я-обработчик сигнала SIGINT */
void exit_handler(int signo)
{
	int save_errno = errno;
	signal(SIGINT, exit_handler);

	break_flag = 1;

	errno = save_errno;
}

/* Ф-я запуска клиента игры */
int main(int argc, char** argv)
{
	signal(SIGINT, exit_handler);

	if (argc != 4)
	{
		fprintf(stderr, "usage: ./bot_mg <hostname> <port> <script_filename>\n");
		return 1;
	}
	
	mi_init(&main_info);
	main_info.pid = getpid();
	
	FILE* fd;
	if ( (fd = fopen(argv[3], "r")) == NULL )
	{
		fprintf(stderr, "Unable to open file \"%s\". File does not exist or you have no perms\n", argv[3]);
		return 1;
	}


	/* Лексический анализ */

	LexemAnalyzer la;
	LexemList* lexem_list = la.Run(fd);

	ll_print(lexem_list);

	fclose(fd);
	
	/* Cинтаксический анализ */
	
	SyntaxAnalyzer sa;
	PolizItem* operators_list = sa.Run(&lexem_list);																		
	PolizItem* op_list_ptr = operators_list;

	label_print(labels_list);
	pi_print(operators_list);
	pi_print(vars_list);
	
	ll_clear(&lexem_list, 1);
	


	/*pi_clear(&operators_list, 1);
	pi_clear(&vars_list, 1);
	label_clear(&labels_list, 1);

	return 0;*/
	

	/* Установка соединения */

	int socket_peer = bot_connect(argv[1], argv[2]);
	if ( socket_peer == -1 )
	{
		fprintf(stderr, "%s", "An error has occured while executing initialization procedure.\n");	
	
		mi_clear(&main_info);
		pi_clear(&operators_list, 1);
		pi_clear(&vars_list, 1);
		label_clear(&labels_list, 1);

		return 1;
	}
	main_info.fd = socket_peer;

	
	char id_buf[SEND_BUFFER_SIZE];
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
	printf("\nSent %d\\%d bytes\n", wc, i+1);


	/* Интерпретация сценария */
	
	PolizItem* stack = NULL;

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	while ( 1 )
	{
		/*
		if ( main_info.execute_script )
		{
			PolizItem* vl = vars_list;
			int vars_size = pi_size(vars_list);
			int x;
			putchar('\n');
			for (x = 1; x <= vars_size; x++ )
			{
				PolizVar* var = dynamic_cast<PolizVar*>(vl->p);
				printf("Variable [%d]:\n"
					   "- name: %s\n"
					   "- address: %d\n"
					   "- value: %d\n"
					   "------------\n",
					   x, var->GetVarName(), var->GetVarAddr()->Get(), var->GetVarValue());

				vl = vl->next;
			}
			putchar('\n');
		}
		*/

		if ( break_flag )
			break;

		fd_set reads;
		FD_ZERO(&reads);
		FD_SET(socket_peer, &reads);
		
		int res = -1;
		if ( (res = select(socket_peer+1, &reads, 0, 0, &tv)) < 0 )
		{
			if ( errno == EINTR )
			{
				fprintf(stderr, "%s", "Got some signal.\n");
				continue;
			}
			
			fprintf(stderr, "select() failed. (errno code = %d)\n", errno);
			continue;
		}

		if ( FD_ISSET(socket_peer, &reads) )
		{
			char read[RECV_BUFFER_SIZE] = { 0 };
			int bytes_received = readline(socket_peer, read, RECV_BUFFER_SIZE);
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
				char buffer[RECV_BUFFER_SIZE];
				int k;
				for ( k = 0; read_tokens[j][k]; k++ )
					buffer[k] = read_tokens[j][k];
				buffer[k] = '\0';

				if ( !bot_check_server_response(buffer) )
				{
					fprintf(stderr, "%s", "\nUnable to process server response\n");
					break_flag = 1;
					break;
				}
			}
			
			if ( break_flag )
				break;
		} 
		
		/*printf("\nmain_info.execute_script = %d\n", main_info.execute_script);*/

		if ( main_info.execute_script )
			if ( (operators_list != NULL) && (operators_list->p != NULL) )
				operators_list->p->Evaluate(&stack, &operators_list);
	}
	
	/* Очистка всех игровых структур */
	mi_clear(&main_info);
	pi_clear(&stack, 1);
	pi_clear(&op_list_ptr, 1);
	pi_clear(&vars_list, 1);
	label_clear(&labels_list, 1);

	printf("%s\n", "Closing socket...");
	close(socket_peer);
	printf("%s\n", "Finished.");

	return 0;
}
