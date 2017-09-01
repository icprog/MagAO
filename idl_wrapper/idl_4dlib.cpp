#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string>

extern "C"{
	#include "idl_export.h"
	#include "base/errordb.h"
	#include "base/common.h"
	#include "base/thrdlib.h"
}
#include "idl_4dlib.h"
#include "AdSecConstants.h"
#include "Logger.h"
#include "AdamLib.h"
#include "stdconfig.h"
#include "IntfLib/InterferometerInterface.h"

using namespace std;
using namespace Arcetri;
using namespace AdSecConstants;


InterferometerInterface* _4DInterface = NULL;

IDL_VPTR idl_4D_init(int /* lArgc */, IDL_VPTR /* Argv */[]) {

	Logger* _logger = Logger::get();
	
	if(_4DInterface == NULL) _4DInterface = new InterferometerInterface(_logger->getLevel());

	return IDL_GettmpLong(NO_ERROR);
}


IDL_VPTR idl_4D_finalize(int /* lArgc */, IDL_VPTR /* Argv */[]) {

	delete _4DInterface;
	_4DInterface = NULL;

	return IDL_GettmpLong(NO_ERROR);
}

IDL_VPTR idl_4D_setTrigger(int lArgc, IDL_VPTR Argv[]) {

	Logger* _logger = Logger::get();

	if(_4DInterface == NULL) {
		_logger->log(Logger::LOG_LEV_INFO, "Please initialize (IDL_4D_INIT) the 4D interface first!");
		return IDL_GettmpLong(IDL_4D_NOT_INIT_ERROR);
	}

	 // Check arguments
	if (lArgc != 1) {
    	_logger->log(Logger::LOG_LEV_ERROR, "idl_4D_setTrigger: wrong parameters number (1 required, %d received)", lArgc);
    	return IDL_GettmpLong(IDL_PAR_NUM_ERROR);
    }

	// Retrieve arguments
	bool enable = false;
	if (IDL_LongScalar(Argv[0]) == 1) {
		enable = true;
	}

	// Do action
	if(_4DInterface->setTrigger(enable)) {
		return IDL_GettmpLong(NO_ERROR);
	}
	else {
		return IDL_GettmpLong(INTERFEROMETER_PARPORT_ERROR);
	}
}

IDL_VPTR idl_4D_setPostProc(int lArgc, IDL_VPTR Argv[]) {

	Logger* _logger = Logger::get();

	if(_4DInterface == NULL) {
		_logger->log(Logger::LOG_LEV_INFO, "Please initialize (IDL_4D_INIT) the 4D interface first!");
		return IDL_GettmpLong(IDL_4D_NOT_INIT_ERROR);
	}

	 // Check arguments
	if (lArgc != 1) {
    	_logger->log(Logger::LOG_LEV_ERROR, "idl_4D_setPostProc: wrong parameters number (1 required, %d received)", lArgc);
    	return IDL_GettmpLong(IDL_PAR_NUM_ERROR);
    }

	// Retrieve arguments
	string ppType = IDL_VarGetString(Argv[0]);

	// Do setting
	_4DInterface->setPostProcessing(ppType);

	return IDL_GettmpLong(NO_ERROR);
}


IDL_VPTR idl_4D_getMeasure(int lArgc, IDL_VPTR Argv[]) {

	Logger* _logger = Logger::get();

	if(_4DInterface == NULL) {
		_logger->log(Logger::LOG_LEV_INFO, "Please initialize (IDL_4D_INIT) the 4D interface first!");
		return IDL_GettmpLong(IDL_4D_NOT_INIT_ERROR);
	}

	 // Check arguments
	 if (lArgc != 1) {
    	_logger->log(Logger::LOG_LEV_ERROR, "idl_4D_getMeasure: wrong parameters number (1 required, %d received)", lArgc);
    	return IDL_GettmpLong(IDL_PAR_NUM_ERROR);
    }

    // Retrieve arguments
    string measName = IDL_VarGetString(Argv[0]);

    // Do acquisition
	_logger->log(Logger::LOG_LEV_INFO, "idl_4D_getMeasure start: %s", Utils::asciiDateAndTime().c_str());
    string remoteFileName = _4DInterface->getMeasurement(measName);
    _logger->log(Logger::LOG_LEV_INFO, "idl_4D_getMeasure end: %s", Utils::asciiDateAndTime().c_str());

	// Check and return
    if(remoteFileName == "") {
		_logger->log(Logger::LOG_LEV_ERROR, "Measurement acquisition failed");
		return IDL_GettmpLong(IDL_4D_ACQUISITION_ERROR);
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Measurement acquisition success!");
		return IDL_StrToSTRING((char*)remoteFileName.c_str());
	}
}

