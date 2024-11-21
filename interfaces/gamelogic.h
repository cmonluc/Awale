#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdlib.h>
#include <time.h>

#define TRUE 1
#define FALSE 0
#define BUF_SIZE 1024
typedef int boolean;

typedef struct Player
{
    int score;
    boolean tie;
} Player;

typedef struct Game
{
    Player *players[2];
    Player *turn;
    int board[2][6];
    int isPrivate;
} Game;

typedef struct
{
    int line;
    int column;
} Pit;

// Initializes a new player
Player *create_player();

// Initializes a new game with the given player names and returns a pointer to the game.
Game *new_game(Player *player1, Player *player2);

// Returns a copy of the given game.
Game *copy_game(Game *game);

// Frees the memory allocated for the given player.
void free_player(Player *player);

// Frees the memory allocated for the given game.
void free_game(Game *game);

// Returns the current player whose turn it is.
Player *get_current_player(Game *game);

// Returns the opponent of the given player.
Player *get_opponent(Player *player, Game *game);

// Returns the number of seeds in the given pit.
int get_seeds(Pit pit, Game *game);

// Give all remaining seeds to the player.
void empty_seeds(Game *game, Player *player);

// Find the pit from the case number between 1 and 12, returns true if found, false otherwise.
boolean get_pit(int caseNumber, Pit *pit);

// Returns true if the given move is valid for the current player, false otherwise.
boolean is_valid_move(Pit pit, Game *game);

// Executes the given legal move for the current player.
void make_move(Game **game, Pit pit);

// Captures the seeds of the opponent if possible.
void capture(Game **game, Pit pit);

// Returns the player whose line is empty if there is, null otherwise.
Player *player_line_empty(Game *game);

// Returns true if the game is over, false otherwise.
boolean is_game_over(Game *game);

// Returns true if the given player is starving, false otherwise.
boolean is_starving(Game *game, Player *player);

// Returns true if the given player can't be fed, false otherwise.
boolean check_starvation(Game *game, Player *player);

// Put the current player to wanting to tie.
void tie(Game *game);

// Returns the winner of the game, or NULL if the game is not over or it is a tie.
Player *get_winner(Game *game);

// Returns the winner of the game, or NULL if the game is not over or it is a tie.
Player *get_loser(Game *game);

#endif // GAME_LOGIC_H
