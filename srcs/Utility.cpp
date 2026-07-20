#ifndef UTILITY_CPP_SENTINEL
#define UTILITY_CPP_SENTINEL


#include "Utility.hpp"
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <csignal>
#include <algorithm>


int Utility::cut_str( char* s, int s_size, int ch )
{
	if (
				( s == nullptr )			||
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

void Utility::heap_make( int* values, int size, int ascending )
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

void Utility::heap_sort(int* values, int size, int ascending)
{
	heap_make( values, size, ascending );

	int i;
	for ( i = size-1; i > 0; i-- )
	{
		int temp = values[i];
		values[i] = values[0];
		values[0] = temp;

		heap_make( values, i, ascending );
	}
}

void Utility::delete_spaces( char* buffer, int* bufsize )
{
	if ( buffer == nullptr )
		return;

	if ( *bufsize < 2 )
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

void Utility::reverse( char* s )
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

int Utility::num_digit_cnt( int number )
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

void Utility::itoa( int number, char* num_buf, int max_buf_len )
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

int Utility::sendall( int fd, const char* buf, int* bufsize )
{
	int total = 0;
	int bytesleft = *bufsize;
	int n = -1;

	while ( total < *bufsize )
	{
		n = send( fd, buf+total, bytesleft, 0 );
		if ( n == -1 )
			break;
		total += n;
		bytesleft -= n;
	}
	*bufsize = total;

	return n == -1 ? -1 : 0;
}

int Utility::readline( int fd, char* buf, int bufsize )
{
	int total_read = 0;
	int rc = 0;
	int lf_flag = 0;

	do
	{
		rc = read( fd, buf+total_read, bufsize-total_read );
		if ( rc < 1 )
			return rc;

		for ( int i = total_read; i < total_read+rc; ++i )
			if ( buf[i] == '\n' )
				lf_flag = 1;
		total_read += rc;

		if ( ( total_read >= bufsize-1 ) && ( !lf_flag ) )
		{
			buf[total_read-1] = '\n';
			break;
		}
	}
	while ( !lf_flag );

	buf[total_read] = '\0';
	return total_read;
}

void Utility::concat_to_str( int number, char* number_buf, int number_len, char* str, int* str_offset )
{
	itoa( number, number_buf, number_len );
	strcpy(str + *str_offset, number_buf);
	*str_offset += strlen(number_buf);
	str[*str_offset] = '|';
	++(*str_offset);
}

int Utility::concat_tokens( char* buffer, int buffer_size, const char** tokens, int tokens_count )
{
	int i, j = 0;
	for ( i = 0; i < tokens_count; ++i )
	{
		for (; ( j < buffer_size-1 ) && tokens[i][j]; ++j )
			buffer[j] = tokens[i][j];
		buffer[j] = '|';
		++j;

		if ( j >= buffer_size-1 )
		{
			buffer[buffer_size-1] = '\0';
			return buffer_size-1;
		}
	}
	buffer[j-1] = '\0';

	return j-1;
}

std::string Utility::current_time_str()
{
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	std::tm tm {};
	localtime_r( &t, &tm ); // Для Linux/MacOS

	std::ostringstream oss;
	oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
		<< "." << std::setfill('0') << std::setw(3) << ms.count();

	return oss.str();
}

void Utility::ignore_unused_signals()
{
	struct sigaction old_act;

	int max_sig = SIGRTMAX;

	for ( int sig = 1; sig <= max_sig; ++sig )
	{
		if (
				sig == SIGKILL			||
				sig == SIGSTOP			||
				sig == SIGSEGV
			)
			continue;

		if ( sigaction( sig, nullptr, &old_act ) == 0 )
		{
			if ( old_act.sa_handler == SIG_DFL )
			{
				struct sigaction new_act;
				set_signal_disposition(new_act, sig, SIG_IGN, 0);
			}
		}
	}
}

void Utility::set_signal_disposition( struct sigaction& sa, int sig_no, sig_hndl_func shf, int flags )
{
	sa.sa_handler = shf;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = flags;
	sigaction(sig_no, &sa, nullptr);
}

// Убрать пробельные символы в конце строки
void Utility::rtrim( std::string& s )
{
	s.erase( std::find_if(s.rbegin(), s.rend(),
				[](unsigned char ch) { return !std::isspace(ch); } ).base(), s.end() );
}

#endif
