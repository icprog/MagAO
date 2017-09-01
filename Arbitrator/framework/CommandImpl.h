#ifndef COMMANDIMPL_H_INCLUDE
#define COMMANDIMPL_H_INCLUDE

#include <math.h>

#include "base/msgcodes.h"

#include "arblib/base/exceptions/CommandExceptions.h"
#include "arblib/base/CommandDecorator.h"
#include "framework/AbstractSystem.h"

#include <boost/thread/mutex.hpp>

using namespace Arcetri::Arbitrator;

#define PROGRESS_DEFAULT "DEFAULT"

namespace Arcetri {
namespace Arbitrator {

/*
 * Forwarded definitions to avoid include loops.
 */
class AbstractArbitrator;

/*
 * @Class: CommandImpl
 * Decorator for the Command class.
 *
 * Add a set of features (extra validation, execution, cancel)
 * to the decorated command.
 *
 * The extra validation is useful to check runtime conditions.
 * @
 */
class CommandImpl: public CommandDecorator {

	public:		// --- METHODS

		/*
		 * Create a decorated command, allowing validation,
		 * execution and undoing
		 *
		 * Dinamically set the systems the are "receivers" for the command
		 */
		CommandImpl(Command* command, map<string, AbstractSystem*> systems);

		/*
		 * This constructor can be used with commands, like Ping, that
		 * doesn't use any subsystem.
		 */
		CommandImpl(Command* command):CommandDecorator(command) { }

		/*
		 * Destroy the decorator.
		 */
		virtual ~CommandImpl();


		/*
		 * Check if the command has been canceled, and throw a CommandCanceledException
		 * that is catched by the execute method()
		 */
		void checkCanceled() throw (CommandCanceledException) { if(isCanceled()) throw CommandCanceledException(); }


	protected:	// --- METHODS

         /* 
          * Overloaded command SetStatus
          */
         void setStatus(CommandStatus status);

         /*
          * Update progress execution
          */
         void updateProgress(int percent, string text = "");

		 /*
		  * Set of methods to atomically check and change the command
		  * status.
		  */
		 bool setValidating();
		 bool setExecuting();
		 bool setCanceling();


		/*
		 * Allow derived classes to obtain the arbitrator
		 * to perform validation (is only a shortcut)
		 */
		static AbstractArbitrator* getArbitrator();


	private:

		// Only the command handler can validate, execute and cancel commands
		friend class CommandHandler;
      friend class AbstractArbitrator;

		/*
		 * Validate the command, calling the pure abstract method validateImpl().
		 * Handle all exceptions thrown in validateImpl(), and set the command
		 * status to CMD_VALIDATION_SUCCESS or CMD_VALIDATION_FAILED.
		 */
		void validate();

		/*
		 * Execute the command, calling the pure abstract method executeImpl().
		 * Handle all exceptions thrown in executeImpl(), and set the command
		 * status to CMD_EXECUTION_SUCCESS or CMD_EXECUTION_FAILED, if not
		 * interrupted by cancel(). In the last case, leaves the status unchanged.
		 */
		void execute();

		/*
		 * Ask for the command termination.
		 */
		 void cancel();

	protected:

		 /*
		  * Utility method to decrease a command timeout before using it
		  * as timeout for a request to an AbstractSystem
		  */
		 unsigned int getReducedTimeout() { return (unsigned int)round(getCommand()->getTimeout_ms() - getCommand()->getTimeout_ms()/10); }


 	protected: 	//FIELDS

 		 /*
		  * System that can must used by this command
		  */
		 map<string, AbstractSystem*> _systems;

         /*
          * completion percent
          */
         RTDBvar * _progressVar;
         int _percent;


	private:	// --- FIELDS

		/*
		 * Mutex to guarantee safe status check&update
		 */
		 boost::mutex _statusMutex;

	private:	// --- METHODS

		/*
		 * Implement this to provide validation.
		 *
		 * Note that this implentation should provide EXTRA validation,
		 * using the AbstractArbitrator. In fact the basic validation is
		 * automatically performed forwarding the call to the decorated
		 * command.
		 */
		virtual bool validateImpl() throw(CommandValidationException) = 0;

		/*
		 * Implement this to provide execution.
		 *
		 * The result can be:
		 * 	- Success: command succesfully completed
		 *  - Retry: command not executed, but retry possible
		 *  - Error: command failed
		 *
		 * To provide "cancel" feature the implementation must call
		 * checkCanceled() method.
		 */
		virtual AbstractSystemCommandResult executeImpl() throw(CommandExecutionException, CommandCanceledException) = 0;

		/*
		 * Override this to provide extra operations needed for command cancel.
		 */
		virtual bool cancelImpl() throw(CommandTerminationException) { return true; }

};

}
}

#endif /*COMMANDIMPL_H_INCLUDE*/
