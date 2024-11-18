#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server.h"
#include "clientServer.h"
#include "gamelogic.h"
#include "display.h"

static void init(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if(err < 0)
   {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
   }
#endif
}

static void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

Client *clients[MAX_CLIENTS];

char* buffer_board(Game *game)
{
    char buffer[BUF_SIZE];
    buffer[0] = 0;  // Initialize buffer to be empty

    // Add the initial lines
    strncat(buffer, "\n\n\n", BUF_SIZE - strlen(buffer) - 1);
    strncat(buffer, "Direction " RED "--->\n" RESET, BUF_SIZE - strlen(buffer) - 1);
    strncat(buffer, "Case    |  1   2   3   4   5   6  | Scores\n", BUF_SIZE - strlen(buffer) - 1);
    strncat(buffer, "---------------------------------------------------\n", BUF_SIZE - strlen(buffer) - 1);

    // Add the board for Player 1
    char player1_row[100]; // Temporary buffer for player 1's row
    snprintf(player1_row, sizeof(player1_row),
             GREEN "%s P1  | %2d  %2d  %2d  %2d  %2d  %2d  | %d\n" RESET,
             game->turn == game->players[0] ? RED "-->" GREEN : "   ",
             game->board[0][0], game->board[0][1], game->board[0][2],
             game->board[0][3], game->board[0][4], game->board[0][5],
             game->players[0]->score);
    strncat(buffer, player1_row, BUF_SIZE - strlen(buffer) - 1);

    strncat(buffer, "---------------------------------------------------\n", BUF_SIZE - strlen(buffer) - 1);

    // Add the board for Player 2
    char player2_row[100]; // Temporary buffer for player 2's row
    snprintf(player2_row, sizeof(player2_row),
             PURPLE "%s P2  | %2d  %2d  %2d  %2d  %2d  %2d  | %d\n" RESET,
             game->turn == game->players[1] ? RED "-->" PURPLE : "   ",
             game->board[1][0], game->board[1][1], game->board[1][2],
             game->board[1][3], game->board[1][4], game->board[1][5],
             game->players[1]->score);
    strncat(buffer, player2_row, BUF_SIZE - strlen(buffer) - 1);

    strncat(buffer, "---------------------------------------------------\n", BUF_SIZE - strlen(buffer) - 1);

    // Add the row of numbers at the bottom
    strncat(buffer, "          12  11  10   9   8   7\n", BUF_SIZE - strlen(buffer) - 1);

    return buffer;
}

static void app(void)
{
   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int max = sock;
   /* an array for all clients */

   fd_set rdfs;

   while(1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for(i = 0; i < actual; i++)
      {
         FD_SET(clients[i]->sock, &rdfs);
      }

      if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs))
      {
         /* stop process when type on keyboard */
         break;
      }
      else if(FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = { 0 };
         size_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if(csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }

         /* after connecting the client sends its name */
         if(read_client(csock, buffer) == -1)
         {
            /* disconnected */
            continue;
         }

         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);
         Client *client = (Client*)malloc(sizeof(Client));
         client->sock = csock;
         client->game=NULL;
         client->player=NULL;
         client->challengedBy=NULL;
         strncpy(client->name, buffer, BUF_SIZE-1);
         int index = add_client(client);
         actual++;
         menu(client);
      }
      else
      {
         int i = 0;
         for(i = 0; i < actual; i++)
         {
            /* a client is talking */
            if(FD_ISSET(clients[i]->sock, &rdfs))
            {
               Client *client = clients[i];
               int c = read_client(clients[i]->sock, buffer);
               /* client disconnected */
               if(c == 0)
               {
                  closesocket(clients[i]->sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, client->name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clients, *client, actual, buffer, 1);
               }
               else
               {
                    if (strcmp(buffer,"1")==0){
                        list_clients(buffer, *clients[i]);
                    }
                    else if (strcmp(buffer,"2")==0){
                        challengeClient(clients[i]);
                    }
                    else if (strcmp(buffer, "accept") == 0)
                     {
                        if (client->challengedBy == NULL)
                        {
                           write_client(client->sock, "\nYou don't have any challenge to accept !\n");
                        }
                        else
                        {
                           write_client(client->sock, "\nYou accepted the challenge !\n Creating the game...\n" );

                           Player *p1 = create_player();
                           Player *p2 = create_player();
                           Game *game = new_game(p1, p2);
                           client->game = game;
                           client->challengedBy->game = game;
                           client->player = p1;
                           client->challengedBy->player = p2;

                           char buffer[BUF_SIZE];
                           snprintf(buffer, BUF_SIZE, "\nGame created between P1 : %s and P2 : %s !\n", client->name, client->challengedBy->name);
                           write_client(client->sock, buffer);
                           write_client(client->challengedBy->sock, buffer);
                           handleGame(client);
                           // handleGame(client->challengedBy);
                        }
                     }
                }
            }
         }
      }
   }

   clear_clients(clients, actual);
   end_connection(sock);
}

