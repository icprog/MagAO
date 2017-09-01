#include "framework/CommandHandler.h"

#include "framework/CommandImplFactory.h"
#include "framework/CommandImpl.h"


using namespace Arcetri::Arbitrator;


CommandHandler::CommandHandler(unsigned int commandHistoryMaxSize, string ident): _ident(ident) {

	_logger = Logger::get("COMMANDHANDLER");
	_logger->log(Logger::LOG_LEV_DEBUG, "Creating CommandHandler...");

	_CMD_HISTORY_MAX_SIZE = commandHistoryMaxSize;
   _currentCommandImpl = NULL;

	_logger->log(Logger::LOG_LEV_DEBUG, "CommandHandler succesfully created!");
}


CommandHandler::~CommandHandler() {
	_logger->log(Logger::LOG_LEV_DEBUG, "Destroying CommandHandler...");
}

Command* CommandHandler::lastCommand() {

   if (_currentCommandImpl)
      return _currentCommandImpl;
   else {
	   if(_cmdHistory.size() != 0) {
   	   Command* lastCmd = _cmdHistory.front()->getCommand();
		   return lastCmd;
	   }
	   else {
	   	return NULL;
	   }
   }
}

bool CommandHandler::doExecute(CommandImpl* cmd) {

	cmd->log();

	_currentCommandImpl = cmd;

	// Check if is a basic command supported directly from the framework (i.e. Ping, ...)
	// that is not related to a specific FSM
	_logger->log(Logger::LOG_LEV_TRACE, "doExecute: Checking the command... Code = %d, Descr = %s", cmd->getCode(), cmd->getDescription().c_str());
	switch(cmd->getCode()) {
		// Set of commands NOT SENT to FSM
		case BasicOpCodes::PING_CMD:
			_logger->log(Logger::LOG_LEV_TRACE, "doExecute: Ping command found!");
			CommandHandler::execute();
			return false;	// Not inserted in the history

		// Set of commands SENT to FSM and inserted in history
		default:
			_logger->log(Logger::LOG_LEV_TRACE, "doExecute: 'normal' command found!");
			execute();	// This dinamically binds to the AbstracFsm method!

			// --- Insert the command into the history ---
			// Check history size and remove oldest if full
			if(_cmdHistory.size() == _CMD_HISTORY_MAX_SIZE) {
				delete _cmdHistory.back()->getCommand();	// Delete decorated command
				delete _cmdHistory.back();					// Delete decorator
				_cmdHistory.pop_back();
			}
			_cmdHistory.push_front(_currentCommandImpl);
			_logger->log(Logger::LOG_LEV_DEBUG, "Command saved into history (new size %d)",  _cmdHistory.size());

			_currentCommandImpl = NULL;
			return true;	// inserted into the history
	}

}


void CommandHandler::doCancel() {

	if(_currentCommandImpl != NULL) {
		_logger->log(Logger::LOG_LEV_INFO, "Canceling the current command:");
		_currentCommandImpl->log();
		_currentCommandImpl->cancel();
	}
	else {
		_logger->log(Logger::LOG_LEV_DEBUG, "No current command to cancel!");
	}
}


void CommandHandler::execute() {

	// --- Validate the command ---
	_logger->log(Logger::LOG_LEV_DEBUG, "Validating the command...");
	_currentCommandImpl->validate();

	// --- Execute the command ---
	// VALIDATED
	if(_currentCommandImpl->getStatus() == Command::CMD_VALIDATION_SUCCESS) {
		_logger->log(Logger::LOG_LEV_DEBUG, "Command succesfully validated:");
		_currentCommandImpl->log();

		// --- Execute the command
		_logger->log(Logger::LOG_LEV_DEBUG, "Executing the command...");
		_currentCommandImpl->execute();
	}
	// NOT VALIDATED (invalid parameters or exception thrown, but is the same!)
	else {
		_logger->log(Logger::LOG_LEV_WARNING, "Command validation failed:");
		_currentCommandImpl->log();
	}
}

