/*
 * @File: CameraLensCtrl.cpp
 * 
 * \CameraLensCtrl\ class implementation
 * 
 * This file contains the implementation of \CameraLensCtrl\ class
 * @
 */

#define VERS_MAJOR   1
#define VERS_MINOR   0
 
#include <math.h>

#include "CameraLensCtrl.h"
#include "Logger.h"
#include "AOStates.h"

using namespace Arcetri;


CameraLensCtrl::CameraLensCtrl( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
}


void CameraLensCtrl::Create() throw (AOException)
{    
   _logger->log(Logger::LOG_LEV_DEBUG, "Constructing...");
   ConfigDictionary().dump();

   try {	
	BcuIdMap idMap;
	_BCU_ID = idMap[ConfigDictionary()["BCU_NAME"]];
	_BCU_DSP = ConfigDictionary()["BCU_DSP"];
	_BCU_ADDRESS = ConfigDictionary()["BCU_ADDRESS"];
	_BCU_TIMEOUT_MS = ConfigDictionary()["BCU_TIMEOUT_MS"];

	_enableOutputsAddress = ConfigDictionary()["BCU_ENABLE_OUT_ADDR"];
	_enableOutputsValue   = ConfigDictionary()["BCU_ENABLE_OUT"];
	
	// Read specific config parameters
	_MAX_X_UM = ConfigDictionary()["MAX_X_UM"];
	_MAX_Y_UM = ConfigDictionary()["MAX_Y_UM"];
	_MIN_X_UM = ConfigDictionary()["MIN_X_UM"];
	_MIN_Y_UM = ConfigDictionary()["MIN_Y_UM"];
	
	_MAX_V_BCU_U = ConfigDictionary()["MAX_V_BCU_U"];
	_MIN_V_BCU_U = ConfigDictionary()["MIN_V_BCU_U"];
	
	_MICRON_PER_VOLT_X = ConfigDictionary()["MICRON_PER_VOLT_X"];
	_MICRON_PER_VOLT_Y = ConfigDictionary()["MICRON_PER_VOLT_Y"];

	_BCUU_PER_VOLT_X = ConfigDictionary()["BCUU_PER_VOLT_X"];
	_BCUU_PER_VOLT_Y = ConfigDictionary()["BCUU_PER_VOLT_Y"];
    } catch (Config_File_Exception &e) {
       _logger->log( Logger::LOG_LEV_FATAL, e.what());
       throw AOException("Fatal: missing configuration data");
    }

    try {
        _invertXY = ConfigDictionary()["invertXY"];
    } catch (Config_File_Exception &e) {
       _logger->log( Logger::LOG_LEV_WARNING, "No keyword 'invertXY' found in configuration file, assuming standard channel ordering");
        _invertXY = 0;
    }

	
	_logger->log(Logger::LOG_LEV_INFO, "----------------- CAMERA LENS CONTROLLER -------------------");
	_logger->log(Logger::LOG_LEV_INFO, " Configuration parameters");
	_logger->log(Logger::LOG_LEV_INFO, "   BCU name (id):      %s (%d)", ((string)ConfigDictionary()["BCU_NAME"]).c_str(), _BCU_ID);
	_logger->log(Logger::LOG_LEV_INFO, "   BCU Dsp:            0x%X", _BCU_DSP);
	_logger->log(Logger::LOG_LEV_INFO, "   BCU address:        0x%X", _BCU_ADDRESS);
	_logger->log(Logger::LOG_LEV_INFO, "   BCU timeout:        %d ms", _BCU_TIMEOUT_MS);
	_logger->log(Logger::LOG_LEV_INFO, "   MAX/MIN X position: %3.3f/%3.3f (um)", _MAX_X_UM, _MIN_X_UM);
	_logger->log(Logger::LOG_LEV_INFO, "   MAX/MIN Y position: %3.3f/%3.3f (um)", _MAX_Y_UM, _MIN_Y_UM);
	_logger->log(Logger::LOG_LEV_INFO, "   MAX/MIN VOLTAGE:    0x%X/0x%X (bcu-U)", _MAX_V_BCU_U, _MIN_V_BCU_U);
	_logger->log(Logger::LOG_LEV_INFO, "   UM/V position X,Y:  %2.2f, %2.2f", _MICRON_PER_VOLT_X, _MICRON_PER_VOLT_Y);
	_logger->log(Logger::LOG_LEV_INFO, "   BCU_U/V X,Y:        %2.2f, %2.2f", _BCUU_PER_VOLT_X, _BCUU_PER_VOLT_Y);
	_logger->log(Logger::LOG_LEV_INFO, "------------------------------------------------------------\n");
	
	// Create the CommandSender to communicate with the BCU47
	_bcuCommandSender = new CommandSender();


}

