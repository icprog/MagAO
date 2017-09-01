//@File:  types.h
//
// Definitions of data types for MsgD-RTDB
//@

#ifndef TYPES_H
#define TYPES_H

// Uncomment to add debug code for Peer active connection
// #define PEER_DEBUG

// Uncomment to add debug code for malloc() in RTDB
// #define MEM_DEBUG

// Uncomment following line to help in debugging mutex deadlocks
// (writes a lot of mutex info to stdout)
//#define MUTEX_DEBUG

// Uncomment following line to compile in more Mutex error check
#define MUTEX_ERROR


#include <stdio.h> 
#include <stddef.h>      // For size_t
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/time.h>
#include <time.h>


#define LOCKMUTEX     1
#define DONTLOCKMUTEX 0

#ifdef MEM_DEBUG
#define MEM_DEBUG_INFO "MEM_DEBUG"
#else
#define MEM_DEBUG_INFO ""
#endif

#ifdef MUTEX_ERROR
#define MUTEX_ERROR_INFO "MUTEX_ERROR"
#else
#define MUTEX_ERROR_INFO ""
#endif

#ifdef MUTEX_DEBUG

#define MUTEX_DEBUG_INFO "MUTEX_DEBUG"

#define MUTEX_LOCK(mutexpt,where) {               \
   int    mstat;                                  \
   struct timeval now;                            \
   gettimeofday(&now, NULL);                      \
   fprintf(stderr,"MTLOCK %ld.%6.6d %s %lx by %lx\n",(long)now.tv_sec,(int)now.tv_usec,where,(unsigned long)mutexpt,(unsigned long)pthread_self());  \
   if((mstat=pthread_mutex_lock(mutexpt))!=0) {  \
       gettimeofday(&now, NULL);                      \
       fprintf(stderr,"LOCKERR %ld.%6.6d %s %lx by %lx err: %s\n",(long)now.tv_sec,(int)now.tv_usec,where,(unsigned long)mutexpt,(unsigned long)pthread_self(),strerror(mstat));    \
   } else {                                       \
       gettimeofday(&now, NULL);                      \
       fprintf(stderr,"LOCKOK %ld.%6.6d %s %lx by %lx\n",(long)now.tv_sec,(int)now.tv_usec,where,(unsigned long)mutexpt,(unsigned long)pthread_self());  \
   } }

#define MUTEX_UNLOCK(mutexpt,where) {               \
   int mstat;                                     \
   struct timeval now;                            \
   gettimeofday(&now, NULL);                      \
   fprintf(stderr,"MTUNLK %ld.%6.6d %s %lx by %lx\n",(long)now.tv_sec,(int)now.tv_usec,where,(unsigned long)mutexpt,(unsigned long)pthread_self());  \
   if((mstat=pthread_mutex_unlock(mutexpt))!=0) {  \
       fprintf(stderr,"UNLKERR %ld.%6.6d %s %lx by %lx err: %s\n",(long)now.tv_sec,(int)now.tv_usec,where,(unsigned long)mutexpt,(unsigned long)pthread_self(),strerror(mstat));    \
   } else {                                       \
       fprintf(stderr,"UNLKOK %ld.%6.6d %s %lx by %lx\n",(long)now.tv_sec,(int)now.tv_usec,where,(unsigned long)mutexpt,(unsigned long)pthread_self());  \
   } }

#define MUTEX_INIT(mutexpt,mutexname) {          \
   int mstat;                                     \
   struct timeval now;                            \
   gettimeofday(&now, NULL);                      \
    if((mstat=pthread_mutex_init(mutexpt,&mutexAttributes))!=0) {    \
         fprintf(stderr,"INITERR %ld.%6.6d %s %lx err: %s\n",(long)now.tv_sec,(int)now.tv_usec,mutexname,(unsigned long)mutexpt,strerror(mstat));    \
   } else {                                       \
       fprintf(stderr,"MTINIT %ld.%6.6d %s %lx\n",(long)now.tv_sec,(int)now.tv_usec,mutexname,(unsigned long)mutexpt);  \
   } }

