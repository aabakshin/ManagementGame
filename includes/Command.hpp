#ifndef COMMAND_HPP_SENTINEL
#define COMMAND_HPP_SENTINEL


#include "MessageTokens.hpp"


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
		HELP_CMD_TOKENS_NUM			=		11,
		MARKET_CMD_TOKENS_NUM		=		 5,
		PLAYER_CMD_TOKENS_NUM		=		10,
		LIST_CMD_TOKENS_NUM			=		 2,
		PROD_CMD_TOKENS_NUM			=		 1,
		BUILD_CMD_TOKENS_NUM		=		 3,
		BUY_CMD_TOKENS_NUM			=		 3,
		SELL_CMD_TOKENS_NUM			=		 3,
		TURN_CMD_TOKENS_NUM			=		 2,
		QUIT_CMD_TOKENS_NUM			=		 1
};

enum
{
		COMMAND_NAME_SIZE		=		10,
		COMMANDS_COUNT			=		10
};


class BCBrokerMessages;

class Command
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
		CommandParams( const CommandParams& ) = delete;
		CommandParams( CommandParams&& ) = delete;
		void operator=( const CommandParams& ) = delete;
	};

private:
	char name[COMMAND_NAME_SIZE];
	CommandParams cmd_params;
	MessageTokens msg_tokens;
public:
	Command( int );
	const char* GetName() const { return name; }
	void SetName( const char* cmd_name );
	const CommandParams& GetCmdParams() const { return cmd_params; }
	void SetCmdParams( const void* value1, const void* value2 );
	const MessageTokens& GetMessageTokens() const { return msg_tokens; }
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) = 0;
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) = 0;
	virtual ~Command() {}
private:
	Command( const Command& ) = delete;
	Command( Command&& ) = delete;
	void operator=( const Command& ) = delete;
};

class HelpCommand : public Command
{
public:
	HelpCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~HelpCommand() {}
private:
	HelpCommand( const HelpCommand& ) = delete;
	HelpCommand( HelpCommand&& ) = delete;
	void operator=( const HelpCommand& ) = delete;
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class MarketCommand : public Command
{
public:
	MarketCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~MarketCommand() {}
private:
	MarketCommand( const MarketCommand& ) = delete;
	MarketCommand( MarketCommand&& ) = delete;
	void operator=( const MarketCommand& ) = delete;
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class PlayerCommand : public Command
{
public:
	PlayerCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~PlayerCommand() {}
private:
	PlayerCommand( const PlayerCommand& ) = delete;
	PlayerCommand( PlayerCommand&& ) = delete;
	void operator=( const PlayerCommand& ) = delete;
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class ListCommand : public Command
{
public:
	ListCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~ListCommand() {}
private:
	ListCommand( const ListCommand& ) = delete;
	ListCommand( ListCommand&& ) = delete;
	void operator=( const ListCommand& ) = delete;
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class ProdCommand : public Command
{
public:
	ProdCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~ProdCommand() {}
private:
	ProdCommand( const ProdCommand& ) = delete;
	ProdCommand( ProdCommand&& ) = delete;
	void operator=( const ProdCommand& ) = delete;
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class BuildCommand : public Command
{
public:
	BuildCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~BuildCommand() {}
private:
	BuildCommand( const BuildCommand& ) = delete;
	BuildCommand( BuildCommand&& ) = delete;
	void operator=( const BuildCommand& ) = delete;
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class BuyCommand : public Command
{
public:
	BuyCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~BuyCommand() {}
private:
	BuyCommand( const BuyCommand& ) = delete;
	BuyCommand( BuyCommand&& ) = delete;
	void operator=( const BuyCommand& ) = delete;
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class SellCommand : public Command
{
public:
	SellCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~SellCommand() {}
private:
	SellCommand( const SellCommand& ) = delete;
	SellCommand( SellCommand&& ) = delete;
	void operator=( const SellCommand& ) = delete;
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class TurnCommand : public Command
{
public:
	TurnCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~TurnCommand() {}
private:
	TurnCommand( const TurnCommand& ) = delete;
	TurnCommand( TurnCommand&& ) = delete;
	void operator=( const TurnCommand& ) = delete;
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class QuitCommand : public Command
{
public:
	QuitCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~QuitCommand() {}
private:
	QuitCommand( const QuitCommand& ) = delete;
	QuitCommand( QuitCommand&& ) = delete;
	void operator=( const QuitCommand& ) = delete;
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

#endif
