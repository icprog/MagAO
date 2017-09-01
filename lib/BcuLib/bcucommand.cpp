//@File: bcucommand.c
//
// sendBCUcommand() function and its accessories.
//
// Plus a few high-level functions unsing sendBCUcommand() that need to be visible
// to most C programs
//@

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/shm.h>

extern "C"  {
#include <netinet/in.h>
#include "commlib.h"
#include "base/thrdlib.h"
#include "BcuLib/BCUmsg.h"
}



// EEV39 tables for reordering
extern int **EEV39_tables;
extern int EEV39_numtables;

static int BCUseqnum = 1;

// A couple of #defines about the system
#define BCUCTRL_name	"MIRRORCTRL"



// ----------------- HEADER NEW -------------------- //
// ------------------------------------------------- //

#include "bcucommand.h"
#include "AdSecConstants.h"
#include "Logger.h"
#include "BcuLib/BcuCommon.h"

using namespace Arcetri;
using namespace Arcetri::Bcu;
using namespace Arcetri::AdSecConstants;


// Gloabal objects to adapt this library to the new BcuCommon and 
// Logger libraries
CommandSender* comSender;


// ----------------- HI-LEVEL FUNCTIONS OK FOR NEW MIRROR CTRL ---------------- //
// ---------------------------------------------------------------------------- //


//@Function: thDisableAllCoils
//
// Sends a command to every MIRROR BCU to disable all coils 
//
//@
int thDisableAllCoils()
{
    Logger* logger = Logger::get();
	
	BcuIdMap idMap;
	
	// Note that a "write" is always SEQ on BCUs (CRATES)
    uint32 data[2*BcuMirror::N_CRATES];
    
    int opcode = OpCodes::MGP_OP_RESET_DEVICES,
        bcuBroadcastId = idMap["BCU_MIRROR_ALL"],
        firstDsp = 0,  
        lastDsp  = BcuMirror::N_DSP_CRATE - 1, 
        address  = 0,
        datalen  = 8*BcuMirror::N_CRATES,
        timeout  = 300;

	for(int i=0; i<BcuMirror::N_CRATES; i++) {
		data[i*2+0] = 0x00000000;
		data[i*2+1] = 0x55555555;
	}

   	logger->log( Logger::LOG_LEV_DEBUG, "OPCODE : %d (MGP_OP_RESET_DEVICES)", opcode);
    logger->log( Logger::LOG_LEV_DEBUG, "Reset devices:");
    logger->log( Logger::LOG_LEV_DEBUG, " All BCU-MIRRORS (BroadcastId = %d)",idMap["BCU_MIRROR_ALL"]);
    logger->log( Logger::LOG_LEV_DEBUG, " from DSP: %d", firstDsp);
    logger->log( Logger::LOG_LEV_DEBUG, " to DSP %d", lastDsp);
    logger->log( Logger::LOG_LEV_DEBUG, " timeout : %d", timeout);
    
    int ErrC;
    try {
		if(comSender == NULL) {
     		comSender = new CommandSender();
     	}
     	comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, bcuBroadcastId, firstDsp, lastDsp, 
     							   	   opcode, address, datalen, 
     							   	   (BYTE*)data, timeout, Constants::WANTREPLY_FLAG);
     	ErrC = NO_ERROR;
     }
     catch(CommandSenderTimeoutException& e) {
		logger->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
		delete comSender;	// !!! see BcuCommon.h->Bcu::CommandSender::sendMultiBcuCommand(...) !!!
		comSender = NULL;	
		ErrC = TIMEOUT_ERROR;
	}
	catch(CommandSenderException& e) {
		logger->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
		ErrC = COMMUNICATION_ERROR;
	}

	return PLAIN_ERROR(ErrC);
}



// ----------------------- TEMP CODE ------------------------- //
// ----------------------------------------------------------- //

//@Function: thSendBCUcommand
//
// threadlib-compatible version of the sendBCUcommand() function.
// Solves the bugs arising from interleaved BCU answers and variable notification messages
//
// Note: Force the WANTREPLY_FLAG.
//
// 
// ATTENTION: Temporary adapted to use the new BcuCommon::sendMultiBcuCommand(...)
// Please don't use this: directly use the BcuCommon instead !!! 
//@
int thSendBCUcommand(const char * /* clientname */, int BCUnumber, int firstDSP, int lastDSP, int opcode, int address,unsigned char *data, int datalen, int timeout, int /* seqnum */, int flags) throw (AOException)
{
    Logger* logger = Logger::get();

    flags |= FLAG_WANTREPLY;    // Set this flag because the old MirrorCtrl forced this, 
                                // (and the clients of this function suppose the flag is forced)
                                // but the new MirrorCtrl don't 
    int ntry=3;
    try {
	if(comSender == NULL) {
     		comSender = new CommandSender();
     	}
        while (1) 
           try {
             comSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, BCUnumber, BCUnumber, firstDSP, lastDSP, 
     							   	   opcode, address, datalen, 
     							   	   data, timeout, flags);
              break;

           } catch (...) {
              ntry--;
              if (ntry==0)
                 throw;
   	     logger->log(Logger::LOG_LEV_WARNING, "Error in BCU command, retrying....");
          }
        }
     catch(CommandSenderTimeoutException& e) {
		logger->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
		delete comSender;	// !!! see BcuCommon.h->Bcu::CommandSender::sendMultiBcuCommand(...) !!!
		comSender = NULL;	
      throw AOException("Timeout error in sendMultiBcuCommand()", TIMEOUT_ERROR, __FILE__, __LINE__);
	}
      catch(CommandSenderException& e) {
		logger->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
      throw AOException("Communication error in sendMultiBcuCommand()", COMMUNICATION_ERROR, __FILE__, __LINE__);
	}

      return NO_ERROR;
}



//+Function: disableAllCoils
//
// Sends a command to a BCU to disable all coils
//-
int disableAllCoils(char *clientname, int BCUnumber){

   Logger* logger = Logger::get();
    unsigned char data[8];
    int i,
        status,
        firstDSP=0,  //TODO
        lastDSP=10,  //TODO
        timeout=300; //TODO

    for (i=0; i<4; i++) data[i]=0;
    for (i=4; i<8; i++) data[i]=0x55; //disable all coils
    

    if (0){
        printf("OPCODE : MGP_OP_RESET_DEVICES\n");
        printf("Reset devices:\n");
        printf("BCU num: %d\n", BCUnumber);
        printf(" from DSP: %d", firstDSP);
        printf(" to DSP %d\n", lastDSP);
        printf("timeout : %d\n", timeout);
    }  


    status = thSendBCUcommand(clientname, 
            BCUnumber, 
            firstDSP,
            lastDSP,
            MGP_OP_RESET_DEVICES,
            0, 
            data, 
            8, 
            timeout, 
            BCUseqnum++,
            FLAG_WANTREPLY);

    logger->log( Logger::LOG_LEV_DEBUG, "sendBCUcommand replied %d",status);
    return status;
}






