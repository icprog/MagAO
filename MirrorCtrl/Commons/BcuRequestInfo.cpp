#include "BcuRequestInfo.h"

#include "Logger.h"
#include "BcuLib/BcuCommon.h"
using namespace Arcetri;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>
#include <sys/time.h>
#include <sys/shm.h>
#include <list>
using namespace std;


BcuRequestInfo::BcuRequestInfo() {
	_bcuRequest = NULL;
	_bcuRequestFailed = false;
	pthread_mutex_init(&_mutex_replyReady, NULL);
    pthread_cond_init(&_condition_replyReady, NULL);
}


// For test purposes only
BcuRequestInfo::BcuRequestInfo(int bcuFrom, int bcuTo, int expectedReplies) {
	
	_bcuRequest = NULL;
	_bcuRequestFailed = false;
    _replyReady = bcuTo - bcuFrom + 1;
    for(int i=bcuFrom; i<=bcuTo; i++) {
        _replyCounters[i] = expectedReplies;
    }
    pthread_mutex_init(&_mutex_replyReady, NULL);
    pthread_cond_init(&_condition_replyReady, NULL);
}


BcuRequestInfo::~BcuRequestInfo() {
	//-->//Logger::get()->log(Logger::LOG_LEV_INFO, "Destroying BcuRequestInfo...");
	clear();
	//pthread_mutex_destroy(&_mutex_replyReady);	// [to check] Can't do this: it locks here !!!
    //pthread_cond_destroy(&_condition_replyReady);
    //-->//Logger::get()->log(Logger::LOG_LEV_INFO, "BcuRequestInfo succesfully destroyed !!! ");
}


void BcuRequestInfo::set(BcuRequest* bcuReq, BYTE* dataAddress) throw(InvalidSharedMemoryException) {
	_bcuRequest = bcuReq;
	_bcuRequestFailed = false;

    int dspNum = (_bcuRequest->getLastDsp()-_bcuRequest->getFirstDsp())/_bcuRequest->getDspFactor()+1;
    int bcuNum = _bcuRequest->getLastBcu()-_bcuRequest->getFirstBcu()+1;
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "BcuRequestInfo initialized:");
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "> OpCode:      %d", _bcuRequest->getOpCode());
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "> Flags:       %d", _bcuRequest->getFlags());
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "> Bcu:         %d first, %d last", _bcuRequest->getFirstBcu(), _bcuRequest->getLastBcu());
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "> Dsp:         %d first, %d last", _bcuRequest->getFirstDsp(), _bcuRequest->getLastDsp());
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "> Address:     %d dword, %d bytes", _bcuRequest->getDataAddressDw(), _bcuRequest->getDataAddressBytes());
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "> Size:        %d dword, %d bytes", _bcuRequest->getDataSizeDw(), _bcuRequest->getDataSizeBytes());
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "> Full size:   %d dword, %d bytes", _bcuRequest->getDataSizeDw()*dspNum*bcuNum, _bcuRequest->getDataSizeBytes()*dspNum*bcuNum);
	
	// Initialize the dataAddress field: note that, if both shmId and dataAddress
	// are specified, the shmId is used.
	if(_bcuRequest->getShmId() != -1) {
		// ATTENTION: attach to the shared memory
		_dataAddress = (BYTE*)shmat(_bcuRequest->getShmId(), 0, 0);
      Logger::get()->log(Logger::LOG_LEV_DEBUG, "shm ID: %d", _bcuRequest->getShmId());
		if(_dataAddress == (BYTE*)-1) {
			throw InvalidSharedMemoryException();
		}

	}
	else {
		_dataAddress = dataAddress;
	}
	
	// Initialize the reply management
	_replyReady = _bcuRequest->getLastBcu() - _bcuRequest->getFirstBcu() + 1;
	int expectedReplies = computeBcuPacketNum(*_bcuRequest); // This uses the BcuRequest copy constructor !!!
	for(int i=_bcuRequest->getFirstBcu(); i<=_bcuRequest->getLastBcu(); i++) {
        _replyCounters[i] = expectedReplies;
    }
    
    // Print for debug
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "> Packets:     %d (for each BCU)", expectedReplies);
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "> Packets tot: %d", expectedReplies*_replyReady);
   	if(Logger::get()->getLevel() >= Logger::LOG_LEV_TRACE) {
   		int bcuDataSize;
	    if(_bcuRequest->isSameOpcode()) {
	    	bcuDataSize = _bcuRequest->getDataSizeBytes();
	    }
	    else {
	    	bcuDataSize = _bcuRequest->getDataSizeBytes()*dspNum;
	    }
		printf("> Data memory (address %x, size %d bytes) content:\n", (uint)_dataAddress, bcuDataSize*bcuNum);
		for(int bcu=0; bcu<bcuNum; bcu++) {
			printf("[BCU %d]: ", bcu+_bcuRequest->getFirstBcu());
    		for(int i=0; i<(bcuDataSize/Constants::DWORD_SIZE); i++) {
    			if(_bcuRequest->isSameOpcode()) {
    				printf(" %08X ", ((unsigned int*)_dataAddress)[i]);
    			}
    			else {
    				printf(" %08X ", ((unsigned int*)_dataAddress)[i+bcu*bcuDataSize/Constants::DWORD_SIZE]);
    			}
    		}
    		printf("\n");
		}
		printf("\n");
    }
}


