#ifndef INPUT_CPP_SENTINEL
#define INPUT_CPP_SENTINEL


#include "Input.hpp"
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdexcept>


static int handle_alphabet_key( const Input& );
static int handle_ctrlw_key( const Input& );
static int handle_newline_key( const Input& );
static int handle_backspace_key( const Input& );
static int handle_arrow_left_key( const Input& );
static int handle_arrow_right_key( const Input& );
static int handle_arrow_up_key( const Input& );
static int handle_arrow_down_key( const Input& );
static int handle_del_key( const Input& );


Input::Input()
{
	if ( (log_file = fopen(log_filename.data(), "w+")) == nullptr )
		throw std::runtime_error(log_filename.data());

	memset( current_input.data(), 0, MAX_BUFFER_SIZE );
	memset( result.data(), 0, MAX_BUFFER_SIZE );

	ih_iter = input_history.end();

	DisableCanonical();
	SetNonblocking();

	key_actions[PredicateTypes::IS_CTRL_C]			=			[this](){ return false; };
	key_actions[PredicateTypes::IS_CTRL_D]			=			[this](){ return true; };
	key_actions[PredicateTypes::IS_END]				=			[this]()
	{
		if ( current_input.data()[0] == '\n' || current_input.data()[0] == '\0' )
			return true;

		++debug_counter;
		key_handlers[Input::KeyEvent::NEWLINE](*this);
#ifdef DEBUG
		ShowDebugLog( GetKeyEventName(Input::KeyEvent::NEWLINE) );
#endif
		return true;
	};
	key_actions[PredicateTypes::IS_BACKSPACE]		=			[this]()
	{
		++debug_counter;
		key_handlers[Input::KeyEvent::BACKSPACE](*this);
#ifdef DEBUG
		ShowDebugLog( GetKeyEventName(Input::KeyEvent::BACKSPACE) );
#endif
		return false;
	};
	key_actions[PredicateTypes::IS_CTRL_W]			=			[this]()
	{
		++debug_counter;
		key_handlers[Input::KeyEvent::CTRLW](*this);
#ifdef DEBUG
		ShowDebugLog( GetKeyEventName(Input::KeyEvent::CTRLW) );
#endif
		return false;
	};
	key_actions[PredicateTypes::IS_LATIN]			=			[this]()
	{
		if ( total < Input::MAX_STR_COUNT )
			if ( ++debug_counter; !key_handlers[Input::KeyEvent::ALPHABET](*this) )
				return true;

		return false;
	};
	key_actions[PredicateTypes::IS_CYRIL]			=			[this]()
	{
		if ( total < Input::MAX_STR_COUNT )
			if ( IsCyrillicSymbol(read_sym) )
				if ( ++debug_counter; !key_handlers[Input::KeyEvent::ALPHABET](*this) )
					return true;

		return false;
	};
	key_actions[PredicateTypes::IS_ARROW_LEFT]		=			[this]()
	{
		++debug_counter;
		key_handlers[Input::KeyEvent::ARROW_LEFT](*this);
#ifdef DEBUG
		ShowDebugLog( GetKeyEventName(Input::KeyEvent::ARROW_LEFT) );
#endif
		return false;
	};
	key_actions[PredicateTypes::IS_ARROW_RIGHT]		=			[this]()
	{
		++debug_counter;
		key_handlers[Input::KeyEvent::ARROW_RIGHT](*this);
#ifdef DEBUG
		ShowDebugLog( GetKeyEventName(Input::KeyEvent::ARROW_RIGHT) );
#endif
		return false;
	};
	key_actions[PredicateTypes::IS_ARROW_UP]		=			[this]()
	{
		++debug_counter;
		key_handlers[Input::KeyEvent::ARROW_UP](*this);
#ifdef DEBUG
		ShowDebugLog( GetKeyEventName(Input::KeyEvent::ARROW_UP) );
#endif
		return false;
	};
	key_actions[PredicateTypes::IS_ARROW_DOWN]		=			[this]()
	{
		++debug_counter;
		key_handlers[Input::KeyEvent::ARROW_DOWN](*this);
#ifdef DEBUG
		ShowDebugLog( GetKeyEventName(Input::KeyEvent::ARROW_DOWN) );
#endif
		return false;
	};
	key_actions[PredicateTypes::IS_DEL]		=			[this]()
	{
		++debug_counter;
		key_handlers[Input::KeyEvent::DEL](*this);
#ifdef DEBUG
		ShowDebugLog( GetKeyEventName(Input::KeyEvent::DEL) );
#endif
		return false;
	};


	predicates[PredicateTypes::IS_CTRL_C]			=			[this](){ return read_sym[0] == CTRL_C; };
	predicates[PredicateTypes::IS_CTRL_D]			=			[this](){ return read_sym[0] == CTRL_D; };
	predicates[PredicateTypes::IS_END]				=			[this](){ return ( read_sym[0] == '\t' ) || ( read_sym[0] == '\n' ); };
	predicates[PredicateTypes::IS_BACKSPACE]		=			[this](){ return ( read_sym[0] == '\b' ) || ( read_sym[0] == BACKSPACE ); };
	predicates[PredicateTypes::IS_CTRL_W]			=			[this](){ return read_sym[0] == CTRL_W; };
	predicates[PredicateTypes::IS_LATIN]			=			[this](){ return ( read_sym[0] >= 32 ) && ( read_sym[0] <= 126 ); };
	predicates[PredicateTypes::IS_CYRIL]			=			[this](){ return ( total < Input::MAX_STR_COUNT ) && IsCyrillicSymbol(read_sym); };
	predicates[PredicateTypes::IS_ARROW_LEFT]		=			[this]()
	{
		// обработка клавиши ARROW_LEFT с 3-байтным кодом
		return
					( read_sym[0] == 0x1b )		&&		/* 27 */
					( read_sym[1] == 0x5b )		&&		/* 91 */
					( read_sym[2] == 0x44 )				/* 68 */
		;
	};
	predicates[PredicateTypes::IS_ARROW_RIGHT]		=			[this]()
	{
		// обработка клавиши ARROW_RIGHT с 3-байтным кодом
		return
					( read_sym[0] == 0x1b )		&&		/* 27 */
					( read_sym[1] == 0x5b )		&&		/* 91 */
					( read_sym[2] == 0x43 )				/* 67 */
		;
	};
	predicates[PredicateTypes::IS_ARROW_UP]			=			[this]()
	{
		// обработка клавиши ARROW_UP с 3-байтным кодом
		return
					( read_sym[0] == 0x1b )		&&		/* 27 */
					( read_sym[1] == 0x5b )		&&		/* 91 */
					( read_sym[2] == 0x41 )				/* 65 */
		;
	};
	predicates[PredicateTypes::IS_ARROW_DOWN]		=			[this]()
	{
		// обработка клавиши ARROW_DOWN с 3-байтным кодом
		return
					( read_sym[0] == 0x1b )		&&		/* 27 */
					( read_sym[1] == 0x5b )		&&		/* 91 */
					( read_sym[2] == 0x42 )				/* 66 */
		;
	};
	predicates[PredicateTypes::IS_DEL]				=			[this]()
	{
		// обработка клавиши DEL с 4-х байтным кодом
		return
					( read_sym[0] == 0x1b )		&&		/* 27 */
					( read_sym[1] == 0x5b )		&&		/* 91 */
					( read_sym[2] == 0x33 )		&&		/* 51 */
					( read_sym[3] == 0x7e )				/* 126 */
		;
	};


	sym_handlers[Input::SymbolTypes::INVAL]				=		[]() { return false; };
	sym_handlers[Input::SymbolTypes::NO_READ]			=		[](){ return false; };
	sym_handlers[Input::SymbolTypes::ONE_BYTE]			=		[this]()
	{
		int current = static_cast<int>(PredicateTypes::IS_CTRL_C);
		int end = static_cast<int>(PredicateTypes::IS_LATIN);
		for ( ; current <= end; ++current )
		{
			PredicateTypes idx = static_cast<PredicateTypes>(current);
			if ( predicates[idx]() )
				return key_actions[idx]();
		}

		return false;
		//throw UnknownKeyAction();
	};
	sym_handlers[Input::SymbolTypes::TWO_BYTE]			=		[this]()
	{
		if ( predicates[PredicateTypes::IS_CYRIL] )
			return key_actions[PredicateTypes::IS_CYRIL]();

		return false;
	};
	sym_handlers[Input::SymbolTypes::THREE_BYTE]		=		[this]()
	{
		int current = static_cast<int>(PredicateTypes::IS_ARROW_LEFT);
		int end = static_cast<int>(PredicateTypes::IS_ARROW_DOWN);
		for ( ; current <= end; ++current )
		{
			PredicateTypes idx = static_cast<PredicateTypes>(current);
			if ( predicates[idx]() )
				return key_actions[idx]();
		}

		return false;
		//throw UnknownKeyAction();
	};
	sym_handlers[Input::SymbolTypes::FOUR_BYTES]		=		[this]()
	{
		if ( predicates[PredicateTypes::IS_DEL] )
			return key_actions[PredicateTypes::IS_DEL]();

		return false;
	};

	key_handlers[Input::KeyEvent::ALPHABET]			=			handle_alphabet_key;
	key_handlers[Input::KeyEvent::CTRLW]			=			handle_ctrlw_key;
	key_handlers[Input::KeyEvent::NEWLINE]			=			handle_newline_key;
	key_handlers[Input::KeyEvent::BACKSPACE]		=			handle_backspace_key;
	key_handlers[Input::KeyEvent::ARROW_LEFT]		=			handle_arrow_left_key;
	key_handlers[Input::KeyEvent::ARROW_RIGHT]		=			handle_arrow_right_key;
	key_handlers[Input::KeyEvent::ARROW_UP]			=			handle_arrow_up_key;
	key_handlers[Input::KeyEvent::ARROW_DOWN]		=			handle_arrow_down_key;
	key_handlers[Input::KeyEvent::DEL]				=			handle_del_key;

	key_events_names[Input::KeyEvent::ALPHABET]		=			"KeyEvent::ALPHABET";
	key_events_names[Input::KeyEvent::CTRLW]		=			"KeyEvent::CTRW";
	key_events_names[Input::KeyEvent::NEWLINE]		=			"KeyEvent::NEWLINE";
	key_events_names[Input::KeyEvent::BACKSPACE]	=			"KeyEvent::BACKSPACE";
	key_events_names[Input::KeyEvent::ARROW_LEFT]	=			"KeyEvent::ARROW_LEFT";
	key_events_names[Input::KeyEvent::ARROW_RIGHT]	=			"KeyEvent::ARROW_RIGHT";
	key_events_names[Input::KeyEvent::ARROW_UP]		=			"KeyEvent::ARROW_UP";
	key_events_names[Input::KeyEvent::ARROW_DOWN]	=			"KeyEvent::ARROW_DOWN";
	key_events_names[Input::KeyEvent::DEL]			=			"KeyEvent::DEL";
}

