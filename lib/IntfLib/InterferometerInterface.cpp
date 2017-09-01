#include "InterferometerInterface.h"
#include "Commands.h"


extern "C" {
#include "base/timelib.h"
}


// TODO Check
#define TIMEOUT_4D_SEC 0       // Wait reply forever
                               // TODO Evaluate better timeout problem



InterferometerInterface::InterferometerInterface(int LogLevel) {

	_logger = Logger::get("INTERF-LIB", LogLevel);

	// Set the MsgD identity of the library server, used to send commands to
	// InterferometerCtrl via MsgD
	// TODO get this form config file?!
	// _interferometerCtrlMsgdIdentity = "INTERFCTRL00";
    _interferometerCtrlMsgdIdentity = "interferometerctrl." + Utils::getAdoptSide();

	_logger->log(Logger::LOG_LEV_INFO, "Creating InterferometerInterface interface to communicate with %s", _interferometerCtrlMsgdIdentity.c_str());

	// The external trigger, implemented with a parallel port pin,
	// is non used by default
	_parallelPortInitialized = false;
}

InterferometerInterface::~InterferometerInterface() {
	// Do nothing, at the moment
}

bool InterferometerInterface::setTrigger(bool enable) {

	// --- Ensure the parallel port is initialized --- //
	if(enable && !_parallelPortInitialized) {
		// Check root privileges (needed to setup the parallel port)
		_logger->log(Logger::LOG_LEV_INFO, "Checking if root privileges granted...");
		int uid = getuid();
		_logger->log(Logger::LOG_LEV_INFO, "Process UID: %d (0=root)", uid);
		if(uid != 0) {
			_logger->log(Logger::LOG_LEV_ERROR, "Root privileges needed to setup the parallel port!");
			return false;
		}

		// Configure the parallel port
		if(pin_init_user(LPT1) == 0) {
			// Config the TRIGGER_OUT as output
			pin_output_mode(TRIGGER_OUT);
			// Set TTL level low
			clear_pin(TRIGGER_OUT);
			_logger->log(Logger::LOG_LEV_INFO, "Parallel port succesfully initialized");
		}
		else {
			_logger->log(Logger::LOG_LEV_INFO, "Parallel port initialization failed");
			return false;
		}

		// Call "setuid" to drop root privileges
//		if(setuid(??) == 0) {
//			_logger->log(Logger::LOG_LEV_INFO, "Root privileges dropped!");
//		}
//		else {
//			_logger->log(Logger::LOG_LEV_INFO, "Root privileges not dropped :-(");
//		}

		_parallelPortInitialized = true;
	}

	// --- Actually enable/disable the trigger --- //
	int setTriggerCmdCode;
	if(enable) {
		setTriggerCmdCode = INTF_ENABLE_TRIGGER;
	}
	else {
		setTriggerCmdCode = INTF_DISABLE_TRIGGER;
	}

	_logger->log(Logger::LOG_LEV_INFO, "setTrigger to %d...", enable);
	int seqNum = thSendMsgPl(0, (char*)_interferometerCtrlMsgdIdentity.c_str(), INTF_COMMAND, setTriggerCmdCode, 0, NULL);
	if (IS_ERROR(seqNum)) {
		_logger->log(Logger::LOG_LEV_ERROR, "...request failed!");
		return false;
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "....request succesfully sent! Waiting reply...");
		int stat;
		MsgBuf* replyBuf = thWaitMsg(INTF_COMMAND, (char*)_interferometerCtrlMsgdIdentity.c_str(), 0, TIMEOUT_4D_SEC, &stat);
		// Check if reply succesfully got
		if (!replyBuf) {
			_logger->log(Logger::LOG_LEV_ERROR, "Error in thWaitMsg: %d (%s)", stat, lao_strerror(stat));
	      	return false;
	    }
	    _logger->log(Logger::LOG_LEV_INFO, "Reply succesfully received");
	    bool result = HDR_PLOAD(replyBuf);
	     _logger->log(Logger::LOG_LEV_INFO, "Result: %d", result);

	    thRelease(replyBuf);
	    return result;
	}
}


bool InterferometerInterface::setPostProcessing(string typeAsString) {

	int postProcCmdCode = INTF_SET_POST_PROC_NONE;
	if(typeAsString=="HDF5") {
		postProcCmdCode = INTF_SET_POST_PROC_HDF5;
	}

	_logger->log(Logger::LOG_LEV_INFO, "setPostProcess to %s...", typeAsString.c_str());
	int seqNum = thSendMsgPl(0, (char*)_interferometerCtrlMsgdIdentity.c_str(), INTF_COMMAND, postProcCmdCode, 0, NULL);
	if (IS_ERROR(seqNum)) {
		_logger->log(Logger::LOG_LEV_ERROR, "...request failed!");
		return false;
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "....request succesfully sent! Waiting reply...", postProcCmdCode, typeAsString.c_str());
		int stat;
		MsgBuf* replyBuf = thWaitMsg(INTF_COMMAND, (char*)_interferometerCtrlMsgdIdentity.c_str(), 0, TIMEOUT_4D_SEC, &stat);
		// Check if reply succesfully got
		if (!replyBuf) {
			_logger->log(Logger::LOG_LEV_ERROR, "Error in thWaitMsg: %d (%s)", stat, lao_strerror(stat));
	      	return false;
	    }
	    _logger->log(Logger::LOG_LEV_INFO, "Reply succesfully received");
	    bool result = HDR_PLOAD(replyBuf);
	    _logger->log(Logger::LOG_LEV_INFO, "Result: %d", result);

	    thRelease(replyBuf);
	    return result;
	}
}