#define MUTEX_DESTROY(mutexpt,mutexname) {         \
   int mstat;                                     \
   struct timeval now;                            \
   gettimeofday(&now, NULL);                      \
   pthread_mutex_lock(mutexpt);                   \
   pthread_mutex_unlock(mutexpt);                 \
    if((mstat=pthread_mutex_destroy(mutexpt))!=0) {    \
         fprintf(stderr,"DSTRERR %ld.%6.6d %s %lx err: %s\n",(long)now.tv_sec,(int)now.tv_usec,mutexname,(unsigned long)mutexpt,strerror(mstat));    \
   } else {                                       \
       fprintf(stderr,"MTDSTR %ld.%6.6d %s %lx\n",(long)now.tv_sec,(int)now.tv_usec,mutexname,(unsigned long)mutexpt);  \
   } }


#else

#define MUTEX_DEBUG_INFO ""

#define MUTEX_LOCK(mutexpt,where) {               \
   int mstat;                                     \
   if((mstat=pthread_mutex_lock(mutexpt))!=0) {  \
       fprintf(stderr,"LOCK ERR [%s] - %s %lx\n",strerror(mstat),where,(unsigned long)mutexpt);    \
   } }

#define MUTEX_UNLOCK(mutexpt,where) {               \
   int mstat;                                     \
   if((mstat=pthread_mutex_unlock(mutexpt))!=0) {  \
       fprintf(stderr,"UNLOCK ERR [%s] - %s %lx\n",strerror(mstat),where,(unsigned long)mutexpt);    \
   } }

#define MUTEX_INIT(mutexpt,mutexname) {         \
   int mstat;                                     \
    if((mstat=pthread_mutex_init(mutexpt,&mutexAttributes))!=0) {    \
         fprintf(stderr,"INIT ERR [%s] - %s %lx\n",strerror(mstat),mutexname,(unsigned long)mutexpt);    \
    } }

#define MUTEX_DESTROY(mutexpt,mutexname) {         \
   int mstat;                                     \
    if((mstat=pthread_mutex_destroy(mutexpt))!=0) {    \
         fprintf(stderr,"DESTROY ERR [%s] - %s %lx\n",strerror(mstat),mutexname,(unsigned long)mutexpt);    \
    } }


#endif

#ifdef PEER_DEBUG
#define PEER_DEBUG_INFO "PEER_DEBUG"

#define PEER_PRINT(x) LAO_Log(NULL,"PEER_DBG> %s",x);

#else

#define PEER_DEBUG_INFO ""
#define PEER_PRINT(x) 

#endif

#include "base/msglib.h"      // For MsgBuf
#include "base/rtdblib.h"     // For Variable

typedef struct {
   void *clPtr;
   char fname[LONG_NAME_LEN+1];
} cListItem;

typedef struct {
    int nItms;
    int size;
    cListItem *body;
} CList;

// Local representation for variables

typedef struct {
    int            Type;         // Variable type
    int            NItems;       // Variable length
    struct timeval32 MTime;        // Last modification time
} LVHeader;

typedef struct {
    LVHeader    H;
    union  {
	double Dv[0];
	long   Lv[0];
	char   Sv[0];
	unsigned char   B8[0];
	unsigned short  B16[0];
	unsigned long   B32[0];
	unsigned long long B64[0];
 
    }            Value;
} localVar;


// RTDB variable header

typedef struct dbVariable {
    char  Name[VAR_NAME_LEN+1]; // Variable Name
    int            OwnerID;     // Owner ID 
                                // Notify list management fields
    CList *         NList;      // Notify list
    size_t          len;        // Size of allocated memory space

    pthread_mutex_t VarMutex;
    int             Mirrored;   // Variable must be mirrored to peers
    int             Lock;       // Who locked this variable (-1: unlocked)
    int             trace;      // If set to 1 a VarDump is performed each
                                // time the variable is modified
    localVar        *Var;       // Pointer to Variable structure
} DB_Var;


// Types to support hash and sorted lists

typedef struct {
    char *key;
    void *value;
} s_item;