Input::~Input()
{
	EnableCanonical();

	if ( log_file != nullptr )
		fclose( log_file );
}

void Input::ShowDebugLog( const char* key_event_type ) const
{
	auto show_buffer = [this] ( size_t buf_len, const std::array<char, MAX_BUFFER_SIZE>& input )
	{
		for ( size_t i = 0; i < buf_len; ++i )
			fputc( input.data()[i], GetLogFile() );
		fputc( '\n', GetLogFile() );
	};

	auto show_buffer_bytes = [this] ( size_t buf_len, const std::array<char, MAX_BUFFER_SIZE>& input )
	{
		for ( size_t i = 0; i < buf_len; ++i )
			fprintf(GetLogFile(), "%4d ", input.data()[i]);
		fputc( '\n', GetLogFile() );
	};


	const char* result = const_cast<Input&>(*this).GetResult().data();

	fprintf(GetLogFile(), "[%lu][%s]:\n"
								"result: ",
								GetDebugMessageCounter(),
								key_event_type
								);
	show_buffer( strlen(result), const_cast<Input&>(*this).GetResult() );

	fprintf( GetLogFile(), "%s", "result_bytes: " );
	show_buffer_bytes( strlen(result), const_cast<Input&>(*this).GetResult() );


	const char* buffer = const_cast<Input&>(*this).GetInput().data();

	fprintf(GetLogFile(), "%s", "buffer: ");
	show_buffer( strlen(buffer), const_cast<Input&>(*this).GetInput() );

	fprintf( GetLogFile(), "%s", "buffer_bytes: " );
	show_buffer_bytes( strlen(buffer), const_cast<Input&>(*this).GetInput() );

	fprintf(GetLogFile(),
								"cur_pos = %d\n"
								"left_offset = %d\n"
								"buffer length = %lu\n"
								"buffer size = %d\n"
								"ascii = %d\n"
								"cyril = %d\n"
								"total = %d\n\n\n\n",
								cur_pos,
								left_offset,
								strlen(buffer),
								GetInputSize(),
								ascii,
								cyril,
								ascii + cyril
								);


}

