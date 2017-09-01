#include "stdio.h"
#include <string>
using namespace std;

#include "BcuSimulator.h"
#include "Commons/BcuPacket.h"

#include "BcuLib/BcuCommon.h"
#include "stdconfig.h"
using namespace Arcetri;


BcuSimulator::BcuSimulator(int localPort, string bcuCommIpAddress, int bcuCommPort) {
	
	_logger = Logger::get();
	
	_conn = new UdpConnection(localPort, bcuCommIpAddress, bcuCommPort, 0);
}

BcuSimulator::~BcuSimulator() {
	delete _conn;
}


pthread_t BcuSimulator::start() {
	pthread_t bcuSim;
  	pthread_create(&bcuSim, NULL, &execute, (void*)this);
	return bcuSim;
}


void* BcuSimulator::execute(void* pthis) {
	
	BcuSimulator* bcuSim = (BcuSimulator*)pthis;
	Logger::get()->log(Logger::LOG_LEV_INFO, "BcuSimulator %s succesfully started !!!", bcuSim->info().c_str());
	bcuSim->simulate();
	Logger::get()->log(Logger::LOG_LEV_INFO, "BcuSimulator %s terminated", bcuSim->info().c_str());
	return NULL;
}

void BcuSimulator::simulate() {
	
	int requestedWords;
	BcuPacket* incomingPacket;
	BYTE* body;

    int curId = 0;
    long int lostId = 0;
		
	while(true) {
		try {
			incomingPacket = new BcuPacket();
			// Receive the request packet
			incomingPacket->receive(_conn);
			requestedWords = incomingPacket->getDataLengthDw();
			if(curId != incomingPacket->getMsgId()) {
                lostId++;
            }
            curId = incomingPacket->getMsgId();
            curId = (curId+1) % 256;
//          _logger->log(Logger::LOG_LEV_DEBUG, "Received MsgId %d", incomingPacket->getMsgId());	
//			_logger->log(Logger::LOG_LEV_DEBUG, "Request received for %d bytes\n", requestedWords*sizeof(DWORD));
			
		
			// Ping-pong the incoming packet with the requested body
			incomingPacket->setOpCode(Bcu::OpCodes::MGP_OP_CMD_SUCCESS);
			body = new BYTE[requestedWords*sizeof(DWORD)];
			incomingPacket->setData(body, requestedWords);
			incomingPacket->send(_conn);
		} 
		catch(UdpFatalException& e) {
			_logger->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
		}
		catch(UdpTimeoutException& e) {
			// Do nothing, simply retry	
			_logger->log(Logger::LOG_LEV_WARNING, "%s", e.what().c_str());
		}
		
		delete body;
		delete incomingPacket;
        //printf("%d\n", lostId);
	}
}

int main() {
	
	Logger* _logger = Logger::get(Logger::LOG_LEV_INFO);
	
	//Utils::setSchedulingRR(45);
	
	// Get config from file
	int simNum;
	int logLevel;
	int bcuCommPort, localPort;
	string bcuCommIpAddress;
	try {
		Config_File conf("MirrorCtrl/Test/BcuSimulator.config");
		simNum = conf["SIM_NUM"];
		logLevel = conf["LOG_LEV"];
		localPort = conf["LOCAL_PORT"];
		bcuCommPort = conf["BCUCOMM_PORT"];
		bcuCommIpAddress = (string)conf["BCUCOMM_IP"];
	}
	catch(Config_File_Exception& e) {
		_logger->log(Logger::LOG_LEV_FATAL, "Error in BcuSimulator config file: %s", e.what().c_str());
		return -1;
	}
	
	// Set verbosity
	_logger->setLevel(logLevel);
	
	_logger->log(Logger::LOG_LEV_INFO, "--- MULTI-THREADED BCU SIMULATOR ---");
	_logger->log(Logger::LOG_LEV_INFO, "Threads number: %d",simNum);
	_logger->log(Logger::LOG_LEV_INFO, "Local port: %d",localPort);
	_logger->log(Logger::LOG_LEV_INFO, "BcuComm ip: %s", bcuCommIpAddress.c_str());
	_logger->log(Logger::LOG_LEV_INFO, "BcuComm (start) port: %d", bcuCommPort);
	_logger->log(Logger::LOG_LEV_INFO, "Bcu Simulator running...");
	
	BcuSimulator* simulators[simNum];
	pthread_t simThreads[simNum];
	for(int i=0; i<simNum; i++) {
		simulators[i] = new BcuSimulator(localPort+i, bcuCommIpAddress, bcuCommPort+i);
	}
	
	for(int i=0; i<simNum; i++) {
		simThreads[i] = simulators[i]->start();
	}
	
	for(int i=0; i<simNum; i++) {
		pthread_join(simThreads[i], NULL);
	}
	
	for(int i=0; i<simNum; i++) {
		delete simulators[i];
	}

	return 0;
}
