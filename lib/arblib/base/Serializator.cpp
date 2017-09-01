#include "arblib/base/Serializator.h"
#include "arblib/base/SerializableMessage.h"

using namespace Arcetri::Arbitrator;

#include <boost/serialization/base_object.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/nvp.hpp>

char* Serializator::serialize(SerializableMessage* msg, int& bufLen) {
	
	char* buf = NULL;
	bufLen = 0;
	
	// Serialize the message into an outoput stream
	try {
		_logger->log(Logger::LOG_LEV_DEBUG, "Serializing message:");
		msg->log();
		ostringstream serializedMsg;
		boost::archive::binary_oarchive archive(serializedMsg);
		archive << BOOST_SERIALIZATION_NVP(msg);	
		_logger->log(Logger::LOG_LEV_DEBUG, "Message succesfully serialized!");
		
		// Store the stream into a buffer
		string strBuf = serializedMsg.str();	// Note that this is in the function stack!
		bufLen = strBuf.length();
		buf = new char[bufLen];					// Store the buffer in the heap, to be sure
		memcpy(buf, strBuf.data(), bufLen);		// it will be available to the client
		_logger->log(Logger::LOG_LEV_DEBUG, "Created buffer of size %d", bufLen);
		
	}
	catch(...) {
		_logger->log(Logger::LOG_LEV_ERROR, "Error in message serialization");
	}
	
	return buf;
}

SerializableMessage* Serializator::deserialize(const char* buf, int bufLen) {
	
	_logger->log(Logger::LOG_LEV_VTRACE, "Received buffer of size %d", bufLen);

	// Create the input stream from buffer
	istringstream serializedMsg(string(buf, bufLen));
	
	// Deserialize it
	SerializableMessage* msg = NULL;
	try {
		_logger->log(Logger::LOG_LEV_VTRACE, "Deserializing message...");
		boost::archive::binary_iarchive archive(serializedMsg);
		archive >> BOOST_SERIALIZATION_NVP(msg);
		_logger->log(Logger::LOG_LEV_VTRACE, "Message succesfully deserialized:");
		msg->log();
	}

	catch(...) {
        throw;
	}
	
	return msg;
}

SerializableMessage* Serializator::deserialize(MsgBuf* buf) {

    SerializableMessage* msg = NULL;
    try {
        msg = deserialize((const char*)MSG_BODY(buf), HDR_LEN(buf));
        msg->setSender(HDR_FROM(buf));
    }
    catch (...) {
        _logger->log(Logger::LOG_LEV_ERROR, "Error in message deserialization from %s (%6.6x/%d)",
                     HDR_FROM(buf), HDR_CODE(buf), HDR_PLOAD(buf));
    }
    
    return msg;
}