void Input::DisableCanonical()
{
	tcgetattr( STDIN_FILENO, &t1 );
	memcpy( &t2, &t1, sizeof(t1) );

	t1.c_lflag &= ~ICANON;
	t1.c_lflag &= ~ISIG;
	t1.c_lflag &= ~ECHO;
	t1.c_cc[VMIN] = 0;
	t1.c_cc[VTIME] = 0;

	tcsetattr( STDIN_FILENO, TCSANOW, &t1 );
}

void Input::EnableCanonical()
{
	tcsetattr( STDIN_FILENO, TCSANOW, &t2 );
}

bool Input::SetNonblocking() const
{
	const int flag = 1;

	if ( ioctl( STDIN_FILENO, FIONBIO, const_cast<int*>(&flag)) < 0 )
		return false;

	return true;
}

int Input::Readline()
{
	while ( true )
	{
		bool return_flag = false;
		TotalSymbolsCount();

		memset(read_sym, 0, sizeof(read_sym));
		int rc = read(0, read_sym, MAX_SYM_CODE_SIZE);

		const SymbolTypes type = static_cast<SymbolTypes>(rc);
		if ( (type >= SymbolTypes::INVAL) && (type <= SymbolTypes::FOUR_BYTES) )
		{
			return_flag = sym_handlers[type]();
		}
		else
		{
			return -1;
			// throw
		}

		if ( return_flag )
			break;
	}

	// Байтовое смещение current_input относительно начала буфера
	return cur_pos;
}

