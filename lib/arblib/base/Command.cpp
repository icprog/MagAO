#include "arblib/base/Command.h"

using namespace Arcetri::Arbitrator;


////////////////////////////////////// COMMAND /////////////////////////////////////////7

Command::Command() {

}

Command::Command(OpCode code, string description, unsigned int timeout_ms, string progressVar, bool fsm):

    SerializableMessage((fsm) ? ARB_CMD : ARB_ASYNC_CMD, code, true, timeout_ms, progressVar) {

	setDescription(description);
	setStatus(CMD_NOT_PROCESSED);
}


Command::~Command() {

}


void Command::setStatus(CommandStatus status) {
	 _status = status;
	 _logger->log(Logger::LOG_LEV_TRACE, "'%s' command status set to %s", getDescription().c_str(), getStatusAsString().c_str());
}


void Command::validate(){
	_logger->log(Logger::LOG_LEV_TRACE, "[BASE VALIDATE] Performing basic validation:");
	log();
	try {
		if(validateImpl()) {
			setStatus(CMD_VALIDATION_SUCCESS);
		}
		else {
			setStatus(CMD_VALIDATION_FAILED);
		}
	}
	catch(CommandValidationException& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "[BASE VALIDATE] %s", e.what().c_str());
		setStatus(CMD_VALIDATION_FAILED);
	}
}

void Command::log() const {
	SerializableMessage::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Command: descr=%s, status=%d (%s)", getDescription().c_str(), getStatus(), getStatusAsString().c_str());
}

string Command::getStatusAsString() const {
	switch(getStatus()) {
		case CMD_NOT_PROCESSED:
			return "NOT PROCESSED";
		case CMD_VALIDATING:
			return "VALIDATING";
		case CMD_VALIDATION_SUCCESS:
			return "VALIDATION SUCCESS";
		case CMD_VALIDATION_FAILED:
			return "VALIDATION FAILED";
		case CMD_EXECUTING:
			return "EXECUTING";
		case CMD_EXECUTION_SUCCESS:
			return "EXECUTION SUCCESS";
		case CMD_EXECUTION_FAILED:
			return "EXECUTION FAILED";
      case CMD_EXECUTION_RETRY:
         return "RETRY";
		case CMD_CANCEL_REQUEST:
			return "CANCEL REQUEST";
		case CMD_CANCEL_SUCCESS:
			return "CANCEL SUCCESS";
		case CMD_CANCEL_FAILED:
			return "CANCEL FAILED";

		default:
			return "UNKNOWN";
	}
}


////////////////////////////////////// REQUEST /////////////////////////////////////////7

Request::Request() {

}


Request::Request(OpCode code, string description, unsigned int timeout_ms):
    SerializableMessage(ARB_REQ, code, true, timeout_ms) {

	setDescription(description);
	setCompleted(false);
}


Request::~Request() {

}


void Request::log() const {
	SerializableMessage::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Request: descr=%s, completed=%d", getDescription().c_str(), getCompleted());
}

