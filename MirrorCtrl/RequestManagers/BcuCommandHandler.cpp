#include "BcuLib/BcuCommon.h"
using namespace Arcetri::Bcu;

#include "BcuCommandHandler.h"


// --------------------- PUBLIC ---------------------- //

BcuCommandHandler::BcuCommandHandler(int id, int bcuStart, int bcuNum, int priority, int logLevel):AbstractBcuRequestManager("BCUCOMMANDHANDLER", id, priority, logLevel) {
    // All the work is done by AbstractBcuRequestManager constructor !!!
    
    initBcuPool(bcuStart, bcuNum);
}

BcuCommandHandler::~BcuCommandHandler() {
    // All the work is done by AbstractBcuRequestManager destructor !!!
}


int BcuCommandHandler::handleBcuRequest(MsgBuf* reqMsgBuf) {
	
	_logger->log(Logger::LOG_LEV_INFO, "----------------------------------------------------------------------------------");
	_logger->log(Logger::LOG_LEV_INFO, "BcuCommandHandler %s handling request...", getObjId().c_str());
	
	int ret;

	// Get the BcuRequest from the msgBuf
	BcuMessage bcuMsg;
	memcpy(&bcuMsg, MSG_BODY(reqMsgBuf), sizeof(BcuMessage));

	BcuRequest request(&bcuMsg);

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Received BCU command: bcu %d-%d, dsp %d-%d, opcode %d, address %d, datasize %d, shmid %d, flags %d", bcuMsg._firstBcu, bcuMsg._lastBcu, bcuMsg._firstDsp, bcuMsg._lastDsp, bcuMsg._opCode, bcuMsg._dataAddress, bcuMsg._dataSize, bcuMsg._shmId, bcuMsg._flags);

	
	try {
		// Create the BcuRequestInfo, also setting the number of 
		// expected reply packets.
		// !!! This attach to the client shm !!!
		setBcuRequestInfo(&request);	 // can throw InvalidSharedMemoryException or AbstractBcuRequestManagerException

		// Mega unpacking and send (made by AbstractBcuRequestManager superclass !!!)
	    sendBcuRequest();	// Can throw AbstractBcuRequestManagerException or MirrorControllerShutdownException    
	    
	    // Wait for the reply (made by AbstractBcuRequestManager superclass !!!)
	    // This also send a message to the client to let it know about the 
	    // estimated timeout
	    waitBcuReply();		// Can throw MirrorControllerShutdownException
	    
	    // --- DEBUG ---
//		BYTE* shmBuf = _currentRequestInfo->getDataAddress();
//		int dspNum = (request->getLastDsp()-request->getFirstDsp())/request->getDspFactor()+1;
//		_logger->log(Logger::LOG_LEV_DEBUG, "BcuCommandHandler::handleBcuRequest: Shared memory (%x) content:", shmBuf);
//	    if(_logger->getLevel() >= Logger::LOG_LEV_DEBUG) {
//	    	for(int i=0; i<request->getDataSizeBytes()*dspNum; i++) {
//	    		printf(" %X", shmBuf[i]);
//	    	}
//	    	printf("\n");
//	    }
	    // -------------
	    
	    // Handle the reply
	    handleBcuReply(reqMsgBuf);	// Can throw BcuCommandHandlerException or MirrorControllerShutdownException 
		
		// Clear the request info and the request
		clearBcuRequestInfo();		// !!! This detach from the client shm !!!
		ret = NO_ERROR;
	}
	catch(InvalidSharedMemoryException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, "Client timed-out: impossible to attach to the shared memory !");
		_logger->log(Logger::LOG_LEV_ERROR, "BcuCommandHandler::handleBcuRequest: REQUEST FAILED");
		// Note that, if here, the BcuRequestInfo haven't been attached to shm
		thRelease(reqMsgBuf);
		ret =  CMD_FAULT_ERROR;
	}
	catch(MirrorControllerShutdownException& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "BcuCommandHandler with priority %d shutting down...", _priority);
		// NOTE: Here it could handle the reply to MsgD, but probably is ok the client timeout 
		// which automatically expires.
		thRelease(reqMsgBuf);
		ret =  CMD_FAULT_ERROR;
	}
	catch(BcuCommandHandlerException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, e.what().c_str());
		_logger->log(Logger::LOG_LEV_ERROR, "BcuCommandHandler::handleBcuRequest: REQUEST FAILED");
		clearBcuRequestInfo();	
		thRelease(reqMsgBuf);
		ret =  CMD_FAULT_ERROR;
	}
	catch(AbstractBcuRequestManagerException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, e.what().c_str());
		_logger->log(Logger::LOG_LEV_ERROR, "BcuCommandHandler::handleBcuRequest: REQUEST FAILED");
		clearBcuRequestInfo();	
		thRelease(reqMsgBuf);
		ret =  CMD_FAULT_ERROR;
	}
	
	_logger->log(Logger::LOG_LEV_INFO, "----------------------------------------------------------------------------------");
	return ret;
}


// --------------------- PRIVATE ---------------------- //


void BcuCommandHandler::handleBcuReply(MsgBuf* reqMsgBuf) throw (BcuCommandHandlerException, MirrorControllerShutdownException) {
    //-->//_logger->log(Logger::LOG_LEV_INFO, "BcuCommandHandler handling reply...");
    
    int resCode;
    
    // Set the status of the BcuMessage depending on BCuRequest result
    if(_currentRequestInfo->isFailed()) {
    	// This can happen in 2 cases:
    	//	- At least one reply packet have opCode == FAULT 
    	//  - At least one reply packet hasn't come back
    	resCode = NAK;
    	_logger->log(Logger::LOG_LEV_ERROR, "BcuCommandHandler::handleBcuReply: REQUEST COMPLETED but FAILED :-(");
    }
    else {
    	resCode = ACK;
    	_logger->log(Logger::LOG_LEV_INFO, "BcuCommandHandler::handleBcuReply:  REQUEST SUCCESFULLY COMPLETED :-)");
    }
    
   	thReplyMsg(resCode, 0, NULL, reqMsgBuf);
}
