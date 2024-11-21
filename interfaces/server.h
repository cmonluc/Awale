#include "clientServer.h"
#include "gamelogic.h"
#include <stdlib.h>

#ifndef SERVER_H
#define SERVER_H

#ifdef WIN32

#include <winsock2.h>

#elif defined (linux)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#else

#error not defined for this platform

#endif

#define CRLF        "\r\n"
#define PORT         1977

#define BUF_SIZE    1024
#define AVAILABLE_BUF_SIZE BUF_SIZE-strlen(buffer)-1

#define MAX_PSEUDO_ATTEMPTS 5



static void init(void);
static void end(void);
static void app(void);
static int init_connection(void);
static void end_connection(int sock);
static int read_client(SOCKET sock, char *buffer);
static void write_client(SOCKET sock, const char *buffer);
static Client *getClientByName(const char *name);
static void send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);
static void remove_client(Client *clients, int to_remove, int *actual);
static void clear_clients(Client *clients, int actual);
static void list_clients(char *buffer, Client client);
static void menu(Client *client);
static int add_client(Client *client);
static int check_socket(int sockFd, char *tempBuffer);
static int challengeClient(Client *challenger);
static int handleGame(Client *client);
void sendBoardToClient(Game* game, Client *client);
void* handleClient(void * indexInClient);
static int handleMenu(Client * client);
static void sendMessageToAllObservers(Game *game, const char *buffer, Client *except);
static void sendChatToAllObservers(Client *client, const char *buffer, const char *color);
static int handleObserver(Client *client);
static int listClientsInGame(char *buffer, Client *client);
static void updateGameForObservers(Game *oldGame, Game *newGame);
static void sendBoardToAllObservers(Game *game);
static void clear_client(Client *client);
static void handleFriends(Client *client);
static void list_friends(Client *client);
static int canObserve(Client *client, Client *observed);
static void remove_friend(Client *client);

#endif /* guard */
