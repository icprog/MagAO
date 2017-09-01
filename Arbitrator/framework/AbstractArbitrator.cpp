
#include "arblib/base/Command.h"
#include "arblib/base/Alerts.h"
#include "arblib/base/Commands.h"

#include "framework/AbstractArbitrator.h"
#include "framework/CommandImplFactory.h"
#include "framework/CommandHandler.h"
#include "framework/AbstractFsm.h"

#include "base/msgcodes.h"
#include "AOStates.h"

extern "C" {
#include "base/timelib.h"
}



using namespace Arcetri::Arbitrator;

AbstractArbitrator* AbstractArbitrator::_instance = NULL;
Serializator* AbstractArbitrator::_serializator = NULL;
bool AbstractArbitrator::_busy = true;
bool AbstractArbitrator::_stop = false;

map<string, AbstractSystem*> 	  AbstractArbitrator::_systems;
map<string, ArbitratorInterface*> AbstractArbitrator::_subArbInterfaces;
map<string, AlertNotifier*> 	  AbstractArbitrator::_clientInterfaces;


// ---------------------- PUBLIC METHODS ----------------------- //

AbstractArbitrator::AbstractArbitrator(int argc, char* argv[]) throw (ArbitratorException):AOApp(argc, argv) {
	create();
}

void AbstractArbitrator::create() {
	setupLoggers();

	_logger->log(Logger::LOG_LEV_DEBUG, "Creating arbitrator...");

	_serializator = new Serializator();

	// Store all configuration parameters //
	_commandHistorySize = (uint32)ConfigDictionary()["COMMAND_HISTORY_SIZE"];
	// ---------------------------------- //

	_logger->log(Logger::LOG_LEV_DEBUG, "Arbitrator %s succesfully created:", MyFullName().c_str());
	_logger->log(Logger::LOG_LEV_DEBUG,  " > Cmd history size: %d", _commandHistorySize);

	// Set the singleton
	_instance = this;


    // set busy and stop current operation flags to false
    setBusyFlag(false);
    setStopFlag(false);
}


AbstractArbitrator::~AbstractArbitrator() {
	_logger->log(Logger::LOG_LEV_DEBUG, "Destroying AbstractArbitrator...");

    // set busy flag
    setBusyFlag();
    setStopFlag();

	delete _cmdHandler;
	delete _cmdImplFactory;
	delete _serializator;

	// Remove sub-arbitrators' interfaces
	map<string, ArbitratorInterface*>::iterator iter1;
	for(iter1 = _subArbInterfaces.begin(); iter1 != _subArbInterfaces.end(); iter1++) {
		delete iter1->second;
	}

	// Remove upper-level client's interfaces
	map<string, AlertNotifier*>::iterator iter2;
	for(iter2 = _clientInterfaces.begin(); iter2 != _clientInterfaces.end(); iter2++) {
		delete iter2->second;
	}

	// Remove systems
	map<string, AbstractSystem*>::iterator iter3;
	for(iter3 = _systems.begin(); iter3 != _systems.end(); iter3++) {
			delete iter3->second;
		}

	// [todo] ...
}


