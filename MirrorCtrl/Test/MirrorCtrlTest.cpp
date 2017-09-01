#include <stdlib.h>
using namespace std;

// Lib
#include "BcuLib/BcuCommon.h"

#include "MirrorCtrlTest.h"
#include "Utils.h"
#include "Commons/TimeToDie.h"
#include "BcuCommunicator/BcuCommunicator.h"


// -------------------------- PUBLIC -------------------------//

MirrorCtrl::MirrorCtrl(const std::string& conffile, int verbosity) throw (AOException)
           :AOApp(conffile) {
    
    TimeToDie::init(&_timeToDie);
    
    try {
    	
    	// Get the number of BCUs from the config dictionary
        _totalBcuNum = ConfigDictionary()["BcuNum"];
    	_logger->log(Logger::LOG_LEV_INFO, "Constructing MirrorCtrl for %d BCUs...", _totalBcuNum);


	    // Creates a BCU COMMUNICATOR for each Bcu
	    // The ip address is that of the corresponding BCU, the local and Bcu udp ports are
	    // the same because the Bcu reply on the destination port, not in the source port !!!
	    int localPort_start = ConfigDictionary()["LocalPort_start"];
	    int bcuPort_start = ConfigDictionary()["BcuPort_start"];
	    int bcuQueueSize = ConfigDictionary()["BcuQueueSize"];
	    int timeout_ms = ConfigDictionary()["Timeout_ms"];
	    int allowedTimeouts = ConfigDictionary()["AllowedTimeouts"];
	    string bcuIp;
	    for(int id=0; id<_totalBcuNum; id++) {
	    	bcuIp = (string)ConfigDictionary()["BcuIp_" + Utils::itoa(id)];
	    	Logger::get("BCUCOMMUNICATOR_"+Utils::itoa(id), ConfigDictionary()["LogLevel"]);
	        _bcuCommunicators.push_back(new BcuCommunicator(localPort_start, id, bcuIp, bcuPort_start, bcuQueueSize, timeout_ms, allowedTimeouts));
	    	localPort_start++;
	    	bcuPort_start++;
	    }
	    
	    
	    // Creates BCU COMMAND HANDLER TESTS for low priority commands
	    for(int id=0; id<(int)ConfigDictionary()["CmdHdlTestNum"]; id++) {
		   Logger::get("BCUCOMMANDHANDLER_TEST_"+Utils::itoa(id), ConfigDictionary()["LogLevel"]);
		   _testCmdHandler[id] = new BcuCommandHandlerTest(id, _totalBcuNum, Priority::LOW_PRIORITY, ConfigDictionary()["PacketNum"]);
	   	}
    
	    // Add BcuPacketQueues to Bcu Communicators
	    for(int i=0; i<_totalBcuNum; i++) {
	        for(int k=0; k<(int)ConfigDictionary()["CmdHdlTestNum"]; k++) {
	        	_bcuCommunicators[i]->addInputQueue(_testCmdHandler[k]->getOutputQueue(i));
	        }
	    }
	    
   	}
   	// Catch exceptions:
   	// - ConfigFile errors
   	// -
    catch(Config_File_Exception e) {
        _logger->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
        throw AOException("Unable to construct the MirrorCtrl object");
    }
}


// -------------------------- PROTECTED -------------------------//

void MirrorCtrl::InstallHandlers(){ 
    _logger->log(Logger::LOG_LEV_DEBUG, "No handlers installed");
}

void MirrorCtrl::Run() {
  
    //1.  Starts the set of BcuCommunicator
    _logger->log(Logger::LOG_LEV_DEBUG, "Starting BcuCommunicator...");
    pthread_t bcuComThreads[_totalBcuNum];
    for(int i=0; i<_totalBcuNum; i++) {
    	bcuComThreads[i] = _bcuCommunicators[i]->start();
    }
    
    //2. Run "runsNum" times a set of "CmdHdlTestNum" BcuCommandHandlerTest
    int cmdHdlNum = ConfigDictionary()["CmdHdlTestNum"];
    int runsNum = ConfigDictionary()["RunsNum"];
    int runsSleep_s = ConfigDictionary()["RunsSleep_s"];
    pthread_t bcuComHdlThreads[cmdHdlNum];
    for(int run=1; run<=runsNum; run++) {
    	_logger->log(Logger::LOG_LEV_DEBUG, ">> Run %d started...", run);
	    for(int i=0; i<cmdHdlNum; i++) {
	    	bcuComHdlThreads[i] = _testCmdHandler[i]->start();
	    }
	    for(int i=0; i<cmdHdlNum; i++) {
	    	pthread_join(bcuComHdlThreads[i], NULL);
	    }
	    if(run != runsNum) {
	    	_logger->log(Logger::LOG_LEV_DEBUG, ">> Run finished, waiting %d s...", runsSleep_s);
	    	sleep(runsSleep_s);
	    }
    }
    
    //3. Checks if stopped or completed
    if(!_timeToDie) {
    	_logger->log(Logger::LOG_LEV_DEBUG, ">> All runs completed");
    }
    else {
    	_logger->log(Logger::LOG_LEV_DEBUG, ">> Runs stopped before completion");
    }
    
    //4. Waits for BcuCommunicators to finish (or stop)
    for(int i=0; i<_totalBcuNum; i++) {
    	pthread_join(bcuComThreads[i], NULL);
    }
    
    //5. Clean BcuCommunicators
    for(int i=0; i<_totalBcuNum; i++) {
    	delete _bcuCommunicators[i];
    }
     for(int i=0; i<cmdHdlNum; i++) {
    	delete _testCmdHandler[i];
    }
    
    _logger->log(Logger::LOG_LEV_WARNING, "<<<--- MIRROR CONTROLLER STOPPED --->>>");
}


// -------------------------- PRIVATE -------------------------//


list<string> MirrorCtrl::readDiagnosticConfig() throw (Config_File_Exception) {

    ifstream* file = NULL;
    try {;
        file = Config_File::openConfigFile(ConfigDictionary()["DiagnConfig"]);
    }
    catch(Config_File_Exception e) {
        // Can't decide what to do
        throw(e);
    }
    
    // Extract text lines
    list<string> diagnLines;
    string cur_line;
    unsigned int commentPos;
    while(!file->eof()) {
    	getline(*file,cur_line);
    	// Ignore comments
    	commentPos = cur_line.find('#');
		if (commentPos != string::npos) 
		{
			//Remove comment ad the end of the line
			cur_line = cur_line.substr(0, commentPos);
		}
		// If something's left
		if (cur_line.size() > 0) {
			//Strip blank characters
        	unsigned int firstGoodChar = cur_line.find_first_not_of(" \t");
	        if (firstGoodChar != std::string::npos) {
	        	cur_line = cur_line.substr(firstGoodChar, cur_line.find_last_not_of(" \t")-firstGoodChar+1);
	        	diagnLines.push_back(cur_line);
	        }	
		}
    }
    file->close();
    return diagnLines;
}

