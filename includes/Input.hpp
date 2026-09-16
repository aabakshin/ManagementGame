#ifndef INPUT_HPP_SENTINEL
#define INPUT_HPP_SENTINEL


#include <string>
#include <list>
#include <termios.h>
#include <array>
#include <functional>
#include <unordered_map>


class Input
{
public:

	enum
	{
				MAX_STR_COUNT				=							   10,
				MAX_BUFFER_SIZE				=			MAX_STR_COUNT * 2 + 2,
				MAX_SYM_CODE_SIZE			=								6,	// 6 - макс. размер в байтах кода клавиши на клавиатуре(F1-F12)
				CTRL_C						=								3,
				CTRL_D						=								4,
				BACKSPACE					=							  127,
				CTRL_W						=							   23,
				MAX_HISTORY_SIZE			=								5
	};

	enum class KeyEvent
	{
				ALPHABET,
				BACKSPACE,
				DEL,
				CTRLW,
				NEWLINE,
				ARROW_UP,
				ARROW_DOWN,
				ARROW_LEFT,
				ARROW_RIGHT
	};

	enum class SymbolTypes
	{
				INVAL		=  -1,
				NO_READ		=	0,
				ONE_BYTE	=	1,
				TWO_BYTE	=	2,
				THREE_BYTE	=	3,
				FOUR_BYTES	=	4
	};

	enum class PredicateTypes
	{
				IS_CTRL_C,
				IS_CTRL_D,
				IS_END,
				IS_BACKSPACE,
				IS_CTRL_W,
				IS_LATIN,
				IS_CYRIL,
				IS_ARROW_LEFT,
				IS_ARROW_RIGHT,
				IS_ARROW_UP,
				IS_ARROW_DOWN,
				IS_DEL
	};

	using KeyHandler = std::function<int( const Input& )>;
	using SymbolHandlers = std::function<bool()>;
	using Predicate = std::function<bool()>;
	using KeyActions = std::function<bool()>;

private:
	FILE* log_file;
	std::string log_filename { "/tmp/debug.log" };
	size_t debug_counter { 0 };
	termios t1, t2;
	std::list<std::string> input_history;
	std::list<std::string>::iterator ih_iter { nullptr };
	std::array<char, MAX_BUFFER_SIZE> current_input;
	std::array<char, MAX_BUFFER_SIZE> save_buf;
	bool save_buf_read { false };
	std::array<char, MAX_BUFFER_SIZE> result;
	std::unordered_map<KeyEvent, const char*> key_events_names;
	// буфер под прочитанный символ
	char read_sym[MAX_SYM_CODE_SIZE] = { 0 };
	// текущая позиция в current_input
	int cur_pos = 0;
	// смещение слева относительно конца строки
	int left_offset = 0;
	int ascii { 0 };
	int cyril { 0 };
	int total { 0 };
	std::unordered_map<KeyEvent, KeyHandler> key_handlers;
	std::unordered_map<SymbolTypes, SymbolHandlers> sym_handlers;
	std::unordered_map<PredicateTypes, Predicate> predicates;
	std::unordered_map<PredicateTypes, KeyActions> key_actions;
public:
	Input();
	~Input();
	int Readline();
	const char* GetReadSymbol() const { return read_sym; }
	int& GetCurPos() { return cur_pos; }
	int& GetLeftOffset() { return left_offset; }
	int GetInputSize() const { return current_input.size(); }
	std::array<char, MAX_BUFFER_SIZE>& GetInput() { return current_input; }
	std::array<char, MAX_BUFFER_SIZE>& GetSaveBuffer() { return save_buf; }
	std::array<char, MAX_BUFFER_SIZE>& GetResult() { return result; }
	std::list<std::string>& GetInputHistory() { return input_history; }
	std::list<std::string>::iterator& GetIterator() { return ih_iter; }
	int& GetAscii() { return ascii; }
	int& GetCyril() { return cyril; }
	int& GetTotal() { return total; }
	FILE* GetLogFile() const { return log_file; }
	const char* GetKeyEventName( KeyEvent key_code ) const { return key_events_names.at(key_code); }
	size_t GetDebugMessageCounter() const { return debug_counter; }
	void SetSaveBufFlag() { save_buf_read = true; }
	void UnsetSaveBufFlag() { save_buf_read = false; }
	bool IsSaveBufFlag() const { return save_buf_read; }
	int AsciiSymbolsCount( int = 0);
	int AsciiSymbolsCount( const char*, int );
	int CyrilSymbolsCount( int = 0);
	int TotalSymbolsCount( int = 0 );
	int IsCyrillicSymbol( const char* ) const;
	void CleanPrintedString( int );
	void ShowDebugLog( const char* ) const;

private:
	Input( const Input& ) = delete;
	Input( Input&& ) = delete;
	void operator=( const Input& ) = delete;
	void DisableCanonical();
	void EnableCanonical();
	bool SetNonblocking() const;
};

#endif
