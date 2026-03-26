#ifndef SERVER_CORE_HPP
#define SERVER_CORE_HPP


#include "Banker.hpp"
#include "CommandExecutor.hpp"
#include "BrokerMessages.hpp"
#include <sys/types.h>


class Server
{
private:

	enum
	{
				BUFSIZE							=		   1024,
				ADDRESS_SIZE					=			 50,
				SERVICE_SIZE					=            10,
				MESSAGE_TOKENS_COUNT			=			  5,
				MESSAGE_TOKEN_SIZE				=			100
	};

	template <class T>
	class EncapsulatedBrokerMessages
	{
	private:
		T* brokerPTR { nullptr };
	public:
		EncapsulatedBrokerMessages() {}
		void MakeBroker( const Banker& );
		const T& GetBroker() const;
		~EncapsulatedBrokerMessages();
	private:
		EncapsulatedBrokerMessages( const EncapsulatedBrokerMessages& ) = delete;
		EncapsulatedBrokerMessages( EncapsulatedBrokerMessages&& ) = delete;
		void operator=( const EncapsulatedBrokerMessages& ) = delete;
	};

	class MessageTokens
	{
	private:
		const char** message_tokens;
		int max_msg_tokens_count;
		int msg_tokens_count;
	public:
		MessageTokens() {}
		void MakeMessageTokens( int );
		const char*& operator[]( int );
		const char** GetValue() const { return message_tokens; }
		int GetMsgTokensCount() const { return msg_tokens_count; }
		void SetMsgTokensCount( int );
		~MessageTokens();
	private:
		MessageTokens( const MessageTokens& ) {}
		MessageTokens( MessageTokens&& ) {}
		void operator=( const MessageTokens& ) {}
		void NullifyMessageTokens();
	};

	class Sender
	{
	private:
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

	class Receiver
	{
	private:
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

	static bool alrm_flag;
	static bool exit_flag;
	static int sig_number;
	static int sent_msgs_count;
	static int recv_msgs_count;
	bool timer_flag;
	int ls;
	struct addrinfo* bind_address;
	char address_buffer[ADDRESS_SIZE];
	char service_buffer[SERVICE_SIZE];
	int max_fd;
	fd_set readfds;
	CommandExecutor cmds_exec;
	Banker banker;
	EncapsulatedBrokerMessages<BCBrokerMessages> EBCbroker;
	EncapsulatedBrokerMessages<GameMessages> EGameMessages;
	Sender sender;
	Receiver receiver;
	MessageTokens msg_tokens;
public:
	Server( const char*, const char* );
	~Server();
	static void SetAlrmFlag( ) { alrm_flag = true; }
	static void UnsetAlrmFlag( ) { alrm_flag = false; }
	static void SetExitFlag( ) { exit_flag = true; }
	static void UnsetExitFlag( ) { exit_flag = false; }
	static int GetSignalNum() { return sig_number; }
	static void SetSignalNum( int value );
	static int GetSentMsgsCount() { return sent_msgs_count; }
	static void SetSentMsgsCount( int msgs_value );
	static int GetRecvMsgsCount() { return recv_msgs_count; }
	static void SetRecvMsgsCount( int msgs_value );
	void SetTimerFlag() { timer_flag = true; }
	void UnsetTimerFlag() { timer_flag = false; }
	bool IsTimerFlag() const { return timer_flag; }
	int GetListenSocket() const { return ls; }
	void SetListenSocket( int socket_value );
	struct addrinfo*& GetBindAddress() { return bind_address; }
	const char* GetAddrBuffer() const { return address_buffer; }
	void SetAddrBuffer( const char*, const char* );
	int GetMaxFd() const { return max_fd; }
	void SetMaxFd( int max_value );
	const fd_set& GetReadfds() const { return readfds; }
	const CommandExecutor& GetCmdsHndl() const { return cmds_exec; }
	const Banker& GetBanker() const { return banker; }
	int Run();

	int PayCharges();
	int ReportOnTurn();
	int ChangeMarketState();
	int StartAuction( const MarketRequestList&, int auction_type );
	int CheckBuildingFactories();
private:
	Server() = delete;
	Server( const Server& ) = delete;
	Server( Server&& ) = delete;
	void operator=( const Server& ) {}
	void ListenSocketInit();
	int CloseConnection( int player_number );
	void Stop( int forcely );
	int QuitPlayer( int player_number );
	int FillReadfds();
	bool IsCorrectIdentityMsg( const char* );
	void ConcatAddrPort();

	int ShowAuctionInfo( const char* auction_type_msg, const MarketRequestList::MarketRequest* );
	int SortRequestsByPrice( const MarketRequestList&, MarketRequestList&, int auction_type );
	int CheckPlayersReports( MarketRequestList& );
};

#endif
