#ifndef SERVER_CORE_HPP
#define SERVER_CORE_HPP


#include "Banker.hpp"
#include "CommandExecutor.hpp"
#include "BrokerMessages.hpp"
#include "Sender.hpp"
#include "Receiver.hpp"
#include <sys/types.h>


class Server
{
private:

	enum
	{
				BUFSIZE							=		   1024,
				ADDRESS_SIZE					=			 50,
				SERVICE_SIZE					=            10
	};

	static bool alrm_flag;
	static bool exit_flag;
	static int sig_number;
	static int sent_msgs_count;
	static int recv_msgs_count;
	static bool timer_flag;
	int ls;
	struct addrinfo* bind_address;
	char address_buffer[ADDRESS_SIZE];
	char service_buffer[SERVICE_SIZE];
	int max_fd;
	fd_set readfds;
	CommandExecutor cmds_exec;
	Banker banker;
	EncapsulatedBrokerMessages<BCBrokerMessages,Banker> EBCbroker;
	EncapsulatedBrokerMessages<GameMessages,Banker> EGameMessages;
	EncapsulatedBrokerMessages<MulticastActionsExec,Banker> EMultiActionsExec;
	Sender sender;
	Receiver receiver;
	MessageTokens msg_tokens;
public:
	Server( const char*, const char* );
	~Server();
	static void SetAlrmFlag() { alrm_flag = true; }
	static void UnsetAlrmFlag() { alrm_flag = false; }
	static bool IsAlrmFlag() { return alrm_flag; };
	static void SetExitFlag() { exit_flag = true; }
	static void UnsetExitFlag() { exit_flag = false; }
	static bool IsExitFlag() { return exit_flag; }
	static int GetSignalNum() { return sig_number; }
	static void SetSignalNum( int value );
	static int GetSentMsgsCount() { return sent_msgs_count; }
	static void SetSentMsgsCount( int msgs_value );
	static int GetRecvMsgsCount() { return recv_msgs_count; }
	static void SetRecvMsgsCount( int msgs_value );
	static void SetTimerFlag() { timer_flag = true; }
	static void UnsetTimerFlag() { timer_flag = false; }
	static bool IsTimerFlag() { return timer_flag; }
	int Run();
private:
	Server() = delete;
	Server( const Server& ) = delete;
	Server( Server&& ) = delete;
	void operator=( const Server& ) = delete;
	int GetListenSocket() const { return ls; }
	void SetListenSocket( int );
	const char* GetAddrBuffer() const { return address_buffer; }
	void SetAddrBuffer( const char*, const char* );
	int GetMaxFd() const { return max_fd; }
	void SetMaxFd( int );
	void ListenSocketInit();
	void CloseConnection( int );
	void Stop( int forcely );
	void FillReadfds();
	bool IsCorrectIdentityMsg( const char* );
	void ConcatAddrPort();

	void NewClientHandle();
	void ClientsInputHandle();
	void GameEventsHandle();

	bool QuitPlayer( int );
	void EndGameTurn();
	void ReportOnTurn();
	void ChangeMarketState();
	void StartAuction( const List<Item<MarketData>>&, int auction_type );
	void PrepareNewTurn();
	void PrepareGameState();
	void ShowAuctionInfo( const char* auction_type_msg, const Item<MarketData>* );
	void SortRequestsByPrice( const List<Item<MarketData>>&, List<Item<MarketData>>&, int auction_type );
	bool CheckPlayersReports( List<Item<MarketData>>&, int );
};


#endif
