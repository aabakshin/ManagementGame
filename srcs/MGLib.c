/* Файл реализации модуля MGLib */

#ifndef MGLIB_C
#define MGLIB_C

#include "../includes/MGLib.h"
#include <stdlib.h>


extern ssize_t read(int fd, void* buf, size_t count);
extern ssize_t write(int fd, const void* buf, size_t count);
extern size_t strlen(const char* s);
extern char* strncpy(char* dest, const char* src, size_t n);
extern void* memset(void* s, int c, size_t n);
extern int printf(const char*, ...);


enum { BUFSIZE = 1024 };


const char* info_game_messages[] =
{
				"*INFO_MESSAGE|AUCTION_RESULTS",
				"*INFO_MESSAGE|SUCCESS_CHARGES_PAY",
				"*INFO_MESSAGE|PLAYER_BANKROT",
				"*INFO_MESSAGE|LOST_ALIVE_PLAYER",
				"*INFO_MESSAGE|WAIT_FOR_NEXT_TURN",
				"*INFO_MESSAGE|PRODUCED",
				"*INFO_MESSAGE|STARTINSECONDS",
				"*INFO_MESSAGE|GAME_STARTED",
				"*INFO_MESSAGE|STARTING_GAME_INFORMATION",
				"*INFO_MESSAGE|STARTCANCELLED",
				"*INFO_MESSAGE|PAY_FACTORY_SUCCESS",
				"*INFO_MESSAGE|FACTORY_BUILT",
				"*INFO_MESSAGE|UNKNOWN_COMMAND",
				"*INFO_MESSAGE|VICTORY_MESSAGE",
				"*INFO_MESSAGE|GAME_ALREADY_STARTED",
				"*INFO_MESSAGE|SERVER_FULL",
				"*INFO_MESSAGE|NEW_PLAYER_CONNECT",
				"*INFO_MESSAGE|GAME_NOT_STARTED",
				"*INFO_MESSAGE|LOST_LOBBY_PLAYER",
				"*INFO_MESSAGE|NEW_TURN",
				"*INFO_MESSAGE|HELP_COMMAND",
				"*INFO_MESSAGE|MARKET_COMMAND",
				"*INFO_MESSAGE|PLAYER_COMMAND_NOT_FOUND",
				"*INFO_MESSAGE|PLAYER_COMMAND",
				"*INFO_MESSAGE|LIST_COMMAND",
				"*INFO_MESSAGE|PROD_COMMAND_SUCCESS",
				"*INFO_MESSAGE|PROD_COMMAND_NO_FACTORIES",
				"*INFO_MESSAGE|PROD_COMMAND_NO_MONEY",
				"*INFO_MESSAGE|PROD_COMMAND_NO_SOURCE",
				"*INFO_MESSAGE|BUILDING_FACTORIES_LIST",
				"*INFO_MESSAGE|BUILDING_FACTORIES_LIST_EMPTY",
				"*INFO_MESSAGE|BUILD_COMMAND_INCORRECT_ARG",
				"*INFO_MESSAGE|BUILD_COMMAND_SUCCESS",
				"*INFO_MESSAGE|BUILD_COMMAND_NO_MONEY",
				"*INFO_MESSAGE|BUY_COMMAND_ALREADY_SENT",
				"*INFO_MESSAGE|BUY_COMMAND_NO_MONEY",
				"*INFO_MESSAGE|BUY_COMMAND_SUCCESS",
				"*INFO_MESSAGE|BUY_COMMAND_INCORRECT_PRICE",
				"*INFO_MESSAGE|BUY_COMMAND_INCORRECT_AMOUNT",
				"*INFO_MESSAGE|SELL_COMMAND_ALREADY_SENT",
				"*INFO_MESSAGE|SELL_COMMAND_SUCCESS",
				"*INFO_MESSAGE|SELL_COMMAND_INCORRECT_PRICE",
				"*INFO_MESSAGE|SELL_COMMAND_INCORRECT_AMOUNT",
				"*INFO_MESSAGE|TURN_COMMAND_SUCCESS",
				NULL
};

const char* error_game_messages[] =
{
				"*ERROR_MESSAGE|COMMAND_INCORRECT_ARGUMENTS_NUM\n",
				"*ERROR_MESSAGE|COMMAND_INTERNAL_ERROR\n",
				NULL
};


// Вспомогательная ф-я для heap_sort. Превращает массив целых в древовидную структуру "куча"
// ascending: 1(по убыванию), 0(по возрастанию)
static void heap_make(int* values, int size, int ascending);

// Вспомогательная ф-я для itoa. Делает реверс строки
static void reverse(char* s);

// Вспомогательная функция для itoa. Подсчитывает кол-во цифр в числе
static int num_digit_cnt(int number);