string InterferometerInterface::getMeasurement(string measPrefix) {

	MeasurementParams params = ParamsFactory::CreateMeasurementParams(measPrefix);

	// Ask to InterferometerCtrl to get a measurement from the interferometer
	_logger->log(Logger::LOG_LEV_INFO, "getMeasurement with prefix %s...", params.measPrefix);
	int seqNum = thSendMsgPl(sizeof(params), (char*)_interferometerCtrlMsgdIdentity.c_str(), INTF_COMMAND, INTF_GET_MEAS, 0, (void*)(&params));
	if (IS_ERROR(seqNum)) {
		_logger->log(Logger::LOG_LEV_ERROR, "...request failed!");
		return "";
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "....request succesfully sent! Waiting reply...");
		int stat;
		MsgBuf* replyBuf = thWaitMsg(INTF_COMMAND, (char*)_interferometerCtrlMsgdIdentity.c_str(), 0, TIMEOUT_4D_SEC, &stat);
		// Check if reply succesfully got
		if (!replyBuf) {
			_logger->log(Logger::LOG_LEV_ERROR, "Error in thWaitMsg: %d (%s)", stat, lao_strerror(stat));
	      	return "";
	    }
	    _logger->log(Logger::LOG_LEV_INFO, "Reply succesfully received");
	    MeasurementParams* params = (MeasurementParams*)(MSG_BODY(replyBuf));
	    string remoteFile = params->remoteFileOut;
	    _logger->log(Logger::LOG_LEV_INFO, "Result: %s", remoteFile.c_str());

	    thRelease(replyBuf);
	    return remoteFile;
	}
}

string InterferometerInterface::getMeasurementBurst(int measNum, string burstName) {

	MeasurementBurstParams params = ParamsFactory::CreateMeasurementBurstParams(measNum, burstName);

	// Ask to InterferometerCtrl to get a measurement from the interferometer
	_logger->log(Logger::LOG_LEV_INFO, "getMeasurementBurst of %d measurements with name %s...", params.measNum, params.burstName);
	int seqNum = thSendMsgPl(sizeof(params), (char*)_interferometerCtrlMsgdIdentity.c_str(), INTF_COMMAND, INTF_GET_BURST, 0, (void*)(&params));
	if (IS_ERROR(seqNum)) {
		_logger->log(Logger::LOG_LEV_ERROR, "...request failed!");
		return "";
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "....request succesfully sent! Waiting reply...");
		int stat;
		MsgBuf* replyBuf = thWaitMsg(INTF_COMMAND, (char*)_interferometerCtrlMsgdIdentity.c_str(), 0, TIMEOUT_4D_SEC, &stat);
		// Check if reply succesfully got
		if (!replyBuf) {
			_logger->log(Logger::LOG_LEV_ERROR, "Error in thWaitMsg: %d (%s)", stat, lao_strerror(stat));
	      	return "";
	    }
	    _logger->log(Logger::LOG_LEV_INFO, "Reply succesfully received");
	    MeasurementBurstParams* params = (MeasurementBurstParams*)(MSG_BODY(replyBuf));
	    string remotePath = params->remotePathOut;
	    _logger->log(Logger::LOG_LEV_INFO, "Result: %s", remotePath.c_str());

	    thRelease(replyBuf);
	    return remotePath;
	}
}

void InterferometerInterface::trigger() {

	_logger->log(Logger::LOG_LEV_INFO, "Triggering interferometer...");
	set_pin(TRIGGER_OUT); 	// Trigger is TTL HIGH
    nusleep(1000);			// TODO Check!!!
    clear_pin(TRIGGER_OUT);

	// TODO ???
    // Wait frame ready
    //while(pin_is_set(FRAME_READY_IN) == 0) {
    //	usleep(1000);
    //}
    _logger->log(Logger::LOG_LEV_INFO, "Frame is ready!");
}

