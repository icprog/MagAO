#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <iostream>
#include <iostream>
using namespace std;

// Libs
#include "AOApp.h"

// RequestManagers
#include "RequestManagers/BcuCommandHandler.h"
#include "RequestManagers/DiagnosticManager.h"
#include "BcuCommandHandlerTest.h"

// BcuCommunicator
#include "BcuCommunicator/BcuCommunicator.h"


typedef vector<BcuCommunicator*> BcuCommunicatorPool;
typedef vector<DiagnosticManager*> DiagnosticManagerPool;


/*
 * Controller of the secondary mirror BCU pool.
 *
 * Forward BCU Commands to BCUs and downloads diagnostic frames.
 *
 * General behaviour:
 *
 *  1. Let's consider a set of N BCUs defined in a config file (i.e. see MirrorCtrl.config)
 *  2. It creates a set of N BcuCommunicators, one for each valid entry of the config file.
 *  3. It creates 2 BcuCommandHandlers and register their execute(...) methods with thrdlib.
 *  4. It creates K DiagnosticManagers (from K entries in a diagnostic config file.
 *  5. For each BcuCommunicator, add to it all producers (AbstractBcuRequestManager) 
 *     created.
 *  6. Starts all Bcu Communicators and Diagnostic Manager in separated threads.
 *     
 */
class MirrorCtrl: public AOApp {
    
    public:
        MirrorCtrl(const std::string& conffile, int verbosity = Logger::LOG_LEV_ERROR) throw (AOException); 
        
        int getBcuNum();
        void setBcuNum(int bcuNum);
    
    protected:
    
        // Install handlers for incoming Bcu requests from MsgD (BcuCommandHandler)
        void InstallHandlers();
    
        // Execute the threads performing diagnostic (DiagnosticManager)
        void Run();
    
    private:
    
        // Number of BCUs (that is, Bcu Communicators) running in the system
        int _totalBcuNum;
        
        // Number of Diagnostic Manager running in the system
        int _diagnNum;
    
        // Handlers for high and low priority Bcu Commands from MsgD
        BcuCommandHandler* _lowPriorityCmdHandler;
        BcuCommandHandler* _highPriorityCmdHandler;
        
        // Thread performing diagnostic download
        DiagnosticManagerPool _diagnManagersPool;
        
        // Threads performing connections to BCUs
        BcuCommunicatorPool _bcuCommunicators;
        
        BcuCommandHandlerTest* _testCmdHandler[];
        
        // Read the Diagnostics config files and return a list with the text lines.
        // Each line corresponds to a set of config parameters for a DiagnosticManager
        list<string> readDiagnosticConfig() throw (Config_File_Exception);
        
};

