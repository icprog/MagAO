#ifndef BCU_REQUEST_INFO_H_INCLUDED
#define BCU_REQUEST_INFO_H_INCLUDED

#include <pthread.h>
#include <string>
#include <map>
using namespace std;

// Lib
#include "BcuLib/BcuCommon.h"

#include "BcuRequest.h"
#include "MirrorCtrlExceptions.h"

/*
 * Stores the BcuRequest and some info about its status:
 * 	- Expected number of reply packets from each Bcu
 *  - Flag bcuRequestFailed
 * 
 * Also store the memory address from/to which read/write data for
 * the request: the address is obtained form the shmId of the request
 * or directly as an otpional constructor parameter.
 *  
 * Allows to know when the reply is ready using the method 
 * waitReplyReady(int timeout_ms)
 */
class BcuRequestInfo {

    public: 	//--- METHODS
    
    	/*
    	 * Default constructor.
    	 * 
    	 * ATTENTION: You must call set(BcuRequest* request, BYTE* dataAddress) 
    	 * before using a BcuRequestInfo constructed with this !!!
    	 */
    	BcuRequestInfo();
 
    
        /*
    	 * Constructor (for test purpose only)
    	 */
        BcuRequestInfo(int bcuFrom,          // First target BCU
                       int bcuTo,            // Last target BCU
                       int expectedReplies); // Number of expected replies from each BCUs
        
        ~BcuRequestInfo();
        
        /*
         * Set the BcuRequest info
         * 
         * This method MUST be called before using the request info !!!
         * If the request doesn't specify a shmId (_shmId = -1) you must initialize
    	 * the dataAddress parameter.
         */
        void set(BcuRequest* request, BYTE* dataAddress = NULL) throw(InvalidSharedMemoryException);
        
        /*
         * Clear the BcuRequest info
         */
        void clear();
                       
        /*
         * Returns the BcuRequest
         */
        BcuRequest* getBcuRequest() { return _bcuRequest; }
        
        /*
         * Return the address of data memory
         */
        BYTE* getDataAddress() { return _dataAddress; }
        
        /*
         * Get/set info about request failure
         */
        bool isFailed()  { return _bcuRequestFailed; }
        void setFailed() { _bcuRequestFailed = true; }
                       
        // Decrease the number of UdpPackets missing to complete the reply
        // from a certain BCUs and returns the new value
        int decreaseReplyCounter(int bcu);
        
        // Lock the requesting thread until the _replyReady is zero
        // or the estimated timout expires.
        // [note] Doesn't use the internal estimated timeout to allow
        // the implementation of the AsbtractBcurequestManager (i.e.
        // the BcuCommandHandler) to change this value if desired.
        int waitReplyReady(int timeout_ms);

    private:	//--- METHODS
    
    	/*
    	 * Compute the number of packet num wich will be created from
    	 * the BcuRequest ***for each Bcu***
    	 * 
    	 * The parameter is received by value because this computation
    	 * modify the request
    	 */
    	uint computeBcuPacketNum(BcuRequest bcuReq);
    
    private:	//--- FIELDS
    
        //Info about the BCU Request currently in progress
		BcuRequest*	_bcuRequest;		// A reference to the request :-)
		bool		_bcuRequestFailed;
		//int			_shmId;				// Copied from _bcuRequest to correctly clear
		BYTE* 		_dataAddress;
		
        map<int, int> _replyCounters;   // Number of expected reply packets from each BCUs
                                        // The key is the BCU id
        int     _replyReady;            // Numbers of BCUs currently replying to the request
                                        // When zero: reply completed !      
                                        
               
        
        // Locking management
        pthread_mutex_t _mutex_replyReady;
        pthread_cond_t  _condition_replyReady;  
        
        // Support methods for computeBcuPacketNum(BcuRequest bcuReq)
        void handleMultiDsp(BcuRequest* request, int reqPacketSize, uint* madePackets);
               
};

#endif
