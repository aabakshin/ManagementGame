#ifndef SERVER_CORE_HPP
#define SERVER_CORE_HPP


#include "SessionsPlanner.hpp"
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
				SERVICE_SIZE					=            10,
				SESSIONS_COUNT					=			  4
	};

	static bool alrm_flag;
	static bool exit_flag;
	static int sig_number;
	int sent_msgs_count;
	int recv_msgs_count;
	bool timer_flag;
	int ls;
	struct addrinfo* bind_address;
	char address_buffer[ADDRESS_SIZE];
	char service_buffer[SERVICE_SIZE];
	int max_fd;
	fd_set readfds;
	CommandExecutor cmds_exec;
	SessionsPlanner session_planner;
	EncapsulatedBrokerMessages<BCBrokerMessages,SessionsPlanner> EBCbroker;
	EncapsulatedBrokerMessages<GameMessages,SessionsPlanner> EGameMessages;
	EncapsulatedBrokerMessages<MulticastActionsExec,SessionsPlanner> EMultiActionsExec;
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
	int GetSentMsgsCount() const { return sent_msgs_count; }
	void SetSentMsgsCount( int msgs_value );
	int GetRecvMsgsCount() const { return recv_msgs_count; }
	void SetRecvMsgsCount( int msgs_value );
	void SetTimerFlag() { timer_flag = true; }
	void UnsetTimerFlag() { timer_flag = false; }
	bool IsTimerFlag() { return timer_flag; }
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
	void CloseConnection( int, int );
	void Stop( int forcely );
	void FillReadfds();
	bool IsCorrectIdentityMsg( const char* );
	void ConcatAddrPort();
	void ShowSentMessage() const;
	void ShowReceivedMessage() const;
	void NewClientHandle();
	void ClientsInputHandle();
	void GameEventsHandle();
	void ErrorEvent( int, const char*, int );
	void AddNewClientToSession( int, const char* );
	bool QuitPlayer( int, int );
	void ChangeMarketState( int );
	void PrepareNewTurn( int );
	void ShowAuctionInfo( int, const char* auction_type_msg, const Item<MarketData>* );
	void ReportOnTurn( int );
	void EndGameTurn( int );
	void SortRequestsByPrice( int session_id, const List<Item<MarketData>>&, List<Item<MarketData>>&, int auction_type );
	bool CheckPlayersReports( int, List<Item<MarketData>>&, int );
	void StartAuction( int, const List<Item<MarketData>>&, int auction_type );
	void PrepareGameState( int );
};


#endif
