#ifndef ABSTRACT_ARB_CONTROL_H
#define ABSTRACT_ARB_CONTROL_H

#include "AOAppGui.h"
#include "RTDBvar.h"

#include <qthread.h>
#include <qevent.h>
#include <qobject.h>
#include <qprocess.h>
#include <qcolor.h>


#include "arblib/base/ArbitratorInterface.h"
#include "arblib/base/Commands.h"
#include "AOGlobals.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <vector>
#include <string>
using namespace std;

using namespace Arcetri;
using namespace Arcetri::Arbitrator;


class SetStatusEvent;
class LogAlertEvent;
class EnableGuiEvent;
class SetMessageEvent;
class AlertEvent;

#define SETSTATUS_EVENT_ID 			 (QEvent::User)
#define LOGALERT_EVENT_ID 	  		    (QEvent::User+2)
#define ENABLEGUI_EVENT_ID 		 	 (QEvent::User+3)
#define SETMESSAGE_EVENT_ID 		    (QEvent::User+4)
#define SETASYNCHMESSAGE_EVENT_ID 	 (QEvent::User+5)
#define ALERT_EVENT_ID        	    (QEvent::User+6)
#define REDISPLAY_EVENT_ID        	 (QEvent::User+7)

// For derived classes
#define FIRST_EVENT_ID               (QEvent::User+8)


typedef struct {
      string text;
         QRgb rgb;
} resultColor;


/*
 * This class implements all the common features of a generic
 * arbitrator control GUI.
 */


class AbstractArbControl: public AOAppGui {


	public:

		AbstractArbControl(string name, string configFile, KApplication &kApp);
		AbstractArbControl(int argc, char* argv[], KApplication &kApp);
		virtual ~AbstractArbControl();

		/*
		 * Initialization needed EVERY time the AdSec
		 * arbitrator is started
		 */
		virtual void connectArb();

                /*
                 * QObject needed for multi-threaded messages
                 * Must be set from derived class
                 */
                QObject *qobject;

        string getArbStatus()     const { return _arbStatus; }


      // Exit cleanly from QThread
      void die();

      virtual void redisplay() {}
      virtual void Periodic() {}

      bool _exitNow;



		/*
		 * Must be used by all *Button_clicked() methods
		 */
		void sendCommandThread(Command* cmd);
		void sendAsynchCommandThread(int msgCode, void* data, int dataLen);

		/*
		 * Retrieve the timeout for the current command
		 */
		int getShortTimeout();
		int getLongTimeout();
		int getTimeout();

	protected:	// AOApp methods

		void SetupVars();
		virtual void SetupVarsChild() {}

		virtual void PostInit();
		virtual void InstallHandlers();

		/*
		 * Handle alerts arriving from WfsArbitrator
		 */
		static int alertHandler(MsgBuf *msgb, void *argp, int hndlrQueueSize);



	protected:	// Virtual methods

		/*
		 * Event receiver: calls pure virtual methods that every subclass MUST implement.
		 * To add events, override this (and REMEMBER to call it to handle basic events!)
		 */
		virtual void customEvent(QCustomEvent* e);

		virtual void setArbitratorStatus(string /*arbStatus*/, string /*lastCmdDescr*/, string /*lastCmdResult*/) {};
		virtual void logAlert(string /*log*/) {}
		virtual void enableGui(bool /*enable*/) {}
		virtual void setMessage(string /*text*/) {}
		virtual void setAsynchMessage(string /*text*/) {}

      virtual void processReply( Command * /*reply*/) {};

		virtual void updateArbStatus();

	private:

		/*
		 * Actually construct the object
		 */
		virtual void init();

		// QThread method
		void run();


	protected:	// Event methods


		/*
		 * Event sender: set the arbitrator and last command labels (top of the GUI)
		 */
		void postSetStatus(string arbStatus, string lastCmdDescr, string lastCmdResult);

		/*
		 * Event sender: add an alert description to the alerts list
		 */
		void postLogAlert(Alert* alert);

		/*
		 * Event sender: sends a redisplay command
		 */
		void postRedisplayEvent();

