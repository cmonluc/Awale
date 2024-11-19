SERVER_EXE = server
CLIENT_EXE = client
GAME_EXE = game
COMPILER = gcc
FLAGS = -c -ansi -pedantic -Wall #-Werror
DEBUG_FLAGS = -g
PERF_FLAGS = -DPERF
BUILD_PATH = ./build
BIN_PATH = ./bin
SRC_PATH = ./src
CLIENT_PATH = $(SRC_PATH)/client
SERVER_PATH = $(SRC_PATH)/server
GAME_PATH = $(SRC_PATH)/game
INCLUDE_PATH = -I ./interfaces

${SERVER_EXE}: ${SERVER_PATH}/server.c ${BUILD_PATH}/game.o ${BUILD_PATH}/display.o ${BUILD_PATH}/gamelogic.o
	gcc -o ${BIN_PATH}/${SERVER_EXE} ${SERVER_PATH}/server.c ${BUILD_PATH}/game.o ${BUILD_PATH}/display.o ${BUILD_PATH}/gamelogic.o ${INCLUDE_PATH} $(DEBUG_FLAGS) -w

${CLIENT_EXE}: ${CLIENT_PATH}/client.c
	gcc -o ${BIN_PATH}/${CLIENT_EXE} ${CLIENT_PATH}/client.c ${INCLUDE_PATH} $(DEBUG_FLAGS)

${BUILD_PATH}/game.o: $(GAME_PATH)/game.c
	gcc -c -o ${BUILD_PATH}/game.o ${GAME_PATH}/game.c ${INCLUDE_PATH} $(DEBUG_FLAGS)


${BUILD_PATH}/gamelogic.o: $(GAME_PATH)/gamelogic.c
	gcc -c -o ${BUILD_PATH}/gamelogic.o ${GAME_PATH}/gamelogic.c ${INCLUDE_PATH} $(DEBUG_FLAGS)

${BUILD_PATH}/display.o: $(GAME_PATH)/display.c
	gcc -c -o ${BUILD_PATH}/display.o ${GAME_PATH}/display.c ${INCLUDE_PATH} $(DEBUG_FLAGS)


clean:
	rm -rf $(BIN_PATH)/* $(BUILD_PATH)/*.o