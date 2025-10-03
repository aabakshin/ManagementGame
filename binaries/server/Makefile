CC = gcc
CWD = ../binaries/server
LIBS_PATH = ../../libs
SRCS_PATH = ../../srcs
C_SRCS = BuildList.c MarketRequest.c MGLib.c Banker.c serverCore.c
OBJMODULES = $(C_SRCS:.c=.o)
CFLAGS = -Wall -g
PORT = 7777


%.o: $(SRCS_PATH)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

server_release: server.c $(OBJMODULES)
	$(CC) $^ -o $@
	mv *.o $(LIBS_PATH)

server_debug: server.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@
	mv *.o $(LIBS_PATH)

run: server_release
	./$^ $(PORT)

debug_run: server_debug
	valgrind --leak-check=full -s --track-origins=yes --show-leak-kinds=all ./$^ $(PORT)

clean:
	rm -rf $(LIBS_PATH)/*.o server_release server_debug
