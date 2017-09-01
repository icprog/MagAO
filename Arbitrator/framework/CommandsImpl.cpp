#include "framework/CommandsImpl.h"

using namespace Arcetri::Arbitrator;

bool PingImpl::validateImpl() throw(CommandValidationException) {

	_logger->log(Logger::LOG_LEV_DEBUG, "<Ping Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult PingImpl::executeImpl() throw(CommandExecutionException, CommandCanceledException) {

	((Ping*)getCommand())->setReplyTime();
	return SysCmdSuccess;
}

bool PingImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Ping Command undo has an empty implementation>");
	return true;
}
