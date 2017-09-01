#include "RoundQueue.h"

RoundQueue::RoundQueue(int bcuId, int maxBcuQueueSize, UdpConnection* conn, int allowedTimeouts) {
	// Retrieve its own logger
	_logger = Logger::get("ROUNDQUEUE_BCU-"+Utils::itoa(bcuId));
	
	_bcuId = bcuId;
	
	// Initialize the connection parameters
	_conn = conn;
	_timeouts = 0;
	ALLOWED_TIMEOUTS = allowedTimeouts;
	
	// Create the round queue
	_maxInPackets = maxBcuQueueSize*2;
	if(_maxInPackets % (ALLOWED_TIMEOUTS+1) == 0) { 
		_maxInPackets++;	// Avoid a packet can have an "indefinite" loop of reinsert !!!
	}
	_maxExpectedReplyPackets = maxBcuQueueSize;
	_roundQueue = new BcuPacket*[_maxInPackets];
	
	// NULL the queue (only for safety)
	for(int i=0; i<_maxInPackets; i++) {
		_roundQueue[i] = NULL;
	}
	
	_curId = -1;
	_inPackets = 0;
	_expectedReplyPackets = 0;
	
	// Initialize the indexes
	_nextIn = _nextToSend = _nextToReceive = 0;
	
	_logger->log(Logger::LOG_LEV_DEBUG, "RoundQueue [incoming/outgoing capacity: %d/%d - Allowed consecutives timeouts: %d] constructed", _maxInPackets,  _maxExpectedReplyPackets, ALLOWED_TIMEOUTS);
}

RoundQueue::~RoundQueue() {
	// Delete packets remained in the queue
	for(int i=0; i<_maxInPackets; i++) {
		delete _roundQueue[i];
	}
	delete [] _roundQueue;
}

void RoundQueue::insert(BcuPacket* packet) {
	// Increase the msgId and set it in the packet
	succId(_curId);
	packet->setMsgID(_curId);
		
	// Insert a reference in the queue
	_roundQueue[_nextIn] = packet;
	succIndex(_nextIn);
	_inPackets++;
	//_mainLogger->log(Logger::LOG_LEV_DEBUG, "Inserted id %d (position %d)", _roundQueue[_nextIn]->getMsgId(), _nextIn);
}

void RoundQueue::reinsert() throw (UdpFatalException) {
	
	// Increase the number of timed-out packets
	_timeouts++;
	// Check if timout counter exceed the allowed value
	if(_timeouts>ALLOWED_TIMEOUTS) {
		handleToManyTimouts();
	}
	else {
		// Save a reference to the "nextToReceive" packet
		BcuPacket* tempPacket = _roundQueue[_nextToReceive];
		
		// Advance the _nextToReceive index as the packet was been received:
		// this free a space for the re-insertion
		_roundQueue[_nextToReceive] = NULL;
		succIndex(_nextToReceive);
		_expectedReplyPackets--;
		
		// Re-insert the packet in the queue for a re-send
		insert(tempPacket);
		_logger->log(Logger::LOG_LEV_WARNING, "...reinserted !");
	}
}

void RoundQueue::send() throw (UdpFatalException) {
	_roundQueue[_nextToSend]->send(_conn);
	_expectedReplyPackets++;
	//_logger->log(Logger::LOG_LEV_DEBUG, "Sent id %d", _roundQueue[_nextToSend]->getMsgId());
	succIndex(_nextToSend);
	_inPackets--;
	// Note that also packets wich doesn't require a reply are
	// left in the round queue for the receiving: the receive()
	// method will take care about them.
}

int RoundQueue::receive() throw (UdpFatalException) {
	
	// Check the packet is waiting for the reply
	if(_roundQueue[_nextToReceive]->getFlagWantreply()) {
		try {
			// Note that the receive(UdpConnection, checkId) checks for the correct id
			_roundQueue[_nextToReceive]->receive(_conn, true);
			
			// Because the receving is ok, decrease the timeout counter: this policy
			// allow to recover from a number of timeouts less the "AllowedTimeouts"
			// defined in config file.
			if(_timeouts>0) {
				_timeouts--;
			}
		} 
		// The reply doesn't come
		catch (UdpTimeoutException& e) {
			_logger->log(Logger::LOG_LEV_WARNING, "UdpTimeoutException:recoverTimeout");
			return recoverTimeout();
		} 
		// The reply is unexpected
		catch (UnexpectedBcuPacketException& e) { 
			_logger->log(Logger::LOG_LEV_WARNING, "UnexpectedBcuPacketException:recoverUnexpected");
			return recoverUnexpected(e.getUnexpectedMsgId(), e.getUnexpectedRawPacket());
		}
	}
	
	// Now notify the requester and clear the packet
	notifyAndClear();
	
	return RECEIVING_OK;
}