CameraLensCtrl::~CameraLensCtrl() {

	_logger->log(Logger::LOG_LEV_DEBUG, "Destroying...");

}

void CameraLensCtrl::SetupVars() {
    try {
    	_logger->log(Logger::LOG_LEV_DEBUG, "Setting up position X and Y variables......");
    	
        _posX_req = RTDBvar( MyFullName(), "POS_X", REQ_VAR, REAL_VARIABLE, 1);
        _posY_req = RTDBvar( MyFullName(), "POS_Y", REQ_VAR, REAL_VARIABLE, 1);
        _posX_cur = RTDBvar( MyFullName(), "POS_X", CUR_VAR, REAL_VARIABLE, 1);
        _posY_cur = RTDBvar( MyFullName(), "POS_Y", CUR_VAR, REAL_VARIABLE, 1);

        _posX_req.Set( _MIN_X_UM);
        _posY_req.Set( _MIN_Y_UM);
       
        Notify(_posX_req, changeLensPosition_hdlr);
        Notify(_posY_req, changeLensPosition_hdlr);
    } 
    catch (AOVarException &e) { 
    	_logger->log(Logger::LOG_LEV_ERROR, "Impossible to complete SetupVars");
    	throw;
    }

	// Wait for TTCtrl 
   string ttCtrlFullName = (string)ConfigDictionary()["ttProcess"]+"."+Side();

     while(1) {
          int ret;
          _logger->log( Logger::LOG_LEV_INFO, "Waiting for tip.tilt controller to become ready....", ttCtrlFullName.c_str());
          if ((ret = thWaitReady( ttCtrlFullName.c_str(), 60 * 1000)) == NO_ERROR)
             break;
          if (ret != TIMEOUT_ERROR)
          throw AOException("Error waiting for TT controller to become ready", ret, __FILE__, __LINE__);
         }
} 

void CameraLensCtrl::PostInit() {
	
	setCurState(STATE_READY);
	
	// Send the default positions to BCU47
	sendPositionsToBcu(_MIN_X_UM, _MIN_Y_UM);
}


int CameraLensCtrl::changeLensPosition_hdlr(void *pt, Variable *var) {
	CameraLensCtrl* clc = (CameraLensCtrl*)pt;
	clc->changeLensPosition(var);
	return NO_ERROR;
}


void CameraLensCtrl::changeLensPosition(Variable *var) {
	
	//--- Check for the updated position ---//
	if(_posX_req.MatchAndSet(var) == 1 ) {
		_logger->log(Logger::LOG_LEV_DEBUG, "Requested position X updated");	
	}
	else if(_posY_req.MatchAndSet(var) == 1) {
		_logger->log(Logger::LOG_LEV_DEBUG, "Requested position Y updated");
	}
	else { //This case should NEVER happens!
		_logger->log(Logger::LOG_LEV_DEBUG, "No position updated (Var doesn't match: shouldn't happens!)");
		return;
	}
	double posX,posY;
	_posX_req.Get(&posX);
	_posY_req.Get(&posY);
	_logger->log(Logger::LOG_LEV_INFO, "New requested lens positions (in micron): X=%3.3f Y=%3.3f", posX, posY);
	
	//--- Check and fix the requested lens positions ---//
	if(posX>_MAX_X_UM) {
		posX = _MAX_X_UM;
	}
	else if(posX<_MIN_X_UM) {
		posX = _MIN_X_UM;
	}
	if(posY>_MAX_Y_UM) {
		posY = _MAX_Y_UM;
	}
	else if(posY<_MIN_Y_UM) {
		posY = _MIN_Y_UM;
	}
	_logger->log(Logger::LOG_LEV_INFO, "Fixed requested lens positions (in micron): X=%3.3f Y=%3.3f", posX, posY);
	
	//--- Apply the new positions, converting the position to BCU unit, and update cur RTDB values ---//
	sendPositionsToBcu(posX, posY);
	
}

