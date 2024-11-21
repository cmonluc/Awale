#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

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
Game *games[MAX_CLIENTS];
pthread_t threads[MAX_CLIENTS];
int actual = 0;

void sendBoardToClient(Game *game, Client* client)
{
    char buffer[BUF_SIZE];
    buffer[0] = 0;
    // Add the initial lines
    strncat(buffer, "\n\n\n", BUF_SIZE - strlen(buffer) - 1);
    write_client(client->sock, buffer);
    buffer[0] = 0;
    strncat(buffer, "Direction " RED "--->\n" RESET, BUF_SIZE - strlen(buffer) - 1);
    write_client(client->sock, buffer);
    buffer[0] = 0;
    strncat(buffer, "Case    |  1   2   3   4   5   6  | Scores\n", BUF_SIZE - strlen(buffer) - 1);
    write_client(client->sock, buffer);
    buffer[0] = 0;
    strncat(buffer, "---------------------------------------------------\n", BUF_SIZE - strlen(buffer) - 1);
    write_client(client->sock, buffer);
    buffer[0] = 0;

    // Add the board for Player 1
    char player1_row[100]; // Temporary buffer for player 1's row
    snprintf(player1_row, sizeof(player1_row),
             GREEN "%s P1  | %2d  %2d  %2d  %2d  %2d  %2d  | %d\n" RESET,
             game->turn == game->players[0] ? RED "-->" GREEN : "   ",
             game->board[0][0], game->board[0][1], game->board[0][2],
             game->board[0][3], game->board[0][4], game->board[0][5],
             game->players[0]->score);
    strncat(buffer, player1_row, BUF_SIZE - strlen(buffer) - 1);
    write_client(client->sock, buffer);
    buffer[0] = 0;
    strncat(buffer, "---------------------------------------------------\n", BUF_SIZE - strlen(buffer) - 1);
    write_client(client->sock, buffer);
    buffer[0] = 0;

    // Add the board for Player 2
    char player2_row[100]; // Temporary buffer for player 2's row
    snprintf(player2_row, sizeof(player2_row),
             PURPLE "%s P2  | %2d  %2d  %2d  %2d  %2d  %2d  | %d\n" RESET,
             game->turn == game->players[1] ? RED "-->" PURPLE : "   ",
             game->board[1][0], game->board[1][1], game->board[1][2],
             game->board[1][3], game->board[1][4], game->board[1][5],
             game->players[1]->score);
    strncat(buffer, player2_row, BUF_SIZE - strlen(buffer) - 1);
    write_client(client->sock, buffer);
    buffer[0] = 0;

    strncat(buffer, "---------------------------------------------------\n", BUF_SIZE - strlen(buffer) - 1);
    write_client(client->sock, buffer);
    buffer[0]=0;

    // Add the row of numbers at the bottom
    strncat(buffer, "          12  11  10   9   8   7\n", BUF_SIZE - strlen(buffer) - 1);
    write_client(client->sock, buffer);
    buffer[0] = 0;  // Initialize buffer to be empty
    strncat(buffer, "\n\n\n", BUF_SIZE - strlen(buffer) - 1);
    write_client(client->sock, buffer);

}

static void app(void)
{
   SOCKET sock = init_connection();

   char buffer[BUF_SIZE];
   /* the index for the array */
   // int actual = 0;
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
         if (actual < MAX_CLIENTS){
            Client *client = (Client*)malloc(sizeof(Client));
            client->sock = csock;
            client->game=NULL;
            client->observe = NULL;
            client->player=NULL;
            client->challengedBy=NULL;
            client->bio[0] = '0';
            client->isPrivate=0;
            client->saved[0]=0;
            strncpy(client->name, buffer, BUF_SIZE-1);
            int index = add_client(client);
            if (index != -1){
               client->index = index;
               pthread_t thread;
               int threadResult = pthread_create(&thread, NULL, handleClient,&index);
               if (threadResult!=0){
                  printf("Error creating thread for the client.\n");
                  exit(-1);
               }
               threads[index]=thread;
               actual++;
               buffer[0]=0;
               strncat(buffer, "Welcome on Awale, ", BUF_SIZE-strlen(buffer)-1);
               strncat(buffer, client->name, BUF_SIZE-strlen(buffer)-1);
               strncat(buffer, " !", BUF_SIZE-strlen(buffer)-1);
               //snprintf(buffer, "Welcome on Awale, %s !\n", client->name);
               write_client(client->sock, buffer);
            }
         }
         else{
            write_client(csock, "The server is full, please try again later.\r\n");
         }
         
      }
   }

   for (int i = 0; i < MAX_CLIENTS; i++)
   {
      pthread_kill(threads[i], 0);
   }
   clear_clients(clients, actual);
   end_connection(sock);
}

