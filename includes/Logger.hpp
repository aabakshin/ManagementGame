#ifndef LOGGER_HPP_SENTINEL
#define LOGGER_HPP_SENTINEL


#include <mutex>
#include <memory>


class Logger
{
public:

	enum class LogLevel { Info, Error };

private:
	std::ostream* m_info_stream = nullptr;
	std::ostream* m_error_stream = nullptr;
	std::unique_ptr<std::ofstream> m_error_file;
	std::mutex m_mutex;
public:
	Logger();
	void SetStream( Logger::LogLevel, std::ostream* );
	void SetErrorFile( const std::string& );

	template<typename... Args>
		void info( Args&&... );
	template<typename... Args>
		void error( Args&&... );

private:
	template<typename... Args>
		void log( Logger::LogLevel, Args&&... );

	Logger( const Logger&  ) = delete;
	Logger( Logger&& ) = delete;
	void operator=( const Logger& ) = delete;
};



#endif
