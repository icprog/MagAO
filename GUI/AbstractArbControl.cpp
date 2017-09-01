#include "AbstractArbControl.h"

#include "arblib/base/Commands.h"
#include "AOGlobals.h"

#include <cstdlib>
#include <iostream>
#include <dirent.h>


extern "C" {
#include "base/timelib.h"
}



using namespace Arcetri;
using namespace Arcetri::Arbitrator;


Serializator AbstractArbControl::_serializator;

string AbstractArbControl::_arbStatus;
string AbstractArbControl::_lastCmdDescr;
string AbstractArbControl::_lastCmdResult;


resultColor colorTable[] = {
   {"NOT PROCESSED", qRgb( 100, 0, 0)},
   {"VALIDATION FAILED", qRgb( 100, 0, 0)},
   {"EXECUTING", qRgb( 100, 100, 100)},
   {"EXECUTION SUCCESS", qRgb( 0, 100, 0)},
   {"EXECUTION FAILED", qRgb( 100, 0, 0)},
   { "\0", NULL }
};

#include <qmessagebox.h>


AbstractArbControl::AbstractArbControl(string name, string configFile, KApplication &kApp): AOAppGui(name, configFile, kApp)  {
	init();	// This won't bind dinamically to subclass method!
}


AbstractArbControl::AbstractArbControl(int argc, char* argv[], KApplication &kApp): AOAppGui(argc, argv, kApp) {
	init();	// This won't bind dinamically to subclass method!
}


AbstractArbControl::~AbstractArbControl() {

   if (_tailProc)
      _tailProc->kill();

   if (_tailProc2)
      _tailProc2->kill();


	delete _arbIntf;
	_logger->log(Logger::LOG_LEV_INFO, "GUI destroyed!");
}


void AbstractArbControl::init() {

	//thDebug(2);

	// Retrieve info about the server arbitrator
   _arbName = (string)ConfigDictionary()["ArbName"];
   _arbMsgdId = _arbName+"."+Side();
   _arbConfig = Config_File( ConfigDictionary().getDir()+"/"+_arbName+".conf");

	_arbConnected = false;
	_arbIntf = NULL;

   qobject=NULL;
   _tailProc = NULL;
   _tailProc2 = NULL;
   _exitNow = false;

   _waitingRedisplay = false;

	_logger->log(Logger::LOG_LEV_INFO, "GUI succesfully created!");
}


void AbstractArbControl::connectArb() {

   if (!_arbIntf)
      return;

	_logger->log(Logger::LOG_LEV_INFO, "");
	_logger->log(Logger::LOG_LEV_INFO, "--------------------------------------------------");
	_logger->log(Logger::LOG_LEV_INFO, ">> Initializing connection with %s...", _arbName.c_str());

	// --- Register for feedback ---
	RequestFeedback feedbackReq(500);
	Command* feedbackReqReply;
	_logger->log(Logger::LOG_LEV_INFO, "Trying to register for alerts notification from %s...", _arbName.c_str());
	try {
		// --- Request for feedback ---
		feedbackReqReply = _arbIntf->requestCommand(&feedbackReq);
		if(feedbackReqReply->isSuccess()) {
			_logger->log(Logger::LOG_LEV_INFO, "Alerts notifications succesfully requested!");
			_arbConnected = true;
			postSetMessage(_arbName + " is online");
		}
		else {
			_logger->log(Logger::LOG_LEV_WARNING, "Alerts notifications request failed");
			_arbConnected = false;
			postSetStatus("offline", "", "");
			postSetMessage(_arbName + " appears offline");
		}
		delete feedbackReqReply;

		// --- Do some other request needed for initialization ---
		// ...

		if(_arbConnected) {
			postEnableGui(true);
		}
	}
	catch(...) {
		_logger->log(Logger::LOG_LEV_ERROR, "Connection to %s failed", _arbName.c_str());
		postSetStatus("offline", "", "");
		postSetMessage(_arbName + " appears offline");
		throw;
	}
}


//////////////////////////////////// AOAPP METHODS ///////////////////////////////////////

// Called by AOApp::Exec()
void AbstractArbControl::InstallHandlers() {
	int stat;;

	AOAppGui::InstallHandlers();

	_logger->log(Logger::LOG_LEV_INFO, "Installing custom handlers ...");
	if((stat=thHandler(ARB_ALERT, (char *)"*", 0, alertHandler, (char *)"alert", this))<0) {
		_logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
		SetTimeToDie(true);
	}
}


