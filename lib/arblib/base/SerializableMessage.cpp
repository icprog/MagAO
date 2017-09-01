#include "arblib/base/SerializableMessage.h"

using namespace Arcetri::Arbitrator;

SerializableMessage::SerializableMessage() { 
	_logger = Logger::get("SERIALIZATION"); 
}

SerializableMessage::SerializableMessage(MsgCode msgCode, OpCode code, bool wantReply, unsigned int timeout_ms, string progressVar) {
	
	_logger = Logger::get("SERIALIZATION");
	
	setMsgCode(msgCode);
	setCode(code);
	setSender("");	// This must be set only if the message is sent to someone.
	setWantReply(wantReply);
	setTimeout_ms(timeout_ms);
    setProgressVar(progressVar);
}

SerializableMessage::~SerializableMessage() {
	_logger->log(Logger::LOG_LEV_TRACE, "Message deleted", getCode());
}

void SerializableMessage::log() const {
	_logger->log(Logger::LOG_LEV_DEBUG, "SerialMsg: sender=%s, msgCode=0x%X, code=%d, reply=%d, timeout_ms=%d progressVar=%s", 
				 getSender().c_str(), getMsgCode(), getCode(), getWantReply(), getTimeout_ms(), getProgressVar().c_str());
}