IDL_VPTR idl_4D_getBurst(int lArgc, IDL_VPTR Argv[]) {

	Logger* _logger = Logger::get();

	if(_4DInterface == NULL) {
		_logger->log(Logger::LOG_LEV_INFO, "Please initialize (IDL_4D_INIT) the 4D interface first!");
		return IDL_GettmpLong(IDL_4D_NOT_INIT_ERROR);
	}

	 // Check arguments
	 if(lArgc < 1) {
    	_logger->log(Logger::LOG_LEV_ERROR, "idl_4D_getBurst: wrong parameters number (1 required + 1 optional, %d received)", lArgc);
    	return IDL_GettmpLong(IDL_PAR_NUM_ERROR);
    }

    // Retrieve arguments
    int measNum = IDL_LongScalar(Argv[0]);
    string burstName = "";
    if(lArgc == 2) {
    	burstName = IDL_VarGetString(Argv[1]);
    }

    // Do acquisition
    _logger->log(Logger::LOG_LEV_INFO, "idl_4D_getBurst start: %s", Utils::asciiDateAndTime().c_str());
     string remotePath = _4DInterface->getMeasurementBurst(measNum, burstName);
    _logger->log(Logger::LOG_LEV_INFO, "idl_4D_getBurst end: %s", Utils::asciiDateAndTime().c_str());

    // Check and return
    if(remotePath == "") {
		_logger->log(Logger::LOG_LEV_ERROR, "Burst acquisition failed");
		return IDL_GettmpLong(IDL_4D_ACQUISITION_ERROR);
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Burst acquisition success!");
		return IDL_StrToSTRING((char*)remotePath.c_str());
	}
}


/*
 * NOTE: the 4D PhaseCam has the TRIGGER IN forced to TTL HIGH, and is
 * triggered by a TTL LOW. This implementation is based on a INVERTED
 * behaviour, because of a custom link between parallel port and
 * interferometer.
 * - Parallel port TRIGGER_OUT is set to TTL LOW
 * - A trigger is sent as TTL HIGH -> TTL LOW
 */
IDL_VPTR idl_4D_trigger(int lArgc, IDL_VPTR /* Argv */[]) {

	Logger* _logger = Logger::get();

	if(_4DInterface == NULL) {
		_logger->log(Logger::LOG_LEV_INFO, "Please initialize (IDL_4D_INIT) the 4D interface first!");
		return IDL_GettmpLong(IDL_4D_NOT_INIT_ERROR);
	}

	// Check arguments
	if (lArgc != 0) {
    	_logger->log(Logger::LOG_LEV_ERROR, "idl_4D_trigger: wrong parameters number (0 required, %d received)", lArgc);
    	return IDL_GettmpLong(IDL_PAR_NUM_ERROR);
    }

	// Do triggering
	_4DInterface->trigger();

    return IDL_GettmpLong(NO_ERROR);
}

IDL_VPTR idl_4D_postProcess(int lArgc, IDL_VPTR Argv[]) {

	Logger* _logger = Logger::get();

	if(_4DInterface == NULL) {
		_logger->log(Logger::LOG_LEV_INFO, "Please initialize (IDL_4D_INIT) the 4D interface first!");
		return IDL_GettmpLong(IDL_4D_NOT_INIT_ERROR);
	}

	 // Check arguments
	if (lArgc < 2) {
    	_logger->log(Logger::LOG_LEV_ERROR, "idl_4D_postProc: wrong parameters number (2 required, %d received)", lArgc);
    	return IDL_GettmpLong(IDL_PAR_NUM_ERROR);
    }

	// Retrieve arguments
	string sourcePath = IDL_VarGetString(Argv[0]);
	string measPrefix = IDL_VarGetString(Argv[1]);
	int measNum = 1;
	if(lArgc == 3) {
		measNum = IDL_LongScalar(Argv[2]);
	}

	// Do post processing
	try {
		_logger->log(Logger::LOG_LEV_INFO, "idl_4D_postProcess start: %s", Utils::asciiDateAndTime().c_str());
		string dest = _4DInterface->postProcess(sourcePath, measPrefix, measNum);
		 _logger->log(Logger::LOG_LEV_INFO, "idl_4D_postProcess end: %s", Utils::asciiDateAndTime().c_str());

		// Check and return
		if(dest == "") {
			_logger->log(Logger::LOG_LEV_ERROR, "Post-processing failed");
			return IDL_GettmpLong(IDL_4D_POSTPROC_ERROR);
		}
		else {
			_logger->log(Logger::LOG_LEV_ERROR, "Processing success!");
			return IDL_StrToSTRING((char*)dest.c_str());
		}
	}
	catch(AOException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, "Post-processing not done (post-proc type is NONE)");
		return IDL_GettmpLong(IDL_4D_POSTPROC_NOTDONE);
	}
}


