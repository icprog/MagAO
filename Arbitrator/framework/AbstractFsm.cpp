#include "framework/AbstractFsm.h"

#include "framework/CommandImpl.h"

using namespace Arcetri::Arbitrator;

AbstractFsm::AbstractFsm(unsigned int commandHistoryMaxSize, string ident): 
    CommandHandler(commandHistoryMaxSize, ident) {
	_logger->log(Logger::LOG_LEV_DEBUG, "Initializing Fsm (command handler)...");
}

AbstractFsm::~AbstractFsm() {

}

// This receives only command
void AbstractFsm::execute() {

	_logger->log(Logger::LOG_LEV_INFO, "FSM (status: %s) has received command %d (%s)",
                                            currentStateAsString().c_str(), 
                                               _currentCommandImpl->getCode(), 
                                                  _currentCommandImpl->getDescription().c_str());

	trigger(_currentCommandImpl->getCode()); // This will call processCommand() or nothing

	if(_currentCommandImpl->getStatus() == Command::CMD_EXECUTION_SUCCESS) {
		_logger->log(Logger::LOG_LEV_INFO, "Command %s (code %d) successfully completed",  _currentCommandImpl->getDescription().c_str(), _currentCommandImpl->getCode());
   }
   else if(_currentCommandImpl->getStatus() == Command::CMD_NOT_PROCESSED ) {
       _logger->log(Logger::LOG_LEV_ERROR, "%s: Fsm is discarding command %d (%s)...", _ident.c_str(),
                    _currentCommandImpl->getCode(), _currentCommandImpl->getDescription().c_str());
       _currentCommandImpl->getCommand()->setErrorString(_ident+":Illegal command for state " + 
                          currentStateAsString()+" (" +_currentCommandImpl->getCommand()->getDescription() +")" );
	}
	// If execution NOT SUCCESS, react appropriatley
	else if(_currentCommandImpl->getStatus() != Command::CMD_EXECUTION_SUCCESS) {
		// CMD_EXECUTION_FAILED
		// This means that the command execution is really failed,
		// usually because an execution exception
		// CASE A: if a RECOVER_FAIL has failed, force a panic
		if(_currentCommandImpl->getCode() == BasicOpCodes::RECOVER_FAILURE && _currentCommandImpl->getStatus() == Command::CMD_EXECUTION_FAILED) {
			_logger->log(Logger::LOG_LEV_INFO, "Command execution failed: Fsm forced to Panic");
			// Move FSM to a panic state
			forcePanic();
		}
		// CASE B: if another command has failed, force a failure
		else if(_currentCommandImpl->getStatus() == Command::CMD_EXECUTION_FAILED) {
			_logger->log(Logger::LOG_LEV_INFO, "Command execution failed: Fsm forced to Failure");
			// Move FSM to a failure state
			forceFailure();
		}
		// CMD_VALIDATION_FAILED/CMD_CANCEL_SUCCESS/CMD_EXECUTION_NULL
		// This means that the command hasn't been executed, because of:
		// 	- Validation failure
		//	- Cancel success
		// or has been executed but without any effect (arbitrator still in the same state)
		else {
			_logger->log(Logger::LOG_LEV_INFO, "Command not executed (status %s) (errstr %s)", _currentCommandImpl->getStatusAsString().c_str(), _currentCommandImpl->getCommand()->getErrorString().c_str());
		}
	}

	_logger->log(Logger::LOG_LEV_DEBUG, "Fsm new state: %s", currentStateAsString().c_str());
}


bool AbstractFsm::processCommand() {
	_logger->log(Logger::LOG_LEV_DEBUG, "Fsm is processing command %d (%s)...", _currentCommandImpl->getCode(), _currentCommandImpl->getDescription().c_str());
	CommandHandler::execute();
	return (_currentCommandImpl->getStatus() == Command::CMD_EXECUTION_SUCCESS);
}


