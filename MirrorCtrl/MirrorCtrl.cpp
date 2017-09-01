/*
 * @File: MirrorCtrl.cpp
 * Implements the main class for the Mirror controller process
 * @
 */

#include <stdlib.h>
using namespace std;

extern "C"{
	#include "base/thrdlib.h"
}

#include "WfsConstants.h"
#include "BcuLib/BcuCommon.h"
#include "Logger.h"
#include "AdamLib.h"

#include "MirrorCtrl.h"
#include "Commons/TimeToDie.h"
#include "BcuCommunicator/BcuCommunicator.h"

using namespace Arcetri;
using namespace Arcetri::Bcu;



// -------------------------- PUBLIC -------------------------//

MirrorCtrl::MirrorCtrl( int argc, char **argv) throw (AOException)
           :AOApp(argc, argv) {

	Create();
}

void MirrorCtrl::Create() throw (AOException) {
    
    TimeToDie::init(&_timeToDie);
    
    try {
    	
    	// Get number and IDs of BCUs
        _totalBcuNum = WfsConstants::Bcu39::N_CRATES + WfsConstants::Bcu47::N_CRATES +
                  	   AdSecConstants::BcuSwitch::N_CRATES +
                  	   AdSecConstants::BcuMirror::N_CRATES;
        _mirrorBcuNum = AdSecConstants::BcuMirror::N_CRATES;
        _switchBcuNum = AdSecConstants::BcuSwitch::N_CRATES;
        BcuIdMap bcuIdMap;	// The default constructor is called !!!
        
                  
        _logger->log(Logger::LOG_LEV_INFO, "");
    	_logger->log(Logger::LOG_LEV_INFO, "-------------------- CONSTRUCTING MIRROR-CTRL FOR %d BCUs --------------------", _totalBcuNum);


	    // --- Creates a BCU COMMUNICATOR for each BCU --- //
	    // ----------------------------------------------- //
	    
	    // The ip address is that of the corresponding BCU, the local and Bcu udp ports are
	    // the same because the Bcu reply on the destination port, not in the source port !!!
	    int localPort_start = ConfigDictionary()["LocalPort_start"];
	    int bcuPort_start = ConfigDictionary()["BcuPort_start"];
	    int bcuQueueSize = ConfigDictionary()["BcuQueueSize"];
	    int timeout_ms = ConfigDictionary()["Timeout_ms"];
	    int allowedTimeouts = ConfigDictionary()["AllowedTimeouts"];
	    string bcuIp;
	    
	    // NULL the BcuCommunicators pool
	    for(int i=0; i<9; i++) {
	    	_bcuCommunicators[i] = NULL;
	    }
	    
	    // Creates BCU COMMUNICATOR for Bcu39
	    if(WfsConstants::Bcu39::N_CRATES == 1) {
		    bcuIp = (string)ConfigDictionary()["Bcu_"+ Utils::itoa(bcuIdMap["BCU_39"])];
	    	_bcuCommunicators[bcuIdMap["BCU_39"]] = new BcuCommunicator(localPort_start, bcuIdMap["BCU_39"], bcuIp, bcuPort_start, bcuQueueSize, timeout_ms, allowedTimeouts, Logger::stringToLevel(ConfigDictionary()["LogLevel"]));
	    	localPort_start++;
	    	bcuPort_start++;
	    }
	    
    	// Creates BCU COMMUNICATOR for Bcu47
    	if(WfsConstants::Bcu47::N_CRATES == 1) {
		    bcuIp = (string)ConfigDictionary()["Bcu_"+ Utils::itoa(bcuIdMap["BCU_47"])];
	    	_bcuCommunicators[bcuIdMap["BCU_47"]] = new BcuCommunicator(localPort_start, bcuIdMap["BCU_47"], bcuIp, bcuPort_start, bcuQueueSize, timeout_ms, allowedTimeouts, Logger::stringToLevel(ConfigDictionary()["LogLevel"]));
	    	localPort_start++;
	    	bcuPort_start++;
    	}
    	
    	// Creates BCU COMMUNICATOR for SwitchBcu
    	if(AdSecConstants::BcuSwitch::N_CRATES == 1) {
		    bcuIp = (string)ConfigDictionary()["Bcu_"+ Utils::itoa(bcuIdMap["BCU_SWITCH"])];
	    	_bcuCommunicators[bcuIdMap["BCU_SWITCH"]] = new BcuCommunicator(localPort_start, bcuIdMap["BCU_SWITCH"], bcuIp, bcuPort_start, bcuQueueSize, timeout_ms, allowedTimeouts, Logger::stringToLevel(ConfigDictionary()["LogLevel"]));
	    	localPort_start++;
	    	bcuPort_start++;
    	}
	    
	    // Creates BCU COMMUNICATOR for each Mirror Bcu
	    for(int mirComId=bcuIdMap["BCU_MIRROR_0"]; mirComId<bcuIdMap["BCU_MIRROR_0"] + AdSecConstants::BcuMirror::N_CRATES; mirComId++) {
	    	bcuIp = (string)ConfigDictionary()["Bcu_" + Utils::itoa(mirComId)];
	    	_bcuCommunicators[mirComId] = new BcuCommunicator(localPort_start, mirComId, bcuIp, bcuPort_start, bcuQueueSize, timeout_ms, allowedTimeouts, Logger::stringToLevel(ConfigDictionary()["LogLevel"]));
	    	localPort_start++;
	    	bcuPort_start++;
	    }
	    
	    
	    // --- Creates a Watchdog (if requested) only for the BCU_MIRROR //
	    // ------------------------------------------------------------- //
	    _watchdogMirror = NULL;
	   
	    if ( ((int)ConfigDictionary()["WatchDog_ms"] != 0)  && (_mirrorBcuNum > 0) ) {
	    	int wdId = 0;
	    	_watchdogMirror = new Watchdog(wdId, bcuIdMap["BCU_MIRROR_0"], _mirrorBcuNum, ConfigDictionary()["WatchDog_ms"], Logger::stringToLevel(ConfigDictionary()["WatchdogMirrorLogLevel"]));
	    	// Attach to BcuCommunicators
	    	for(int i = 0; i < _mirrorBcuNum; i++) {
    	 		_bcuCommunicators[i+_watchdogMirror->getBcuStart()]->addInputQueue(_watchdogMirror->getOutputQueue(i));
    	 	} 
	    }
		else {
			_logger->log(Logger::LOG_LEV_WARNING, "Watchdog to Mirror BCUs is disabled");
		}
		
		// --- Creates a Watchdog (if requested) only for the BCU_SWITCH //
	    // -------------------------------------------------------------- //
		_watchdogSwitch = NULL;
	   
	    if ( ((int)ConfigDictionary()["WatchDog_ms"] != 0)  && (_switchBcuNum > 0) ) {
	    	int wdId = 1;
	    	_watchdogSwitch = new Watchdog(wdId, bcuIdMap["BCU_SWITCH"], _switchBcuNum, ConfigDictionary()["WatchDog_ms"], Logger::stringToLevel(ConfigDictionary()["WatchdogSwitchLogLevel"]));
	    	// Attach to BcuCommunicator
	    	_bcuCommunicators[_watchdogSwitch->getBcuStart()]->addInputQueue(_watchdogSwitch->getOutputQueue(0));
	    }
		else {
			_logger->log(Logger::LOG_LEV_WARNING, "Watchdog to Switch BCU is disabled");
		}
		
		// --- Creates a Watchdog (if requested) for the ADAM //
	    // -------------------------------------------------- //
	    _watchdogAdam = NULL;
	    if ( (int)ConfigDictionary()["AdamWatchdogEnable"] != 0 ) {
	    	_watchdogAdam = new AdamWatchdog();
	    }
	    else {
			_logger->log(Logger::LOG_LEV_WARNING, "Watchdog to ADAM is disabled");
		}
		
		// --- CREATES A SET OF SINGLE-BCU COMMAND HANDLERS --- //
		// And attach their output queues to BcuCommunicators
		int id;
		
		_lowPriorityCmdHandler39 = NULL;
        _highPriorityCmdHandler39 = NULL;    
        _lowPriorityCmdHandler47 = NULL;
        _highPriorityCmdHandler47 = NULL;  
        _lowPriorityCmdHandlerSwitch = NULL;
        _highPriorityCmdHandlerSwitch = NULL;
        _lowPriorityCmdHandlerMirror = NULL;
        _highPriorityCmdHandlerMirror = NULL;

	    // --- Creates BCU COMMAND HANDLERS for BCU39 for low and high priority commands 
	    if(WfsConstants::Bcu39::N_CRATES == 1) {
	    	id = 0;
		    _lowPriorityCmdHandler39 = new BcuCommandHandler(id, bcuIdMap["BCU_39"], 1, Priority::LOW_PRIORITY, Logger::stringToLevel(ConfigDictionary()["LogLevel"]));
		    _bcuCommunicators[(int)bcuIdMap["BCU_39"]]->addInputQueue(_lowPriorityCmdHandler39->getOutputQueue(0));
		   
		   _highPriorityCmdHandler39 = new BcuCommandHandler(id, bcuIdMap["BCU_39"], 1, Priority::HIGH_PRIORITY, Logger::stringToLevel(ConfigDictionary()["LogLevel"]));
		   	_bcuCommunicators[(int)bcuIdMap["BCU_39"]]->addInputQueue(_highPriorityCmdHandler39->getOutputQueue(0));
	    }	
	   
	   	// --- Creates BCU COMMAND HANDLERS for BCU47 for low and high priority commands
	   	if(WfsConstants::Bcu47::N_CRATES == 1) {
		    id = 1;
		    _lowPriorityCmdHandler47 = new BcuCommandHandler(id, bcuIdMap["BCU_47"], 1, Priority::LOW_PRIORITY, Logger::stringToLevel(ConfigDictionary()["LogLevel"]));
		    _bcuCommunicators[(int)bcuIdMap["BCU_47"]]->addInputQueue(_lowPriorityCmdHandler47->getOutputQueue(0));
		   
		    _highPriorityCmdHandler47 = new BcuCommandHandler(id, bcuIdMap["BCU_47"],1, Priority::HIGH_PRIORITY, Logger::stringToLevel(ConfigDictionary()["LogLevel"]));
		   	_bcuCommunicators[(int)bcuIdMap["BCU_47"]]->addInputQueue(_highPriorityCmdHandler47->getOutputQueue(0));
	   	}
	   
	   	// --- Creates BCU COMMAND HANDLERS for SWITCHBCU for low and high priority commands
	   	if(AdSecConstants::BcuSwitch::N_CRATES == 1) {
	   		id = 2;
		    _lowPriorityCmdHandlerSwitch = new BcuCommandHandler(id, bcuIdMap["BCU_SWITCH"], 1, Priority::LOW_PRIORITY, Logger::stringToLevel(ConfigDictionary()["LogLevel"]));
		    _bcuCommunicators[(int)bcuIdMap["BCU_SWITCH"]]->addInputQueue(_lowPriorityCmdHandlerSwitch->getOutputQueue(0));
		    
		    _highPriorityCmdHandlerSwitch = new BcuCommandHandler(id, bcuIdMap["BCU_SWITCH"], 1, Priority::HIGH_PRIORITY, Logger::stringToLevel(ConfigDictionary()["LogLevel"]));
		    _bcuCommunicators[(int)bcuIdMap["BCU_SWITCH"]]->addInputQueue(_highPriorityCmdHandlerSwitch->getOutputQueue(0));
	   	}
	   
	   	// --- CREATE 2 COMMAND HANDLER FOR ALL MIRROR BCUs --- //
	   	if(_mirrorBcuNum > 0) {
	   		id = 3;
		    _lowPriorityCmdHandlerMirror = new BcuCommandHandler(id, bcuIdMap["BCU_MIRROR_0"], _mirrorBcuNum, Priority::LOW_PRIORITY, Logger::stringToLevel(ConfigDictionary()["LogLevel"]));
		    for(int i = 0; i <_mirrorBcuNum; i++) {	
		    	_bcuCommunicators[i+bcuIdMap["BCU_MIRROR_0"]]->addInputQueue(_lowPriorityCmdHandlerMirror->getOutputQueue(i));
		    }
		    
			_highPriorityCmdHandlerMirror = new BcuCommandHandler(id, bcuIdMap["BCU_MIRROR_0"], _mirrorBcuNum, Priority::HIGH_PRIORITY, Logger::stringToLevel(ConfigDictionary()["LogLevel"]));
		    for(int i = 0; i <_mirrorBcuNum; i++) {	
		    	_bcuCommunicators[i+bcuIdMap["BCU_MIRROR_0"]]->addInputQueue(_highPriorityCmdHandlerMirror->getOutputQueue(i));
		    }
	   	}
	   
	    // --- Creates a DIAGNOSTIC MANAGER for each entry in "diagn-config-file" --- //
	    // And attach their output queues to BcuCommunicators
	   list<string> diagnConfigLines = readDiagnosticConfig();
        _diagnNum = diagnConfigLines.size();
        _logger->log(Logger::LOG_LEV_INFO, "Constructing %d Diagnostic Managers...", _diagnNum);
        list<string>::iterator iter;
        int diagnManagerId = 0;
        _diagnManagersPool = new DiagnosticManagerPool(Logger::stringToLevel(ConfigDictionary()["DiagnManLogLev"]));
        DiagnosticManager* tempDm;
        string prefix = ConfigDictionary().getDir()+"/";
        printf("%s\n", prefix.c_str());
        for(iter = diagnConfigLines.begin(); iter != diagnConfigLines.end(); iter++) {
            tempDm = new DiagnosticManager(MyFullName(), prefix + (*iter), diagnManagerId, Priority::MEDIUM_PRIORITY, Logger::stringToLevel(ConfigDictionary()["DiagnManLogLev"]));
            _diagnManagersPool->add(tempDm);

        	 diagnManagerId++;
        	 // Attach output queues to BcuCommunicators
        	 for(int i = 0; i < tempDm->getBcuNum(); i++) {
        	 	if(_bcuCommunicators[i+(tempDm->getBcuStart())] != NULL) {
        	 		_bcuCommunicators[i+(tempDm->getBcuStart())]->addInputQueue(tempDm->getOutputQueue(i));
        	 	}
        	 	else {
        	 		_logger->log(Logger::LOG_LEV_WARNING, "> BcuCommunicator %d not running: impossibile to attach DiagnosticManager", i+(tempDm->getBcuStart()));
        	 	}
        	 }
        	 tempDm = NULL;
        }
    
	    
   	}
   	// Catch exceptions:
   	// - ConfigFile errors
   	// - Objects construction errors
    catch(Config_File_Exception& e) {
        _logger->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
        throw MirrorControllerConstructionException("check config file");
    }
    catch(DiagnosticManagerException& e) {
    	 _logger->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
        throw MirrorControllerConstructionException("check diagnostic config files");
    }
}

