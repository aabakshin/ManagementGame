/*
 *	Модуль CommandsHandler содержит обработчики игровых команд,а также
 *	вспомогательную структуру для передачи данных из клиентского ответа
 *	в обработчик команд
 *	Этот модуль вызывается только в модуле serverCore
 *
 */

#ifndef COMMANDS_HANDLER_H
#define COMMANDS_HANDLER_H

#include "Banker.h"

struct CommandHandlerParams
{
	int fd;
	void* param1;
	void* param2;
};
typedef struct CommandHandlerParams CommandHandlerParams;

void help_command_handler(Banker* b, CommandHandlerParams* chp);
void market_command_handler(Banker* b, CommandHandlerParams* chp);
void player_command_handler(Banker* b, CommandHandlerParams* chp);
void list_command_handler(Banker* b, CommandHandlerParams* chp);
void prod_command_handler(Banker* b, CommandHandlerParams* chp);
void build_command_handler(Banker* b, CommandHandlerParams* chp);
void buy_command_handler(Banker* b, CommandHandlerParams* chp);
void sell_command_handler(Banker* b, CommandHandlerParams* chp);
void turn_command_handler(Banker* b, CommandHandlerParams* chp);
void quit_command_handler(Banker* b, CommandHandlerParams* chp);



#endif
