#include <list>
using namespace std;

#include "BcuCommunicator.h"
#include "Scheduler.h"
#include "Commons/TimeToDie.h"


BcuCommunicator::BcuCommunicator(int localPort, int bcuId, string bcuIpAddress, int bcuPort, int bcuQueueSize, int receiveTimeout_ms, int allowedTimeouts, int logLev) {

    // Get its own logger, wich should have been created by the MirrorCtrl with an
    // appropriate logging level. Otherwise, initialize it with LOG_LEV_ERROR logging level.
	_logger = Logger::get("BCUCOMMUNICATOR_"+Utils::itoa(bcuId), logLev);
	_logger->printStatus();

	// Creates some loggers with the same logging level
	Logger::get("SCHEDULER_BCU-"+Utils::itoa(bcuId), _logger->getLevel());
	Logger::get("ROUNDQUEUE_BCU-"+Utils::itoa(bcuId), _logger->getLevel());

    _bcuId = bcuId;
    try {
    	_bcuConn = new UdpConnection(localPort, bcuIpAddress, bcuPort, receiveTimeout_ms);
    }
    catch(UdpException& e) {
    	_logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
    	throw;
    }
    _scheduler = new Scheduler(bcuId);
    _roundQueue = new RoundQueue(bcuId, bcuQueueSize, _bcuConn, allowedTimeouts);

    _logger->log(Logger::LOG_LEV_INFO, "BcuCommunicator %s succesfully constructed", info().c_str());
}


BcuCommunicator::~BcuCommunicator() {
	_logger->log(Logger::LOG_LEV_INFO, "Destroying BcuCommunicator %s...", info().c_str());

	// Clear the connection
	delete _bcuConn;

	// Clear the scheduler
	delete _scheduler;

	// Clear the round-queue
	delete _roundQueue;
}

void BcuCommunicator::addInputQueue(BcuPacketQueue* inputQueue) {
    _logger->log(Logger::LOG_LEV_INFO, "   > Adding input-queue with %s to BCU Communicator %s", Priority::getPriorityDescription(inputQueue->getPriority()).c_str(), info().c_str());
    _scheduler->addInputQueue(inputQueue);
}


pthread_t BcuCommunicator::start() {

	pthread_t bcuCom;
	pthread_create(&bcuCom, NULL, &execute, (void*)this);
    return bcuCom;
}


void* BcuCommunicator::execute(void* pthis) {

   bool work=true;

	// Set a Real-Time priority for this thread
	Utils::setSchedulingRR(99);

    BcuCommunicator* bcuCom = (BcuCommunicator*)pthis;
   	bcuCom->getLogger()->log(Logger::LOG_LEV_INFO, "BcuCommunicator %s succesfully started !!!", bcuCom->info().c_str());

	// Do all the dirty job :-)
   while(work) {
	   try {
		   bcuCom->communicate();
//		   while(!TimeToDie::check()) {
//			   msleep(1000);
//		   }
	   }
	   catch(UdpFatalException& e) {
		   bcuCom->getLogger()->log(Logger::LOG_LEV_ERROR, "BcuCommunicator %s failed: %s", bcuCom->info().c_str(),e.what().c_str());
	   }
	   catch(MirrorControllerShutdownException& e) {
		   bcuCom->getLogger()->log(Logger::LOG_LEV_WARNING, "BcuCommunicator %s shutting down...", bcuCom->info().c_str());
         work = false;
      }
   }

	return NULL;
}


void BcuCommunicator::communicate() throw (UdpFatalException, MirrorControllerShutdownException) {

	BcuPacket* currentPacket;

	// Initialize the scheduler
	_scheduler->reset();

	// This infinite loop stop in case of
	//   - UdpFatalException: Bcu or network dead
	//   - MirrorControllerShutdownException: MirrorCtrl have been requested to die
	while (!TimeToDie::check()) {

		//nusleep(10000);

		currentPacket = NULL;

		// Try to fill the round-queue (output-queue)
		while(_roundQueue->canInsert() && (currentPacket = _scheduler->nextBcuPacket()) != NULL) {
			_roundQueue->insert(currentPacket);
		}

		// If the following conditions happen togheter:
		//	- All input-queues are empty
		//  - Round-queue can't send neither receive
		// wait on a condition notified by the scheduler, instead of looping !!!
		if( _roundQueue->canInsert() && currentPacket == NULL &&
		    !(_roundQueue->canSend()) && !(_roundQueue->canReceive()) ) {
		    //_logger->log(Logger::LOG_LEV_DEBUG, "BcuCommunicator %d - Input-queues empty: waiting for BcuPackets...", _bcuId);
			_scheduler->waitBcuPackets(500); //Timeout in ms
		}
		// Else send and/or receive
		else {
			// Fills the BCU input queue
			while(_roundQueue->canSend()) {
				_roundQueue->send(); // Here it sends also automatically re-inserted packets !!!
				//_logger->log(Logger::LOG_LEV_DEBUG, "Sent");
			}

			// Receives a packet in the RoundQueue, and eventually recovers timed-out,
			// expected-later or unexpected packets
			if(_roundQueue->canReceive()) {
				_roundQueue->receive();
				//_logger->log(Logger::LOG_LEV_DEBUG, "Received");
			}
		}
	}
}