MirrorCtrl::~MirrorCtrl() {
	_logger->log(Logger::LOG_LEV_INFO, "Destroying MirrorCtrl...");
}


int MirrorCtrl::handle39LpRequest(MsgBuf* msgBuf, void* lpCmdHdlPtr, int hndlrQueueSize) {
	
	if(hndlrQueueSize > HNDLR_QUEUE_LIMIT) {
		Logger::get()->log(Logger::LOG_LEV_ERROR, "MirrorCtrl::handle39LpRequest -> Queue exceeding normal size...");
		thRelease(msgBuf);
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handle39LpRequest -> ...message ignored!");
		return THRD_QUEUE_OVERRUN_ERROR;
	}
	
	// Check again if is a low-priority BCU command...
	if(HDR_CODE(msgBuf) == BCUMSG_39_LP) { 	
		BcuCommandHandler* lpCmdHdlr = (BcuCommandHandler*)(lpCmdHdlPtr);
		//PrintHeader(msgBuf,"MsgHeader -",0);
		return lpCmdHdlr->handleBcuRequest(msgBuf); //[tofix]
	}
	// ...else refuse to manage it !!!
	// MUST NEVER enter HERE !!!
	else {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handle39LpRequest -> Unexpected MsgBuf");
		PrintHeader(msgBuf,"UnexpectedMsg -",0);
		exit(-1);	
		return TBD_ERROR;	//[tofix]
	}
}