static int handleMenu(Client *client){
   char buffer[BUF_SIZE];
   if (client->challengedBy == NULL){
      menu(client);
   }
   
   int c = read_client(client->sock,buffer);
   if (c<0){
      return -1;
   }
   else if(c == 0)
   {
      closesocket(client->sock);
      remove_client(clients, client->index, &actual);
      strncpy(buffer, client->name, BUF_SIZE - 1);
      strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
      send_message_to_all_clients(clients, *client, actual, buffer, 1);
   }
   else{
      if (strcmp(buffer, "0")==0){
         return -1;
      }
      else if (strcmp(buffer,"1")==0){
         list_clients(buffer, *client);
         write_client(client->sock, "To see more information about a player, type their username. Type anything to go back to the menu.\n\n");
         read_client(client->sock, buffer);
         Client* searched = getClientByName(buffer);
         if (searched != NULL){
            write_client(client->sock, PURPLE"\nUsername : "RESET);
            write_client(client->sock, searched->name);
            write_client(client->sock, PURPLE"\nBio : "RESET);
            write_client(client->sock, searched->bio);
            write_client(client->sock, "\n\n\n");
         }
      }
      else if (strcmp(buffer,"2")==0){
         challengeClient(client);
      }
      else if (strcmp(buffer,"3")==0){
         if(client->bio[0] == '0'){
            write_client(client->sock, "\nYour biography is empty.");
         }
         else{
            write_client(client->sock, "\nHere is your biography : \n\n"BLUE);
            write_client(client->sock, client->bio);
         }
         write_client(client->sock, RESET "\n\nDo you want to modify your biography ?\nType yes to modify, no to go back to the menu.\n");
         read_client(client->sock, buffer);
         if(strcmp(buffer, "yes")==0){
            write_client(client->sock, "\nPlease write below your new biography. (1000 characters max)\n");
            read_client(client->sock, buffer);
            strncpy(client->bio, buffer, BUF_SIZE-1);
         }
      }
      else if (strcmp(buffer,"4")==0){
         handleObserver(client);
      }
      else if (strcmp(buffer,"5")==0){
         list_clients(buffer,*client);
         write_client(client->sock, "\n\n");
         write_client(client->sock, "To send a message to a client, type their username !\n");
         read_client(client->sock, buffer);
         Client* messaged = getClientByName(buffer);
         if (messaged == NULL){
            write_client(client->sock, "\nNot a client. Going back to the menu\n");
         }
         else{
            write_client(client->sock, "\nType in a message to send to "BLUE);
            write_client(client->sock, messaged->name);
            write_client(client->sock, RESET"\n");
            read_client(client->sock,buffer);
             write_client(client->sock, "\n");
            write_client(client->sock, GREEN"Message sent : ");
            write_client(client->sock, buffer);
            write_client(client->sock, RESET" \n\n\n");
            write_client(messaged->sock, GREEN"Message received from ");
            write_client(messaged->sock, client->name);
            write_client(client->sock, buffer);
            write_client(messaged->sock, buffer);
            write_client(messaged->sock, RESET" \n\n\n");
            write_client(messaged->sock, "To answer, go to the menu and type 5");
            write_client(messaged->sock, " \n\n\n");
         }
      }
      else if(strcmp(buffer, "6")==0){
         handleFriends(client);
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
            games[client->index]=game;
            games[client->challengedBy->index]=game;
            client->game = game;
            client->observe = game;
            client->challengedBy->game = game;
            client->challengedBy->observe = game;
            client->player = p1;
            client->challengedBy->player = p2;

            char buffer[BUF_SIZE];
            snprintf(buffer, BUF_SIZE, "\nGame created between P1 : %s and P2 : %s !\n", client->name, client->challengedBy->name);
            write_client(client->sock, buffer);
            write_client(client->challengedBy->sock, buffer);
         }
      }
      else
        {
            write_client(client->sock, RED "\nInvalid input! Please enter a valid option.\n" RESET);
        }
   }

}

