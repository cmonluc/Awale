SERVER_EXE = server
CLIENT_EXE = client
GAME_EXE = game
COMPILER = gcc
FLAGS = -c -ansi -pedantic -Wall # -Werror
DEBUG_FLAGS = -g
PERF_FLAGS = -DPERF
BUILD_PATH = ./build
BIN_PATH = ./bin
SRC_PATH = ./src
CLIENT_PATH = $(SRC_PATH)/client
SERVER_PATH = $(SRC_PATH)/server
GAME_PATH = $(SRC_PATH)/game
INCLUDE_PATH = -I ./interfaces

server: ./src/server/server.c display.o gamelogic.o
	gcc -o server ./src/server/server.c display.o gamelogic.o $(DEBUG_FLAGS)

client: ./src/client/client.c
	gcc -o client ./src/client/client.c $(DEBUG_FLAGS)

game: ./src/game/game.c gamelogic.o display.o
	gcc -o game ./src/game/game.c display.o gamelogic.o $(DEBUG_FLAGS)

gameLogic.o: ./src/game/gamelogic.c
	gcc -c -o gamelogic.o ./src/game/gamelogic.c $(DEBUG_FLAGS)

display.o: $(GAME_PATH)/display.c
	gcc -c -o display.o ./src/game/display.c $(DEBUG_FLAGS)


clean:
	rm -rf $(BIN_PATH)/* $(BUILD_PATH)/*.o