#include "AbstractBcuRequestManager.h"
#include "MirrorCtrlExceptions.h"
#include "TimeToDie.h"


// --------------------- PUBLIC ---------------------- //

AbstractBcuRequestManager::AbstractBcuRequestManager(string concreteClassName, int id, int priority, int logLevel) {
     
    _bcuStart = 0;
    _bcuNum = 0;
    _bcuPoolInitialized = false;
     
    _concreteClassName = concreteClassName;
    _objId = Utils::itoa(id) + "_P" + Utils::itoa(priority);
    _priority = priority;
    
    // Get the logger
    _logger =  Logger::get(_concreteClassName+"_"+_objId, logLevel);
    _logger->log(Logger::LOG_LEV_INFO, "---------------------------------------------------------------------------------------");
    _logger->printStatus();
    _logger->log(Logger::LOG_LEV_INFO, "Constructing %s [ID:%s] with %s...", _concreteClassName.c_str(), _objId.c_str(), Priority::getPriorityDescription(_priority).c_str());
    
    _currentRequestInfo = new BcuRequestInfo();
}

AbstractBcuRequestManager::~AbstractBcuRequestManager() {
	_logger->log(Logger::LOG_LEV_INFO, "Destroying %s [ID:%s] with %s...", _concreteClassName.c_str(), _objId.c_str(), Priority::getPriorityDescription(_priority).c_str());
	
	for(int i=0; i<_bcuNum; i++) {
		delete _outputQueuePool[i];
	}
	_outputQueuePool.clear();
	
	delete _currentRequestInfo;
	_logger->log(Logger::LOG_LEV_INFO, "%s [ID:%s] with %s succesfully destroyed", _concreteClassName.c_str(), _objId.c_str(), Priority::getPriorityDescription(_priority).c_str());
}

BcuPacketQueue* AbstractBcuRequestManager::getOutputQueue(int bcu) {
    return _outputQueuePool[bcu];
}


// --------------------- PROTECTED ---------------------- //

void AbstractBcuRequestManager::initBcuPool(int bcuStart, int bcuNum) {
	
	_bcuStart = bcuStart;
	_bcuNum = bcuNum;
	
	// Init the pool of output-queues
	if(_bcuNum > 0) {
		_logger->log(Logger::LOG_LEV_INFO, "Attaching %s [ID:%s] with %s to %d BCUs...", _concreteClassName.c_str(), _objId.c_str(), Priority::getPriorityDescription(_priority).c_str(), _bcuNum);
		for(int i=0; i<_bcuNum; i++) {
	    	BcuPacketQueue* tmpQueuePtr = new BcuPacketQueue(_priority);
	        _outputQueuePool.push_back(tmpQueuePtr);
    	}
    	_bcuPoolInitialized = true;
	}
	else {
		_logger->log(Logger::LOG_LEV_WARNING, "%s [ID:%s] should be attached to 1 Bcu or more...", _concreteClassName.c_str(), _objId.c_str());
	} 
}

void AbstractBcuRequestManager::setBcuRequestInfo(BcuRequest* request, BYTE* dataAddress) throw(InvalidSharedMemoryException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "Setting BcuRequestInfo for %s [%s]", getClassName().c_str(), getObjId().c_str());
	_currentRequestInfo->set(request, dataAddress);	
}

