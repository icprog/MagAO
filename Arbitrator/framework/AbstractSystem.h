#ifndef ABSTRACTSYSTEM_H_INCLUDE
#define ABSTRACTSYSTEM_H_INCLUDE

#include "Logger.h"

#include "arblib/base/Alerts.h"
#include "arblib/base/Command.h"
#include "arblib/base/exceptions/CommandExceptions.h"

#include <string>

using namespace std;

namespace Arcetri {
namespace Arbitrator {

/*
 * Forwarded definitions to avoid unuseful includes
 * that may cause loops.
 */
class AbstractArbitrator;

/*
 * Execution result of a command
 */
enum AbstractSystemCommandResult { SysCmdSuccess=0, SysCmdError=1, SysCmdRetry=2};

/*
   @Class: AbstractSystem
   Define an abstract class for a "(sub)system" implementation. A subsystem
   can be:
   	- Another arbitrator
    - A pool of processes
  
   In practice this is a wrapper around a (sub)system interface library, that
   in addition stores the (sub)system status.
  
   Class responsibilities:
   	- Maintain the controlled system's status (used for undo).
    - Implements all the commands required by an implementation
      of the AbstractArbitrator.
    - Notify feedback to the AbstractArbitrator.
   @
 */
class AbstractSystem {

	public:

		/* 
         @@Method: AbstractSystem

		   Create an abstract system.
         @
		 */
		AbstractSystem(string systemName);  //@P{systemName}: MsgD client name

		/*
		 * Copy conctructor is implemented by default.
		 * If special copy needs, implement it.
		 */
		 //AbstractSystem(const AbstractSystem& system) {}

		/*
		 *
		 */
		virtual ~AbstractSystem();


		/*
         @@Method: getName

		   Returns the system MsgD client name.
         @
		 */
		string getName() { return _name; }

		/*
         @@Method:  getCommandResult

		   Returns the result of the executed command
         @
		 */
		AbstractSystemCommandResult getCommandResult() { return _cmdResult; }
      
      /*
       * Returns the error string (if any) of the executed command
       */
      string getErrorString() {  return _cmdErrorString; }

      /*
       * Set error string (if any) of the executed command
       */
      void setErrorString( string errString) { _cmdErrorString = errString; }

	protected:

            /*
             *   Initialize command result
             */
            void initCommandResult() {
                  _cmdResult = SysCmdError;
                  _cmdErrorString = "";
	    }
		/*
		 * Set the result of the executed command
		 */
		void setCommandResult(AbstractSystemCommandResult cmdRes, string errorString="");


      /*
       * Propagate the result of an executed command. Safe to call with a NULL pointer.
       */

      void setCommandResultFromReply( Command *reply);


	protected:

		Logger* _logger;

	private:

		string _name;

		AbstractSystemCommandResult _cmdResult;
      string _cmdErrorString;
};

}
}

#endif /*ABSTRACTSYSTEM_H_INCLUDE*/