static void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

static void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

static void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for(i = 0; i < actual; i++)
   {
      /* we don't send message to the sender */
      if(sender.sock != clients[i].sock)
      {
         if(from_server == 0)
         {
            strncpy(message, sender.name, BUF_SIZE - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
         write_client(clients[i].sock, message);
      }
   }
}

static int init_connection(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
{
   int n = 0;

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

static void write_client(SOCKET sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

static int is_valid_pseudo(const char *pseudo){
   int length = strlen(pseudo);

   if (length<3 || length >= BUF_SIZE){
      return 0;
   }

   return 1 ;
}

static int add_client(Client *newClient) {
    char buffer[BUF_SIZE];
    int is_unique = 0;
    int attempts = 0;

    // Si le client n'a pas encore de nom, on lui demande un nom
    if (strcmp(newClient->name, "0") == 0) {
        while (!is_unique && attempts < MAX_PSEUDO_ATTEMPTS) {
            write_client(newClient->sock, "Please enter a username (3+ characters, no special symbols):");

            // Lire le nom d'utilisateur du client
            if (read_client(newClient->sock, buffer) <= 0) {
                write_client(newClient->sock, "Error reading username. Try again.\r\n");
                continue;
            }

            buffer[BUF_SIZE - 1] = '\0';

            // Vérifier la validité du nom
            if (!is_valid_pseudo(buffer)) {
                write_client(newClient->sock, "Invalid username. Must be at least 3 characters, alphanumeric only.\r\n");
                attempts++;
                continue;
            }

            // Vérifier l'unicité du nom d'utilisateur
            is_unique = 1;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] != NULL && strcmp(clients[i]->name, buffer) == 0) {
                    write_client(newClient->sock, "Username already taken. Please choose another.\r\n");
                    is_unique = 0;
                    break;
                }
            }
        }

        // Si trop de tentatives échouées, déconnecter le client
        if (attempts >= MAX_PSEUDO_ATTEMPTS) {
            write_client(newClient->sock, "Too many invalid attempts. Disconnecting.\r\n");
            return -1;  // Déconnecter après trop d'échecs
        }

        // Si le nom est valide, on le sauvegarde
        strncpy(newClient->name, buffer, BUF_SIZE - 1);
    }
    else {
        // Si le client a déjà un nom, on vérifie s'il est valide et unique
        while (1) {
            // Vérifier la validité du nom
            if (!is_valid_pseudo(newClient->name)) {
                write_client(newClient->sock, "Invalid username. Must be at least 3 characters, alphanumeric only.\r\n");
                attempts++;
                continue;  // Nom invalide
            }

            // Vérifier l'unicité du nom
            is_unique = 1;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] != NULL && strcmp(clients[i]->name, newClient->name) == 0) {
                    write_client(newClient->sock, "Username already taken. Please choose another.\r\n");
                    is_unique = 0;
                    break;
                }
            }

            // Si le nom est unique et valide, on l'accepte
            if (is_unique) {
                break;
            }

            // Demander un nouveau nom si nécessaire
            write_client(newClient->sock, "Please enter a username (3+ characters, no special symbols):");

            if (read_client(newClient->sock, buffer) <= 0) {
                write_client(newClient->sock, "Error reading username. Try again.\r\n");
                continue;
            }

            buffer[BUF_SIZE - 1] = '\0';
            if (!is_valid_pseudo(buffer)) {
                write_client(newClient->sock, "Invalid username. Must be at least 3 characters, alphanumeric only.\r\n");
                continue;
            }

            if (attempts >= MAX_PSEUDO_ATTEMPTS) {
            write_client(newClient->sock, "Too many invalid attempts. Disconnecting.\r\n");
            return -1;  // Déconnecter après trop d'échecs
        }

            // Si le nom est valide et unique, on le met à jour
            strncpy(newClient->name, buffer, BUF_SIZE - 1);
        }
    }

    // Trouver une place libre pour le client
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == NULL) {
            clients[i] = newClient;
            return i;  // Retourner l'index du client
        }
    }

    // Si la table des clients est pleine, renvoyer une erreur
    write_client(newClient->sock, "The server is full, please try again later.\r\n");
    return -1;
}