void *handleClient(void *indexInClients)
{
   char buffer[BUF_SIZE];
   buffer[0]=0;
   int index = *(int *)indexInClients;
   Client *client = clients[index];

   fd_set rdfs;
   FD_ZERO(&rdfs);
   FD_SET(client->sock, &rdfs);

   while (1)
   {
      int disc = handleGame(client);
      printf("%d\n\n",disc);
      fflush(stdout);
      if (disc){
         printf(client->name);
         printf("here\n");
         fflush(stdout);
         remove_client(client->challengedBy,client->challengedBy->index,actual);
         printf("here2\n");
         fflush(stdout);
         remove_friend(client->challengedBy);
         printf("here3\n");
         fflush(stdout);
         clear_client(client->challengedBy);

      }
      //handleGame(client);
      if (FD_ISSET(client->sock, &rdfs))
      {
         printf(client->name);
         printf("hey\n");
         if (handleMenu(client) == -1)
         {
            break;
         }
      }
   }

   strncat(buffer, "Client ", BUF_SIZE-strlen(buffer)-1);
   strncat(buffer, client->name, BUF_SIZE-strlen(buffer)-1);
   strncat(buffer, " disconnected !\n", BUF_SIZE-strlen(buffer)-1);
   printf(buffer);
   fflush(stdout);
   //send_message_to_all_clients(clients, *client, actual, buffer, 1);
   remove_friend(client);
   remove_client(clients,index,&actual);
   clear_client(client); 
   return NULL;
}

static void remove_friend(Client *client){
   for(int i=0 ; i<MAX_CLIENTS ; i++){
      if(clients[i]!=NULL){
         if(strcmp(clients[i]->name, client->name)!=0){
            for(int j=0 ; j<MAX_FRIENDS ; j++){
               if(clients[i]->friends[j]!=NULL){
                  if(strcmp(clients[i]->friends[j]->name, client->name)==0){
                     clients[i]->friends[j]=NULL;
                  }
               }
            }
         }  
      }
   }
}

static void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

static void clear_client(Client *client)
{
   if (client == NULL) {
      return;
   }
   if (client->game != NULL){
      printf("client a un game en cours\n\n");
      fflush(stdout);
      free_player(client->game->players[0]);
      free_player(client->game->players[0]);
      free_game(client->game);
      client->challengedBy->game=NULL;
      write_client(client->challengedBy->sock, "\n\nYour opponent has disconnected.\n\n");
      client->challengedBy->challengedBy =NULL;
      printf("client supp askip\n\n");
      fflush(stdout);
   }
   closesocket(client->sock);
   free(client);
   //clients[client->index] = NULL;
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
   //  write_client(newClient->sock, "The server is full, please try again later.\r\n");
    return -1;
}

