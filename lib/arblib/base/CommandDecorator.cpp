#include "arblib/base/CommandDecorator.h"

CommandDecorator::CommandDecorator(Command* command) {
	
		_command = command;
}

CommandDecorator::~CommandDecorator() {
	// Doesn't destroy the decorated object!
	_command = NULL;
	_logger->log(Logger::LOG_LEV_TRACE, "Command decorator deleted");
}
