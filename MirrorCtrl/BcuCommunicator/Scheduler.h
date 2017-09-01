#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include <list>
using namespace std;

#include "Commons/BcuPacketQueue.h"


/*
 * Defines a pool of BcuPacketQueue and a scheduling policy to
 * pop packets.
 *
 * The pool is implemented as a list of references because the BcuPacketQueues
 * actually belongs to a set of AbstractBcuRequestManagers, not to this object.
 * 
 * Allowed operations are:
 *  - Add e new BcuPacketQueue reference, also at runtime. [todo] To check and test, if feature needed
 *  - Get the next BcuPacket (as defined by the scheduling policy)  
 *  - Wait for a BcuPacket present if all queues are empty
 * 
 * The Scheduler is used by a BcuCommunicator with a certain Id to obtain
 * BcuPackets.
 */
class Scheduler {
	
	public:
		Scheduler(int bcuId);
		~Scheduler();
		
		/*
		 * Add an input-queue
		 */ 
		void addInputQueue(BcuPacketQueue* inputQueue);
		
		/*
		 * Reset the scheduler.
		 * Call this before using the scheduler or after inserting a new queue 
		 * at runtime !!!
		 */
		void reset();
		
		/*
		 * Wait for BcuPackets in input-queues.
		 */
		void waitBcuPackets(int timeout_ms);
		
		/*
		 * Returns the next BcuPacket according to a scheduling policy.
		 * The scheduling policy is at the moment defined in the implementation
		 * of this method.
		 */
		BcuPacket* nextBcuPacket();
		
		/*
		 * Get the list of the input queues sorted by descending priority or
		 * ***Usefull only for debugging***.
		 */
		list<BcuPacketQueue*>* getInputQueuesSortedByDescendingPriority();
		
		/*
		 * Get the iterator to the list of the input queues sorted by descending priority or
		 * ***Usefull only for debugging. Reset the scheduler befor calling this !!!***
		 */
		list<BcuPacketQueue*>::iterator getInputQueuesSortedByDescendingPriorityIterator();
	
	private:
	
		int _bcuId;
	
		// This list is maintened sorted (by descending priority) by the add(...) method
		list<BcuPacketQueue*> _inputQueues;
		
		// Support variables to know when at least one queue is not empty
		// (input-queues empty condition)
		pthread_mutex_t _inputQueuesNotEmpty_mutex;
		pthread_cond_t  _inputQueuesNotEmpty;
		bool		    _inputQueuesEmpty;
		
		// Iterator used to traverse the _inputQueues
		list<BcuPacketQueue*>::iterator _inputQueuesIterator;
		
		// Reference to its own logger named "SCHEDULER_BCU-Id"
		Logger* _logger;	
};

#endif