static void menu(Client *client)
{
    char buffer[BUF_SIZE];
    buffer[0] = 0;
    strncat(buffer, "\r\nHere is the menu, choose a number : \r\n", BUF_SIZE-strlen(buffer)-1);
    strncat(buffer, "\t1 - View all online players\r\n", BUF_SIZE-strlen(buffer)-1);
    strncat(buffer, "\t2 - Challenge a player\r\n", BUF_SIZE-strlen(buffer)-1);
    strncat(buffer, "\t3 - Modify your bio\r\n", BUF_SIZE-strlen(buffer)-1);
    strncat(buffer, "\t4 - Observe a game\r\n", BUF_SIZE-strlen(buffer)-1);
    strncat(buffer, "\t5 - Chat with other users\r\n", BUF_SIZE-strlen(buffer)-1);
    strncat(buffer, "\t6 - Add, remove or list your friends\r\n", BUF_SIZE-strlen(buffer)-1);
    strncat(buffer, "\t0 - quit the server\r\n", BUF_SIZE-strlen(buffer)-1);
    write_client(client->sock, buffer);
}

static void list_clients(char *buffer, Client client)
{
    buffer[0] = 0;
    strncat(buffer, "\r\nList of clients : \r\n", BUF_SIZE-strlen(buffer)-1);
    for (int i=0 ; i<MAX_CLIENTS; i++)
    {
        if (clients[i] != NULL)
        {
            strncat(buffer, " - ", BUF_SIZE-strlen(buffer)-1);
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

      int private = 0;
      write_client(challenger->sock, "\nDo you want to make a private or public challenge ?\n");
      write_client(challenger->sock, "Type 'private' to do a private challenge, 'public' to do a public one, 'cancel' to cancel");
      while(1){
         buffer[0]=0;
         if(read_client(challenger->sock, buffer) <= 0){
            return -1;
         }
         else if(strcmp(buffer, "cancel")==0){
            return EXIT_SUCCESS;
         }
         else if (strcmp(buffer, "private")==0){
            private=1;
            break;
         }
         else if(strcmp(buffer, "public")==0){
            private=0;
            break;
         }
         else{
            write_client(challenger->sock, "Invalid input ! Type 'private', 'public' or 'cancel'\n");
         }
      }
   

      write_client(challenger->sock, "\nWho do you want to challenge ?\ncancel to cancel\n");
      list_clients_not_in_game(buffer, challenger);
      write_client(challenger->sock, buffer);

      buffer[0] = 0;
      if (read_client(challenger->sock, buffer) <= 0)
         return -1;
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
         challengee->isPrivate = private;
         challenger->isPrivate = private;

         message[0]=0;
         strncat(message, challenger->name, BUF_SIZE-strlen(message)-1);
         strncat(message, " wants to challenge you in a ", BUF_SIZE-strlen(message)-1);
         if (private){
            strncat(message, "private ", BUF_SIZE-strlen(message)-1);
         }
         else{
            strncat(message, "public ", BUF_SIZE-strlen(message)-1);
         }
         strncat(message, "game !\n", BUF_SIZE-strlen(message)-1);

         write_client(challengee->sock, message);
         snprintf(message, BUF_SIZE, "Type accept to accept the challenge !");
         write_client(challengee->sock, message);

         snprintf(message, BUF_SIZE, "Waiting for %s's response...\n", challengee->name);
         write_client(challenger->sock, message);
         break;
      }
   }
   // Wait for the challengee to accept or refuse
   while (challenger->challengedBy != NULL)
   {
      char tempBuffer[BUF_SIZE];
      // Check for disconnect while waiting
      if (!check_socket(challenger->sock, tempBuffer))
      {
         return -1;
      }
      // If a game is created, it means the other accepted it
      else if (challenger->game != NULL)
      {
         strncat(challenger->saved, "Game between ",2048-strlen(challenger->saved)-1);
         strncat(challenger->saved, challenger->name,2048-strlen(challenger->saved)-1);
         strncat(challenger->saved, " and ",2048-strlen(challenger->saved)-1);
         strncat(challenger->saved, challenger->challengedBy->name,2048-strlen(challenger->saved)-1);
         strncat(challenger->saved, "\r\n",2048-strlen(challenger->saved)-1);
         break;
      }
   }
   return EXIT_SUCCESS;
}