int MirrorCtrl::handle39HpRequest(MsgBuf* msgBuf, void* hpCmdHdlPtr, int hndlrQueueSize) {
	
	if(hndlrQueueSize > HNDLR_QUEUE_LIMIT) {
		Logger::get()->log(Logger::LOG_LEV_ERROR, "MirrorCtrl::handle39HpRequest -> Queue exceeding normal size...");
		thRelease(msgBuf);
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handle39HpRequest -> ...message ignored!");
		return THRD_QUEUE_OVERRUN_ERROR;
	}
	
	// Check again if is a high-priority BCU command...
	if(HDR_CODE(msgBuf) == BCUMSG_39_HP) {
		BcuCommandHandler* hpCmdHdlr = (BcuCommandHandler*)(hpCmdHdlPtr);
		return hpCmdHdlr->handleBcuRequest(msgBuf); //[tofix]
	}
	// ...else refuse to manage it !!!
	// MUST NEVER enter HERE !!!
	else {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handle39HpRequest -> Unexpected MsgBuf");
		PrintHeader(msgBuf,"UnexpectedMsg -",0);
		exit(-1);
		return TBD_ERROR;	//[tofix]
	}
}

int MirrorCtrl::handle47LpRequest(MsgBuf* msgBuf, void* lpCmdHdlPtr, int hndlrQueueSize) {
	
	if(hndlrQueueSize > HNDLR_QUEUE_LIMIT) {
		Logger::get()->log(Logger::LOG_LEV_ERROR, "MirrorCtrl::handle47LpRequest -> Queue exceeding normal size...");
		thRelease(msgBuf);
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handle47LpRequest -> ...message ignored!");
		return THRD_QUEUE_OVERRUN_ERROR;
	}
	
	// Check again if is a low-priority BCU command...
	if(HDR_CODE(msgBuf) == BCUMSG_47_LP) { 	
		BcuCommandHandler* lpCmdHdlr = (BcuCommandHandler*)(lpCmdHdlPtr);
		//PrintHeader(msgBuf,"MsgHeader -",0);
		return lpCmdHdlr->handleBcuRequest(msgBuf); //[tofix]
	}
	// ...else refuse to manage it !!!
	// MUST NEVER enter HERE !!!
	else {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handle47LpRequest -> Unexpected MsgBuf");
		PrintHeader(msgBuf,"UnexpectedMsg -",0);
		exit(-1);	
		return TBD_ERROR;	//[tofix]
	}
}

