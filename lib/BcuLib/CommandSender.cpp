
extern "C" {
	#include "base/thrdlib.h"
}

#include "CommandSender.h"

#include "Utils.h"
#include "stdconfig.h"
#include "AdSecConstants.h"
#include "WfsConstants.h"
#include "BcuLib/BcuMaps.h"
#include "BcuLib/Commons.h"
#include "BcuLib/BcuMessage.h"

#include <iostream>
#include <algorithm>
#include <iomanip>

using namespace Arcetri::Bcu;

#include <sys/shm.h>

Bcu::CommandSender::CommandSender() { 
	// Init log level using this one of the AOApp (or the idl wrapper)  using 
	// this library
	_logger = Logger::get("MAIN");

	// Init the bcu id values for efficiency
	BcuIdMap idMap;
	_BCU_39 = idMap["BCU_39"];
	_BCU_47 = idMap["BCU_47"];
	_BCU_SWITCH = idMap["BCU_SWITCH"];
	_BCUMIRROR_0 = idMap["BCU_MIRROR_0"];
	_BCUMIRROR_1 = idMap["BCU_MIRROR_1"];
	_BCUMIRROR_2 = idMap["BCU_MIRROR_2"];
	_BCUMIRROR_3 = idMap["BCU_MIRROR_3"];
	_BCUMIRROR_4 = idMap["BCU_MIRROR_4"];
	_BCUMIRROR_5 = idMap["BCU_MIRROR_5"];
	_BCUMIRROR_ALL = idMap["BCU_MIRROR_ALL"];
	_BCU_ALL = idMap["BCU_ALL"];


    /////////////// GET MIRRORCTRL NAME /////////////////

    _mirrorctrl = "mirrorctrl."+Utils::getAdoptSide();

	_shared = NULL; 
	_logger->log(Logger::LOG_LEV_TRACE, "CommandSender constructed [%s:%d]", __FILE__,__LINE__);
}


Bcu::CommandSender::~CommandSender() { 
	_logger->log(Logger::LOG_LEV_TRACE, "Destroying CommandSender...[%s:%d]",__FILE__,__LINE__);
	if(_shared) {
		shmFree(); 
	}
	_logger->log(Logger::LOG_LEV_TRACE, "CommandSender destroyed.  [%s:%d]",__FILE__,__LINE__);
}


/*
 * Allocates the shared memory
 */
void Bcu::CommandSender::shmAlloc(int size) throw(SharedMemoryException) {

	// --- Create the shared memory manager
	_shared = new SharedMem();
	
	if (!_shared) {
		throw SharedMemoryException("Unable to allocate the shared memory");
	}
	// --- Create the shared memory and attach to it
	else {
		_shared->blocksize = size;
		// Create the shared memory
		_shared->shm_id = shmget(IPC_PRIVATE, _shared->blocksize, IPC_CREAT | 0777);
		if (_shared->shm_id == -1) {
			delete _shared;
			_shared = NULL;
			throw SharedMemoryException("Unable to allocate the shared memory");
		}
		// Attach to shared memory
		_shared->shm_addr = (unsigned char*)shmat(_shared->shm_id, 0, 0);
	}
	
	_logger->log(Logger::LOG_LEV_TRACE, "sendMultiBcuCommand: Shared memory of size %d bytes allocated", _shared->blocksize);
}

/*
 * Frees the shared memory
 */
void Bcu::CommandSender::shmFree() {
	shmdt(_shared->shm_addr);
	shmctl(_shared->shm_id, IPC_RMID, NULL);
	_logger->log(Logger::LOG_LEV_TRACE, "sendMultiBcuCommand: Shared memory of size %d bytes deallocated", _shared->blocksize);
	
	delete _shared;
	_shared = NULL;
}	


