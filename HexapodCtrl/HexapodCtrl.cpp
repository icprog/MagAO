/*
 * @File: HexapodCtrl.cpp
 * 
 * \HexapodCtrl\ class implementation
 * 
 * This file contains the implementation of \HexapodCtrl\ class
 * @
 */
 
#define VERS_MAJOR   1
#define VERS_MINOR   0

#include <math.h>
#include <limits>

#include "HexapodCtrl.h"
#include "Logger.h"
#include "AOStates.h"
#include "aoslib/aoscodes.h"


extern "C" {
#include "base/timelib.h"
}


using namespace Arcetri;

Hexapod* HexapodCtrl::_hexapod = NULL;


HexapodCtrl::HexapodCtrl( int argc, char **argv) throw (AOException) : AOApp( argc, argv) {
   Create();
}


void HexapodCtrl::Create() throw (AOException) {    
	// Initialize the hexapod "library"
    // TODO a do, while() ??? 
    try {
	    _hexapod = new Hexapod(ConfigDictionary()["HexapodConfig"]);
    } catch (AOException &e) {
        _logger->log(Logger::LOG_LEV_ERROR, "%s",e.str().c_str());
		throw;
    }

	_posUpdatePeriod_us = 100 * (int)ConfigDictionary()["PosUpdatePeriod_100ms"] * 1000;
	_logger->log(Logger::LOG_LEV_DEBUG, "Will update position every %d ms", _posUpdatePeriod_us/1000);
}

HexapodCtrl::~HexapodCtrl() {
	_logger->log(Logger::LOG_LEV_DEBUG, "Destroying...");
	//_hexapod->DeInit(); ??? //NO, you loose the data in the firmware
	delete _hexapod;
}

void HexapodCtrl::SetupVars() {
    try {
    	// Creates (or attaches) variables that STORE hexapod position
    	// This controller maintains the variables updated asking the position
    	// to hexapod with a certain frequency
    	_hexaPos = RTDBvar(MyFullName(), "HXPD_POS", NO_DIR, REAL_VARIABLE, 6);
    } 
    catch (AOVarException &e) { 
    	_logger->log(Logger::LOG_LEV_ERROR, "Impossible to complete SetupVars");
    	throw;
    }
} 

