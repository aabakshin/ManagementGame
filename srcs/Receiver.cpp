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

	ShowReceivedMessage();
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
/*
void Receiver::ShowReceivedMessage() const
{
	Server::SetRecvMsgsCount( Server::GetRecvMsgsCount() + 1 );
	printf("\n==================== (%d) ====================\n", Server::GetRecvMsgsCount());

	for ( int i = 0; ( i < BUFSIZE ) && ( i < message_length ); ++i )
	{
		printf("%3d ", message[i]);
		if ( ( (i+1) % 10 ) == 0 )
			putchar('\n');
	}
	putchar('\n');

	printf("\nmessage: <[ %s ]>\n"
			"Received from [%s] %d\\%d bytes\n"
			"==================== (%d) ====================\n\n", message, target_address, recv_bytes, message_length, Server::GetRecvMsgsCount());
}*/

#endif
