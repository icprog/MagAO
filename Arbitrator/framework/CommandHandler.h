#ifndef COMMANDHANDLER_H_INCLUDE
#define COMMANDHANDLER_H_INCLUDE

#include "Logger.h"

#include <list>

using namespace std;
using namespace Arcetri;

namespace Arcetri {
namespace Arbitrator {

/*
 * Forwarded definitions to avoid unuseful includes
 * that may cause loops.
 */
class CommandImplFactory;
class CommandImpl;
class Command;

/*
 * @Class: CommandHandler
 * Defines a generic command invoker class.
 *
 * Class responsibilities:
 * 	- Validate commands
 *  - Execute commands
 *  - Cancel commands
 *  - Maintain a command history useful for logging, debugging (and undo, if possible)
 * @
 */
class CommandHandler {

	public:

    CommandHandler(unsigned int commandHistoryMaxSize, string ident = "unknown");

		virtual ~CommandHandler();

		/*
		 * Returns the last processed command
		 */
		Command* lastCommand();

		/*
		 * Execute a command.
		 *
		 * Put every executed command in an history queue. The history queue
		 * is responsible of the command deletion later (the caller must
		 * guarantee the command isn't destroyed).
		 *
		 * When returns, the command's status is changed: an executed
		 * command has a status equals to Command::CMD_EXECUTION_SUCCESS.
		 * All other states (CMD_VALIDATION_FAILED, CMD_ERROR, CMD_CANCELED, ...)
		 * correspond to a NOT executed command.
		 */
		bool doExecute(CommandImpl* cmd);

		/*
		 * Cancel the command currently in execution
		 */
		void doCancel();

	protected:

		/*
		 * Execute the current command (except CMD_CANCEL).
		 *
		 * The current command is stored by _currentCommandImpl
		 */
		virtual void execute();


	protected:

		Logger* _logger;

		/*
		 * Currently processed command.
		 */
		CommandImpl* _currentCommandImpl;

      string _ident;

	private:

		/*
		 * Object used to retrieve the command implementations
		 */
		CommandImplFactory* _cmdImplFactory;


		/*
		 * History list for undo
		 */
		unsigned int _CMD_HISTORY_MAX_SIZE;
		list<CommandImpl*> _cmdHistory;

};

}
}

#endif /*COMMANDHANDLER_H_INCLUDE*/