int AbstractArbitrator::handleRequest(MsgBuf* requestMsgBuf, void *argp, int hndlrQueueSize) {

    int ret=NO_ERROR;
    Request *req = NULL;

    try {
        // Check the queue of this handler: if it's becoming full
        // react in the appropriate way, i.e. ignoring the current buffer
        // (remember thRelease()!!!)
        if(hndlrQueueSize > HNDLR_QUEUE_REQUESTS_LIMIT) {
            thRelease(requestMsgBuf);
            return THRD_QUEUE_OVERRUN_ERROR;
        }

        static Logger* logger = Logger::get("REQUESTHANDLER");


        AbstractArbitrator* myself = (AbstractArbitrator*)argp;
        if(!myself->IsReady()) {
            logger->log(Logger::LOG_LEV_ERROR, "Arbitrator (AoApp) still not ready (Run not completed)");
            thRelease(requestMsgBuf);
            return NOT_INIT_ERROR;
        }

        // --- Retrieve the request
        logger->log(Logger::LOG_LEV_DEBUG, "REQUEST received from %s", requestMsgBuf->Msg->HD.From);
        req = (Request*)(_serializator->deserialize(requestMsgBuf));
        if (req == NULL) {
            thRelease(requestMsgBuf);
            return ARB_MSG_DESERIALIZATION_ERROR;
        }
        req->log();

        // set busy flag
        setBusyFlag();

        // Check the type of request
        switch(req->getCode()) {
		case BasicOpCodes::REQUEST_FEEDBACK: {
			RequestFeedback* feedbackReq = (RequestFeedback*)req;
			string requesterName = HDR_FROM(requestMsgBuf);
			myself->addAlertNotifier(requesterName);
			feedbackReq->setDone();
			break;
		}
		case BasicOpCodes::REQUEST_STATUS: {
			RequestStatus* statusReq = (RequestStatus*)req;
			statusReq->setArbitratorStatus(myself->fsmState());
			Command* lastCmd = myself->lastCommand();
			if(lastCmd != NULL) {
				statusReq->setLastCommandDescription(lastCmd->getDescription());
				statusReq->setLastCommandResult(lastCmd->getStatusAsString());
			}
			statusReq->setDone();
			break;
		}
		default:
			// Calls virtual method !!!
			myself->serviceRequest(req,requestMsgBuf);

        }
        setBusyFlag(false);

        // --- Send back the request as reply (the command status is now changed)
        ret = myself->sendReply(req, requestMsgBuf, logger); // This also release msgBuf with thReplyMsg

    } catch (AOException &e) {
        setBusyFlag(false);
        Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "handleRequest(): Exception: %s", e.what().c_str());
        return NO_ERROR;
    }
  
	if (req) delete req;
	return ret;
}

int AbstractArbitrator::handleCommand(MsgBuf* commandMsgBuf, void *argp, int hndlrQueueSize) {

    try {
        // Check the queue of this handler: if it's becoming full
        // react in the appropriate way, i.e. ignoring the current buffer
        // (remember thRelease()!!!)
        if(hndlrQueueSize > HNDLR_QUEUE_COMMANDS_LIMIT) {
            thRelease(commandMsgBuf);
            return THRD_QUEUE_OVERRUN_ERROR;
        }

        static Logger* logger = Logger::get("COMMANDHANDLER");

        AbstractArbitrator* myself = (AbstractArbitrator*)argp;
        if(!myself->IsReady()) {
            logger->log(Logger::LOG_LEV_ERROR, "Arbitrator (AoApp) still not ready (Run not completed)");
            thRelease(commandMsgBuf);
            return NOT_INIT_ERROR;
        }

        // --- Retrieve the command
        logger->log(Logger::LOG_LEV_DEBUG, "COMMAND received from %s", commandMsgBuf->Msg->HD.From);
        Command* cmd = (Command*)(_serializator->deserialize(commandMsgBuf));
        if (cmd == NULL) {
            thRelease(commandMsgBuf);
            return ARB_MSG_DESERIALIZATION_ERROR;
        }
        cmd->log();

        // set busy flag and clear stop command flag
        setBusyFlag();
        setStopFlag(false);

        // --- Process the command (this will change the command status)
        bool inHistory = myself->processCommand(cmd);
        setBusyFlag(false);

        // --- Send back the command as reply (the command status is now changed)
        int ret = myself->sendReply(cmd, commandMsgBuf, logger); // This also release msgBuf with thReplyMsg
        if(!inHistory) {
            delete cmd;
        }
        return ret;

    } catch (AOException &e) {
        setBusyFlag(false);
        Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "handleCommand(): Exception: %s", e.what().c_str());
        return NO_ERROR;
    }

}



