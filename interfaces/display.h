#ifndef DISPLAY_H
#define DISPLAY_H

#define RED "\033[0;31m"
#define RESET "\033[0m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define PURPLE "\033[0;35m"
#define CYAN "\033[0;36m"
#define WHITE "\033[0;37m"

#define BUF_SIZE    1024

#include "gameLogic.h"
#include <stdio.h>

// Prints the game board on stdout.
void print_board(Game *game);

// Construct the string of a game board.
void construct_board(Game *game, char *board, char *nameP1, char *nameP2);

// Construct the string of a turn message.
void construct_turn_message(Game *game, char *message);

#endif