int MirrorCtrl::handle47HpRequest(MsgBuf* msgBuf, void* hpCmdHdlPtr, int hndlrQueueSize) {
	
	if(hndlrQueueSize > HNDLR_QUEUE_LIMIT) {
		Logger::get()->log(Logger::LOG_LEV_ERROR, "MirrorCtrl::handle47HpRequest -> Queue exceeding normal size...");
		thRelease(msgBuf);
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handle47HpRequest -> ...message ignored!");
		return THRD_QUEUE_OVERRUN_ERROR;
	}
	
	// Check again if is a high-priority BCU command...
	if(HDR_CODE(msgBuf) == BCUMSG_47_HP) {
		BcuCommandHandler* hpCmdHdlr = (BcuCommandHandler*)(hpCmdHdlPtr);
		return hpCmdHdlr->handleBcuRequest(msgBuf); //[tofix]
	}
	// ...else refuse to manage it !!!
	// MUST NEVER enter HERE !!!
	else {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handle47HpRequest -> Unexpected MsgBuf");
		PrintHeader(msgBuf,"UnexpectedMsg -",0);
		exit(-1);
		return TBD_ERROR;	//[tofix]
	}
}

int MirrorCtrl::handleSwitchLpRequest(MsgBuf* msgBuf, void* lpCmdHdlPtr, int hndlrQueueSize) {
	
	if(hndlrQueueSize > HNDLR_QUEUE_LIMIT) {
		Logger::get()->log(Logger::LOG_LEV_ERROR, "MirrorCtrl::handleSwitchLpRequest -> Queue exceeding normal size...");
		thRelease(msgBuf);
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handleSwitchLpRequest -> ...message ignored!");
		return THRD_QUEUE_OVERRUN_ERROR;
	}
	
	// Check again if is a low-priority BCU command...
	if(HDR_CODE(msgBuf) == BCUMSG_SWITCH_LP) { 	
		BcuCommandHandler* lpCmdHdlr = (BcuCommandHandler*)(lpCmdHdlPtr);
		//PrintHeader(msgBuf,"MsgHeader -",0);
		return lpCmdHdlr->handleBcuRequest(msgBuf); //[tofix]
	}
	// ...else refuse to manage it !!!
	// MUST NEVER enter HERE !!!
	else {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handleSwitchLpRequest -> Unexpected MsgBuf");
		PrintHeader(msgBuf,"UnexpectedMsg -",0);
		exit(-1);	
		return TBD_ERROR;	//[tofix]
	}
}

