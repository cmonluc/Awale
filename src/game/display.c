#include "display.h"
#include <string.h>

void print_board(Game *game)
{
    printf("\n\n\n");
    printf("Direction " RED "--->\n" RESET);
    printf("Case    |  1   2   3   4   5   6  | Scores\n");
    printf("---------------------------------------------------\n");
    printf(GREEN "%s P1  | %2d  %2d  %2d  %2d  %2d  %2d  | %d\n" RESET, game->turn == game->players[0] ? RED "-->" GREEN : "   ", game->board[0][0], game->board[0][1], game->board[0][2], game->board[0][3], game->board[0][4], game->board[0][5], game->players[0]->score);
    printf("---------------------------------------------------\n");
    printf(PURPLE "%s P2  | %2d  %2d  %2d  %2d  %2d  %2d  | %d\n" RESET, game->turn == game->players[1] ? RED "-->" PURPLE : "   ", game->board[1][0], game->board[1][1], game->board[1][2], game->board[1][3], game->board[1][4], game->board[1][5], game->players[1]->score);
    printf("---------------------------------------------------\n");
    printf("          12  11  10   9   8   7\n");
}

// char* buffer buffer_board(Game *game)
// {
//     char buffer[BUF_SIZE];
//     buffer[0] = 0;
//     strncat(buffer, "\n\n\n", BUF_SIZE-strlen(buffer)-1);
//     strncat(buffer,"Direction " RED "--->\n" RESET, BUF_SIZE-strlen(buffer)-1);
//     strncat(buffer,"Case    |  1   2   3   4   5   6  | Scores\n", BUF_SIZE-strlen(buffer)-1);
//     strncat(buffer,"---------------------------------------------------\n", BUF_SIZE-strlen(buffer)-1);
//     strncat(buffer,GREEN "%s P1  | %2d  %2d  %2d  %2d  %2d  %2d  | %d\n" RESET, game->turn == game->players[0] ? RED "-->" GREEN : "   ", game->board[0][0], game->board[0][1], game->board[0][2], game->board[0][3], game->board[0][4], game->board[0][5], game->players[0]->score, BUF_SIZE-strlen(buffer)-1);
//     strncat(buffer,"---------------------------------------------------\n", BUF_SIZE-strlen(buffer)-1);
//     strncat(buffer,PURPLE "%s P2  | %2d  %2d  %2d  %2d  %2d  %2d  | %d\n" RESET, game->turn == game->players[1] ? RED "-->" PURPLE : "   ", game->board[1][0], game->board[1][1], game->board[1][2], game->board[1][3], game->board[1][4], game->board[1][5], game->players[1]->score, BUF_SIZE-strlen(buffer)-1);
//     strncat(buffer,"---------------------------------------------------\n", BUF_SIZE-strlen(buffer)-1);
//     strncat(buffer,"          12  11  10   9   8   7\n", BUF_SIZE-strlen(buffer)-1);
//     return buffer;
// }

// char* buffer_board(Game *game)
// {
//     static char buffer[BUF_SIZE];  // Utilisation de 'static' pour que la fonction retourne un pointeur valide
//     buffer[0] = '\0';  // Initialisation de la chaîne vide

//     // Ajouter du contenu dans le buffer
//     strncat(buffer, "\n\n\n", BUF_SIZE - strlen(buffer) - 1);
//     strncat(buffer, "Direction " RED "--->\n" RESET, BUF_SIZE - strlen(buffer) - 1);
//     strncat(buffer, "Case    |  1   2   3   4   5   6  | Scores\n", BUF_SIZE - strlen(buffer) - 1);
//     strncat(buffer, "---------------------------------------------------\n", BUF_SIZE - strlen(buffer) - 1);

//     // Ajouter les informations du joueur 1
//     strncat(buffer, GREEN, BUF_SIZE - strlen(buffer) - 1);
//     strncat(buffer, (game->turn == game->players[0]) ? RED "-->" GREEN : "   ", BUF_SIZE - strlen(buffer) - 1);
//     snprintf(buffer + strlen(buffer), BUF_SIZE - strlen(buffer), "%s P1  | %2d  %2d  %2d  %2d  %2d  %2d  | %d\n", 
//              (game->turn == game->players[0]) ? RED "-->" GREEN : "   ", 
//              game->board[0][0], game->board[0][1], game->board[0][2], game->board[0][3], game->board[0][4], game->board[0][5], 
//              game->players[0]->score);

//     strncat(buffer, "---------------------------------------------------\n", BUF_SIZE - strlen(buffer) - 1);

//     // Ajouter les informations du joueur 2
//     strncat(buffer, PURPLE, BUF_SIZE - strlen(buffer) - 1);
//     strncat(buffer, (game->turn == game->players[1]) ? RED "-->" PURPLE : "   ", BUF_SIZE - strlen(buffer) - 1);
//     snprintf(buffer + strlen(buffer), BUF_SIZE - strlen(buffer), "%s P2  | %2d  %2d  %2d  %2d  %2d  %2d  | %d\n", 
//              (game->turn == game->players[1]) ? RED "-->" PURPLE : "   ", 
//              game->board[1][0], game->board[1][1], game->board[1][2], game->board[1][3], game->board[1][4], game->board[1][5], 
//              game->players[1]->score);

//     strncat(buffer, "---------------------------------------------------\n", BUF_SIZE - strlen(buffer) - 1);
//     strncat(buffer, "          12  11  10   9   8   7\n", BUF_SIZE - strlen(buffer) - 1);

//     return buffer;
// }

void construct_board(Game *game, char *board, char *nameP1, char *nameP2)
{
    sprintf(board, "\n\n\n");
    sprintf(board, "%s\t\tDirection " CYAN "--->\n\n" RESET, board);
    sprintf(board, "%sCase    |  1   2   3   4   5   6  | Scores\n", board);
    sprintf(board, "%s--------------------------------------------\n", board);
    sprintf(board, "%s" GREEN "%s P1  | %2d  %2d  %2d  %2d  %2d  %2d  | %d          %s\n" RESET, board, game->turn == game->players[0] ? RED "-->" GREEN : "   ", game->board[0][0], game->board[0][1], game->board[0][2], game->board[0][3], game->board[0][4], game->board[0][5], game->players[0]->score, nameP1);
    sprintf(board, "%s--------------------------------------------\n", board);
    sprintf(board, "%s" PURPLE "%s P2  | %2d  %2d  %2d  %2d  %2d  %2d  | %d          %s\n" RESET, board, game->turn == game->players[1] ? RED "-->" PURPLE : "   ", game->board[1][0], game->board[1][1], game->board[1][2], game->board[1][3], game->board[1][4], game->board[1][5], game->players[1]->score, nameP2);
    sprintf(board, "%s--------------------------------------------\n", board);
    sprintf(board, "%s          12  11  10   9   8   7\n", board);
}

void construct_turn_message(Game *game, char *message)
{
    sprintf(message, "It's player %d's turn !\n", game->turn == game->players[0] ? 1 : 2);
}