static int check_socket(int sockFd, char *tempBuffer)
{
   fd_set read_fds;
   struct timeval tv;
   FD_ZERO(&read_fds);
   FD_SET(sockFd, &read_fds);

   tv.tv_sec = 1;
   tv.tv_usec = 0;
   fflush(stdout);

   int select_ret = select(sockFd + 1, &read_fds, NULL, NULL, &tv);

   if (select_ret == -1)
   {
      perror("select");
      return 0;
   }
   else if (select_ret > 0)
   {
      // Data available, check if disconnect
      int bytes_read = read_client(sockFd, tempBuffer);
      if (bytes_read <= 0)
      {
         return 0;
      }
   }
   return 1;
}



static int handleGame(Client *client)
{
   
   int disconnexion = 0;
   while (client->game != NULL)
   {
      Game *game = client->game;
      char buffer[BUF_SIZE];
      while (game != NULL && game->turn != NULL && game->turn == client->challengedBy->player)
      {
         char tempBuffer[BUF_SIZE];
         tempBuffer[0] = 0;
         char message[BUF_SIZE];
         message[0] = 0;

         // Checks for disconnect and absorbs all inputs while waiting for the other player
         int checkSocket = check_socket(client->sock, tempBuffer);
         printf("checkSocket : ");
         printf("%d\n\n",checkSocket);
         if (!checkSocket)
         {
            disconnexion = 1;
            break;
         }
         else if (strlen(tempBuffer) > 0)
         {
            sendChatToAllObservers(client, tempBuffer, client->player == game->players[0] ? GREEN : PURPLE);
         }
      }
      // Pointer to game may have changed in the other thread
      // Checks if the game is still going
      // Checks for disconnect
      if ((game = client->game) == NULL || game->turn == NULL || disconnexion)
      {
         return 1;
         //break;
      }

      // write_client(client->sock, "\n" RED "It's your turn !\n" RESET "Type " PURPLE "tie" RESET " to tie\n" BLUE "Type anything else to send it to your opponent and the observers\n" RESET);
      char *nameP1 = client->player == game->players[0] ? client->name : client->challengedBy->name;
      char *nameP2 = client->player == game->players[1] ? client->name : client->challengedBy->name;
      //construct_board(game, buffer, nameP1, nameP2);
      //sendBoardToClient(game,client);
      sendBoardToAllObservers(game);
      write_client(client->sock, "\n" RED "It's your turn !" RESET );
      if (client->player == client->game->players[0]){
         write_client(client->sock, " Type a number between 1 and 6\n" );
      }
      else{
         write_client(client->sock, " Type a number between 7 and 12\n" );
      }
      write_client(client->sock, "Type " PURPLE "tie" RESET " to tie\n" BLUE "Type anything else to send it to your opponent and the observers\n" RESET);
      

      // Check if the game and the socket are still alive
      if (client->game == NULL || read_client(client->sock, buffer) <= 0)
      {
         break;
      }
      else
      {
         game = client->game;
         int caseNumber = atoi(buffer);
         Pit pit;

         // Check if the player wants to tie
         if (strcmp(buffer, "tie") == 0)
         {
            tie(game);
            write_client(client->sock, GREEN "\nYou asked to tie !\n" RESET);
            write_client(client->challengedBy->sock, GREEN "\nYour opponent asked to tie !\n" RESET);
            game->turn = get_opponent(game->turn, game);
         }
         // Check if conversion was successful
         else if (caseNumber == 0)
         {
            sendChatToAllObservers(client, buffer, client->player == game->players[0] ? GREEN : PURPLE);
         }
         else if (get_pit(caseNumber, &pit) && is_valid_move(pit, game))
         {
            make_move(&(client->game), pit);

            updateGameForObservers(game, client->game);
            game = client->game;

            write_client(client->sock, "\nMove done !\nWait for your turn...\nAny message " BLUE "typed will be sent to your opponent" RESET " and the observers.\n");
            char message[BUF_SIZE];
            message[0]=0;
            strncat(message,"\n",BUF_SIZE - strlen(message)-1);
            strncat(message,client->name,BUF_SIZE - strlen(message)-1);
            strncat(message," has selected the pit : ",BUF_SIZE - strlen(message)-1);
            strncat(message, buffer ,BUF_SIZE - strlen(message)-1);
            sendMessageToAllObservers(game,message, client);
            strncat(client->saved, message,2048-strlen(client->saved)-1);
            strncat(client->challengedBy->saved, message,2048-strlen(client->challengedBy->saved)-1);
         }
         else
         {
            write_client(client->sock, RED "Invalid move\nPlease retry another one.\n" RESET);
         }

         // Game is over
         if (game != NULL && is_game_over(game))
         {
            //construct_board(game, buffer, nameP1, nameP2);
            sendBoardToClient(game,client);

            sendMessageToAllObservers(game, buffer, NULL);

            Player *winner = get_winner(game);
            if (winner != NULL)
            {
               char message[BUF_SIZE];
               snprintf(message, BUF_SIZE, BLUE "\nPlayer %d won !\n" RESET, winner == game->players[0] ? 1 : 2);

               sendMessageToAllObservers(game, message, NULL);

               if (winner == client->player)
               {
                  write_client(client->sock, GREEN "\nCongratulations, you won !\n" RESET);
                  write_client(client->challengedBy->sock, RED "\nYou lost...\n" RESET);
                  strncat(client->saved,"\n\nCongratulations, you won !\n",2048-strlen(client->saved)-1);
                  strncat(client->challengedBy->saved,"\n\nYou lost...\n",2048-strlen(client->challengedBy->saved)-1);
               }
               else
               {
                  write_client(client->sock, RED "\n\nYou lost...\n" RESET);
                  write_client(client->challengedBy->sock, GREEN "\r\nCongratulations, you won !\n" RESET);
                  strncat(client->challengedBy->saved,"\n\nCongratulations, you won !\n",2048-strlen(client->challengedBy->saved)-1);
                  strncat(client->saved,"\n\nYou lost...\n",2048-strlen(client->saved)-1);
               }
               
            }
            else
            {
               sendMessageToAllObservers(game, BLUE "\nIt's a tie !\n" RESET, NULL);
               strncat(client->saved,"\n\nIt's a tie !\n",2048-strlen(client->saved)-1);
               strncat(client->challengedBy->saved,"\n\nIt's a tie !\n",2048-strlen(client->challengedBy->saved)-1);
            }

            updateGameForObservers(game, NULL);

            write_client(client->sock, "\nDo you want to save your game ?\nType 'yes' or 'no'\n");
            read_client(client->sock, buffer);
            if(strcmp(buffer, "yes")==0){
               FILE *file = fopen("./savedGames/game.txt", "w+");

               if(file!=NULL){
                  //fwrite(saved,1,sizeof(saved),file);
                  fprintf(file, client->saved);
                  fclose(file);
               }
            }
            else{}

            write_client(client->challengedBy->sock, "\nDo you want to save your game ?\nType 'yes' or 'no'\n");
            read_client(client->challengedBy->sock, buffer);
            if(strcmp(buffer, "yes")==0){
               FILE *file = fopen("./savedGames/game.txt", "w+");
               if(file!=NULL){
                  fprintf(file, client->saved);
                  fclose(file);
                  menu(client->challengedBy);
               }
            }
            else{
               menu(client->challengedBy);
            }

            // Free all the memory allocated for the game
            free_player(game->players[0]);
            free_player(game->players[1]);
            free_game(game);

            client->player = NULL;
            client->challengedBy->player = NULL;
            client->challengedBy->challengedBy = NULL;
            client->challengedBy = NULL;
           
            break;
         }
         
      }
   }
   return disconnexion;
}

