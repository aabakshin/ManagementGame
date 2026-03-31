#ifndef RECEIVER_HPP_SENTINEL
#define RECEIVER_HPP_SENTINEL


class Receiver
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
	int recv_bytes { 0 };
	int target_socket { -1 };
	char target_address[ADDRESS_SIZE] { 0x00 };
public:
	Receiver() {}
	const char* GetMessage() const { return message; }
	void RecvMessage( int, const char* );
	int GetRecvBytes() const { return recv_bytes; }
	int GetMessageLength() const { return message_length; }
private:
	Receiver( const Receiver& ) = delete;
	Receiver( Receiver&& ) = delete;
	void operator=( const Receiver& ) = delete;
	void Reset();
	void ResetMessage();
	void ResetMessageLength();
	void ResetCurPos();
	void ResetTargetSocket();
	void ResetTargetAddress();
	void ResetSentBytes();
	void ShowReceivedMessage() const;
	bool IsRecvMessage() const;
};

#endif