static void menu(Client *client)
{
    char buffer[BUF_SIZE];
    buffer[0] = 0;
    strncat(buffer, "\r\nWelcome on Awale ! Here is the menu, choose a number : \r\n", BUF_SIZE-strlen(buffer)-1);
    strncat(buffer, "1 - View all online players\r\n", BUF_SIZE-strlen(buffer)-1);
    strncat(buffer, "2 - Challenge a player\r\n", BUF_SIZE-strlen(buffer)-1);
    write_client(client->sock, buffer);
}

static void list_clients(char *buffer, Client client)
{
    buffer[0] = 0;
    strncat(buffer, "List of clients : \r\n", BUF_SIZE-strlen(buffer)-1);
    for (int i=0 ; i<MAX_CLIENTS; i++)
    {
        if (clients[i] != NULL)
        {
            strncat(buffer, clients[i]->name, BUF_SIZE-strlen(buffer)-1);
            if (strcmp(clients[i]->name, client.name)==0)
            {
                strncat(buffer, " (you)", BUF_SIZE-strlen(buffer)-1);
            }

            else if (clients[i]->game != NULL)
            {   
                strncat(buffer, " (in game)", BUF_SIZE-strlen(buffer)-1);
            }

            strncat(buffer, "\r\n", BUF_SIZE-strlen(buffer)-1);
        }
    }
    write_client(client.sock, buffer);
}

static void list_clients_not_in_game(char *buffer, Client *client)
{
   buffer[0] = 0;
   strncat(buffer, "\nList of clients:\n", BUF_SIZE - strlen(buffer) - 1);
   for (int i = 0; i < MAX_CLIENTS; i++)
   {
      if (clients[i] != NULL && clients[i]->game == NULL && clients[i]->challengedBy == NULL)
      {
         strncat(buffer, "\t- ", BUF_SIZE - strlen(buffer) - 1);
         if (clients[i] == client)
         {
            //strncat(buffer,GREEN, BUF_SIZE - strlen(buffer) - 1);
            strncat(buffer, clients[i]->name, BUF_SIZE - strlen(buffer) - 1);
            strncat(buffer, " (you)", BUF_SIZE - strlen(buffer) - 1);
            //strncat(buffer,RESET, BUF_SIZE - strlen(buffer) - 1);
         }
         else
         {

            strncat(buffer, clients[i]->name, BUF_SIZE - strlen(buffer) - 1);
         }
         strncat(buffer, "\n", BUF_SIZE - strlen(buffer) - 1);
      }
   }
}

static Client *getClientByName(const char *name)
{
   for (int i = 0; i < MAX_CLIENTS; i++)
   {
      if (clients[i] != NULL && strcmp(clients[i]->name, name) == 0)
      {
         return clients[i];
      }
   }
   return NULL;
}