int MirrorCtrl::handleSwitchHpRequest(MsgBuf* msgBuf, void* hpCmdHdlPtr, int hndlrQueueSize) {
	
	if(hndlrQueueSize > HNDLR_QUEUE_LIMIT) {
		Logger::get()->log(Logger::LOG_LEV_ERROR, "MirrorCtrl::handleSwitchHpRequest -> Queue exceeding normal size...");
		thRelease(msgBuf);
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handleSwitchHpRequest -> ...message ignored!");
		return THRD_QUEUE_OVERRUN_ERROR;
	}
	
	// Check again if is a high-priority BCU command...
	if(HDR_CODE(msgBuf) == BCUMSG_SWITCH_HP) {
		BcuCommandHandler* hpCmdHdlr = (BcuCommandHandler*)(hpCmdHdlPtr);
		return hpCmdHdlr->handleBcuRequest(msgBuf); //[tofix]
	}
	// ...else refuse to manage it !!!
	// MUST NEVER enter HERE !!!
	else {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handleSwitchHpRequest -> Unexpected MsgBuf");
		PrintHeader(msgBuf,"UnexpectedMsg -",0);
		exit(-1);
		return TBD_ERROR;	//[tofix]
	}
}

int MirrorCtrl::handleMirrorLpRequest(MsgBuf* msgBuf, void* lpCmdHdlPtr, int hndlrQueueSize) {
	
	if(hndlrQueueSize > HNDLR_QUEUE_LIMIT) {
		Logger::get()->log(Logger::LOG_LEV_ERROR, "MirrorCtrl::handleMirrorLpRequest -> Queue exceeding normal size...");
		thRelease(msgBuf);
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handleMirrorLpRequest -> ...message ignored!");
		return THRD_QUEUE_OVERRUN_ERROR;
	}
	
	// Check again if is a low-priority BCU command...
	if(HDR_CODE(msgBuf) == BCUMSG_MIRROR_LP) { 	
		BcuCommandHandler* lpCmdHdlr = (BcuCommandHandler*)(lpCmdHdlPtr);
		//PrintHeader(msgBuf,"MsgHeader -",0);
		return lpCmdHdlr->handleBcuRequest(msgBuf); //[tofix]
	}
	// ...else refuse to manage it !!!
	// MUST NEVER enter HERE !!!
	else {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handleMirrorhLpRequest -> Unexpected MsgBuf");
		PrintHeader(msgBuf,"UnexpectedMsg -",0);
		exit(-1);	
		return TBD_ERROR;	//[tofix]
	}
}

int MirrorCtrl::handleMirrorHpRequest(MsgBuf* msgBuf, void* hpCmdHdlPtr, int hndlrQueueSize) {
	
	if(hndlrQueueSize > HNDLR_QUEUE_LIMIT) {
		Logger::get()->log(Logger::LOG_LEV_ERROR, "MirrorCtrl::handleMirrorHpRequest -> Queue exceeding normal size...");
		thRelease(msgBuf);
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handleMirrorHpRequest -> ...message ignored!");
		return THRD_QUEUE_OVERRUN_ERROR;
	}
	
	// Check again if is a high-priority BCU command...
	if(HDR_CODE(msgBuf) == BCUMSG_MIRROR_HP) {
		BcuCommandHandler* hpCmdHdlr = (BcuCommandHandler*)(hpCmdHdlPtr);
		return hpCmdHdlr->handleBcuRequest(msgBuf); //[tofix]
	}
	// ...else refuse to manage it !!!
	// MUST NEVER enter HERE !!!
	else {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "MirrorCtrl::handleMirrorHpRequest -> Unexpected MsgBuf");
		PrintHeader(msgBuf,"UnexpectedMsg -",0);
		exit(-1);
		return TBD_ERROR;	//[tofix]
	}
}