int AbstractArbitrator::handleAsyncCommand(MsgBuf* commandMsgBuf, void *argp, int hndlrQueueSize) {

	// Check the queue of this handler: if it's becoming full
	// react in the appropriate way, i.e. ignoring the current buffer
	// (remember thRelease()!!!)
	if(hndlrQueueSize > HNDLR_QUEUE_COMMANDS_LIMIT) {
		thRelease(commandMsgBuf);
		return THRD_QUEUE_OVERRUN_ERROR;
	}

	static Logger* logger = Logger::get("ASYNCCOMMANDHANDLER");

	AbstractArbitrator* myself = (AbstractArbitrator*)argp;
	if(!myself->IsReady()) {
		logger->log(Logger::LOG_LEV_ERROR, "Arbitrator (AoApp) still not ready (Run not completed)");
		thRelease(commandMsgBuf);
		return NOT_INIT_ERROR;
	}

	// --- Retrieve the command
	logger->log(Logger::LOG_LEV_INFO, "COMMAND received from %s", commandMsgBuf->Msg->HD.From);
	Command* cmd = (Command*)(_serializator->deserialize(commandMsgBuf));
	if (cmd == NULL) {
        thRelease(commandMsgBuf);
        return ARB_MSG_DESERIALIZATION_ERROR;
    }
	cmd->log();

    int ret = 0;

    try {
        // --- Process the command (this will change the command status)
        setBusyFlag();
        setStopFlag(false);
        bool inHistory = myself->processAsyncCommand(cmd);
        setBusyFlag(false);
        // --- Send back the command as reply (the command status is now changed)
        ret = myself->sendReply(cmd, commandMsgBuf, logger); // This also release msgBuf with thReplyMsg
        if(!inHistory) {
            delete cmd;
        }
    }
    catch (...) {
        setBusyFlag(false);
        throw;
    }

	return ret;
}


int AbstractArbitrator::handleCancelCommand(MsgBuf* commandMsgBuf, void *argp, int hndlrQueueSize) {
	// Check the queue of this handler: if it's becoming full
	// react in the appropriate way, i.e. ignoring the current buffer
	// (remember thRelease()!!!)
	if(hndlrQueueSize > HNDLR_QUEUE_COMMANDS_LIMIT) {
		thRelease(commandMsgBuf);
		return THRD_QUEUE_OVERRUN_ERROR;
	}

	static Logger* logger = Logger::get("COMMANDHANLDER");

	AbstractArbitrator* myself = (AbstractArbitrator*)argp;
	if(!myself->IsReady()) {
		logger->log(Logger::LOG_LEV_ERROR, "Arbitrator (AoApp) still not ready (Run not completed)");
		thRelease(commandMsgBuf);
		return NOT_INIT_ERROR;
	}

	// --- Retrieve the command
	logger->log(Logger::LOG_LEV_INFO, "CANCEL command received from %s", commandMsgBuf->Msg->HD.From);


    try {
        // --- Process the cancel command
        setBusyFlag();
        myself->cancelCommand();
        setBusyFlag(false);

        // Don't send a reply: the result of the "cancel" is only visible on the reply
        // of the canceled command!
        thRelease(commandMsgBuf);
    }
    catch (...) {
        setBusyFlag(false);
        throw;
    }

	return NO_ERROR;
}


int AbstractArbitrator::handleAlert(MsgBuf* alertMsgBuf, void *argp, int hndlrQueueSize) {

	// Check the queue of this handler: if it's becoming full
	// react in the appropriate way, i.e. ignoring the current buffer
	// (remember thRelease()!!!)
	if(hndlrQueueSize > HNDLR_QUEUE_ALERTS_LIMIT) {
		thRelease(alertMsgBuf);
		return THRD_QUEUE_OVERRUN_ERROR;
	}

	static Logger* logger = Logger::get("ALERTHANDLER");

	AbstractArbitrator* myself = (AbstractArbitrator*)argp;
	if(!myself->IsReady()) {
		logger->log(Logger::LOG_LEV_ERROR, "Arbitrator (AoApp) still not ready (Run not completed)");
		thRelease(alertMsgBuf);
		return NOT_INIT_ERROR;
	}

    // This log is at debug level because the processAlert() is pure virtual and must be implemented
	logger->log(Logger::LOG_LEV_DEBUG, "Alert received from %s", alertMsgBuf->Msg->HD.From);

	// --- Retrieve the alert
	Alert* alert = (Alert*)(_serializator->deserialize(alertMsgBuf));
	thRelease(alertMsgBuf);
	if (alert == NULL) return ARB_MSG_DESERIALIZATION_ERROR;

    try {
        // Process the alert
        setBusyFlag();
        myself->processAlert(alert);
        setBusyFlag(false);
    
        // Clean
        delete alert;
    }
    catch (...) {
        setBusyFlag(false);
        throw;
    }

	return NO_ERROR;
}

