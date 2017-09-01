#ifndef BCU_COMMUNICATOR_TEST_H_INCLUDED
#define BCU_COMMUNICATOR_TEST_H_INCLUDED

#include "BcuCommunicator/BcuCommunicator.h"

/*
 * This test id the version using the RoundQueue
 */
class BcuCommunicatorTest: public  BcuCommunicator  {
	
	public:
	
		BcuCommunicatorTest(int maxBcuQueueSize, int packetNum, int packetSize, int localPort, int bcuId, string bcuIpAddress, int bcuPort, int receiveTimeout_ms, int acceptedTimeouts, bool mute);
        virtual ~BcuCommunicatorTest();
        
        int _packetNum;				// Number of packet to send
        int _packetSize;			// Size of packet to receive
        
        // Start the Bcu Communicator Test in a thread
        pthread_t start() throw (UdpFatalException); 
        
        void testSendReceiveSampleStream() throw (UdpFatalException);	
	
	private:
	
		// Entry points for the thread, used in start()->pthread_create(...)
        static void* execute(void* pthis) throw (UdpFatalException);
        
};

#endif