int MirrorCtrl::handleUnexpectedRequest(MsgBuf* msgBuf, void* /* thisPtr */, int /* hndlrQueueSize */) {
	Logger::get()->log(Logger::LOG_LEV_WARNING, "MirrorCtrl:handleUnexpectedMassage: can't handle a message with code %d", HDR_CODE(msgBuf));
	thRelease(msgBuf);
	return TBD_ERROR;	//[tofix]
}


// -------------------------- PROTECTED -------------------------//

void MirrorCtrl::InstallHandlers() { 
	
	int stat;
	_logger->log(Logger::LOG_LEV_INFO, "");
    _logger->log(Logger::LOG_LEV_INFO, "-------------------- INSTALLING HANDLERS --------------------", _totalBcuNum);
	
    
    // --- Install handlers for Bcu 39 --- //
    if( (_lowPriorityCmdHandler39 == NULL) || (stat=thHandler(BCUMSG_39_LP, "*" ,0 , handle39LpRequest, "BcuCommandLowPriority39", _lowPriorityCmdHandler39))<0) {
         if(_lowPriorityCmdHandler39 != NULL) {
       		 _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from MirrorCtrl::InstallHandlers()", stat, lao_strerror(stat));
        	SetTimeToDie(true);
         }
    }
    else {
    	_logger->log(Logger::LOG_LEV_INFO, "MirrorCtrl low-priority Command handler for Bcu 39 installed");
    }
    if( (_highPriorityCmdHandler39 == NULL) || (stat=thHandler(BCUMSG_39_HP, "*", 0, handle39HpRequest, "BcuCommandHighPriority39", _highPriorityCmdHandler39))<0) {
        if(_highPriorityCmdHandler39 != NULL) {
        	_logger->log(Logger::LOG_LEV_INFO, "Error %d (%s) from MirrorCtrl::InstallHandlers()", stat, lao_strerror(stat));
  			SetTimeToDie(true);
        }
    }
	else {
    	_logger->log(Logger::LOG_LEV_INFO, "MirrorCtrl high-priority Command handler for Bcu 39 installed");
    }
    
    // --- Install handlers for Bcu 47 --- //
    if( (_lowPriorityCmdHandler47 == NULL) || (stat=thHandler(BCUMSG_47_LP, "*" ,0 , handle47LpRequest, "BcuCommandLowPriority47", _lowPriorityCmdHandler47))<0) {
        if(_lowPriorityCmdHandler47 != NULL) {
        	_logger->log(Logger::LOG_LEV_WARNING, "Error %d (%s) from MirrorCtrl::InstallHandlers()", stat, lao_strerror(stat));
    		SetTimeToDie(true);
        }
    }
    else {
    	_logger->log(Logger::LOG_LEV_INFO, "MirrorCtrl low-priority Command handler for Bcu 47 installed");
    }
    if( (_highPriorityCmdHandler47 == NULL) || (stat=thHandler(BCUMSG_47_HP, "*", 0, handle47HpRequest, "BcuCommandHighPriority47", _highPriorityCmdHandler47))<0) {
        if(_highPriorityCmdHandler47 != NULL) {
        	_logger->log(Logger::LOG_LEV_WARNING, "Error %d (%s) from MirrorCtrl::InstallHandlers()", stat, lao_strerror(stat));
    		SetTimeToDie(true);
        }
    }
	else {
    	_logger->log(Logger::LOG_LEV_INFO, "MirrorCtrl high-priority Command handler for Bcu 47 installed");
    }
    
    // --- Install handlers for Bcu Switch --- //
    if( (_lowPriorityCmdHandlerSwitch == NULL) || (stat=thHandler(BCUMSG_SWITCH_LP, "*" ,0 , handleSwitchLpRequest, "BcuCommandLowPrioritySwitch", _lowPriorityCmdHandlerSwitch))<0) {
        if(_lowPriorityCmdHandlerSwitch != NULL) {
        	_logger->log(Logger::LOG_LEV_WARNING, "Error %d (%s) from MirrorCtrl::InstallHandlers()", stat, lao_strerror(stat));
    		SetTimeToDie(true);
        }
    }
    else {
    	_logger->log(Logger::LOG_LEV_INFO, "MirrorCtrl low-priority Command handler for Bcu Switch installed");
    }
    if( (_highPriorityCmdHandlerSwitch == NULL) || (stat=thHandler(BCUMSG_SWITCH_HP, "*", 0, handleSwitchHpRequest, "BcuCommandHighPrioritySwitch", _highPriorityCmdHandlerSwitch))<0) {
        if(_highPriorityCmdHandlerSwitch != NULL) {
        	_logger->log(Logger::LOG_LEV_WARNING, "Error %d (%s) from MirrorCtrl::InstallHandlers()", stat, lao_strerror(stat));
    		SetTimeToDie(true);
        }
    }
	else {
    	_logger->log(Logger::LOG_LEV_INFO, "MirrorCtrl high-priority Command handler for Bcu Switch installed");
    }
    
    // --- Install handlers for Mirror Bcus --- //
    if( (_lowPriorityCmdHandlerMirror == NULL) || (stat=thHandler(BCUMSG_MIRROR_LP, "*" ,0 , handleMirrorLpRequest, "BcuCommandLowPriorityMirror", _lowPriorityCmdHandlerMirror))<0) {
        if(_lowPriorityCmdHandlerMirror != NULL) {
        	_logger->log(Logger::LOG_LEV_WARNING, "Error %d (%s) from MirrorCtrl::InstallHandlers()", stat, lao_strerror(stat));
    		SetTimeToDie(true);
        }
    }
    else {
    	_logger->log(Logger::LOG_LEV_INFO, "MirrorCtrl low-priority Command handler for BcuS Mirror installed");
    }
    if( (_highPriorityCmdHandlerMirror == NULL) || (stat=thHandler(BCUMSG_MIRROR_HP, "*", 0, handleMirrorHpRequest, "BcuCommandHighPriorityMirror", _highPriorityCmdHandlerMirror))<0) {
        if(_highPriorityCmdHandlerMirror != NULL) {
        	_logger->log(Logger::LOG_LEV_WARNING, "Error %d (%s) from MirrorCtrl::InstallHandlers()", stat, lao_strerror(stat));
    		SetTimeToDie(true);
        }
    }
	else {
    	_logger->log(Logger::LOG_LEV_INFO, "MirrorCtrl high-priority Command handler for BcuS Mirror installed");
    }
}


