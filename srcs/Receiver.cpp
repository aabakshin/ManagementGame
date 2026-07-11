#ifndef RECEIVER_CPP_SENTINEL
#define RECEIVER_CPP_SENTINEL


#include "Receiver.hpp"
#include "Utility.hpp"
#include <cstring>
#include <cstdio>
#include <stdexcept>


void Receiver::RecvMessage( int cs, const char* address )
{
	Reset();

	target_socket = cs;

	recv_bytes = Utility::readline( target_socket, message, BUFSIZE-1 );

	if ( !IsRecvMessage() )
		throw std::runtime_error("Unable to receive data error in \"Receiver::RecvMessage\" function!");

	strncpy(target_address, address, ADDRESS_SIZE );

	message[recv_bytes] = '\0';
	Utility::cut_str(message, recv_bytes, '\n');

	int message_size = strlen(message) + 1;
	Utility::delete_spaces(message, &message_size);

	message_length = message_size - 1;
}

bool Receiver::IsRecvMessage() const
{
	if ( ( recv_bytes >= 0 ) && ( recv_bytes <= BUFSIZE-1 ) )
		return true;

	return false;
}

void Receiver::SetRecvMsgsCount( int msgs_value )
{
	if ( msgs_value < 0 )
		throw std::runtime_error("Invalid 'msgs_value' error in \"Receiver::SetRecvMsgsCount\" function!");

	recv_msgs_count = msgs_value;
}

void Receiver::ShowReceivedMessage() const
{
#ifdef DEBUG_MODE
	std::cout << "\n" << "[" << Utility::current_time_str() << "] " << "[DEBUG] ==================== " << "(" << GetRecvMsgsCount() << ")" << "=====================\n"
	std::cout << "[" << Utility::current_time_str() << "] " << "[DEBUG] ";

	for ( int i = 0; ( i < Receiver::BUFSIZE ) && ( i < GetMessageLength() ); ++i )
	{
		std::cout << std::setw(3) << GetMessage()[i] << " ";
		if ( ( (i+1) % 10 ) == 0 )
			std::cout << "\n" << "[" << Utility::current_time_str() << "] " << "[DEBUG] ";
	}
	std::cout << "\n";

	std::cout << "\n" << "[" << Utility::current_time_str() << "] " << "[DEBUG] " << "message: <[ " << GetMessage() << " ]>\n";
	std::cout << "[" << Utility::current_time_str() << "] "  << "[DEBUG] " << "Received from [" << GetTargetAddress() << "] " << GetRecvBytes() << "\\" << GetMessageLength << " bytes\n";
	std::cout << "[" << Utility::current_time_str() << "] " << "[DEBUG] ==================== " << "(" << GetRecvMsgsCount() << ")" << "=====================\n\n";
#endif
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
