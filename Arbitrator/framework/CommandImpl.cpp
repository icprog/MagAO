#include "framework/CommandImpl.h"
#include "framework/AbstractArbitrator.h"

using namespace Arcetri::Arbitrator;


CommandImpl::CommandImpl(Command* command, map<string, AbstractSystem*> systems):
             CommandDecorator(command) 
{
	_systems = systems;
    if (getProgressVar() != "") {
        if (getProgressVar().compare(PROGRESS_DEFAULT) == 0) {
            // if default is set then take owns progress var
            _progressVar = new RTDBvar(getArbitrator()->MyName() + ".PROGRESS", CHAR_VARIABLE, 50);
        }
        else {
            // otherwise use the one passed from the command
            _progressVar = new RTDBvar(getProgressVar(), CHAR_VARIABLE, 50);
        }
    }
    else {
        _progressVar = NULL;
    }
}


CommandImpl::~CommandImpl() {

}

AbstractArbitrator* CommandImpl::getArbitrator() {
	return AbstractArbitrator::getInstance();
}



void CommandImpl::setStatus(CommandStatus status) {

    CommandDecorator::setStatus(status);

    int percent;

    switch (status) {
    case CMD_VALIDATING:
        percent = 0;
        break;
    case CMD_EXECUTING:
        percent = 1;
        break;
    default:
        percent = 100;
    }
    
    updateProgress(percent);
}

void CommandImpl::updateProgress(int percent, string text) {

    if ((_progressVar != NULL) && (percent != _percent)) {

        char value[50];
        
        if (text.length() > 0)
            snprintf(value, 50, "%i:%s", percent, text.c_str());
        else 
            snprintf(value, 50, "%i", percent);
            
        _progressVar->Set(value);
        _percent = percent;
    }
}

bool CommandImpl::setValidating() {
	boost::mutex::scoped_lock scoped_lock(_statusMutex);
	if(!isCanceled()) {
		setStatus(CMD_VALIDATING);
		return true;
	}
	else {
		return false;
	}
}


bool CommandImpl::setExecuting() {
	boost::mutex::scoped_lock scoped_lock(_statusMutex);
	if(!isCanceled()) {
		setStatus(CMD_EXECUTING);
		return true;
	}
	else {
		return false;
	}
}


bool CommandImpl::setCanceling() {
	boost::mutex::scoped_lock scoped_lock(_statusMutex);
	if(!isExecuted()) {
		setStatus(CMD_CANCEL_REQUEST);
		return true;
	}
	else {
		return false;
	}
}


void CommandImpl::validate() {

	// Check if request already canceled
	if(setValidating()) {
		try {
			// Calls basic validation
			// [to check] This could be moved after object construction
			CommandDecorator::validate();
			if(getStatus() == CMD_VALIDATION_SUCCESS) {
				_logger->log(Logger::LOG_LEV_DEBUG, "[EXTRA VALIDATE] Performing extra validation:");
				log();
				if(!validateImpl()) {
					setStatus(CMD_VALIDATION_FAILED);
				}
			}
		}
		catch (CommandValidationException& e) {
			_logger->log(Logger::LOG_LEV_WARNING, "[EXTRA VALIDATE] %s", e.what().c_str());
			setStatus(CMD_VALIDATION_FAILED);
		}
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "[EXTRA VALIDATE] Can't validate: command already canceled");
	}
}


void CommandImpl::execute() {

	// Check if request already canceled
	if(setExecuting()) {
		_logger->log(Logger::LOG_LEV_DEBUG, "[EXECUTE] Performing execution:");
		log();

		try {
			// Execute (executeImpl() method can exit because terminated or canceled)
			AbstractSystemCommandResult res = executeImpl();
			if(res == SysCmdSuccess) {
				setStatus(CMD_EXECUTION_SUCCESS);
			}
			else if(res == SysCmdRetry) {
				setStatus(CMD_EXECUTION_RETRY);
			}
			else {
				setStatus(CMD_EXECUTION_FAILED);
			}
		}
		catch (CommandExecutionException& e) {
			_logger->log(Logger::LOG_LEV_WARNING, "[EXECUTE] %s", e.what().c_str());
			setStatus(CMD_EXECUTION_FAILED);
		}
		catch (CommandCanceledException& e) {
			_logger->log(Logger::LOG_LEV_INFO, "[EXECUTE] Command canceled");
		}
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "[EXECUTE] Can't execute: command already canceled");
	}
}


void CommandImpl::cancel() {

	// Check if execution already terminated and force checkCanceled() to
	// throw an exception, catched by the execute() method
	if(setCanceling()) {
		_logger->log(Logger::LOG_LEV_DEBUG, "[CANCEL] Requesting cancel:");
		log();

		try {
			// This call should reduce the execution of the executeImpl()
			// method.
			if (cancelImpl()) {
				setStatus(CMD_CANCEL_SUCCESS);
			}
			else {
				setStatus(CMD_CANCEL_FAILED);
			}
		}
		catch(CommandTerminationException& e) {
			_logger->log(Logger::LOG_LEV_WARNING, "[CANCEL] %s", e.what().c_str());
			setStatus(CMD_CANCEL_FAILED);
		}
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "[CANCEL] Can't cancel: command already executed");
	}
}

