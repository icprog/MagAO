
#include "InterferometerCtrl.h"

#include "IntfLib/Commands.h"

#include "Factory4D.h"
#include "Measurement4D.h"
#include "MeasurementBurst4D.h"

extern "C" {
#include "base/timelib.h"
}



using namespace Arcetri;



InterferometerCtrl::InterferometerCtrl(int argc, char **argv) throw (AOException) : AOApp(argc, argv) {
	Create();
}


InterferometerCtrl::~InterferometerCtrl() {
	_logger->log(Logger::LOG_LEV_INFO, "Destroying interferometer controller for '%s'...", _measFactory->getInterferometerName().c_str());
	delete _measFactory;
	_logger->log(Logger::LOG_LEV_INFO, "Interferometer controller for '%s' destroyed", _measFactory->getInterferometerName().c_str());
}


void InterferometerCtrl::Create() throw (AOException) {

	 _measFactory = NULL;
	 // Interferometer dependant initialization: uses 4D PhaseCam4020
	// TODO It would be nice to use RTTI to instance the Factory4D using
	//      a name defined in the config file
	_measFactory = Factory4D::instance();

	_logger->log(Logger::LOG_LEV_INFO, "Creating Interferometer controller for '%s'...", _measFactory->getInterferometerName().c_str());

	_enableExtTrigger = (int)ConfigDictionary()["ENABLE_EXT_TRIGGER"] == 1;
	_postProcType = (string)ConfigDictionary()["POST_PROC_TYPE"];

	// Apply settings
	if(!enableTrigger(_enableExtTrigger) || !setPostProcessing(_postProcType)) {
		throw AOException("Error in InterferometerCtrl creation!", INTERFEROMETER_INIT_ERROR, __FILE__, __LINE__);
	}

	_logger->log(Logger::LOG_LEV_INFO, "Interferometer controller for '%s' created", _measFactory->getInterferometerName().c_str());
}


void InterferometerCtrl::SetupVars() {
    try {
    	// TODO
    }
    catch (AOVarException &e) {
    	_logger->log(Logger::LOG_LEV_ERROR, "Impossible to complete SetupVars");
    	throw;
    }
}


