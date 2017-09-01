//@File: shmlib.h
//
// Header file for the shared memory library

// The following definition are needed for programs using the "shmlib"
// functions.
//@

#ifndef SHMLIB_H
#define SHMLIB_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "base/common.h"
#include "base/buflib.h"

#include "types.h"             // For Client structure

#define SEM_KEY_BASE	19820414  // (0x12e6f7e) Base for semaphore key generation
#define SHM_KEY_BASE	19790126  // (0x12df92e) Base for shared memory key generation

typedef struct {
        int nCons;        // Number of accepted consumers
        Client * cList[MAX_CONSUMERS];
} CnsInfo;

typedef struct {
    int next;
    BufCom  *bInfo;
    CnsInfo *cInfo;
} ShmListscan;

typedef struct {
    int maxUsedBuffer;                // Max index for shared buffers
    int nBuffers;                     // Current number of buffers
    BufCom bInfo[MAX_SHM_BLOCKS];     // shared buffers headers
    CnsInfo cInfo[MAX_SHM_BLOCKS];    // consumer info
} ShmBufferPool;

// Internal library routines

int shmCreate(BufCom *inp, pthread_mutex_t *mutex);
int shmAttach(Client *client, BufCom *inp, pthread_mutex_t *mutex);
int shmDetach(Client *client, int, int, pthread_mutex_t *);
void  InitShmList(ShmListscan *);
int NextShmList(ShmListscan *);
void shmInit(void);
int shmDestroy(int bufid, pthread_mutex_t *mutex);
int nBuffers(void);

#endif