string InterferometerInterface::postProcess(string remotePath, string measPrefix, int measNum) throw (AOException) {

	PostProcessParams params = ParamsFactory::CreatePostProcessParams(remotePath, measPrefix, measNum);

	// Ask to InterferometerCtrl to get a measurement from the interferometer
	_logger->log(Logger::LOG_LEV_INFO, "postProcess %d measurements with prefix %s in %s...", params.measNum, params.measPrefix, params.remotePath);
	int seqNum = thSendMsgPl(sizeof(params), (char*)_interferometerCtrlMsgdIdentity.c_str(), INTF_COMMAND, INTF_POST_PROC, 0, (void*)(&params));
	if (IS_ERROR(seqNum)) {
		_logger->log(Logger::LOG_LEV_ERROR, "...request failed!");
		return "";
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "....request succesfully sent! Waiting reply...");
		int stat;
		MsgBuf* replyBuf = thWaitMsg(INTF_COMMAND, (char*)_interferometerCtrlMsgdIdentity.c_str(), 0, TIMEOUT_4D_SEC, &stat);
		// Check if reply succesfully got
		if (!replyBuf) {
			_logger->log(Logger::LOG_LEV_ERROR, "Error in thWaitMsg: %d (%s)", stat, lao_strerror(stat));
	      	return "";
	    }
	    _logger->log(Logger::LOG_LEV_INFO, "Reply succesfully received");
	    PostProcessParams* params = (PostProcessParams*)(MSG_BODY(replyBuf));
	    string remotePath = params->remotePathOut;
	    _logger->log(Logger::LOG_LEV_INFO, "Result: %s", remotePath.c_str());

	    if(remotePath == "NONE") {
	    	throw AOException("Post-processing not done (type is NONE)", INTERFEROMETER_POSTPROC_NOT_DONE, __FILE__, __LINE__);
	    }

	    thRelease(replyBuf);
	    return remotePath;
	}
}


/**
 *
 */
string InterferometerInterface::capture(int n, string name) {

    // reuse MeasurementBurstParams
    MeasurementBurstParams params = ParamsFactory::CreateMeasurementBurstParams(n, name);

    _logger->log(Logger::LOG_LEV_INFO, "InterferometerInterface::capture(%d, %s) ...", params.measNum, params.burstName);

    // send request
    int seqNum = thSendMsgPl(sizeof(params), (char*)_interferometerCtrlMsgdIdentity.c_str(), INTF_COMMAND, INTF_CAPTURE, 0, (void*)(&params));

    if (IS_ERROR(seqNum)) {
	_logger->log(Logger::LOG_LEV_ERROR, "...request failed!");
	return "";
    }
    else {
	_logger->log(Logger::LOG_LEV_INFO, "....request succesfully sent! Waiting reply...");
	int stat;
	MsgBuf* replyBuf = thWaitMsg(INTF_COMMAND, (char*)_interferometerCtrlMsgdIdentity.c_str(), 0, TIMEOUT_4D_SEC, &stat);
	// Check if reply succesfully got
	if (!replyBuf) {
	    _logger->log(Logger::LOG_LEV_ERROR, "Error in thWaitMsg: %d (%s)", stat, lao_strerror(stat));
	    return "";
	}
	_logger->log(Logger::LOG_LEV_INFO, "Reply succesfully received");
    MeasurementBurstParams* params = (MeasurementBurstParams*)(MSG_BODY(replyBuf));
    string remotePath = params->remotePathOut;
    _logger->log(Logger::LOG_LEV_INFO, "Result: %s", remotePath.c_str());
    
    thRelease(replyBuf);
    return remotePath;
    }
}

/**
 *
 */
string InterferometerInterface::produce(string name) {

    // reuse CreateMeasurementParams
    MeasurementParams params = ParamsFactory::CreateMeasurementParams(name);

    _logger->log(Logger::LOG_LEV_INFO, "produce with argument %s ...", params.measPrefix);

    int seqNum = thSendMsgPl(sizeof(params), (char*)_interferometerCtrlMsgdIdentity.c_str(), INTF_COMMAND, INTF_PRODUCE, 0, (void*)(&params));
    
    if (IS_ERROR(seqNum)) {
	_logger->log(Logger::LOG_LEV_ERROR, "...request failed!");
	return "";
    }
    else {
	_logger->log(Logger::LOG_LEV_INFO, "....request succesfully sent! Waiting reply...");
	int stat;
	MsgBuf* replyBuf = thWaitMsg(INTF_COMMAND, (char*)_interferometerCtrlMsgdIdentity.c_str(), 0, TIMEOUT_4D_SEC, &stat);
	// Check if reply succesfully got
	if (!replyBuf) {
	    _logger->log(Logger::LOG_LEV_ERROR, "Error in thWaitMsg: %d (%s)", stat, lao_strerror(stat));
	    return "";
	}
	_logger->log(Logger::LOG_LEV_INFO, "Reply succesfully received");
	MeasurementParams* params = (MeasurementParams*)(MSG_BODY(replyBuf));
	string remoteFile = params->remoteFileOut;
	_logger->log(Logger::LOG_LEV_INFO, "Result: %s", remoteFile.c_str());
	
	thRelease(replyBuf);
	return remoteFile;
    }
}