int cut_str(char* s, int s_size, int ch)
{
	if (
				( s == NULL )			||
				( s[0] == '\0' )
		)
	{
		return 0;
	}

	int len = strlen(s);
	int found_ch_flag = 0;

	for ( int i = 0; (i < s_size) && s[i]; ++i )
	{
		if ( s[i] == ch )
		{
			memset(s + i, 0x00, len - i);
			found_ch_flag = 1;
			break;
		}
	}

	if ( !found_ch_flag )
		return 0;

	return 1;
}

static void heap_make(int* values, int size, int ascending)
{
	int i;
	for ( i = size-1; i > 0; i-- )
	{
		int idx = i;
		while ( idx != 0 )
		{
			int parent = (idx - 1) / 2;

			if ( ascending )
			{
				if ( values[idx] > values[parent] )
					break;
			}
			else
			{
				if ( values[idx] <= values[parent])
					break;
			}

			int temp = values[idx];
			values[idx] = values[parent];
			values[parent] = temp;

			idx = parent;
		}
	}
}

void heap_sort(int* values, int size, int ascending)
{
	heap_make(values, size, ascending);

	int i;
	for ( i = size-1; i > 0; i-- )
	{
		int temp = values[i];
		values[i] = values[0];
		values[0] = temp;

		heap_make(values, i, ascending);
	}
}

void delete_spaces(char* buffer, int* bufsize)
{
	if ( buffer == NULL )
		return;

	if ( (*bufsize < 2) || (*bufsize > BUFSIZE) )
		return;

	/* ---------------------------Удаление слева--------------------------- */
	int left_space_count = 0;
	int i;
	for (i = 0; i < (*bufsize)-1; i++)
	{
		if ( (buffer[i] == ' ') || (buffer[i] == '\t') || (buffer[i] == '\r') )
		{
			left_space_count++;
		}
		else
		{
			break;
		}
	}

	if ( left_space_count == ((*bufsize)-1) )
		return;

	int j = 0;
	for (; i < *bufsize; i++)
	{
		buffer[j] = buffer[i];
		j++;
	}
	buffer[j] = '\0';
	*bufsize = strlen(buffer)+1;
	/* ---------------------------Удаление слева--------------------------- */


	/* ---------------------------Удаление справа--------------------------- */
	for ( i = (*bufsize)-2; i > 0; i-- )
	{
		if ( (buffer[i] != ' ') && (buffer[i] != '\t') && (buffer[i] != '\r') )
			break;
	}
	buffer[i+1] = '\0';
	*bufsize = strlen(buffer)+1;
	/* ---------------------------Удаление справа--------------------------- */

	for ( i = 0; i < (*bufsize-1); i++ )
		if ( (buffer[i] == '\t') || (buffer[i] == '\r') )
			buffer[i] = ' ';

	int spaces_count = 0;
	for (i = 0; buffer[i]; i++ )
	{
		if ( (buffer[i] != ' ') )
		{
			if ( spaces_count > 1 )
			{
				int j;
				for ( j = i-spaces_count+1; buffer[i]; j++, i++ )
					buffer[j] = buffer[i];
				buffer[j] = '\0';
				i = 0;
				spaces_count = 0;
			}
			else
			{
				spaces_count = 0;
			}
		}
		else
		{
			spaces_count++;
			continue;
		}
	}

	*bufsize = i+1;
}

static void reverse(char* s)
{
	int i = 0;
	int j = strlen(s)-1;

	for ( ; i < j; i++, j-- )
	{
		char c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

static int num_digit_cnt(int number)
{
	int counter = 0;

	if ( number == 0 )
		return 1;

	if ( number < 0 )
		number *= -1;

	while ( number > 0 )
	{
		counter++;
		number /= 10;
	}

	return counter;
}

void itoa(int number, char* num_buf, int max_buf_len)
{
	if ( number == 0 )
	{
		num_buf[0] = '0';
		num_buf[1] = '\0';
		return;
	}

	int cnt = num_digit_cnt(number);

	if ( cnt > (max_buf_len-1) )
		cnt = max_buf_len-1;

	int flag = 0;
	if ( number < 0 )
	{
		number *= -1;
		flag = 1;
	}

	int i = 0;
	while ( number > 0 && (i < cnt) )
	{
		num_buf[i] = (number % 10) + '0';
		number /= 10;
		i++;
	}

	if ( flag )
	{
		num_buf[i] = '-';
		i++;
	}
	num_buf[i] = '\0';

	reverse(num_buf);
}

int readline(int fd, char* buf, int bufsize)
{
	unsigned int total_read = 0;
	char buffer[bufsize];
	int rc = 0;
	int lf_flag = 0;

	do
	{
		rc = read(fd, buffer+total_read, bufsize-total_read);
		if ( rc < 1 )
			return rc;

		int i;
		for (i = total_read; i < total_read+rc; i++)
			if (buffer[i] == '\n')
				lf_flag = 1;
		total_read += rc;

		if ( (total_read >= bufsize) && (!lf_flag) )
		{
			buffer[total_read-1] = '\n';
			break;
		}
	}
	while ( !lf_flag );

	strncpy(buf, buffer, total_read);

	return total_read;
}

#endif
