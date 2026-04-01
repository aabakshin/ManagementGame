#ifndef SENDER_CPP_SENTINEL
#define SENDER_CPP_SENTINEL


#include "Sender.hpp"
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>


void Sender::SendMessage( const char* const* message_tokens, int tokens_count, int cs, const char* address )
{
	bool overflow = false;

	Reset();

	for ( int j = 0; j < tokens_count; ++j )
	{
		for ( int k = 0; ( cur_pos < BUFSIZE-2 ) && message_tokens[j][k]; ++k, ++cur_pos )
			message[cur_pos] = message_tokens[j][k];

		if ( cur_pos >= BUFSIZE-2 )
		{
			message[BUFSIZE-2] = '\n';
			message[BUFSIZE-1] = '\0';
			message_length = BUFSIZE;
			overflow = true;
			break;
		}

		message[cur_pos] = '|';
		++cur_pos;
	}

	if ( !overflow )
	{
		message[cur_pos] = '\n';
		message[cur_pos+1] = '\0';
		message_length = cur_pos + 2;
	}

	target_socket = cs;

	strncpy(target_address, address, ADDRESS_SIZE-1);

	sent_bytes = send( target_socket, message, message_length, 0 );

	if ( sent_bytes < 0 )
	{
		return;
		// throw UnableSendDataException();
	}
}

void Sender::SendMessage( const char* msg, int cs, const char* address )
{
	Reset();

	for ( ; ( cur_pos < BUFSIZE-2 ) && msg[cur_pos]; ++cur_pos )
		message[cur_pos] = msg[cur_pos];

	message[cur_pos] = '\n';
	message[cur_pos+1] = '\0';
	message_length = cur_pos + 2;

	target_socket = cs;

	strncpy(target_address, address, ADDRESS_SIZE-1);

	sent_bytes = send( target_socket, message, message_length, 0 );

	if ( !IsSentMessage() )
	{
		return;
		// throw UnableSendDataException();
	}
}

bool Sender::IsSentMessage() const
{
	if ( ( sent_bytes >= 0 ) && ( sent_bytes <= BUFSIZE-1 ) )
		return true;

	return false;
}

void Sender::Reset()
{
	ResetMessage();
	ResetMessageLength();
	ResetCurPos();
	ResetTargetSocket();
	ResetTargetAddress();
	ResetSentBytes();
}

void Sender::ResetMessage()
{
	memset(message, 0x00, BUFSIZE);
}

void Sender::ResetMessageLength()
{
	message_length = 0;
}

void Sender::ResetCurPos()
{
	cur_pos = 0;
}

void Sender::ResetTargetSocket()
{
	target_socket = -1;
}

void Sender::ResetTargetAddress()
{
	memset(target_address, 0x00, ADDRESS_SIZE);
}

void Sender::ResetSentBytes()
{
	sent_bytes = 0;
}


#endif
