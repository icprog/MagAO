#ifndef BCU_COMMUNICATOR_H_INCLUDED
#define BCU_COMMUNICATOR_H_INCLUDED

#include <string> 
#include <pthread.h>
using namespace std;

// Lib
#include "UdpConnection.h"

#include "BcuCommunicator/Scheduler.h"
#include "BcuCommunicator/RoundQueue.h"
#include "Commons/MirrorCtrlExceptions.h"
#include "Commons/BcuPacket.h"
#include "Commons/BcuPacketQueue.h"
#include "Utils.h"

using namespace Arcetri;


/*
 * Threaded module implementing communicaton with a single BCU.
 *
 * A BCU is identified by:
 *  - An *unique* id, ranging from 0 to N.
 *  - An *unique* remote ip address
 *  - A remote port
 * The unique IDs for all BCUs of the systems are defined in lib/BcuCommon.h
 * 
 * A BcuCommunicator is identified by:
 *  - The same *unique* id of the BCU which is connected to
 *  - An *unique* local port
 * 
 * - ATTENTION -
 * When the BcuCommunicator is connected to a real BCU, the remote and local ports must
 * have the same value, because the BCU is able only to reply using the same port from
 * where it receives the request.
 * 
 * 
 * Is responsibility of the user of this class to never instantiate conflicting
 * (that is, using the same *unique* identifiers) BcuComunicators.
 * 
 * The Udp connection info are stored in an UdpConnection object.
 * The pool of input queues, from where the BcuCommunicator obtains BcuPackets,
 * are stored in a Scheduler object.
 *
 * UdpPacket sending/receiving
 * The basic principle is that the same BcuPacket (a reference to it) is used both
 * for the request to the BCU and for the reply from the BCU. This allow a simple 
 * send/retry mechanism and a fast storage of reply data (togheter with the notify 
 * to the producer of the BcuPacket).
 * 
 * A "request" BcuPacket is obtained from the Scheduler, with a call to scheduler.nextBcuPacket(),
 * and is sent to BCU, managing retry mechanism for lost packets.
 * When the corresponding "reply" BcuPacket is got from the BCU, the reply data (in case 
 * of a read) are stored in a memory location (prioviously set inside the BcuPacket by the 
 * AbstractBcuRequestManager), and the AbstractBcuRequestManager is notified with a call 
 * bcuPacket.notifyReply().
 * 
 */
class BcuCommunicator {
    
    public:
    
    	/*
    	 * Create a BcuCommunicator.
    	 * IMPORTANT: never create two BcuCommunicators using the same localPort, bcuId 
    	 * or bcuIpAddress, because they won't work.
    	 */
        BcuCommunicator(int localPort, 				// Local port for the connection to the  BCU
        				int bcuId, 					// The ID of the BCU (used to identify the BcuCommunicator)
        				string bcuIpAddress, 		// The remote ip address of the BCU
        				int bcuPort,				// The remote port of the Bcu
        				int bcuQueueSize,			// Max size of the BCU input queue
        				int receiveTimeout_ms,		// Timeout for packet receiving
        				int allowedTimeouts,		// Allowed successive timeouts in receiving
        				int logLev);
        				
        /*
         * Destroy and clean the BcuCommunicator.
         */
        virtual ~BcuCommunicator();
        
        /*
         * Some useful getters
         */
        int 			getBcuId() 	 { return _bcuId; }
        UdpConnection*	getBcuConn() { return _bcuConn; }
        Logger*			getLogger()  { return _logger; }
        
        /*
         * Some shortcuts to UdpConnection properties
         */
        int 	getLocalPort()		{ return _bcuConn->getLocalPort(); }
        string  getBcuIpAddress() 	{ return _bcuConn->getRemoteIpAsString(); }
        int 	getBcuPort() 		{ return _bcuConn->getRemotePort(); }
        
      
        /*
         * Add an input queue containing BcuPackets.
         * The request is forwarded to the Scheduler.
         */
        void addInputQueue(BcuPacketQueue* inputQueue);
        
        /*
         * Start the Bcu Communicator in a thread, returning a reference to it
         */
        pthread_t start();    
        
         /*
          * Return info about the object
          */
        const string info() { return ("[ID="+Utils::itoa(_bcuId)+" IP="+getBcuIpAddress()+":"+Utils::itoa(getBcuPort())+"]"); }  
        
    // These "protected" are only for test subclassing (could be private)
    protected: 	//---FIELDS
    
    	int 		   	_bcuId;
    	UdpConnection* 	_bcuConn;	// Usefull to test this class without using RoundQueue
        
        // A scheduler managing the input-queues containing BcuPackets
        Scheduler* _scheduler;
        
        // The RoundQueue to send/retransmit/receive the packets. It wraps the
        // UdpConnection
        RoundQueue* _roundQueue;
        
        // Its own logger: enable differents BCuCommunicator threads to 
    	// have different logging levels
        Logger* _logger;	
        
	protected:	//---METHODS  
    
        // Entry point for the thread, used in start()->pthread_create(...)
        static void* execute(void* pthis);
        
        /*
         * Implements the main infinite loop of the BcuCommunicator,
         * sending and receiving Bcu packets.
         */
        void communicate()  throw (UdpFatalException, MirrorControllerShutdownException);
        
};

#endif
