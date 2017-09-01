#ifndef ABSTRACTARBITRATOR_H_INCLUDE
#define ABSTRACTARBITRATOR_H_INCLUDE

#include "AOApp.h"

#include "arblib/base/ArbitratorInterface.h"
#include "arblib/base/Serializator.h"
#include "framework/ArbitratorExceptions.h"

using namespace Arcetri::Arbitrator;


namespace Arcetri {
namespace Arbitrator {

/*
 * Forwarded definitions to avoid unuseful includes
 * that may cause loops.
 */
class AbstractSystem;
class CommandImplFactory;
class CommandHandler;
class Command;
class Alert;


/*
 * @Class: AbstractArbitrator
 * Defines a basic abstract class for an "arbitrator" implementation, i.e.
 * the AOArbitrator.
 *
 * The AbstractArbitrator is the main framework module, and is implemented as
 * a SINGLETON. The instance of a AbstractArbitrator knows about the entire
 * system and its status (*): therefore it is able to react to command requests
 * and alerts notify.
 *
 * NOTE (*)
 * From the initial system status, all status' changes MUST derives from:
 * 	- Commands execution (and their results)
 *  - Alerts notifications
 *
 * Class responsibilities:
 * - Read configuration files
 * - Initializes a pool af Loggers
 * - Initialize the CommandHandler
 * - Contains interfaces to sub-arbitrators
 * - Contains interfaces to client that want to be notified on alerts
 * - Check the connection with upper-level arbitrators
 * - Registers handlers for messages (containing Requests, Commands and Alerts)
 * - Set up mirror of variabels (to export low-level variables into arbitrator ones)
 * - Receives MsgD messages containing Commands an Alerts and:
 *  	. Process the commands, using the command handler (AbstractCommandHandler)
 * 		. Process Alerts, reacting appropriately
 * - Receives MsgD messages containing Requests, execute them (or forward to derived class) and reply
 * - Is notified on internally generated alerts (by AbstractSystems), and process them
 * - Clean up everything when destroyed
 * @
 */
 class AbstractArbitrator: public AOApp {

	public:

		/*
		 * Retrieve the singleton instance
		 */
		static AbstractArbitrator* getInstance() { return _instance; }

		/*
		 *	Destructor, cleans up.
		 */
		virtual ~AbstractArbitrator();

		/*
		 * Return the command handler used by this arbitrator.
		 *
		 * Used by CancelCommand implementation.
		 */
		CommandHandler* getCommandHandler() { return _cmdHandler; }

		/*
		 * Notify the arbitrator for an alert.
		 *
		 * Used by arbitrator clients (framework's AbstractSystems implementations)
		 * to notify internally detected problems.
		 */
		void notifyAlert(Alert* alert);

		/*
		 * Notifies specified alert to registred clients.
         *
         * Used in order to broadcast a processed alert or abnormal 
         * condition to registered clients.
		 */
		void notifyClients(Alert* alert);

        /*
         * Returns the interface for another arbitrator, if added before
         */
		ArbitratorInterface* getArbitratorInterface(string targetArbMsgDIdnty);


	public: // Thrdlib handlers

		/*
		 * Registered handler for messages with code ARB_REQ
		 *
		 * A request is a command that doesn't modify the Arbitrator state,
		 * and can be satisfied without being processed by command handler
		 * (typically a request about the arbitrator status)
		 */
		static int handleRequest(MsgBuf* commandMsgBuf, void *argp, int hndlrQueueSize);

		/*
		 * Registered handler for messages with code AOARB_CMD
		 * (see lib/base/msgcodes.h).
		 *
		 * If the reply is requested (see Command and SerializableMessage classe), a reply
		 * is sent to the client. The reply is the same command with changed status.
		 */
		static int handleCommand(MsgBuf* commandMsg, void *argp, int hndlrQueueSize);

		/*
		 * Registered handler for messages with code AOARB_ASYNC_CMD
		 * (see lib/base/msgcodes.h).
       *
       * These commands are not handled by the FSM and do not change the FSM state.
		 *
		 * If the reply is requested (see Command and SerializableMessage classe), a reply
		 * is sent to the client. The reply is the same command with changed status.
		 */
		static int handleAsyncCommand(MsgBuf* commandMsg, void *argp, int hndlrQueueSize);

		/*
		 * Registered handler for messages with code  AOARB_CANC_CMD.
		 * This kind of messages must be handled in a cuncurrent way with respect
		 * to the other ones.
		 *
		 * The arbitrator don't reply to this kind of command: the effect is visible
		 * in the reply of the canceled command.
		 */
		static int handleCancelCommand(MsgBuf* commandMsg, void *argp, int hndlrQueueSize);

		/*
		 * Registered handler for messages with code AOARB_ALERT (see lib/base/msgcodes.h)
		 * Simply extract the Alert from the buffer and calls processAlert(alert).
		 *
		 * A reply is never sent, since Alerts are all asynchronous.
		 */
		static int handleAlert(MsgBuf* alertMsg, void *argp, int hndlrQueueSize);


	protected: // METHODS from AOApp

		/*
		 * [todo] Evaluate which vars need.
		 */
		virtual void SetupVars();

		/*
		 * Install:
		 *  - Request handler
		 * 	- Command handler
		 *  - Alert handler
		 *  - Cancel command handler
		 */
        virtual void InstallHandlers();

        /*
         * Perform:
         * 	- Command handler installed check
         *  - ...
         */
        virtual void PostInit();

        /*
         * Execute the arbitrator.
         */
        virtual void Run();

        /*
         * Useful ???
         */
        virtual int  VariableHandler(Variable *){return NO_ERROR;};

