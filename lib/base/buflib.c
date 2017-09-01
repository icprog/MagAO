//@File: buflib.c
//
// Shared buffer library for MsgD-RTDB:
//
//@


#include <stdlib.h>
#include <string.h>		// memcpy()
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>		// semop() semtimedop()
#include <sys/shm.h>		// shmat()

#ifdef  THRDLIB
#include "base/thrdlib.h"
#else
#include "base/msglib.h"
#endif

#include "base/rtdblib.h"
#include "base/errlib.h"

#include "base/buflib.h"

//@C

// Part of the library code has been used to implement functions in {\tt thrdlib}
// the threading library developed for \msgd\  based applications. The
// preprocessor symbol {\tt THRDLIB} has been used to condittionally compile
// parts of codes for the two version of the library.

// In the make process the library is thus compiled twice (see the related
// {\tt makefile}
//@

#ifndef THRDLIB
static int msg_seqNum=0;
#endif

// the following to be used as array field in "union semun" argument to semctl()

static unsigned short writeunlock_bufsnap[]=BUFSNAP_SEM_WRITE_UNLOCK;
static unsigned short writeunlock_bufcont[]=BUFCONT_SEM_WRITE_UNLOCK;
                             
static unsigned short setsem_bufcont[] = BUFCONT_SEM_INIT;
static unsigned short setsem_bufsnap[] = BUFSNAP_SEM_INIT;


// The following to be used as struct sembuf argument to semop()

static struct sembuf writelock_bufcont[]=BUFCONT_SEM_WRITE_LOCK;
static struct sembuf readlock_bufcont[]=BUFCONT_SEM_READ_LOCK;
static struct sembuf readunlock_bufcont[]=BUFCONT_SEM_READ_UNLOCK;

static struct sembuf writelock_bufsnap[]=BUFSNAP_SEM_WRITE_LOCK;
static struct sembuf readlock_bufsnap[]=BUFSNAP_SEM_READ_LOCK;
static struct sembuf readunlock_bufsnap[]=BUFSNAP_SEM_READ_UNLOCK;

//@Function[API]: bufCreate
//
// Requests MsgD to create a block of shared memory and related support structure

// Returns a BufInfo structure pointer to be used with bufWrite calls, or NULL
// on errors. If null is returned a call to bufErrno() will provide an error code

// bufCreate() sends a message to the MsgD-RTDB, who creates the buffer and 
// sends back a reply containing a shared memory identifier. 

// The buffer just created will be identified by a buffer name which has the
// following syntax: "producer:bufname", where "bufname" is the name
// assigned by the producer when the buffer was created.

// The function will then attach to the shared memory and will create and fill
// the BufInfo structure wich will be passed back to the caller.

//@ 

int bufCreate(const char *cname,   // @P{cname}: name of client (producer)
                                   //                 creating buffer
              const char *bufname, // @P{bufname}: name of buffer to be created
              int accmode,         // @P{accmode}: access mode (@see{BUFCONT} or @see{BUFSNAP})
              int lng,             // @P{len}: buffer len in bytes
              int maxCS,           // @P{maxCS}: in continuous mode, maximum 
                                   //            number of concurrent consumers.
                                   //            In snapshot mode, maximum number 
                                   //            of buffers which can be requested.
              BufInfo *info)       // @P{info}:  Structure to be filled in
                                   // with shared buffer management data.
                                   // @R: status code
{
if(info==NULL) return PLAIN_ERROR(NOT_INIT_ERROR);
	
int stat;
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
} setsem;

#ifdef THRDLIB
info->loc.msgb=NULL;
#else
info->loc.msgb.Msg=NULL;
#endif

if(accmode==BUFCONT) {           // initialize semaphores operations
    info->loc.locker.sbuf=writelock_bufcont;
    info->loc.unlocker.ushort=writeunlock_bufcont;
    info->com.maxC=maxCS;
    info->com.maxSnap=0;
    setsem.array=setsem_bufcont;
} else {
    info->loc.locker.sbuf=writelock_bufsnap;
    info->loc.unlocker.ushort=writeunlock_bufsnap;
    info->com.maxC=1;
    info->com.maxSnap=maxCS;
    setsem.array=setsem_bufsnap;
}