void BcuRequestInfo:: clear() {
	
	// If a BcuRequest is set, detach it from shm
    if(_bcuRequest != NULL) {
	    // Detach from shm
		if(_bcuRequest->getShmId() != -1) {
			// ATTENTION: detach from the shared memory
			shmdt(_dataAddress);
			shmctl(_bcuRequest->getShmId(), IPC_RMID, NULL);
			Logger::get()->log(Logger::LOG_LEV_DEBUG, "BcuRequestInfo cleared (detached from shm) !!!");
		}
		
		// Clear fields
		_bcuRequest = NULL;
		_bcuRequestFailed = false;
	    _replyReady = 0;
	    _replyCounters.clear();
		
		
	}
	// Else do nothing
	else {
		//-->//Logger::get()->log(Logger::LOG_LEV_DEBUG, "-->> BcuRequestInfo already cleared, OK.");
	}
}


int BcuRequestInfo::decreaseReplyCounter(int bcu) {
    _replyCounters[bcu]--;
    int newValue = _replyCounters[bcu];
    if(newValue == 0) {
    	//!!!//Logger::get()->log(Logger::LOG_LEV_INFO, "---[ All packets from Bcu %d ready !!! ]---", bcu);
        pthread_mutex_lock(&_mutex_replyReady);
	    _replyReady--;
	    if(_replyReady == 0) {
	        pthread_cond_signal(&_condition_replyReady);
	    }
	    pthread_mutex_unlock(&_mutex_replyReady);
    } 
    return newValue;
}


int BcuRequestInfo::waitReplyReady(int timeout_ms) {
	
	//!!!//for(int i=_bcuRequest->getFirstBcu(); i<=_bcuRequest->getLastBcu(); i++) {
		//!!!//Logger::get()->log(Logger::LOG_LEV_INFO, "Waiting %d replies from Bcu %d", _replyCounters[i], i);
	//!!!//}
	
	// Init the timeout: because must be an absolute timeout, 
	// I need to get the current time and add the desired timeout !!!
	struct timeval nowTime;
	struct timespec timeout;
	
	int ret = 0;
    pthread_mutex_lock(&_mutex_replyReady);
    gettimeofday(&nowTime, NULL);
	timeout.tv_nsec = (nowTime.tv_usec*1000);	
	timeout.tv_sec = nowTime.tv_sec;
	// Add the timeout
	timeout.tv_sec += timeout_ms/1000;
	timeout.tv_nsec += (timeout_ms%1000)*(int)1E6;
	timeout.tv_sec += timeout.tv_nsec/(int)1E9;
	timeout.tv_nsec = timeout.tv_nsec%(int)1E9;
	
    while(ret == 0 && _replyReady > 0) { 
        ret = pthread_cond_timedwait(&_condition_replyReady, &_mutex_replyReady, &timeout);
        // ATTENTION: can exit from pthread_cond_timedwait also if condition is not changed (see man pages)
        // The while checks the condition again only if not timed-out
    }
    
    pthread_mutex_unlock(&_mutex_replyReady);
	return ret;
}

