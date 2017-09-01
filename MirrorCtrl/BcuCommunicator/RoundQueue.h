#ifndef ROUNDQUEUE_H_INCLUDED
#define ROUNDQUEUE_H_INCLUDED

#include <set>
using namespace std;

// lib
#include "UdpConnection.h"
#include "BcuLib/BcuCommon.h"

#include "Commons/BcuPacket.h"

using namespace Arcetri;

/*
 * Round buffer for sending/retransmit/receiving BcuPackets.
 * 
 * For efficiency reasons the queue host references to BcuPackets, not BcuPacket 
 * objects.
 */
class RoundQueue {
	
	public:
	
		/*
		 * Create a round-queue with a maximum capacity of sent - and not received -
		 * BcuPackets and a default UdpConnection to a BCU. 
		 * The maxBcuQueueSize parameter value must be the MAX_BCU_QUEUE_SIZE (from config file) to
		 * guarantee the Bcu's working correctly.
		 */
		RoundQueue(int bcuId, int maxBcuQueueSize, 	// Maximum capacity of sent - and not received -
		 										   	// BcuPackets
							  UdpConnection* conn, 	// Udp connection to be used
							  int allowedTimeouts);	// Maximum number of consecutives timouts allowed
							  						// before a reaction is started (defined in the method
							  						// handleToManyTimeouts())
		
		/*
		 * Destroy the round-queue.
		 */		   
		virtual ~RoundQueue();
		
		/*
		 * Check the round-queue status.
		 * IMPORTANT: call the correct one before using insert(), send(), receive() !!!
		 */
		bool canInsert()  { return (_inPackets + _expectedReplyPackets < _maxInPackets); }
		bool canSend() 	  { return ((_inPackets != 0) && (_expectedReplyPackets < _maxExpectedReplyPackets)); }
		bool canReceive() { return (_expectedReplyPackets > 0); }
		
		/*
		 * Insert a BcuPacket (reference) in the round-queue.
		 * IMPORTANT: it doesn't check for the "full" condition - check it before !!!
		 */
		void insert(BcuPacket* packet);
		
		/*
		 * Send a BcuPacket to BCU.
		 * IMPORTANT: it doesn't check for the "canSend" condition - check it before !!!
		 */
		void send() throw (UdpFatalException);
		
		/*
		 * Receive the next expected UdpPacket, automatically managing for an unexpected 
		 * or timed-out packet. 
		 * Returns 0 only if the packet has been correctly received; return some codes
		 * for errors or warnings (see above)
		 * IMPORTANT: it doesn't check for canReceive condition - check it before !!!
		 */
		static const int RECEIVING_OK 	  	  	 = 0;
		static const int RECEIVING_UNEXPECTED_OK = 1;
		static const int TIMEOUT_RECOVERED 	  	 = 2;
		static const int DUPLICATE_DISCARDED  	 = 3;
		int receive() throw (UdpFatalException);
		
	private:
	
		int _bcuId;
		
		// The current and tolerated number of successive timouts
		int _timeouts;
		int ALLOWED_TIMEOUTS;
		
		// A reference to the UdpConnection used to send/receive
		UdpConnection* _conn;
		
		// An array of references to BcuPacket
		BcuPacket** _roundQueue;
		
		// Max and current value of packets inserted but not sent
		int _maxInPackets;
		int _inPackets;
		
		// Max and current number of packets sent but not replied
		int _maxExpectedReplyPackets;
		int _expectedReplyPackets;
		
		// The max msgId allowed for a BcuPacket and an id counter
		static const int MAX_ID = Bcu::Constants::MAX_ID;
		int _curId;
		void succId(int &id) { id = ++id % (MAX_ID+1); }
		
		// Indexes over the array
		int _nextIn;
		int _nextToSend;
		int _nextToReceive;
		void succIndex(int &i) { i = (++i % _maxInPackets); }


		/*
		 * Reinsert the current "nextToReceive" packet for a new sending.
		 * This is used when a packet timeout or when an unexpected packet 
		 * is received. (see recoverTimeout() and recoverUnexpected(...)
		 * methods).
		 * 
		 */
		void reinsert() throw (UdpFatalException);
		
		/*
		 * If number of timeouts is not allowed, reacts appropriately.
		 */
		void handleToManyTimouts() throw (UdpFatalException);
		
		/*
		 * Notify the requester for the packet received and clear the packet
		 */
		void notifyAndClear();
		
		/*
		 * Recover from a timeout or unexpected packet
		 */
		int recoverTimeout();
		int recoverUnexpected(uint8 unexpectedMsgId, BYTE* rawPacket);
		
		/*
		 * Reference to its own logger named "ROUNDQUEUE_BCU-Id"
		 */
		Logger* _logger;
		
};

#endif