void CameraLensCtrl::sendPositionsToBcu(double posX, double posY) {
	
	// Convert the position to volts
	double posX_V, posY_V;
	posX_V = (posX + fabs(_MIN_X_UM)) / _MICRON_PER_VOLT_X;
	posY_V = (posY + fabs(_MIN_Y_UM)) / _MICRON_PER_VOLT_Y;
	_logger->log(Logger::LOG_LEV_INFO, "Requested lens positions (in Volts): X=%3.3f Y=%3.3f", posX_V, posY_V);
	
	_logger->log(Logger::LOG_LEV_DEBUG, "Sending positions to BCU47...");
	try {
		if(_bcuCommandSender==NULL) {
			_bcuCommandSender = new CommandSender();
		}
		
		// Convert to BCU units and check for safety
		LensPosition lensPosition[2];	//X and Y position
      	memset(lensPosition, 0, sizeof(LensPosition)*2);
		
		uint16 posX_bcu_u, posY_bcu_u;

  		posX_bcu_u = (uint16) (posX_V * _BCUU_PER_VOLT_X);
  		posY_bcu_u = (uint16) (posY_V * _BCUU_PER_VOLT_Y);

  		_logger->log(Logger::LOG_LEV_DEBUG, "Posx: %f * %f = 0x%04X (bcu-U)", posX_V, _BCUU_PER_VOLT_X, posX_bcu_u);
  		_logger->log(Logger::LOG_LEV_DEBUG, "Posy: %f * %f = 0x%04X (bcu-U)", posY_V, _BCUU_PER_VOLT_Y, posY_bcu_u);

  		_logger->log(Logger::LOG_LEV_DEBUG, "Limits: 0x%04X - 0x%04X (bcu-U)", _MIN_V_BCU_U, _MAX_V_BCU_U);
  
   
		if(posX_bcu_u>_MAX_V_BCU_U) {
			posX_bcu_u = _MAX_V_BCU_U;
		}
		else if(posX_bcu_u<_MIN_V_BCU_U) {
			posX_bcu_u = _MIN_V_BCU_U;
		}
		if(posY_bcu_u>_MAX_V_BCU_U) {
			posY_bcu_u = _MAX_V_BCU_U;
		}
		else if(posY_bcu_u<_MIN_V_BCU_U) {
			posY_bcu_u = _MIN_V_BCU_U;
		}

                if (!_invertXY) {	
		    lensPosition[0].pos = posX_bcu_u;
		    lensPosition[1].pos = posY_bcu_u;
                } else {
		    lensPosition[1].pos = posX_bcu_u;
		    lensPosition[0].pos = posY_bcu_u;
                }

        _logger->log(Logger::LOG_LEV_DEBUG, "Sending low level commands: 0x%04X  0x%04X  \n", posX_bcu_u, posY_bcu_u);

	    // Send the new values
		_bcuCommandSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, 
										 	   _BCU_ID, 
										 	   _BCU_DSP, _BCU_DSP, 
	    								 	   OpCodes::MGP_OP_WRITE_WAVEGEN_RAM, // [to check]
	    								 	   _BCU_ADDRESS, 
	    								 	   sizeof(LensPosition)*2,	//X and Y position
	    								 	   (unsigned char*)(&lensPosition), 
	    								 	   _BCU_TIMEOUT_MS, 
	    								 	   Constants::WANTREPLY_FLAG);  

	     // Enable the bcu outputs
    	_bcuCommandSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, 
									 		    _BCU_ID, 
										 	    _BCU_DSP, _BCU_DSP,  
    								 		    OpCodes::MGP_OP_WRITE_WAVEGEN_RAM, 
    								 		    _enableOutputsAddress, 
    								 		    sizeof(_enableOutputsValue),	
    								 		    (unsigned char*)(&_enableOutputsValue), 
    								 		    _BCU_TIMEOUT_MS, 
    								 		    Constants::WANTREPLY_FLAG); 
    	Logger::get()->log(Logger::LOG_LEV_DEBUG, "BCU outputs succesfully enabled!");
     
		
		// Update cur variables
		_logger->log(Logger::LOG_LEV_DEBUG, "Updating current position variables to X=%3.3f Y=%3.3f", posX, posY);
		_posX_cur.Set(posX, 0, CHECK_SEND);
		_posY_cur.Set(posY, 0, CHECK_SEND);
	}
	catch(CommandSenderTimeoutException& e) {
		Logger::get()->log(Logger::LOG_LEV_ERROR, "Error in sending positions to BCU47 (detail: %s)", e.what().c_str());
		delete _bcuCommandSender;
		_bcuCommandSender = NULL;
	}
	catch(CommandSenderException& e) {
		Logger::get()->log(Logger::LOG_LEV_ERROR, "Error in sending positions to BCU47 (detail: %s)", e.what().c_str());
	}
}

              			 

int main( int argc, char **argv) {

    SetVersion(VERS_MAJOR,VERS_MINOR);

    try {
      CameraLensCtrl *ctrl;

      ctrl = new CameraLensCtrl( argc, argv);

      ctrl->Exec();
    }
    catch(AOException& e) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
    }
    
    return 0;
}
