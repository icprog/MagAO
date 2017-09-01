/*
 * Defines Command and Request classes.
 */

#ifndef COMMAND_H_INCLUDE
#define COMMAND_H_INCLUDE

#include "base/msgcodes.h"

#include "arblib/base/SerializableMessage.h"
#include "arblib/base/exceptions/CommandExceptions.h"

#include <boost/serialization/base_object.hpp>

#define DO_FSM true
#define NO_FSM false

using namespace Arcetri;
using namespace Arcetri::Arbitrator;


namespace Arcetri {
namespace Arbitrator {

/*
 * Forwarded definitions to avoid unuseful includes
 * that may cause loops.
 */
class CommandDecorator;

/*
 * Type for the command status.
 * Change into a class if needed.
 */
typedef int CommandStatus;

/*
 * @Class: Command
 * Defines the basic class for an Arbitrator Command object.
 *
 * Basically a Command class must be derived to implement
 * a concrete command, with its own fields
 * @
 */
class Command: public SerializableMessage {

	/*
	 * Allow a command decorator to correctly forward
	 * all methods.
	 */
	friend class CommandDecorator;

	public:		// --- METHODS

		/*
		 * Available command status.
		 */
		static const CommandStatus CMD_NOT_PROCESSED 	  = 0; 		// Command still not processed by Arbitrator
		static const CommandStatus CMD_VALIDATING		  = 100;	// Commang validation in progress
		static const CommandStatus CMD_VALIDATION_SUCCESS = 101; 	// Command validation success
		static const CommandStatus CMD_VALIDATION_FAILED  = 102; 	// Command validation failed
		static const CommandStatus CMD_EXECUTING		  = 200;	// Commang execution in progress
		static const CommandStatus CMD_EXECUTION_SUCCESS  = 201; 	// Command succesfully executed by Arbitrator (state changed)
		static const CommandStatus CMD_EXECUTION_RETRY    = 202; 	// Command execution failed, but is possible to retry (state unchanged)
		static const CommandStatus CMD_EXECUTION_FAILED	  = 203;  	// Command not executed because of errors (state error - TODO TBD)
		static const CommandStatus CMD_CANCEL_REQUEST	  = 300;	// Command has been requested for cancel
		static const CommandStatus CMD_CANCEL_SUCCESS	  = 301;	// Command succesfully canceled
		static const CommandStatus CMD_CANCEL_FAILED	  = 302;	// Command cancel failed because of errors


		/*
		 * Create a command.
		 *
		 * ATTENTION: please set a reasonable timeout_ms!
		 *
		 * the <fms> flag indicates whether this command must be processed by the Arbitrator fsm, and is normally true.
		 */
        Command(OpCode code, string description, unsigned int timeout_ms, string progressVar = "", bool fsm=true);

		/*
		 * Destroy a command.
		 */
		virtual ~Command();

		/*
		 * Log all the command info.
		 */
		virtual void log() const;

		/*
		 * Get the command description.
		 */
		virtual string getDescription() const { return _description; }

		/*
		 * Get the result of the command execution.
		 */
		 virtual CommandStatus getStatus() const { return _status; }

		/*
		 * Get the result of the command execution as a string.
		 */
		 virtual string getStatusAsString() const;

		/*
		 * Set the error string of the command execution.
		 */
		void setErrorString( string errString ) { _errString = errString; }

		/*
		 * Get the error string of the command execution.
		 */
		 string getErrorString() const { return _errString; }

		 /*
		 * Return true only if command succesfully executed.
		 */
		 bool isSuccess() const { return _status == CMD_EXECUTION_SUCCESS; }

		 /*
		 * Return true only if command got a RETRY reply.
		 */
		 bool isRetry() const { return _status == CMD_EXECUTION_RETRY; }

		 /*
		 * Return true only if command got an ERROR or VALIDATION FAILED reply.
		 */
		 bool isError() const { return (_status == CMD_EXECUTION_FAILED) || (_status == CMD_VALIDATION_FAILED); }

		 /*
		 * Return true only if command got an ERROR or VALIDATION FAILED reply.
		 */
		 bool isNotValidated() const { return _status == CMD_VALIDATION_FAILED; }

		 /*
		  * Validate the command.
		  * This method can be called or not by the Command creator,
		  * but in any case the Arbitrator will repeat the validation
		  * for safety reasons.
		  */
		 virtual void validate();


	protected:	// --- METHODS

		/*
		 * Default constructor.
		 * Needed for boost serialization.
		 */
		Command();

		/*
		 * Set a command description.
		 */
		virtual void setDescription(string descr) { _description = descr; }

		/*
		 * Set the result of the command execution.
		 */
		virtual void setStatus(CommandStatus newStatus);

		/*
		 * Return true only if a cancel has been requested or completed with
		 * success or failure.
		 */
		bool isCanceled() { return getStatus() == CMD_CANCEL_REQUEST || getStatus() == CMD_CANCEL_SUCCESS || getStatus() == CMD_CANCEL_FAILED; }

		/*
		 * Return true only if a execution has completed, with success or failure.
		 */
		bool isExecuted() { return getStatus() == CMD_EXECUTION_SUCCESS || getStatus() == CMD_EXECUTION_FAILED; }


	private:	// --- FIELDS

		// A short command description
		string _description;

		/*
		 * Result of the command execution.
		 */
		CommandStatus _status;

      /*
       * Error string of comman execution.
       */
      string _errString;


	private:	// --- METHODS

		/*
		 * Implement this to provide basic (client side) validation
		 */
		virtual bool validateImpl() throw(CommandValidationException) = 0;

		/*
		 * Boost base serialization.
		 */
		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<SerializableMessage>(*this);
			// Complete serialization with its own fields
			ar & _status;
			ar & _description;
         ar & _errString;
			// A derived class must especially serialize the command paramaters
		}

};


/*
 * @Class: Request
 *
 * A request is handled independently from commands, that is in a concurrent way, by a dedicated
 * thrdlib handler.
 * @
 */
class Request: public SerializableMessage {

	public:		// --- METHODS

		/*
		 * Create a command.
		 *
		 * ATTENTION: please set a reasonable timeout_ms!
		 */
		Request(OpCode code, string description, unsigned int timeout_ms);

		/*
		 * Destroy a request.
		 */
		virtual ~Request();

		/*
		 * Log all the request info.
		 */
		virtual void log() const;

		/*
		 * Get the request description.
		 */
		virtual string getDescription() const { return _description; }


		bool getCompleted() const { return _completed; }
		void setCompleted(bool completed) { _completed = completed; }


	protected:	// --- METHODS

		/*
		 * Default constructor.
		 * Needed for boost serialization.
		 */
		Request();


		/*
		 * Set a command description.
		 */
		virtual void setDescription(string descr) { _description = descr; }


	private:	// --- FIELDS

		// A short command description
		string _description;

		// The result
		bool _completed;


	private:	// --- METHODS

		/*
		 * Boost base serialization.
		 */
		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<SerializableMessage>(*this);
			// Complete serialization with its own fields
			ar & _description;
			ar & _completed;
			// A derived class must especially serialize the command paramaters
		}

};


}
}

using namespace Arcetri::Arbitrator;

#endif /*COMMAND_H_INCLUDE*/
