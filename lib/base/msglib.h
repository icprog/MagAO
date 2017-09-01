//@File: msglib.h

// Definitions for programs using \msgl

// This file contains all definitions needed to use function from \msgl.
// All programs which want to use the library must include this one

// It contains essentially definitions of all message codes and function 
// prototypes.
//@

#ifndef MSGLIB_H
#define MSGLIB_H

#include <string.h>       // For strerror_r() definition
#include <stddef.h>       // For size_t definition
#include <sys/time.h>     // For time_t definition
#include <netinet/in.h>   // For sockaddr_in definition
#include <time.h>         // For timeval definition

#include "base/common.h"
#include "base/timelib.h"

#define BROADCAST    "BCAST"  // Dummy client name for broadcast messages

#define MSGD_PORT     9752    // Port where MsgD-RTBD listens for clients

#define MSGD_LOCAL        1           // Value for MSGD socket mode
#define MSGD_REMOTE       2           // Value for MSGD socket mode


#define STD_CLIENT        1    // Standard client connection
#define PEER_MSGD         2    // Peer MsgD connection

#define CLIENT_IS_UNREGISTERED   0
#define CLIENT_IS_REGISTERED     1
#define CLIENT_IS_READY          2

//@C
// Here we include the definition of all message codes
// used in the communication among Supervisor components.
//@

#include "base/msgcodes.h"

#define NOHANDLE_FLAG      0x01
#define DISCARD_FLAG       0x02
#define REPLY_FLAG         0x40

#define IS_REPLY(x)  ((x)&REPLY_FLAG)
#define NOT_REPLY(x)  (!((x)&REPLY_FLAG))

#define TO_DISCARD(x)  (((x)&(REPLY_FLAG|DISCARD_FLAG))==(REPLY_FLAG|DISCARD_FLAG))
#define TO_HANDLE(x)  (!(((x)&REPLY_FLAG)&&((x)&NOHANDLE_FLAG)))


// #define CLEAR_REQUEST_FLAG(x) ((x)->Msg->HD.Flags) &= (~REQUEST_FLAG)
// #define SET_REQUEST_FLAG(x)   ((x)->Msg->HD.Flags) |= REQUEST_FLAG

#define CLEAR_REPLY(x) ((x)->Msg->HD.Flags) &= (~REPLY_FLAG)
#define SET_REPLY(x)   ((x)->Msg->HD.Flags) |= REPLY_FLAG

#define IPPORTLNG 24

typedef struct {               // Structure for server connection
    int       sock;            // Socket
    struct sockaddr_in skadr;  // Struct sockaddr
    char      ip[IPPORTLNG];   // IP/Port number (xx.xx.xx.xx:pp)
} Connection;

                     // Old header structure (for message magic==43)
typedef struct {
    unsigned  SeqNum;               // Sequence number (just echoed in replies)
    unsigned  Code;                 // Message code
    unsigned  Flags;                // Message flags
    int       ttl;                  // Message Time to live
    int       pLoad;                // Small payload 
    char From[PROC_NAME_LEN+1];     // Message sender
    char To[PROC_NAME_LEN+1];       // Message destination
} Header43;

//            NOTE: whenever a modification is made to the message header, the
//                  value of MSG_HEADER_VERS (see: common.h) must be increased

typedef struct {
    unsigned  Magic;                // Magic number
    unsigned  Len;                  // Message body length
} Signature;

typedef struct {
    unsigned  SeqNum;               // Sequence number (just echoed in replies)
    unsigned  Code;                 // Message code
    unsigned  Flags;                // Message flags
    int       ttl;                  // Message Time to live
    int       pLoad;                // Small payload 
    char From[LONG_NAME_LEN+1];     // Message sender
    char To[LONG_NAME_LEN+1];       // Message destination
} Header;

typedef struct {
    Signature     SG;
    Header43      HD;
    unsigned char Body[];
} Message43;

typedef struct {
    Signature     SG;
    Header        HD;
    unsigned char Body[];
} Message;

//                     values of MSGB.status for thrdlib management
#define B_IN_USE    1
#define B_FREE      2

typedef struct MSGB {
    struct MSGB  *next;    // Pointer for list management
    size_t       buflen;   // size of message body buffer
    int          hd_ok;    // Message communication status true: header correctly received
    int          dwncntr;  // Down counter for message expiration
    int          bId;      // Buffer Identifier (used by thrdlib)
    int          status;   // Buffer status
//            The following part is the actual message
    Message*     Msg;
} MsgBuf;

//                 MsgBuf initializer
#define MSGB_ZERO {NULL,(-1),0,0,0,0,NULL}

// Macros to manipulate header area in a message buffer
// x is a pointer to a MsgBuf structure

#define HDR_MAGIC(x) ((x)->Msg->SG.Magic)
#define HDR_LEN(x) ((x)->Msg->SG.Len)
#define HDR_TO(x) ((x)->Msg->HD.To)
#define HDR_FROM(x) ((x)->Msg->HD.From)
#define HDR_SEQN(x) ((x)->Msg->HD.SeqNum)
#define HDR_CODE(x) ((x)->Msg->HD.Code)
#define HDR_PLOAD(x) ((x)->Msg->HD.pLoad)
#define HDR_FLAGS(x) ((x)->Msg->HD.Flags)
#define HDR_TTL(x)  ((x)->Msg->HD.ttl)