/*
 * Notify the arbitrator for an alert.
 *
 * Used by arbitrator clients (framework's AbstractSystems implementations)
 * to notify internally detected problems.
 */
void AbstractArbitrator::notifyAlert(Alert* alert) {
    try {
        setBusyFlag();
        processAlert(alert);
        setBusyFlag(false);
    }
    catch (...) {
        setBusyFlag(false);
        throw;
    }
}

// ---------------------- PROTECTED METHODS (INHERITED FROM AOAPP) ----------------------- //

void AbstractArbitrator::SetupVars(){

	setCurState(STATE_CONFIGURING);

	_logger->log(Logger::LOG_LEV_DEBUG, "Setting up RTDB vars...");

   _fsmStateVar = new RTDBvar( MyFullName(), "FSM_STATE", NO_DIR, CHAR_VARIABLE, 50);
   _fsmStateVar->Set( fsmState());

   // Initialize clStat array and ask for notifications
   try {
      string processes = ConfigDictionary()["processList"];
      char *s = (char*)processes.c_str();
      char *token = strtok(s, ",");
      while (token != NULL) {
         string varname = string(token) + "." + Side() + ".CLSTAT";
         Notify(varname, clStatHandler); 

         // First value
         int stat;
         MsgBuf *msgb = thGetVar( varname.c_str(), 0, &stat);
         if (msgb) {
            clStatHandler( this, thValue(msgb));
            thRelease(msgb);
         }
         token = strtok(NULL, ",");
      }
   } catch (AOException &e) {
      _logger->log(Logger::LOG_LEV_ERROR, "Error installing clStat notify: %s", e.what().c_str());
      _logger->log(Logger::LOG_LEV_ERROR, "Process state reporting will not work.");
   }

   // Setup variable mirroring
   try {
      Config_File *mirror = ConfigDictionary().extract("MIRROR_VAR");
      Config_File::iterator iter;
      _toMirror.clear();
      for (iter = mirror->begin(); iter != mirror->end(); iter++) {
          size_t pos;
          string v = (std::string) ((*iter).second);
          v = ReplaceSide(v);
          if ((pos = v.find(" ")) != std::string::npos)
              _toMirror[ v.substr(0, pos)] = v.substr(pos+1);
      }

      map<string, string>::iterator iter2;
      for (iter2 = _toMirror.begin(); iter2 != _toMirror.end(); iter2++) {
          _logger->log( Logger::LOG_LEV_INFO, "Setting variable mirror: from %s to %s", (*iter2).first.c_str(), (*iter2).second.c_str());
           RTDBvar var;
           //int errCode;
           string varname = (*iter2).first.c_str();
           Notify(varname, mirrorHandler, 1);

      }
      delete mirror;
   } catch (AOException &e) {}


	_logger->log(Logger::LOG_LEV_DEBUG, "RTDB vars setup done.");
}

string AbstractArbitrator::ReplaceSide( string s) {
    size_t pos;
    while ((pos = s.find("<side>")) != std::string::npos)
        s.replace(pos, 6, Side());
    return s;
}