void MirrorCtrl::SetupVars() {
	
	try {
		_techViewerFrameSize = RTDBvar( MyFullName(), "TV_NPIXELS", NO_DIR);
		Notify(_techViewerFrameSize, changeTechViewerFrameSizeHdlr);
		_logger->log(Logger::LOG_LEV_INFO, " -> SUCCESFULLY installed support to run-time change Technical Viewer frame size !");
	}
	catch (AOVarException& e) {
		_logger->log(Logger::LOG_LEV_INFO, " -> NOT installed support to run-time change Technical Viewer frame size !");
	}
}

void MirrorCtrl::Run() {
	
	_logger->log(Logger::LOG_LEV_INFO, "");
	_logger->log(Logger::LOG_LEV_INFO, "-------------------- STARTING MIRROR-CTRL FOR %d BCUs --------------------", _totalBcuNum);
	
  
    //1.  Starts the pool of BCU COMMUNICATORS
    _logger->log(Logger::LOG_LEV_INFO, " >>> Starting BcuCommunicators...");
    pthread_t  bcuComThreads[Constants::MAX_BCU_NUM];
    pthread_t* bcuComThreadsPtr[Constants::MAX_BCU_NUM];
    for(int i=0; i<Constants::MAX_BCU_NUM; i++) {
    	if(_bcuCommunicators[i] != NULL) {
    		bcuComThreads[i] = _bcuCommunicators[i]->start();
    		bcuComThreadsPtr[i]  = &(bcuComThreads[i]);
    	}
    	else {
    		bcuComThreadsPtr[i] = NULL;
    	}
    }
    
    //2. Start the WATCHDOGS
    if(_watchdogMirror != NULL) {
		_logger->log(Logger::LOG_LEV_INFO, " >>> Starting Mirror WatchDog...");
    	_watchdogMirrorThread = _watchdogMirror->start();
	}
	
	if(_watchdogSwitch != NULL) {
		_logger->log(Logger::LOG_LEV_INFO, " >>> Starting Switch WatchDog)...");
    	_watchdogSwitchThread = _watchdogSwitch->start();
	}
	
	
	if(_watchdogAdam != NULL) {
		_logger->log(Logger::LOG_LEV_INFO, " >>> Starting Adam WatchDog...");
    	_watchdogAdam->start();
	}
    
    //3. Start a subset of DIAGNOSTIC MANAGERS
    //	- FastDiagnostic
    //  - SlopeComputer
    // The others (those belonging to TECH_VIEW_ID group) are started by the handler 
    // based on TV_NPIXELS RTDBvariable
    int fastDiagnId = -1;
	try {
		fastDiagnId = _diagnManagersPool->startDiagnosticManager(DiagnosticManagerGroups::FASTDIAGN_ID);
		_logger->log(Logger::LOG_LEV_INFO, "Fast diagnostic (diagnostic manager) succesfully started !!!");
	}
	catch(DiagnosticManagerPoolException &e) {
		_logger->log(Logger::LOG_LEV_WARNING, "%s", e.what().c_str());
		_logger->log(Logger::LOG_LEV_INFO, "Fast diagnostic (diagnostic manager) not started (may be not enabled in MirrorCtrl config file)");
	}
	
    int slopeCompId = -1;
	try {
		slopeCompId = _diagnManagersPool->startDiagnosticManager(DiagnosticManagerGroups::SLOPE_COMP_ID);
		_logger->log(Logger::LOG_LEV_INFO, "Slope computer (diagnostic manager) succesfully started !!!");
	}
	catch(DiagnosticManagerPoolException &e) {
		_logger->log(Logger::LOG_LEV_WARNING, "%s", e.what().c_str());
		_logger->log(Logger::LOG_LEV_INFO, "Slope computer (diagnostic manager) not started (may be not enabled in MirrorCtrl config file)");
	}
    
    //4. ...
    // NOTE that bcuCommandHandler threads are started by the thrdlib 
    // when notified by MsgD. This imply that is impossible to wait
    // them to finish whether a shutdown is requested :-(

	//5. Waits for watchdogs to finish (only for error: otherwise they shouldn't never stop !)
	if(_watchdogMirror != NULL) {
		pthread_join(_watchdogMirrorThread, NULL);
		delete _watchdogMirror;
	}
	if(_watchdogSwitch != NULL) {
		pthread_join(_watchdogSwitchThread, NULL);
		delete _watchdogSwitch;
	}
	
	
	//6. Waits Fast Diagnostic and SlopeComp to finish (only for error or CTRL-C: otherwise 
	//   they should never stop !)
	if(fastDiagnId != -1) {
		_diagnManagersPool->waitDiagnosticManager(fastDiagnId);
	}
	if(slopeCompId != -1) {
		_diagnManagersPool->waitDiagnosticManager(slopeCompId);
	}
	
	//7. Waits for BcuCommunicators to finish (only for error: otherwise they should never stop !)
    for(int i=0; i<Constants::MAX_BCU_NUM; i++) {
    	if(bcuComThreadsPtr[i] != NULL) {
    		pthread_join(bcuComThreads[i], NULL);
    	}
    }
    
    
    // 8. Clear everything 
    // Note that the instances of AbstractBcuRequestManagers are detroyed
    // later because this remove the output-queues attached to BcuCommunicators
    for(int i=0; i<_totalBcuNum; i++) {
    	delete _bcuCommunicators[i];
    }
    delete _diagnManagersPool;
    delete _lowPriorityCmdHandler39;
    delete _highPriorityCmdHandler39;
    delete _lowPriorityCmdHandler47;
    delete _highPriorityCmdHandler47;
    delete _lowPriorityCmdHandlerSwitch;
    delete _highPriorityCmdHandlerSwitch;
    delete _lowPriorityCmdHandlerMirror;
    delete _highPriorityCmdHandlerMirror;
    
    if(_watchdogAdam != NULL) {
    	delete _watchdogAdam; // Automatically stopped before in destruction
	}
    
    _logger->log(Logger::LOG_LEV_WARNING, "MirrorCtrl shutting down... please wait");
    msleep(2000);
}

