#include "gameLogic.h"

Player *create_player()
{
    Player *player = (Player *)malloc(sizeof(Player));
    player->score = 0;
    player->tie = FALSE;
    return player;
}

Game *new_game(Player *player1, Player *player2)
{
    Game *game = (Game *)malloc(sizeof(Game));

    game->players[0] = player1;
    game->players[1] = player2;

    // chooses randomly who starts
    srand(time(NULL));
    game->turn = rand() % 2 == 0 ? player1 : player2;

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            game->board[i][j] = 4;
        }
    }
    return game;
}

Game *copy_game(Game *game)
{
    Game *copy = (Game *)malloc(sizeof(Game));
    copy->players[0] = game->players[0];
    copy->players[1] = game->players[1];
    copy->turn = game->turn;
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            copy->board[i][j] = game->board[i][j];
        }
    }
    return copy;
}

void free_player(Player *player)
{
    free(player);
}

void free_game(Game *game)
{
    free(game);
}

Player *get_current_player(Game *game)
{
    return game->turn;
}

Player *get_opponent(Player *player, Game *game)
{
    if (game->players[0] == player)
    {
        return game->players[1];
    }
    else
    {
        return game->players[0];
    }
}

int get_seeds(Pit pit, Game *game)
{
    return game->board[pit.line][pit.column];
}

void empty_seeds(Game *game, Player *player)
{
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            player->score += game->board[i][j];
            game->board[i][j] = 0;
        }
    }
}

boolean get_pit(int caseNumber, Pit *pit)
{
    if (caseNumber <= 0 || caseNumber > 12)
    {
        pit = NULL;
        return FALSE;
    }
    else if (caseNumber <= 6)
    {
        pit->line = 0;
        pit->column = caseNumber - 1;
    }
    else if (caseNumber <= 12)
    {
        pit->line = 1;
        pit->column = 12 - caseNumber;
    }
    return TRUE;
}

boolean is_valid_move(Pit pit, Game *game)
{
    if (pit.line < 0 || pit.line > 1 || pit.column < 0 || pit.column > 5 || get_seeds(pit, game) == 0 || (game->turn == game->players[0] && pit.line == 1) || (game->turn == game->players[1] && pit.line == 0))
    {
        return FALSE;
    }
    else if (is_starving(game, get_opponent(game->turn, game)) && !check_starvation(game, get_opponent(game->turn, game)))
    {
        if (game->turn == game->players[0])
        {
            return get_seeds(pit, game) >= 6 - pit.column;
        }
        else
        {
            return get_seeds(pit, game) >= pit.column + 1;
        }
    }
    return TRUE;
}

void make_move(Game **gamePtr, Pit pit)
{
    Game *game = *gamePtr;

    // If move is done, it means it's not a tie
    game->players[0]->tie = FALSE;
    game->players[1]->tie = FALSE;

    int seeds = get_seeds(pit, game);
    game->board[pit.line][pit.column] = 0;
    int line = pit.line;
    int column = pit.column;
    while (seeds > 0)
    {
        if (line == 0)
        {
            column++;
            if (column == pit.column && line == pit.line)
            {
                column++;
            }
            if (column > 5)
            {
                line = 1;
                column = 5;
            }
        }
        else
        {
            column--;
            if (column == pit.column && line == pit.line)
            {
                column--;
            }
            if (column < 0)
            {
                line = 0;
                column = 0;
            }
        }
        game->board[line][column]++;
        seeds--;
    }
    capture(gamePtr, (Pit){line, column});
    game = *gamePtr;
    game->turn = get_opponent(game->turn, game);
}

