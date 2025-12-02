#ifndef SERVER_CORE_HPP
#define SERVER_CORE_HPP

#include "Banker.hpp"
#include "CommandsExecutor.hpp"
#include <sys/types.h>									// contains fd_set structure


class Server
{
	static bool alrm_flag;
	static bool exit_flag;
	static int sig_number;
	static int sent_msgs_count;
	bool timer_flag;
	int ls;												/* LISTENING SOCKET */
	struct addrinfo* bind_address;
	char address_buffer[ADDRESS_BUFFER];
	int max_fd;
	fd_set readfds;
	CommandsExecutor cmds_exec;
	Banker banker;
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
	fd_set& GetReadfds() { return readfds; }
	CommandsExecutor& GetCmdsHndl() { return cmds_exec; }
	Banker& GetBanker() { return banker; }
	int Run();
private:
	Server() {}
	Server( const Server& obj ) {}
	void operator=( const Server& obj ) {}
	void ListenSocketInit();
	int CloseConnection( int player_number );
	void Stop( int forcely );
	int QuitPlayer( int player_number );
	int FillReadfds();
};

#endif