strncpy(info->com.name,bufname,VAR_NAME_LEN);
info->com.name[VAR_NAME_LEN]='\0';
strncpy(info->com.producer,cname,PROC_NAME_LEN);
info->com.producer[PROC_NAME_LEN]='\0';
info->com.accMode=accmode | PRODUCER;
info->com.lng=lng;
info->com._lng=0;

	// Request shared memory creation to MsgD-RTDB and wait for an answer
#ifdef THRDLIB

stat=thSendMsg(sizeof(BufCom), "", SHMCREA, NOHANDLE_FLAG,&(info->com));
if(IS_ERROR(stat)) return stat;
info->loc.msgb=thWaitMsg(SHMCREA,"",stat,MSG_TIMEOUT,&stat);
if(!info->loc.msgb) return stat;
if(IS_ERROR(stat=HDR_PLOAD(info->loc.msgb))) {   // Check if reply is OK
   thRelease(info->loc.msgb);
   return stat;
}
memcpy(&(info->com), MSG_BODY(info->loc.msgb), sizeof(BufCom));
thRelease(info->loc.msgb);

#else

msg_seqNum=NewSeqN();
InitMsgBuf(&(info->loc.msgb),sizeof(BufCom));
FillHeader(0, msg_seqNum ,cname,"",SHMCREA, &(info->loc.msgb));
FillBody(&(info->com), sizeof(BufCom), &(info->loc.msgb));
SendToServer(&(info->loc.msgb));

stat=WaitMessageSet(&msg_seqNum, 1, &(info->loc.msgb), NULL, MSG_TIMEOUT);
if(IS_ERROR(stat)) return stat;
if(IS_ERROR(stat=hdr_PLOAD(info->loc.msgb))) return stat;   // Check if reply is OK
memcpy(&(info->com), msg_BODY(info->loc.msgb), sizeof(BufCom));

#endif

                                         // Get and initialize semaphore

info->loc.sem_id = semget(info->com.sem_key, info->com.maxC+1,0);
if(info->loc.sem_id== (-1)) return SYSTEM_ERROR(SEM_GET_ERROR);

if(semctl(info->loc.sem_id,info->com.maxC+1,SETALL,setsem)<0)
    return SYSTEM_ERROR(SHM_CREATE_ERROR);

                                        // Get and initialize shared memory

info->loc.shm_id = shmget(info->com.shm_key, info->com._lng,0);
if(info->loc.shm_id== (-1)) return SYSTEM_ERROR(SHM_GET_ERROR);
info->loc.b=(SharedBuffer *)shmat(info->loc.shm_id,NULL,0);

if(info->loc.b ==NULL) stat=SYSTEM_ERROR(SHM_ATTACH_ERROR);

return stat;
}

//@Function[API]: bufWaitSnap
//
// Waits for an SHMSNAP request

// A producer in snapshot mode must wait for an SHMSNAP request before
// starting to write to the shared buffer.

// The request, is sent by the attached consumer by calling bufSnap()
//@

int bufWaitSnap(BufInfo *info,  //@P{info}: shared buffer pointer
                int *start,     //@P{start}: Index of starting buffer.
                                //           This value is passed as is to the producer
                                //           which may use it to select data.
                int *numb,      //@P{number}: Numbers of buffers.
                                //    Note: After receiving the request, the producer
                                //    must call bufWite() "number" times to write data.
                MsgProc cback)  //@P{cback}: Callback function to process
                                //           other messages. If NULL, Messages 
                                //           different from SHMSNAP will be
                                //           ignored.
{
if(info==NULL) return PLAIN_ERROR(NOT_INIT_ERROR);
	
BufSnapReq *data;
int stat;

#ifdef THRDLIB

info->loc.msgb=thWaitMsg(SHMSNAP,"",0,0,&stat);
if(!info->loc.msgb) return stat;
data = (BufSnapReq *)(MSG_BODY(info->loc.msgb));
if(data->uniq != info->com.uniq) 
    stat = BUF_NOMATCH_ERROR;
else {
    *start=data->start;
    *numb=data->numb;
    stat = NO_ERROR;
}
thRelease(info->loc.msgb);

#else

if(msg_BODY(info->loc.msgb)==NULL)      // Check if message buffer has been initialized
    return SYSTEM_ERROR(MSGB_UNINITIALIZED_ERROR);

for(;;) {
    if((stat=WaitMsgFromServer(&(info->loc.msgb)))!=NO_ERROR) return stat;

    if(hdr_CODE(info->loc.msgb)==SHMSNAP) {
        data = (BufSnapReq *)msg_BODY(info->loc.msgb);
        if(data->uniq != info->com.uniq) 
            stat= BUF_NOMATCH_ERROR;
        else {
            *start=data->start;
            *numb=data->numb;
            stat=NO_ERROR;
        }
        break;
    } else {
        if(cback!=NULL) {
            stat=cback(&(info->loc.msgb));
            if(stat!=NO_ERROR) break;
        }
    }
}
#endif
return stat;
}



