// @File: thrdlib.c
//
// Thread Management Library
//
// This library defines a set of services which may be used to build well
// structured threaded applications which interact with {\tt MsgD-RTDB}.

// Services provided by this library are layered upon three lower level 
// libraries: \msgl, \rtdbl\  and \bufl. When using services from \thrdlib, 
// calls from underlying libraries must no be used: use corresponding services
// from \thrdlib.

//@

#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>


#define POLLING_INTERVAL   5     // Communication established polling time (ms)
#define POLLING_NUMBER     2000  // Number of polling cycles


#define QCLEAN_INTERVAL    1000  // Queue clean loop time (ms)

#define LIBRARY_DEVELOPER 1

#include "base/thrdlib.h"

#define NOT_CONNECTED (Connected != 1)
#define IS_CONNECTED  (Connected == 1)

static int Initialized=0;
static int Connected=0;
static struct timeval startTime;

static int thD1bug=0;              // When set to 1, print to stdout debug info
                                   // related to message management
static int thD2bug=0;              // When set to 1, print more debug adding
                                   // buffer management info
                                   // Use thDebug() to set values.

static int qMgrError=NO_ERROR;     // Errors from qManager are stored here

static int thDeblev=0;

static int thDefaultTTL=DEFAULT_TTL;

pthread_mutex_t snd_mutex = PTHREAD_MUTEX_INITIALIZER;

//
// Default error notifier
//
static void defErrNotif(int level, char *msg)
{
   if(thD1bug) {
      char *levch;
      switch(level) {
         case MASTER_LOG_LEV_ERROR:
            levch=MASTER_LOG_SYM_ERROR;
            break;
         case MASTER_LOG_LEV_WARNING:
            levch=MASTER_LOG_SYM_WARNING;
            break;
         case MASTER_LOG_LEV_INFO:
            levch=MASTER_LOG_SYM_INFO;
            break;
         case MASTER_LOG_LEV_DEBUG:
            levch=MASTER_LOG_SYM_DEBUG;
            break;
         case MASTER_LOG_LEV_TRACE:
            levch=MASTER_LOG_SYM_TRACE;
            break;
         case MASTER_LOG_LEV_VTRACE:
            levch=MASTER_LOG_SYM_VTRACE;
            break;
         case MASTER_LOG_LEV_VVTRACE:
            levch=MASTER_LOG_SYM_VVTRACE;
            break;
         default:
            levch=MASTER_LOG_SYM_FATAL;
            break;
      }
      fprintf(stderr,"%s> %s\n",levch,msg);
   }
}


static void (*thNotify)(int, char *) = &defErrNotif;

static
char *getDomain(const char *name,  // @P{name}: variable full name
                 char      *nbuf)  // @P{vnbuf}: work buffer. It must be
                                   // at least PROC_NAME+LEN+1 in length
                                   // @R: pointer to domain, including @ (possibly "")
{
   char *dom=strchr(name,'@');
   int nc;

   if(dom) {
      nc=dom-name;
      if(nc>PROC_NAME_LEN) return NULL;
   } else {
      nc=PROC_NAME_LEN;
      dom=&nbuf[nc];
   }
   strncpy(nbuf,name,nc);
   nbuf[nc]='\0';

   return dom;
}



//@C
// The following structure holds all the details of the message queue and the handlers list.
//
// The message queue is made up of two double linked lists: pool, a list of reusable message buffers
// and mqueue: the active message queue. The latter is used as a FIFO.

// Messages can be extracted from the list either as a result to a call to @see{thWaitMsg()} or
// by a registered handler. Because of the message matching mechanism, messages can be extracted
// from any point of the queue. When a message is extracted the correspinding slot is marked empty
// and will be reused as soon as there aren't any more messages to be extracted before it.

// Queue access is managed by means of a mutex ({\tt q_mutex}) and a condition variable ({\tt q_cond},
// used to notify the handler loop thread that there is a new message in the queue.
// 
//@

static struct {
    char            myname[PROC_NAME_LEN+1]; // My name (to be registerd as client to \msgd)
    char            msgdIP[20];              // Message daemon IP address (ASCII)
    pthread_mutex_t q_mutex;                 // Queue mutex
    pthread_cond_t  q_cond;                  // Condition variable for message waiting thread
    MsgBuf          *oldst;                  // Pointer to oldest message in queue
    MsgBuf          *newst;                  // Pointer to newest message in  queue
    int             nItms;                   // Number of items in queue
    int             nFlushd;                 // Number of items flushed since last call to thInfo()
    int             disconnected;            // Disconnected flag. Usually 0 The qManager disconnect handler
                                             // will set to 1 before signalling the condition variable.
                                             // This will allow a waiting thread to return with error
                                             // when the MsgD disconnects
} msgQ;

//@C

// The following structure holds the message buffer pool queue;
//@

static struct {
    MsgBuf* list[MAX_BUFFERS]; // Sequential list of allocated buffers
    int     iL;                // Index of last allocated buffer
    pthread_mutex_t q_mutex;   // Queue mutex
    MsgBuf *pool;              // Pointer to first reusable buffer
                               //     for a message handler
    int    nItms;              // Number of items in queue
} pooQ;


//@C
// The {\tt hType} type defines elements of the handler list
//@
typedef struct {
    int msgC;                    // Matching message code
    char from[PROC_NAME_LEN+1];  // Matching sender
    int pLoad;                   // Matching payload

    int (*handler)();            // Message handling routine
    char *hndName;               // Handler Name
    void *arg;                   // Handler arguments
    MsgBuf  *oldst;              // Oldest message in queue
    MsgBuf  *newst;              // Newest message in queue
    int     nItms;               // Number of messages in queue
    int     nFlushd;             // Number of items flushed since last call to thInfo()
    pthread_t bId;               // Thread ID
    pthread_mutex_t q_mutex;     // Queue mutex
    pthread_cond_t  q_cond;      // Condition variable for handlers thread
} hType;



//@C
// The following structure holds the list of registered handlers.
//@

static struct {
    int             nItms;               // Number of handlers currently registered
    hType           hl[MAX_HANDLERS];
}  hList;
  


qInfo myI;         // Storage area for queue status info

pthread_t qManagerThread;    // Message queue thread
pthread_t qCleanThread = 0;   // Queue clean thread




static void myTimeStr(struct timeval *tv, char *tmstr,int maxlen)
{

    struct tm lt;
    time_t tt=tv->tv_sec;

    localtime_r(&tt,&lt);
    snprintf(tmstr,maxlen, "%2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d",
                           lt.tm_mday,lt.tm_mon+1,lt.tm_year+1900,
                           lt.tm_hour,lt.tm_min,lt.tm_sec);
}


//@Function: qMatch

// Test for message match

// This function checks if a message matches given conditions
//@

static int qMatch(int code,const char *from,int seq, MsgBuf *msgb)
{
    unsigned mcode;

    mcode = HDR_CODE(msgb);
    if(code!=ANY_MSG && mcode != (unsigned)code) return 0;
    if(from[0]!='*' && strncmp(HDR_FROM(msgb), from, PROC_NAME_LEN)!=0) return 0;
    if(seq!=0 && HDR_SEQN(msgb)!=(unsigned)seq) return 0;
    return 1;
}

//@Function: hMatch

// Test for message match (handler selection)

// This function checks if a message matches given conditions
//@

static int hMatch(int code,char *from, int pLoad, MsgBuf *msgb)
{
    unsigned mcode;

    mcode = HDR_CODE(msgb);
    if(code!=ANY_MSG && mcode != (unsigned)code) return 0;
    if(from[0]!='*' && strncmp(HDR_FROM(msgb), from, PROC_NAME_LEN)!=0) return 0;
    if(pLoad!=0 && HDR_PLOAD(msgb)!=pLoad) return 0;
    return 1;
}


//@Function: bGet

// Returns the index of a reusable buffer

// After usage the element must be released by calling bRelease
//@

