#include "idllib/IdlCtrlInterface.h"

extern "C" {
#include "idllib/idllib.h"
}

using namespace Arcetri;

// Define this to use the idl rpc interface
#undef USE_RPC

/************+*************** IDLCTRLINTERFACE ********************************/

IdlCtrlInterface::IdlCtrlInterface(string idlCtrlName, int logLevel) {
//IdlCtrlInterface::IdlCtrlInterface(int server_id, int logLevel, string hostname){
	
	// --- Create the default logger --- //
	_logger = Logger::get("IDL-INTERFACE", logLevel);
	_logger->printStatus();

#ifdef USE_RPC   
	const char* host = hostname.c_str();

   // RPC code
	// Register the client, check for errors      
   
    if( (_pClient = IDL_RPCInit( server_id, (char*)host)) ==NULL){
		_logger->log(Logger::LOG_LEV_ERROR, "Can't register with server on %s [%s:%d]",
	       host[0]!='\0' ? host : "localhost", __FILE__, __LINE__);
      	throw IdlCtrlInterfaceException("Failed to initialize communication with IDL");
   	}

	// Reset the timeout
	int timeout = 120;
    if(!IDL_RPCTimeout(timeout)){ 
		_logger->log(Logger::LOG_LEV_ERROR, "Unable to set timeout to %d sec [%s:%d]",
	       timeout, __FILE__, __LINE__);
    }
#else

    _idlCtrlName = idlCtrlName;

#endif
}

IdlCtrlInterface::~IdlCtrlInterface() {

#ifdef USE_RPC
	int ret = IDL_RPCCleanup(_pClient, 0);
    if (!ret) _logger->log(Logger::LOG_LEV_ERROR, "IDL_RPCCleanup failed [%s:%d]", __FILE__, __LINE__);
#endif
}

IdlCommandReply IdlCtrlInterface::sendCommand(string idlCmd, int timeout_sec) throw (IdlCtrlInterfaceException) {


#ifdef USE_RPC
	
     _logger->log(Logger::LOG_LEV_INFO, "Sending Command to IDLRPC w/ timeout %d sec: %s [%s:%d]", 
              timeout_sec, idlCmd.c_str(), __FILE__, __LINE__);

	
	// set timeout
    if(!IDL_RPCTimeout(timeout_sec)){ 
		_logger->log(Logger::LOG_LEV_ERROR, "Unable to set timeout to %d sec [%s:%d]",
	       timeout_sec, __FILE__, __LINE__);
    }
	
    // reset error code in case of previous errors, otherwise IDL_RPCExecuteStr return error forever
	IDL_RPCExecuteStr(_pClient, "!error_state.code=0");
	// call execute string
	int seqNum = IDL_RPCExecuteStr(_pClient, (char*)idlCmd.c_str());
	
	// Send FAILURE
	if (seqNum != 1) {
		_logger->log(Logger::LOG_LEV_ERROR, "Error in IDL_RPCExecuteStr: %d [%s:%d]",
				seqNum, __FILE__, __LINE__ );
      	throw IdlCtrlInterfaceException("Failed to send command to IDL");
   	}
#else

	// --- Insert the idlCmd in a MsgD message and send it
   int stat, seqn;
	seqn = thSendMsg(idlCmd.length(), (char*)_idlCtrlName.c_str(), IDLCMD, 0, (char*)idlCmd.c_str());
   if (IS_ERROR(seqn)) {
		_logger->log(Logger::LOG_LEV_ERROR, "Error sending IDL message: %d (%s) [%s:%d]",
				seqn, lao_strerror(seqn), __FILE__, __LINE__ );
      	throw IdlCtrlInterfaceException("Failed to send command to IDL", seqn, __FILE__, __LINE__);
   	}

   // --- Wait for the reply
   // idl_answer answ;
   MsgBuf *msgb = thWaitMsg( IDLREPLY, (char *)_idlCtrlName.c_str(), seqn, 0, &stat);
   IdlCommandReply reply(msgb);
//   if (msgb) {
//      PrepareIDLAnswer(&answ);
//      GetIDLAnswer( msgb, &answ);
//      if (answStr) {
//        *answStr = answ.output;
//         *errStr  = answ.error;
//      }
//   FreeIDLAnswer(&answ);
//   }
     thRelease(msgb);
     return reply;

#endif

}

