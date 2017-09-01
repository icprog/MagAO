/* @File: ArbitratorInterface.h
 * Declaration of the SerializableMessageSender, AlertNotifier and
 * ArbitratorInterface classes.
 * @
 */

#ifndef ABSTRACTARBITRATORINTERFACE_H_INCLUDE
#define ABSTRACTARBITRATORINTERFACE_H_INCLUDE

#include "AOExcept.h"
#include "Logger.h"

#include "arblib/base/Serializator.h"
#include "arblib/base/exceptions/InterfaceExceptions.h"
#include "arblib/base/BasicOpCodes.h"
#include "arblib/base/Command.h"
#include "arblib/base/Alerts.h"

#include <string>

using namespace std;
using namespace Arcetri;
using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace Arbitrator {

/*
 * @Class: SerializableMessageSender
 * Class able to send SerializableMessages to a target application and
 * wait for a reply (optionally).
 *
 * 						*** ATTENTION ***
 * The client using this library must initialize and start the
 * thread lib before!!!
 *                      *****************
 * @
 */
class SerializableMessageSender {

	public:

		/*
		 * Create an object able to send a SerializableMessage to a target.
		 * The interfaceName is only used for logging purposes.
		 *
		 */
		SerializableMessageSender(string intfName = "SER-MSG-SENDER", int logLevel = Logger::LOG_LEV_INFO);

		~SerializableMessageSender();

		/*
		 * Send a SerializableMessage to a target, defined by its MsgD identity.
		 * Optionally waits for a reply (see SerializableMessage class).
		 *
		 * The message reply is returned if message succesfully sent and
		 * processed by the target; else NULL is returned.
		 */
		SerializableMessage* send(string target, SerializableMessage* m) throw(ArbitratorInterfaceException);

		/*
		 * Set the interface log level
		 */
		void setLogLevel(int logLevel) { _logger->setLevel(logLevel); }

	protected:

		/*
		 * Default logger.
		 */
		Logger* _logger;

	private:

			string _interfaceName;

			/*
			 * Object implementing serialization/deserialization
			 */
			Serializator _serializator;
};

/*
 * @Class: AlertNotifier
 * Interface class providing the basic functionalities to to notify alerts
 * to a target.
 *
 * This class provides a set of default 'alerts', defined
 * in "Alerts.h".
 * @
 */
class AlertNotifier: public SerializableMessageSender {

	public:

		AlertNotifier(string targetMsgdIdentity, int logLevel = Logger::LOG_LEV_INFO);

		~AlertNotifier() {}

		/*
		 * Notify an alert to a target application.
		 * Doesn't expect a reply, but immediately returns.
		 */
		void notifyAlert(Alert* alert) throw(ArbitratorInterfaceException);

	private:

		string _targetMsgdIdentity;

};


/*
 * @Class: ArbitratorInterface
 * Interface class providing the basic functionalities to send 'commands' and
 * 'commands cancel' to an Arbitrator application.
 *
 * Note that the Arbitrator will send back:
 * 1. Synchronous reply to the commands - if not disabled with Command::setWantReply(false).
 * 2. Asynchronous alerts (messages with code ARB_ALERT).
 *
 * For the second case the client (usually derived from AOApp) must register a
 * thrdlib handler capable of receiving them. An example is provided in
 * Arbitrator/aoarbitrator/AOArbitrator.cpp, that receives alerts from the AdSecArbitrator.
 *
 * A specific cliend should provide a set of custom 'commands'
 * accepted by the 'target' Arbitrator.
 * (i.e. see "arblib/aoArb/AOCommands.h")

 * @
 */
class ArbitratorInterface: public SerializableMessageSender {

	public:

		ArbitratorInterface(string targetMsgdIdentity, int logLevel = Logger::LOG_LEV_INFO); 	// Log level used for all modules

		~ArbitratorInterface() {}

		/*
		 * Request the Arbitrator to execute a generic command.
		 * A reply is expected if the command requires it (see Command and
		 * SerializableMessage classes).
		 *
		 * The command reply (the same command with status changed) is returned
		 * if command succesfully sent and processed by the arbitrator; NULL
		 * is returned if the reply is not requested.
		 *
		 * An ArbitratorInterfaceException is thrown in case of errors in command
		 * sending or reply receiving. In this case the reply is invalid, and must
		 * not be used!
		 */
		Command* requestCommand(Command* cmd) throw(ArbitratorInterfaceException);

		/*
		 * Request the Arbitrator to cancel the current command.
		 *
		 * Note that NOTHING is returned: the correct execution of this request
		 * is only reported by the "early termination" of the canceled command!
		 */
		void cancelCommand();

                /*
                 * Return the arbitrator to which this interface is sending commands.
                 */

                string getTargetArbitrator();

                /*
                 * Checks whether the target arbitrator exists and is reachable.
                 */
                bool check();


	private:

		string _targetMsgdIdentity;

};

}
}

#endif /*ABSTRACTARBITRATORINTERFACE_H_INCLUDE*/
