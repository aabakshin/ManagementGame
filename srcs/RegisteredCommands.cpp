#ifndef REGISTERED_COMMANDS_CPP_SENTINEL
#define REGISTERED_COMMANDS_CPP_SENTINEL

#include "RegisteredCommands.hpp"


RegisteredCommands::RegisteredCommands()
{
	registered_commands[HELP_COMMAND_NUM]			=			new HelpCommand( MAX_CMD_TOKENS );
	registered_commands[MARKET_COMMAND_NUM]			=			new MarketCommand( MAX_CMD_TOKENS );
	registered_commands[PLAYER_COMMAND_NUM]			=			new PlayerCommand( MAX_CMD_TOKENS );
	registered_commands[LIST_COMMAND_NUM]			=			new ListCommand( MAX_CMD_TOKENS );
	registered_commands[PROD_COMMAND_NUM]			=			new ProdCommand( MAX_CMD_TOKENS );
	registered_commands[BUILD_COMMAND_NUM]			=			new BuildCommand( MAX_CMD_TOKENS );
	registered_commands[BUY_COMMAND_NUM]			=			new BuyCommand( MAX_CMD_TOKENS );
	registered_commands[SELL_COMMAND_NUM]			=			new SellCommand( MAX_CMD_TOKENS );
	registered_commands[TURN_COMMAND_NUM]			=			new TurnCommand( MAX_CMD_TOKENS );
	registered_commands[QUIT_COMMAND_NUM]			=			new QuitCommand( MAX_CMD_TOKENS );

	count = COMMANDS_COUNT;
}

RegisteredCommands::~RegisteredCommands()
{
	for ( int i = HELP_COMMAND_NUM; i <= QUIT_COMMAND_NUM; ++i )
	{
		if ( registered_commands[i] != nullptr )
		{
			delete registered_commands[i];
			registered_commands[i] = nullptr;
		}
	}
}

const Command* const RegisteredCommands::operator[]( int idx ) const
{
	if ( ( idx < 0 ) || ( idx > QUIT_COMMAND_NUM ) )
	{
		return nullptr;
		// throw IndexOutOfRangeException();
	}

	return const_cast<const Command*>(registered_commands[idx]);
}

#endif
