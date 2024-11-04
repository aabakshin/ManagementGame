CC = gcc
LIBS_PATH = ../../libs
SRCMODULES = $(LIBS_PATH)/BuildList.c $(LIBS_PATH)/MarketRequest.c $(LIBS_PATH)/MGLib.c $(LIBS_PATH)/Banker.c $(LIBS_PATH)/CommandsHandler.c $(LIBS_PATH)/serverCore.c
OBJMODULES = $(SRCMODULES:.c=.o)
CFLAGS = -Wall -g
PORT = 7777

%.o: $(LIBS_PATH)/%.c
	$(CC) $(CFLAGS) -c $< -o $(LIBS_PATH)/$@

server: server.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@

run: server
	./server $(PORT)

clean:
	rm -rf $(OBJMODULES) server
