#ifndef IDLCTRLINTERFACE_H_INCLUDE
#define IDLCTRLINTERFACE_H_INCLUDE

#include "Logger.h"
#include "stdconfig.h"
#include "AOExcept.h"
#include "base/errordb.h"

#include "idl_rpc.h"

#include <string>

using namespace std;
using namespace Arcetri;

namespace Arcetri {

class IdlCommandReply;

/*
 * Base class for all the idl controller interface exceptions.
 */
class IdlCtrlInterfaceException: public AOException {
	
	public:
		IdlCtrlInterfaceException(string message, 
		                    	  int errcode = IDL_GENERIC_ERROR, 
		                    	  string file = "", 
		                    	  int line = 0): AOException(message, errcode, file, line) {}

};



/*
 * @Class: IdlCtrlInterface
 * Send commands to Idl controller via MsgD.
 * 
 * 						***ATTENTION***
 * The client using this library must initialize and start the
 * thread lib before.
 *                      ***************
 */
class IdlCtrlInterface {
	
	public:
	
		/*
		 * Creates an interface capable of sending command to the
		 * IdlRPC specified by the server_id identifier
		 */
		//IdlCtrlInterface(int server_id, int logLevel, string hostname="");
		IdlCtrlInterface(string idlCtrlName, int logLevel);
		
		/*
		 * 
		 */
		virtual ~IdlCtrlInterface();
		
		/*
		 * Send a command to IDL, waiting by default
		 * for the reply. 
		 */
		IdlCommandReply sendCommand(string idlCmd, int timeout_sec = 15) throw(IdlCtrlInterfaceException);
		
		/*
		 * Read a variable from IDL
		 */
		int getIntVariable(string idlVariable, int timeout_sec = 3) throw(IdlCtrlInterfaceException);
		
		/*
		 * Lock the Idl controller (IDLLOCK) for a mutually exclusive session.
		 * Always not blocking.
		 */
		 //void sendLock(); // Not yet implemented
		 
		 /*
		  * Unlock the Idl controller (IDLUNLOCK) for a mutually exclusive session.
		  * Always not blocking.
		  */
		 //void sendUnlock(); // Not yet implemented
		 
         /*
		  * Send SIGINT (Ctrl-C) to IDL (IDLCTRLC)
		  */
		 void sendCtrlC() throw(IdlCtrlInterfaceException);


	private:
		/*
		 * client id for RPC routines.
		 */
		CLIENT * _pClient;
		
		Logger* _logger;

      string _idlCtrlName;
};


/*
 * Defines the result of an IDL command,
 * wrapping the output and the error.
 */
class IdlCommandReply {
	
	public:
	
		// Creates an empty object
		IdlCommandReply() { _idlOut = ""; _idlErr = ""; }
	
		//  Creates the object from a msg buffer
		IdlCommandReply(MsgBuf* msgb);
		
		virtual ~IdlCommandReply(){}
		
		string getOutput() { return _idlOut; }
		string getError() { return _idlErr; }
	
	private:
	
		string _idlOut;
		string _idlErr;
};


}

#endif /*IDLCTRLINTERFACE_H_INCLUDE*/