static int listClientsInGame(char *buffer, Client *client)
{
   buffer[0] = 0;
   int gameFound = 0;
   strncat(buffer, "\nList of games:\n", BUF_SIZE - strlen(buffer) - 1);
   for (int i = 0; i < MAX_CLIENTS; i++)
   {
      if (clients[i] != NULL && clients[i]->game != NULL && clients[i]->player == clients[i]->game->players[0])
      {
         if (!clients[i]->isPrivate || canObserve(client, clients[i]))
         {
            gameFound = 1;
            strncat(buffer, "\t- ", BUF_SIZE - strlen(buffer) - 1);
            strncat(buffer, clients[i]->name, BUF_SIZE - strlen(buffer) - 1);
            strncat(buffer, " (P1)" RED " VS " RESET, BUF_SIZE - strlen(buffer) - 1);
            strncat(buffer, clients[i]->challengedBy->name, BUF_SIZE - strlen(buffer) - 1);
            strncat(buffer, " (P2)\n", BUF_SIZE - strlen(buffer) - 1);
         }
      }
   }
   if (!gameFound)
   {
      strncat(buffer, RED "No game available\n" RESET, BUF_SIZE - strlen(buffer) - 1);
   }
   return gameFound;
}

static int handleObserver(Client *client)
{
   char buffer[BUF_SIZE];
   while (1)
   {
      if (!listClientsInGame(buffer, client))
      {
         write_client(client->sock, "\nThere is no game available to observe !\n");
         break;
      }
      write_client(client->sock, "\nWhose game do you want to observe ?\n" RED "cancel" RESET " to cancel\n");
      write_client(client->sock, buffer);

      buffer[0] = 0;
      if (read_client(client->sock, buffer) <= 0)
         return -1;
      if (strcmp(buffer, "cancel") == 0)
      {
         break;
      }
      Client *observed = getClientByName(buffer);

      char message[BUF_SIZE];
      if (observed == NULL)
      {
         snprintf(message, BUF_SIZE, "\nPlayer " RED "%s" RESET " not found !\n", buffer);
         write_client(client->sock, message);
      }
      else if (observed->game == NULL)
      {
         snprintf(message, BUF_SIZE, "\nPlayer " RED "%s" RESET " is not playing !\n", buffer);
         write_client(client->sock, message);
      }
      else
      {

         if(observed->isPrivate==1){
            if (!canObserve(client, observed)){
               snprintf(message, BUF_SIZE, "\nYou can't observe %s's game.\n", buffer);
               write_client(client->sock, message);
               break;
            }
         }

         client->observe = observed->game;
         snprintf(message, BUF_SIZE, "\nYou are now observing %s's game !\nType " RED "quit" RESET " to quit", observed->name);
         write_client(client->sock, message);

         while (client->observe != NULL)
         {
            // Check for disconnect while waiting
            char tempBuffer[BUF_SIZE];
            tempBuffer[0] = 0;
            if (!check_socket(client->sock, tempBuffer))
            {
               client->observe = NULL;
               return -1;
            }
            else if (strcmp(tempBuffer, "quit") == 0)
            {
               client->observe = NULL;
               return EXIT_SUCCESS;
            }
            else if (strlen(tempBuffer) > 0)
            {
               sendChatToAllObservers(client, tempBuffer, BLUE);
            }
         }
      }
   }
   return EXIT_SUCCESS;
}

