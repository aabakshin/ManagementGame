/* Файл реализации модуля MGLib */

#ifndef MGLIB_C
#define MGLIB_C

#include "../includes/MGLib.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

/* Системная константа */
enum
{
	BUFSIZE = 1024
};

/* Вспомогательная ф-я для heap_sort. Превращает массив целых в древовидную структуру "куча" */
static void heap_make(int* values, int size, int ascending)	/* ascending = 1 => по убыванию*/
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

/* Вспомогательная ф-я для itoa. Делает реверс строки */
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

/* Вспомогательная функция для itoa. Подсчитывает кол-во цифр в числе */
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
