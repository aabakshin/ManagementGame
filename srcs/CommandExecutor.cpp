#ifndef COMMAND_EXECUTOR_CPP
#define COMMAND_EXECUTOR_CPP


#include "CommandExecutor.hpp"
#include "MGLib.h"
#include <cstdlib>
#include <cstring>


extern const char* info_game_messages[];


CommandExecutor::CommandExecutor()
{
	for ( int i = 0; i < MAX_CMD_TOKENS_AMOUNT; ++i )
		SetCmdToken( i, nullptr );

	for ( int i = 0; i < MAX_CMD_TOKENS; ++i )
		SetCmdResultToken( i, nullptr );

	SetCmdTokensAmount( 0 );
	SetCmdResultTokensAmount( MAX_CMD_TOKENS );
}

const char* CommandExecutor::GetCmdToken( int idx ) const
{
	if ( ( idx < 0 ) || ( idx >= MAX_CMD_TOKENS_AMOUNT ) )
	{
		return nullptr;
		// throw InvalidCmdTokenIndexException();
	}

	return cmd_tokens[idx];
}

const char* CommandExecutor::GetCmdResultToken( int idx ) const
{
	if ( ( idx < 0 ) || ( idx >= MAX_CMD_TOKENS ) )
	{
		return nullptr;
		// throw InvalidCmdTokenIndexException();
	}

	return cmd_result_tokens[idx];
}

void CommandExecutor::SetCmdTokensAmount( int tokens_amount )
{
	if ( ( tokens_amount < 0 ) || ( tokens_amount > MAX_CMD_TOKENS_AMOUNT ) )
	{
		return;
		// throw InvalidCmdTokensAmountException();
	}

	cmd_tokens_amount = tokens_amount;
}

void CommandExecutor::SetCmdResultTokensAmount( int tokens_amount )
{
	if ( ( tokens_amount < 1 ) || ( tokens_amount > MAX_CMD_TOKENS ) )
	{
		return;
		// throw InvalidCmdTokensAmountException();
	}

	cmd_result_tokens_amount = tokens_amount;
}

void CommandExecutor::SetCmdToken( int idx, const char* cmd_token )
{
	if ( ( idx < 0 ) || ( idx >= MAX_CMD_TOKENS_AMOUNT ) )
	{
		return;
		// throw InvalidCmdTokenIndexException();
	}

	cmd_tokens[idx] = cmd_token;
}

void CommandExecutor::SetCmdResultToken( int idx, const char* cmd_token )
{
	if ( ( idx < 0 ) || ( idx >= MAX_CMD_TOKENS ) )
	{
		return;
		// throw InvalidCmdTokenIndexException();
	}

	cmd_result_tokens[idx] = cmd_token;
}

void CommandExecutor::MakeCmdTokens( const char* read_buf )
{
	char* istr = strtok(const_cast<char*>(read_buf), " ");
	int j = 0;
	while ( ( istr != nullptr ) && ( j < MAX_CMD_TOKENS_AMOUNT ) )
	{
		SetCmdToken(j, istr);
		j++;
		istr = strtok(nullptr, " ");
	}
	SetCmdTokensAmount( j );
}

void CommandExecutor::ProcessCommand( int sender_player_id, const BCBrokerMessages& BCbroker )
{
	char command_str[100];
	strcpy(command_str, GetCmdToken( 0 ));

	for ( int j = 0; reg_cmds[j] != nullptr; ++j )
	{
		if ( strcmp(command_str, reg_cmds[j]->GetName()) == 0 )
		{
			const_cast<Command*>(reg_cmds[j])->PrepareAndProc( sender_player_id, GetCmdTokensAmount(), GetCmdToken(1), GetCmdToken(2), BCbroker );

			int tokens_count = reg_cmds[j]->GetMessageTokens().GetMsgTokensCount();

			for ( int i = 0; i < tokens_count; ++i )
				SetCmdResultToken( i, reg_cmds[j]->GetMessageTokens().GetValue()[i] );

			SetCmdResultTokensAmount( tokens_count );
			return;
		}
	}

	SetCmdResultToken( 0, info_game_messages[UNKNOWN_COMMAND]);
	SetCmdResultTokensAmount( 1 );
}

#endif
