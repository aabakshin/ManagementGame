#ifndef COMMAND_HPP_SENTINEL
#define COMMAND_HPP_SENTINEL

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
		COMMANDS_COUNT			=		10,
		MESSAGE_TOKEN_SIZE		=	   100
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
		CommandParams( const CommandParams& ) {}
		CommandParams( CommandParams&& ) {}
		void operator=( const CommandParams& ) {}
	};
	class MessageTokens
	{
	private:
		const char** message_tokens;
		int max_msg_tokens_count;
		int msg_tokens_count;
	public:
		MessageTokens() {}
		void MakeMessageTokens( int );
		const char*& operator[]( int );
		const char** GetValue() const { return message_tokens; }
		int GetMsgTokensCount() const { return msg_tokens_count; }
		void SetMsgTokensCount( int );
		~MessageTokens();
	private:
		MessageTokens( const MessageTokens& ) {}
		MessageTokens( MessageTokens&& ) {}
		void operator=( const MessageTokens& ) {}
		void NullifyMessageTokens();
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
	Command(const Command& obj) {}
	Command(Command&& obj) {}
	void operator=( const Command& obj ) {}
};

class HelpCommand : public Command
{
public:
	HelpCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~HelpCommand() {}
private:
	void operator=( const HelpCommand& obj ) {}
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class MarketCommand : public Command
{
public:
	MarketCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~MarketCommand() {}
private:
	void operator=( const MarketCommand& obj ) {}
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class PlayerCommand : public Command
{
public:
	PlayerCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~PlayerCommand() {}
private:
	void operator=( const PlayerCommand& obj ) {}
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class ListCommand : public Command
{
public:
	ListCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~ListCommand() {}
private:
	void operator=( const ListCommand& obj ) {}
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class ProdCommand : public Command
{
public:
	ProdCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~ProdCommand() {}
private:
	void operator=( const ProdCommand& obj ) {}
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class BuildCommand : public Command
{
public:
	BuildCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~BuildCommand() {}
private:
	void operator=( const PlayerCommand& obj ) {}
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class BuyCommand : public Command
{
public:
	BuyCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~BuyCommand() {}
private:
	void operator=( const BuyCommand& obj ) {}
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class SellCommand : public Command
{
public:
	SellCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~SellCommand() {}
private:
	void operator=( const SellCommand& obj ) {}
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class TurnCommand : public Command
{
public:
	TurnCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~TurnCommand() {}
private:
	void operator=( const TurnCommand& obj ) {}
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

class QuitCommand : public Command
{
public:
	QuitCommand( int );
	virtual void PrepareAndProc( int, int, const char*, const char*, const BCBrokerMessages& ) override;
	virtual ~QuitCommand() {}
private:
	void operator=( const QuitCommand& obj ) {}
	virtual void Process( int, const Command::CommandParams&, const BCBrokerMessages& ) override;
};

#endif
