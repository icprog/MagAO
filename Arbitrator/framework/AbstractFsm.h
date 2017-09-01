#ifndef ABSTRACTFSM_H_INCLUDE
#define ABSTRACTFSM_H_INCLUDE

#include "framework/CommandHandler.h"
#include "arblib/base/BasicOpCodes.h"

namespace Arcetri {
namespace Arbitrator {

/*
 * @Class: AbstractFsm
 * Extends the CommandHandler functionalities with the FSM logic.
 * @
 */
class AbstractFsm : public CommandHandler {

	public:

		/*
		 * Constructor.
		 */
    AbstractFsm(unsigned int commandHistoryMaxSize, string ident = "abstract");

		/*
		 * Destructor.
		 */
		virtual ~AbstractFsm();

		/*
		 * Return the current state of the FSM as a string.
		 */
		virtual string currentStateAsString() = 0;

	protected:

		/*
		 * Extends the base class method, providing FSM features.
		 *
		 * A command received with status CMD_NOT_PROCESSED is returned with status:
		 *  - CMD_NOT_PROCESSED: command not accepted by FSM.
		 * 	- CMD_VALIDATION_FAILED: command not validated (i.e. wrong parameters).
		 *  - CMD_EXECUTION_FAILED: command has failed its execution. The FSM is moved to a failure
		 * 	  state.
		 *  - CMD_EXECUTION_NULL: command has failed its execution, but the FSM is still in the same state.
		 * 						  This means that the client can repeat the command.
		 *  - CMD_EXECUTION_SUCCESS: command succesfully executed.
		 */
		void execute();

		/*
		 * Used by derived class to continue the command processing,
		 * when the FSM allow the command.
		 *
		 * Return true only if the command has been succesfully processed
		 * and the FSM can move to target state.
		 */
		bool processCommand();


	private:

		/*
		 * Trigger an event to the FSM using a command OpCode.
		 *
		 * The derived class must implement this method.
		 */
		virtual void trigger(OpCode code) = 0;

		/*
		 * Force a transition to a failure state in the FSM
		 * (if the current state allow this transition!)
		 *
		 * The derived class must implement this method.
		 */
		 virtual void forceFailure() = 0;

		/*
		 * Force a transition to a panic state in the FSM
		 * (if the current state allow this transition!)
		 *
		 * The derived class must implement this method.
		 */
		 virtual void forcePanic() = 0;


};

}
}

#endif /*ABSTRACTFSM_H_INCLUDE*/
