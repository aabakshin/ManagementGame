/* Файл реализации модуля LexemAnalyzer */

#ifndef LEXEM_ANALYZER_CPP
#define LEXEM_ANALYZER_CPP

#ifndef __CPP_FILE__
#define __CPP_FILE__
#endif

#include "../includes/LexemAnalyzer.hpp"
#include "../includes/LexemList.hpp"
#include "../includes/Lexem.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* Вспомогательная функция, определяющая, является ли текущий символ валидным разделителем */
static int is_delimiter_symbol(const char* delimiters, int ch)
{
	if ( delimiters == NULL )
	{
		return 0;
	}

	while ( *delimiters != '\0' )
	{
		if ( ch == *delimiters )
			return 1;

		delimiters++;
	}
	
	return 0;
}

LexemAnalyzer::LexemAnalyzer()
{
	if ( BUFSIZE < 1 )
		size = 1024;
	else
		size = BUFSIZE;
	
	buffer = new char[BUFSIZE];
	if ( buffer == NULL )
		fprintf(stderr, "%s", "Unable to allocate memory to \"buf\" string\n");
	
	for ( int i = 0; i < BUFSIZE; i++ )
		buffer[i] = 0;

	cur_state = START;
}

LexemList* LexemAnalyzer::Run(FILE* fd)
{
	LexemList* list = NULL;	
	
	/*struct stat file_stat;
	fstat(fd, &file_stat);
	int size = file_stat.st_size;
	printf("size = %d bytes\n", size);
	*/

	int lines = 1;
	int ch;
	int i = 0;
	do
	{
		ch = fgetc(fd);
		
		if ( ch == EOF )
		{
			cur_state = START;
			break;
		}

		if ( ch == '\n' )
			lines++;

		if ( cur_state == START )
		{
			const char* delimiters = "+-/*%<>=&|!,;()";
			if ( (ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r') || (ch == 4) )
			{
				continue;
			}
			
			if ( (ch >= '0') && (ch <= '9') )
			{
				cur_state = NUMBER;
				buffer[i] = ch;
				i++;
				continue;
			}

			if ( (ch == '?') || (ch == '@') || (ch == '$') )
			{
				cur_state = IDENTIFIER;
				buffer[i] = ch;
				i++;
				continue;
			}

			if ( ( (ch >= 'A') && (ch <= 'Z') ) || ( (ch >= 'a') && (ch <= 'z') ) )
			{
				cur_state = KEYWORD;
				buffer[i] = ch;
				i++;
				continue;
			}

			if ( ch == ':' )
			{
				cur_state = ASSIGN;
				buffer[i] = ch;
				i++;
				continue;
			}
			
			if ( ch == '"' )
			{
				cur_state = STRING;
				buffer[i] = ch;
				i++;
				continue;
			}

			if ( is_delimiter_symbol(delimiters, ch) )
			{
				buffer[0] = ch;
				buffer[1] = '\0';
				
				ll_insert(&list, new Lexem(buffer, 1, lines, LEXEM_TYPE_DELIMITER));

				for (int j = 0; buffer[j]; j++ )
					buffer[j] = '\0';
				i = 0;
				continue;
			}
			continue;
		}
		
		if ( cur_state == NUMBER )
		{
			if ( (ch >= '0') && (ch <= '9') )
			{
				buffer[i] = ch;
				i++;
				continue;
			}

			buffer[i] = '\0';
			const char* delimeters = " \t\n\r+-*/%,;:<>=";
			ungetc(ch, fd);
			if ( is_delimiter_symbol(delimeters, ch) )
			{
				ll_insert(&list, new Lexem(buffer, i, lines, LEXEM_TYPE_INTEGER));
			}
			else
			{
				fprintf(stderr, "\n[LEXICAL_ANALYZER] Line #%d: An error with lexem - expected a number!\n", lines);
			}

			for ( int j = 0; buffer[j]; j++ )
				buffer[j] = '\0';
			i = 0;
			cur_state = START;
			continue;
		}
		
		if ( cur_state == IDENTIFIER )
		{
			if ( 
					( (ch >= '0') && (ch <= '9') ) ||
					( (ch >= 'A') && (ch <= 'Z') ) ||
					( (ch >= 'a') && (ch <= 'z') ) ||
					( ch == '_' )
			   )
			{
				buffer[i] = ch;
				i++;
				continue;
			}
			
			buffer[i] = '\0';

			const char* delimeters = " \t\n\r+-*/%,;:<>=()";
			if ( is_delimiter_symbol(delimeters, ch) )
			{
				ll_insert(&list, new Lexem(buffer, i, lines, LEXEM_TYPE_IDENTIFIER));
				
				if ( ch == ':' )
				{
					buffer[0] = ch;
					buffer[1] = '\0';
					
					ll_insert(&list, new Lexem(buffer, 1, lines, LEXEM_TYPE_DELIMITER));
				}
				else
				{
					ungetc(ch, fd);
				}
			}
			else
			{
				ungetc(ch, fd);
				fprintf(stderr, "\n[LEXICAL_ANALYZER] Line #%d: An error with lexem - expected an identifier!\n", lines);
			}
			
			for ( int j = 0; buffer[j]; j++ )
				buffer[j] = '\0';
			i = 0;
			cur_state = START;
			continue;
		}
		
		if ( cur_state == KEYWORD )
		{
			if ( 
				   ( (ch >= 'A') && (ch <= 'Z') ) ||
				   ( (ch >= 'a') && (ch <= 'z') )
			   )
			{
				buffer[i] = ch;
				i++;
				continue;
			}
			buffer[i] = '\0';

			const char* delimeters = " \t\n\r+-*/%,;:<>=";
			ungetc(ch, fd);
			if ( is_delimiter_symbol(delimeters, ch) )
			{
				ll_insert(&list, new Lexem(buffer, i, lines, LEXEM_TYPE_KEYWORD));
			}
			else
			{
				fprintf(stderr, "\n[LEXICAL_ANALYZER] Line #%d: An error with lexem - expected a keyword!\n", lines);
			}
		
			for ( int j = 0; buffer[j]; j++ )
				buffer[j] = '\0';
			i = 0;
			cur_state = START;
			continue;
		}

		if ( cur_state == ASSIGN )
		{
			if ( ch == '=' )
			{
				buffer[i] = ch;
				buffer[i+1] = '\0';
				
				ll_insert(&list, new Lexem(buffer, i+1, lines, LEXEM_TYPE_ASSIGN));
			}
			else
			{
				ungetc(ch, fd);
				fprintf(stderr, "\n[LEXICAL_ANALYZER] Line #%d: An error with lexem - expected an assign!\n", lines);
			}
			
			for ( int j = 0; buffer[j]; j++ )
				buffer[j] = '\0';
			i = 0;
			cur_state = START;
			continue;
		}

		if ( cur_state == STRING )
		{
			if ( ch != '"' )
			{
				if ( ch == EOF )
				{

					ungetc(ch, fd);
					fprintf(stderr, "\n[LEXICAL_ANALYZER] Line #%d: An error with lexem - expected a string!\n", lines);
					
					for ( int j = 0; buffer[j]; j++ )
						buffer[j] = '\0';
					i = 0;
					cur_state = START;

					continue;
				}

				buffer[i] = ch;
				i++;
				continue;
			}
			buffer[i] = ch;
			buffer[i+1] = '\0';
			
			ll_insert(&list, new Lexem(buffer, i+1, lines, LEXEM_TYPE_STRING));

			for ( int j = 0; buffer[j]; j++ )
				buffer[j] = '\0';
			i = 0;
			cur_state = START;
			continue;
		}
	}
	while ( 1 );

	return list;
}


LexemAnalyzer::~LexemAnalyzer()
{
	if ( buffer != NULL )
		delete[] buffer;
	buffer = NULL;
}

#endif
