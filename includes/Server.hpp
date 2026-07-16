#ifndef SERVER_CORE_HPP
#define SERVER_CORE_HPP


#include "SessionsPlanner.hpp"
#include "Config.hpp"
#include <memory>
#include <mutex>
#include <atomic>


class Logger;

class Server
{
private:
	inline static std::atomic<bool> exit_flag { false };
	inline static std::atomic<bool> reload_cfg_flag { false };
	inline static std::atomic<int> sig_number { 0 };
	int ls;
	struct addrinfo* bind_address { nullptr };
	char address_buffer[Sender::ADDRESS_SIZE];
	char service_buffer[Sender::SERVICE_SIZE];
	int max_fd;
	fd_set readfds;
	Logger* srv_msgs_logger { nullptr };
	std::shared_ptr<const Config::GameSettings> m_game_config_settings;
	std::mutex m_game_settings_mutex;
	SessionsPlanner sessions_planner;
public:
	Server() {}
	~Server();
	static void SetExitFlag() noexcept { exit_flag.store( true, std::memory_order_relaxed ); }
	static bool CheckAndClearExitFlag() noexcept { return exit_flag.exchange( false, std::memory_order_relaxed ); }
	static void SetReloadCfgFlag() noexcept { reload_cfg_flag.store( true, std::memory_order_relaxed ); }
	static bool CheckAndClearCfgFlag() noexcept { return reload_cfg_flag.exchange( false, std::memory_order_relaxed ); }
	static int GetSignalNum() { return sig_number; }
	static void SetSignalNum( int value ) { sig_number.store( value, std::memory_order_relaxed ); }
	void Make( const char*, const char*, Logger*, std::shared_ptr<const Config::GameSettings> );
	int Run();
private:
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
	void CloseConnection( int, std::string );
	void Stop( int forcely );
	void ReloadGameConfig();
	void RefillReadfds();
	void ConcatAddrPort( int );
	void NewClientHandle();
	void IncomingEventsHandle();
};

#endif
