#ifndef BCU_COMMAND_HANDLER_TEST_H_INCLUDED
#define BCU_COMMAND_HANDLER_TEST_H_INCLUDED

// Lib
#include "base/rtdblib.h"

// Commons
#include "Commons/AbstractBcuRequestManager.h"

           
/*
 * 
 */
class BcuCommandHandlerTest: public AbstractBcuRequestManager {
    
    public:
        
        BcuCommandHandlerTest(int id, int bcuNum, int priority, int packetNum);
        virtual ~BcuCommandHandlerTest();
        
        pthread_t start();
    
        static void* execute(void *pthis);      
        
    private:    
    
    	void run();
         
        // Num of packets to send
        int _packetNum;
        
};

#endif
