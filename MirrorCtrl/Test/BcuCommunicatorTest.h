#ifndef BCU_COMMUNICATOR_TEST_H_INCLUDED
#define BCU_COMMUNICATOR_TEST_H_INCLUDED

#include "BcuCommunicator.h"

class BcuCommunicatorTest: public  BcuCommunicator  {
	
	public:
	
		BcuCommunicatorTest(int maxBcuQueueSize, int packetNum, int packetSize, int localPort, int bcuId, string bcuIpAddress, int bcuPort, int receiveTimeout_ms);
        virtual ~BcuCommunicatorTest();
        
        int _packetNum;				// Number of packet to send
        int _packetSize;			// Size of packet to receive
        int _BCU_QUEUE_MAX_SIZE;    // Max size of the output queue
        int _bcuQueueSize;			// Runtime estimated size of the BCU input queue: 
        							// the estimation is based on sent-received packet

        
        // Start the Bcu Communicator Test in a thread
        pthread_t start(); 
	
		void testSendSampleBcuPacket(uint8 msgId) throw (UdpException);
        void testReceiveSampleBcuPacket(BYTE* receivedData) throw (UdpException);   
        
        void testSendReceiveSampleStream();	
	
	private:
	
		// Entry points for the thread, used in start()->pthread_create(...)
        static void* execute(void* pthis);;
        
};

#endif