//@Function[API]: bufWrite
//
// Writes into a shared buffer. 

// This function will be used only by a producer process.
// Data are copied from the <data> argument and the buffer can be
// reutilized as soon as the function returns.
// <counter> must be an ever-increasing counter, specifying the
// frame number to which the data refers.
// Since the function may block for snapshot-type buffers, a timeout
// value in milliseconds can be specified.
//@
int bufWrite(BufInfo *info,    // P{info}: buffer specifier
             void *data,       // P{data}: data to be written
             int counter,      // P{counter}: buffer sequence counter
                               // Must be increased before each write
                               // It MUST be a non negative integer.
                               // If a negative value is passed, it is
                               // reset to 0, but the calling program 
                               // should avoid negative values because this 
                               // spoils the monotonically increasing
                               // sequence of counters. Counter overflow shouldn't
                               // be a problem: a 4 byte integer increased at
                               // 1000 Hz rate would overflow after 24 days.
             int timeout)      // P{timeout}: Timeout (ms). If 0, wait forever. 
                               // NOTE: The write operation can block only for
                               // SNAPSHOT access. Thus in BUFCONT mode this
                               // argument is useless.
{
//@C
// Internals:

// The buffer is protected by a set of semaphores: one for each consumer
// and one for the producer. The producer's semaphore is the first of the
// set (index 0). Semaphores are used differently according to the buffer
// access mode.
//
// BUFCONT Access mode.

// In continuous mode a single producer will write continuously data to the
// shared buffer. Several consumer may access the buffer at random getting
// the most recent data. When a consumer has read out a copy of the data
// it can immediately issue a new request, because the synchronization
// mechanism will ensure that it will be allowed to read again only as soon
// as the producer has written fresh data to the buffer. Notice that this will
// not ensure that a consumer will receive all data produced.

// The producer semaphore is used to make producer write
// operations mutually exclusive with consumer read operations, while
// allowing multiple consumers to read concurrently. The consumers semaphores
// are used by the producer to signal "new buffer available" so that
// a consumer will not reread already read data when looping.
//
// At the beginning semaphores are initialized at values: 
// MAX\_CONSUMERS, 1,1,1, ...

// The write operation uses a single lock on the first semaphore, the
// semop() function is called with value (-MAX\_CONSUMERS) which sets the 
// value at 0. 
// After writing the producer sets the semaphores at values: 
// MAX\_CONSUMERS, 0,0,0,...

// For read operations each consumer uses the producer semaphore and its
// own read semaphore calling semop() with values: (-1, 0), this decreases
// the producer semaphore value, while waiting for a 0 value for its own
// semaphore.
// After reading the consumer increases by 1 the value of producer semaphore and sets
// its own semaphore at 1.
// 
// All this means that the producer can write only when no consumer is reading
// (producer semaphore value is MAX\_CONSUMERS). Each consumer can read if
// the producer is not writing (producer semaphore greater than zero) and
// its own semaphore is 0 (buffer not already read).

// BUFSNAP Access mode

// In "snapshot" mode the producer is serving a single consumer at a time. Moreover
// data access synchronization will allow the consumer to read access the buffer only
// when the producer has filled it with data and, conversely, will allow the producer
// to write data only as soon as the consumer has got the previous buffer. This will
// ensure that no data is lost.

// Synchronization is managed by two semaphores in a set 

// The sequence of operations is shown in the following table:

//\begin{verbatim}
//                      Semaphore    Semaphore
// Status                 value      operation
// buffer empty           1   1
// lock for write                    -1  -1  semop
// writing to memory      0   0 
// unlock after write                 1   0  semctl
// data ready             1   0
// lock for read                     -1   0  semop
// reading from memory    0   0
// unlock after read                  1   1  semop
// buffer empty           1   1
//\end{verbatim}

// At the beginning semaphores are initialized at values: [1,1] corresponding to
// status: "buffer empty"

// {\bf Note: } because the synchronization mechanism will block either the producer
//     or the consumer (depending on relative speed), this mode of access is actually
//     useful only for very simple programs, i.e.: when the only putrpose of the
// program is to send (or receive) data. Otherwise porograms using this feature will
// need to be threaded in order to be able to do something useful while the shared buffer
// communcation is waiting for buffer access.

//@

if(info==NULL) return PLAIN_ERROR(NOT_INIT_ERROR);

SharedBuffer  *buf=info->loc.b;
int sem=info->loc.sem_id;
int lng=info->com.lng;
int i,nsem;
struct timespec *tmout;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
} setsem;

