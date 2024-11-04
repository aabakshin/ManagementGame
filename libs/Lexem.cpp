/* Файл реализации модуля Lexem */

#ifndef LEXEM_CPP
#define LEXEM_CPP

#ifndef __CPP_FILE__
#define __CPP_FILE__
#endif

#include "../includes/Lexem.hpp"
#include <cstdlib>
#include <cstdio>

Lexem::Lexem(const char* str, int str_len, int num_str, int lexem_type)
{
	if ( (str == NULL) || (str_len < 1) || (str[0] == '\0') )
	{
		Lexem::lexem = NULL;
		Lexem::size = 0;
		Lexem::num_str = -1;
		Lexem::lexem_type = -1;
		
		return;
	}
	
	Lexem::size = str_len+1;
	Lexem::lexem = new char[Lexem::size];
	if ( Lexem::lexem == NULL )
	{
		fprintf(stderr, "%s", "An internal error has occured while constructing an \"Lexem\" object\n");
		return;
	}
	
	int i;
	for ( i = 0; i < str_len; i++ )
		Lexem::lexem[i] = str[i];
	Lexem::lexem[i] = '\0';

	Lexem::num_str = num_str;
	Lexem::lexem_type = lexem_type;
}

Lexem::~Lexem()
{
	if ( Lexem::lexem == NULL )
		return;

	delete[] Lexem::lexem;
	Lexem::lexem = NULL;
}

Lexem::Lexem(const Lexem& obj)
{
	this->size = (obj.size < 1) ? 0 : obj.size; 
	
	if ( this->size < 1 )
	{
		this->lexem = NULL;
		return;
	}
	
	this->lexem = new char[Lexem::size];
	if ( this->lexem == NULL )
	{
		fprintf(stderr, "%s", "An internal error has occured while constructing an \"Lexem\" object\n");
		return;
	}

	int i;
	for ( i = 0; i < this->size-1; i++ )
		this->lexem[i] = obj.lexem[i];
	this->lexem[i] = '\0';
	
	this->num_str = (obj.num_str < 1) ? -1 : obj.num_str;
	this->lexem_type = (obj.lexem_type < 0) ? -1 : obj.lexem_type;

}

Lexem& Lexem::operator=(const Lexem& obj)
{	
	if ( obj.size < 1 )
	{
		return *this;
	}
	
	if ( obj.lexem == NULL )
	{
		return *this;
	}

	this->size = obj.size;
	this->lexem = new char[this->size];
	if ( this->lexem == NULL )
	{
		return *this;
	}

	int i;
	for ( i = 0; i < obj.size-1; i++ )
		this->lexem[i] = obj.lexem[i];
	this->lexem[i] = '\0';
	
	this->lexem_type = obj.lexem_type;
	this->num_str = obj.num_str;

	return *this;
}

#endif