// The parameter is received by value (using BcuRequest copy constructor)
// because this method will modify the BcuRequest
//
// The algorithm is almost the same as sendBcuPackets() + handleMultiDspAndBcu(...) 
// in AbstractBcuRequestManager class. The only difference is that this doesn't loop
// over the BCUs (because the returned value is the number of packet num wich will 
// be created ***for each Bcu***)
uint BcuRequestInfo::computeBcuPacketNum(BcuRequest bcuReq) {
		
	BcuRequest* request = &bcuReq;
		
	// Init outgoing Bcu Packets info
	uint madePackets = 0;		// Total number of made packets
	int madePacketsSize = 0;	// Size in DWORD of the all packet sent for each pair (BCU,DSP)
	
	bool firstPacket = true;
	int reqPacketSize = 0;		// Size of the current packet to send (DWORDS)
	while(madePacketsSize < request->getDataSizeDw()) {
		
		// --- FIRST PACKET not-aligned --- //
		if(firstPacket && (request->getDataAddressDw() & 0x03) && (request->getDataSizeDw() >= 8) ) {
		   	
			reqPacketSize = 4 - (request->getDataAddressDw() & 0x03);	// Is 1, 2 or 3 DWORDS
			handleMultiDsp(request, reqPacketSize, &madePackets);
	
			firstPacket = false;
		}
		
		// --- OTHER PACKETS (now they are all aligned, except the last) --- //
		else if(madePacketsSize <= request->getDataSizeDw() - 0x03) {
			// Try to send a full packet
			reqPacketSize = Constants::MAX_PACKET_SIZE_DW;
			
			// If remained data are not enough to fill the packet
			if(madePacketsSize + reqPacketSize > request->getDataSizeDw()) {
				reqPacketSize = request->getDataSizeDw()-madePacketsSize;
			}
			// Check for alignment: the last packet can have the same problems as 
			// the first (see above)
			if((reqPacketSize >= 8) && (reqPacketSize & 0x03)) {
				reqPacketSize = reqPacketSize & 0xFFFC;
			}
			
			handleMultiDsp(request, reqPacketSize, &madePackets);
		}
		
		//--- LAST PACKET --- //
		// Can be not aligned but is smaller than 4 dwords (see previous step)
		else if(madePacketsSize < request->getDataSizeDw()) {
			reqPacketSize = request->getDataSizeDw() - madePacketsSize;
			handleMultiDsp(request, reqPacketSize, &madePackets);
		}
		
		// Updates the current status
		request->increaseDataAddressDw(reqPacketSize);		// Bcu mem address to read/write
		madePacketsSize += reqPacketSize;					// Size of made packets for each pair (BCU,DSP)
	}
	
	return madePackets;
}

void BcuRequestInfo::handleMultiDsp(BcuRequest* request, int reqPacketSize, uint* madePackets) {
	//------------------------//
	//--- COMMAND WRSAME_* ---//
	if(request->isSameOpcode() && request->isWriteOpcode()) {
		(*madePackets)++;
	}	
	//-----------------------------------//
	//--- COMMANDS RDSEQ_* or WRSEQ_* ---//
	else if(request->isSeqOpcode() && (request->isWriteOpcode() || request->isReadOpcode())) {
		
		// Number of dsp handled by a single command (depends on request->_opCode)
		int dspFactor = request->getDspFactor();
		
		// Number of DSP to which I can send "reqPacketSize" data to fill
		// a BcuPacket
		int dspIncrement = (Constants::MAX_PACKET_SIZE_DW/reqPacketSize) * dspFactor; 
		
		// --- Loop over DSPs increasing by "dspIncrement" --- //
		for(int j=request->getFirstDsp(); j<=request->getLastDsp(); j+=dspIncrement) {
			(*madePackets)++;
		} // End loop DSPs
	}
}