if((info->com.accMode&ACCMODE_TYPE_MASK)!=PRODUCER) 
    return PLAIN_ERROR(BUF_TYPE_ERROR);

setsem.array=info->loc.unlocker.ushort;
if(counter<0) counter=0;

tmout=NULL;
if(info->com.accMode==BUFSNAP) {
    if(timeout>0) {
        struct timespec delay;                    // Compute timeout
        delay.tv_sec=timeout/1000;
        delay.tv_nsec=(timeout%1000)*1000000;
        tmout=&delay;
    }
    nsem=2;
} else  nsem=1;

if(semtimedop(sem,info->loc.locker.sbuf,nsem,tmout) == -1) { // Lock buffer for write
    if(errno==EAGAIN) return PLAIN_ERROR(TIMEOUT_ERROR);
    return SYSTEM_ERROR(SEM_LOCK_ERROR);
}

for(i=0;i<info->com.maxC;i++)
    buf->counter[i]=counter;        // store buffer counter

memcpy(&(buf->data), data, lng);      // Store data into shared buffer

if (semctl(sem, info->com.maxC+1, SETALL, setsem) == -1)   // Done. unlock buffer	
	return SYSTEM_ERROR(SEM_UNLOCK_ERROR);

return NO_ERROR;
}


//@Function[API]: bufRequest
//
// Attaches a client (consumer) to a block of shared memory. 

// Calling this function is required before calling bufRead()
// Returns an identifier to be used for bufRead() calls, or a negative 
// error value. Multiple calls have no effect
//@

int bufRequest(const char *cname,   //@P{cname}: Client's name
               const char *bufname, //@P{bufname}: name of the requested buffer
               int  accmode,        //@P{accType}: access mode, BUFCONT or BUFSNAP
               BufInfo *info)       //@P{info}: shared buffer pointer
                                    //@R: Completion code

