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
	int max_fd;
	std::string address;
	std::string service;
	fd_set readfds;
	Logger* srv_msgs_logger { nullptr };
	std::shared_ptr<const Config::GameSettings> m_game_config_settings;
	std::mutex m_game_settings_mutex;
	SessionsPlanner sessions_planner;
public:
	Server( const std::string, const std::string, Logger*, std::shared_ptr<const Config::GameSettings> );
	static void SetExitFlag() noexcept { exit_flag.store( true ); }
	static bool CheckAndClearExitFlag() noexcept { return exit_flag.exchange( false ); }
	static void SetReloadCfgFlag() noexcept { reload_cfg_flag.store( true ); }
	static bool CheckAndClearCfgFlag() noexcept { return reload_cfg_flag.exchange( false ); }
	static int GetSignalNum() { return sig_number; }
	static void SetSignalNum( int value ) { sig_number.store( value ); }
	int Run();
private:
	Server( const Server& ) = delete;
	Server( Server&& ) = delete;
	void operator=( const Server& ) = delete;
	int GetListenSocket() const { return ls; }
	void SetListenSocket( int );
	const std::string GetAddr() const { return address; }
	const std::string GetPort() const { return service; }
	int GetMaxFd() const { return max_fd; }
	void SetMaxFd( int );
	void ListenSocketInit( const std::string, const std::string );
	void CloseConnection( int, std::string );
	void Stop( int );
	void ReloadGameConfig();
	void RefillReadfds();
	void NewClientHandle();
	void IncomingEventsHandle();
};

#endif
