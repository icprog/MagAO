#ifndef BCU_REQUEST_FRAME_QUEUE_H_INCLUDED
#define BCU_REQUEST_FRAME_QUEUE_H_INCLUDED

#include <queue>
#include <pthread.h>
using namespace std;

#include "BcuPacket.h"


/*
 * A thread-safe queue to host BcuRequestFrames.
 * 
 * Note that all implemented operations are guaranteed to be thread-safe
 * using a mutex.
 */
class BcuPacketQueue {

    public:
    
        BcuPacketQueue(int priority);
        ~BcuPacketQueue();
        
        /*
         * Initialize the support to take care about "input-queues empty condition":
         * in fact each BcuPacketQueue belongs to a pool (see BcuCommunicator/Scheduler)
         * of BcuPacketQueue wich are treated as a global BcuPacket source.
         */
        void initNotEmptyCondSupport(bool*	pred, 
        						  	 pthread_cond_t* cond, 
        						     pthread_mutex_t* mutex) { _inputQueuesEmpty_global = pred; 
        													   _inputQueuesNotEmpty_global_cond =  cond; 
        													   _inputQueuesNotEmpty_global_mutex = mutex; }
        
        /*
         * Return the queue priority.
         */
        int getPriority() { return _priority; }
        
        /*
         * Main actions on queue
         */
        bool empty();
        void push(BcuPacket*);
        BcuPacket* pop();
        int size();
        
        /*
         * Compares basing on priority
         * Used by Scheduler mantain the a list sorted
         */
        bool operator<(BcuPacketQueue& q) { return _priority < q.getPriority(); }
    
    private:
    	int _priority;
        queue<BcuPacket*> _tsQueue;
        pthread_mutex_t _tsQueue_mutex; 
        
        // Support variables to know when at least one queue is not empty
		// (input-queues empty condition, see BcuCommunicator/Scheduler)
        pthread_mutex_t* _inputQueuesNotEmpty_global_mutex;
        pthread_cond_t*  _inputQueuesNotEmpty_global_cond;
        bool*			 _inputQueuesEmpty_global;
};

#endif
