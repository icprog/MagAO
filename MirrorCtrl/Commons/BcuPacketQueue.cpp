#include "BcuPacketQueue.h"
#include "Utils.h"

BcuPacketQueue::BcuPacketQueue(int priority) {
   if((Priority::MIN_ALLOWED_PRIORITY <= priority) && (priority <= Priority::MAX_ALLOWED_PRIORITY)) {
		_priority = priority;
	}
	else {
		_priority = Priority::MIN_ALLOWED_PRIORITY;
		Logger::get()->log(Logger::LOG_LEV_WARNING, "Invalid BcuPacketQueue priority (%d): using MIN_ALLOWED_PRIORITY instead", priority);
	}
	pthread_mutex_init(&_tsQueue_mutex, NULL);
}

BcuPacketQueue::~BcuPacketQueue() {
	int mySize = size();
	Logger::get()->log(Logger::LOG_LEV_TRACE, "Clearing BcuPacketQueue with priority %s of size %d...", Priority::getPriorityDescription(_priority).c_str(), mySize);
    for(int i=0; i<mySize; i++) {
    	delete pop();
    	Logger::get()->log(Logger::LOG_LEV_TRACE, ">> Packet deleted <<");
    }
}

bool BcuPacketQueue::empty() {
	pthread_mutex_lock(&_tsQueue_mutex);
	bool empty = _tsQueue.empty();
	pthread_mutex_unlock(&_tsQueue_mutex);
	return empty;
}

void BcuPacketQueue::push(BcuPacket* fp) {
    pthread_mutex_lock(&_tsQueue_mutex);
    // Manage the empty condition of the set of input-queues to wich
    // this queue belong
    if(_tsQueue.empty() && *_inputQueuesEmpty_global) {
    	pthread_mutex_lock(_inputQueuesNotEmpty_global_mutex);
    	_tsQueue.push(fp);
    	*_inputQueuesEmpty_global = false;
    	pthread_cond_signal(_inputQueuesNotEmpty_global_cond);
    	pthread_mutex_unlock(_inputQueuesNotEmpty_global_mutex);
    }
    else {
    	_tsQueue.push(fp);
    }
    pthread_mutex_unlock(&_tsQueue_mutex);
}

BcuPacket* BcuPacketQueue::pop() {
    pthread_mutex_lock(&_tsQueue_mutex);
    BcuPacket* fp = _tsQueue.front();
    _tsQueue.pop();
    pthread_mutex_unlock(&_tsQueue_mutex);
    return fp;
}

int BcuPacketQueue::size() {
	pthread_mutex_lock(&_tsQueue_mutex);
	int size = _tsQueue.size();
	pthread_mutex_unlock(&_tsQueue_mutex);
	return size;
}