void RoundQueue::handleToManyTimouts() throw (UdpFatalException) {
	// Discard the packet: note that 
	notifyAndClear();
	//!!!//_logger->log(Logger::LOG_LEV_DEBUG, "To many timeouts: packet discarded !");
	// Reset the timout counter
	_timeouts = 0;
}

void RoundQueue::notifyAndClear() {
	_expectedReplyPackets--;
	// Notify the Request
	//!!!//_logger->log(Logger::LOG_LEV_DEBUG, ">>> Bcu[%d] - Requester notified ! <<<", _bcuId);
	_roundQueue[_nextToReceive]->notifyReply(_bcuId);
	
	// Clear the packet
	delete _roundQueue[_nextToReceive];
	//-->//_logger->log(Logger::LOG_LEV_DEBUG, "Packet cleared !");
	_roundQueue[_nextToReceive] = NULL;
	succIndex(_nextToReceive);
}


int RoundQueue::recoverTimeout() {
	
	_logger->log(Logger::LOG_LEV_WARNING, "TIMEOUT id %d: recovering...", _roundQueue[_nextToReceive]->getMsgId());
	
	// Reinsert the packet for a new send
	reinsert();
	
	//_logger->log(Logger::LOG_LEV_WARNING, "...done !");
	return TIMEOUT_RECOVERED;
}

int RoundQueue::recoverUnexpected(uint8 unexpectedMsgId, BYTE* unexpectedRawPacket) {
	// There are 2 possibilities:
	//  - The packets is expected later (at least one previously expected packet is lost)
	//  - The packet is completely unexpected: is a duplicate or a Bcu "svarione"
	
	_logger->log(Logger::LOG_LEV_WARNING, "UNEXPECTED id %d: recovering...", unexpectedMsgId); 

	// --- (1) Reinsert the current packet ---- //
	// Of course the currently expected hasn't been received: reinsert it !
	//reinsert();

	// NOTE: should be possible to perform only one loop for steps 2) and 3), collecting the indexes
	// to be resent, and resending them only if the unexpected id is expected later: this will complicate
	// the loop and isn't usefull because the loop length is negligible (4-8),and I can do it twice !

	// --- (2) Check if the id is expected later --- // 
	// Note that the _nextToReceive has been increased int the previous reinsert()
	int iter = _nextToReceive;
	// Loop over the expected packets
	bool expectedLater = false;
	_logger->log(Logger::LOG_LEV_WARNING, "\tScanning output-queue for expected later...", unexpectedMsgId); 
	while(iter != _nextToSend && !expectedLater) {
		if(_roundQueue[iter]->getMsgId() == unexpectedMsgId) {
			//_mainLogger->log(Logger::LOG_LEV_DEBUG, "-> Found in outgoing queue in position %d !!!", i);
			expectedLater = true;
		}
		succIndex(iter);
	}
	
	// --- (3) If is expected later, reinsert all the previous (as the timed out) and receive it --- //
	if(expectedLater) {
		bool received = false;
		while((_nextToReceive != _nextToSend) && !received) {
			if(_roundQueue[_nextToReceive]->getMsgId() == unexpectedMsgId) {
				_roundQueue[_nextToReceive]->receive(unexpectedRawPacket);
				received = true;
				notifyAndClear();
			}
			else {
				_logger->log(Logger::LOG_LEV_WARNING, "\t...found a lost packet (will be reinserted)", unexpectedMsgId); 
				reinsert();
			}
		}
		
		_logger->log(Logger::LOG_LEV_WARNING, "...expected later received !");
		return RECEIVING_UNEXPECTED_OK;
	}
	// Is a completely unexpected packet, probably a duplicate (previously timed-out)
	else {
		_logger->log(Logger::LOG_LEV_WARNING, "...duplicate discarded !");
		delete unexpectedRawPacket;
		return DUPLICATE_DISCARDED;
	}
}

