#ifndef COMMANDS_HANDLER_H
#define COMMANDS_HANDLER_H

struct CommandHandlerParams
{
	void* param1;
	void* param2;
};
typedef struct CommandHandlerParams CommandHandlerParams;

/* Константы для нумерации индексов валидных команд(порядок должен совпадать с порядком следования элементов в valid_commands) */
enum
{
	ERROR_COMMAND_NUM		=		-1,
	HELP_COMMAND_NUM,
	MARKET_COMMAND_NUM,
	PLAYER_COMMAND_NUM,
	LIST_COMMAND_NUM,
	PROD_COMMAND_NUM,
	BUILD_COMMAND_NUM,
	BUY_COMMAND_NUM,
	SELL_COMMAND_NUM,
	TURN_COMMAND_NUM,
	QUIT_COMMAND_NUM,
	UNKNOWN_COMMAND_NUM
};

#endif
