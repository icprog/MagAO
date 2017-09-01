//@File: shmlib.c
//
// Shared memory library for MsgD-RTDB:
//@


#include <stdio.h>
#include <stdlib.h>
#include <string.h>		// memcpy()

#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>		// semop()
#include <sys/shm.h>		// shmat()

#include "base/msglib.h"
#include "base/errlib.h"

#include "shmlib.h"
#include "logerr.h"

extern ShmBufferPool bPool;

//@Function: shmInit

// Initializes shared memory support

// This routine initializes the structure for the management
// of shared memory. First creates the shared memory and semaphore keys,
// then removes both structures with that keys (if existing).

// It is called once when msgdrtdb starts

//@

void shmInit(void)
{
int sem,shm,i,j;

for(i=0; i<MAX_SHM_BLOCKS; i++) {    // mark elements empty
    bPool.bInfo[i].lng=(-1);
    bPool.bInfo[i].sem_key = SEM_KEY_BASE+i;
    bPool.bInfo[i].shm_key = SHM_KEY_BASE+i;
    bPool.bInfo[i].producer[0] = '\0';
    bPool.bInfo[i].name[0] = '\0';
    if((sem=semget(bPool.bInfo[i].sem_key, 0, 0))>=0) {
        semctl(sem,0,IPC_RMID);           // Delete existing sem
    }
    if((shm=shmget(bPool.bInfo[i].shm_key, 1, 0))>=0) {
        shmctl(shm,IPC_RMID,NULL);        // Delete existing shared mem. segment
    }

    for(j=0; j<MAX_CONSUMERS; j++) {      // Initialize consumer list
        bPool.cInfo[i].cList[j]=NULL;
    }
    bPool.cInfo[i].nCons=0;
}
bPool.maxUsedBuffer=(-1);
bPool.nBuffers=0;
}


//@Function: blockInit

// Initializes shared block support data

// This routine initializes the structures for the management
// of a block of shared memory. 

// It is called when a block is created on producer's request

//@

static int blockInit(BufCom* info,BufCom *inp)
{
int sem,shm;
size_t dataLen;
extern int UniqGenerator;

dataLen = sizeof(SharedBuffer)+ inp->lng;

if(inp->accMode&BUFCONT) {
    inp->maxSnap=0;
} else {
    inp->maxC=1;
}
                                // Create semaphore array
if((sem=semget(info->sem_key, inp->maxC+1, 0666|IPC_CREAT))<0)
    return SYSTEM_ERROR(SEM_CREATE_ERROR);

                               // Create shared memory buffer
if((shm=shmget(info->shm_key,dataLen,0666|IPC_CREAT))<0) {
    semctl(sem,0,IPC_RMID);                 // Remove semaphore on error
    return SYSTEM_ERROR(SHM_CREATE_ERROR);
}

                               // Store info into buffer structure
strncpy(info->name,inp->name,PROC_NAME_LEN);
info->name[PROC_NAME_LEN]='\0';
strncpy(info->producer,inp->producer,PROC_NAME_LEN);
info->producer[PROC_NAME_LEN]='\0';
info->accMode=inp->accMode;
info->maxC=inp->maxC;
info->lng=inp->lng;
info->_lng=dataLen;
info->bufID=inp->bufID;
info->maxSnap=inp->maxSnap;
info->uniq=UniqGenerator++;

if(info->bufID>bPool.maxUsedBuffer) bPool.maxUsedBuffer=info->bufID;

return info->bufID;
}


//@Function: shmAddConsumer

// Adds a consumer to a given memory buffer

// This routine adds a consumer to the specified shared buffer
//@

static int shmAddConsumer(int bufid,  //@P{bufid}: buffer to attach to
                          Client *cons) //@P{cons}: consumer name
                                      //@R: consumer index in buffer consumer list
                                      //    or error code
{
int i,found=PLAIN_ERROR(SLOT_NOT_FOUND_ERROR);

for(i=0; i<bPool.bInfo[bufid].maxC; i++) {  // Search empty slot for consumer
    if(bPool.cInfo[bufid].cList[i]==NULL) {
       found=i;
       break;
    }
}
if(found>=0) {
    bPool.cInfo[bufid].cList[found]=cons; // Mark slot occupied
    bPool.cInfo[bufid].nCons++;           // Update number of consumers
    
}
return found;
}

//@Function: shmCreate

// Allocates a shared memory block on producer's request

// This function is called by MSGD-RTDB when an SHMCREA message is received
// from a client (producer).

// Returns buffer ID or error code
//@

int shmCreate(BufCom *inp,             // Request structure
              pthread_mutex_t *mutex)
{
int i,found;
BufCom *info;

if(inp->maxC > MAX_CONSUMERS)
    return PLAIN_ERROR(MAX_CONSUMERS_ERROR);

found= PLAIN_ERROR(SLOT_NOT_FOUND_ERROR);

MUTEX_LOCK(mutex,"shmCreate");

for(i=0; i<MAX_SHM_BLOCKS; i++) {   // Search empty slot for block
    info=&bPool.bInfo[i];
    if(info->lng<0) {
        found=i;
        inp->bufID=found;
        break;
    } else {
        if( (strncmp(info->producer,inp->producer,PROC_NAME_LEN)==0) &&
            (strncmp(info->name,inp->name,PROC_NAME_LEN)==0)         ) {
            found = PLAIN_ERROR(BUF_DUPL_NAME_ERROR);
            break;
        }
    }
}

if(found>=0) {                                    // block slot found
    found = blockInit(info,inp);
    bPool.nBuffers++;
}

MUTEX_UNLOCK(mutex,"shmCreate");

return found;
}


