//@File: client.h
//
// Definitions and prototypes for the module: client.h
//@

#ifndef CLIENT_H
#define CLIENT_H

#define CLIENT_NONE                0
#define CLIENT_ALLOCATED           1
#define WAITING_FOR_PEER           2
#define CLIENT_JUST_CONNECTED      3
#define CLIENT_REGISTERED          4

#define NETIO_WAIT     100000   // Timeout for network messages (microsecs)

#define MAX_BLOCK_ERRORS   2   // Block error count treshold

#define BUFID_SPEC   0
#define SLOT_SPEC    1

#define CLNT_INFO_LNG 256

#define MSG_RECEIVED  1
#define MSG_ROUTING   2
#define MSG_SENDING   3


#include "types.h"

Client *GetClientByName(char *name, int select);
void InitClient(Client *aClient);
int CleanClient(Client *aClient);
int FreeClient(Client *aClient);
int ClientTableInit(int);
void RemoveClient(int clid);
int ClientConnection(Client *cl);
int ConnectToPeer(char *ip);
int PeerConnection(Client *cl);
void TerminateAllClientThreads(void);


#endif