static int challengeClient(Client *challenger)
{
   char buffer[BUF_SIZE];
   char list[BUF_SIZE];
   while (1)
   {
      // boolean isPrivate = FALSE;
      // write_client(challenger->sock, "\nDo you want to make a " RED "private" RESET " or " RED "public" RESET " challenge ?\n" RED "cancel" RESET " to cancel\n");
      // while (1)
      // {
      //    buffer[0] = 0;
      //    if (read_client(challenger->sock, buffer) <= 0)
      //       return SOCKET_ERROR;
      //    else if (strcmp(buffer, "cancel") == 0)
      //    {
      //       return EXIT_SUCCESS;
      //    }
      //    else if (strcmp(buffer, "private") == 0)
      //    {
      //       isPrivate = TRUE;
      //       break;
      //    }
      //    else if (strcmp(buffer, "public") == 0)
      //    {
      //       isPrivate = FALSE;
      //       break;
      //    }
      //    else
      //    {
      //       write_client(challenger->sock, RED "Invalid input !\n" RESET "Enter " RED "private" RESET " or " RED "public" RESET "\n");
      //    }
      // }

      write_client(challenger->sock, "\nWho do you want to challenge ?\ncancel to cancel\n");
      list_clients_not_in_game(buffer, challenger);
      write_client(challenger->sock, buffer);

      buffer[0] = 0;
      if (read_client(challenger->sock, buffer) <= 0)
         return SOCKET_ERROR;
      if (strcmp(buffer, "cancel") == 0)
      {
         break;
      }
      Client *challengee = getClientByName(buffer);
      char message[BUF_SIZE];
      if (challengee == NULL)
      {
         snprintf(message, BUF_SIZE, "\nPlayer %s not found !\n", buffer);
         write_client(challenger->sock, message);
      }
      else if (challengee->game != NULL)
      {
         snprintf(message, BUF_SIZE, "\nPlayer %s is already playing !\n", buffer);
         write_client(challenger->sock, message);
      }
      else if (challenger->challengedBy != NULL)
      {
         snprintf(message, BUF_SIZE, "\nYou are already challenged by %s !\n", challenger->challengedBy->name);
         write_client(challenger->sock, message);
      }
      else if (challengee == challenger)
      {
         snprintf(message, BUF_SIZE, "\nYou can't challenge yourself !\n");
         write_client(challenger->sock, message);
      }
      else if (challengee->challengedBy != NULL)
      {
         snprintf(message, BUF_SIZE, "\nPlayer %s is already challenged !\n", buffer);
         write_client(challenger->sock, message);
      }
      else
      {
         challengee->challengedBy = challenger;
         challenger->challengedBy = challengee;

         // Before creating the game, set if it's private or not
         // It can be checked only here since game is created just after this event
         // challengee->isPrivate = isPrivate;
         // challenger->isPrivate = isPrivate;

         snprintf(message, BUF_SIZE, "\n%s wants to challenge you in a game!\n", challenger->name);
         write_client(challengee->sock, message);
         snprintf(message, BUF_SIZE, "\nType accept to accept the challenge !");
         write_client(challengee->sock, message);

         snprintf(message, BUF_SIZE, "\nWaiting for %s's response...\n", challengee->name);
         write_client(challenger->sock, message);
         break;
      }
   }
   // Wait for the challengee to accept or refuse
   // while (challenger->challengedBy != NULL)
   // {
   //    char tempBuffer[BUF_SIZE];
   //    // Check for disconnect while waiting
   //    if (!check_socket(challenger->sock, tempBuffer))
   //    {
   //       return SOCKET_ERROR;
   //    }
   //    // If a game is created, it means the other accepted it
   //    else if (challenger->game != NULL)
   //    {
   //       break;
   //    }
   // }
   return EXIT_SUCCESS;
}

// static int check_socket(int sockFd, char *tempBuffer)
// {
//    fd_set read_fds;
//    struct timeval tv;
//    FD_ZERO(&read_fds);
//    FD_SET(sockFd, &read_fds);

//    tv.tv_sec = 1;
//    tv.tv_usec = 0;

//    int select_ret = select(sockFd + 1, &read_fds, NULL, NULL, &tv);

//    if (select_ret == -1)
//    {
//       perror("select");
//       return 0;
//    }
//    else if (select_ret > 0)
//    {
//       // Data available, check if disconnect
//       int bytes_read = read_client(sockFd, tempBuffer);
//       if (bytes_read <= 0)
//       {
//          return 0;
//       }
//    }
//    return 1;
// }

static void handleGame(Client* client)
{
   client->player = create_player();
   printf("597\r\n");
   client->challengedBy->player = create_player();
   printf("599\r\n");
   Client* clientQuiJoue = client;
   printf("601\r\n");
   Game *game = new_game(client->player,client->challengedBy->player);
   printf("603\r\n");
   //char buffer[BUF_SIZE];
   printf("605\r\n");
   while(!is_game_over(game)){
      printf("607\r\n");
      //buffer[0] = 0;
      char * buffer = buffer_board(game);
      write_client(clientQuiJoue->sock, buffer);
      printf("609\r\n");
      if (clientQuiJoue == client){
         clientQuiJoue = client->challengedBy;
         printf("612\r\n");
      }
      else{
         clientQuiJoue = client;
         printf("616\r\n");
      }
      printf("%s\r\n", clientQuiJoue->name);
      printf("619\r\n");
      break;
   }
   printf("622\r\n");
}

