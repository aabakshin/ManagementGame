#ifndef UTILITY_HPP_SENTINEL
#define UTILITY_HPP_SENTINEL


#include <string>


class Utility
{
public:
	using sig_hndl_func = void (*)(int);

	Utility() {}
	static void heap_sort( int* values, int size, int ascending );
	static void delete_spaces( char* buffer, int* bufsize );
	static void itoa( int number, char* num_buf, int max_buf_len );
	static int readline( int fd, char* buf, int bufsize );
	static int sendall( int fd, const char* buf, int* bufsize );
	static int cut_str( char* s, int s_size, int ch );
	static void concat_to_str( int number, char* number_buf, int number_len, char* str, int* str_offset );
	static int concat_tokens( char* buffer, int buffer_size, const char** tokens, int tokens_count );
	static std::string current_time_str();
	static void ignore_unused_signals();
	static void set_signal_disposition( struct sigaction&, int, sig_hndl_func, int );
	static void rtrim( std::string& );
private:
	Utility( const Utility& ) = delete;
	Utility( Utility&& ) = delete;
	void operator=( const Utility& ) = delete;
	static void heap_make( int* values, int size, int ascending );
	static void reverse( char* s );
	static int num_digit_cnt( int number );
};

#endif
