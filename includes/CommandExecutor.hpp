#ifndef COMMAND_EXECUTOR_HPP
#define COMMAND_EXECUTOR_HPP


#include "RegisteredCommands.hpp"


enum
{
	MAX_CMD_TOKENS_AMOUNT					=				3
};

class BCBrokerMessages;

class CommandExecutor
{
private:
	const char* cmd_tokens[MAX_CMD_TOKENS_AMOUNT];
	int cmd_tokens_amount;
	RegisteredCommands reg_cmds;
	const char* cmd_result_tokens[MAX_CMD_TOKENS];
	int cmd_result_tokens_amount;
public:
	CommandExecutor();
	int GetCmdTokensAmount() const { return cmd_tokens_amount; }
	const char* GetCmdToken( int ) const;
	int GetCmdResultTokensAmount() const { return cmd_result_tokens_amount; }
	const char* GetCmdResultToken( int ) const;
	const char* const* GetCmdResultTokens() const { return static_cast<const char* const*>(cmd_result_tokens); }
	void ProcessCommand( int, const char*, int, const BCBrokerMessages& );
private:
	CommandExecutor( const CommandExecutor& ) = delete;
	CommandExecutor( CommandExecutor&& ) = delete;
	void operator=( const CommandExecutor& ) = delete;
	void MakeCmdTokens( const char* );
	void SetCmdTokensAmount( int );
	void SetCmdToken( int, const char* );
	void SetCmdResultTokensAmount( int );
	void SetCmdResultToken( int, const char* );
};

#endif