typedef struct {
    int     size;     // Size of sort table
    int     incr;     // Table increment when extending
    int    nItms;     // Number of valid items
    s_item *body;     // pointer to array of s_items
} sort_table;

typedef struct {
    unsigned    size;    // Size of hash table
    unsigned    ovfl;    // Size of hash table + overflow
    unsigned    nItms;   // Number of valid Items (both tables)
    unsigned    nColls;  // Number of Item collisions
    unsigned    nInsert; // Number of Item inserts
    unsigned    nRemove; // Number of Item removes
    DB_Var    **body;    // Hash table
} hash_table;

typedef struct {
    hash_table      htable;    // Hash table
    sort_table      stable;    // Sorted table
    pthread_mutex_t mutex;
} VarTable;

typedef struct {
    sort_table      stable;    // Sorted table
    pthread_mutex_t mutex;
} SortedTable;





//@C
// The following structure is used for implementing search operation
// on sorted lists
//@


typedef struct {
    sort_table  *table;    // Table to search
    int         sId;       // Next element index
    char        *skey;     // name prefix to match
    int         kl;        // matching length
    int         locked;    // Remember if list has been locked
    void        *dummy;
} Listscan;

typedef struct {                  // Client data structure
    int  clType;                  // Client type (STD_CLIENT,PEER_MSGD)
    int  prot43;                  // If true, the client uses old protocol 43
    char Name[PROC_NAME_LEN+1];   // Client name
    char Remote[PROC_NAME_LEN+1]; // Remote client name. If the client is a peer
                                  // here is stored the name of the remote client
    int  Id;                      // Client Id (index in the Item Table)
    struct timeval startTm;       // Client start time
    int  Status;                  // Client connection status
    int  toLog;                   // Log flag
    int  enabled;                 // Client enabled flag
    int  blkErrCount;             // Block error count
    int  toclose;                 // Client marked for forced close
    int  ClientReady;             // Client ready flag; It is initialized to
                                  // zero, and can be set by a proper message
                                  // by the client itself, typically when the
                                  // client has properly defined and initialized
                                  // the database variables. Other clients may
                                  // synchronize on this flag (typically before
                                  // becoming active)
    char Waiting[LONG_NAME_LEN+1];// Client Waiting flag. Indicates that the
                                  // client is waiting for synchronization
                                  // from another client. 
    unsigned WSeqN;               // Store here message sequence number received
                                  // with WAITCLNRDY command
    Connection  conn;             // Connection data (fd, sockaddr)
    pthread_t   thId;             // Thread ID of running thread
    pthread_mutex_t  mutex;       // Client mutex

    MsgBuf rMsgb;                 // Receive message buffer structure
    void  *aux43;                 // Auxiliary buffer to support protocol 43 based clients
    int    auxln;                 // Auxiliary buffer length
    char version[VERSION_ID_LNG]; // Version string
  } Client;


typedef struct {
    int            id;                    // Index in ClientTable
    Client         *cl;                   // Pointer to associated client structure
    char           name[PROC_NAME_LEN+1]; // Peer name
    char           ip[IPPORTLNG];         // Normalized IP/Port spec.
    pthread_cond_t cond;                  // Condition variable;
} Peer;

#define MAXCLIENTS   200   // Max numbers of clients
#define MAXPEERS     8     // Max number of peers

typedef struct {
   Peer body[MAXPEERS];
   int  nPeers;
}  PeerTable;

//  Definition of table for Clients and Peers
//
typedef struct {
  pthread_mutex_t mutex;       // Table mutex
  int     needsCleanup;        // Flag set when the table needs cleanup
  int     nItems;              // Total number of registered clients
  int     nPeers;              // Number of registered peers
  int     maxItem;             // Table physical length
  int     maxPeer;             // Max index of registered peer
  int     maxCurItem;          // Max index of registered client
  Client  *Items[MAXCLIENTS];  // Pointers to Registered clients
  char    *Names[MAXCLIENTS];  // Names of clients
  int     nConn[MAXCLIENTS];   // Client connections counters
}  CLtable;

#endif // TYPES_h