		/*
		 * Event sender:: enable/disable all GUI interactive widgets
		 */
		void postEnableGui(bool enable);

		/*
		 * Event sender: set the message label (bottom of the GUI)
		 */
		void postSetMessage(string text);


		/*
		 * Event sender: set the asynch message label (bottom of the GUI)
		 */
		void postSetAsynchMessage(string text);


	protected:

		/*
		 * Event sender: sends an alert message
		 */
		void postAlert(string text);

		/*
		 * Update the following GUI items:
		 * 	- Arbitrator status
		 *  - Last command and its result
		 */
      void updateArbStatusInternal( RequestStatus *useRequest, RequestStatus **useReply);



		/*
		 * Sends a command to the arbitrator and waits for the reply
		 */
		void sendCommand(Command* cmd);
		void sendCommandWithReply(Command* cmd, Command **reply=NULL);
		void sendAsynchCommand(int msgCode, void* data, int dataLen);


	protected:

      Config_File _arbConfig; // Configuration of target arbitrator
		string _arbName;	// Got from GUI configuration file
		string _arbMsgdId;	// Got from GUI configuration file

		bool _arbConnected;

		// Useful to enable/disable GUI's widgets depending on system and arbitrator status
		static string _arbStatus;
		static string _lastCmdDescr;
		static string _lastCmdResult;

   protected:
      void followLogFile (string fileName, QObject *receiver);
      void followLogFile2 (string fileName, QObject *receiver);
      QProcess *_tailProc;
      QProcess *_tailProc2;
      string addExtension( string s, string extension);
		ArbitratorInterface* _arbIntf;

   protected slots:
      virtual void slotDataAvail() {};
      virtual void slotDataAvail2() {};

	private:

		bool _guiEnabled;
        bool _waitingRedisplay;

		static Serializator _serializator;


};


/*
 *
 */
class SetStatusEvent : public QCustomEvent {

    public:
    	SetStatusEvent(string arbStatus, string lastCmdDescr, string lastCmdResult): QCustomEvent(SETSTATUS_EVENT_ID),
																					_arbStatus(arbStatus),
																					_lastCmdDescr(lastCmdDescr),
																					_lastCmdResult(lastCmdResult) {}

    	string getArbStatus()     const { return _arbStatus; }
    	string getLastCmdDescr()  const { return _lastCmdDescr; }
    	string getLastCmdResult() const { return _lastCmdResult; }

    private:
    	string _arbStatus;
    	string _lastCmdDescr;
    	string _lastCmdResult;
};



/*
 *
 */
class LogAlertEvent : public QCustomEvent {

    public:
    	LogAlertEvent(Alert* alert): QCustomEvent(LOGALERT_EVENT_ID), _alert(alert) {}
        Alert* getAlert() const { return _alert; }

    private:
        Alert* _alert;
};


/*
 *
 */
class EnableGuiEvent : public QCustomEvent {

    public:
    	EnableGuiEvent(bool enable): QCustomEvent(ENABLEGUI_EVENT_ID), _enable(enable) {}
    	bool getEnable() const { return _enable; }

    private:
        bool _enable;
};



/*
 *
 */
class RedisplayEvent : public QCustomEvent {

    public:
    	RedisplayEvent(): QCustomEvent(REDISPLAY_EVENT_ID) {}
};



/*
 *
 */
class SetMessageEvent : public QCustomEvent {

    public:
    	SetMessageEvent(string text): QCustomEvent(SETMESSAGE_EVENT_ID), _text(text) {}
    	string getText() const { return _text; }

    private:
        string _text;
};

class AlertEvent : public QCustomEvent {

    public:
    	AlertEvent(string text): QCustomEvent(ALERT_EVENT_ID), _text(text) {}
    	string getText() const { return _text; }

    private:
        string _text;
};


/*
 *
 */
class SetAsynchMessageEvent : public QCustomEvent {

    public:
    	SetAsynchMessageEvent(string text): QCustomEvent(SETASYNCHMESSAGE_EVENT_ID), _text(text) {}
    	string getText() const { return _text; }

    private:
        string _text;
};


#endif  // ABSTRACT_ARB_CONTROL_H
