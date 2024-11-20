#ifndef CLIENT_H
#define CLIENT_H

#define BUF_SIZE 1024
#define MAX_CLIENTS 100
#define MAX_FRIENDS MAX_CLIENTS-1

//#include "server.h"
#include "gamelogic.h"

typedef int SOCKET;
typedef int boolean;

typedef struct _Client
{
   SOCKET sock;
   char name[BUF_SIZE];
   Game *game;
   Player *player;
   struct _Client *challengedBy;
   char bio[BUF_SIZE];
   int index;
   Game *observe;
   struct _Client *friends[MAX_FRIENDS]
}Client;

#endif /* guard */