void Bcu::CommandSender::sendMultiBcuCommand(int priority, int firstBcu, int lastBcu, int firstDSP, 
					int lastDSP, int opcode, int address, int datalen, 
					unsigned char *data, int timeout_ms, int flags) 
						throw (RequestFaultException, CommandSenderTimeoutException, CommandSendingException) {
	
	BcuMessage cmd;
	
	int seqNum, numBcu, numDsp;

	// --- (1) Arrange the shared memory --- //

	// Compute number of BCUs and DSPs involved

	numBcu = lastBcu-firstBcu+1;
	if (OpCodes::isSame(opcode)) {
     	numDsp = 1;
     	_logger->log(Logger::LOG_LEV_DEBUG, "CommandSender::sendMultiBcuCommand: SAME request");
	}
   	else {
    	numDsp = (lastDSP-firstDSP)/OpCodes::getDspFactor(opcode)+1;
    	_logger->log(Logger::LOG_LEV_DEBUG, "CommandSender::sendMultiBcuCommand: SEQ request");
	}

   // Check data size
   if (datalen & 0x03)
      throw CommandLengthException(datalen);

	
	// Compute the size of data to transfer
	int dataSize = 0;	
	if(OpCodes::isSame(opcode)) {
		dataSize = datalen;
	}
	else {
		dataSize = datalen*numDsp*numBcu;
	}

	// See if we have to change the dimensions of the shared buffer
	if ((_shared != NULL) && (_shared->blocksize < dataSize)) {
		shmFree();
		_shared = NULL;
	}
	if (!_shared) {
		try {
			_logger->log(Logger::LOG_LEV_DEBUG, "CommandSender::sendMultiBcuCommand: allocating shm...");
			shmAlloc(dataSize);
			_logger->log(Logger::LOG_LEV_DEBUG, "CommandSender::sendMultiBcuCommand: shm allocated !!!");
		} catch (SharedMemoryException) {
			throw CommandSendingException();
		}
	}
	
	// Only for test: clean the shm
	memset(_shared->shm_addr, 0, dataSize);
	_logger->log(Logger::LOG_LEV_DEBUG, "sendMultiBcuCommand: shared memory cleaned");
	
	// Copy the data into the shared area if needed
	if (OpCodes::isWrite(opcode)) {
		_logger->log(Logger::LOG_LEV_DEBUG, "sendMultiBcuCommand: copying data (%d Bytes, %d Dword) to shm...", dataSize, dataSize/Constants::DWORD_SIZE);
		memcpy(_shared->shm_addr, data, dataSize);	
		_logger->log(Logger::LOG_LEV_DEBUG, "sendMultiBcuCommand: data copied to shared memory ! ");
//		_logger->log(Logger::LOG_LEV_TRACE, "sendMultiBcuCommand: data in shared memory:");
//		if(_logger->getLevel()>=Logger::LOG_LEV_TRACE) {
//			for(int i=0; i<(dataSize/Constants::DWORD_SIZE); i++) {
//				printf("%08X ", ((unsigned int*)data)[i]);
//			}
//			printf("\n");
//		}
   }

	// --- (2) Build the binary struct --- //
	memset(&cmd, 0, sizeof(cmd));
	cmd._firstBcu = firstBcu;	
	cmd._lastBcu = lastBcu;
	cmd._firstDsp = firstDSP;
	cmd._lastDsp = lastDSP;
	cmd._opCode = opcode;
	cmd._dataAddress = address;
	cmd._dataSize = datalen;
	cmd._shmId = _shared->shm_id;
	cmd._flags = flags;

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Sending BCU command: bcu %d-%d, dsp %d-%d, opcode %d, address %d, datasize %d, shmid %d, flags %d", firstBcu, lastBcu, firstDSP, lastDSP, opcode, address, datalen, _shared->shm_id, flags);

	// --- (3) Establish the code of the message --- //
	int bcuMsgCode;
	bool err = false;
	// Broadcast messages
	if (firstBcu == _BCU_39 && lastBcu == _BCUMIRROR_5) {
		// [todo] I need to install an handler for this and I need to check
		// which BCU are on !!!
		err = true;	
	}
	// Messages to BCU 39
	else if (firstBcu == _BCU_39 && lastBcu == _BCU_39 && WfsConstants::Bcu39::N_CRATES == 1) {
		if(priority == Priority::LOW_PRIORITY) {
			bcuMsgCode = BCUMSG_39_LP;
		}
		else if(priority == Priority::HIGH_PRIORITY) {
			bcuMsgCode = BCUMSG_39_HP;
		}
		else err = true;
	}
	// Messages to BCU 47
	else if(firstBcu == _BCU_47 && lastBcu == _BCU_47 && WfsConstants::Bcu47::N_CRATES == 1) {
		if(priority == Priority::LOW_PRIORITY) {
			bcuMsgCode = BCUMSG_47_LP;
		}
		else if(priority == Priority::HIGH_PRIORITY) {
			bcuMsgCode = BCUMSG_47_HP;
		}
		else err = true;
	}
	// Messages to BCU SWITCH
	else if(firstBcu == _BCU_SWITCH && lastBcu == _BCU_SWITCH && AdSecConstants::BcuSwitch::N_CRATES == 1) {
		if(priority == Priority::LOW_PRIORITY) {
			bcuMsgCode = BCUMSG_SWITCH_LP;
		}
		else if(priority == Priority::HIGH_PRIORITY) {
			bcuMsgCode = BCUMSG_SWITCH_HP;
		}
		else err = true;
	}
	// Messages to BCUMIRROR0-BCUMIRROR5
	else if(firstBcu >= _BCUMIRROR_0 && lastBcu <= _BCUMIRROR_0 + AdSecConstants::BcuMirror::N_CRATES - 1) {
		if(priority == Priority::LOW_PRIORITY) {
			bcuMsgCode = BCUMSG_MIRROR_LP;
		}
		else if(priority == Priority::HIGH_PRIORITY) {
			bcuMsgCode = BCUMSG_MIRROR_HP;
		}
		else err = true;
	}
	// Messages to nobody !!!
	else {
		err = true;
	}
	
	// Check for errors
	if(err) {
		_logger->log(Logger::LOG_LEV_ERROR, "sendMultiBcuCommand: impossible to handle a request with firstBcu=%d and lastBcu=%d and priority %s", firstBcu, lastBcu, Priority::getPriorityDescription(priority).c_str());
		throw CommandSendingException();
	}
		
	// --- (4) Send the BCU command	--- //
   	seqNum = thSendMsg( sizeof(BcuMessage), _mirrorctrl.c_str(), bcuMsgCode, 0, &cmd);
	if (IS_ERROR(seqNum)) {
      int stat = PLAIN_ERROR(seqNum);
		_logger->log(Logger::LOG_LEV_ERROR, "Error in thSendMsg: %d %s", stat, lao_strerror(stat));
      	throw CommandSendingException();
   	}

	// --- (5) Waits for the answer if requested--- //
	if((flags & Constants::WANTREPLY_FLAG) == Constants::WANTREPLY_FLAG) {
                int stat;
		
		_logger->log(Logger::LOG_LEV_DEBUG, "sendMultiBcuCommand: REPLY EXPECTED !!!");
		
	   	MsgBuf *buf = thWaitMsg(ANY_MSG, _mirrorctrl.c_str(), seqNum, timeout_ms, &stat);
		if (!buf) {
			//_logger->log(Logger::LOG_LEV_ERROR, "Error in thWaitMsg: %d %s", stat, lao_strerror(stat));
	      	throw CommandSenderTimeoutException();
	    }
	     
	    // --- (6) Fill the answer buffer for read commands --- //
		if (HDR_CODE(buf) == ACK) {
			if (OpCodes::isRead(opcode)) {
				memcpy(data, _shared->shm_addr, dataSize);
			}
			thRelease(buf);
		}
		else {
			thRelease(buf);	
			throw RequestFaultException();
		} 
	}
	else {
		_logger->log(Logger::LOG_LEV_DEBUG, "sendMultiBcuCommand: NO REPLY EXPECTED !!!");
	}
}


