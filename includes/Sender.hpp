#ifndef SENDER_HPP_SENTINEL
#define SENDER_HPP_SENTINEL


class Sender
{
private:

	enum
	{
				BUFSIZE							=		   1024,
				ADDRESS_SIZE					=			 50,
				SERVICE_SIZE					=            10
	};

	char message[BUFSIZE] { 0x00 };
	int message_length { 0 };
	int cur_pos { 0 };
	int sent_bytes { 0 };
	int target_socket { -1 };
	char target_address[ADDRESS_SIZE] { 0x00 };
public:
	Sender() {}
	void SendMessage( const char* const*, int, int, const char* );
	void SendMessage( const char*, int, const char* );
private:
	Sender( const Sender& ) = delete;
	Sender( Sender&& ) = delete;
	void operator=( const Sender& ) = delete;
	void Reset();
	void ResetMessage();
	void ResetMessageLength();
	void ResetCurPos();
	void ResetTargetSocket();
	void ResetTargetAddress();
	void ResetSentBytes();
	void ShowSendingMessage() const;
	bool IsSentMessage() const;
};

#endif