{
if(info==NULL) return PLAIN_ERROR(NOT_INIT_ERROR);
	
int ret,stat;
char aux[PROC_NAME_LEN+VAR_NAME_LEN+2], *pt;

#ifndef THRDLIB
info->loc.msgb.Msg=NULL;
#endif

strncpy(aux,bufname,PROC_NAME_LEN+VAR_NAME_LEN+1);
aux[PROC_NAME_LEN+VAR_NAME_LEN+1]='\0';
pt=index(aux,':');
if(!pt) return PLAIN_ERROR(BUF_NAME_ERROR);

*pt++='\0';

strncpy(info->com.name,pt,VAR_NAME_LEN);
info->com.name[VAR_NAME_LEN]='\0';
strncpy(info->com.producer,aux,PROC_NAME_LEN);
info->com.producer[PROC_NAME_LEN]='\0';

info->com.accMode=accmode | CONSUMER;

	// Request shared memory data to MsgD-RTDB and wait for an answer
#ifdef THRDLIB

ret=thSendMsg(sizeof(BufCom), "", SHMREQST, NOHANDLE_FLAG,&(info->com));
if(IS_ERROR(ret)) return ret;
info->loc.msgb=thWaitMsg(SHMREQST,"",ret,MSG_TIMEOUT,&ret);
if(!info->loc.msgb) return ret;
if (IS_ERROR(stat=HDR_PLOAD(info->loc.msgb))) {
   thRelease(info->loc.msgb);
   return stat;  // Check if answer is OK
}
memcpy(&(info->com), MSG_BODY(info->loc.msgb), sizeof(BufCom));
thRelease(info->loc.msgb);

#else

msg_seqNum=NewSeqN();
InitMsgBuf(&(info->loc.msgb),sizeof(BufCom));
FillHeader(0, msg_seqNum ,cname,"",SHMREQST, &(info->loc.msgb));
FillBody(&(info->com), sizeof(BufCom), &(info->loc.msgb));
SendToServer(&(info->loc.msgb));

ret=WaitMessageSet(&msg_seqNum, 1, &(info->loc.msgb), NULL, MSG_TIMEOUT);
if(IS_ERROR(ret)) return ret;
memcpy(&(info->com), msg_BODY(info->loc.msgb), sizeof(BufCom));
if (IS_ERROR(stat=hdr_PLOAD(info->loc.msgb))) return stat;  // Check if answer is OK

#endif


info->loc.sem_id = semget(info->com.sem_key, info->com.maxC,0);
if(info->loc.sem_id== (-1)) return SYSTEM_ERROR(SEM_GET_ERROR);

info->loc.shm_id = shmget(info->com.shm_key, info->com._lng,0);
if(info->loc.shm_id== (-1)) return SYSTEM_ERROR(SHM_GET_ERROR);

info->loc.b=(SharedBuffer *)shmat(info->loc.shm_id,NULL,0);
if(info->loc.b ==NULL) ret=SYSTEM_ERROR(SHM_ATTACH_ERROR);

if(accmode==BUFCONT) {           // initialize semaphores operations
    info->loc.locker.sbuf=readlock_bufcont;
    info->loc.unlocker.sbuf=readunlock_bufcont;
} else {
    info->loc.locker.sbuf=readlock_bufsnap;
    info->loc.unlocker.sbuf=readunlock_bufsnap;
    info->loc.snapN=(-1);
}

return NO_ERROR;
}

//@Function[API]: bufSnap
//
// Requests a sequence of BUFSNAP buffers

// This function is used by a consumer to request a sequence of data
// blocks in BUFSNAP mode.

// Reading snapshot data requires the following sequence of calls:

// \begin{verbatim}
// bufRequest()               # attach to the producer
// 
// for each buffer:
//     bufSnap(start,number)  # Request data
//        do number:
//            bufRead()       # Read each block
//
// bufRelease()               # detach from producer
// \end{verbatim}
//
// Returns an identifier to be used for bufRead() calls, or a negative 
// error value. Multiple calls have no effect
//@

int bufSnap(const char *cname,
            BufInfo *info,
            int     start,
            int     number)

{
if(info==NULL) return PLAIN_ERROR(NOT_INIT_ERROR);

BufSnapReq req;

if(info->com.accMode!=(BUFSNAP|CONSUMER)) return PLAIN_ERROR(BUF_MODE_ERROR);
if(start+number > info->com.maxSnap) return PLAIN_ERROR(BUF_SNAPLNG_ERROR);

req.bufID=info->com.bufID;
req.uniq=info->com.uniq;
req.start=start;
req.numb=number;

	// send request to Producer
#ifdef THRDLIB

thSendMsg(sizeof(BufCom), "", SHMSNAP,0,&(info->com));

#else

msg_seqNum=NewSeqN();
InitMsgBuf(&(info->loc.msgb),sizeof(BufSnapReq));
FillHeader(0, msg_seqNum ,cname,info->com.producer,SHMSNAP, &(info->loc.msgb));
FillBody(&req, sizeof(BufCom), &(info->loc.msgb));
SendToServer(&(info->loc.msgb));

#endif

info->loc.snapN = --number;   // Initialize reads counter

return NO_ERROR;
}

//@Function: bufRead
//
// Retrieve a local copy of the shared memory buffer. That is, copies the
// specified shared memory content into a local buffer. The shared memory
// must have been previously attached.

