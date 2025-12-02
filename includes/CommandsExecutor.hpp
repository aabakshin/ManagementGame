#ifndef COMMANDS_EXECUTOR_HPP
#define COMMANDS_EXECUTOR_HPP

#include "Banker.hpp"


enum
{
	INTERNAL_COMMAND_ERROR					=				-4,
	INCORRECT_ARGS_COMMAND_ERROR			=				-3,
	WFNT_COMMAND_ERROR						=				-2,
	UNKNOWN_COMMAND_ERROR					=				-1
};

enum
{
	HELP_COMMAND_NUM,
	MARKET_COMMAND_NUM,
	PLAYER_COMMAND_NUM,
	LIST_COMMAND_NUM,
	PROD_COMMAND_NUM,
	BUILD_COMMAND_NUM,
	BUY_COMMAND_NUM,
	SELL_COMMAND_NUM,
	TURN_COMMAND_NUM,
	QUIT_COMMAND_NUM
};

enum
{
	MAX_CMD_TOKENS_AMOUNT			=			3
};


class CommandsExecutor
{
public:
	class CommandParams
	{
	private:
		void* param1;
		void* param2;
	public:
		CommandParams();
		const void* GetParam1() const { return param1; }
		const void* GetParam2() const { return param2; }
		void SetParam1( const void* value ) { param1 = const_cast<void*>(value); }
		void SetParam2( const void* value ) { param2 = const_cast<void*>(value); }
	private:
		CommandParams( const CommandParams& ) {}
		void operator=( const CommandParams& ) {}
	};
private:
	char* command_tokens[MAX_CMD_TOKENS_AMOUNT];
	int cmd_tokens_amount;
	CommandParams cmd_params;
public:
	CommandsExecutor();
	const CommandParams& GetCmdParams() const { return cmd_params; }
	int GetCmdTokensAmount() const { return cmd_tokens_amount; }
	const char* GetCmdToken( int idx ) const;
	void SetCmdTokensAmount( int tokens_amount );
	void SetCmdToken( int idx, char* cmd_token );
	void SetCmdParams( const void* value1, const void* value2 );
	int make_cmd_tokens( const char* read_buf );
	int process_command( Banker* b, int sender_num );
	int func();
private:
	CommandsExecutor( const CommandsExecutor& ) {}
	void operator=( const CommandsExecutor& ) {}
};

#endif
