CC = gcc
CXX = g++
LIBS_PATH = ../../libs
C_SRC_MODULES = $(LIBS_PATH)/MGLib.c
CPP_SRC_MODULES = $(LIBS_PATH)/MainInfo.cpp $(LIBS_PATH)/botCore.cpp $(LIBS_PATH)/Lexem.cpp $(LIBS_PATH)/LexemList.cpp $(LIBS_PATH)/LexemAnalyzer.cpp $(LIBS_PATH)/OperStack.cpp $(LIBS_PATH)/PolizElem.cpp $(LIBS_PATH)/PolizEx.cpp $(LIBS_PATH)/PolizItem.cpp $(LIBS_PATH)/LabelsList.cpp $(LIBS_PATH)/SA_Additional.cpp $(LIBS_PATH)/SyntaxAnalyzer.cpp
OBJMODULES = $(C_SRC_MODULES:.c=.o) $(CPP_SRC_MODULES:.cpp=.o)
CFLAGS = -Wall -g
ADDRESS = 192.168.50.128
PORT = 7777
GAME_SCRIPT_FILE = ../../game_scripts/example.txt

MGLib.o: $(LIBS_PATH)/MGLib.c
	$(CC) $(CFLAGS) -c $< -o $(LIBS_PATH)/$@
%.o: $(LIBS_PATH)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $(LIBS_PATH)/$@

bot_mg: bot_mg.cpp $(OBJMODULES)
	$(CXX) $(CFLAGS) $^ -o $@

run: bot_mg
	./bot_mg $(ADDRESS) $(PORT) $(GAME_SCRIPT_FILE)

clean:
	rm -rf $(OBJMODULES) bot_mg

