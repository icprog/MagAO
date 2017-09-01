//@File: buflib.h
//
// Header file for the shared buffer management library
//@

#ifndef BUFLIB_H_INCLUDED
#define BUFLIB_H_INCLUDED

#include "base/common.h"

#define BUFCONT 0x01
#define BUFSNAP 0x02

#define PRODUCER 0x10
#define CONSUMER 0x20

#define ACCMODE_MODE_MASK  0x0f
#define ACCMODE_TYPE_MASK  0xf0

#define MSG_TIMEOUT 1000   // Timeout for MsgD replay. 1000 looks as a better
                            // value for production code. 60000 is useful
                            // when debugging.
//@C
//  Note: the following lines define the structure needed to operate on a 
//   set of semaphores. The number of semaphores is MAX\_CONSUMERS, so the
//   array must have MAX\_CONSUMERS elements. Here some repetition operator
//   would be handy. In the lack of it great care must be used when modifying
//   the value of MAX\_CONSUMERS.
//@

// Macros to lock and unlock semaphores

// Note: we need one sempahore for each consumer plus one for the producer
// So in total there are (MAX_CONSUMERS+1) semaphores. The producer semaphore
// has index: 0

// The following values are to be used by semop()

                              // wait wr.sem == MAX_CONSUMERS
#define BUFCONT_SEM_WRITE_LOCK { { 0, -MAX_CONSUMERS, SEM_UNDO } }

                              // Wait wr.sem == 0
#define BUFSNAP_SEM_WRITE_LOCK { { 0, -1, SEM_UNDO }, \
                                 { 1, -1, SEM_UNDO } }

#define BUFCONT_SEM_READ_LOCK { {0, -1, SEM_UNDO},\
                                {0,  0, SEM_UNDO} }  // Consumer semaphore


#define BUFCONT_SEM_READ_UNLOCK { {0, 1, 0},\
                                  {0, 1, 0} }  // Consumer semaphore

                              // Increase Write semaphore (to no effect)
                              // Wait read semaphore 0
#define BUFSNAP_SEM_READ_LOCK { { 0, 1, SEM_UNDO }, \
                                { 1, 0, SEM_UNDO } }

                             //   set wr.sem at 0        set rd.sem at 1
#define BUFSNAP_SEM_READ_UNLOCK { { 0, 1, 0 }, \
                                  { 1, 1, 0 } }


// The following values are to be used by semctl()
                          // MAX_CONSUMERS=15
#define BUFCONT_SEM_INIT {   MAX_CONSUMERS, \
                             1, 1, 1, 1, 1, \
                             1, 1, 1, 1, 1, \
                             1, 1, 1, 1, 1     }

#define BUFSNAP_SEM_INIT {   1, 1 }

                                // MAX_CONSUMERS=15
#define BUFCONT_SEM_WRITE_UNLOCK { MAX_CONSUMERS,\
                                   0, 0, 0, 0, 0,\
                                   0, 0, 0, 0, 0,\
                                   0, 0, 0, 0, 0  }

                             //  set wr.sem at 1  set rd.sem at 0
#define BUFSNAP_SEM_WRITE_UNLOCK { 1,                   0 }


//@Struct: SharedBuffer
//
// Base structure for shared buffer manipulation

//@

typedef struct {
    int counter[MAX_CONSUMERS];      // Buffer counters
    unsigned char data[];            // Data buffer (to be allocated)
} SharedBuffer;

//@Struct: BufCom
//
// Base structure holding information related to attached buffers 

// This structure is used to hold shared buffer related variables which are
// globally menaingful. It is also used as message body when sending requests
// to the Msgd-RTDB, and receiving replies.
//@


typedef struct 
{
    char name[VAR_NAME_LEN+1];      // Shared buffer name
    char producer[PROC_NAME_LEN+1]; // Producer's name
    int accMode;                    // Either BUFCONT or BUFSNAP
                                    // or-ed with either PRODUCER or CONSUMER
                                    // On return from MsgD it is set to a
                                    // negative value to indicate error
    int maxC;                       // Max accepted consumers. This value is
                                    // defined when the buffer is created.
                                    // and may not be higher than MAX_CONSUMERS
    int maxSnap;                    // Maximum number of buffers in a BUFSNAP
                                    // mode request (meaningless in BUFCONT mode)
    int sem_key;                    // Buffer access semaphore Key returned by
                                    // calling bufCreate() or bufRequest()
    int shm_key;                    // Shared memory key returned by calling
                                    // bufCreate() or bufRequest()
    int bufID;                      // Index in Buffer list
    int uniq;                       // Unique identifier for given buffer
    int slot;                       // Shared memory slot (semaphore index for
                                    // read access). This field is used to send
                                    // back to consumer the index of the assigned
                                    // semaphore.
    int lng;                        // buffer size in bytes (data)
    int _lng;                       // Total buffer length (including counters)
} BufCom;


//@Struct: BufLocal
//
// Structure holding information related to attached buffers for process local use

// This structure is used to hold shared buffer related variables which are
// meaningful only within each process.
//@
typedef struct {
    SharedBuffer *b;             // buffer memory structure
    int  msg_seqnum;             // Incremental Counter
    int  sem_id;                 // Buffer access semaphore ID. This field is
                                 // used to store the local semaphore ID (it has
                                 // no meaning for other processes)
    int  shm_id;                 // Shared memory ID. This field is used to 
                                 // store the local shared memory ID (it has
                                 // no meaning for other processes)
    int snapN;                   // Counter for buffer reads in BUFSNAP mode
                                 // Semaphore operators
    union {
        struct sembuf *sbuf;
        unsigned short *ushort;
    }  locker;                   // Lock structure     
    union {
        struct sembuf *sbuf;
        unsigned short *ushort;
    }  unlocker;                 // Unlock structure     
#ifdef THRDLIB
    MsgBuf *msgb;                // Message buffer pointer for SHMSNAP support
#else
    MsgBuf msgb;                 // Message buffer for SHMSNAP support
#endif
} BufLocal;



//@Struct: BufInfo
//
// Structure holding information related to attached buffers 

// This structure is the combination of BufCom, BufLocal and BufSnapReq
// and defines all management variables for shared buffers. 
//@

typedef struct {

    BufCom   com;       // Global and communication section
    BufLocal loc;       // Process local section

} BufInfo;

//@Struct: BufSnapReq
//
// Structure to be used for SHMSNAP request messages

//@

typedef struct 
{
    int bufID;                    // Index in Buffer list
    int uniq;                     // Unique identifier for given buffer
    int start;                    // Starting position
    int numb;                     // number of buffers
} BufSnapReq;


// Producer function prototypes

int bufCreate(const char *myName, const char *bufName, int accMode, int len,
              int maxCS, BufInfo *info);
int bufWrite(BufInfo *info, void *data, int counter, int timeout);

int bufWaitSnap(BufInfo *info, int *start, int *numb, MsgProc cback);

// Counsumer function prototypes

int bufRequest(const char *myName, const char *bufName, int accMode, BufInfo *info);
int bufRead(BufInfo *info, void *data, int timeout);
int bufSnap(const char *myName, BufInfo *info, int start, int numb);
int bufRelease(const char *cname, BufInfo *info);


#endif // BUFLIB_H_INCLUDED
