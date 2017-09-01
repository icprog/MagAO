
#include <ctime>
#include <sys/time.h>
using namespace std;

#include "BcuCommunicatorTest_rq.h"
#include "stdconfig.h"
#include "Utils.h"
using namespace Arcetri;

;


BcuCommunicatorTest::BcuCommunicatorTest(int maxBcuQueueSize, int packetNum, int packetSize, int localPort, int bcuId, string bcuIpAddress, int bcuPort, int receiveTimeout_ms, int acceptedTimeouts, bool mute):
					 BcuCommunicator(localPort, bcuId, bcuIpAddress, bcuPort, maxBcuQueueSize, receiveTimeout_ms, acceptedTimeouts) {
					 
		_packetNum = packetNum;
		_packetSize = packetSize;
		
		if(mute) {
			_logger->setLevel(Logger::LOG_LEV_INFO);
		}
}
					 
BcuCommunicatorTest::~BcuCommunicatorTest() {}


pthread_t BcuCommunicatorTest::start() throw (UdpFatalException) {
	pthread_t bcuCom;
  	pthread_create(&bcuCom, NULL, &execute, (void*)this);
	return bcuCom;
}


void* BcuCommunicatorTest::execute(void* pthis) throw (UdpFatalException) {
	
    BcuCommunicatorTest* bcuCom = (BcuCommunicatorTest*)pthis;
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "BcuCommunicatorTest %s succesfully started !!!", bcuCom->info().c_str());
	try {
		bcuCom->testSendReceiveSampleStream();
		Logger::get()->log(Logger::LOG_LEV_DEBUG, "BcuCommunicatorTest %s terminated", bcuCom->info().c_str());
	}
	catch(UdpFatalException& e) {
		printf("%s\n", e.what().c_str());
		Logger::get()->log(Logger::LOG_LEV_DEBUG, "BcuCommunicatorTest %s failed", bcuCom->info().c_str());
	}
	return NULL;
}


void BcuCommunicatorTest::testSendReceiveSampleStream() throw (UdpFatalException) {
	
	BYTE* inDataBuffer = new BYTE[_packetSize];
	
	int curId = 0;
	int loops = 0;
	int sent = 0;
	int received = 0;
	
	int receiveResult;
	int receivedUnexpected = 0;
	int timeoutErrors = 0;
	int duplicatesDiscarded = 0;
	
	BcuRequestInfo* myRequest = new BcuRequestInfo(0,0,_packetNum);
	
	_logger->log(Logger::LOG_LEV_INFO, "BcuCommunicatorTest %s sending %d packets", info().c_str(), _packetNum);
	 while (curId < _packetNum ||  _roundQueue->canSend() || _roundQueue->canReceive()) {
		
		// Fills the round queue: note that the msgId is set to 0, but the RoundQueue will set it
		// to an correct value
		while(curId < _packetNum && _roundQueue->canInsert()) {
			_roundQueue->insert(new BcuPacket(0, 1, 1, Bcu::OpCodes::MGP_OP_RDSEQ_SDRAM, 0x106640, BcuPacket::WANTREPLY_FLAG, _packetSize/4, inDataBuffer, myRequest));
			curId++;
		}
		
		// Fills the BCU input queue
		while(_roundQueue->canSend()) {
			_roundQueue->send();
			sent++;  // Here it sends also automatically re-inserted packets !!!
		}
		
		// Receives a packet in the RoundQueue, and eventually recovers timed-out,
		// expected-later or unexpected packets
		if(_roundQueue->canReceive()) {
			receiveResult = _roundQueue->receive();
			switch(receiveResult) {
				case RoundQueue::RECEIVING_OK: 
						received++;
						break;
				case RoundQueue::RECEIVING_UNEXPECTED_OK:
						received++;
						receivedUnexpected++;
						break;
				case RoundQueue::TIMEOUT_RECOVERED:
						timeoutErrors++;
						break;
				case RoundQueue::DUPLICATE_DISCARDED:
						duplicatesDiscarded++;
						break;
			}
		}
		
		loops++;
	}
	
	_logger->log(Logger::LOG_LEV_INFO, "");
	_logger->log(Logger::LOG_LEV_INFO, "----- BCU-COMMUNICATOR %d TRANSFER SUMMARY -----", getBcuId());
	_logger->log(Logger::LOG_LEV_INFO, "Loops: %d", loops);
	_logger->log(Logger::LOG_LEV_INFO, "Sent: %d  |  received: %d", sent, received);
	_logger->log(Logger::LOG_LEV_INFO, "Unexpected (lost/timeout): %d  |  Timeouts: %d  |  duplicates: %d", receivedUnexpected, timeoutErrors, duplicatesDiscarded);
}