// Called by AOApp::Exec()
void AbstractArbControl::SetupVars() {
	_logger->log(Logger::LOG_LEV_INFO, "Setting up custom rtdb variables ...");

    SetupVarsChild();
}


// Called by AOApp::Exec()
void AbstractArbControl::PostInit() {
	_arbIntf = new ArbitratorInterface(_arbMsgdId, Logger::stringToLevel(ConfigDictionary()["ArbInterfLogLevel"]));

	// Setup the GUI
	postEnableGui(false);

	// thDebug(2);

	_logger->log(Logger::LOG_LEV_INFO, "Post init done!");
}



int AbstractArbControl::alertHandler(MsgBuf *alertMsgBuf, void *argp, int hndlrQueueSize) {
	// Check the queue of this handler: if it's becoming full
	// react in the appropriate way, i.e. ignoring the current buffer
	// (remember thRelease()!!!)
	if(hndlrQueueSize > 20) {
		thRelease(alertMsgBuf);
		return THRD_QUEUE_OVERRUN_ERROR;
	}

	AbstractArbControl* myself = (AbstractArbControl*)argp;

	static Logger* logger = Logger::get();
	logger->log(Logger::LOG_LEV_INFO, "-------------------------------------------------------------"); // Just to read better the log

	// --- Retrieve the alert
	logger->log(Logger::LOG_LEV_INFO, "Alert received from %s", alertMsgBuf->Msg->HD.From);
	Alert* alert = (Alert*)(_serializator.deserialize(alertMsgBuf));
	thRelease(alertMsgBuf);
	if (alert == NULL) return ARB_MSG_DESERIALIZATION_ERROR;

	// Process the alert
	myself->postLogAlert(alert); // The QT will delete the object!

	return NO_ERROR;
}




//////////////////////////////////// EVENT METODS ///////////////////////////////////////


void AbstractArbControl::customEvent(QCustomEvent *e) {

	switch(e->type()) {

		case SETSTATUS_EVENT_ID: {
			SetStatusEvent* setStatusEvent = (SetStatusEvent*)e;
			setArbitratorStatus(setStatusEvent->getArbStatus(),		// PURE VIRTUAL
								setStatusEvent->getLastCmdDescr(),
								setStatusEvent->getLastCmdResult()
								);
			break;
		}

		case LOGALERT_EVENT_ID: {
			LogAlertEvent* logAlertEvent = (LogAlertEvent*)e;
			Alert* alert = logAlertEvent->getAlert();

			string alertShortDescr= alert->getShortDescription();
			string alertMessage = alert->getTextMessage();
			string fullAlert = alertShortDescr + ": " + alertMessage;
			logAlert(fullAlert);	// PURE VIRTUAL
			break;
		}

		// EnableGuiEvent
		case ENABLEGUI_EVENT_ID: {
			EnableGuiEvent* enableGuiEvent = (EnableGuiEvent*)e;
			bool enable = enableGuiEvent->getEnable();

			enableGui(enable);				// PURE VIRTUAL
			break;
		}

		case SETMESSAGE_EVENT_ID: {
			SetMessageEvent* setMessageEvent = (SetMessageEvent*)e;
			setMessage(setMessageEvent->getText());	// PURE VIRTUAL
			break;
		}

      case ALERT_EVENT_ID: {
         QMessageBox::warning( NULL, "Warning", ((AlertEvent*)e)->getText());
         break;
      }
		case SETASYNCHMESSAGE_EVENT_ID: {
			SetAsynchMessageEvent* setAsyncMessageEvent = (SetAsynchMessageEvent*)e;
			setAsynchMessage(setAsyncMessageEvent->getText());	// PURE VIRTUAL
			break;
		}

      case REDISPLAY_EVENT_ID: {
         redisplay();
         _waitingRedisplay = false;
      }

      default:
      break;
	}
}

void AbstractArbControl::postSetStatus(string arbStatus, string lastCmdDescr, string lastCmdResult) {
	SetStatusEvent* use = new SetStatusEvent(arbStatus, lastCmdDescr, lastCmdResult);
	postEvent(qobject, use);
}



void AbstractArbControl::postLogAlert(Alert* alert) {
	LogAlertEvent* lae = new LogAlertEvent(alert);
	postEvent(qobject, lae);
}

void AbstractArbControl::postRedisplayEvent() {
	RedisplayEvent* e = new RedisplayEvent();
    if (!_waitingRedisplay) {
        _waitingRedisplay = true;
        postEvent(qobject, e);
    }
}