//@Function: shmAttach
//
// Attaches a consumer to a shared buffer

// This function is called by MsgD-RTDB when the SHMATTACH message is 
// received. The consumer is attached to the requested buffer and related info
// is sent back to the requester.
//@

int shmAttach(Client *aClient, //@P{aClient}: Client structure
              BufCom *inp,     //@P{info}: Buffer request structure
                               //@R: buffer id (or negative error code)
              pthread_mutex_t *mutex)
{
int bufid,slot,i;

MUTEX_LOCK(mutex,"shmAttach");

bufid=PLAIN_ERROR(BUF_NOT_AVAIL_ERROR);

for(i=0; i<MAX_SHM_BLOCKS; i++) {   // Search given slot into buffer table
    BufCom *info=&bPool.bInfo[i];
    if(info->lng>0)  {
        if( (strncmp(info->producer,inp->producer,PROC_NAME_LEN)==0) &&
            (strncmp(info->name,inp->name,VAR_NAME_LEN)==0)         ) {
            bufid=i;
            break;
        }
    }
}        
if(bufid>=0)  {
    slot = shmAddConsumer(bufid,aClient);     // Add consumer to pool
    if(slot>=0)  
        bPool.bInfo[bufid].slot=slot;
    else
        bufid=slot;   // Return error code !
}
 
MUTEX_UNLOCK(mutex,"shmAttach");

return bufid;
}



//@Function: shmDetach
//
// Detaches a consumer from a shared buffer

// This function is called by MsgD-RTDB when the SHMDETACH message is 
// received. The consumer is detached from the indicated buffer and related 
// resourced are freed.
//@

int shmDetach(Client *cl,  //@P{cl}: Client to remove from list
              int bufid,   //@P{bufid}: Buffer identifier
              int slot,    //@P{slot}:  consumer slot identifier
              pthread_mutex_t *mutex)  //@P{mutex}: shared memory mutex
{
int ret=NO_ERROR;

MUTEX_LOCK(mutex,"shmDetach");

if(bPool.cInfo[bufid].cList[slot]==cl) {                              //*****
                                                                      //*****
    bPool.cInfo[bufid].cList[slot]=NULL;                              //*****
    bPool.cInfo[bufid].nCons--;               // Update number of consumers *****
} else
    ret=PLAIN_ERROR(CONS_DETACH_ERROR);
MUTEX_UNLOCK(mutex,"shmDetach");

return ret;
}


//@Function: shmDestroy
//
// Destroys a shared buffer

// This function is called by MsgD-RTDB when a producer disconnects.
// or requires deletion of a shared memory buffer

// Consumers attempting to access the shared memory buffer should return
// with an error status.

//@

int shmDestroy(int bufid, pthread_mutex_t *mutex)
{
int sem,shm;
int ret=NO_ERROR;
BufCom *info=&bPool.bInfo[bufid];

MUTEX_LOCK(mutex,"shmDestroy");

if(bPool.bInfo[bufid].lng<=0) {
   pthread_mutex_unlock(mutex);
   return ret;
}

     // Remove semaphores
sem = semget(info->sem_key, info->maxC+1,0);       // Get semaphore identifier
if(sem== (-1)) 
  ret = SYSTEM_ERROR(SEM_GET_ERROR);
else
  if(semctl(sem,0,IPC_RMID)== -1)                    // Remove semaphore array
    ret = SYSTEM_ERROR(SEM_REMOVE_ERROR);

     // Remove shared memory buffer
shm = shmget(info->shm_key, info->_lng,0);          // Get shared memory identifier
if(shm == (-1)) 
  ret = SYSTEM_ERROR(SHM_GET_ERROR);
else
  if(shmctl(shm,IPC_RMID,NULL) == (-1))
    ret = SYSTEM_ERROR(SHM_REMOVE_ERROR);

bPool.bInfo[bufid].lng=(-1);                       // Signal free slot
bPool.bInfo[bufid].producer[0] = '\0';
bPool.bInfo[bufid].name[0] = '\0';
bPool.nBuffers--;

MUTEX_UNLOCK(mutex,"shmDestroy");

return ret;
}

//@Function: InitShmList

// Initialize shared buffer retrieval list

//@

void InitShmList(ShmListscan *listX)
{
listX->next=(-1);
}

//@Function: NextShmList

// Get next item in shared buffer retrieval list 

// Returns 1 until the list is empty. List scan must be initialized
// by calling @see{InitShmList}.

//@

int NextShmList(ShmListscan *listX)
{
while(++listX->next< MAX_SHM_BLOCKS) {
    if(bPool.bInfo[listX->next].lng>=0) {
        listX->bInfo=bPool.bInfo+listX->next;
        listX->cInfo=bPool.cInfo+listX->next;
        return 1;
    }
}
return 0;
}


//@Function: nBuffers

// Returns the current number of active buffers

//@

int nBuffers(void)
{
return bPool.nBuffers;
}

