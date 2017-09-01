#ifndef BCU_COMMAND_HANDLER_H_INCLUDED
#define BCU_COMMAND_HANDLER_H_INCLUDED

// Lib
extern "C"{
	#include "base/thrdlib.h"
}

#include "Commons/AbstractBcuRequestManager.h"
#include "Commons/MirrorCtrlExceptions.h"

           
/*
 * Define the interface for an handler of "Bcu Commands", that is a "Bcu Requestes"
 * received from MsgD.
 *
 * General behaviour:
 *  1. Is started by thrdlib when a BcuRequest arrives from the MsgD
 *  2. Generates a set of BcuPackets from the BcuRequest
 *  3. Fills its own output-queues with the generated BcuPackets 
 *  4. Waits for the condition "BcuReply received"
 *  5. Send the Bcu Command Reply to MsgD
 * Note that steps 2-5 are performed by base class (AbstractBcuRequestManager)
 * 
 */
class BcuCommandHandler: public AbstractBcuRequestManager {
    
    public:
        
        explicit BcuCommandHandler(int id, int bcuStart, int bcuNum, int priority, int logLevel);
        virtual ~BcuCommandHandler();
        
        // Handle a Bcu request from msgD
        int handleBcuRequest(MsgBuf* msgBuf);
        
    private:    
    
    /*
     * Handles the reply of the request: send the reply message to MsgD
     * Receive as parameter the MsgBuf request to wich is replying.
     */
    void handleBcuReply(MsgBuf* msgBuf) throw (BcuCommandHandlerException, MirrorControllerShutdownException);
        
};

#endif
