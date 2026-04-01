#ifndef RECEIVER_CPP_SENTINEL
#define RECEIVER_CPP_SENTINEL


#include "Receiver.hpp"
#include "MGLib.h"
#include <cstring>


void Receiver::RecvMessage( int cs, const char* address )
{
	Reset();

	target_socket = cs;

	strncpy(target_address, address, ADDRESS_SIZE );

	recv_bytes = readline( target_socket, message, BUFSIZE-1 );

	if ( !IsRecvMessage() )
	{
		return;
		// throw UnableRecvDataException();
	}

	message[recv_bytes] = '\0';
	cut_str(message, recv_bytes, '\n');

	int message_size = strlen(message) + 1;
	delete_spaces(message, &message_size);

	message_length = message_size - 1;
}

bool Receiver::IsRecvMessage() const
{
	if ( ( recv_bytes >= 0 ) && ( recv_bytes <= BUFSIZE-1 ) )
		return true;

	return false;
}

void Receiver::Reset()
{
	ResetMessage();
	ResetMessageLength();
	ResetCurPos();
	ResetTargetSocket();
	ResetTargetAddress();
	ResetSentBytes();
}

void Receiver::ResetMessage()
{
	memset(message, 0x00, BUFSIZE);
}

void Receiver::ResetMessageLength()
{
	message_length = 0;
}

void Receiver::ResetCurPos()
{
	cur_pos = 0;
}

void Receiver::ResetTargetSocket()
{
	target_socket = -1;
}

void Receiver::ResetTargetAddress()
{
	memset(target_address, 0x00, ADDRESS_SIZE);
}

void Receiver::ResetSentBytes()
{
	recv_bytes = 0;
}

#endif