void AbstractBcuRequestManager::sendBcuRequest() throw (AbstractBcuRequestManagerException, MirrorControllerShutdownException) {
	
	if(!_bcuPoolInitialized) {
		_logger->log(Logger::LOG_LEV_WARNING, "Bcu pool of %s [ID:%s] not initialized", _concreteClassName.c_str(), _objId.c_str());
		throw AbstractBcuRequestManagerException("AbstractBcuRequestManagerException", "Uninitialized Bcu pool");
	}
	
	BcuRequest* request = _currentRequestInfo->getBcuRequest();
	
	// Pointers to data to read/write from/to each BCU
	BYTE* shmBuf = _currentRequestInfo->getDataAddress();
	
	// Size of data to read/write from/to each BCU: the "shmBuf" has size equals
	// to bcuDataSize*bcuNum
	// Note that read/write are always *SEQ* over BCUs !!!
	int bcuDataSize = request->getDataSizeBytes() * ((request->getLastDsp()-request->getFirstDsp())/request->getDspFactor()+1);
	
	// --- DEBUG ---
//	int dspNum = (request->getLastDsp()-request->getFirstDsp())/request->getDspFactor()+1;
//	_logger->log(Logger::LOG_LEV_TRACE, "sendBcuRequest: Shared memory (%x) content:", shmBuf);
//    if(_logger->getLevel() >= Logger::LOG_LEV_TRACE) {
//    	for(int i=0; i<request->getDataSizeBytes()*dspNum; i++) {
//    		printf(" %X", shmBuf[i]);
//    	}
//    	printf("\n");
//    }
    // -------------
	
	// Init outgoing Bcu Packets info
	uint madePackets = 0;		// Total number of made packets
	int madePacketsSize = 0;	// Size in DWORD of the all packet sent for each pair (BCU,DSP)
	
	// Loop over all data to READ/WRITE: this loop manage the packing process over
	// the Dsp/Sdram/... remote memory, so the loop will modify:
	// 	- "_dataAddress" field of the BcuRequest: is the pointer to the remote memory
	//	- "shmBuf" pointer: is the pointer to the local memory
	bool firstPacket = true;
	int reqPacketSize = 0;		// Size of the current packet to send (DWORDS)
	while(madePacketsSize < request->getDataSizeDw()) {
		
		// --- FIRST PACKET not-aligned --- //
		// If:
		// 	- Data address is not aligned to multiple of 4 DWORD, AND
		//  - Data size is greater than 8 DWORDS
		// I need to create a first small packet (size = 1..3 DWORD) to
		// realign the start address.
		if(firstPacket && (request->getDataAddressDw() & 0x03) && (request->getDataSizeDw() >= 8) ) {
		   	
			reqPacketSize = 4 - (request->getDataAddressDw() & 0x03);	// Is 1, 2 or 3 DWORDS
			handleMultiDspAndBcu(request, reqPacketSize, shmBuf, bcuDataSize, &madePackets);
	
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
			
			handleMultiDspAndBcu(request, reqPacketSize, shmBuf, bcuDataSize, &madePackets);
		}
		
		//--- LAST PACKET --- //
		// Can be not aligned but is smaller than 4 dwords (see previous step)
		else if(madePacketsSize < request->getDataSizeDw()) {
			reqPacketSize = request->getDataSizeDw() - madePacketsSize;
			handleMultiDspAndBcu(request, reqPacketSize, shmBuf, bcuDataSize, &madePackets);
		}
		
		// Updates the current status
		request->increaseDataAddressDw(reqPacketSize);			// Bcu mem address to read/write
		shmBuf += reqPacketSize*Constants::DWORD_SIZE;			// Shared memory buffer to write/read
		madePacketsSize += reqPacketSize;						// Size of made packets for each pair (BCU,DSP)
	}
	
	//!!!//_logger->log(Logger::LOG_LEV_DEBUG, "AbstractBcuRequestManager::sendBcuRequest - Sent packets:      %d", madePackets);
	//!!!//_logger->log(Logger::LOG_LEV_TRACE, "AbstractBcuRequestManager::sendBcuRequest - Sent packets size: %d dword", madePacketsSize);
}


