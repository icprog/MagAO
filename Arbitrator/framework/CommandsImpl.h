#ifndef COMMANDSIMPL_H_INCLUDE
#define COMMANDSIMPL_H_INCLUDE

#include "arblib/base/Commands.h"
#include "framework/CommandImpl.h"

using namespace Arcetri::Arbitrator;


namespace Arcetri {
namespace Arbitrator {

class PingImpl: public CommandImpl {

	public:

		/*
		 * Command parameters: [none]
		 */
		PingImpl(Ping* command): CommandImpl(command) {}

		virtual ~PingImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult executeImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

}
}

#endif /*COMMANDSIMPL_H_INCLUDE*/
