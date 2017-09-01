#ifndef SERIALIZATOR_H_
#define SERIALIZATOR_H_

extern "C" {
	#include "base/thrdlib.h"
}

#include "Logger.h"

namespace Arcetri {
namespace Arbitrator {

/*
 * Forwarded definitions to avoid unuseful includes
 * that may cause loops.
 */
class SerializableMessage;

/*
 * @Class: Serializator
 * Provide serialization and deserialization for objects of class
 * SerializableMessage (including derived classes).
 * 
 * A SerializableMessage is serialized into an array of chars.
 * @
 */
class Serializator {
	
	public:
		
		/*
		 * Create a serializator with a given log level
		 */
		Serializator() {
			_logger = Logger::get("SERIALIZATION");
		}
		
		/*
		 * Destroy the serializator
		 */
		virtual ~Serializator() {}
	
		/*
		 * Create a buffer from a SerializableMessage.
		 * The size of the buffer returned is set in the bufLen parameter.
		 * If the serialization fails, a NULL pointer is returned, and 
		 * bufLen is set to zero.
		 * 
		 * ATTENTION: the client has the responsibility of deleting the 
		 * returned buffer.
		 */
		virtual char* serialize(SerializableMessage* msg, int& bufLen);
		
		/*
		 * Create a serializable message from a buffer.
		 * If deserialization fails, returns NULL.
		 */
		virtual SerializableMessage* deserialize(const char* buf, int bufLen);
		
		
		/*
		 * SHORCUT METHOD (useful for use with thrdlib):
		 * create a SerializableMessage from a MsgBuf.
		 * If deserialization fails, returns NULL.
		 */
		virtual SerializableMessage* deserialize(MsgBuf*);
		
		
		private:
		
			Logger* _logger;
};

}
}

#endif /*SERIALIZATOR_H_*/