int main() {	
	
	Logger* logger = Logger::get(Logger::LOG_LEV_INFO);
	logger->printStatus();
	Utils::setSchedulingRR(99);
	
	// Get config from file
	int logLevel;
	int threadsNum;
    int maxBcuQueueSize; 
	int packetNum, packetSize;
	int bcuPort, localPort;
	string bcuIpAddress;
	int receiveTimeout_ms, allowedTimeouts;
	try {
		Config_File conf("MirrorCtrl/Test/BcuCommunicatorTest.config");
		logLevel = conf["LOG_LEV"];
		threadsNum = conf["COM_NUM"];
        maxBcuQueueSize = conf["MAX_BCU_QUEUE_SIZE"]; 
		packetNum = conf["PACKET_NUM"];
		packetSize = conf["PACKET_SIZE"];
		localPort = conf["LOCAL_PORT"];
		bcuIpAddress = (string)conf["BCU_IP"];
		bcuPort = conf["BCU_PORT"];
		receiveTimeout_ms = conf["RECEIVE_TIMEOUT_MS"];
		allowedTimeouts = conf["ALLOWED_TIMEOUTS"];
	}
	catch(Config_File_Exception& e) {
		logger->log(Logger::LOG_LEV_ERROR, "Error in BcuCommunicatorTest config file: %s", e.what().c_str());
		return -1;
	}
	
	// --- Set verbosity for all modules ---
	// Itself
	logger->setLevel(logLevel);
	logger->printStatus();
	// UdpConnection logger
	Logger::get("UDPCONNECTION")->setLevel(logLevel);

	
	// Execute tests
	logger->log(Logger::LOG_LEV_INFO, "--- MULTI-THREADED BCU COMMUNICATOR with ROUND-QUEUE TEST ---");
	logger->log(Logger::LOG_LEV_INFO, "Threads number: %d", threadsNum);
    logger->log(Logger::LOG_LEV_INFO, "Bcu queue size: %d", maxBcuQueueSize);
	logger->log(Logger::LOG_LEV_INFO, "Packets number: %d", packetNum);
	logger->log(Logger::LOG_LEV_INFO, "Packets size: %d", packetSize);
	logger->log(Logger::LOG_LEV_INFO, "Local port: %d", localPort);
	logger->log(Logger::LOG_LEV_INFO, "Bcu Ip: %s", bcuIpAddress.c_str());
	logger->log(Logger::LOG_LEV_INFO, "Bcu port: %d", bcuPort);
	logger->log(Logger::LOG_LEV_INFO, "Receive timeout: %d ms", receiveTimeout_ms);
	logger->log(Logger::LOG_LEV_INFO, "Allowed timeouts: %d", allowedTimeouts);
    
    // Start time
    struct timeval start, end;
    gettimeofday(&start,NULL);
	
	BcuCommunicatorTest* bcuCommunicators[threadsNum];
	pthread_t bcuThreads[threadsNum];
	for(int i=0; i<threadsNum; i++) {
		bcuCommunicators[i] = new BcuCommunicatorTest(maxBcuQueueSize, packetNum, packetSize, localPort+i, i, bcuIpAddress, bcuPort+i, receiveTimeout_ms, allowedTimeouts, false);
	}
		
	for(int i=0; i<threadsNum; i++) {
		bcuThreads[i] = bcuCommunicators[i]->start();
	}

	for(int i=0; i<threadsNum; i++) {
		 pthread_join(bcuThreads[i], NULL);
	}
	
	for(int i=0; i<threadsNum; i++) {
		delete bcuCommunicators[i];
	}
	
	// End time
	gettimeofday(&end,NULL);
    double time_ms = ((end.tv_usec-start.tv_usec)/1000.0 + (end.tv_sec - start.tv_sec)*1000.0);
    logger->log(Logger::LOG_LEV_INFO, "");
    logger->log(Logger::LOG_LEV_INFO, "--------- MULTI-THREADED BCU COMMUNICATOR STATISTICS --------");
 	logger->log(Logger::LOG_LEV_INFO, "TOTAL TIME ELAPSED time: %.0f ms", time_ms);
 	double transferredMBits = (packetNum/1024.0*packetSize/1024.0*8.0*threadsNum);
 	logger->log(Logger::LOG_LEV_INFO, "TOTAL DATA TRANSFER: %.0f Mb", transferredMBits);
	double throughput = (transferredMBits/(time_ms/1000));
	logger->log(Logger::LOG_LEV_INFO, "TOTAL THROUGHPUT (%d BcuCommunicators): %f Mb/s", threadsNum, throughput);
	
	return 0;
	
}