void Bcu::CommandSender::sendMultiBcuCommand(int priority, int bcuId,int firstDSP, int lastDSP, 
					 int opcode, int address, int datalen, 
					 unsigned char *data, int timeout_ms, int flags) 
						throw (RequestFaultException, CommandSenderTimeoutException, CommandSendingException) {

	int firstBcu, lastBcu;
	
	if(bcuId == _BCUMIRROR_ALL) {
		firstBcu = _BCUMIRROR_0;
		lastBcu = _BCUMIRROR_0 + AdSecConstants::BcuMirror::N_CRATES - 1;
	}
	else if(bcuId == _BCU_ALL) {
		// [todo] Create an handler for this !!!
		// firstBcu = _BCU_39;
		// lastBcu = _BCUMIRROR_5;
		_logger->log(Logger::LOG_LEV_ERROR, "sendMultiBcuCommand: impossible to send a message to ALL BCUs (bcuId = %d)", bcuId);
		throw new CommandSendingException();
	}
	else if( bcuId == _BCU_39 || bcuId == _BCU_47 || bcuId == _BCU_SWITCH || 
	        (bcuId >= _BCUMIRROR_0 && bcuId <= _BCUMIRROR_5) ) {
		firstBcu = bcuId;
		lastBcu = bcuId;
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "sendMultiBcuCommand: impossible to send a bcu message with code %d", bcuId);
		throw new CommandSendingException();
	}

	sendMultiBcuCommand(priority, firstBcu, lastBcu, firstDSP, lastDSP, 
					    opcode, address, datalen, data, 
						timeout_ms, flags);
}

