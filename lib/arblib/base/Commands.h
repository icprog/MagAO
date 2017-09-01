#ifndef COMMANDS_H_INCLUDE
#define COMMANDS_H_INCLUDE

#include "arblib/base/Command.h"
//#include "arblib/wfsArb/WfsArbitratorOpCodes.h"

using namespace Arcetri::Arbitrator;


namespace Arcetri {
namespace Arbitrator {

class Ping: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		Ping(unsigned int timeout_ms): Command(BasicOpCodes::PING_CMD, "Ping", timeout_ms) {
			_sendTime = Utils::asciiDateAndTime();
			_replyTime = "undefined";
		}

		virtual ~Ping() {}

		void log() const;

		string getSendTime() { return _sendTime; }
		string getReplyTime() { return _replyTime; }

		void setReplyTime() {
			_replyTime = Utils::asciiDateAndTime();
		}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 Ping() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _sendTime;
			ar & _replyTime;
		}

		string _sendTime;
		string _replyTime;

};

/*
 * A command that ask to arbitrator to try recovering from a FAILURE state.
 *
 * Each arbitrator SHOULD implement this command if has a FAILURE state:
 * tipically this command can be implemented as a restore to  an OPERATE
 * state.
 */
class RecoverFailure: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		RecoverFailure(unsigned int timeout_ms): Command(BasicOpCodes::RECOVER_FAILURE, "Recover failure", timeout_ms) {}

		virtual ~RecoverFailure() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 RecoverFailure() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

/*
 * A command that ask to arbitrator to try recovering from a PANIC state.
 *
 * Each arbitrator SHOULD implement this command if has a PANIC state:
 * tipically this command can be implemented as a restore to an OFF
 * state.
 */
class RecoverPanic: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		RecoverPanic(unsigned int timeout_ms): Command(BasicOpCodes::RECOVER_PANIC, "Recover panic", timeout_ms) {}

		virtual ~RecoverPanic() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		RecoverPanic() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class RequestFeedback: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		RequestFeedback(unsigned int timeout_ms): Command(BasicOpCodes::REQUEST_FEEDBACK, "RequestFeedback", timeout_ms) {
			// TODO This is ugly to change MsgCode to a Request!!!
			setMsgCode(ARB_REQ);
		}

		virtual ~RequestFeedback() {}

		void setDone() { setStatus(CMD_EXECUTION_SUCCESS); }

		void log() const;

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		RequestFeedback() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class RequestStatus: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		RequestStatus(unsigned int timeout_ms): Command(BasicOpCodes::REQUEST_STATUS, "RequestStatus", timeout_ms) {
			// TODO This is ugly to change MsgCode to a Request!!!
			setMsgCode(ARB_REQ);

			_arbStatus = "UNKNOWN";
			_lastCommandDescr = "NONE";
			_lastCommandResult = "UNDEFINED";
		}

		RequestStatus(unsigned int timeout_ms, OpCode opcode): Command(opcode, "RequestStatus", timeout_ms) {
			// TODO This is ugly to change MsgCode to a Request!!!
			setMsgCode(ARB_REQ);

			_arbStatus = "UNKNOWN";
			_lastCommandDescr = "NONE";
			_lastCommandResult = "UNDEFINED";
		}

		virtual ~RequestStatus() {}

		string getArbitratorStatus()	   { return _arbStatus; }
		string getLastCommandDescription() { return _lastCommandDescr; }
		string getLastCommandResult()      { return _lastCommandResult; }

		void setArbitratorStatus(string status) 			    { _arbStatus = status; }
		void setLastCommandDescription(string lastCommandDescr) { _lastCommandDescr = lastCommandDescr; }
		void setLastCommandResult(string lastCommandResult) 	{ _lastCommandResult = lastCommandResult; }

		void setDone() { setStatus(CMD_EXECUTION_SUCCESS); }

		void log() const;

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		RequestStatus() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _arbStatus;
			ar & _lastCommandDescr;
			ar & _lastCommandResult;
		}

	    string _arbStatus;
		string _lastCommandDescr;
		string _lastCommandResult;

};

}
}


#endif /*COMMANDS_H_INCLUDE*/