void AbstractArbitrator::InstallHandlers() {

	_logger->log(Logger::LOG_LEV_DEBUG, "Installing generic arbitrator handlers...");
	int stat = NO_ERROR;

	if(!IS_ERROR(stat = thHandler(ARB_REQ, "*", 0, handleRequest, "arbRequestHandler", this))) {
		_logger->log(Logger::LOG_LEV_DEBUG, "Handler for Requests succesfully installed!");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Error (code %d) while installing Feedback Request handler.", stat);
		throw ArbitratorException("Impossible to install Requests handler", ARB_INIT_ERROR);
	}

	if(!IS_ERROR(stat = thHandler(ARB_CMD, "*", 0, handleCommand, "arbCommandsHandler", this))) {
		_logger->log(Logger::LOG_LEV_DEBUG, "Handler for Commands succesfully installed!");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Error (code %d) while installing Commands handler.", stat);
		throw ArbitratorException("Impossible to install the Commands handler", ARB_INIT_ERROR);
	}

	if(!IS_ERROR(stat = thHandler(ARB_ASYNC_CMD, "*", 0, handleAsyncCommand, "arbAsyncCommandsHandler", this))) {
		_logger->log(Logger::LOG_LEV_DEBUG, "Handler for Async Commands succesfully installed!");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Error (code %d) while installing Async Commands handler.", stat);
		throw ArbitratorException("Impossible to install the Async Commands handler", ARB_INIT_ERROR);
	}

	if(!IS_ERROR(stat = thHandler(ARB_CANC_CMD, "*", 0, handleCancelCommand, "arbTerminateCommandHandler", this))) {
		_logger->log(Logger::LOG_LEV_DEBUG, "Handler for Cancel Commands termination succesfully installed!");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Error (code %d) while installing Commands termination handler.", stat);
		throw ArbitratorException("Impossible to install the Commands termination handler", ARB_INIT_ERROR);
	}

	if(!IS_ERROR(stat = thHandler(ARB_ALERT, "*", 0, handleAlert, "arbAlertsHandler", this))) {
		_logger->log(Logger::LOG_LEV_DEBUG, "Handler for Alerts succesfully installed!");
	}
	else
	{
		_logger->log(Logger::LOG_LEV_ERROR, "Error (code %d) while installing Commands handler.", stat);
		throw ArbitratorException("Impossible to install the Alerts handler", ARB_INIT_ERROR);
	}
	_logger->log(Logger::LOG_LEV_DEBUG, "Message handlers installation done.");
}

int AbstractArbitrator::mirrorHandler( void *pt, Variable *var) {

   AbstractArbitrator *arbitrator = (AbstractArbitrator *)pt;

   map<string, string>::iterator iter;

   for (iter = arbitrator->_toMirror.begin(); iter != arbitrator->_toMirror.end(); iter++)
      if (strcmp( (*iter).first.c_str(), var->H.Name) == 0)
         thWriteVar( (*iter).second.c_str(), var->H.Type, var->H.NItems, (void*) &(var->Value.Lv[0]), 1000);

   return NO_ERROR;
}


void AbstractArbitrator::PostInit() {

	_logger->log(Logger::LOG_LEV_DEBUG, "PostInit checks and initializations...");

	int thMsgTTL_sec = ConfigDictionary()["thLibTimeout_s"];
	thDefTTL(thMsgTTL_sec);

	// Check if command handler installed
	if(_cmdHandler == NULL) {
		_logger->log(Logger::LOG_LEV_ERROR, "CommandHandler not initialized!");
		throw ArbitratorException("Error while creating Arbitrator");
	}

	// Check if command implementation factory installed
	if(_cmdImplFactory == NULL) {
		_logger->log(Logger::LOG_LEV_ERROR, "CommandImplFactory not initialized!");
		throw ArbitratorException("Error while creating Arbitrator");
	}

	if(!TimeToDie()) {
		_logger->log(Logger::LOG_LEV_DEBUG, "PostInit checks and initializations done.");
	}
	else  {
		_logger->log(Logger::LOG_LEV_ERROR, "PostInit checks and initializations failed.");
	}
}

ArbitratorInterface* AbstractArbitrator::addArbitratorInterface(string targetArbMsgDIdnty) {
	_logger->log(Logger::LOG_LEV_DEBUG, "Adding ArbitratorInterface to %s", targetArbMsgDIdnty.c_str());
	ArbitratorInterface* arbIntf = new ArbitratorInterface(targetArbMsgDIdnty, Logger::get()->getLevel());
	_subArbInterfaces.insert(make_pair(targetArbMsgDIdnty, arbIntf));
	return arbIntf;
}

inline Command* AbstractArbitrator::lastCommand() {
	return _cmdHandler->lastCommand();
}

ArbitratorInterface* AbstractArbitrator::getArbitratorInterface(string targetArbMsgDIdnty) {
   return _subArbInterfaces[targetArbMsgDIdnty];
}




inline string AbstractArbitrator::fsmState() {
	//TODO Unsafe cast!!! In theory the _cmdHandler could point
	// a CommandHandler object...
	return ((AbstractFsm*)_cmdHandler)->currentStateAsString();
}




