#ifndef COMMANDEXCEPTIONS_H_INCLUDE
#define COMMANDEXCEPTIONS_H_INCLUDE

#include "AOExcept.h"

namespace Arcetri {
namespace Arbitrator {

	/*
	 * Base class for all the exceptions related to Command class.
	 */
	class CommandException: public AOException {
		public: 
			CommandException(string message, 
							 int errcode = ARB_CMD_GENERIC_ERROR,
							 string file = "", 
			                 int line = 0): AOException(message, errcode, file, line) {}
	}; 
	
	class CommandValidationException: public CommandException {
		public: 
			CommandValidationException(): CommandException("CommandValidationException", ARB_COMMAND_VALID_ERROR) {}
	};
	
	class CommandExecutionException: public CommandException {
		public: 
			CommandExecutionException(): CommandException("CommandExecutionException", ARB_COMMAND_EXEC_ERROR) {}
		
	};
	
	class CommandCanceledException: public CommandException {
		public: 
			CommandCanceledException(): CommandException("CommandCanceledException", ARB_COMMAND_EXEC_ERROR) {}
		
	};
	
	class CommandTerminationException: public CommandException {
		public: 
			CommandTerminationException(): CommandException("CommandTerminationException", ARB_COMMAND_EXEC_ERROR) {}
		
	};
	
	class CommandUndoingException: public CommandException {
		public: 
			CommandUndoingException(): CommandException("CommandUndoingException", ARB_COMMAND_EXEC_ERROR) {}
		
	};
	
}
}

#endif /*COMMANDEXCEPTIONS_H_INCLUDE*/