static MsgBuf *bGet(void)
{
MsgBuf *ret;
char errM[1024];

pthread_mutex_lock(&pooQ.q_mutex); 
if(pooQ.pool==NULL) {
    if(pooQ.iL>=MAX_BUFFERS) {
        unsigned long myth=pthread_self();
        snprintf(errM,1024,"thrdlib: [t:%0lx] bGet: exceeded MAX_BUFFERS",myth);
        thNotify(MASTER_LOG_LEV_ERROR,errM);
        pthread_mutex_unlock(&pooQ.q_mutex); 
        return NULL;
    }
    ret=malloc(sizeof(MsgBuf));
    if(ret!=NULL) {
        ret->bId=pooQ.iL;
        pooQ.list[pooQ.iL++]=ret;
        InitMsgBuf(ret,0);
        if(thD2bug) {
            unsigned long myth=pthread_self();
            snprintf(errM,1024,"thrdlib: [t:%0lx] bGet: new buffer #%d:%lX allocated",myth,ret->bId,(unsigned long)MSG_BODY(ret));
            (*thNotify)(MASTER_LOG_LEV_VVTRACE,errM);
        }
    } else {
        unsigned long myth=pthread_self();
        snprintf(errM,1024,"thrdlib: [t:%0lx] bGet: malloc() error: %s",myth,strerror(errno));
        (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
        pthread_mutex_unlock(&pooQ.q_mutex); 
        return NULL;
    }
} else {
    ret=pooQ.pool;
    pooQ.pool=ret->next;
    pooQ.nItms--;
    if(thD2bug) {
        unsigned long myth=pthread_self();
        snprintf(errM,1024,"thrdlib: [t:%0lx] bGet: buffer #%d:%lX reused",myth,ret->bId,(unsigned long)MSG_BODY(ret));
        (*thNotify)(MASTER_LOG_LEV_VVTRACE,errM);
    }
}
ret->next=NULL;
ret->status=B_IN_USE;
pthread_mutex_unlock(&pooQ.q_mutex); 
HDR_TTL(ret)=0;                      // Clear TTL field of message
return ret;
}

//@Procedure: bRel

// Returns a buffer to the buffer pool
//
// This call is to be used when the queue has been already locked.
// To also lock the queue use: bRelease()

//@
static void bRel(MsgBuf *imsg)
{
char errM[1024];

if(imsg->status != B_IN_USE) {
    unsigned long myth=pthread_self();
    snprintf(errM,1024,"thrdlib: [t:%0lx] bRelease: releasing free buffer #%d:%lX",myth,imsg->bId,(unsigned long)MSG_BODY(imsg));
    (*thNotify)(MASTER_LOG_LEV_WARNING,errM);
} else {
    imsg->next=pooQ.pool;
    imsg->status=B_FREE;
    pooQ.nItms++;
    pooQ.pool=imsg;
    if(thD2bug) {
        unsigned long myth=pthread_self();
        snprintf(errM,1024,"thrdlib: [t:%0lx] bRelease: buffer #%d:%lX released",myth,imsg->bId,(unsigned long)MSG_BODY(imsg));
        (*thNotify)(MASTER_LOG_LEV_VVTRACE,errM);
    }
}
}

//@Procedure: bRelease

// Returns a buffer to the buffer pool

//@
static void bRelease(MsgBuf *imsg)
{
if(!imsg) return;
pthread_mutex_lock(&pooQ.q_mutex); 
bRel(imsg);
pthread_mutex_unlock(&pooQ.q_mutex); 
}

//@Procedure: bFree

// Frees memory allocated by bGet

//@
static void bFree(MsgBuf * item)
{
   char errM[1024];
   if(thD2bug) {
        unsigned long myth=pthread_self();
        snprintf(errM,1024,"thrdlib: [t:%0lx] bFree: buffer #%d:%lX freed",myth,item->bId,(unsigned long)MSG_BODY(item));
        (*thNotify)(MASTER_LOG_LEV_VVTRACE,errM);
   }
   FreeMsgBuf(item);
   free(item);
}

//@Procedure: bClean

// Cleanup the buffer pool

//@

static void bClean(void)
{
int i;

pthread_mutex_lock(&pooQ.q_mutex); 
for(i=0;i<pooQ.iL;i++) bFree(pooQ.list[i]);
pthread_mutex_unlock(&pooQ.q_mutex); 

pthread_mutex_destroy(&(pooQ.q_mutex));   // Destroy buffer pool mutex
pooQ.iL=0;
pooQ.pool=NULL;                             // Pointer to first reusable buffer
pooQ.nItms=0;                               // Number of buffers in pool
}

//@Function: qCreate

// Create all queues

//@

static int qCreate(const char *myname)
{
pthread_mutex_init(&(msgQ.q_mutex),NULL);   // Initialize message queue mutex
pthread_cond_init(&(msgQ.q_cond),NULL);     // Initialize message queue condition variable
pthread_mutex_init(&(pooQ.q_mutex),NULL);   // Initialize buffer pool mutex
strncpy(msgQ.myname,myname,PROC_NAME_LEN);  // Store client's name
msgQ.myname[PROC_NAME_LEN]='\0';
msgQ.oldst=NULL;                            // Pointer to oldest message in queue
msgQ.newst=NULL;                            // Pointer to newest message in  queue
msgQ.nItms=0;                // No messages waiting to be retrieved
msgQ.nFlushd=0;              // No messages flushed (yet)
msgQ.disconnected=0;         // To be used by disconnect_handler

pooQ.pool=NULL;              // Pointer to first reusable buffer
pooQ.nItms=0;                // Number of buffers in pool
pooQ.iL=0;
return NO_ERROR;
}

//@Function: qDelete

// Release resources used for queues

//@

static void qDelete(void)
{

pthread_cond_destroy(&(msgQ.q_cond));     // Destroy message queue condition variable
pthread_mutex_destroy(&(msgQ.q_mutex));   // Destroy message queue mutex
*msgQ.myname='\0';
msgQ.oldst=NULL;                            // Pointer to oldest message in queue
msgQ.newst=NULL;                            // Pointer to newest message in  queue
msgQ.nItms=0;                // No messages waiting to be retrieved
msgQ.nFlushd=0;              // No messages flushed (yet)
}


//@Function: qAppend

// Append a new message to the message queue

// The specified message is appended to the queue. 

// Note: The function is NOT thread safe. It must be used only with queue mutex locked.
//@

static void qAppend(MsgBuf *imsg)  // Append a new message to the queue
{
imsg->next=NULL;
if(msgQ.newst==NULL)      // This is the only message in queue
    msgQ.oldst=imsg;      // Initialize pointer to oldest message
else
    msgQ.newst->next=imsg;
msgQ.newst=imsg; 
msgQ.nItms++;
}

//@Function: hAppend

// Append a new message to the input queue of an handler

//@

static void hAppend(MsgBuf *imsg,    // Message to appened
                    hType  *hidx)    // Pointer to handler
{
imsg->next=NULL;
if(hidx->newst==NULL)      // This is the first and only message in queue
    hidx->oldst=imsg;      // Initialize pointer to oldest message
else
    hidx->newst->next=imsg;
hidx->newst=imsg; 
hidx->nItms++;
}

//@Function: qScan

// Scan the message queue and return a pointer to the first matching message
// The message is removed from the queue.

// Note: The function is NOT thread safe. It must be used only with queue mutex locked.
//@

static MsgBuf *qScan(int code,
                     const char *from,
                     int seq)
{
MsgBuf *ret=msgQ.oldst;
MsgBuf *prev=NULL;

while(ret) {
    if(qMatch(code,from,seq,ret)) {
        if(ret==msgQ.oldst) msgQ.oldst=ret->next;
        if(ret==msgQ.newst) msgQ.newst=prev;
        if(prev) prev->next=ret->next;
        ret->next=NULL;                    // Unlink following message
        msgQ.nItms--;
        break;    
    }
    prev=ret;
    ret=ret->next;
}
return ret;
}



//@Procedure: disconnect_handler
//
// Disconnects from MsgD-RTDB when the thread is terminated
//
// This procedure is called by the phtread_cleanup mechanism when the thread is terminated.
// It will disconnect from the MsgD-RTDB
//
//@

static void disconnect_handler(void *argt)
{
int stat;
unsigned long myth=pthread_self();    // Get thread id for logging
char errM[1024];

snprintf(errM,1024,"thrdlib: [t:%0lx] disconnect_handler(%lx) called",myth,(unsigned long)argt);
(*thNotify)(MASTER_LOG_LEV_VTRACE,errM);

stat = CloseServer(msgQ.myname);
if(IS_ERROR(stat)) {
   snprintf(errM,1024,"thrdlib: [t:%0lx] disconnect_handler(): CloseServer(%s) returned error: %d",myth,msgQ.myname,stat);
   (*thNotify)(MASTER_LOG_LEV_WARNING,errM);
}

Connected=0;
}

//@Procedure: unlockamutex
//
// Unlocks given mutex
//
// This procedure is called by the phtread_cleanup mechanism when the thread is terminated.
// It will unlock the mutex passed as argument
//
//@

static void unlockamutex(void *arg)
{
char errM[1024];

int stat=pthread_mutex_unlock((pthread_mutex_t*) arg);
if (stat!=0) {
    unsigned long myth=pthread_self();
    snprintf(errM,1024,"thrdlib: [t:%0lx] unlockamutex: pthread_mutex_unlock() error: %s",myth,strerror(stat));
    (*thNotify)(MASTER_LOG_LEV_WARNING,errM);
}
}

//@Procedure: qClean

// Message queue cleaner

// This routine runs at 1 s intervals to remove expired messages from the message queue

//@

static void qClean(void)
{
unsigned long myth=pthread_self();

for(;;) {
  MsgBuf *pt;
  MsgBuf *prev;
  char errM[1024];
  msleep(QCLEAN_INTERVAL);
  prev=NULL;
  pthread_mutex_lock(&msgQ.q_mutex);
  pt=msgQ.oldst;
  while(pt) {
    if(thD2bug) {
      snprintf(errM,1024,"thrdlib: [t:%0lx] qClean: msg (#%d:%lX) CNTDWN:%d",myth,pt->bId,(unsigned long)MSG_BODY(pt),pt->dwncntr);
      (*thNotify)(MASTER_LOG_LEV_VVTRACE,errM);
    }
    if((pt->dwncntr--)<=0) {
      MsgBuf *next=pt->next;
      if(pt==msgQ.oldst) msgQ.oldst=next;
      if(pt==msgQ.newst) msgQ.newst=prev;
      if(prev) prev->next=next;
      if(thD1bug) {
        snprintf(errM,1024,"thrdlib: [t:%0lx] qClean: removed expired msg (#%d:%lX)",myth,pt->bId,(unsigned long)MSG_BODY(pt));
        (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
      }
      bRel(pt);
      msgQ.nItms--;
      msgQ.nFlushd++;
      pt=next;
    } else {
      prev=pt;
      pt=pt->next;
    }
  }
  pthread_mutex_unlock(&msgQ.q_mutex);
}
}

#define INFO_LEN  256

//@Procedure: qManager

// Message queue manager

// This routine waits for messages from MsgDRTDB and manages the message queue.

// The routine is intended to be run in a dedicated thread. When a new message
// is received from \msgd\  first the list of ignore massages is checked for a match,
// if no match is found the list of message handlers is scanned for a match.
// If a match is found, the message handler manager is notified,
// otherwise the message is put at the end of the message queue.

// Messages are removed from the queue by the application program calling thMsgWait()
// or as a consequence of message expiration by the qClean process.

//@

static void qManager(void)
{
MsgBuf *imsg=NULL;
int stat=NO_ERROR;
struct timeval before,after;
double mstime;
unsigned flags;
unsigned mcode;
unsigned long myth=pthread_self();    // Get thread id for logging
char errM[1024];

snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: started at time 0.000",myth);
(*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
gettimeofday(&before,NULL);

qMgrError=NO_ERROR;
stat = ConnectToServer(msgQ.msgdIP,msgQ.myname,1);     // Open connection to server
if(IS_ERROR(stat)) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: ConnectToServer returned error: %d",myth,PLAIN_ERROR(stat));
    (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
    qMgrError=stat;
    Connected=(-1);  // Signal connection error
    return;
}

gettimeofday(&after,NULL);
mstime=(after.tv_sec-before.tv_sec)*1000.0 + (after.tv_usec-before.tv_usec)*0.001;
snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: connected to MsgD-RTDB at time %.3f ms",myth,mstime);
(*thNotify)(MASTER_LOG_LEV_VTRACE,errM);

Connected=1;

pthread_cleanup_push(disconnect_handler,NULL);   // Install cleanup handler
for(;;) {                                        // Message receiving loop
    if(imsg==NULL) {
       imsg=bGet();                  // Get pointer to empty message buffer
       if(imsg==NULL) {
          snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: bGet() returned NULL",myth);
          (*thNotify)(MASTER_LOG_LEV_FATAL,errM);
          break;
       }
    }
    if(IS_ERROR(stat=WaitMsgFromServer(imsg))) { // Wait message from MsgD/RTDB
        if(thD1bug) { 
            snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: WaitMsgFromServer() returned error %d on msg (#%d:%lX)",
                                        myth,PLAIN_ERROR(stat),imsg->bId,(unsigned long)MSG_BODY(imsg));
            (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
            if(imsg->hd_ok) {
               char strHD[200];
               StrHeader(imsg,strHD,200);
               snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: header OK - %s",myth,strHD);
               (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
            }
        }
        bRelease(imsg);     // Release buffer on error
        break;
    }

    if(thD1bug) {
        char strHD[200];
        StrHeader(imsg,strHD,200);
        snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: received msg (#%d:%lX) - %s",myth,imsg->bId,(unsigned long)MSG_BODY(imsg),strHD);
        (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
    }

    mcode=HDR_CODE(imsg);
    flags=HDR_FLAGS(imsg);
    if(mcode==ECHO)  {                 // Manage replies to echo requests
       ReverseHeader(imsg);
       SET_REPLY(imsg);
       HDR_CODE(imsg)=ECHOREPLY;
       pthread_cleanup_push(unlockamutex,(void *)&snd_mutex);  // Install cleanup handler
       pthread_mutex_lock(&snd_mutex);
       stat=SendToServer(imsg);
       if(IS_ERROR(stat)) {
          snprintf(errM,1024,"[t:%0lx] qManager: error %d in SendToServer",myth,stat);
          (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
       } else {
          if(thD1bug) {
              char strHD[200];
              StrHeader(imsg,strHD,200);
              snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: sent echo reply (#%d:%lX) - %s",myth,imsg->bId,(unsigned long)MSG_BODY(imsg),strHD);
              (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
          }
       }
       pthread_cleanup_pop(1);    // This unlocks the mutex
       continue;
    }
    if(mcode==RQSTINFO) {                // Manage replies to thrdlib info requests
       char thinfo[INFO_LEN];
       struct timeval now;
       long nsecs;
       int hours,minutes,stat,msglen;
       char tmstr[24];
       qInfo *qi=thInfo();


       ReverseHeader(imsg);
       HDR_CODE(imsg)=INFOREPLY;
       SET_REPLY(imsg);
       gettimeofday(&now, NULL);   
       nsecs=(now.tv_sec-startTime.tv_sec);
       hours=nsecs/3600;
       nsecs -= hours*3600;
       minutes=nsecs/60;
       nsecs -= minutes*60;
       myTimeStr(&startTime,tmstr,24);

       msglen=snprintf(thinfo,INFO_LEN,"%s %s\n Started on: %s - uptime (hours): %d:%2.2d:%2.2d\n Def_ttl:%d\n Debug level:%d\n #handlers:%d\n #tot_bufs:%d\n #wtng_msgs:%d\n #free_bufs:%d\n #rmvd_msgs:%d",

                      msgQ.myname,GetVersionID(),tmstr,hours,minutes,(int)nsecs,
                      thDefaultTTL, thDeblev,
                      qi->nHandlers,qi->nTot,qi->nWaiting,qi->nFree,qi->nFlushd);
       if(++msglen>INFO_LEN) msglen=INFO_LEN;
       stat=FillBody(thinfo,msglen,imsg);
       pthread_cleanup_push(unlockamutex,(void *)&snd_mutex);  // Install cleanup handler
       pthread_mutex_lock(&snd_mutex);
       stat=SendToServer(imsg);
       if(IS_ERROR(stat)) {
           snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: error %d in SendToServer",myth,stat);
           (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
       } else {
           if(thD1bug) {
               char strHD[200];
               StrHeader(imsg,strHD,200);
               snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: sent info reply (#%d:%lX)- %s",myth,imsg->bId,(unsigned long)MSG_BODY(imsg),strHD);
               (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
           }
       }
       pthread_cleanup_pop(1);    // This unlocks the mutex
       continue;
    }
    if(mcode==GETVERSION) {                // Manage replies to get version query
       ReverseHeader(imsg);
       SetMsgCode(VERSREPLY,imsg);
       SET_REPLY(imsg);
       SetMsgPload(GetVersion(),imsg);
       SetMsgLength(0,imsg);
       pthread_cleanup_push(unlockamutex,(void *)&snd_mutex);  // Install cleanup handler
       pthread_mutex_lock(&snd_mutex);
       stat=SendToServer(imsg);
       if(IS_ERROR(stat)) {
           snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: error %d in SendToServer",myth,stat);
           (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
       } else {
           if(thD1bug) {
               char strHD[200];
               StrHeader(imsg,strHD,200);
               snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: sent wget version reply (#%d:%lX) - %s",myth,imsg->bId,(unsigned long)MSG_BODY(imsg),strHD);
               (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
           }
       }
       pthread_cleanup_pop(1);    // This unlocks the mutex
       continue;
    }
#define HNDL_LEN   MAX_HANDLERS*50
    if(mcode==HNDLINFO) {                // Manage replies to handlers info requests
       char thinfo[HNDL_LEN];
       int remaining=HNDL_LEN,tot,msglen,i;

       ReverseHeader(imsg);
       HDR_CODE(imsg)=INFOREPLY;
       SET_REPLY(imsg);
       msglen=snprintf(thinfo,100,"%s handler queues",msgQ.myname);
       tot=msglen+1;
       for(i=0;i<hList.nItms;i++) {
           char aux[100];
           hType *hidx=hList.hl+i;

           remaining -= msglen;
           msglen=snprintf(aux,100,"\n %s:%d [nFlush:%d]",hidx->hndName,hidx->nItms,hidx->nFlushd);
           hidx->nFlushd=0;
           if(++msglen>100) msglen=100;
           if(remaining>msglen) { 
              strcat(thinfo,aux); 
              tot += msglen;
           }
       }
       stat=FillBody(thinfo,tot,imsg);
       pthread_cleanup_push(unlockamutex,(void *)&snd_mutex);  // Install cleanup handler
       pthread_mutex_lock(&snd_mutex);
       stat=SendToServer(imsg);
       if(IS_ERROR(stat)) {
           snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: error %d in SendToServer",myth,stat);
           (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
       } else {
           if(thD1bug) {
               char strHD[200];
               StrHeader(imsg,strHD,200);
               snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: sent info reply (#%d:%lX) - %s",myth,imsg->bId,(unsigned long)MSG_BODY(imsg),strHD);
               (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
           }
       }
       pthread_cleanup_pop(1);    // This unlocks the mutex
       continue;
    }
    if(mcode==LOGINCR) {                // Increment logging level
       thDeblev=thDebug(++thDeblev);
       snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: log level set to %d (#%d:%lX)",myth,thDeblev,imsg->bId,(unsigned long)MSG_BODY(imsg));
       (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
       continue;
    }
    if(mcode==LOGDECR) {                // Decrement logging level
       thDeblev=thDebug(--thDeblev);
       snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: log level set to %d (#%d:%lX)",myth,thDeblev,imsg->bId,(unsigned long)MSG_BODY(imsg));
       (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
       continue;
    }
    if(mcode==LOGLEVSET) {                // Set logging level
       thDeblev=thDebug(HDR_PLOAD(imsg));
       snprintf(errM,1024,"thrdlib: DBG> [t:%0lx] qManager: log level set to %d (#%d:%lX)",myth,thDeblev,imsg->bId,(unsigned long)MSG_BODY(imsg));
       (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
       continue;
    }
    if(TO_DISCARD(flags)) {           // Check if message must be discarded
                                      // Yes: Discard message
        if(thD1bug) {
           char strHD[200];
           StrHeader(imsg,strHD,200);
           snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: dropped msg (#%d:%lX) - %s",myth,imsg->bId,(unsigned long)MSG_BODY(imsg),strHD);
           (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
        }
        continue;
    }
        
    if(TO_HANDLE(flags)) {                        // Check if message is handler candidate
        int i;
        for(i=0;i<hList.nItms;i++) {              // Check if an handler exists
            if(hMatch(hList.hl[i].msgC,hList.hl[i].from,hList.hl[i].pLoad,imsg)) {
                hType *hidx=hList.hl+i;
                pthread_cleanup_push(unlockamutex,(void *)&(hidx->q_mutex));  // Install cleanup handler
                pthread_mutex_lock(&(hidx->q_mutex)); 
                hAppend(imsg,hidx);
                if(thD1bug) {
                    char strHD[200];
                    StrHeader(imsg,strHD,200);
                    snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: msg (#%d:%lX) put into handler queue - %s",myth,imsg->bId,(unsigned long)MSG_BODY(imsg),strHD);
                    (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
                }
                pthread_cleanup_pop(1);    // This unlocks the mutex
                stat=pthread_cond_signal(&(hidx->q_cond));  // Awake handler's thread
                imsg=NULL;
                break;
            }
        }
    }
    if(imsg) {
        pthread_cleanup_push(unlockamutex,(void *)&msgQ.q_mutex);  // Install cleanup handler
        pthread_mutex_lock(&msgQ.q_mutex); 

        if(HDR_TTL(imsg)>0)
           imsg->dwncntr=HDR_TTL(imsg);
        else
           imsg->dwncntr=thDefaultTTL;

        qAppend(imsg);                      // Put new message into message queue
        if(thD1bug) {
            char strHD[200];
            StrHeader(imsg,strHD,200);
            snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: msg (#%d:%lX) put into message queue (cntr:%d) - %s",myth,imsg->bId,(unsigned long)MSG_BODY(imsg),imsg->dwncntr,strHD);
            (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
        }
        msgQ.disconnected=0;
        pthread_cleanup_pop(1);                     // This unlocks the mutex
        stat=pthread_cond_broadcast(&msgQ.q_cond);  // Awake message wait thread(s)
        imsg=NULL;
    } 
    if(stat!=0) {
        snprintf(errM,1024,"thrdlib: [t:%0lx] qManager: error from pthread_cond_signal(): %s",myth,strerror(stat));
        (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
    }
}
pthread_cleanup_pop(1);  // Deinstall cleanup handler
}


//@Procedure: handler_cleanup_proc

// Releases resources related to an handler

// This routine is launched by the pthread_cleanup mechanism
// it closes cleanly operations of an handler and releases
// the related resources.
//@

static void handler_cleanup_proc(void *arg)  //@P{hidx}: pointer to handler structure
{
hType *hidx=arg;
MsgBuf *imsg;
unsigned long myth=pthread_self();        // Get thread id for logging
char errM[1024];

// When we enter here the handler queue mutex is locked by this thread.
snprintf(errM,1024,"thrdlib: [t:%0lx] hManager: handler %s received pthread_cancel signal",myth,hidx->hndName);
(*thNotify)(MASTER_LOG_LEV_VTRACE,errM);

while((imsg=hidx->oldst)) {    // Remove remaining messages from queue
   bRel(imsg);
   hidx->oldst=imsg->next;
}
hidx->handler=NULL;

pthread_mutex_unlock(&(hidx->q_mutex));  // Destroy cond variable and mutex
pthread_cond_destroy(&(hidx->q_cond));
pthread_mutex_destroy(&(hidx->q_mutex));

snprintf(errM,1024,"thrdlib: [t:%0lx] hManager: handler %s terminated",myth,hidx->hndName);
(*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
free(hidx->hndName);             // Free memory for handler name
}


//@Procedure: hrelock

// Relock handler message queue mutex

// This routine is launched by the pthread_cleanup mechanism
// it relocks the message queue mutex, so that the status when
// entering handler_cleanup_proc, is known
//@

static void hrelock(void *arg)
{
    pthread_mutex_lock((pthread_mutex_t *)arg);     // Relock handler queue
}

//@Procedure: hManager

// Message Handlers loop manager

// This routine is intended to be run within a dedicated thread. It usually waits
// to be notified by the message queue manager of a new message ready.
// When notified the routine scans the message queue and starts the handler
// on each message in the queue.

//@

static void hManager(void *arg)
{
MsgBuf *msgb=NULL;
hType *hidx=(hType *)arg;
int stat;
unsigned long myth=pthread_self();        // Get thread id for logging
char errM[1024];

snprintf(errM,1024,"thrdlib: [t:%0lx] hManager: handler %s started",hidx->bId,hidx->hndName);
(*thNotify)(MASTER_LOG_LEV_VTRACE,errM);

pthread_mutex_init(&(hidx->q_mutex),NULL);    // Initialize message queue mutex
pthread_cond_init(&(hidx->q_cond),NULL);      // Initialize message queue condition variable

pthread_cleanup_push(handler_cleanup_proc,(void*)hidx);
pthread_mutex_lock(&(hidx->q_mutex));
for(;;) {
                              // Assertion: handler queue mutex LOCKED
    while(hidx->oldst) {      // Check messages in input queue
        msgb=hidx->oldst;
        hidx->oldst=msgb->next;
        if(!hidx->oldst)hidx->newst=NULL;
        hidx->nItms--;
        msgb->next=NULL;
        if(thD1bug) {
            char strHD[200];
            StrHeader(msgb,strHD,200);
            snprintf(errM,1024,"thrdlib: [t:%0lx] hManager: handler %s called for msg (#%d:%lX) - %s",myth,hidx->hndName,msgb->bId,(unsigned long)MSG_BODY(msgb),strHD);
            (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
        }
        pthread_cleanup_push(hrelock,(void*)&(hidx->q_mutex));
        pthread_mutex_unlock(&(hidx->q_mutex));          // Unlock handler queue mutex while ...
        stat=hidx->handler(msgb,hidx->arg,hidx->nItms);  // executing handler
        pthread_cleanup_pop(1);                          // Relock handler queue mutex
        if(IS_ERROR(stat)) {
            snprintf(errM,1024,"thrdlib: [t:%0lx] hManager: handler %s returnerd error: %d",myth,hidx->hndName,stat);
            (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
            while(hidx->oldst) {  // When handler returns with error
                                  // Cleanup handler queue
                msgb=hidx->oldst;
                hidx->oldst=msgb->next;
                if(!hidx->oldst)hidx->newst=NULL;
                hidx->nItms--;
                bRelease(msgb); 
                hidx->nFlushd++;
            }
        }
    }
    if(thD1bug) {
        snprintf(errM,1024,"thrdlib: [t:%0lx] hManager: handler %s waiting msg notification",myth,hidx->hndName);
        (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
    }
    stat=pthread_cond_wait(&(hidx->q_cond),&(hidx->q_mutex)); // Wait for notification
                              // Assertion: handler queue mutex still LOCKED
    if(stat!=0) {
        snprintf(errM,1024,"thrdlib: [t:%0lx] hManager: error from pthread_cond_wait(): %s",myth,strerror(stat));
        (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
        pthread_exit(0);
    }
}
pthread_cleanup_pop(1);     
}



//@Function: _sendMsg

// Sends a message to \msgd.
//
// This routine is used to implement thSendMsg(), thSendMsgPl(), and thSendMessage()
//
//@


static int  _sendMsg(int msglen,       //Length of data (in bytes)
                     const char *dest, //Name of the recipient. Use "" for the MsgD.
                     int  msgC,        //Message code
                     int  pLoad,       // Payload
                     int  flag,        //Message flag. If not 0, it must be either NOHANDLE_FLAG
                                       //or DISCARD_FLAG. The same flag will be included in the reply.
                                       //
                                       //If NOHANDLE_FLAG is set the reply message will be forcibily
                                       //put into the message queue, i.e.: it will not be received
                                       //by an handler even though a matching handler has been
                                       //defined. 
                                       //
                                       //If the DISCARD_FLAG is set, the return message will be 
                                       //silently dropped.
                                       //
                     int  ttl,         // Message time to live
                     const void *data) //Pointer to data
                                       //@R: if > 0, the message was successfully sent, and the value is the 
                                       //         unique sequence number of the message sent.
                                       //    if < 0: error code
{
MsgBuf *msgb; int ret; int seqN;
unsigned long myth=pthread_self();
char errM[1024];

if(NOT_CONNECTED) return THRD_NOT_CONNECTED_ERROR;
msgb=bGet();
if(!msgb) return NO_BUFFER_ERROR;
seqN=NewSeqN();
FillMessage(msglen,seqN,msgQ.myname,dest,msgC,data,msgb);
if(IS_ERROR(ret=SetFlag(flag,msgb))) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] thSendMsg: SetFlag(%d) error",myth,flag);
    (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
    bRelease(msgb);
    return ret;
}
SetMsgTTL(ttl,msgb);
HDR_PLOAD(msgb)=pLoad;

pthread_mutex_lock(&snd_mutex);
ret = SendToServer(msgb);
if(IS_ERROR(ret)) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] _sendMsg: error %d in SendToServer",myth,ret);
    (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
} else {
    if(thD1bug) {
        char strHD[200];
        StrHeader(msgb,strHD,200);
        snprintf(errM,1024,"[t:%0lx] thSendMsg: sent msg (#%d:%lX) - %s",myth,msgb->bId,(unsigned long)MSG_BODY(msgb),strHD);
        (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
    }
}
pthread_mutex_unlock(&snd_mutex);
bRelease(msgb);
if(IS_NOT_ERROR(ret)) return seqN;
return ret;
}



//@Function(API): thForward

// Forward a message
//
// This routine forwards a message received.
//
//@

int thForward(const char *sender, //@P{sender}: Message sender. Set to NULL to
                                  // keep the original sender
              const char *dest,   //@P{dest}: Name of the recipient. Use "" for the MsgD.
              MsgBuf     *msgb)   //Message buffer containing the message to forward.
                                  //Note: the message buffer is released after sending the message
                                  //@R: if > 0, the message was successfully sent,
                                  //            and the value is the unique sequence number.
                                  //            of the message sent
                                  //    if < 0: error code
{
int ret,seqN;
unsigned long myth=pthread_self();
char errM[1024];

if(NOT_CONNECTED) return THRD_NOT_CONNECTED_ERROR;

if(sender) SetMsgSender(sender, msgb);
SetMsgDest(dest, msgb);
seqN=HDR_SEQN(msgb);

pthread_mutex_lock(&snd_mutex);
ret = SendToServer(msgb);
if(IS_ERROR(ret)) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] _replyMsg: error %d from SendToServer",myth,ret);
    (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
} else {
    if(thD1bug) {
        char strHD[200];
        StrHeader(msgb,strHD,200);
        snprintf(errM,1024,"thrdlib: [t:%0lx] thReplyMsg: sent msg (#%d:%lX) - %s",myth,msgb->bId,(unsigned long)MSG_BODY(msgb),strHD);
        (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
    }
}
pthread_mutex_unlock(&snd_mutex);
bRelease(msgb);
if(IS_NOT_ERROR(ret)) return seqN;
return ret;
}



//@Function: _replyMsg

// Sends a message to \msgd.
//
// This routine is used to implement thReplyMsg() and thReplyMsgPl()
//
//@

static int _replyMsg(int  msgC,       //Message code
                    int msglen,       //Length of data (in bytes)
                    const void *data, //Pointer to data
                    MsgBuf *msgb)     //Message buffer containing the request message.
                                      //Note: the message buffer is reused for the reply, and
                                      //      then released. The caller must save the message 
                                      //      contents if it is needed after the call.
                                      //@R: if > 0, the message was successfully sent,
                                      //            and the value is the unique sequence number.
                                      //            of the message sent
                                      //    if < 0: error code
{
int ret,seqN;
unsigned long myth=pthread_self();
char errM[1024];

if(NOT_CONNECTED) return THRD_NOT_CONNECTED_ERROR;

HDR_CODE(msgb) = msgC;
seqN=HDR_SEQN(msgb);
ReverseHeader(msgb);
SET_REPLY(msgb);
if (IS_ERROR(ret=FillBody(data,msglen,msgb))) return ret;
pthread_mutex_lock(&snd_mutex);
ret = SendToServer(msgb);
if(IS_ERROR(ret)) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] _replyMsg: error %d from SendToServer",myth,ret);
    (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
} else {
    if(thD1bug) {
        char strHD[200];
        StrHeader(msgb,strHD,200);
        snprintf(errM,1024,"thrdlib: [t:%0lx] thReplyMsg: sent msg (#%d:%lX) - %s",myth,msgb->bId,(unsigned long)MSG_BODY(msgb),strHD);
        (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
    }
}
pthread_mutex_unlock(&snd_mutex);
bRelease(msgb);
if(IS_NOT_ERROR(ret)) return seqN;
return ret;
}



//@Function(API): thReplyMsg

// Sends a message to \msgd\  in reply to some request

// This routine can be called to reply to a message received from
// an application which is requesting a service. 

// Applications which have to send messages to request services by
// any server (including the \msgd\  itself) must use @see{thSendMsg()}
// or @see{thSendMsgPl() or @see{thSendMessage}}.
//
// When using this roputine the message header payload from the
// received message is left unaffected. Use @see{thReplyMsgPl()} to
// modify the header payload.
//
// The routine uses the message buffer as received from the client to
// set up the reply. After sending the message, the message buffer  is
// automatically released. So after a call to thReplyMsg() no explicit
// call to thRelease() must be done.
//@


int thReplyMsg(int  msgC,        //@P{msgC}: Message code
               int msglen,       //@P{msglen}: Length of data (in bytes)
               const void *data, //@P{data}: Pointer to data
               MsgBuf *msgb)     //@P{msgb}: Message buffer containing the request message.
                                 //          Note: the message buffer is reused for the reply, and
                                 //                then released. The caller must save the message 
                                 //                contents if it is needed after the call.
                                 //@R: if > 0, the message was successfully sent,
                                 //            and the value is the unique sequence number.
                                 //            of the message sent
                                 //    if < 0: error code
{
return _replyMsg(msgC,msglen,data,msgb);
}


//@Function(API): thReplyMsgPl

// Sends a message to \msgd\  in reply to some request

// This routine can be called to reply to a message received from
// an application which is requesting a service. 
//
// This routine allows to set the message header payload (use @see{thReplyMsg()
// to leave original header payload unaffected.

// Applications which have to send messages to request services by
// any server (including the \msgd\  itself) must use @see{thSendMsg()}.
//
// The routine uses the message buffer as received from the client to
// set up the reply. After sending the message, the message buffer  is
// automatically released. So after a call to thReplyMsg() no explicit
// call to thRelease() must be done.
// 
//@


int thReplyMsgPl(int  msgC,        //@P{msgC}: Message code
                 int  pLoad,       //@P{pLoad}: header payload.
                 int msglen,       //@P{msglen}: Length of data (in bytes)
                 const void *data, //@P{data}: Pointer to data
                 MsgBuf *msgb)     //@P{msgb}: Message buffer containing the request message.
                                   //          Note: the message buffer is reused for the reply, and
                                   //                then released. The caller must save the message 
                                   //                contents if it is needed after the call.
                                   //@R: if > 0, the message was successfully sent,
                                   //            and the value is the unique sequence number.
                                   //            of the message sent
                                   //    if < 0: error code
{
HDR_PLOAD(msgb)=pLoad;
return _replyMsg(msgC,msglen,data,msgb);
}


//@Function(API): thSendMsg

// Sends a message to \msgd.
//
// This routine can be called to send a generic request message to \msgd.
// Reply messages must be sent by calling @see{thReplyMsg()}
//
// The payload and message time to live fields in in message header are 
// both set to zero. Use @see{thSendMsgPl()} or @see{thSendMessage()}
// to control also those fields.
//
//@


int thSendMsg(int msglen,       //@P{msglen}: Length of data (in bytes)
              const char *dest, //@P{dest}: Name of the recipient. Use "" for the MsgD.
              int  msgC,        //@P{msgC}: Message code
              int  flag,        //@P(flag}: Message flag. If not 0, it must be either NOHANDLE_FLAG
                                //          or DISCARD_FLAG. The same flag will be included in the reply.
                                //          If NOHANDLE_FLAG is set the reply message will be forcibily
                                //          put into the message queue, i.e.: it will not be received
                                //          by an handler even though a matching handler has been
                                //          defined.
                                //          If the DISCARD_FLAG is set, the return message will be 
                                //          silently dropped.
              const void *data) //@P{data}: Pointer to data
                                //@R: if > 0, the message was successfully sent, and the value is the 
                                //         unique sequence number of the message sent.
                                //    if < 0: error code
{
return _sendMsg(msglen,dest,msgC,0,flag,0,data);
}

//@Function(API): thSendMsgPl

// Sends a message to \msgd.
//
// This routine can be called to send a generic message to \msgd\  by
// a client application. Server applications (i.e.: programs which
// reply to messages sent by clients) must use @see{thReplyMsg()}
//
// This routine allows to specify an integer value as payload in the
// message header.
//
// The message time to live field in in message header is set to zero. 
// Use @see{thSendMessage()} to control also the message TTL.
//
//@


int thSendMsgPl(int msglen,       //@P{msglen}: Length of data (in bytes)
	        const char *dest, //@P{dest}: Name of the recipient. Use "" for the MsgD.
                int  msgC,        //@P{msgC}: Message code
                int  pLoad,       //@P{msgC}: Header payload
                int  flag,        //@P(flag}: Message flag. If not 0, it must be either NOHANDLE_FLAG
                                  //          or DISCARD_FLAG. The same flag will be included in the reply.
                                  //          If NOHANDLE_FLAG is set the reply message will be forcibily
                                  //          put into the message queue, i.e.: it will not be received
                                  //          by an handler even though a matching handler has been
                                  //          defined.
                                  //          If the DISCARD_FLAG is set, the return message will be 
                                  //          silently dropped.
                const void *data) //@P{data}: Pointer to data
                                  //@R: if > 0, the message was successfully sent, and the value is the 
                                  //         unique sequence number of the message sent.
                                  //    if < 0: error code
{
return _sendMsg(msglen,dest,msgC,pLoad,flag,0,data);
}

//@Function(API): thSendMessage

// Sends a message to \msgd.
//
// This routine can be called to send a generic message to \msgd\  by
// a client application. 
//
// Server applications (i.e.: programs which reply to messages sent by 
// clients) must use @see{thReplyMsg()}
//
// All messages can be assigned a "Time to Live" (TTL) value. This value
// is used by the application receiving the message to implement a message 
// expiration mechanism which prevents message queue overflow.
//
// The mechanism is as follows:
//
// 1. When a message is received and put in the message queue, the TTL value
// is checked. If it is 0 the  message is assigned a default TTL value by the 
// receiver. If the TTL value in the message header is greather than 0, this 
// TTL value is used instead of the default.
//
// 2. Each thrdlib based application has a queue cleaning process which runs
// at 1 second intervals and decreases the TTL value for all messages in queue
// dropping messages whose TTL is zero.
//
// 3. When replying to a message by using thReplyMsg() or thReplyMsgPl(), the
//    TTL value in the message header is echoed back.
//
//  Rules 1 and 2 means that any message which is not managed by the receiving 
//  application will be dropped after a given time. Usually this time is
//  the default value established by the receiving application (see also: @see{thDefTTL})
// 
//  Rule 3 allows an application to assign a TTL value different from the default
//  for each message which is followed by a reply by assigning a message specific
//  TTL when sending the request.  Note: replies to messages sent with DISCARD_FLAG set
//  are dropped anyway.
//@


int thSendMessage(int msglen,       //@P{msglen}: Length of data (in bytes)
	          const char *dest, //@P{dest}: Name of the recipient. Use "" for the MsgD.
                  int  msgC,        //@P{msgC}: Message code
                  int  pLoad,       //@P{msgC}: Header payload
                  int  flag,        //@P(flag}: Message flag. If not 0, it must be either NOHANDLE_FLAG
                                    //          or DISCARD_FLAG. The same flag will be included in the reply.
                                    //          If NOHANDLE_FLAG is set the reply message will be forcibily
                                    //          put into the message queue, i.e.: it will not be received
                                    //          by an handler even though a matching handler has been
                                    //          defined.
                                    //          If the DISCARD_FLAG is set, the return message will be 
                                    //          silently dropped.
                  int  ttl,         // Message time to live (sec.).
                  const void *data) //@P{data}: Pointer to data
                                    //@R: if > 0, the message was successfully sent, and the value is the 
                                    //         unique sequence number of the message sent.
                                    //    if < 0: error code
{
return _sendMsg(msglen,dest,msgC,pLoad,flag,ttl,data);
}


// @Function(API): thCheckMsg 

// Check message queue for given message
//
// This routine looks in the input message queue for messages matching
// specified selection criteria and returns true if such a message exists.
//@

int thCheckMsg(int   msgC,         // @P{msgC}: Wait for specific message code
                                   // The special code "ANY_MSG" may be used to match
                                   // any message code.
               const char *sender, // Specify sender of message. The NUL string ""
                                   // matches messages originating from the \msgd.
                                   // The string "*" matches any sender
               int   seqN)         // @P{seqN}: specify sequence number.
                                   // if 0, any sequence number will match
                                   // @R: ture if a matching message is found. False otherwise.
{
MsgBuf *ret;
				// Scan the message queue
pthread_mutex_lock(&msgQ.q_mutex);
ret=msgQ.oldst;
while(ret) {
    if(qMatch(msgC,sender,seqN,ret)) break;    
    ret=ret->next;
}
pthread_mutex_unlock(&msgQ.q_mutex);
if(ret)
    return 1;
else
    return 0;
}


// @Function(API): thWaitMsg 

// Waits for a message from \msgd
//
// This routine looks in the input message queue for messages matching
// specified selection criteria. If no matching message is available in the queue,
// it waits tmout milliseconds (or forever if tmpout is 0) for incoming messages.
//
// The routine returns with a pointer to the message buffer or with NULL if no
// matching message has been received.
//
// If a message is received, after using message data, the buffer must be released
// with a call to thRelease().
// 
//@

MsgBuf *thWaitMsg(int   msgC,         // @P{msgC}: Wait for specific message code
                                      // The special code "ANY_MSG" may be used to match
                                      // any message code.
                  const char *sender, // Specify sender of message. The NUL string ""
                                      // matches messages originating from the \msgd.
                                      // The name "*" matches any sender
                  int   seqN,         // @P{seqN}: specify sequence number.
                                      // if 0, any sequence number will match
                  int   tmout,        // @P{tmout}: Timeout (millisec).    0: wait forever
                                      //                                  (-1): return immediately
                  int*  errc)         // @P{errc}: Error code when NULL is returned
                                      // @R: Pointer to message buffer (NULL if no message has 
                                      //     been selected)
                                      //    Note: After using it, the message buffer must be 
                                      //    released by  calling thRelease()
{
int stat;
MsgBuf *ret=NULL;
long nsecs,nnsec;
struct timespec tfinal;
unsigned long myth=pthread_self();
*errc=UNMANAGED_ERROR;          // This error should never be returned!
char errM[1024];
				// First look into the message queue
pthread_mutex_lock(&msgQ.q_mutex);
if(thD1bug) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: scanning message queue [msgC:%x sender:%s seqN:%d]",
                   myth,                                     msgC,   sender,   seqN); 
    (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
}

ret=qScan(msgC,sender,seqN);
if(ret) {
    pthread_mutex_unlock(&msgQ.q_mutex);
    if(thD1bug) {
        char strHD[200];
        StrHeader(ret,strHD,200);
        snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: match found (#%d:%lX) - %s",myth,ret->bId,(unsigned long)MSG_BODY(ret),strHD); 
       (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
    }
    *errc=NO_ERROR;
    return ret;
}

if(tmout<0) {
    pthread_mutex_unlock(&msgQ.q_mutex);
    if(thD1bug) {
        snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: no match found. Returning.",myth); 
       (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
    }
    *errc=COND_WAIT_ERROR;
    return NULL;
}
if(thD1bug) {
    if(tmout==0) {
        snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: no match found. Waiting forever.",myth); 
    } else {
        snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: no match found. Waiting %d ms.",myth,tmout); 
    }
    (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
}
                               // No matching message, prepare for wait
if(tmout==0) {
    for(;;) {
        stat=pthread_cond_wait(&msgQ.q_cond, &msgQ.q_mutex);
        if(msgQ.disconnected) {     // MsgD disconnected
            if(thD1bug) {
                snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: MsgD disconnected",myth); 
                (*thNotify)(MASTER_LOG_LEV_WARNING,errM);
            }
            *errc=COMMUNICATION_ERROR;
            return NULL;
        }
        if(stat==0) {
            ret=qScan(msgC,sender,seqN);
            if(ret) {
                pthread_mutex_unlock(&msgQ.q_mutex);
                if(thD1bug) {
                    char strHD[200];
                    StrHeader(ret,strHD,200);
                    snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: got notif... match found (#%d:%lX) - %s",myth,ret->bId,(unsigned long)MSG_BODY(ret),strHD); 
                    (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
                }
                *errc=NO_ERROR;
                return ret;
            }
            if(thD1bug) {
                snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: got notif... no match found",myth); 
                (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
            }
        } else {
            snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: pthread_cond_wait() returned error %s",myth,strerror(stat));
            (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
            pthread_mutex_unlock(&msgQ.q_mutex);
            *errc=COND_WAIT_ERROR;
            return NULL;
        }
    }
} else {
    nsecs=tmout/1000;
    nnsec=(tmout-nsecs*1000L)*1000000L;
    clock_gettime(CLOCK_REALTIME,&tfinal);    // Get current time
    tfinal.tv_sec += nsecs;                   // Compute final time
    tfinal.tv_nsec += nnsec;
    if(tfinal.tv_nsec > 1000000000L) {
        tfinal.tv_nsec -= 1000000000L;
        tfinal.tv_sec++;
    }
    for(;;) {
        stat=pthread_cond_timedwait(&msgQ.q_cond, &msgQ.q_mutex, &tfinal);
        if(msgQ.disconnected) {     // MsgD disconnected
            if(thD1bug) {
                snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: MsgD disconnected",myth); 
                (*thNotify)(MASTER_LOG_LEV_WARNING,errM);
            }
            *errc=COMMUNICATION_ERROR;
            return NULL;
        }
        if(stat == ETIMEDOUT) {                   // If timeout, return
            pthread_mutex_unlock(&msgQ.q_mutex);
            if(thD1bug)  {
                snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: timeout expired",myth); 
                (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
            }
            *errc=TIMEOUT_ERROR;
            return NULL;
        }
        if(stat==0) {
            ret=qScan(msgC,sender,seqN);
            if(ret) {
                pthread_mutex_unlock(&msgQ.q_mutex);
                if(thD1bug) {
                    char strHD[200];
                    StrHeader(ret,strHD,200);
                    snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: got notif... match found (#%d:%lX) - %s",myth,ret->bId,(unsigned long)MSG_BODY(ret),strHD); 
                    (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
                }
                *errc=NO_ERROR;
                return ret;
            }
        } else {
            snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: pthread_cond_timedwait() returned error %s",myth,strerror(stat));
            (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
            pthread_mutex_unlock(&msgQ.q_mutex);
            *errc=COND_WAIT_ERROR;
            return NULL;
        }
        if(thD1bug) {
            snprintf(errM,1024,"thrdlib: [t:%0lx] thWaitMsg: got msg notif... no match found",myth); 
            (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
        }
    }
}
}



// @Function(API): thInit 

// Initializes the threading services

// This routine must be called only once prior of using other functions from the
// threading library.

// In order to properly startup the threaded client, the following sequence
// must be used:

// \begin{itemize}
// \item  thInit().   Initialize the client
// \item  thHandler() (optional) Add message handler
// \item  thHandler() (optional) .....
// \item  thStart().  Start the connection
// \item   ... do the job
// \item  thClose().  Close connection with the server
// \end{itemize}
// Then optionally:
// \item  thStart().  Restart the connection
// \item   ... do the job
// \item  thClose().  Close connection with the server
// \item   ... and so on
// \begin{itemize}
// \end{itemize}

// I.e.: message handlers must be defined after initialization and
// before starting the connection.

//@

int thInit(const char * myname)     // @{myName}: client's name (max 10 characters)
                              // @R: completion code
{
int ret;
unsigned long myth=pthread_self();
char errM[1024];

if(Initialized) return THRD_INITIALIZE_ERROR ;

if(thD1bug) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] thInit: called",myth);
    (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
}
ret=qCreate(myname);
if(ret!=NO_ERROR) {
    if(thD1bug) {
        snprintf(errM,1024,"thrdlib: [t:%0lx] thInit: error %d from qCreate",myth,ret);
        (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
    }
    return ret;
}

hList.nItms=0;      // Initialize handler queue

Initialized=1;
gettimeofday(&startTime, NULL);
return NO_ERROR;
}

// @Function(API): thStart 

// Start the threading services

// This routine must be called to start the connection with
// the \msgd. 

// In order to properly startup the threaded client, the following sequence
// must be used:
// \begin{itemize}
// \item  thInit().   Initialize the client
// \item  thHandler() (optional) Add message handler
// \item  thHandler() (optional) .....
// \item  thStart().  Start the connection
// \end{itemize}

// I.e.: message handlers must be defined before starting the connection
//
// After opening the connection and doing the job, a call to @see{thClose} can be
// used to close connection. A closed connection can be reopened by calling again thStart().

//@

int thStart(const char * msgdIP, // Specify Msgd-RTDB IP address.
                                 // If empty or NULL, use: "127.0.0.1"
            int    ntry)         // number of connection retries (0: wait forever)
                                 // @R: completion code
{
pthread_attr_t attribute;
int stat,step;
unsigned long myth=pthread_self();    // Get thread id for logging
int cnt;
char errM[1024];

if(!Initialized || (Connected>0)) {
    if(thD1bug) {
        snprintf(errM,1024,"thrdlib: [t:%0lx] thStart: wrong status for start",myth);
        (*thNotify)(MASTER_LOG_LEV_WARNING,errM);
    }
    return THRD_INITIALIZE_ERROR;
}

strncpy(msgQ.msgdIP,msgdIP,20);

pthread_attr_init(&attribute);
pthread_attr_setdetachstate(&attribute,               // Declare thread "detached"
                            PTHREAD_CREATE_DETACHED); // So that it dies at end

if(ntry>0) step=1;
else { step=0; ntry=1; }

while(ntry) {
   if(thD1bug) {
      snprintf(errM,1024,"thrdlib: [t:%0lx] thStart: spawning qManager. Remaing retries: %d",myth,ntry);
      (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
   }
   Connected=0;
   if((stat=pthread_create(&(qManagerThread),&attribute,(void *)&qManager, NULL))!=0) {
       snprintf(errM,1024,"thrdlib: [t:%0lx] thStart: error from pthread_create when spawning qManager - %s",myth,strerror(stat));
       (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
       pthread_attr_destroy(&attribute);        // Attributes can be destroyed
       return THREAD_CREATION_ERROR;
   }

   cnt=POLLING_NUMBER; stat=0;
   if(thD1bug) {
      snprintf(errM,1024,"thrdlib: [t:%0lx] thStart: waiting for qManager to be connected with MsgD",myth);
       (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
   }
   while(cnt--) {      // Wait for the connection to establish
       msleep(POLLING_INTERVAL);
       if(Connected<0) break;
       if(Connected==1) {
           stat=1;
           break;
       }
   }
   if(stat) break;
   ntry -= step;
   pthread_cancel(qManagerThread);
   if(thD1bug) {
      snprintf(errM,1024,"thrdlib: [t:%0lx] thStart: qManager thread %0lx canceled",myth,qManagerThread);
      (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
   }
}
if(stat) {
   if(thD1bug) {
       int msec = (POLLING_NUMBER-cnt)*POLLING_INTERVAL;
       snprintf(errM,1024,"thrdlib: [t:%0lx] thStart: connection established within %d ms",myth,msec);
       (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
   }
} else {
   if(thD1bug) {
      snprintf(errM,1024,"thrdlib: [t:%0lx] thStart: connection refused",myth);
      (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
   }
   pthread_attr_destroy(&attribute);  // Attributes can be destroyed
   return qMgrError;
}
                       // Now spawn the Queue Cleaning thread
if((stat=pthread_create(&(qCleanThread),&attribute,(void *)&qClean, NULL))!=0) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] thStart: error from pthread_create when spawning qClean - %s",myth,strerror(stat));
    (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
    pthread_cancel(qManagerThread);    // Cancel queue manager thread
    pthread_attr_destroy(&attribute);  // Attributes can be destroyed
    return THREAD_CREATION_ERROR;
}
if(thD1bug) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] thStart: qClean thread successfully spawned.",myth);
    (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
}
                                         // The threads have been successfully spawned.
pthread_attr_destroy(&attribute);        // Attributes can be destroyed
return NO_ERROR;
}

// @Function(API): thCleanup 

// Releases resources allocated by thInit

// This routine must be called to terminate a thrdlib based application.
// It will clean up and release resources allocated when @see{thInit} was called.
// Handlers will be terminated as soon as the current message processing is
// done.
// All messages still waiting in any queue will be discarded.
//
// After calling thCleanup, an application may initiate a new
// thrdlib based session by repeating the initialization sequence
// (see @see{thInit}).

// Note: if needed @see{thClose} will be called automatically, so there
// is no need for an application to call thClose if thCleanup is going
// to be called thereafter (it will not do any harm, anyway)/
//@

int thCleanup(void)               // @R: completion code
{
int ret=NO_ERROR,i,stat;
unsigned long myth = pthread_self();
char errM[1024];

if(!Initialized) return THRD_INITIALIZE_ERROR;

ret=thClose();    // Close communication with MsgD, if needed

msgQ.disconnected=1;                        // Notify disconnection when ..
stat=pthread_cond_broadcast(&msgQ.q_cond);  // awaking message wait thread(s)

for(i=0;i<hList.nItms;i++) {  // Stop handler threads
    hType* hidx=hList.hl+i;
    if(thD1bug) {
        snprintf(errM,1024,"thrdlib: [t:%0lx] thCleanup: sending pthread_cancel(%0lx)",myth,hidx->bId);
        (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
    }
    stat=pthread_cancel(hidx->bId);
    if(stat!=0 && thD1bug) {
        snprintf(errM,1024,"thrdlib: [t:%0lx] thCleanup: pthread_cancel() returned error: %s",myth,strerror(stat));
        (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
    }
}

if(thD1bug) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] thCleanup: waiting for handler threads to terminate",myth);
    (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
}

for(i=0;i<hList.nItms;i++) {  // wait for handler threads to close
    hType* hidx=hList.hl+i;
    void *res;
    pthread_join(hidx->bId,&res);
}

if(thD1bug) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] thCleanup: cleaning handler list",myth);
    (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
}
hList.nItms=0;
if(thD1bug) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] thCleanup: cleaning message queue",myth);
    (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
}
qDelete();        // remove all queues
if(thD1bug) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] thCleanup: cleaning buffer pool",myth);
    (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
}
bClean();         // Remove all buffers
Initialized=0;

return NO_ERROR;
}

// @Function(API): thClose 

// Stops the connection with MsgD

// This routine can be called to close the connection with \msgd.
// A call to @see{thStart} can be used to reopen the connection.
// In this case @see{thInit} is not to be called again.

// If the connection is  not intended to be reopened, then
// the routine @see{thCleanup} must be called instead of thClose.
//@

int thClose(void)               // @R: completion code
{
int ret;
char errM[1024];

if(!Initialized) return THRD_INITIALIZE_ERROR;
if(Connected) {
   pthread_t myth=pthread_self();
   ret=pthread_cancel(qManagerThread);
   if(ret!=0 && thD1bug) {
      snprintf(errM,1024,"thrdlib: [t:%0lx] thClose error: %s",myth,strerror(ret));
      (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
   } else {
      snprintf(errM,1024,"thrdlib: [t:%0lx] thClose: qManager thread stopped",myth);
      (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
   }
   if (qCleanThread) {
      ret=pthread_cancel(qCleanThread);
      qCleanThread=0;
      if(ret!=0) {
         pthread_t myth=pthread_self();
         snprintf(errM,1024,"thrdlib: [t:%0lx] thClose error: %s",myth,strerror(ret));
         (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
      } else {
         snprintf(errM,1024,"thrdlib: [t:%0lx] thClose: qClean thread stopped",myth);
         (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
      }
   }
}
return NO_ERROR;
}


// @Function(API): thHandler

// Register handler for specified message

// This routine  may be called to register and handling routine for messages
// matching given criteria. Each registerd handler is executed in its own
// thread and is provided with a message queue.

// When installing more than one handler, the test for matching criteria is performed
// in the order of installation and the first matching handler is selected.
// I.e.: if an handler matching every message is registered as last, it will
// work as a default handler.
//
// Note: messages may be excluded by the matching mechanism for handlers by
//       using the proper flag in thSendMsg().
//
// Handlers are called with three arguments: a message buffer pointer, pointing to the
// message to be handled; a pointer to the "arg" argument specified when the
// handler is installed (see below); an integer with the number of messagfes remaining in
// the handler's input queue. 
//
// When message processing is finished the handler must free the message buffer by calling
// thRelease() on it.
//
// Message handlers are supposed to behave properly, i.e.: they should perform some 
// operation and then return. In other words, they should not block themselves 
// waiting for some synchronization (e.g.: waiting for a reply to some message they've sent)
// or for some resource to become available.
//
// While the handler is running, the handler input queue is unlocked: so new messages may be
// insterted into queue. The handler managing process will continue to call the handler on
// each message in sequence until the queue is empty.
//
// Handlers must return an integer error code. The return value must be NO_ERROR when the
// message processing was successful. A negative return value indicates processing
// errors. When the handler retunrs an error value, the messages which may be waiting in
// the handler queue, are unconditionally removed from the queue.
//@

int thHandler(int   msgC,         // @P{msgC}: Handler will serve specific message code
                                  // The special code "ANY_MSG" may be used to match
                                  // any message code.
	      const char *sender, // @P{sender}: Handler will serve messages sent by specified sender. 
                                  // The NUL string "" matches messages originating from the \msgd.
                                  // The string "*" matches any sender.
              int   pLoad,        // @P{pLoad}: handler will serve messages with
                                  // specified pLoad.
                                  // if 0, any pLoad value will match.
              int (*handler)(),   // @P{handler}: Handling routine. The routine will be called with
                                  // two arguments: a pointer to the message buffer and the 
                                  // pointer to arguments specified below.
              const char *hndName,// Handler name
              void *arg)          // Pointer to argument structure. This pointer, if not NULL, must
                                  // point to statically allocated memory. When used in the handler code
                                  // it must be properly casted.
                                  // @R: completion code
{
int stat,ret=NO_ERROR;
pthread_attr_t attribute;
hType *hidx;

if(!Initialized || (Connected>0)) return THRD_INITIALIZE_ERROR;
if(hList.nItms>=MAX_HANDLERS) return THRD_TOOMANY_HNDLRS_ERROR;

hList.hl[hList.nItms].msgC=msgC;
hList.hl[hList.nItms].pLoad=pLoad;
strncpy(hList.hl[hList.nItms].from,sender,PROC_NAME_LEN);
hList.hl[hList.nItms].from[PROC_NAME_LEN]='\0';

hList.hl[hList.nItms].handler=handler;
hList.hl[hList.nItms].hndName=strdup(hndName);
hList.hl[hList.nItms].arg=arg;
hList.hl[hList.nItms].nFlushd=0;
hidx=hList.hl+hList.nItms;
hList.nItms++;

                                              // Spawn handler thread
pthread_attr_init(&attribute);
if((stat=pthread_create(&(hidx->bId),&attribute, (void *)&hManager, hidx))!=0) {
    char errM[1024];
    unsigned long myth=pthread_self();
    snprintf(errM,1024,"thrdlib: [t:%0lx] thHandler: pthread_create() returned error: %s",myth,strerror(stat));
    (*thNotify)(MASTER_LOG_LEV_ERROR,errM);
    ret = THREAD_CREATION_ERROR;
}
pthread_attr_destroy(&attribute);     

return ret;
}

//@Function(API): thGetVar
//
// Retrieves from the Real-Time Database a specified variable
//
// This routine sends a request to get the values of a single variable.
// After the call it waits for the replies from \msgd\ and returns a pointer
// to the reply message buffer.
//
// Variable values can then retrieved form the buffer by using: thValue().
//
// After retrieving values the pointer must be released by calling thRelease().
// When the function returns an error condition (i.e.: a NULL pointer), 
// thRelease must not be called!

// Note: the use of thGetVar() is by far more efficient than thReadVar().
//@

MsgBuf *thGetVar(const char *varName, //@P{varName}: Variable name.
                 int   tmout,         //@P{tmout}: Timeout in milliseconds, to wait for reply to request
                                      //           If 0, wait forever. 
                 int*  errc)          //@P{errc}: store here error code
                                      //@R: Handle for variable retrieval
{
   int lng,stat;
   unsigned mcode;
   MsgBuf *msgb=NULL;
   unsigned long myth=pthread_self();
   char destbuf[PROC_NAME_LEN+1];
   char *dest, *dpt;

   *errc=UNMANAGED_ERROR;          // This error should never be returned!

   if(NOT_CONNECTED) {
      *errc = THRD_NOT_CONNECTED_ERROR;
      return NULL;
   }

   lng=strlen(varName);
   if(lng>VAR_NAME_LEN || lng<=0) {
       *errc=VAR_NAME_ERROR;
       return NULL;
   }

   if((dpt=strchr(varName,'@')))
      dest=mkFullName("",++dpt,destbuf);
   else
      dest="";

   if(IS_ERROR(stat=thSendMsg(lng+1,dest,GETVAR,NOHANDLE_FLAG,varName))) {
      *errc=stat;
      return NULL;
   }

   if((msgb=thWaitMsg(ANY_MSG,"*",stat,tmout,errc))) {
      char errM[1024];
      mcode=HDR_CODE(msgb);
      if(mcode==VARREPLY) {
         *errc=NO_ERROR;
         return msgb;
      } 
      if(mcode==VARLOCKD) {
          if(thD1bug) {
              snprintf(errM,1024,"thrdlib: [t:%0lx] thGetVar(%s): VAR_LOCKED_ERROR",myth,varName);
              (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
          }
          bRelease(msgb);
          *errc=VAR_LOCKED_ERROR;
          return NULL;
      }
      if(mcode==VARERROR) {
          if(thD1bug) {
              snprintf(errM,1024,"thrdlib: [t:%0lx] thGetVar(%s): VAR_NOT_FOUND_ERROR",myth,varName);
              (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
          }
          bRelease(msgb);
          *errc=VAR_NOT_FOUND_ERROR;
          return NULL;
      }
   } 
   return NULL;
}

//@Function(API): _readVar
//
// Implementation of thReadVar, thClntsList and thBufsList
//@

static MsgBuf* _readVar(int  code,          // Message code, either: VARLIST, CLNTLIST, BUFLIST
                        const char *prefix, //@P{prefix}: Variable name specification. If terminated
                                            //  with '*' all variables with given prefix are selected,
                                            // otherwise requires exact matching.
                                            // if  code is not VARLIST, it is ignored.
                        const char* dest,   // Destination MsgD.
                        int   tmout,        //@P{tmout}: Timeout in milliseconds, to wait for reply to request
                                            //           (Each variable). 
                                            //           If 0, wait forever. 
                        int*  errc)         //@P{errc}: store here error code
                                            //@R: Handle for variable retrieval
{
int lng,stat,exp_err;
unsigned int exp_ret,exp_end;
MsgBuf *ret=NULL, *last=NULL, *msgb=NULL;
*errc=UNMANAGED_ERROR;          // This error should never be returned!

if(NOT_CONNECTED) {
   *errc = THRD_NOT_CONNECTED_ERROR;
   return NULL;
}

switch(code) {
   case VARLIST:              // Deal with thReadVar request
      if(!prefix) prefix="*";
      exp_ret=VARREPLY;
      exp_end=VAREND;
      exp_err=VLIST_MSG_ERROR;
      lng=strlen(prefix);
      if(lng>VAR_NAME_LEN) {
         *errc = VAR_NAME_ERROR;
         return NULL;
      }
      lng++;     // Allow terminator to be sent
      break;
   case CLNTLIST:            // Deal with thClntsList request
      exp_ret=CLNTRPLY;
      exp_end=CLNTEND;
      exp_err=CLIST_MSG_ERROR;
      lng=0;
      break;
   case BUFLIST:              // Deal with thBufsList request
      exp_ret=BUFREPLY;
      exp_end=BUFEND;
      exp_err=BLIST_MSG_ERROR;
      lng=0;
}

if(IS_ERROR(stat=thSendMsg(lng,dest,code,NOHANDLE_FLAG,prefix))) { // Send Command
   *errc = stat;
   return NULL;
}

               // Now replies are concatenated into a list
while((msgb=thWaitMsg(ANY_MSG,"*",stat,tmout,errc))) {
   unsigned int mcode=HDR_CODE(msgb);
   if(mcode==exp_ret) {
      msgb->next=NULL;
      if(ret==NULL) 
         ret=msgb;
      else
         last->next=msgb;
      last=msgb;
   } else if(mcode==exp_end) {    // Get here when receiving VAREND msg
      *errc=HDR_PLOAD(msgb);
      bRelease(msgb);
      break;
   } else {                   // Unexpected message code
      *errc=exp_err;
      bRelease(msgb);
      bRelease(ret);
      ret=NULL;
      break;
   } 
}
return ret;
}


//@Function(API): thReadVar
//
// Retrieves from the Real-Time Database a set of variables
//
// This routine sends a request to get the values of variables whose names
// match a given prefix.  After the call waits for the replies from \msgd
// and assembles the returned values into a proper structure providing a pointer
// to retrieved values.
//
// Variable values can be managed by using: thValue(), thNextVar()
//
//@

MsgBuf *thReadVar(const char *prefix, //@P{prefix}: Variable name specification. If terminated
                                      //  with '*' all variables with given prefix are selected,
                                      // otherwise requires exact matching.
                                      // To read vars from remote MsgD use: "prefix@name"
                  int   tmout,        //@P{tmout}: Timeout in milliseconds, to wait for reply to request
                                      //           (Each variable). 
                                      //           If 0, wait forever. 
                  int*  errc)         //@P{errc}: store here error code
                                      //@R: Handle for variable retrieval
{
   char destbuf[PROC_NAME_LEN+1];
   char *dest, *dpt;

   if((dpt=strchr(prefix,'@')))
      dest=mkFullName("",++dpt,destbuf);
   else
      dest="";

   return _readVar(VARLIST,prefix,dest,tmout,errc);
}


//@Function{API}: thWriteVar
//
// Write a variable. Create the variable if not existent. Optionally verify completion
//
// This routine sends a request to write a variable in the real-time database.
// then waits for a completion code. If the variable is not defined, it is
// created.
//
// When a variable is created as result of te call, its access mode is public.
// If the variable is intended to be private, a call to VarOp(..,TAKEVAR,..) must follow.
//
// Note: if the value of timeout is negative, the message code SETVAR is used
// and no error checking is performed. This means that if the variable is not
// yet defined into the database (or is existing and not writable) the
// operation will silently fail.
//
//@

int thWriteVar(const char *varName, //@P{varName}: Variable name
               int   VarType,       //@P{VarType}: Variable type (as defined in rtdblib.h)
               int  NElements,      //@P{NElements}: Variable length (number of elements).
                                    // String variables are padded with a terminating NUL. So an
                                    // empty string is sent a a single NUL byte and length 0.
                                    // A variable of zero length is legal and is considered of
                                    // undefined value.
               const void *Value,   //@P{Value}: Pointer to variable value(s). If NULL, the
                                    // variable is zero-filled
               int tmout)           //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                    //           If 0, wait forever. If -1, do not request acknowledgement.
                                    // @R: Completion code.
{
MsgBuf *msgb;
int stat;
unsigned long myth=pthread_self();
char errM[1024];

msgb=bGet();
if(!msgb) return NO_BUFFER_ERROR;
if(tmout<0) {
    stat = VarCmd(msgQ.myname,SETVAR,varName,VarType,NElements,Value,msgb);
    if(thD1bug) {
        snprintf(errM,1024,"thrdlib: [t:%0lx] thWriteVar(%s): SETVAR command sent",myth,varName);
        (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
    }
    bRelease(msgb);
    if(stat>0) return NO_ERROR;
} else {
    SetFlag(NOHANDLE_FLAG,msgb);   // MsgD will send back an acknowledgement
    stat = VarCmd(msgQ.myname,WRITEVAR,varName,VarType,NElements,Value,msgb);
    if(thD1bug) {
        snprintf(errM,1024,"thrdlib: [t:%0lx] thWriteVar(%s): WRITEVAR msg (#%d:%lX) sent",myth,varName,msgb->bId,(unsigned long)MSG_BODY(msgb));
        (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
    }
    bRelease(msgb);

    if(stat<0) return stat;
    if((msgb=thWaitMsg(ANY_MSG,"*",stat,tmout,&stat))) {
       int mcode=HDR_CODE(msgb);
       if(mcode==VARWRITN) {
          stat=NO_ERROR;
       } else {
          if(thD1bug) {
              snprintf(errM,1024,"thrdlib: [t:%0lx] thWriteVar(%s): RTDB returned status NOT_OWNER_ERROR",myth,varName);
              (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
          }
          stat=NOT_OWNER_ERROR;
       }
       bRelease(msgb);
    }
} 
return stat;
}

int thVarOp(const char *varName, //@P{varName}: Variable name. It may be a prefix terminating
                                 //             with '*'
            int   VarOp,         //@P{VarOp}:   Operation specifier. Operations permitted: 
                                 //             DELVAR: delete a variable
                                 //             DUMPVARS: Send a DumpVar request (selected variables
                                 //             are written to MsgD log stream).
                                 //             LOCKVAR: lock/unlock given variables; 
                                 //             TRACEVAR: set/unset variable trace flag
                                 //             VARNOTIF: register/unregister for variable change notification; 
            int   scode,         //@P{scode}: Operation sub code (currently used by LOCKVAR, TRACEVAR, VARNOTIF)
            int   tmout)         //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                 //           If 0, wait forever. If -1, do not request acknowledgement.
                                 // @R: Completion code.
{
   int lng,stat;
   MsgBuf *msgb;
   unsigned long myth=pthread_self();
   char errM[1024];
   char destbuf[PROC_NAME_LEN+1];
   char *dest, *dpt;

   lng=strlen(varName)+1;
   if(lng>VAR_NAME_LEN) return VAR_NAME_ERROR;

   if((dpt=strchr(varName,'@')))
      dest=mkFullName("",++dpt,destbuf);
   else
      dest="";


   switch(VarOp) {        // The following operations do not
                          // get an answer from MsgDRTDB.
       case TAKEVAR:     // Obsolete code
       case GIVEVAR:     // Obsolete code
          return NO_ERROR;
       case DUMPVARS:
           stat=thSendMsg(lng,dest,VarOp,0,varName);
           break;
       case VARNOTIF:
       case TRACEVAR:
           stat=thSendMsgPl(lng,dest,VarOp,scode,0,varName);
           break;
                       // The following operations will be
                       // answered by MsgDRTDB
       case DELVAR:
            scode=0;
       case LOCKVAR:
           if(tmout>=0) {           // We want to check the reply
              stat=thSendMsgPl(lng,dest,VarOp,scode,NOHANDLE_FLAG,varName);
              if(IS_ERROR(stat)) {
                  if(thD1bug) {
                      snprintf(errM,1024,"thrdlib: [t:%0lx] thVarOp(%s): thSendMSG returned status %d",myth,varName,stat);
                      (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
                  }
                  return stat;
              }
              if((msgb=thWaitMsg(ANY_MSG,"*",stat,tmout,&stat))) {
                  int mcode=HDR_CODE(msgb);
                  if(mcode == NAK) {
                      if(thD1bug) {
                          snprintf(errM,1024,"thrdlib: [t:%0lx] thVarOp: RTDB replied NAK",myth);
                          (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
                      }
                      if(VarOp==DELVAR)
                          stat=VAR_DEL_ERROR;
                      else
                          stat=VAR_LOCK_ERROR;
                  }
              }
           } else {                   // We do not want to check the reply
              stat=thSendMsgPl(lng,dest,VarOp,scode,DISCARD_FLAG,varName);
              if(IS_ERROR(stat)) {
                  if(thD1bug) {
                      snprintf(errM,1024,"thrdlib: [t:%0lx] thVarOp(%s): thSendMSG returned status %d",myth,varName,stat);
                      (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
                  }
                  return stat;
              }
           }
           break;
       default: stat=ILLEGAL_MSG_ERROR;
   }
   return stat;
}

//@Function(API): thClList
//
// Request the list of clients currently connected to some MsgD-RTDB
//
// This routine sends a request to get the list of clients currently
// connected to some MsgD-RTDB.
//
// After the call waits for the replies from \msgd and assembles the returned 
// values into a proper structure providing a pointer to retrieved values.
//
// Returned items are actually Integer variables whose names are the Client's names
// and whose value is the Client Id number.
//
// Variable values can thus be managed by using: thValue(), thNextVar().
//
// NOTE: this procedure substitutes thClntsList
//@

MsgBuf *thClList(int   tmout, //@P{tmout}: Timeout in milliseconds, to wait for reply to request
                              //           If 0, wait forever. 
                 const char* dest,  //@P{dest}: Destination MsgD. Use "" for local MsgD, use: "@name"
                              //          for remote MsgD.
                 int*  errc)  //@P{errc}: store here error code
                              //@R: Handle for variable retrieval
{
   return _readVar(CLNTLIST,NULL,dest,tmout,errc);
}


//@Function(API): thClntsList
//
// Request the list of clients currently connected to MsgD-RTDB
//
// This routine is OBSOLETE. Use thClList() instead.
//
//@

MsgBuf *thClntsList( int   tmout, //@P{tmout}: Timeout in milliseconds, to wait for reply to request
                                  //           If 0, wait forever. 
                     int*  errc)  //@P{errc}: store here error code
                                  //@R: Handle for variable retrieval
{
   return _readVar(CLNTLIST,NULL,"",tmout,errc);
}



//@Function(API): thBfList
//
// Request the list of buffers currently defined in some MsgD-RTDB
//
// This routine sends a request to get the list of buffers currently
// defined in some MsgD-RTDB.
//
// After the call waits for the replies from \msgd and assembles the returned 
// values into a proper structure providing a pointer to retrieved values.
//
// Returned items are actually Character variables whose names are the Buffers names
// and whose value is properly formatted information about buffers
//
// Variable values can be thus managed by using: thValue(), thNextVar().
//@

MsgBuf *thBfList(int         tmout, //@P{tmout}: Timeout in milliseconds, to wait for reply to request
                                    //           If 0, wait forever. 
                 const char* dest,  //@P{dest}: Destination MsgD. Use "" for local MsgD, use: "@name"
                                    //          for remote MsgD.
                 int*        errc)  //@P{errc}: store here error code
                                    //@R: Handle for variable retrieval
{
   return _readVar(BUFLIST,NULL,dest,tmout,errc);
}



//@Function(API): thBufsList
//
// Request the list of buffers currently defined in the MsgD-RTDB
//
// This routine is OBSOLETE. Use thBfList() instead.
//
//@

MsgBuf *thBufsList( int   tmout, //@P{tmout}: Timeout in milliseconds, to wait for reply to request
                                  //           If 0, wait forever. 
                     int*  errc)  //@P{errc}: store here error code
                                  //@R: Handle for variable retrieval
{
   return _readVar(BUFLIST,NULL,"",tmout,errc);
}

//@Procedure(API): thDetach

// Detaches the message body from a message buffer
//
// This routine will release the message buffer (as thRelease()) after
// "detaching" the message. The calling program may then continue
// to use the memory associated with the message after the detaching.
//
// Message data can be accessed via the macros:
// DTC_TO(x), DTC_FROM(x), DTC_SEQN(x), DTC_LEN(x), DTC_CODE(x),
// DTC_PLOAD(x), DTC_BODY(x)
//
// The routine returns a pointer to the message body. The calling
// program must provide deallocation by calling free() on that pointer
// when the memory buffer is not used any longer.
// The returned pointer may be null (if message body is empty)
//
// If the message buffer is a list of buffer, the message body of first 
// message is detached, all other messages are released.
//@

/*
Message * thDetach(MsgBuf *msgb)
{
Message* ret=NULL;

if(msgb) {
   ret=MSG_BODY(msgb);
   MSG_BODY(msgb)=NULL;
   msgb->buflen=0;
   thRelease(msgb);
}
return ret;
}
*/

//@Procedure(API): thRelease

// Releases a message buffer
//
// This routine must be used to return a message buffer to the buffer pool
// after use.

// Note 1: some routines will automatically release the message buffer, so
// you must check the routine documentation to verify wether an explicit
// release is actually needed.

// Note 2: It is safe to call thRelease() on a NULL pointer

//@


void thRelease(MsgBuf *msgb)
{
MsgBuf *next=msgb;
while(msgb) {
    next=msgb->next;
    bRelease(msgb);
    msgb=next;
}
}

//@Function{API}: thWaitReady

// Wait for specified client to become ready

// This function waits for a specified client to set its ready flag.
// The routine will return with NO_ERROR state when the other client's
// notification is received.

// Note: The specified client must be "well behaved" , i.e.: if it
// doesn't call thSendMsg() with code SETREADY, the waiting client will 
// never be notified. 

// When timeout is expired the routine returns with a TIMEOUT_ERROR code
//@

int thWaitReady(const char *cname, // @P{cname}: Name of client to wait for.
                int  tmout)        // @P{tmout}: Timeout (millisec).
                                   // @R: status code.
{
MsgBuf *msgb;
int stat;
int lng=strlen(cname);

if(lng>PROC_NAME_LEN || lng<=0) return CLIENT_NAME_ERROR;

if(strchr(cname,'@')) return NO_REMOTE_ERROR;

if(IS_ERROR((stat=thSendMsg(lng+1,"",WAITCLRDY,NOHANDLE_FLAG,cname)))) return stat;


if((msgb=thWaitMsg(CLNREADY,"*",0,tmout,&stat))) {
    bRelease(msgb);
    stat=NO_ERROR;
}
return stat;
}



//@Function{API}: thValue

// Extract variable from message buffer

// This function returns pointer to variable data into a message buffer
// returned by thReadVar().

//@

Variable *thValue(MsgBuf *msgb)  //@P{msgb}: Message buffer
{
return (Variable *)MSG_BODY(msgb);
}

//@Function{API}: thNextVar

// This function can be used to retrieve variables from a message buffer
// list returned by thReadVar().

// At each call the function returns a pointer to next message buffer.
// Variable data can be extracted from each message buffer with thValue().

// Note: Each call to thNextVar() also release the previous message buffer.
// If not all message buffer are used by the calling program
// the remaining buffers in the chain must be released by calling thRelease().

// Here follows a typical use example:

// \code{ 
// msgbv = thReadVar( ...... );
// while(msgbv) {
//    var=thValue(msgbv);
//          /* ... use variable data from structure var */
//    msgbv=thNextVar(msgbv);
// }
//@

MsgBuf *thNextVar(MsgBuf *msgb)   //@P{msgb}: Message buffer pointer returned
                                  //          by thReadVar()
                                  //@R: next message buffer (NULL at end)
{
MsgBuf *next=NULL;

if(msgb) {
    next=msgb->next;
    bRelease(msgb);
}  
return next;
}

//@Function{API}: thHaveYou

// Ask MsgD if a client is connected

// This function asks MsgD if a specified client is currently conencted.

//@

int thHaveYou(const char *cname) // @P{cname}: Name of client to search for.
                                 // Use name@dom to serch for remote clients
                                 // @R: 1/0 if client found/not found. Or
                                 // negative error code.
{
   MsgBuf *msgb;
   int stat,seqn;
   char nbuf[PROC_NAME_LEN+1];
   char *dom=getDomain(cname,nbuf);
   int lng;

   if(!dom) return CLIENT_NAME_ERROR;

   lng=strlen(nbuf);

   if(IS_ERROR((seqn=thSendMsg(lng+1,dom,HAVEYOUCL,NOHANDLE_FLAG,nbuf)))) return seqn;

   if((msgb=thWaitMsg(ACK,"*",seqn,1000,&stat))) {
      stat=HDR_PLOAD(msgb);
      bRelease(msgb);
   }

   return stat;
}


//@Function{API}: thDefTTL

// Changes the application default message Time to Live value
//
// This call changes the defualt TTL value assigned to messages in the
// message queue. A different TTL value can be assigned for each message
// using the related argument in function @see{thSendMessage()}.
//
//@

int thDefTTL(int ttl)   //@P{ttl}: Message time to live (>0)
{
char errM[1024];

if(ttl<=0) return THRD_ILLEGAL_TTL;
thDefaultTTL=ttl;
if(thD1bug) {
    snprintf(errM,1024,"thrdlib: thDefTTL: default TTL se to %d",ttl);
    (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
}
return NO_ERROR;
}
   

//@Function{API}: thFlush

// Clean message queue discarding selected waiting messages
//
// This routine looks in the input message queue for messages matching
// specified selection criteria and removes matching messages from the
// message queue
//@

int thFlush(int   msgC,         // @P{msgC}: Wait for specific message code
                                // The special code "ANY_MSG" may be used to match
                                // any message code.
            const char *sender, // Specify sender of message. The NUL string ""
                                // matches messages originating from the \msgd.
                                // The string "*" matches any sender
            int   seqN)         // @P{seqN}: specify sequence number.
                                // if 0, any sequence number will match
                                // @R: Number of messages removed from the queue
{
MsgBuf *ret;
int nremvd=0;
unsigned long myth=pthread_self();
char errM[1024];

if(thD1bug) {
    snprintf(errM,1024,"thrdlib: [t:%0lx] thFlush: flushing message queue",myth);
    (*thNotify)(MASTER_LOG_LEV_VTRACE,errM);
}

				// Scan message queue
pthread_mutex_lock(&msgQ.q_mutex);
while(ret) {
    ret=qScan(msgC,sender,seqN);
    if(!ret) break;
    bRel(ret);
    nremvd++;
}
pthread_mutex_unlock(&msgQ.q_mutex);
return nremvd;
}


//@Function{API}: thInfo

// Returns information about status of message queue

// This routine may be used to retrieve information about the current
// message buffer status. It is mainly useful for debugging purposes.
// 
//@

qInfo *thInfo(void)   //@R: returns pointer to static memory containing
                      //    message queus status info
{
myI.initialized=Initialized;
myI.connected = (IS_CONNECTED) ? 1:0;

if(Initialized) {
    pthread_mutex_lock(&msgQ.q_mutex);
    pthread_mutex_lock(&pooQ.q_mutex); 
}
myI.nWaiting=msgQ.nItms;     // Number of messages waiting in input queue
myI.nFlushd=msgQ.nFlushd;    // Number of messages flushed since previous call
msgQ.nFlushd=0;
myI.nHandlers=hList.nItms;
myI.nFree=pooQ.nItms;        // Number of free buffers
myI.nTot=pooQ.iL;
if(Initialized) {
    pthread_mutex_unlock(&pooQ.q_mutex); 
    pthread_mutex_unlock(&msgQ.q_mutex);
}
myI.DbgLevel=thDeblev;
myI.DefTTL=thDefaultTTL;
  
strncpy(myI.myname,msgQ.myname,PROC_NAME_LEN);
myI.myname[PROC_NAME_LEN]='\0';
return &myI;
}

//@Function{API}: thDebug

// Set the Debug level

// This procedure modifies the debug level value. Usually debug
// level is set to 0. You may want to set it to 1 to show message queue
// related debug or to 2 in order to add buffer management related info
//@

int thDebug(int level)   //@P{level}: logging level. Use the following values:
                         //           MASTER_LOG_LEV_ERROR:   displays only errors
                         //           MASTER_LOG_LEV_VTRACE:  displays debug messages from library
                         //           MASTER_LOG_LEV_VVTRACE: displays trace messages from library
                         //@R: assigned logging level 
{
   if(level<MASTER_LOG_LEV_INFO) level=MASTER_LOG_LEV_INFO;
   if(level>MASTER_LOG_LEV_VVTRACE) level=MASTER_LOG_LEV_VVTRACE;
   switch(level) {
      default:
        thD1bug=0;
        thD2bug=0;
        break;
      case MASTER_LOG_LEV_VTRACE:
        thD1bug=1;
        thD2bug=0;
        break;
      case MASTER_LOG_LEV_VVTRACE:
        thD1bug=1;
        thD2bug=2;
        break;
   }
   return level;
}


//@Function{API}: thLogMsg

// Request the MsgD to log a message

// This function sends a log message to the MsgD-RTDB to be properly
// logged. 



//@

int thLogMsg(int level,      // @P{level}: Log Level. Values are defined in errlib.h
                             // Values accepted are MASTER_LOG_LEV_INFO, MASTER_LOG_LEV_WARNING,
                             // MASTER_LOG_LEV_ERROR, MASTER_LOG_LEV_FATAL,
                             // Message with different log levels are silently dropped
            int  seqn,       // Message sequence number. May be used to correlate
                             // msgd log file with application specific one. Use the value
                             // returned by a previous call to logString()
            const char *msg) // @P{msg}: Message string

{
    int stat,size;
    str_LOGSTAT mstruct;

    if(level>MASTER_LOG_LEV_INFO || level<MASTER_LOG_LEV_FATAL) return NO_ERROR;

    if(msg==NULL) return UNSPECIFIED_LOG_MSG_ERROR;
    if(*msg=='\0') return UNSPECIFIED_LOG_MSG_ERROR;
    if(NOT_CONNECTED) return THRD_NOT_CONNECTED_ERROR;

    mstruct.txt[LOG_MAX]='\0';
    size=0;

    strncpy(mstruct.txt,msg,LOG_MAX);
    size=2*sizeof(int)+strlen(mstruct.txt)+1;
    mstruct.lev=level;
    mstruct.seq=seqn;

    if(IS_ERROR(stat=thSendMsgPl(size, "", LOGSTAT, 0, NOHANDLE_FLAG, (void *)&mstruct)))
        return stat;

    return NO_ERROR;
}



//@Function{API}: thNewLog

// Request the MsgD to close current log file and reopen a new one

// This function sends a log message to the MsgD-RTDB to request
// that current log file is closed (and saved) a a new one with the
// same name is opened.

//@

int thNewLog(void)
{
return thSendMsg(0, "", CLOSELOG, DISCARD_FLAG, NULL);
}


//@Function{API}: thPing

// Ping a client and wait answer

// This function pings a specified client and waits for an answer.
// The routine will return with NO_ERROR state when the other client's
// answer is received.

// Use "" in cname to ping MsgD
//@

int thPing(const char *cname, // @P{cname}: Name of client to ping.
           int  tmout,        // @P{tmout}: Timeout (millisec).
           double *delay)     // @P{tmout}: on success return round trip time.
                              //    if NULL it is ignored.
                              // @R: status code.
{
    int stat;
    int ret=NO_ERROR;
    MsgBuf *msgb=NULL;
    struct timeval before,after;
    double dsec;
    long nsec,nusec;

    if(NOT_CONNECTED) return THRD_NOT_CONNECTED_ERROR;

    if(IS_ERROR(stat=thSendMsg(0, cname, ECHO, NOHANDLE_FLAG, NULL)))
        return stat;

    gettimeofday(&before,NULL);         // Get time before

    if((msgb=thWaitMsg(ANY_MSG,cname,stat,tmout,&ret))) {
        gettimeofday(&after,NULL);         // Get time before
        bRelease(msgb);

        nsec=after.tv_sec-before.tv_sec;    // Compute delay
        nusec=after.tv_usec-before.tv_usec;
        dsec = (double)nsec+(double)nusec*0.000001;

        if(delay) *delay=dsec;

        return NO_ERROR;
    }
    return ret;
}



//@Function{API}: thErrNotify

// Install a user provided function for error notification

// This function installs a user provided function which is called
// when an asynchronous error is detected. Such errors may occurr
// in the message queue manager.
//
// The notify function is called as:
//
//    notify(int level, char *msg)
//
//@

void thErrNotify(void (*notifier)(int , char *))
{
   if(notifier) thNotify=notifier;
}


//@Function{API}: thVersion

// Sends a GetVersion request to client and returns numeric version

// Numeric version is returned as: 1000*VERS_MAJOR+VERS_MINOR

// Use "" in cname to get version from MsgD.
//@

int thVersion(const char *cname) // @P{cname}: Name of client.
                                 // @R: <0 error code.
                                 //     >=0 version number
{
   int stat;
   int ret=NO_ERROR;
   MsgBuf *msgb=NULL;

   if(NOT_CONNECTED) return THRD_NOT_CONNECTED_ERROR;

   if(IS_ERROR(stat=thSendMsg(0,cname,GETVERSION,NOHANDLE_FLAG,NULL)))
      return stat;

   if((msgb=thWaitMsg(VERSREPLY,cname,stat,1500,&ret))) {
      stat=HDR_PLOAD(msgb);
      bRelease(msgb);
   } else
      stat=ret;
    return stat;
}


//@Function{API}: thGetStartT

// Get client's start time from MsgD

// This function requests the start time of a given client. 
// The return value is stored in a buffer provided by the caller
// as a struct timeval value (see: gettimeofday function for details)

// If the client is not currently connected or in case of communication errors
// a the buffer is kept unaltered and the return code is set accordingly.

//@

int thGetStartT(const char *cname,   // @P{cname}: Name of client.
                struct timeval *tv)  // @P{tv}: Buffer for returned time
                                     // @R: status code.
{
MsgBuf *msgb;
int stat;
char name[LONG_NAME_LEN+1];
char msgd[LONG_NAME_LEN+1];

strncpy(name, cname, LONG_NAME_LEN);
name[LONG_NAME_LEN] = 0;

char * domain = strchr(name, '@');

if (domain != NULL) {
    snprintf(msgd, LONG_NAME_LEN, "%s", domain);
    *domain = 0; // strip domain out from name
}
else {
    msgd[0] = 0;
}

int lng=strlen(name);

if(lng>LONG_NAME_LEN || lng<0) return CLIENT_NAME_ERROR;

if(IS_ERROR((stat=thSendMsg(lng+1,msgd,GETSTARTT,NOHANDLE_FLAG,name)))) return stat;

if((msgb=thWaitMsg(STARTREPLY,"*",0,500,&stat))) {
   if(HDR_PLOAD(msgb)==0)
      stat=NO_CLIENT_ERROR;
   else {
      memcpy(tv,MSG_BODY(msgb),sizeof(struct timeval));
      bRelease(msgb);
      stat=NO_ERROR;
   }
}
return stat;
}


