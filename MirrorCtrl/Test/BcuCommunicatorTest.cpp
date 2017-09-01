
#include <ctime>
#include <sys/time.h>
using namespace std;

#include "BcuCommunicatorTest.h"
#include "stdconfig.h"
#include "Utils.h"
using namespace Arcetri;


BcuCommunicatorTest::BcuCommunicatorTest(int maxBcuQueueSize, int packetNum, int packetSize, int localPort, int bcuId, string bcuIpAddress, int bcuPort, int receiveTimeout_ms):
					 BcuCommunicator(localPort, bcuId, bcuIpAddress, bcuPort, maxBcuQueueSize, receiveTimeout_ms) {
					 
        _BCU_QUEUE_MAX_SIZE = maxBcuQueueSize;
		_bcuQueueSize = 0;
		_packetNum = packetNum;
		_packetSize = packetSize;
}
					 
BcuCommunicatorTest::~BcuCommunicatorTest() {}

// Returns the throughput in Mb/s
pthread_t BcuCommunicatorTest::start() {
	pthread_t bcuCom;
  	pthread_create(&bcuCom, NULL, &execute, (void*)this);
	return bcuCom;
}


void* BcuCommunicatorTest::execute(void* pthis) {
	
    BcuCommunicatorTest* bcuCom = (BcuCommunicatorTest*)pthis;
    Logger::get("BCUCOMMUNICATOR_"+bcuCom->getBcuId())->log(Logger::LOG_LEV_INFO, "BcuCommunicatorTest %s succesfully started !!!", bcuCom->info().c_str());
	
	bcuCom->testSendReceiveSampleStream();
	
	Logger::get("BCUCOMMUNICATOR_"+bcuCom->getBcuId())->log(Logger::LOG_LEV_INFO, "BcuCommunicatorTest %s terminated", bcuCom->info().c_str());
	return NULL;
}


void BcuCommunicatorTest::testSendSampleBcuPacket(uint8 msgId) throw (UdpException) {

	// -- CREATION
	BcuPacket* p = new BcuPacket();
	//_logger->log(Logger::LOG_LEV_TRACE, "<--- Creating sample Udp Packet --->");
    p->set(msgId, 1, 1, Bcu::OpCodes::MGP_OP_RDSEQ_SDRAM, 0x106640, 1, 0, _packetSize/sizeof(DWORD), NULL, NULL); 
	
	//_logger->log(Logger::LOG_LEV_TRACE, "Header: %s", p->info());
	
	// -- SENDING
	//_logger->log(Logger::LOG_LEV_TRACE, "<--- Sending sample Udp Packet --->");
	UdpConnection* conn = getBcuConn();
	//_logger->log(Logger::LOG_LEV_TRACE, "Connection got");
	p->send(conn);
	//_logger->log(Logger::LOG_LEV_TRACE, "Packet sent");
	delete p;
}

void BcuCommunicatorTest::testReceiveSampleBcuPacket(BYTE* receivedData) throw (UdpException) {

	//_logger->log(Logger::LOG_LEV_TRACE, "<--- Testing receive sample Udp Packet --->");
	BcuPacket* rp = new BcuPacket();
    rp->setData(receivedData, _packetSize/sizeof(DWORD));
	
	rp->receive(getBcuConn());
	//_logger->log(Logger::LOG_LEV_INFO, "Packet received: %s", rp->info());
	delete rp;
}