void HexapodCtrl::InstallHandlers() {

	int stat;

	// TODO Handler for all the commands
	if((stat=thHandler(AOS_HXP_CMD, "*", 0, commandsHandler, "commandsHandler", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
        SetTimeToDie(true);
    }
}

void HexapodCtrl::PostInit() {
	
	// TODO
    //  maybe here is the best place to try the connection to the HW?
	// ...
	
	setCurState(STATE_READY);
}

int HexapodCtrl::commandsHandler(MsgBuf *msgb, void * /* argp */, int hndlrQueueSize) {
	if(hndlrQueueSize > HNDLR_QUEUE_LIMIT) {
		Logger::get()->log(Logger::LOG_LEV_ERROR, "HexapodCtrl::commandsHandler -> Queue exceeding normal size...");
		thRelease(msgb);
		Logger::get()->log(Logger::LOG_LEV_FATAL, "HexapodCtrl::commandsHandler -> ...message ignored!");
		return THRD_QUEUE_OVERRUN_ERROR;
	}
	
	// Discover actual command
	int cmdCode = HDR_PLOAD(msgb);
	if(cmdCode == AOS_HXPINIT) {
        try {
	            _logger->log(Logger::LOG_LEV_INFO, "Received home");
                _hexapod->Home();
                // Here it can be blocking and go to the specified position after homing
                thReplyMsg(AOS_ACK, 0, NULL, msgb);
            } catch (AOException &e) {
	            _logger->log(Logger::LOG_LEV_ERROR, "%s",e.str().c_str());
                thReplyMsg(AOS_NACK, 0, NULL, msgb);
            }
	}
	else if(cmdCode == AOS_HXPOPENBRAKE) {
        try {
	            _logger->log(Logger::LOG_LEV_INFO, "Received openbrake");
                _hexapod->OpenBrake();
                thReplyMsg(AOS_ACK, 0, NULL, msgb);
            } catch (AOException &e) {
	            _logger->log(Logger::LOG_LEV_ERROR, "%s",e.str().c_str());
                thReplyMsg(AOS_NACK, 0, NULL, msgb);
            }
	}	  
	else if(cmdCode == AOS_HXPCLOSEBRAKE) {
       try {
	            _logger->log(Logger::LOG_LEV_INFO, "Received closebrake");
                _hexapod->CloseBrake();
                thReplyMsg(AOS_ACK, 0, NULL, msgb);
            } catch (AOException &e) {
	            _logger->log(Logger::LOG_LEV_ERROR, "%s",e.str().c_str());
                thReplyMsg(AOS_NACK, 0, NULL, msgb);
            }
	}
	else if(cmdCode == AOS_HXPNEWREF) {
			thReplyMsg(AOS_NACK, 0, NULL, msgb);
	}
	else if(cmdCode == AOS_HXPMOVETO) {      // These commands require position specification
			try {
                HexaTuple hexaPos = HexaTuple( (double*) (MSG_BODY(msgb)) );
	            _logger->log(Logger::LOG_LEV_INFO, "Received moveto: %s",hexaPos.str().c_str());
			    _hexapod->MoveTo(hexaPos);
                thReplyMsg(AOS_ACK, 0, NULL, msgb);
            } catch (AOException &e) {
	            _logger->log(Logger::LOG_LEV_ERROR, "%s",e.str().c_str());
                thReplyMsg(AOS_NACK, 0, NULL, msgb);
            }
	}
	else if(cmdCode == AOS_HXPMOVEBY) {
			try {
                HexaTuple hexaPos = HexaTuple( (double*) (MSG_BODY(msgb)) );
	            _logger->log(Logger::LOG_LEV_INFO, "Received moveby: %s",hexaPos.str().c_str());
			    _hexapod->MoveBy(hexaPos);
                thReplyMsg(AOS_ACK, 0, NULL, msgb);
            } catch (AOException &e) {
	            _logger->log(Logger::LOG_LEV_ERROR, "%s",e.str().c_str());
                thReplyMsg(AOS_NACK, 0, NULL, msgb);
            }
    } else if(cmdCode == AOS_HXPMOVSPH){
			try {
                double* spherePos = (double*) (MSG_BODY(msgb)) ;
	            _logger->log(Logger::LOG_LEV_INFO, "Received moveonsphere: radius: %g  angles: %g %g",
                        spherePos[0], spherePos[1], spherePos[2]);
			    _hexapod->MoveOnSphere(spherePos[0], spherePos[1], spherePos[2]);
                thReplyMsg(AOS_ACK, 0, NULL, msgb);
            } catch (AOException &e) {
	            _logger->log(Logger::LOG_LEV_ERROR, "%s",e.str().c_str());
                thReplyMsg(AOS_NACK, 0, NULL, msgb);
            }
	}
    else if(cmdCode == AOS_HXPISINITIALIZED) {
			try {
	            _logger->log(Logger::LOG_LEV_INFO, "Received isinitialized");
                bool status = _hexapod->isInitialized();
                thReplyMsgPl(AOS_ACK, status,  0 , NULL, msgb);
            } catch (AOException &e) {
	            _logger->log(Logger::LOG_LEV_ERROR, "%s",e.str().c_str());
                thReplyMsg(AOS_NACK, 0, NULL, msgb);
            }
    } 
    else if(cmdCode == AOS_HXPISMOVING) {
			try {
	            _logger->log(Logger::LOG_LEV_INFO, "Received ismoving [%s:%d]", __FILE__, __LINE__);
                bool status = _hexapod->isMoving();
                thReplyMsgPl(AOS_ACK, status, 0, NULL, msgb);
            } catch (AOException &e) {
	            _logger->log(Logger::LOG_LEV_ERROR, "%s",e.str().c_str());
                thReplyMsg(AOS_NACK, 0, NULL, msgb);
            }
    }

	//case AOS_HXPGETPOS:	// To enable these as "asynchronous commands", the client
	//case AOS_HXPGETABS: 	// reads the RTDB variables that store the hxp position.
							// The client should use the aoslib to do that!
							
    //thRelease(msgb);
	return NO_ERROR;
}

void HexapodCtrl::updateHexapodPositionVars() {
	_logger->log(Logger::LOG_LEV_INFO, "Updating hexapod position RTDB vars...");
	HexaTuple curPos;
	try {
        curPos = _hexapod->GetPos();
    } catch(AOException& e) {
        curPos  = HexaTuple(
                numeric_limits<double>::quiet_NaN(),numeric_limits<double>::quiet_NaN(),
                numeric_limits<double>::quiet_NaN(),numeric_limits<double>::quiet_NaN(),
                numeric_limits<double>::quiet_NaN(),numeric_limits<double>::quiet_NaN()
                );
    }
    vector<double> v = curPos.asVector();
    _logger->log(Logger::LOG_LEV_INFO, "Current hexapod position is %s", curPos.str().c_str());
	_hexaPos.Set(v);
}

void HexapodCtrl::updateHexapodStatus() {
	_logger->log(Logger::LOG_LEV_INFO, "getting hexapod status...");
	HexaStatus curStat;
	try {
        curStat = _hexapod->GetStatus();
    } catch(AOException& e) {
    }
	curStat.log(_logger, Logger::LOG_LEV_INFO);
    //_logger->log(Logger::LOG_LEV_INFO, "Current hexapod status is %s", curStat.str().c_str());
}


void HexapodCtrl::Run() {
    while(!TimeToDie())
    {
       try {
        updateHexapodPositionVars();
        updateHexapodStatus();
        nusleep(_posUpdatePeriod_us);
       }
       catch (TcpException &e) {
        Logger::get()->log(Logger::LOG_LEV_ERROR, "No connection with hexapod: %s", e.what().c_str());
        Logger::get()->log(Logger::LOG_LEV_ERROR, "Reconnect in 5 seconds...\n");
        msleep(5000);
       }
    }
    _logger->log(Logger::LOG_LEV_WARNING, "STOP updating hexapod position and status");
}

int main( int argc, char **argv) {

	HexapodCtrl *ctrl = NULL;

   SetVersion(VERS_MAJOR,VERS_MINOR);

    try {
      ctrl = new HexapodCtrl( argc, argv);

      ctrl->Exec();

    }
    catch(LoggerFatalException &e) {
		// In this case the logger can't log!!!
      	printf("%s\n", e.what().c_str());
   	} 
    catch(AOException& e) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
    }
    
    delete ctrl;
    return 0;
}
