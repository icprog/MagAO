// Me
#include "BcuCommandHandlerTest.h"

//Commons
#include "Commons/TimeToDie.h"


// --------------------- PUBLIC ---------------------- //

BcuCommandHandlerTest::BcuCommandHandlerTest(int id, int bcuNum, int priority, int packetNum):AbstractBcuRequestManager("BcuCommandHandler", id, priority, Logger::LOG_LEV_TRACE) {
    
    _logger = Logger::get("BCUCOMMANDHANDLER_TEST_"+Utils::itoa(id));
    _packetNum = packetNum;
}

BcuCommandHandlerTest::~BcuCommandHandlerTest() {
    // [todo]
}

pthread_t BcuCommandHandlerTest::start() {
	pthread_t bcuComHdlrTest;
  	pthread_create(&bcuComHdlrTest, NULL, &execute, (void*)this);
  	_logger->log(Logger::LOG_LEV_DEBUG, "BcuCommandHandlerTest started !");
	return bcuComHdlrTest;
}

void* BcuCommandHandlerTest::execute(void *pthis) {
	BcuCommandHandlerTest* bcuCmdHdl = (BcuCommandHandlerTest*)pthis;
    bcuCmdHdl->run();
    return 0;
}


// --------------------- PRIVATE ---------------------- //

void BcuCommandHandlerTest::run() {
	
	// Set a Real-Time priority for this thread
	//Utils::setSchedulingRR(99);
	
	 _logger->log(Logger::LOG_LEV_DEBUG, "BcuCommandHandlerTest running...");
 
	// Allocate a single packet space for each BcuCommunicator
	int packetSize = 1456;
    BYTE* inDataBuffer[_bcuNum];
    for(int i=0; i<_bcuNum; i++) {
    	inDataBuffer[i] = new BYTE[packetSize];
    }
    
    
    // START TIME
    struct timeval start, end;
    gettimeofday(&start,NULL);
    
    _currentRequestInfo = new BcuRequestInfo(0, _bcuNum-1, _packetNum);
    
    int i = 1;
    
    try { 
	    // Outputs packets if not asked to die
	    while(!(TimeToDie::check()) && i <= _packetNum) {
	    	// Fill the queue for each BcuCommunicator
	    	for(int j=0; j<_bcuNum; j++) {
	    		_outputQueuePool[j]->push(new BcuPacket(0, 1, 1, Bcu::OpCodes::MGP_OP_RDSEQ_SDRAM, 0x106640, BcuPacket::WANTREPLY_FLAG, packetSize/4, inDataBuffer[j], _currentRequestInfo));
	    	}
	    	i++;
	    }
	
	    // Wait for the reply and handle it: waitBcuReply takes care of "time to die"
	    waitBcuReply();
	    
	     _logger->log(Logger::LOG_LEV_INFO, "BcuCommandHandlerTest terminated !!!");
	     
	     // END TIME
	     gettimeofday(&end,NULL);
	     
	     // Statistics
	     double time_ms = ((end.tv_usec-start.tv_usec)/1000.0 + (end.tv_sec - start.tv_sec)*1000.0);
	     _logger->log(Logger::LOG_LEV_INFO, "--------- BCU COMMAND HANDLER TEST STATISTICS --------");
	      _logger->log(Logger::LOG_LEV_INFO, "USED BCU: %d", _bcuNum);
	 	 _logger->log(Logger::LOG_LEV_INFO, "TOTAL TIME ELAPSED time: %.0f ms", time_ms);
	 	 double transferredMBits = (_packetNum/1024.0*packetSize/1024.0*8.0*_bcuNum);
	 	 _logger->log(Logger::LOG_LEV_INFO, "TOTAL DATA TRANSFER: %.0f Mb", transferredMBits);
		 double throughput = (transferredMBits/(time_ms/1000));
		 _logger->log(Logger::LOG_LEV_INFO, "TOTAL THROUGHPUT: %f Mb/s", throughput);
	}
	catch(MirrorControllerShutdownException& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "%s", e.what().c_str());
		_logger->log(Logger::LOG_LEV_WARNING, "BcuCommandHandlerTest shutting down...");
	}
}