void AbstractBcuRequestManager::handleMultiDspAndBcu(BcuRequest* request, int reqPacketSize, BYTE* shmBuffer, int bcuDataSize, uint* madePackets) throw(AbstractBcuRequestManagerException) {
	
	BcuPacket* tempPacket; //For debug purpose only
	
	//------------------------//
	//--- COMMAND WRSAME_* ---//
	// The SAME is considered both for BCU and for DSP !!! See (*) above !!!
	if(request->isSameOpcode() && request->isWriteOpcode()) {
		// Loop over requested BCUs
		for(int i=request->getFirstBcu()-_bcuStart; i<=request->getLastBcu()-_bcuStart; i++) {
			// Create the BcuPacket and insert it in the correct output-queue
			tempPacket = new BcuPacket(0, request->getFirstDsp(), request->getLastDsp(),
							  		   request->getOpCode(), 
							  		   request->getDataAddressDw(), // Has been updated in the calling method !!!
							  	 	   request->getFlags(), 
							  		   reqPacketSize,				// This is the real size of data write
							  		   								// to the Dsp/Sdram/...
							  		   shmBuffer, 	
							  		   _currentRequestInfo);
							  		   
			_outputQueuePool[i]->push(tempPacket);
			(*madePackets)++;
			// Increase to the next BCU buffer 
			// (*) Removed to perform SAME also in BCU !!!
			//shmBuffer+=bcuDataSize;
		}
	}	
	//-----------------------------------//
	//--- COMMANDS RDSEQ_* or WRSEQ_* ---//
	// The SEQ is considered both for BCU and for DSP !!!
	else if(request->isSeqOpcode() && (request->isWriteOpcode() || request->isReadOpcode())) {
		
		int firstDsp, lastDsp;
		int maxLastDsp;
		
		// Number of dsp handled by a single command (depends on request->_opCode)
		int dspFactor = request->getDspFactor();
		
		// Number of DSP to which I can send "reqPacketSize" data to fill
		// a BcuPacket
		int dspIncrement = (Constants::MAX_PACKET_SIZE_DW/reqPacketSize) * dspFactor; 
		
		// --- Loop over requested BCUs --- //
		for(int i=request->getFirstBcu()-_bcuStart; i<=request->getLastBcu()-_bcuStart; i++) {
			// --- Loop over DSPs increasing by "dspIncrement" --- //
			for(int j=request->getFirstDsp(); j<=request->getLastDsp(); j+=dspIncrement) {
				
				// Currently addressed DSPs
				firstDsp = j;
				maxLastDsp = j+dspIncrement-dspFactor;
				if(maxLastDsp > request->getLastDsp()) {
					lastDsp = request->getLastDsp();
				}
				else {
					lastDsp = maxLastDsp;
				}
				
				// Create the BcuPacket and insert it in the correct output-queue
				tempPacket = new BcuPacket(0, firstDsp, lastDsp,
								  		   request->getOpCode(), 
								  		   request->getDataAddressDw(),	// Has been updated in the calling method !!!
								  		   request->getFlags(), 
								  		   reqPacketSize,				// The correct size (depending on the number of
								  		   								// addressed Dsp) will be handled by BcuPacket !!!		 
								  		   shmBuffer, 
								  		   _currentRequestInfo);
				
				_outputQueuePool[i]->push(tempPacket);
								
				// Increment the number of made packets
				(*madePackets)++;
			} // End loop DSPs
			// Increase to the next BCU buffer
			shmBuffer+=bcuDataSize;
		} // End loop BCUs
	}
}


void AbstractBcuRequestManager::waitBcuReply() throw (MirrorControllerShutdownException) {
	 
	_logger->log(Logger::LOG_LEV_DEBUG, "Request flags: %d (WANTREPLY=%d, ASQUADWORD=%d)", _currentRequestInfo->getBcuRequest()->getFlags(), Constants::WANTREPLY_FLAG, Constants::ASQUADWORD_FLAG);
	
	if(_currentRequestInfo->getBcuRequest()->getFlags() && Constants::WANTREPLY_FLAG) {
		_logger->log(Logger::LOG_LEV_DEBUG, "--> Waiting reply...");
	}
	else {
		_logger->log(Logger::LOG_LEV_DEBUG, "--> Reply not requested: waiting all packets cleared (data in use)...");
	}
	
	// Wait for the complete reply: the timout is set to allow
    // the detection of a shutdown request while waiting.
    // ***Note*** that if a reply is not expected, it waits for packets succesfully
    // sent and cleared by BcuCommunicator: this is needed to avoid shm detached
    // before all packets are sent (data in use).
    int ret = -1;
    int timeout_ms = 500;
    while(ret != 0 && !(TimeToDie::check())) {
    	ret = _currentRequestInfo->waitReplyReady(timeout_ms);
    };
}


