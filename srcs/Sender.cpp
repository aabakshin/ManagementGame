#ifndef SENDER_CPP_SENTINEL
#define SENDER_CPP_SENTINEL


#include "Sender.hpp"
#include "Utility.hpp"
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef DEBUG_MODE
	#include <iostream>
	#include <iomanip>
#endif

#include <stdexcept>


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

	int sent_code = Utility::sendall( target_socket, message, &message_length );
	sent_bytes = message_length;

	if ( sent_code < 0 )
		throw std::runtime_error("An error has occurred while sending data in \"Sender::SendMessage\" function!");
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

	int sent_code = Utility::sendall( target_socket, message, &message_length );
	sent_bytes = message_length;

	if ( sent_code < 0 )
		throw std::runtime_error("An error has occurred while sending data in \"Sender::SendMessage\" function!");
}

void Sender::SetSentMsgsCount( int msgs_value )
{
	if ( msgs_value < 0 )
		throw std::runtime_error("Invalid 'sent_msgs_count' error in \"Sender::SetSentMsgsCount\" function!");

	sent_msgs_count = msgs_value;
}

void Sender::ShowSentMessage() const
{
#ifdef DEBUG_MODE
	std::cout << "\n" << "[" << Utility::current_time_str() << "] " << "[DEBUG] ==================== " << "(" << GetSentMsgsCount() << ")" << "=====================\n"
	std::cout << "[" << Utility::current_time_str() << "] " << "[DEBUG] ";

	for ( int i = 0; ( i < Sender::BUFSIZE ) && ( i < GetMessageLength() ); ++i )
	{
		std::cout << std::setw(3) << GetMessage()[i] << " ";
		if ( ( (i+1) % 10 ) == 0 )
			std::cout << "\n" << "[" << Utility::current_time_str() << "] " << "[DEBUG] ";
	}
	std::cout << "\n";

	std::cout << "\n" << "[" << Utility::current_time_str() << "] " << "[DEBUG] " << "message: <[ " << GetMessage() << " ]>\n";
	std::cout << "[" << Utility::current_time_str() << "] "  << "[DEBUG] " << "Sent to [" << GetTargetAddress() << "] " << GetSentBytes() << "\\" << GetMessageLength << " bytes\n";
	std::cout << "[" << Utility::current_time_str() << "] " << "[DEBUG] ==================== " << "(" << GetSentMsgsCount() << ")" << "=====================\n\n";
#endif
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
