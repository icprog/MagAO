#ifndef COMMANDIMPLFACTORY_H_INCLUDE
#define COMMANDIMPLFACTORY_H_INCLUDE

#include "Logger.h"

#include "framework/AbstractSystem.h"
#include "framework/CommandImpl.h"
#include "framework/CommandsImpl.h"

namespace Arcetri {
namespace Arbitrator {

/*
 * @Class: CommandImplFactory
 * Bind each command to its implementation, that can be different depending
 * on runtime conditions.
 * This base class only binds the basic Ping command; a derived arbitrator must
 * bind its own commands.
 * @
 */
class CommandImplFactory {

	public:

		CommandImplFactory() {
			_logger = Logger::get("COMMANDHANDLER");
		}

		virtual ~CommandImplFactory() { _logger->log(Logger::LOG_LEV_DEBUG, "Destroying CommandImplFactory..."); }

		/*
		 * Implements a given command, providing the receiver/s (systems) at runtime.
		 *
		 * Define a few base command implementations:
		 * 	- Ping
		 *  - ...
		 */
		virtual CommandImpl* implementCommand(Command* cmd, map<string, AbstractSystem*> /*systems*/) {
			_logger->log(Logger::LOG_LEV_DEBUG, "Base CommandImplFactory retrieving command implementation...");
			switch(cmd->getCode()) {

				case BasicOpCodes::PING_CMD:
					_logger->log(Logger::LOG_LEV_DEBUG, "CommandImplFactory: PING_CMD found");
					return new PingImpl((Ping*)cmd);

				default:
					_logger->log(Logger::LOG_LEV_DEBUG, "CommandImplFactory: NO COMMANDS found");
					return NULL;
			}
		}

	protected:

		Logger* _logger;

};

}
}

#endif /*COMMANDIMPLFACTORY_H_INCLUDE*/
