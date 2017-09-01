#ifndef COMMANDDECORATOR_H_INCLUDE
#define COMMANDDECORATOR_H_INCLUDE

#include "arblib/base/Command.h"

using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace Arbitrator {

/*
 * @Class: CommandDecorator
 * A base decorator (wrapper) class for a Command object.
 * 
 * This basic decorator doesn't add any feature to a Command, but
 * allow to create custom decorators because it forwards all methods 
 * to the decorated command.
 * 
 * A concrete decorator must provides extra fetures to a command.
 * I.e. see the frameowrk/CommandImpl, which provides validation 
 * and execution.
 * @
 */
class CommandDecorator: public Command {
	
	public:
		
		CommandDecorator(Command* command);
		
		/*
		 * Doesn't destroy the decorated command
		 */
		virtual ~CommandDecorator();
		
		virtual void log() const  { _command->log(); }
		
		Command* getCommand() { return _command; }
		
		 
		void setMsgCode(MsgCode msgCode) { _command->setMsgCode(msgCode); };
		
		void setCode(OpCode code) {_command->setCode(code); };
		
		void setDescription(string descr) { _command->setDescription(descr); }
		
		void setSender(string sender) { _command->setSender(sender); };
		
		void setWantReply(bool wantReply) { _command->setWantReply(wantReply); }
		
		void setTimeout_ms(unsigned int timeout_ms) { _command->setTimeout_ms(timeout_ms); }

        void setProgressVar(string progressVar) { _command->setProgressVar(progressVar); }
		
		
		MsgCode getMsgCode() const { return _command->getMsgCode(); }
		
		OpCode getCode() const { return _command->getCode(); }
		
		string getDescription() const { return _command->getDescription(); }
		
		string getSender() const { return _command->getSender(); }
		
		bool getWantReply() const { return _command->getWantReply(); }
		
		unsigned int getTimeout_ms() { return _command->getTimeout_ms(); }

        string getProgressVar() const { return _command->getProgressVar(); }
		
		CommandStatus getStatus() const { return  _command->getStatus(); }
		
		string getStatusAsString() const { return  _command->getStatusAsString(); }
		
		bool isSuccess() const { return _command->isSuccess(); }
		
		virtual void validate() { _command->validate(); }
		
		
	protected:	// METHODS
	
		/*
		 * Used by derived classes to set command status
		 */
		void setStatus(CommandStatus status) { _command->setStatus(status); }
		
	private: // FIELDS
	
		Command* _command;
		
};

}
}

#endif /*COMMANDDECORATOR_H_INCLUDE*/