void IdlCtrlInterface::sendCtrlC() throw (IdlCtrlInterfaceException) 
{
   
#ifdef USE_RPC 
    _logger->log(Logger::LOG_LEV_INFO, "Sending Ctrl-C Command to IDLRPC [%s:%d]",
			 __FILE__, __LINE__);

   int seqNum =0;  // TODO
	
	// Send FAILURE
	if (IS_ERROR(seqNum)) {
      	int stat = PLAIN_ERROR(seqNum);
		_logger->log(Logger::LOG_LEV_ERROR, "Error sending Ctrl-C signal: %d (%s) [%s:%d]", 
				stat, lao_strerror(stat), __FILE__, __LINE__);
      	throw IdlCtrlInterfaceException("Failed to send Ctrl-C command");
   	}
#else

	int stat = thSendMsg(0, (char*)_idlCtrlName.c_str(), IDLCTRLC, DISCARD_FLAG, NULL);
   if (IS_ERROR(stat)) {
		_logger->log(Logger::LOG_LEV_ERROR, "Error sending Ctrl-C signal: %d (%s) [%s:%d]", 
				stat, lao_strerror(stat), __FILE__, __LINE__);
      	throw IdlCtrlInterfaceException("Failed to send Ctrl-C command");
   	}
#endif

}
	
int IdlCtrlInterface::getIntVariable(string idlVariable, int timeout_sec) throw(IdlCtrlInterfaceException) {

#ifndef USE_RPC
   return -1;
#else
    
	// set timeout
    if(!IDL_RPCTimeout(timeout_sec)){ 
		_logger->log(Logger::LOG_LEV_ERROR, "Unable to set timeout to %d sec [%s:%d]",
	       timeout_sec, __FILE__, __LINE__);
    }

    // Get idl variable
    IDL_VPTR vTmp = IDL_RPCGetMainVariable(_pClient, (char *)idlVariable.c_str()); 
    if (!vTmp) {
        _logger->log(Logger::LOG_LEV_ERROR, "IDL_RPCGetMainVariable (%s) failed [%s:%d]",
	                (char *)idlVariable.c_str(),  __FILE__, __LINE__);
      	throw IdlCtrlInterfaceException("IDL_RPC GetMainVariable failed");
    }
    
    int ret;
    switch IDL_RPCGetVarType(vTmp) {
        case IDL_TYP_BYTE:
            ret = (int) IDL_RPCGetVarByte(vTmp);
            break;
        case IDL_TYP_INT:
            ret = (int) IDL_RPCGetVarInt(vTmp);
            break;
        case IDL_TYP_LONG:
            ret = (int) IDL_RPCGetVarLong(vTmp);
            break;
        default:
            IDL_RPCDeltmp(vTmp);
		    _logger->log(Logger::LOG_LEV_ERROR, "IDL variable %s is of type %d, that is not integer [%s:%d]",
	                (char *)idlVariable.c_str(), IDL_RPCGetVarType(vTmp),  __FILE__, __LINE__);
      	    throw IdlCtrlInterfaceException("IDL variable is of wrong type");
    }
    IDL_RPCDeltmp(vTmp);

    return ret;

#endif
}


//void IdlCtrlInterface::sendLock() {
//	
//}
		 
//void IdlCtrlInterface::sendUnlock() {
//
//}

/************+*************** IDLCOMMANDREPLY ********************************/

IdlCommandReply::IdlCommandReply(MsgBuf* msgb) {
   if (msgb) {
	   char* msgBody = (char*)MSG_BODY(msgb);
	
	   _idlOut = string(msgBody); 				// Stops at the first null termination
	   _idlErr = string(strchr(msgBody, 0)+1);	// Begins after the first null termination 
   } else {
      _idlOut = "";
      _idlErr = "Error receving IDL answer";
   }
}