int Input::IsCyrillicSymbol( const char* symbol ) const
{
	// UTF-16LE
	const char rus_alpha_codes[] = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя";
	int len = strlen(rus_alpha_codes);

	for ( int j = 0; j < len/2; j++ )
		if ( symbol[0] == rus_alpha_codes[j*2] )
			if ( symbol[1] == rus_alpha_codes[j*2+1] )
				return 1;

	return 0;
}

int Input::AsciiSymbolsCount( int idx )
{
	int local_ascii = 0;

	if ( idx == 0 )
		ascii = 0;

	const char* buffer = current_input.data();

	for ( size_t i = idx; i < static_cast<size_t>(cur_pos); ++i )
	{
		if ( ( buffer[i] > 0 ) && ( buffer[i] <= 127 ) )
		{
			if ( idx == 0 )
			{
				++ascii;
			}
			else
			{
				++local_ascii;
			}
		}
	}

	return ( idx == 0 ) ? ascii : local_ascii;
}

int Input::AsciiSymbolsCount( const char* buf, int buf_size )
{
	int counter = 0;

	for ( int i = 0; i < buf_size; ++i )
		if ( ( buf[i] > 0 ) && ( buf[i] <= 127 ) )
			++counter;

	return counter;
}

int Input::CyrilSymbolsCount( int idx )
{
	int local_cyril = 0;

	if ( idx == 0 )
		cyril = ( cur_pos - idx - ascii ) / 2;
	else
		local_cyril = ( cur_pos - idx - AsciiSymbolsCount(idx) ) / 2;

	return ( idx == 0 ) ? cyril : local_cyril;
}

int Input::TotalSymbolsCount( int idx )
{
	int local_total = 0;

	if ( idx == 0 )
		total = cyril + ascii;
	else
	{
		local_total = AsciiSymbolsCount(idx);
		local_total += CyrilSymbolsCount(idx);
	}

	return ( idx == 0 ) ? total : local_total;
}

void Input::CleanPrintedString( int idx )
{
	int t = TotalSymbolsCount( idx );

	for ( int i = 1; i <= t; ++i )
		putchar(' ');

	t = total;
	while ( t > 0 )
	{
		printf("%s", "\b \b");
		--t;
	}
	fflush(stdout);
}

