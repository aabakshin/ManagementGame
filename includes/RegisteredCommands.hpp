#ifndef REGISTERED_COMMANDS_HPP_SENTINEL
#define REGISTERED_COMMANDS_HPP_SENTINEL


#include "Command.hpp"
#include "Config.hpp"
#include <memory>


enum
{
				MAX_CMD_TOKENS			=			20
};

class RegisteredCommands
{
private:
	Command* registered_commands[COMMANDS_COUNT];
	std::shared_ptr<const Config::GameSettings> m_game_settings;
	int count;
public:
	RegisteredCommands() {}
	void Make( std::shared_ptr<const Config::GameSettings> );
	void ApplySettings( std::shared_ptr<const Config::GameSettings> );
	~RegisteredCommands();
	const Command* const operator[]( int ) const;
	int GetCount() const { return count; }
private:
	RegisteredCommands( const RegisteredCommands& ) {}
	RegisteredCommands( RegisteredCommands&& ) {}
	void operator=( const RegisteredCommands& ) {}
};

#endif
