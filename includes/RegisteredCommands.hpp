#ifndef REGISTERED_COMMANDS_HPP_SENTINEL
#define REGISTERED_COMMANDS_HPP_SENTINEL

#include "Command.hpp"

enum
{
				MAX_CMD_TOKENS			=			20
};

class RegisteredCommands
{
private:
	Command* registered_commands[COMMANDS_COUNT];
	int count;
public:
	RegisteredCommands();
	~RegisteredCommands();
	const Command* const operator[]( int idx ) const;
	int GetCount() const { return count; }
private:
	RegisteredCommands( const RegisteredCommands& ) {}
	RegisteredCommands( RegisteredCommands&& ) {}
	void operator=( const RegisteredCommands& ) {}
};

#endif