int MirrorCtrl::changeTechViewerFrameSizeHdlr(void *thisPtr, Variable *var) {
	_logger->log(Logger::LOG_LEV_INFO, "");
	_logger->log(Logger::LOG_LEV_INFO, "--->> MirrorCtrl: changeTechViewerFrameSizeHdlr");
	MirrorCtrl* mc = (MirrorCtrl*)thisPtr;
	mc->changeTechViewerFrameSize(var->Value.Lv[0]);
	return NO_ERROR;
}


// -------------------------- PRIVATE -------------------------//

void MirrorCtrl::changeTechViewerFrameSize(unsigned int frameSizePx) {
	if(frameSizePx != 0) {
		_diagnManagersPool->startDiagnosticManager(DiagnosticManagerGroups::TECH_VIEW_ID, frameSizePx);
	}
	else {
		_diagnManagersPool->stopDiagnosticManagersByGroup(DiagnosticManagerGroups::TECH_VIEW_ID);
	}
}


list<string> MirrorCtrl::readDiagnosticConfig() throw (Config_File_Exception) {

	 _logger->log(Logger::LOG_LEV_INFO, "Serching for Diagnostic Managers config files...", _diagnNum);

    int diagnNum = ConfigDictionary()["DiagnNum"];
    list<string> diagnLines;
    
    for(int i = 1; i <= diagnNum; i++) {
    	_logger->log(Logger::LOG_LEV_DEBUG, "Diagnostic config file %s found", ((string)(ConfigDictionary()["DiagnConfig_"+Utils::itoa(i)])).c_str());
    	diagnLines.push_back(ConfigDictionary()["DiagnConfig_"+Utils::itoa(i)]);
    }
	     
    return diagnLines;
}