void AbstractArbitrator::addAlertNotifier(string targetClientMsgDIdnty) {
	map<string, AlertNotifier*>::iterator iter = _clientInterfaces.find(targetClientMsgDIdnty);
	if(iter == _clientInterfaces.end()) {
		_logger->log(Logger::LOG_LEV_INFO, "Adding AlertNotifier for %s", targetClientMsgDIdnty.c_str());
		AlertNotifier* alertNotif = new AlertNotifier(targetClientMsgDIdnty, Logger::get()->getLevel());
		_clientInterfaces.insert(make_pair(targetClientMsgDIdnty, alertNotif));
	}
	else {
		_logger->log(Logger::LOG_LEV_WARNING, "AlertNotifier for %s is already installed!", targetClientMsgDIdnty.c_str());
	}
}


void AbstractArbitrator::notifyClients(Alert* alert) {

	map<string, AlertNotifier*>::iterator clients4Notif;
	for(clients4Notif = _clientInterfaces.begin(); clients4Notif != _clientInterfaces.end(); clients4Notif++) {
		_logger->log(Logger::LOG_LEV_DEBUG, "Notifying %s...", clients4Notif->first.c_str());
		 clients4Notif->second->notifyAlert(alert);
	}
}

void AbstractArbitrator::waitSubArbitrator(string targetArbMsgDIdnty, int delay_ms) {
	ArbitratorInterface* arbIntf = _subArbInterfaces[targetArbMsgDIdnty];
	Ping pingCmd(delay_ms);	// Delay is used as timeout_ms
	Command* pingReply = NULL;
	while (!TimeToDie() && (pingReply == NULL || !pingReply->isSuccess()))  {
		_logger->log(Logger::LOG_LEV_DEBUG, "Waiting %d ms for %s ready...", delay_ms, targetArbMsgDIdnty.c_str());
		try {
			delete pingReply;
			pingReply = arbIntf->requestCommand(&pingCmd);
		}
		catch(...) {}
	}
	_logger->log(Logger::LOG_LEV_DEBUG, "%s is ready!", targetArbMsgDIdnty.c_str());
	delete pingReply;
}

void AbstractArbitrator::Run() {

	_logger->log(Logger::LOG_LEV_DEBUG, "Arbitrator succesfully initialized. Starting...");
	setCurState(STATE_OPERATING);

	// Wait sub-arbitrators (if installed)
/* Removed: sub-arbitrator crash terribly!   
	map<string, ArbitratorInterface*>::iterator subArbIter;
	for(subArbIter = _subArbInterfaces.begin(); subArbIter != _subArbInterfaces.end(); subArbIter++) {
		waitSubArbitrator(subArbIter->first, 1000);
	}
*/
	//--- Main loop ---//
	while(!TimeToDie()) {
      try {
		   msleep(1000);
         Periodic();
      } catch (AOException &e) {
         // Loop for any timeout exception, but exit for other things
         Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "AbstractArbitrator::Run() Caught exception: %s", e.what().c_str());
         if (e._errcode != TIMEOUT_ERROR)
            throw;
      }
   }


}


// ---------------------- PROTECTED METHODS ----------------------- //


Logger* AbstractArbitrator::setLoggerLevel(string loggerName) {

	Logger * logger = NULL;

	int logLevel = Logger::LOG_LEV_ERROR;
	string loggerConfigKey = loggerName + "LogLev";

	try {
		// Search level in config file
		logLevel = Logger::stringToLevel(ConfigDictionary()[loggerConfigKey]);
	}
	catch (Config_File_Exception& e) {
		// Use the default log level
		Logger::get()->log(Logger::LOG_LEV_WARNING, "Logger %s not configured in config file (key %s). Using default level (%s).", loggerName.c_str(), loggerConfigKey.c_str(), Logger::levelDescription(Logger::get()->getLevel()));
		logLevel = Logger::get()->getLevel();
	}

	// If the logger exists, its log level is set to the new value.
	logger = Logger::get(loggerName);
	logger->setLevel(logLevel);
	logger->printStatus();
	return logger;
}


// ---------------------- PRIVATE METHODS ----------------------- //

