#ifndef MESSAGE_TOKENS_CPP_SENTINEL
#define MESSAGE_TOKENS_CPP_SENTINEL


#include "MessageTokens.hpp"
#include <cstring>
#include <stdexcept>


void MessageTokens::NullifyMessageTokens()
{
	for ( int i = 0; i < max_msg_tokens_count; ++i )
	{
		if ( message_tokens[i] != nullptr )
		{
			delete[] message_tokens[i];
			message_tokens[i] = nullptr;
		}
	}
}

MessageTokens::MessageTokens( int tokens_count )
{
	message_tokens = new const char*[tokens_count];

	max_msg_tokens_count = tokens_count;
	msg_tokens_count = tokens_count;

	for ( int i = 0; i < max_msg_tokens_count; ++i )
	{
		message_tokens[i] = new char[MESSAGE_TOKEN_SIZE];
		memset(const_cast<char*>(message_tokens[i]), 0, MESSAGE_TOKEN_SIZE);
	}
}

const char*& MessageTokens::operator[]( int idx )
{
	if ( ( idx < 0 ) || ( idx > max_msg_tokens_count-1 ) )
		throw std::runtime_error("IncorrectMsgTokensIdx error in \"MessageTokens::operator[]\" function!");

	return message_tokens[idx];
}

void MessageTokens::SetMsgTokensCount( int tokens_value )
{
	if ( ( tokens_value < 1 ) || ( tokens_value > max_msg_tokens_count ) )
		throw std::runtime_error("IncorrectMsgTokensValue error in \"MessageTokens::SetMsgTokensCount\" function!");

	msg_tokens_count = tokens_value;
}

MessageTokens::~MessageTokens()
{
	NullifyMessageTokens();
	delete[] message_tokens;
}

#endif
