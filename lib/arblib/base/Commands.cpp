#include "Commands.h"

using namespace Arcetri::Arbitrator;


////////////////////////

bool Ping::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "Ping Command has an empty (basic) validation>");
	return true;
}

void Ping::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_TRACE, " >> Output parameters:");
	_logger->log(Logger::LOG_LEV_TRACE, "      Send time: %s", _sendTime.c_str());
	_logger->log(Logger::LOG_LEV_TRACE, "      Reply time: %s", _replyTime.c_str());
}

////////////////////////

bool RecoverFailure::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<RecoverFailure Command has an empty (basic) validation>");
	return true;
}

////////////////////////

bool RecoverPanic::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<RecoverPanic Command has an empty (basic) validation>");
	return true;
}


////////////////////////

bool RequestFeedback::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "RequestFeedback Command has an empty (basic) validation>");
	return true;
}

void RequestFeedback::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_TRACE, " >> Input parameters:");
	_logger->log(Logger::LOG_LEV_TRACE, "      Requester (sender): %s", getSender().c_str());
}

////////////////////////

bool RequestStatus::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "RequestStatus Command has an empty (basic) validation>");
	return true;
}

void RequestStatus::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_TRACE, " >> Input parameters:");
	_logger->log(Logger::LOG_LEV_TRACE, "      Requester (sender): %s", getSender().c_str());
	_logger->log(Logger::LOG_LEV_TRACE, " >> Output parameters:");
	_logger->log(Logger::LOG_LEV_TRACE, "      Arbitrator status:   %s", _arbStatus.c_str());
	_logger->log(Logger::LOG_LEV_TRACE, "      Last command:        %s", _lastCommandDescr.c_str());
	_logger->log(Logger::LOG_LEV_TRACE, "      Last command result: %s", _lastCommandResult.c_str());
}