void capture(Game **gamePtr, Pit startingPit)
{
    Game *game = *gamePtr;
    boolean capture = FALSE;
    int currentLine = startingPit.line;
    int currentColumn = startingPit.column;
    int seedsCollected = 0;

    // Check if the starting pit is empty, or has 1 or more than 3 seeds, or the current player isn't in the opponent's line
    if ((game->board[currentLine][currentColumn] != 2 && game->board[currentLine][currentColumn] != 3) || (game->turn == game->players[0] && currentLine == 0) || (game->turn == game->players[1] && currentLine == 1))
        return;

    // Create a copy of the game where the seeds will be removed
    Game *gameCopy = copy_game(game);

    // Collect seeds and look at previous squares until conditions met
    while (game->board[currentLine][currentColumn] == 2 || game->board[currentLine][currentColumn] == 3)
    {
        seedsCollected += game->board[currentLine][currentColumn];
        gameCopy->board[currentLine][currentColumn] = 0;

        // Move to the previous column in the opposing camp
        if (currentLine == 0 && currentColumn > 0)
        {
            currentColumn--; // Move left on player 0's camp
        }
        else if (currentLine == 1 && currentColumn < 6)
        {
            currentColumn++; // Move right on player 1's camp
        }
        else
        {
            break;
        }
    }

    // Check if the opponent has seeds left in their camp
    boolean opponentHasSeeds = FALSE;
    for (int j = 0; j < 6; j++)
    {
        if (gameCopy->board[currentLine][j] > 0)
        {
            opponentHasSeeds = TRUE;
            break;
        }
    }

    // If the opponent has seeds, the capture is valid
    if (opponentHasSeeds)
    {
        free_game(game);
        *gamePtr = gameCopy;
        game = *gamePtr;
        game->turn->score += seedsCollected;
    }
    else
    {
        free_game(gameCopy);
    }
}

Player *player_line_empty(Game *game)
{
    for (int i = 0; i < 2; i++)
    {
        boolean lineEmpty = TRUE;
        for (int j = 0; j < 6; j++)
        {
            if (game->board[i][j] > 0)
            {
                lineEmpty = FALSE;
            }
        }
        if (lineEmpty)
        {
            return game->players[i];
        }
    }
    return NULL;
}

boolean is_game_over(Game *game)
{
    printf("Ah vraiment je rentre pas\r\n");
    boolean gameOver = FALSE;
    printf("263\r\n");
    Player *player;
    printf("265\r\n");
    // if one player has above 25 captured seeds
    if (game->players[0]->score >= 25 || game->players[1]->score >= 25)
    {
        gameOver = TRUE;
        printf("270\r\n");
    }
    // if one player has no seeds left on his line and the next player can't feed him
    else if ((player = player_line_empty(game)) != NULL && check_starvation(game, player))
    {
        empty_seeds(game, get_opponent(player, game));
        printf("276\r\n");
        gameOver = TRUE;
        printf("278\r\n");
    }
    // if it is not possible to capture seeds <-> players tie the game together
    else if (game->players[0]->tie && game->players[1]->tie)
    {
        gameOver = TRUE;
        printf("284\r\n");
    }
    if (gameOver)
        game->turn = NULL;
        printf("288\r\n");
    return gameOver;
}

boolean is_starving(Game *game, Player *player)
{
    int line = game->players[0] == player ? 0 : 1;
    for (int j = 0; j < 6; j++)
    {
        if (game->board[line][j] > 0)
        {
            return FALSE;
        }
    }
    return TRUE;
}

boolean check_starvation(Game *game, Player *player)
{
    if (game->players[0] == player)
    {
        for (int j = 0; j < 6; j++)
        {
            if (game->board[1][j] >= j + 1)
            {
                return FALSE;
            }
        }
    }
    else
    {
        for (int j = 5; j >= 0; j--)
        {
            if (game->board[0][j] >= 6 - j)
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

void tie(Game *game)
{
    game->players[0]->tie = game->turn == game->players[0] ? TRUE : game->players[0]->tie;
    game->players[1]->tie = game->turn == game->players[1] ? TRUE : game->players[1]->tie;
}

Player *get_winner(Game *game)
{
    if (game->players[0]->tie && game->players[1]->tie || game->players[0]->score == game->players[1]->score)
    {
        return NULL;
    }
    else if (game->players[0]->score > game->players[1]->score)
    {
        return game->players[0];
    }
    else
    {
        return game->players[1];
    }
}