static int handle_alphabet_key( const Input& input )
{
	bool cyril_flag = false;

	if ( input.IsCyrillicSymbol( input.GetReadSymbol() ) )
		cyril_flag = true;

	const_cast<Input&>(input).UnsetSaveBufFlag();

	int save_pos = 0;
	int& left_offset = const_cast<Input&>(input).GetLeftOffset();
	int& cur_pos = const_cast<Input&>(input).GetCurPos();
	char* buffer = const_cast<Input&>(input).GetInput().data();
	int& ascii = const_cast<Input&>(input).GetAscii();
	int& cyril = const_cast<Input&>(input).GetCyril();


	if ( left_offset > 0 )
	{
		int last_ch = cur_pos - 1;
		cur_pos -= left_offset;
		save_pos = cur_pos;

		char remaining_buf[Input::MAX_BUFFER_SIZE];
		int j = 0;
		for ( int x = cur_pos; x <= last_ch; ++x, ++j )
			remaining_buf[j] = buffer[x];
		remaining_buf[j] = '\0';

		buffer[cur_pos] = input.GetReadSymbol()[0];
		++cur_pos;
		++ascii;
		++last_ch;

		if ( cyril_flag )
		{
			buffer[cur_pos] = input.GetReadSymbol()[1];
			++cur_pos;
			++last_ch;
			++cyril;
			--ascii;
		}

		for ( int x = 0; remaining_buf[x]; ++x )
		{
			buffer[cur_pos] = remaining_buf[x];
			++cur_pos;
		}
		buffer[cur_pos] = '\0';

		// вывод содержимого buffer начиная с вставленного элемента
		for ( int x = save_pos; x <= last_ch; ++x )
			write(1, &buffer[x], 1);


		// возвращение курсора в прежнее положение после вставки очереднего символа
		const_cast<Input&>(input).TotalSymbolsCount();
		for( int x = 1; x <= const_cast<Input&>(input).GetTotal(); ++x )
			putchar('\b');
		fflush(stdout);

		++save_pos;

		if ( cyril_flag )
			++save_pos;

		for ( int x = 0; x < save_pos; ++x )
			write(1, &buffer[x], 1);
	}
	else
	{
		buffer[cur_pos] = input.GetReadSymbol()[0];
		++cur_pos;
		++ascii;

		if ( cyril_flag )
		{
			buffer[cur_pos] = input.GetReadSymbol()[1];
			++cur_pos;
			++cyril;
			--ascii;
		}

		write(1, &input.GetReadSymbol()[0], 1 );
		if ( cyril_flag )
			write( 1, &input.GetReadSymbol()[1], 1 );
	}

#ifdef DEBUG
	input.ShowDebugLog( input.GetKeyEventName(Input::KeyEvent::ALPHABET) );
#endif

	return 1;
}

static int handle_ctrlw_key( const Input& input )
{
	int& left_offset = const_cast<Input&>(input).GetLeftOffset();
	int& c_pos = const_cast<Input&>(input).GetCurPos();
	char* buffer = const_cast<Input&>(input).GetInput().data();
	int& ascii = const_cast<Input&>(input).GetAscii();
	int& cyril = const_cast<Input&>(input).GetCyril();

	if ( c_pos < 1 )
		return 0;

	int last_ch = c_pos - 1;
	int cur_pos = c_pos - left_offset;
	int save_pos = cur_pos;

	if ( left_offset < c_pos )
		const_cast<Input&>(input).UnsetSaveBufFlag();

	if ( cur_pos > 0 )
	{
		if ( buffer[cur_pos-1] == ' ' )
		{
			while ( (cur_pos > 0) && (buffer[cur_pos-1] == ' ')  )
				--cur_pos;
		}
		else
		{
			while ( (cur_pos > 0) && (buffer[cur_pos-1] != ' ') )
				--cur_pos;
		}


		int del_bytes = save_pos - cur_pos;

		char buf[Input::MAX_BUFFER_SIZE];
		int x = 0;
		for ( int k = cur_pos; k < save_pos; ++k, ++x )
			buf[x] = buffer[k];
		buf[x] = '\0';

		int buf_len = strlen(buf);
		int ascii_cnt = const_cast<Input&>(input).AsciiSymbolsCount(buf, buf_len+1);
		int cyril_cnt = (buf_len - ascii_cnt) / 2;
		int buf_cnt = ascii_cnt + cyril_cnt;
		memset(buf, 0, Input::MAX_BUFFER_SIZE);

		ascii -= ascii_cnt;
		cyril -= cyril_cnt;

		for ( int k = 1; k <= buf_cnt; ++k )
		{
			printf("\b \b");
			fflush(stdout);
		}


		x = 0;
		for ( int k = save_pos; k <= last_ch; ++k, ++x )
			buf[x] = buffer[k];
		buf[x] = '\0';

		buf_len = strlen(buf);
		ascii_cnt = const_cast<Input&>(input).AsciiSymbolsCount(buf, buf_len+1);
		cyril_cnt = (buf_len - ascii_cnt) / 2;
		buf_cnt = ascii_cnt + cyril_cnt;

		int v = cur_pos;
		for ( x = 0; buf[x]; ++x, ++v )
		{
			buffer[v] = buf[x];
			putchar(buffer[v]);
		}
		fflush(stdout);

		for ( x = v; x <= last_ch; ++x )
		{
			putchar(' ');
			buffer[x] = '\0';
		}

		int total_cnt = last_ch - v + 1 + buf_cnt;
		for ( x = 1; x <= total_cnt; ++x )
			putchar('\b');
		fflush(stdout);

		c_pos -= del_bytes;
	}

	return 1;
}