void InterferometerCtrl::InstallHandlers() {

	int stat;

	// Handler for all the commands: the command is psecified in the payload
	if((stat=thHandler(INTF_COMMAND, "*", 0, commandsHndlr, "commandsHndlr", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
        SetTimeToDie(true);
    }
}


void InterferometerCtrl::PostInit() {

	// TODO
}


int InterferometerCtrl::commandsHndlr(MsgBuf *msgb, void *argp, int hndlrQueueSize) {
	if(hndlrQueueSize > 10) {	// TODO remove 10
		Logger::get()->log(Logger::LOG_LEV_ERROR, "InterferometerCtrl::commandsHndlr -> Queue exceeding normal size...");
		thRelease(msgb);
		Logger::get()->log(Logger::LOG_LEV_FATAL, "InterferometerCtrl::commandsHndlr -> ...message ignored!");
		return THRD_QUEUE_OVERRUN_ERROR;
	}

	InterferometerCtrl* mySelf = (InterferometerCtrl*)argp;

	// Discover the actual command
	int cmdCode = HDR_PLOAD(msgb);
	_logger->log(Logger::LOG_LEV_INFO, "commandsHndlr: received command %x", cmdCode);

	bool result;

	AbstractMeasurement* meas = NULL;
	MeasurementParams measParams;
	string remoteFile;

	AbstractMeasurementBurst* burst = NULL;
	MeasurementBurstParams burstParams;
	string remotePath;

	PostProcessParams ppParams;
	string ppRemotePath;

	switch(cmdCode) {

		case INTF_ENABLE_TRIGGER:
			result = mySelf->enableTrigger(true);
			thReplyMsgPl(INTF_COMMAND, (int)result, 0, NULL, msgb);
			break;

		case INTF_DISABLE_TRIGGER:
			result = mySelf->enableTrigger(false);
			thReplyMsgPl(INTF_COMMAND, (int)result, 0, NULL, msgb);
			break;

		case INTF_SET_POST_PROC_NONE:
			result = mySelf->setPostProcessing("NONE");
			thReplyMsgPl(INTF_COMMAND, (int)result, 0, NULL, msgb);
			break;

		case INTF_SET_POST_PROC_HDF5:
			result = mySelf->setPostProcessing("HDF5");
			thReplyMsgPl(INTF_COMMAND, (int)result, 0, NULL, msgb);
			break;

		case INTF_GET_MEAS:
			memcpy(&measParams, MSG_BODY(msgb), sizeof(measParams));	// Copy for safety
			_logger->log(Logger::LOG_LEV_DEBUG, "Acquiring measurement with prefix %s...", measParams.measPrefix);
			try {
				meas = mySelf->getMeasurement(measParams.measPrefix);
				remoteFile = meas->getRemoteFilename();
				_logger->log(Logger::LOG_LEV_DEBUG, "Acquired measurement to %s", remoteFile.c_str());
			}
			catch (AOException& e) {
				_logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
				remoteFile = "";
			}
			ParamsFactory::SetMeasurementParamsRemoteFileOut(measParams, remoteFile);
			thReplyMsg(INTF_COMMAND, sizeof(measParams), (void*)(&measParams), msgb);
			delete meas;
			break;

		case INTF_GET_BURST:
			memcpy(&burstParams, MSG_BODY(msgb), sizeof(burstParams));	// Copy for safety
			_logger->log(Logger::LOG_LEV_DEBUG, "Acquiring measurement burst of %d measurements with name %s...", burstParams.measNum, burstParams.burstName);
			try {
				burst = mySelf->getMeasurementBurst(burstParams.measNum, burstParams.burstName);
				remotePath = burst->getRemotePath();
				_logger->log(Logger::LOG_LEV_DEBUG, "Acquired %d measurements to %s", burstParams.measNum, remotePath.c_str());
			}
			catch (AOException& e) {
				_logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
				remotePath = "";
			}
			ParamsFactory::SetMeasurementBurstParamsRemotePathOut(burstParams, remotePath);
			thReplyMsg(INTF_COMMAND, sizeof(burstParams), (void*)(&burstParams), msgb);
			delete burst;
			break;

		case INTF_POST_PROC:
			memcpy(&ppParams, MSG_BODY(msgb), sizeof(ppParams));	// Copy for safety
			_logger->log(Logger::LOG_LEV_DEBUG, "Post-processing %d measurements from %s with prefix %s...", ppParams.measNum, ppParams.remotePath, ppParams.measPrefix);
			try {
				ppRemotePath = mySelf->postProcess(ppParams.remotePath, ppParams.measPrefix, ppParams.measNum);
				if(ppRemotePath != "") {
					_logger->log(Logger::LOG_LEV_DEBUG, "Processed measurements stored in %s", ppRemotePath.c_str());
				}
				// This means post-process not done because is set to NONE
				else {
					_logger->log(Logger::LOG_LEV_DEBUG, "Setting NONE as result string...");
					ppRemotePath = "NONE";
				}
			}
			catch (AOException& e) {
				_logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
				ppRemotePath = "";
			}
			ParamsFactory::SetPostProcessParamsRemotePathOut(ppParams, ppRemotePath);
			thReplyMsg(INTF_COMMAND, sizeof(ppParams), (void*)(&ppParams), msgb);
			break;

		case INTF_CAPTURE:
			memcpy(&burstParams, MSG_BODY(msgb), sizeof(burstParams));	// Copy for safety
			_logger->log(Logger::LOG_LEV_DEBUG, "Capture of %d measurements %s...", burstParams.measNum, burstParams.burstName);
			try {
                remotePath = mySelf->capture(burstParams.measNum, burstParams.burstName);
				_logger->log(Logger::LOG_LEV_DEBUG, "Captured %d measurements to %s", burstParams.measNum, remotePath.c_str());
			}
			catch (AOException& e) {
				_logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
				remotePath = "";
			}
			ParamsFactory::SetMeasurementBurstParamsRemotePathOut(burstParams, remotePath);
			thReplyMsg(INTF_COMMAND, sizeof(burstParams), (void*)(&burstParams), msgb);
			break;

       case INTF_PRODUCE:
			memcpy(&measParams, MSG_BODY(msgb), sizeof(measParams));	// Copy for safety
			_logger->log(Logger::LOG_LEV_DEBUG, "Producing %s...", measParams.measPrefix);
			try {
				remoteFile = mySelf->produce(measParams.measPrefix);
				_logger->log(Logger::LOG_LEV_DEBUG, "Produced %s", remoteFile.c_str());
			}
			catch (AOException& e) {
				_logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
				remoteFile = "";
			}
			ParamsFactory::SetMeasurementParamsRemoteFileOut(measParams, remoteFile);
			thReplyMsg(INTF_COMMAND, sizeof(measParams), (void*)(&measParams), msgb);
			break;
           
		default:
            result = -1;
			_logger->log(Logger::LOG_LEV_ERROR, "Unknown command");
			thReplyMsgPl(INTF_COMMAND, (int)result, 0, NULL, msgb);
			break;

	}

	thRelease(msgb);
}


void InterferometerCtrl::Run() {
    while(!TimeToDie())
    {
		nusleep(1000000);
    }
}


bool InterferometerCtrl::enableTrigger(bool enable) {

	if(enable) {
		_logger->log(Logger::LOG_LEV_INFO, "Interferometer controller enabling trigger...");
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "Interferometer controller disabling trigger...");
	}
	return _measFactory->enableTrigger(enable);
}


bool InterferometerCtrl::setPostProcessing(string ppType) {
	_logger->log(Logger::LOG_LEV_INFO, "Interferometer controller setting post-processing to %s...", ppType.c_str());
	return _measFactory->setPostProcessing(ppType);
}


AbstractMeasurement* InterferometerCtrl::getMeasurement(string measName) {
	_logger->log(Logger::LOG_LEV_INFO, "Interferometer controller getting measurement...");
	return _measFactory->getMeasurement(measName);
}


AbstractMeasurementBurst* InterferometerCtrl::getMeasurementBurst(int measNum, string burstName) {
	_logger->log(Logger::LOG_LEV_INFO, "Interferometer controller getting measurement burst (%d meas)...", measNum);
	return _measFactory->getMeasurementBurst(measNum, burstName);
}


string InterferometerCtrl::postProcess(string remotePath, string measPrefix, int measNum) {
	_logger->log(Logger::LOG_LEV_INFO, "Interferometer controller performing post-processing...");
	return _measFactory->postProcess(remotePath, measPrefix, measNum);
}


//
string InterferometerCtrl::capture(int n, string name) {
    _logger->log(Logger::LOG_LEV_INFO, "Interferometer controller capturing %s (%d meas)...", name.c_str(), n);
    return _measFactory->capture(n, name);
}

//
string InterferometerCtrl::produce(string name) {
    _logger->log(Logger::LOG_LEV_INFO, "Interferometer controller producing %s...", name.c_str());
    return _measFactory->produce(name);
}