// In BUFSNAP mode data must be requested by calling bufSnap() prior
// of calling bufRead()

// Internals: see bufWrite()

//@

int bufRead(BufInfo * info,  //@P{info}: buffer specifier
            void *data,      //@P{data}: buffer for read data. It must provide
                             //   enough space to store the buffer.
            int timeout)     //@P{timeout}: Timeout (ms)
                             // If 0 or negative: wait forever.

                             //@R: Return status. If positive is the current
                             //    buffer count. If negative indicates an
                             //    error condition.
{
if(info==NULL) return PLAIN_ERROR(NOT_INIT_ERROR);

int sem=info->loc.sem_id;
int theConsumer=info->com.slot;
int ret;
SharedBuffer *buf=info->loc.b;
struct timespec *tmout;

if((info->com.accMode&ACCMODE_TYPE_MASK)!=CONSUMER) 
    return PLAIN_ERROR(BUF_TYPE_ERROR);

if((info->com.accMode&ACCMODE_MODE_MASK)==BUFSNAP) {
    if(info->loc.snapN<0) return PLAIN_ERROR(BUF_MODE_ERROR);
    info->loc.snapN--;
} 
info->loc.locker.sbuf[1].sem_num=theConsumer+1;
info->loc.unlocker.sbuf[1].sem_num=theConsumer+1;

if(timeout>0) {
    struct timespec delay;
    delay.tv_sec=timeout/1000;
    delay.tv_nsec=(timeout%1000)*1000000;
    tmout=&delay;
} else tmout=NULL;


if (semtimedop(sem, info->loc.locker.sbuf, 2, tmout) == -1) { // Lock buffer for read
    if(errno==EAGAIN) 
        ret=PLAIN_ERROR(TIMEOUT_ERROR);
    else
        ret=SYSTEM_ERROR(SEM_LOCK_ERROR);
} else {
   ret=buf->counter[theConsumer];

                             // If counter < 0, data is not valid
                             // This means that the semaphore has been released
                             // because of some producer error (e.g.: has died)
    if(ret>=0) {
        memcpy(data, &(buf->data[0]), info->com.lng);  // Copy data from buffer
        buf->counter[theConsumer]=INVALID_DATA_ERROR;  // Signal: "consumer has read the buffer"
    }

    if (semop(sem,info->loc.unlocker.sbuf,2) == -1)   // Done with buffer, unlock semaphore	
	ret= SYSTEM_ERROR(SEM_UNLOCK_ERROR);
}
return ret;
}

//@Function: bufRelease
//
// Releases a shared buffer

// This function sends to \msgd\  a request to unregister from a 
// shared buffer.

// The actual operation perfomed depends on the type of client and
// on the access mode, as follows:

// Client type: consumer and access mode: BUFCONT
// The client is simply detached, the buffer remains available for other 
// clients.

// Client type: consumer and access mode: BUFSNAP
// The consumer and the producer are both detached and the buffer is destroyed

// Client type: producer and access mode: BUFCONT
// The producer and all attached consumers are detached and the buffer
// is destroyed

// Client type: producer and access mode: BUFSNAP
// The producer and the consumer are both detached and the buffer is destroyed
//@

int bufRelease(const char *cname,  //@P{cname}: Client's name
               BufInfo * info)     //@P{info}: buffer specifier
                                   //@R: Return status.
{
if(info==NULL) return PLAIN_ERROR(NOT_INIT_ERROR);	

int ret;

ret=shmdt(info->loc.b);     // Locally detach from shared memory segment
if(ret<0) 
    return SYSTEM_ERROR(SHM_DETACH_ERROR);

	// Request shared memory detach to MsgD-RTDB no answer is expected
#ifdef THRDLIB

ret=thSendMsg(sizeof(BufCom), "", SHMDETACH, 0,&(info->com));

#else

msg_seqNum=NewSeqN();
FillHeader(0, msg_seqNum ,cname,"",SHMDETACH, &(info->loc.msgb));
FillBody(&(info->com), sizeof(BufCom), &(info->loc.msgb));
ret=SendToServer(&(info->loc.msgb));
FreeMsgBuf(&(info->loc.msgb));     // free message buffer

#endif

return ret;
}

