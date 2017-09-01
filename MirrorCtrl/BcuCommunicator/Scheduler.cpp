#include "Scheduler.h"

#include "errno.h"

Scheduler::Scheduler(int bcuId) {
	// Retrieve its own logger
	_logger = Logger::get("SCHEDULER_BCU-"+Utils::itoa(bcuId));
	
	_bcuId = bcuId;
	_inputQueuesIterator = _inputQueues.begin();
	
	// Initialize variable to control input-queues empty condition
	_inputQueuesEmpty = true;
	pthread_mutex_init(&_inputQueuesNotEmpty_mutex, NULL);
	pthread_cond_init(&_inputQueuesNotEmpty, NULL);
}

Scheduler::~Scheduler() {
	// Has nothing to destroy...
}

void Scheduler::addInputQueue(BcuPacketQueue* inputQueue) {
	// Set the variables for "input-queues empty condition"
	inputQueue->initNotEmptyCondSupport(&_inputQueuesEmpty, &_inputQueuesNotEmpty, &_inputQueuesNotEmpty_mutex);
    // Insert the queue to its pool preserving order based on priority
	list<BcuPacketQueue*>::iterator iter = _inputQueues.begin();
	while( (iter != _inputQueues.end()) && (inputQueue->getPriority() < (*iter)->getPriority()) ) {
		iter++;
	}
    _inputQueues.insert(iter, inputQueue);
}

void Scheduler::reset() {
	_inputQueuesIterator = _inputQueues.begin();
}

void Scheduler::waitBcuPackets(int timeout_ms) {
	// Init the timeout: because must be an absolute timout, 
	// I need to get the current time and add the desired timout !!!
	struct timeval nowTime;
	struct timespec timeout;
	
	int ret = 0;
	
	pthread_mutex_lock(&_inputQueuesNotEmpty_mutex);
	gettimeofday(&nowTime, NULL);
	timeout.tv_nsec = (nowTime.tv_usec*1000);
	timeout.tv_sec = nowTime.tv_sec;
	// Add the timeout
	timeout.tv_nsec += (timeout_ms%1000)*(int)1E6;
	timeout.tv_sec += timeout_ms/1000;
	timeout.tv_sec += timeout.tv_nsec/(int)1E9;
	timeout.tv_nsec = timeout.tv_nsec%(int)1E9;
	while(ret == 0 &&_inputQueuesEmpty) {
		ret = pthread_cond_timedwait(&_inputQueuesNotEmpty, &_inputQueuesNotEmpty_mutex, &timeout);
		// ATTENTION: can exit from pthread_cond_timedwait also if condition is not changed (see man pages)
        // The while checks the condition again
    }
	// Note that the pthread_cond_wait relese thw mutex while waiting, but
	// locks it when returns !!!
	pthread_mutex_unlock(&_inputQueuesNotEmpty_mutex);
}

BcuPacket* Scheduler::nextBcuPacket() {
	// Current sheduling policy: round-robin
	
	BcuPacket* nextPacket = NULL;
	// Check is the iterator is at the end of the list
	if(_inputQueuesIterator == _inputQueues.end()) {
		_inputQueuesIterator = _inputQueues.begin();
	}
	
	// Save the current position: if all queues are inspected and all are empty,
	// will stop the loop in the current position
	BcuPacketQueue* firstExaminedQueue = (*_inputQueuesIterator);
	
	BcuPacketQueue* currentQueue = NULL;
	do {
		// Get the current queue end extract the next packet
		currentQueue = (*_inputQueuesIterator);
		//_logger->log(Logger::LOG_LEV_DEBUG, "Polling input-queue with priority %d", currentQueue->getPriority());
		if(!(currentQueue->empty())) {
			nextPacket = currentQueue->pop();
		}
		// Advance the iterator, checking if is at the end of the list
		_inputQueuesIterator++;
		if(_inputQueuesIterator == _inputQueues.end()) {
			_inputQueuesIterator = _inputQueues.begin();
		}
	} while (nextPacket == NULL && (*_inputQueuesIterator) != firstExaminedQueue);
	
	// If all visited queues are empty, set the "input-queues empty condition" to true
	if(nextPacket == NULL) {
		pthread_mutex_lock(&_inputQueuesNotEmpty_mutex);
		_inputQueuesEmpty = true;
		pthread_mutex_unlock(&_inputQueuesNotEmpty_mutex);
	}
	return nextPacket;
}

list<BcuPacketQueue*>* Scheduler::getInputQueuesSortedByDescendingPriority() {
	return  &_inputQueues;
}

list<BcuPacketQueue*>::iterator Scheduler::getInputQueuesSortedByDescendingPriorityIterator() {
	return _inputQueuesIterator;
}
