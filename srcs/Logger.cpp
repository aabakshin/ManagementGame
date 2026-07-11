#ifndef LOGGER_CPP_SENTINEL
#define LOGGER_CPP_SENTINEL


#include "Logger.hpp"
#include "Utility.hpp"
#include <iostream>
#include <fstream>
#include <sstream>


Logger::Logger()
{
	SetStream( LogLevel::Info, &std::cout );
	SetStream( LogLevel::Error, &std::cerr );
}

void Logger::SetStream( Logger::LogLevel lvl, std::ostream* stream )
{
	switch ( lvl )
	{
		case LogLevel::Info:
			m_info_stream = stream;
			break;
		case LogLevel::Error:
			m_error_stream = stream;
			break;
	}
}

void Logger::SetErrorFile( const std::string& filename )
{
	m_error_file = std::make_unique<std::ofstream>(filename, std::ios::app);
	SetStream( LogLevel::Error, m_error_file.get() );
}

template<typename... Args>
void Logger::info( Args&&... args )
{
	log(LogLevel::Info, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::error( Args&&... args )
{
	log(LogLevel::Error, std::forward<Args>(args)...);
}

template<typename... Args>
void Logger::log( Logger::LogLevel lvl, Args&&... args )
{
	std::ostream* stream = nullptr;
	std::string lvl_str;
	std::string cur_time_str = "[" + Utility::current_time_str() + "] ";

	switch ( lvl )
	{
		case LogLevel::Info:
			stream = m_info_stream;
			lvl_str.append("[INFO] ");
			break;
		case LogLevel::Error:
			stream = m_error_stream;
			lvl_str.append("[ERROR] ");
			break;
	}

	if ( !stream )
		return;

	std::lock_guard<std::mutex> lock(m_mutex);

	std::ostringstream oss;
	(oss << ... << std::forward<Args>(args));
	*stream << cur_time_str << lvl_str << oss.str() << std::endl;
}

#endif
