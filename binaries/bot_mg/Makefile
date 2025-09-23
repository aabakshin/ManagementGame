CC = gcc
CXX = g++
CWD = ../binaries/bot_mg
LIBS_PATH = ../../libs
SRCS_PATH = ../../srcs
C_SRCS = MGLib.c
CPP_SRCS = MainInfo.cpp botCore.cpp Lexem.cpp LexemList.cpp LexemAnalyzer.cpp OperStack.cpp PolizElem.cpp PolizEx.cpp PolizItem.cpp LabelsList.cpp SA_Additional.cpp SyntaxAnalyzer.cpp
OBJMODULES = $(C_SRCS:.c=.o) $(CPP_SRCS:.cpp=.o)
CFLAGS = -Wall -g
ADDRESS = 192.168.88.129
PORT = 7777
GAME_SCRIPT_FILE = ../../game_scripts/example.txt


MGLib.o: $(SRCS_PATH)/MGLib.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: $(SRCS_PATH)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

bot_mg_debug: bot_mg.cpp $(OBJMODULES)
	$(CXX) $(CFLAGS) $^ -o $@
	mv *.o $(LIBS_PATH)

bot_mg_release: bot_mg.cpp $(OBJMODULES)
	$(CXX) $^ -o $@
	mv *.o $(LIBS_PATH)

run: bot_mg_release
	./$^ $(ADDRESS) $(PORT) $(GAME_SCRIPT_FILE)

debug_run: bot_mg_debug
	valgrind --leak-check=full -s --track-origins=yes --show-leak-kinds=all ./$^ $(ADDRESS) $(PORT) $(GAME_SCRIPT_FILE)

clean:
	rm -rf $(LIBS_PATH)/*.o bot_mg_debug bot_mg_release
