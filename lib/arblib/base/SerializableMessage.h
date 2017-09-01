#ifndef SERIALIZABLEMESSAGE_H_INCLUDE
#define SERIALIZABLEMESSAGE_H_INCLUDE

#include <string>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "base/msgcodes.h"
#include "Logger.h"

#include "arblib/base/BasicOpCodes.h"

using namespace std;
using namespace Arcetri;
using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace Arbitrator {


/*
 * @Class: SerializableMessage
 * Generic message for the AOArbitrator.
 * 
 * The message can be serialized/deserialized to be used as buffer
 * for a MsgD message.
 * 
 * Each serializable message is identified by:
 * 	- msgCode: the code of the message, i.e. alert or command 
 *             (see AOARB_* in msgcode.h for valid types)
 *  - opCode: the code of the specific command or alert.
 *  - description: a (possibly) short description.
 *  - sender: the process which sends the message.
 * @
 */
class SerializableMessage {
	
	public:
	
		/*
		 * Constructor.
		 */
		SerializableMessage(MsgCode msgCode, 				// Generic code defined in msgcodes.h	
							OpCode code, 					// Specific code
							bool wantReply = false,			// Wait the reply
							unsigned int timeout_ms = 0, 	// 0 = wait forever (only if wantReply = true)
                            string progressVar = "");       // Progress variable to be filled by server
		
		virtual ~SerializableMessage();
		
		virtual void log() const;
		
		// MODIFIERS //
		
		virtual void setMsgCode(MsgCode msgCode) { _msgCode = msgCode; };
		
		virtual void setCode(OpCode code) { _code = code; };
		
		virtual void setSender(string sender) { _sender = sender; };
		
		virtual void setWantReply(bool wantReply) { _wantReply = wantReply; }
		
		virtual void setTimeout_ms(unsigned int timeout_ms) { _timeout_ms= timeout_ms; }

        virtual void setProgressVar(string progressVar) { _progressVar = progressVar; };
		
		// ACCESSORS ///
		
		virtual MsgCode getMsgCode() const { return _msgCode; }
		
		virtual OpCode getCode() const { return _code; }
		
		virtual string getSender() const { return _sender; }
		
		virtual bool getWantReply() const { return _wantReply; }
		
		virtual unsigned int getTimeout_ms() const { return _timeout_ms; }

        virtual string getProgressVar() const { return _progressVar; }
	
	protected:
	
		/*
		 * Default constructor.
		 * Needed for boost serialization.
		 */
		SerializableMessage();
	
		Logger* _logger;
		
	private:
	
		/*
		 * Defines the boost serialize function.
		 * 
		 * Override this if derived class contains extra member fields
		 * to be serialized/deserialized. 
		 * Also remeber to call
		 * 
		 *   ar & boost::serialization::base_object<SerializableMessage>(*this);
		 * 
		 * before serializing your owns member fields.
		 */
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			ar & _msgCode;
		    ar & _code;
		    ar & _sender;
		    ar & _wantReply;
		    ar & _timeout_ms;
            ar & _progressVar;
		}	
	
		// Message type (see AOARB_* in msgcode.h for valid types)
		MsgCode _msgCode;
	
		// Operation code
		OpCode _code;
	
		// The Arbitrator client which sends the abstract operation
		string _sender;
		
		// This message wants a reply from the receiver.
		bool _wantReply;
		
		// Reply timeout
		unsigned int _timeout_ms;

        // Progress variable to be filled by server
        string _progressVar;
};

}
}


#endif /*SERIALIZABLEMESSAGE_H_INCLUDE*/