void AbstractArbControl::postEnableGui(bool enable) {

   try {
	   updateArbStatus();	// The management of the GUI will require an updated status
   }
   catch(...) {
	   // In this case a GUI that requires am updated state cannot correctly enable/disable}
   }

	EnableGuiEvent* ege = new EnableGuiEvent(enable);
	postEvent(qobject, ege);

}


void AbstractArbControl::postSetMessage(string text) {
	SetMessageEvent* sme = new SetMessageEvent(text);
	postEvent(qobject, sme);
}

void AbstractArbControl::postAlert(string text) {
	AlertEvent* e = new AlertEvent(text);
	postEvent(qobject, e);
}

void AbstractArbControl::postSetAsynchMessage(string text) {
	SetAsynchMessageEvent* same = new SetAsynchMessageEvent(text);
	postEvent(qobject, same);
}

////////////////////////////////  GUI management ///////////////////////////////////

void AbstractArbControl::updateArbStatus() {

	RequestStatus  arbStatus(150);
   updateArbStatusInternal( &arbStatus, NULL);
}

void AbstractArbControl::updateArbStatusInternal( RequestStatus *useRequest, RequestStatus **useReply) {

	bool updated = false;
	int MAX_RETRIES = 3;
	int retry = 0;
   bool deleteReply = false;

   RequestStatus *reply = NULL;
   if (!useReply) {
      useReply = &reply;
      deleteReply = true;
   }

	// Try to update MAX_RETRIES times, if can't throw an exception
	// TODO This "retry" mechanism could be embedded in the ArbitratorInterface
	while(!updated && retry <= MAX_RETRIES) {
		try {
			*useReply = (RequestStatus*)(_arbIntf->requestCommand(useRequest));
			updated = true;

			// Update text
			_arbStatus = (*useReply)->getArbitratorStatus();
			_lastCmdDescr = (*useReply)->getLastCommandDescription();
			_lastCmdResult = (*useReply)->getLastCommandResult();
			postSetStatus(_arbStatus, _lastCmdDescr, _lastCmdResult);

		}
		catch(...) {
			retry++;
			if(retry > MAX_RETRIES) {
				_logger->log(Logger::LOG_LEV_ERROR, "Update status failed");
				throw;
			}
			else {
				_logger->log(Logger::LOG_LEV_WARNING, "Update status retry %d of %d", retry, MAX_RETRIES);
			}
		}
	}
   if (deleteReply)
      delete reply;

}


int AbstractArbControl::getShortTimeout() {
   return 5*1000;
}

int AbstractArbControl::getLongTimeout() {
   return 600*1000;
}

int AbstractArbControl::getTimeout() {
	int timeoutMs = 0;

	if(false/*timeoutFromConfCheck->isChecked()*/) {
		// TODO
	}
	else {
      timeoutMs =  200*1000;

		_logger->log(Logger::LOG_LEV_WARNING, "Timeout set to default value (%d ms)", timeoutMs);
	}

	_logger->log(Logger::LOG_LEV_INFO, "Command timeout: %d ms", timeoutMs);

	return timeoutMs;
}

void AbstractArbControl::sendCommandWithReply(Command* cmd, Command **reply) {

	Command* cmdReply;
	try {
		postSetMessage("Executing " + cmd->getDescription() + " command...");
		_logger->log(Logger::LOG_LEV_INFO, " >> Sending command %s request...", cmd->getDescription().c_str());
		cmdReply = _arbIntf->requestCommand(cmd);
		_logger->log(Logger::LOG_LEV_INFO, " >> Command reply (%s) received!", cmdReply->getStatusAsString().c_str());
		postSetMessage(cmd->getDescription() + " command reply received");
      processReply( cmdReply); // Virtual
		cmdReply->log();
      if (reply)
         *reply = cmdReply;
      else
		   delete cmdReply;
	}
	catch(...) {
		setMessage(cmd->getDescription() + " command has timed out");
      postAlert(cmd->getDescription() + " command has timed out");
	}

	delete cmd;

}

void AbstractArbControl::sendCommand(Command* cmd) {

   sendCommandWithReply( cmd, NULL);
}