static int handle_newline_key( const Input& input )
{
	int& cur_pos = const_cast<Input&>(input).GetCurPos();
	int& left_offset = const_cast<Input&>(input).GetLeftOffset();
	int& ascii = const_cast<Input&>(input).GetAscii();
	int& cyril = const_cast<Input&>(input).GetCyril();
	int& total = const_cast<Input&>(input).GetTotal();

	std::array<char, Input::MAX_BUFFER_SIZE>& buffer = const_cast<Input&>(input).GetInput();
	char* result = const_cast<Input&>(input).GetResult().data();
	std::list<std::string>& input_history = const_cast<Input&>(input).GetInputHistory();
	std::list<std::string>::iterator& ih_iter = const_cast<Input&>(input).GetIterator();

	const_cast<Input&>(input).UnsetSaveBufFlag();

	buffer[cur_pos] = '\n';
	++cur_pos;
	buffer[cur_pos] = '\0';

	strncpy(result, buffer.data(), Input::MAX_BUFFER_SIZE);
	buffer[cur_pos-1] = '\0';
	--cur_pos;

	if ( ( buffer[0] == '\0' ) || ( buffer[0] == '\n' ) || ( buffer[0] == '\r' ) )
	{
		ih_iter = input_history.end();
		return 1;
	}

	if ( input_history.size() >= Input::MAX_HISTORY_SIZE )
		input_history.pop_front();

	input_history.push_back( buffer.data() );
	ih_iter = input_history.end();

	const_cast<Input&>(input).CleanPrintedString( cur_pos - left_offset );

	memset( buffer.data(), 0, Input::MAX_BUFFER_SIZE );
	cur_pos = 0;
	left_offset = 0;
	ascii = 0;
	cyril = 0;
	total = 0;

	return 1;
}

static int handle_backspace_key( const Input& input )
{
	int& left_offset = const_cast<Input&>(input).GetLeftOffset();
	int& c_pos = const_cast<Input&>(input).GetCurPos();
	char* buffer = const_cast<Input&>(input).GetInput().data();
	int& ascii = const_cast<Input&>(input).GetAscii();
	int& cyril = const_cast<Input&>(input).GetCyril();


	if ( (left_offset >= 0) && (left_offset < c_pos) )
	{
		const_cast<Input&>(input).UnsetSaveBufFlag();

		char buf[Input::MAX_BUFFER_SIZE];
		// проверка, является ли удаляемый символ кириллическим
		bool is_cyril_flag = false;
		int last_ch_pos = c_pos - 1;

		if ( left_offset <= 0 )
		{
			if ( c_pos >= 2 )
			{
				char sym[3] = { 0 };
				sym[0] = buffer[c_pos-2];
				sym[1] = buffer[c_pos-1];
				sym[2] = '\0';

				if ( input.IsCyrillicSymbol(sym) )
					is_cyril_flag = true;
			}

			printf("%s", "\b \b");
			fflush(stdout);
			buffer[c_pos-1] = '\0';
			if ( is_cyril_flag )
				buffer[c_pos-2] = '\0';

			--c_pos;
			--ascii;
			if ( is_cyril_flag )
			{
				--c_pos;
				--cyril;
				++ascii;
			}

			return 1;
		}

		int cur_pos = c_pos - left_offset;

		if ( cur_pos >= 2 )
		{
			char sym[3] = { 0 };
			sym[0] = buffer[cur_pos-2];
			sym[1] = buffer[cur_pos-1];
			sym[2] = '\0';

			if ( input.IsCyrillicSymbol(sym) )
				is_cyril_flag = true;
		}

		int x, z = 0;
		for ( x = cur_pos; x <= last_ch_pos; ++x, ++z )
			buf[z] = buffer[x];
		buf[z] = '\0';


		x = cur_pos - 1;
		if ( is_cyril_flag )
			--x;

		putchar('\b');
		for ( z = 0; buf[z]; ++z, ++x )
		{
			buffer[x] = buf[z];
			putchar(buffer[x]);
		}
		putchar(' ');
		putchar('\b');
		buffer[x] = '\0';
		fflush(stdout);


		int buf_len = strlen(buf);
		int ascii_cnt = const_cast<Input&>(input).AsciiSymbolsCount(buf, buf_len+1);
		int cyril_cnt = (buf_len - ascii_cnt) / 2;
		int total_cnt = ascii_cnt + cyril_cnt;

		for ( x = 1; x <= total_cnt; ++x )
			putchar('\b');
		fflush(stdout);


		if ( c_pos > 0 )
		{
			--c_pos;
			--ascii;

			if ( is_cyril_flag )
			{
				--c_pos;
				++ascii;
				--cyril;
			}
		}
	}

	return 1;
}

