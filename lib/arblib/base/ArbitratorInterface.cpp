
extern "C" {
	#include "base/thrdlib.h"
}

#include "arblib/base/ArbitratorInterface.h"
#include "Utils.h"

using namespace Arcetri;
using namespace Arcetri::Arbitrator;

// ******************************** SerializableMessageSender ********************************* //

SerializableMessageSender::SerializableMessageSender(string intfName, int logLevel) {

	_logger = Logger::get(intfName, logLevel);
	Logger::get("SERIALIZATION", logLevel);
	_logger->printStatus();


	_logger->log(Logger::LOG_LEV_DEBUG, "SerializableMessageSender for succesfully created!");
}


SerializableMessageSender::~SerializableMessageSender() {
	//Do something?
	_logger->log(Logger::LOG_LEV_DEBUG, "SerializableMessageSender succesfully destroyed");
}


SerializableMessage* SerializableMessageSender::send(string target, SerializableMessage* msg) throw(ArbitratorInterfaceException) {

	// Set the message sender
	msg->setSender(thInfo()->myname);

	// --- Serialize the message
	int bufLen;
	char* buf = _serializator.serialize(msg, bufLen);
	if(buf == NULL) {
		throw(ArbitratorInterfaceException("Serialization error"));
	}

	// *** Internal deserialization for test *** //
	if(_logger->getLevel() == Logger::LOG_LEV_VTRACE) {
		_logger->log(Logger::LOG_LEV_VTRACE, "*** TEST: internal deserialization ***");
		SerializableMessage* msg = _serializator.deserialize(buf, bufLen);
		_logger->log(Logger::LOG_LEV_VTRACE, "**************************************");
		delete msg;
	}

	// --- Insert the buffer in a MsgD message and send it
	int seqNum = thSendMsgPl(bufLen, (char*)target.c_str(), msg->getMsgCode(), msg->getCode(), NOHANDLE_FLAG, (void*)buf);
	delete buf;

	// Send FAILURE
	if (IS_ERROR(seqNum)) {
      	int stat = PLAIN_ERROR(seqNum);
		_logger->log(Logger::LOG_LEV_DEBUG, "Error in thSendMsg: %d (%s)", stat, lao_strerror(stat));
      	throw ArbitratorInterfaceException("Command send error");
   	}
   	// Send SUCCESS
   	else {
   		_logger->log(Logger::LOG_LEV_DEBUG, "Message succesfully sent");

		// Optionally waits for the reply: the arbitrator reply with a message
		// with the same msgCode (status and description changed)
		if(msg->getWantReply()) {
			_logger->log(Logger::LOG_LEV_DEBUG, "Waiting message reply: sender=%s, code=%d", target.c_str(), msg->getCode());
			int stat;
			MsgBuf* replyBuf = thWaitMsg(ARB_REPLY, (char*)target.c_str(), seqNum, msg->getTimeout_ms(), &stat);
			// Check if reply succesfully got
			if (!replyBuf) {
				_logger->log(Logger::LOG_LEV_DEBUG, "Error in thWaitMsg: %d (%s)", stat, lao_strerror(stat));
		      	throw ArbitratorInterfaceException("Command timeout");
		    }
		    _logger->log(Logger::LOG_LEV_DEBUG, "Message reply succesfully received");

			// Retrieve serializable message from the message buffer
			SerializableMessage* msg = _serializator.deserialize(replyBuf);

			// Release the buffer
			thRelease(replyBuf);

			// Finalize
			if(msg == NULL) {
				throw(ArbitratorInterfaceException("Deserialization error"));
			}
			else {
				return msg;
			}
		}
		else {
			return NULL;
		}
   	}
}

// *************************************** AlertNotifier *************************************** //

AlertNotifier::AlertNotifier(string targetMsgdIdentity, int logLevel): SerializableMessageSender("ALERT-NOTIFIER", logLevel) {

	_targetMsgdIdentity = targetMsgdIdentity;
	_logger->log(Logger::LOG_LEV_INFO, "ArbitratorInterface to %s succesfully created!", _targetMsgdIdentity.c_str());
}

void AlertNotifier::notifyAlert(Alert* alert) throw(ArbitratorInterfaceException) {
	// [todo] Could check if opCode is a valid alert code
	_logger->log(Logger::LOG_LEV_DEBUG, "notifyAlert: sender=%s,  code=%d, message='%s'", alert->getSender().c_str(), alert->getCode(), alert->getTextMessage().c_str());
	send(_targetMsgdIdentity, alert);
}

// ************************************ ArbitratorInterface ************************************ //


ArbitratorInterface::ArbitratorInterface(string targetMsgdIdentity, int logLevel): SerializableMessageSender("ARB-INTERFACE", logLevel) {

	_targetMsgdIdentity = targetMsgdIdentity;
	_logger->log(Logger::LOG_LEV_INFO, "ArbitratorInterface to %s succesfully created!", _targetMsgdIdentity.c_str());
}


Command* ArbitratorInterface::requestCommand(Command* cmd) throw(ArbitratorInterfaceException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "Requesting command %s to %s...", cmd->getDescription().c_str(), _targetMsgdIdentity.c_str());
	try {
		return (Command*)send(_targetMsgdIdentity, cmd);
	}
	catch(ArbitratorInterfaceException& e) {
        if (e._message.compare("Command timeout") != 0)
            _logger->log(Logger::LOG_LEV_WARNING, "%s", e.what().c_str());
		throw;
	}

}

string ArbitratorInterface::getTargetArbitrator()
{
	return _targetMsgdIdentity;
}

bool ArbitratorInterface::check() {
	return thHaveYou( _targetMsgdIdentity.c_str()) == 1;
}


void ArbitratorInterface::cancelCommand() {
	// The cancel command doesn't have a payload, and don't generate
	// a reply
	_logger->log(Logger::LOG_LEV_DEBUG, "Requesting cancel command...");
	int seqNum = thSendMsg(0, (char*)_targetMsgdIdentity.c_str(), ARB_CANC_CMD, 0, NULL);

	if (IS_ERROR(seqNum)) {
      	int stat = PLAIN_ERROR(seqNum);
		_logger->log(Logger::LOG_LEV_DEBUG, "Error in thSendMsg: %d (%s)", stat, lao_strerror(stat));
      	throw ArbitratorInterfaceException("Failed to send Cancel command");
   	}
   	// Send SUCCESS
   	else {
   		_logger->log(Logger::LOG_LEV_INFO, "Cancel command succesfully sent");
   	}
}