#define MSG_BUFFER(x) ((x)->Msg)        // Pointer to buffer area
#define MSG_HEADER(x) (&(x)->Msg->HD)   // Pointer to Beginning of header
#define MSG_BODY(x) ((x)->Msg->Body)    // Pointer to message body

// Macros to manipulate header area in a message
// x is an MsgBuf structure

#define hdr_LEN(x) ((x).Msg->SG.Len)
#define hdr_TO(x) ((x).Msg->HD.To)
#define hdr_FROM(x) ((x).Msg->HD.From)
#define hdr_SEQN(x) ((x).Msg->HD.SeqNum)
#define hdr_CODE(x) ((x).Msg->HD.Code)
#define hdr_PLOAD(x) ((x).Msg->HD.pLoad)
#define hdr_FLAGS(x) ((x).Msg->HD.Flags)
#define hdr_TTL(x) ((x).Msg->HD.ttl)

#define msg_BUFFER(x) ((x).Msg)        // Pointer to buffer area
#define msg_BODY(x) ((x).Msg->Body)    // Pointer to message body


typedef int (*MsgProc)(MsgBuf *);

// Prototypes

char* StrHeader(MsgBuf *msg, char buf[], int bufl);
char* StrHeaderE(MsgBuf *msg, char *srcd, char *dstd, char buf[], int bufl);
char* HexHeader(MsgBuf *msg, char pref[], int ret);
void PrintHeader(MsgBuf *msg, const char *pref, int ret);
void SetVersion(int vMajor, int vMinor);
char* GetVersionID(void);
int GetVersion(void);
void MsgDebug(int);
int NormalizeIP(char *ip, int lng, struct sockaddr_in * saddr);

void timeString(struct timeval *tv, char *res);

char *logDescr(int);
int   logNumLevel(const char*);

int logString(const char * procName, int level,
               struct timeval *tv, char *result);

int ipPort(void);

char *GetBldFlags(void);

// ------------------------------------ FOR INTERNAL USE ONLY !!!!

void nlogString(const char * procName, int level, int seqn,
               struct timeval *tv, char *result);

void setvers(int,int,int);  
// -------------------------------------------------------------------

#ifdef THRDLIB
#define SetVersion(x,y) setvers(x,y,1)
#else
#define SetVersion(x,y) setvers(x,y,0)
#endif

#ifndef USER_THRDLIB

void setSeed(int);
int SendToSK(int socketfd, MsgBuf *Hdr);
int WaitMessage(int socketfd, MsgBuf *Msg);

int NewSeqN(void);

void ReverseHeader(MsgBuf *msg);
int SetMsgCode(int MsgCode, MsgBuf *msgb);
int SetMsgPload(int pLoad, MsgBuf *msgb);
int SetMsgDest(const char *MsgDest, MsgBuf *msgb);
int SetMsgSender(const char *MsgSender, MsgBuf *msgb);
int SetMsgLength(int MsgLen, MsgBuf *msgb);
int SetFlag(int flag, MsgBuf *msgb);
int SetMsgTTL(int ttl, MsgBuf *msgb);
//int SetRequest(MsgBuf *msgb);
//int ClearRequest(MsgBuf *msgb);
int SetSeqNum(int SeqNum, MsgBuf *msgb);
int FillHeader(int MsgLen, int SeqNum,const char *Sender,
	       const char *Destination, int MsgCode, MsgBuf *msgb);
int FillBody(const void * Buffer, int BufLen, MsgBuf *msgb);
int FillMessage(int MsgLen, int SeqNum, const char *Sender,
	       const char *Destination, int MsgCode,const void *Data, MsgBuf *msgb);


int InitMsgBuf(MsgBuf *msg, int len);
int ZeroMsgBuf(MsgBuf *msg);
int ResizeMsgBuf(MsgBuf *msg, int len);
int FreeMsgBuf(MsgBuf *msg);

int ConnectInit(Connection *tserv) ;
int ConnectTo(Connection*, const char *MyName, int peerID, MsgBuf *);
int ConnectToServer(const char *ServerAddress, const char *MyName, int ntry);
int SendToServer (MsgBuf *Msg);
int CloseServer(const char *Sender);
int WaitMsgFromServer(MsgBuf *msg);
int WaitMsgTimeout(MsgBuf *msg, int timeout);
int WaitMessageSet(int *msgSet, int len, MsgBuf *msg, MsgProc lResort,
                    int timeout); 
int WaitClientReady(const char *myName, const char *wtclient, MsgBuf *msg, MsgProc callbk, int timeout) ;
int GetServerSocket(void);

int LogMsg(const char *MyName, int seqN, int level,const char *msg);

char *mkFullName(char *nm, char *dom, char *buff);

#endif

#include "rtdblib.h"
#include "buflib.h"
#include "errlib.h"

#endif
