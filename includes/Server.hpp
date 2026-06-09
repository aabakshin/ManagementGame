#ifndef SERVER_CORE_HPP
#define SERVER_CORE_HPP


#include "SessionsPlanner.hpp"
#include <csignal>


class Server
{
private:
	static volatile sig_atomic_t exit_flag;
	static volatile sig_atomic_t sig_number;
	int ls;
	struct addrinfo* bind_address;
	char address_buffer[Sender::ADDRESS_SIZE];
	char service_buffer[Sender::SERVICE_SIZE];
	int max_fd;
	fd_set readfds;
	SessionsPlanner sessions_planner;
public:
	Server() {}
	~Server();
	static void SetExitFlag() { exit_flag = true; }
	static void UnsetExitFlag() { exit_flag = false; }
	static bool IsExitFlag() { return exit_flag; }
	static int GetSignalNum() { return sig_number; }
	static void SetSignalNum( int value );
	void Make( const char*, const char* );
	int Run();
private:
	Server( const Server& ) = delete;
	Server( Server&& ) = delete;
	void operator=( const Server& ) = delete;
	void IgnoreUnusedSignals();
	int GetListenSocket() const { return ls; }
	void SetListenSocket( int );
	const char* GetAddrBuffer() const { return address_buffer; }
	void SetAddrBuffer( const char*, const char* );
	int GetMaxFd() const { return max_fd; }
	void SetMaxFd( int );
	void ListenSocketInit();
	void CloseConnection( int, std::string );
	void Stop( int forcely );
	void RefillReadfds();
	void ConcatAddrPort( int );
	void NewClientHandle();
	void IncomingEventsHandle();
};

#endif