        /*
         * Useful ???
         */
        virtual void StateChange(int /*oldstate*/, int /*state*/) { };
		/*
		 * Returns the last processed command
		 */
		Command* lastCommand();

	protected: // METHODS used by derived class

		/*
		 * Command line constructor
		 */
		AbstractArbitrator(int argc, char* argv[]) throw (ArbitratorException);

		/*
		 * Creates and add the interface for a sub arbitrator.
		 * The interface is added to _subArbInterfaces dictionary
		 * and a reference is returned
		 */
		ArbitratorInterface* addArbitratorInterface(string targetArbMsgDIdnty);

		 /*
		  * Set a logger with a given level and returns a reference to it.
		  *
		  * The logger name is used to retrieve the log level in the config file.
		  * If the config file value isnt' found, the default value
		  * ("LogLevel" config file parameter value) is used.
		  */
		Logger* setLoggerLevel(string loggerName);

		 /*
		  * Send the reply corresponding to the received command:
		  * the reply is the same object with MsgCode changed
		  */
		 static int sendReply(SerializableMessage* msg, MsgBuf* buf, Logger* logger);

       /*
        * Function called roughly once per second.
        */

       virtual void Periodic() {}

       /*
        * Replace <side> pattern in variable names
        */
       string ReplaceSide( string s);
   
        /* 
         * Sets busy flag
         */
        static void setBusyFlag( bool busy = true) { _busy = busy; };

   public:
		/*
		 * Returns the state of the FSM
		 */
		string fsmState();

        /* 
         * Returns busy flag
         */
        static bool getBusyFlag() { return _busy; };

        /* 
         * Get Stop commands flag
         */
        static bool getStopFlag() { return _stop; };

        /* 
         * Set Stop commands flag
         */
        static void setStopFlag(bool stop = true) { _stop = stop; };

       

        // Returns systems map
        map<string, AbstractSystem*> &getSystems();


	private: // METHODS

		/*
		 * Constructor's implementation
		 */
		void create();

		/*
		 * Setup all the framework's loggers.
		 */
		 void setupLoggers();

		 /*
		 * Wait that a target arbitrator (identified in MsgD with 'targetArbMsgDIdnty')
		 * is running, retrying with a certain delay.
		 * The Arbitrator Interface used to communicate with it must be present
		 * in _arbInterfaces dictionary.
		 */
		void waitSubArbitrator(string targetArbMsgDIdnty, int delay_ms = 1000);

		/*
		 * Creates and add an alert notifier for a client.
		 * The interface is added to _arbInterfaces dictionary and each
		 * registered client is notified in case of an alert.
		 */
		void addAlertNotifier(string targetArbMsgDIdnty);

		/*
		 * Called by the handler of messages with code ARB_REQ in case
		 * the request can't be satisfied by base class
		 */
		virtual void serviceRequest(Request* /*req*/, MsgBuf* /*requestMsgBuf*/) {}

		/*
		 * Process a command, modifying its status.
		 *
		 * Returns true if the command has been inserted
		 * into the history
		 *
		 * A derived Arbitrator can override this to modify
		 * the pool of AbstractSystems at runtime
		 */
		virtual bool processCommand(Command* command);

		/*
		 * Process a command, modifying its status.
		 *
		 * A derived Arbitrator can override this to modify
		 * the pool of AbstractSystems at runtime
		 */
		virtual bool processAsyncCommand(Command* command);

		/*
		 * Cancel the command currently in execution
		 */
		virtual void cancelCommand();

		/*
		 * Process an alert.
		 */
		virtual void processAlert(Alert* alert) = 0;

      /*
       * Handler for variable mirroring into MSGD.
       */

      static int mirrorHandler( void *pt, Variable *msgb);
      map<string, string> _toMirror;

	protected:	// FIELDS

		/*
		 * Handle the commands
		 * To be set by derived class!
		 */
		CommandHandler* _cmdHandler;

		/*
		 * Implements commands
		 * To be set by derived class!
		 */
		CommandImplFactory* _cmdImplFactory;

		/*
		 * Systems that are dinamically provided
		 * for command implementation
		 * To be added by derived class!
		 */
		static map<string, AbstractSystem*> _systems;


		/*
		 * Interfaces used to communicate with lower-level arbitrators.
		 */
		static map<string, ArbitratorInterface*> _subArbInterfaces;

		/*
		 * Interfaces used to notify upper-level clients.
		 */
		static map<string, AlertNotifier*> _clientInterfaces;



		// --- Configuration parameters --- //
		int _commandHistorySize;
		// -------------------------------- //


      // RTDB variables
      RTDBvar  *_fsmStateVar;

      // busy flag (singleton)
      static bool _busy;

      // stop commands flag (global singleton)
      static bool _stop;

      // Map of CLSTAT variables
      map<string, int> _clStat;

      // CLSTAT variable handlers
      static int clStatHandler( void *pt, Variable *msgb);

      // Aggregate info on clStat variables
      bool clStatOK();



	private: // FIELDS

		/*
		 * Singleton instance, set by constructor
		 */
		static AbstractArbitrator* _instance;


		// [todo] These two values must be set to some reasonable value
		static const int HNDLR_QUEUE_COMMANDS_LIMIT = 10;
		static const int HNDLR_QUEUE_REQUESTS_LIMIT = 10;
		static const int HNDLR_QUEUE_ALERTS_LIMIT   = 10;


		// Used in static handlers
		static Serializator* _serializator;

};

}
}

#endif /*ABSTRACTARBITRATOR_H_INCLUDE*/