void AbstractArbControl::sendAsynchCommand(int msgCode, void* data, int dataLen) {

	int TIMEOUT_MS = 500;

	int seqNum = thSendMsg(dataLen, (char*)(_arbMsgdId.c_str()), msgCode, 0, data);
	postSetAsynchMessage("Executing asynchronous command...");

	if(IS_ERROR(seqNum)) {
		postSetAsynchMessage("Async command not sent");
		_logger->log(Logger::LOG_LEV_ERROR, "Asynchronous command not sent (thSendMsg failed)");
	}
	else {
		int errCode;
		// Wait the reply
		MsgBuf* reply = thWaitMsg(ANY_MSG, (char*)(_arbMsgdId.c_str()), seqNum, TIMEOUT_MS, &errCode);
		// Reply received
		if(reply != NULL) {
			if(HDR_CODE(reply) == ACK) {
				postSetAsynchMessage("Async command executed");
				_logger->log(Logger::LOG_LEV_INFO, "Asynchronous command executed");
			}
			else if(HDR_CODE(reply) == NAK) {
				postSetAsynchMessage("Async command failed");
				_logger->log(Logger::LOG_LEV_ERROR, "Asynchronous command failed (NAK reply received)");
			}
			thRelease(reply);
		}
		// reply not received
		else {
			postSetAsynchMessage("Async command reply timed out");
			_logger->log(Logger::LOG_LEV_ERROR, "Asynchronous command reply timed out (thWaitMsg failed)");
			_logger->log(Logger::LOG_LEV_ERROR, "Error in thWaitMsg: %d (%s)", errCode, lao_strerror(errCode));
		}
	}
}


void AbstractArbControl::sendCommandThread(Command* cmd) {

	// Launch the new command in a thread: the command is deleted by called method!!!
	boost::thread* thCommand = new boost::thread(boost::bind(&AbstractArbControl::sendCommand, this, cmd));

	delete thCommand;	// The thread is not interrupted (see Boost documentation!)
	thCommand = NULL;
}


void AbstractArbControl::sendAsynchCommandThread(int msgCode, void* data, int dataLen) {

	// Launch the new command in a thread: the command is deleted by called method!!!
	boost::thread* thCommand = new boost::thread(boost::bind(&AbstractArbControl::sendAsynchCommand, this, msgCode, data, dataLen));

	delete thCommand;	// The thread is not interrupted (see Boost documentation!)
	thCommand = NULL;
}

//////////// Log file following

void AbstractArbControl::followLogFile( string fileName, QObject *receiver) {

   if (_tailProc)
      _tailProc->kill();

   _tailProc = new QProcess();
   _tailProc->addArgument("tail");
   _tailProc->addArgument("-F");
   _tailProc->addArgument(fileName);
   _tailProc->setCommunication(QProcess::Stdout | QProcess::Stderr);

   QObject::connect( _tailProc, SIGNAL( readyReadStdout() ), receiver, SLOT( slotDataAvail() ) );
   _tailProc->launch("");
}

void AbstractArbControl::followLogFile2( string fileName, QObject *receiver) {

   if (_tailProc2)
      _tailProc2->kill();

   _tailProc2 = new QProcess();
   _tailProc2->addArgument("tail");
   _tailProc2->addArgument("-F");
   _tailProc2->addArgument(fileName);
   _tailProc2->setCommunication(QProcess::Stdout | QProcess::Stderr);

   QObject::connect( _tailProc2, SIGNAL( readyReadStdout() ), receiver, SLOT( slotDataAvail2() ) );
   _tailProc2->launch("");
}



string AbstractArbControl::addExtension( string s, string extension)
{
  if (s== "")
     return s;

  if ((s.size() < extension.size()) || (s.substr(s.size() - extension.size()) != extension))
     s += extension;

  return s;
}





//////////////////////////////////// MAIN METHODS ///////////////////////////////////////

void AbstractArbControl::run() {

	bool guiEnabled = true;	// To not repeat a lot of postEnableGui(false) in
							// case of disconnections

	while(!_exitNow) {

		try {

			// --- Try the arbitrator initialization ---
			connectArb();

			// --- Run main loop that updates GUI
			while( (!_exitNow) && _arbConnected) {

				// Update AdSecArbitrator status
				updateArbStatus();
                Periodic();

				msleep(250);
			}
		}
		// Catch to discover suspect disconnections with AdSecArbitrator
		catch(...) {
			_logger->log(Logger::LOG_LEV_WARNING, "Connection with %s (probably) lost", _arbName.c_str());
			_arbConnected = false;

			// Udpate GUI
			if(guiEnabled) {
				postEnableGui(false);
				guiEnabled = false;
			}

			// Wait a little that Arb becomes running again...
			msleep(250);
		}

	}
}

void AbstractArbControl::die() {

   SetTimeToDie(true);
   _exitNow = true;
   wait();
}


