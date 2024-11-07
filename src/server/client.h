#ifndef CLIENT_H
#define CLIENT_H

#include "server.h"
#include "gamelogic.h"

typedef struct _Client
{
   SOCKET sock;
   char name[BUF_SIZE];
   Game *game;
   int player;
   struct _Client *challengedBy;
}Client;

#endif /* guard */
