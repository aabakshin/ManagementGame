CC = gcc
LIBS_PATH = ../../libs
SRCMODULES = $(LIBS_PATH)/MGLib.c $(LIBS_PATH)/CommandsHistoryList.c $(LIBS_PATH)/clientCore.c
OBJMODULES = $(SRCMODULES:.c=.o)
CFLAGS = -Wall -g
ADDRESS = 192.168.50.128
PORT = 7777

%.o: $(LIBS_PATH)/%.c
	$(CC) $(CFLAGS) -c $< -o $(LIBS_PATH)/$@

client: client.c $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@

run: client
	./client $(ADDRESS) $(PORT)

clean:
	rm -rf $(OBJMODULES) client