// static void handleGame(Client *client)
// {
//    while (client->game != NULL)
//    {
//       Game *game = client->game;
//       char buffer[BUF_SIZE];
//       boolean quit = FALSE;
//       while (game != NULL && game->turn != NULL && game->turn == client->challengedBy->player)
//       {
//          char tempBuffer[BUF_SIZE];
//          tempBuffer[0] = 0;
//          char message[BUF_SIZE];
//          message[0] = 0;

//          // Checks for disconnect and absorbs all inputs while waiting for the other player
//          if (!check_socket(client->sock, tempBuffer))
//          {
//             quit = TRUE;
//             break;
//          }
//          else if (strlen(tempBuffer) > 0)
//          {
//             //send_chat_message_to_all_observers(client, tempBuffer, client->player == game->players[0] ? GREEN : PURPLE);
//          }
//       }
//       // Pointer to game may have changed in the other thread
//       // Checks if the game is still going
//       // Checks for disconnect
//       if ((game = client->game) == NULL || game->turn == NULL || quit)
//       {
//          break;
//       }

//       write_client(client->sock, "\nIt's your turn !\nType tie to tie\nType anything else to send it to your opponent and the observers\n");
//       char *nameP1 = client->player == game->players[0] ? client->name : client->challengedBy->name;
//       char *nameP2 = client->player == game->players[1] ? client->name : client->challengedBy->name;
//       construct_board(game, buffer, nameP1, nameP2);

//       //send_message_to_all_observers(game, buffer, NULL);

//       // Check if the game and the socket are still alive
//       if (client->game == NULL || read_client(client->sock, buffer) <= 0)
//       {
//          break;
//       }
//       else
//       {
//          game = client->game;
//          int caseNumber = atoi(buffer);
//          Pit pit;

//          // Check if the player wants to tie
//          if (strcmp(buffer, "tie") == 0)
//          {
//             tie(game);
//             write_client(client->sock, "\nYou asked to tie !\n" );
//             write_client(client->challengedBy->sock,  "\nYour opponent asked to tie !\n");
//             game->turn = get_opponent(game->turn, game);
//          }
//          // Check if conversion was successful
//          else if (caseNumber == 0)
//          {
//             //send_chat_message_to_all_observers(client, buffer, client->player == game->players[0] ? GREEN : PURPLE);
//          }
//          else if (get_pit(caseNumber, &pit) && is_valid_move(pit, game))
//          {
//             make_move(&(client->game), pit);

//             //update_game_of_all_observers(game, client->game);
//             game = client->game;

//             write_client(client->sock, "\nMove done !\nWait for your turn...\nAny message typed will be sent to your opponent and the observers.\n");
//          }
//          else
//          {
//             write_client(client->sock,"Invalid move\nPlease retry another one.\n");
//          }

//          // Game is over
//          if (game != NULL && is_game_over(game))
//          {
//             construct_board(game, buffer, nameP1, nameP2);

//             //send_message_to_all_observers(game, buffer, NULL);

//             Player *winner = get_winner(game);
//             if (winner != NULL)
//             {
//                char message[BUF_SIZE];
//                snprintf(message, BUF_SIZE, "\nPlayer %d won !\n", winner == game->players[0] ? 1 : 2);

//                //send_message_to_all_observers(game, message, NULL);

//                if (winner == client->player)
//                {
//                   write_client(client->sock,  "\nCongratulations, you won !\n" );
//                   write_client(client->challengedBy->sock,  "\nYou lost !\n" );
//                }
//                else
//                {
//                   write_client(client->sock, "\nYou lost !\n");
//                   write_client(client->challengedBy->sock, "\nCongratulations, you won !\n" );
//                }
//             }
//             else
//             {
//                //send_message_to_all_observers(game, "\nIt's a tie !\n" , NULL);
//             }

//             //update_game_of_all_observers(game, NULL);

//             // Free all the memory allocated for the game
//             free_player(game->players[0]);
//             free_player(game->players[1]);
//             free_game(game);

//             client->player = NULL;
//             client->challengedBy->player = NULL;
//             client->challengedBy->challengedBy = NULL;
//             client->challengedBy = NULL;
//             break;
//          }
//       }
//    }
// }

// char* buffer_board(Game *game)
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




int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}
