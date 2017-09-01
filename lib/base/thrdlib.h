//@File: thrdlib.h

// Definitions for the Threaded library

// Version 2.1  - 22 Nov. 2009
//@

#ifndef THRDLIB_H
#define THRDLIB_H  

#ifdef MSGLIB_H
#error You must not include msglib.h when using thrdlib
#endif

#ifndef LIBRARY_DEVELOPER          // This will define USER_THRDLIB
#define USER_THRDLIB               // When the include file is used by
#endif                             // API user

#ifndef THRDLIB
#define THRDLIB
#endif

#define MAX_HANDLERS       50      // Max # of registered handlers
#define MAX_BUFFERS        2048    // Max # of allocated buffers


#include "base/errlib.h"
#include "base/msglib.h"
#include "base/rtdblib.h"
#include "base/buflib.h"

typedef struct {
    int  initialized;   // Library is in initialized status
    int  connected;     // Library is connected to MsgD
    int  nWaiting;      // Number of waiting messages in message queue
    int  nFree;         // Number of free buffers in buffer queue
    int  nHandlers;     // Number of registered handlers
    int  nFlushd;       // Number of messages flushed since last
                        // call to thInfo()
    int  nTot;          // Total number of allocated buffers
    int DbgLevel;       // 0, 1, 2
    int DefTTL;         // Default message time to live
    char myname[PROC_NAME_LEN+1];
} qInfo;




// Function prototypes

int thInit(const char *myname);
int thHandler(int MsgCode, const char *Sender, int SeqNum, int (*handler)(MsgBuf *, void *, int), const char *name, void *arg);
int thStart(const char *msgdIP, int ntry);
int thClose(void);
int thCleanup(void);
int thSendMsg(int msglen, const char *dest, int  msgC, int ignF, const void *data);
int thSendMsgPl(int msglen, const char *dest, int  msgC, int pLoad, int ignF, const void *data);
int thSendMessage(int msglen, const char *dest, int  msgC, int pLoad, int ignF, int ttl, const void *data);
int thReplyMsg(int  msgC, int msglen, const void *data,MsgBuf *msgb);
int thReplyMsgPl(int  msgC, int pLoad, int msglen, const void *data,MsgBuf *msgb);
int thCheckMsg(int MsgCode, const char *Sender, int   SeqNum); 
MsgBuf *thWaitMsg(int MsgCode, const char *Sender, int SeqNum, int timeout, int* errcod);
int thForward(const char *sender,const char *dest, MsgBuf *msgb);
int thWriteVar(const char *VarName, int VarType, int NElements, const void *Value, int tmout);
MsgBuf *thGetVar(const char *varName, int tmout, int *errcod);
MsgBuf *thReadVar(const char *prefix, int tmout, int *errcod);
MsgBuf *thNextVar(MsgBuf *msgb);

MsgBuf *thClntsList(int tmout, int *errcod) ;
MsgBuf *thBufsList(int tmout, int *errcod) ;

MsgBuf *thClList(int tmout, const char *dest, int *errcod) ;
MsgBuf *thBfList(int tmout, const char *dest, int *errcod) ;

void thRelease(MsgBuf *msgb);
// Message* thDetach(MsgBuf *msgb);
int thVarOp(const char *VarName, int VarOp, int sbc, int tmout);
Variable *thValue(MsgBuf *);
int thWaitReady(const char *client, int tmout);
qInfo *thInfo(void);

int thHaveYou(const char *client);

int thDefTTL(int);
int thFlush(int MsgCode, const char *Sender, int   SeqNum); 
int thDebug(int);
int thLogMsg(int level, int seqn, const char *msg);
int thNewLog(void);

void thErrNotify(void (*notifier)(int, char *));


// shortcuts
int thPing(const char *client, int tmout, double * delay);
int thVersion(const char *client);
int thGetStartT(const char *client, struct timeval *tv);

#endif