/** 
 * Capture
 * @param n_frames
 * @param name (optional)
 */
IDL_VPTR idl_4D_capture(int lArgc, IDL_VPTR Argv[]) {

    Logger* _logger = Logger::get();
    
    // Check 4D interface
    if(_4DInterface == NULL) {
	_logger->log(Logger::LOG_LEV_INFO, "Please initialize (IDL_4D_INIT) the 4D interface first!");
	return IDL_GettmpLong(IDL_4D_NOT_INIT_ERROR);
    }

    // Check arguments
    if ((lArgc < 1) || (lArgc > 2)) {
    	_logger->log(Logger::LOG_LEV_ERROR, "idl_4D_capture: wrong parameters number (1 required + 1 optional, %d received)", lArgc);
    	return IDL_GettmpLong(IDL_PAR_NUM_ERROR);
    }

    // Get arguments
    int n_frames = IDL_LongScalar(Argv[0]);
    string name = (lArgc == 2) ? IDL_VarGetString(Argv[1]) : "";

    // Call function 
    _logger->log(Logger::LOG_LEV_INFO, "idl_4D_capture start: %s", Utils::asciiDateAndTime().c_str());
    string reply = _4DInterface->capture(n_frames, name);
    _logger->log(Logger::LOG_LEV_INFO, "idl_4D_capture end: %s", Utils::asciiDateAndTime().c_str());

    // Check and return
    if (reply == "") {
	_logger->log(Logger::LOG_LEV_ERROR, "Capture acquisition failed");
	return IDL_GettmpLong(IDL_4D_ACQUISITION_ERROR);
    }
    else {
	_logger->log(Logger::LOG_LEV_INFO, "Capture acquisition successed");
	return IDL_StrToSTRING((char*)reply.c_str());
    }
}


/**
 * Produce
 * @param folder
 */
IDL_VPTR idl_4D_produce(int lArgc, IDL_VPTR Argv[]) {

    Logger* _logger = Logger::get();
    
    if(_4DInterface == NULL) {
	_logger->log(Logger::LOG_LEV_INFO, "Please initialize (IDL_4D_INIT) the 4D interface first!");
	return IDL_GettmpLong(IDL_4D_NOT_INIT_ERROR);
    }

/*
    // Check arguments
    if ((lArgc <= 1) || (lArgc >= 4)) {
    	_logger->log(Logger::LOG_LEV_ERROR, "idl_4D_produce: wrong parameters number (1 required, %d received)", lArgc);
    	return IDL_GettmpLong(IDL_PAR_NUM_ERROR);
    }

    // Get arguments
    string name = IDL_VarGetString(Argv[0]);
    int from, to;
    if (lArgc == 2) {
       from = IDL_LongScalar(Argv[1]);
       to = IDL_LongScalar(Argv[1]);
    } else {
       from = IDL_LongScalar(Argv[1]);
       to = IDL_LongScalar(Argv[2]);
    }
*/

    // temporal one argument
    string name = IDL_VarGetString(Argv[0]);


     // Call function 
    _logger->log(Logger::LOG_LEV_INFO, "idl_4D_produce start: %s", Utils::asciiDateAndTime().c_str());
    string reply = _4DInterface->produce(name); // , from, to);
    _logger->log(Logger::LOG_LEV_INFO, "idl_4D_produce end: %s", Utils::asciiDateAndTime().c_str());
   
    // Check and return
    if (reply == "") {
	_logger->log(Logger::LOG_LEV_ERROR, "Capture acquisition failed");
	return IDL_GettmpLong(IDL_4D_ACQUISITION_ERROR);
    }
    else {
	_logger->log(Logger::LOG_LEV_INFO, "Capture acquisition successed");
	return IDL_StrToSTRING((char*)reply.c_str());
    }
}

