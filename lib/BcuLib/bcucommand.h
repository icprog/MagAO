//+File: bcucommand.h
//
// Header file for bcucommand functions
//
// ATTENTION: temporary adapted to use the new library BcuCommon !!!
// To be removed definitively !!!
//-


#ifndef BCUCOMMAND_H_INCLUDED
#define BCUCOMMAND_H_INCLUDED

#include "BcuLib/BCUmsg.h"

#include "aotypes.h"
#include "AOExcept.h"

#define BCU_DSP  (0xFF)


void DumpCmd( BCUmessage *cmd);

typedef struct {
 int shm_id;                     // Shared memory id
 int blocksize;                  // Size of shared memory buffer
 unsigned char *shm_addr;        // Shared memory buffer address
} SharedMem;


// Fixed to use the new MirrorCtrl
int thDisableAllCoils();

// Temporary adapted to use the new BcuCommon::sendMultiBcuCommand(...)
// Please don't use this: directly use the BcuCommon instead !!!
int thSendBCUcommand(const char *clientname, int BCUnumber, int firstDSP, int lastDSP, int opcode, int address,unsigned char *data, int datalen, int timeout, int seqnum, int flags) throw (AOException);

//SharedMem *shmAlloc( int size);
//int shmFree( SharedMem *shared);

int askPixels( SharedMem *shared, char *clientname, int BCUnumber, int framenum, int seqnum);
int askSlopes( SharedMem *shared, char *clientname, int BCUnumber, int framenum, int seqnum);
int askCommands( SharedMem *shared, char *clientname, int BCUnumber, int framenum, int seqnum);

int disableAllCoils(char *clientname, int BCUnumber);

#endif //BCUCOMMAND_H_INCLUDED