static int handle_arrow_left_key( const Input& input )
{
	int& left_offset = const_cast<Input&>(input).GetLeftOffset();
	int& cur_pos = const_cast<Input&>(input).GetCurPos();
	char* buffer = const_cast<Input&>(input).GetInput().data();

	// если текущая позиция буфера не в начале строки - перемещать курсор влево
	if ( left_offset < cur_pos )
	{
		putchar('\b');
		fflush(stdout);

		int x = cur_pos - left_offset - 1;

		if ( x > 0 )
		{
			char sym[3] =
			{
				buffer[x-1],
				buffer[x],
				'\0'
			};

			if ( input.IsCyrillicSymbol(sym) )
			{
				left_offset += 2;
			}
			else
			{
				++left_offset;
			}

			return 1;
		}

		if ( x == 0 )
			++left_offset;
	}

	return 1;
}

static int handle_arrow_right_key( const Input& input )
{
	int& left_offset = const_cast<Input&>(input).GetLeftOffset();
	int& cur_pos = const_cast<Input&>(input).GetCurPos();
	char* buffer = const_cast<Input&>(input).GetInput().data();

	// если не конец строки - перемещать курсор вправо
	if ( left_offset > 0 )
	{
		int x = cur_pos - left_offset;

		if ( left_offset > 1 )
		{
			char sym[3] =
			{
				buffer[x],
				buffer[x+1],
				'\0'
			};

			if ( input.IsCyrillicSymbol(sym) )
			{
				write(1, sym, 2);
				left_offset -= 2;
			}
			else
			{
				putchar(buffer[cur_pos - left_offset]);
				fflush(stdout);
				--left_offset;
			}

			return 1;
		}

		if ( left_offset == 1 )
		{
			putchar(buffer[cur_pos - left_offset]);
			fflush(stdout);
			--left_offset;
		}
	}

	return 1;
}

static int handle_arrow_up_key( const Input& input )
{
	std::list<std::string>& input_history = const_cast<Input&>(input).GetInputHistory();
	int& cur_pos = const_cast<Input&>(input).GetCurPos();
	int& left_offset = const_cast<Input&>(input).GetLeftOffset();
	char* buffer = const_cast<Input&>(input).GetInput().data();
	std::list<std::string>::iterator& ih_iter = const_cast<Input&>(input).GetIterator();
	std::array<char, Input::MAX_BUFFER_SIZE>& save_buf = const_cast<Input&>(input).GetSaveBuffer();


	if ( input_history.size() > 0 )
	{
		if ( !input.IsSaveBufFlag() )
		{
			memset(save_buf.data(), 0, Input::MAX_BUFFER_SIZE);
			strncpy(save_buf.data(), buffer, cur_pos);
			const_cast<Input&>(input).SetSaveBufFlag();
		}

		if ( ih_iter != input_history.begin() )
			ih_iter = std::prev(ih_iter);
		else
			return 1;

		const_cast<Input&>(input).CleanPrintedString( cur_pos - left_offset );

		memset(buffer, 0, Input::MAX_BUFFER_SIZE);
		cur_pos = 0;
		for ( int j = 0; ih_iter->data()[j]; ++j, ++cur_pos )
		{
			buffer[cur_pos] = ih_iter->data()[j];
			putchar(ih_iter->data()[j]);
		}
		fflush(stdout);

		left_offset = 0;
		const_cast<Input&>(input).AsciiSymbolsCount();
		const_cast<Input&>(input).CyrilSymbolsCount();
	}

	return 1;
}