static void updateGameForObservers(Game *oldGame, Game *newGame)
{
   for (int i = 0; i < MAX_CLIENTS; i++)
   {
      if (clients[i] != NULL && clients[i]->game == oldGame)
      {
         clients[i]->game = newGame;
      }
   }
}

static void sendBoardToAllObservers(Game *game){
   for (int i = 0; i < MAX_CLIENTS; i++)
   {
      if (clients[i] != NULL && clients[i]->observe == game)
      {
         sendBoardToClient(game, clients[i]);
      }
   }
}

static void sendChatToAllObservers(Client *client, const char *buffer, const char *color)
{
   char message[BUF_SIZE];
   message[0] = 0;
   strncat(message, "\n", BUF_SIZE - strlen(message) - 1);
   strncat(message, color, BUF_SIZE - strlen(message) - 1);
   strncat(message, client->name, BUF_SIZE - strlen(message) - 1);
   strncat(message, " : ", BUF_SIZE - strlen(message) - 1);
   strncat(message, buffer, BUF_SIZE - strlen(message) - 1);
   strncat(message, "\n", BUF_SIZE - strlen(message) - 1);
   strncat(message, RESET, BUF_SIZE - strlen(message) - 1);
   sendMessageToAllObservers(client->observe, message, NULL);
}

