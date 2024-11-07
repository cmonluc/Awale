#ifndef CLIENT_H
#define CLIENT_H

#include "server.h"

typedef struct
{
   SOCKET sock;
   char name[BUF_SIZE];
   int game;
   int player;
   int challengedBy;
}Client;

#endif /* guard */
