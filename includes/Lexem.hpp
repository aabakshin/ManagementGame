/* 
 *	Модуль Lexem предоставляет промежуточный объект-лексему, 
 *	которая служит внутренним представлением инструкций польз.
 *	скрипта на этапе лексического анализа
 *	Этот модуль вызывается в след. модулях: LexemAnalyzer, LexemList, SyntaxAnalyzer
 * */

#ifndef LEXEM_HPP
#define LEXEM_HPP

/* Константы, представляющие типы лексем */
enum
{
	LEXEM_TYPE_INTEGER,
	LEXEM_TYPE_STRING,
	LEXEM_TYPE_KEYWORD,
	LEXEM_TYPE_ASSIGN,
	LEXEM_TYPE_IDENTIFIER,
	LEXEM_TYPE_DELIMITER
};

struct Lexem
{
	int num_str;
	int lexem_type;
	char* lexem;
	int size;
public:
	Lexem(const char* str, int str_len, int num_str, int lexem_type);
	~Lexem();
	Lexem(const Lexem& obj);
	Lexem& operator=(const Lexem& obj);
private:
	Lexem() {}
};

#endif