static int handle_arrow_down_key( const Input& input )
{
	std::list<std::string>& input_history = const_cast<Input&>(input).GetInputHistory();
	int& cur_pos = const_cast<Input&>(input).GetCurPos();
	int& left_offset = const_cast<Input&>(input).GetLeftOffset();
	char* buffer = const_cast<Input&>(input).GetInput().data();
	std::list<std::string>::iterator& ih_iter = const_cast<Input&>(input).GetIterator();
	std::array<char, Input::MAX_BUFFER_SIZE>& save_buf = const_cast<Input&>(input).GetSaveBuffer();


	if ( input_history.size() > 0 )
	{
		std::string output;

		if ( ih_iter != input_history.end() )
		{
			ih_iter = std::next(ih_iter);

			if ( ih_iter != input_history.end() )
				output = ih_iter->data();
		}

		if ( ih_iter == input_history.end() )
			output = save_buf.data();

		const_cast<Input&>(input).CleanPrintedString( cur_pos - left_offset );

		memset(buffer, 0, Input::MAX_BUFFER_SIZE);
		cur_pos = 0;
		for ( int j = 0; output[j]; ++j, ++cur_pos )
		{
			buffer[cur_pos] = output[j];
			putchar(output[j]);
		}
		fflush(stdout);

		left_offset = 0;
		const_cast<Input&>(input).AsciiSymbolsCount();
		const_cast<Input&>(input).CyrilSymbolsCount();
	}

	return 1;
}

static int handle_del_key( const Input& input )
{
	int& left_offset = const_cast<Input&>(input).GetLeftOffset();
	int& c_pos = const_cast<Input&>(input).GetCurPos();
	char* buffer = const_cast<Input&>(input).GetInput().data();
	int& ascii = const_cast<Input&>(input).GetAscii();
	int& cyril = const_cast<Input&>(input).GetCyril();

	// если не конец строки
	if ( left_offset > 0 )
	{
		const_cast<Input&>(input).UnsetSaveBufFlag();

		// проверка, является ли удаляемый символ кириллическим
		bool is_cyril_flag = false;

		char buf[Input::MAX_BUFFER_SIZE];
		int last_ch_pos = c_pos - 1;
		int cur_pos = c_pos - left_offset;

		if ( (cur_pos + 1) < c_pos )
		{
			char sym[3] =
			{
						buffer[cur_pos],
						buffer[cur_pos+1],
						'\0'
			};

			if ( input.IsCyrillicSymbol(sym) )
			{
				is_cyril_flag = true;
			}
		}

		int k = cur_pos + 1;

		if ( is_cyril_flag )
			++k;

		int x = 0;
		for ( ; k <= last_ch_pos; ++k, ++x )
			buf[x] = buffer[k];
		buf[x] = '\0';


		x = 0;
		for ( k = cur_pos; buf[x]; ++x, ++k )
		{
			buffer[k] = buf[x];
			putchar(buffer[k]);
		}
		buffer[k] = '\0';
		putchar(' ');
		putchar('\b');
		fflush(stdout);


		int buf_len = strlen(buf);
		int ascii_cnt = const_cast<Input&>(input).AsciiSymbolsCount(buf, buf_len+1);
		int cyril_cnt = (buf_len - ascii_cnt) / 2;
		int total_cnt = ascii_cnt + cyril_cnt;

		for ( x = 1; x <= total_cnt; ++x )
			putchar('\b');
		fflush(stdout);


		if ( c_pos > 0 )
		{
			--c_pos;
			--ascii;

			if ( is_cyril_flag )
			{
				--c_pos;
				++ascii;
				--cyril;
			}
		}

		--left_offset;

		if ( is_cyril_flag )
			--left_offset;
	}

	return 1;
}

#endif