void AbstractArbitrator::setupLoggers() {

	// Setup the other loggers
	setLoggerLevel("SERIALIZATION");
	setLoggerLevel("COMMANDHANDLER");
	setLoggerLevel("REQUESTHANDLER");
	setLoggerLevel("ALERTHANDLER");
}

int AbstractArbitrator::sendReply(SerializableMessage* msg, MsgBuf* buf, Logger* logger) {

	// --- Send back the command as reply (the command status is now changed)
	msg->setMsgCode(ARB_REPLY);	// This avoid loops of commands/reply with the same msg code!
	string replyTo = HDR_FROM(buf);
	msg->setSender(MyFullName());
	int bufferLen;
	char* buffer = _serializator->serialize(msg, bufferLen);
	if (buffer == NULL) return ARB_MSG_SERIALIZATION_ERROR;

	// *** Internal deserialization for test *** //
	if(logger->getLevel() == Logger::LOG_LEV_VTRACE) {
		logger->log(Logger::LOG_LEV_VTRACE, "*** TEST: internal deserialization ***");
		SerializableMessage* msgTmp = _serializator->deserialize(buffer, bufferLen);
		logger->log(Logger::LOG_LEV_VTRACE, "**************************************");
		delete msgTmp;
	}

	thReplyMsg(msg->getMsgCode(), bufferLen, (void*)buffer, buf);
	delete[] buffer;
	logger->log(Logger::LOG_LEV_DEBUG, "Sent message reply to %s", replyTo.c_str());
	return NO_ERROR;
}

bool AbstractArbitrator::processCommand(Command* cmd) {

	static Logger* logger = Logger::get("COMMANDHANDLER");

	logger->log(Logger::LOG_LEV_DEBUG, "Processing command:");
	cmd->log();

	// Implement the command and execute it
	CommandImpl* cmdImpl = NULL;
	cmdImpl = _cmdImplFactory->implementCommand(cmd, _systems);

	bool inHistory = false;

	if(cmdImpl) {
		inHistory = _cmdHandler->doExecute(cmdImpl);

		// If the command has been succesfully executed, it is stored in the
		// command history, and must not be deleted
		if(!inHistory) {
			delete cmdImpl;
		}

		_logger->log(Logger::LOG_LEV_DEBUG, "Command processed!");
      _fsmStateVar->Set( fsmState());
	}
	return inHistory;
}

bool AbstractArbitrator::processAsyncCommand(Command* cmd) {

	static Logger* logger = Logger::get("ASYNCCOMMANDHANDLER");

	logger->log(Logger::LOG_LEV_DEBUG, "Processing async command:");
	cmd->log();

	// Implement the command and execute it
	CommandImpl* cmdImpl = NULL;
	cmdImpl = _cmdImplFactory->implementCommand(cmd, _systems);

   cmdImpl->log();
   cmdImpl->validate();
   if(cmdImpl->getStatus() == Command::CMD_VALIDATION_SUCCESS) {
      _logger->log(Logger::LOG_LEV_DEBUG, "Command succesfully validated");
      cmdImpl->execute();
   }
   else
      _logger->log(Logger::LOG_LEV_WARNING, "Command validation failed:");

   return true;
}

void AbstractArbitrator::cancelCommand() {

	static Logger* logger = Logger::get("COMMANDHANLDER");

	logger->log(Logger::LOG_LEV_DEBUG, "Canceling current command...");
	_cmdHandler->doCancel();
	logger->log(Logger::LOG_LEV_DEBUG, "Command canceled!");
}

int AbstractArbitrator::clStatHandler( void *pt, Variable *var) {

   AbstractArbitrator *me = (AbstractArbitrator *)pt;
   string varname = var->H.Name;
   int value = var->Value.Lv[0];

   me->_clStat[varname] = value;
   return NO_ERROR;
}

bool AbstractArbitrator::clStatOK() {

   bool res = true;

   // Check that all processes report Ready state
   map<string, int>::iterator iter;
   for (iter = _clStat.begin(); iter != _clStat.end(); iter++) 
      res &= iter->second >= 1;

   return res;
}

map<string, AbstractSystem*> &AbstractArbitrator::getSystems() {
    return _systems;
}
