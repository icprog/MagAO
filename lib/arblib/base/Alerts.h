#ifndef ALERT_H_INCLUDE
#define ALERT_H_INCLUDE

#include "base/msgcodes.h"

#include "arblib/base/SerializableMessage.h"

#include <boost/serialization/base_object.hpp>

using namespace Arcetri::Arbitrator;


namespace Arcetri {
namespace Arbitrator {
	
/* @Class: Alert
 * 
 * @
 */
class Alert : public SerializableMessage {
	
	public:
		
		Alert(OpCode code, string shortDescription, string textMessage = "");
		
		virtual ~Alert();
		
		virtual void log() const;
		
		/*
		 * Set a short alert description.
		 */
		virtual void setShortDescription(string descr) { _shortDescription = descr; }
		
		
		
		/*
		 * Return a human-readable text message
		 * about the alert.
		 */
		virtual void setTextMessage(string textMessage) { _textMessage = textMessage; }
		
		/*
		 * Get the short alert description.
		 */
		virtual string getShortDescription() const { return _shortDescription; }
		
		/*
		 * Set a human-readable text message
		 * about the alert.
		 */
		virtual string getTextMessage() const { return _textMessage; }
		
	protected:
	
		/*
		 * Default constructor.
		 */
		Alert() {}
		
	private:
	
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<SerializableMessage>(*this);
			// Complete serialization with its own fields
			ar & _shortDescription;
			ar & _textMessage;
		}
		
		// A short description (i.e. "Warning", "Error"...)
		string _shortDescription;
	
		// Human-readable text message
		string _textMessage;
		
};


/* @Class: Warning
 * 
 * @
 */
class Warning: public Alert {
	
	public:
	
		Warning(string message): Alert(BasicOpCodes::WARNING, "Warning", message) {}
		
		virtual ~Warning() {}
		
	protected:
	
		/*
		 * Default constructor.
		 * Needed for boost/serialization
		 */
		Warning(): Alert() {}
		
	private:
	
		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Alert>(*this);
		}
	
};


/* @Class: Error
 * 
 * @
 */
class Error: public Alert {
	
	public:
	
		Error(string message, bool requestRip=false): Alert(BasicOpCodes::ERROR, "Error", message), _requestRip(requestRip) {}
		
		virtual ~Error() {}

      bool requestRip() { return _requestRip; }
		
	protected:
	
		/*
		 * Default constructor.
		 * Needed for boost/serialization
		 */
		Error(): Alert() {}
		
	private:

   bool _requestRip;

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Alert>(*this);
         ar & _requestRip;
		}
	
};


/* @Class: Panic
 * 
 * @
 */
class Panic: public Alert {
	
	public:
	
		Panic(string message): Alert(BasicOpCodes::PANIC, "Panic", message) {}
		
		virtual ~Panic() {}
		
	protected:
	
		/*
		 * Default constructor.
		 * Needed for boost/serialization
		 */
		Panic(): Alert() {}
		
	private:
	
		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Alert>(*this);
		}
	
};

/* @Class: HighOrderOffload
 * 
 * @
 */
class HighOrderOffload: public Alert {
	
	public:
	
		HighOrderOffload(string filename): Alert(BasicOpCodes::HOOFFLOAD, "HighOrderOffload", "HighOrderOffload vector ready") {_filename=filename;}
		
		virtual ~HighOrderOffload() {}
		
		string getHighOrderOffloadFilename() { return _filename;} 
	protected:
	
		/*
		 * Default constructor.
		 * Needed for boost/serialization
		 */
		HighOrderOffload(): Alert() {}
		
	private:
		string _filename;
	
		friend class boost::serialization::access;
	    	template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Alert>(*this);
			ar & _filename;
		}
	
};

/* @Class: DumpSaved
 * 
 * @
 */
class DumpSaved: public Alert {

   public:

      DumpSaved(string filename): Alert(BasicOpCodes::DUMPSAVED, "DumpSaved", "DumpSaved vector ready") {_filename=filename;}

      virtual ~DumpSaved() {}

      string filename() { return _filename;}
   protected:

      /*
       * Default constructor.
     * Needed for boost/serialization
      */
      DumpSaved(): Alert() {}
                                       
   private:
      string _filename;

      friend class boost::serialization::access;
         template<class Archive>
      void serialize(Archive& ar, const unsigned int /*version*/) {
         // Serialize base class object
         ar & boost::serialization::base_object<Alert>(*this);
         ar & _filename;
   }
                                                        
};


}
}

using namespace Arcetri::Arbitrator; 
//  BOOST_IS_ABSTRACT(Alert);    ?? L.Fini

#endif /*ALERT_H_INCLUDE*/