void BcuCommunicatorTest::testSendReceiveSampleStream() {
	
	// Allocate memory for the replies (only for one to save space !!!)
	BYTE replyBuffer[_packetSize];
	
	_logger->log(Logger::LOG_LEV_INFO, "Sending %d packets...", _packetNum);
	
	int sent = 0;
	int received = 0;
	
	long int nextOutId = 0;
	int loops = 0;
	
	while(nextOutId<_packetNum) {
		// Try to fill the output queue
		while(nextOutId<_packetNum && _bcuQueueSize<_BCU_QUEUE_MAX_SIZE) {
			try {
				testSendSampleBcuPacket(nextOutId % 256);
				_bcuQueueSize++;
				sent++;
			} catch (UdpException& e) {
				//Do nothing, simply skip the packet
				_logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
			} 
			nextOutId++;
		}
		
		// Try to receive a packet
		try {
		    testReceiveSampleBcuPacket(replyBuffer);
		    received++;
		    _bcuQueueSize--;
		} catch (UdpException& e) {
			// It's timed-out
			//_mainLogger->log(Logger::LOG_LEV_WARNING, "Packet lost");
		    _bcuQueueSize--;   
		     //Should retry !!!
		} 
		loops++;
	}
	
	// Receive remaining packets
	while(_bcuQueueSize) {
		try {
			testReceiveSampleBcuPacket(replyBuffer);
			received++;
			_bcuQueueSize--;	
		} catch (UdpException& e) {
			// It's timed-out
			//_mainLogger->log(Logger::LOG_LEV_WARNING, "Packet lost");
		    _bcuQueueSize--;
		    //Should retry !!!
		} 
	}
	
	_logger->log(Logger::LOG_LEV_INFO, "%d loops done to send %d and receive %d packets", loops, sent, received);
}


int main() {	
	
	Logger* logger = Logger::get(Logger::LOG_LEV_INFO);
	logger->printStatus();
	
	//Utils::setSchedulingRR(46);
	
	// Get config from file
	int logLevel;
	int threadsNum;
    int maxBcuQueueSize; 
	int packetNum, packetSize;
	int bcuPort, localPort;
	string bcuIpAddress;
	int receiveTimeout_ms;
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
	}
	catch(Config_File_Exception& e) {
		logger->log(Logger::LOG_LEV_ERROR, "Error in BcuCommunicatorTest config file: %s", e.what().c_str());
		return -1;
	}
	
	logger->setLevel(logLevel);
	logger->printStatus();
	
	// Execute tests
	logger->log(Logger::LOG_LEV_INFO, "--- BCU COMMUNICATOR TEST ---");
	logger->log(Logger::LOG_LEV_INFO, "Threads number: %d", threadsNum);
    logger->log(Logger::LOG_LEV_INFO, "Bcu queue size: %d", maxBcuQueueSize);
	logger->log(Logger::LOG_LEV_INFO, "Packets number: %d", packetNum);
	logger->log(Logger::LOG_LEV_INFO, "Packets size: %d", packetSize);
	logger->log(Logger::LOG_LEV_INFO, "Local port: %d", localPort);
	logger->log(Logger::LOG_LEV_INFO, "Bcu Ip: %s", bcuIpAddress.c_str());
	logger->log(Logger::LOG_LEV_INFO, "Bcu port: %d", bcuPort);
	logger->log(Logger::LOG_LEV_INFO, "Receive timeout: %d ms", receiveTimeout_ms);
    
    // Start time
    struct timeval start, end;
    gettimeofday(&start,NULL);
		
	BcuCommunicatorTest* bcuCommunicators[threadsNum];
	pthread_t bcuThreads[threadsNum];
	for(int i=0; i<threadsNum; i++) {
		bcuCommunicators[i] = new BcuCommunicatorTest(maxBcuQueueSize, packetNum, packetSize, localPort+i, i, bcuIpAddress, bcuPort+i, receiveTimeout_ms);
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
    double time = (end.tv_usec-start.tv_usec) + (end.tv_sec - start.tv_sec)*1000000;
 	logger->log(Logger::LOG_LEV_INFO, "TOTAL TIME ELAPSED time: %.0f ms", time/1000);
 	logger->log(Logger::LOG_LEV_INFO, "TOTAL DATA TRANSFER: %.0f Mb", packetNum*packetSize*8.0*threadsNum/1000000);
	double throughput = ((packetNum*packetSize*8.0*threadsNum)/time);
	logger->log(Logger::LOG_LEV_INFO, "TOTAL THROUGHPUT (%d BcuCommunicators): %.0f Mb/s", threadsNum, throughput);
	
	logger->log(Logger::LOG_LEV_INFO, "--- TEST SUCCESFULLY TERMINATED ---");
	return 0;
}
