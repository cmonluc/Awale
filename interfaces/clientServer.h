#ifndef CLIENT_H
#define CLIENT_H

#define BUF_SIZE 1024

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
}Client;

#endif /* guard */
