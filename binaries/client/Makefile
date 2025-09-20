CC = gcc
CWD = ../binaries/client
LIBS_PATH = ../../libs
SRCS_PATH = ../../srcs
C_SRCS = MGLib.c CommandsHistoryList.c Input.c clientCore.c
OBJMODULES = $(C_SRCS:.c=.o)
CFLAGS = -Wall -g
ADDRESS = 192.168.88.129
PORT = 7777


%.o: $(SRCS_PATH)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

client_release: client.c $(OBJMODULES)
	$(CC) $^ -o $@
	mv *.o $(LIBS_PATH)

client_debug: client.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@
	mv *.o $(LIBS_PATH)

run: client_release
	./$^ $(ADDRESS) $(PORT)

debug_run: client_debug
	valgrind --leak-check=full -s --track-origins=yes ./$^ $(ADDRESS) $(PORT)

clean:
	rm -rf $(LIBS_PATH)/*.o client_release client_debug