static void sendMessageToAllObservers(Game *game, const char *buffer, Client *except)
{
   for (int i = 0; i < MAX_CLIENTS; i++)
   {
      if (clients[i] != NULL && clients[i]->observe == game && clients[i] != except)
      {
         write_client(clients[i]->sock, buffer);
      }
   }
}

static void handleFriends(Client *client){
   char buffer[BUF_SIZE];
   while(1){
      write_client(client->sock, "\n\nWhat do you want to do ?\n");
      write_client(client->sock, "\t1 - add a friend\n");
      write_client(client->sock, "\t2 - remove a friend\n");
      write_client(client->sock, "\t3 - list your friends\n");
      write_client(client->sock, "\t0 - go back to the menu\n");
      read_client(client->sock, buffer);

      if(strcmp(buffer, "1")==0){
         list_clients(buffer, *client);
         write_client(client->sock, "Type the name of the user to add him to your friends\n");
         read_client(client->sock, buffer);
         Client* searched = getClientByName(buffer);
         if (searched != NULL){
            for(int i=0; i<MAX_FRIENDS; i++){
               if(client->friends[i]==NULL){
                  client->friends[i]=searched;
                  break;
               }
            }
            write_client(client->sock, "Friend added !");
            buffer[0]=0;
            strncat(buffer, client->name, BUF_SIZE-strlen(buffer)-1);
            strncat(buffer, " added you to his friends !\n", BUF_SIZE-strlen(buffer)-1);
            write_client(searched->sock, buffer);
         }
         else{
            write_client(client->sock, "Invalid username\n");
         }

      }
      else if(strcmp(buffer, "2")==0){
         list_friends(client);
         write_client(client->sock, "Type the name of the friend you want to remove\n");
         read_client(client->sock, buffer);
         Client* searched = getClientByName(buffer);
         int found = 0;
         if(searched!=NULL){
            for(int i=0; i<MAX_FRIENDS; i++){
               if(strcmp(client->friends[i]->name, searched->name)==0){
                  client->friends[i]=NULL;
                  found=1;
                  write_client(client->sock, "User removed from your friends!\n");
                  break;
               }
            }
            if(!found){
               write_client(client->sock, "Username not found in your friends\n");
            }
         }
         else{
            write_client(client->sock, "This user doesn't exist\n");
         }
         
      }
      else if(strcmp(buffer, "3")==0){
         list_friends(client);
      }
      else if(strcmp(buffer, "0")==0){
         break;
      }
   }
}

static void list_friends(Client *client){
   char buffer[BUF_SIZE];
   buffer[0] = 0;
    strncat(buffer, "List of your friends : \r\n", BUF_SIZE-strlen(buffer)-1);
    for (int i=0 ; i<MAX_FRIENDS; i++)
    {
        if (client->friends[i] != NULL)
        {
            strncat(buffer, "    - ", BUF_SIZE-strlen(buffer)-1);
            strncat(buffer, client->friends[i]->name, BUF_SIZE-strlen(buffer)-1);
            strncat(buffer, "\r\n", BUF_SIZE-strlen(buffer)-1);
        }
    }
    write_client(client->sock, buffer);

}

static int canObserve(Client *client, Client *observed){
   int friend=0;
   if (observed->isPrivate)
   for(int i=0; i<MAX_FRIENDS; i++){
      if (observed->friends[i]!=NULL){
         if(strcmp(observed->friends[i]->name, client->name) == 0){
            friend =1 ;
         }
      }
   }

   return friend;
}


int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}
