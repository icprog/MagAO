#include <iostream>
#include <sstream>
#include <iomanip>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string>
#include <arpa/inet.h>


extern "C"{
	#include "idl_export.h"
	#include "base/errordb.h"
	#include "base/common.h"
	#include "base/thrdlib.h"
   #include "idllib/idllib.h"
   
}

#include "AdSecConstants.h"
#include "Logger.h"
#include "AdamLib.h"
#include "UdpConnection.h"
#include "BcuLib/BcuCommon.h"
#include "stdconfig.h"
#include "Utils.h"

#include "idl_wraplib.h"
#include "idl_diagnlib.h"
#include "idl_hexlib.h"
#include "idl_4dlib.h"

using namespace std;
using namespace Arcetri;
using namespace Arcetri::Bcu;
using namespace AdSecConstants;


//string _clientName;
//string _serverIp;

// Global logger
static Logger* _logger;

// Global shared memory variables
BufInfo *Info = NULL;
int info_size = 0;


// ********************************************************************************************************************* //
// ************************************************ DLM SUPPORT SECTION *************************************************** //
// See DLM INIT section at the and of this file

// Define message codes and their corresponding printf(3) format
// strings. Note that message codes start at zero and each one is
// one less that the previous one. Codes must be monotonic and
// contiguous.
static IDL_MSG_DEF msg_arr[] = {
	#define M_TM_INPRO 0 {  "M_TM_INPRO",   "%NThis is from a loadable module procedure." },
	#define M_TM_INFUN   {  "M_TM_INFUN",   "%NThis is from a loadable module function." },
};

// The load function fills in this message block handle with the
// opaque handle to the message block used for this module. The other
// routines can then use it to throw errors from this block.
static IDL_MSG_BLOCK msg_block;

void idl_free_cb(unsigned char* loc) {
    Logger::get()->log(Logger::LOG_LEV_TRACE, "IDL freeing memory starting at %p",(void*)loc);
    free(loc);
}


// ********************************************************************************************************************* //
// *********************************************** IDL INIT SECTION *************************************************** //

bool 		_thLib_initialized = false;
int 		_idl_connected = 0;
Config_File _adsecConfig;

IDL_VPTR idl_setup_wrap(int lArgc, IDL_VPTR Argv[])
{
	string configFile;

	string clientName;
	string serverIp;
	string logLevelAsString;
	int logLevel;

	int retryNum = 5;

	int ErrC = 0;

    // configuration file required
    if (lArgc != 1) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

    configFile = IDL_VarGetString(Argv[0]);

    try {
	    _logger->log(Logger::LOG_LEV_INFO, "idl_setup_wrap: reading config file %s...", configFile.c_str());
	    _adsecConfig = Config_File(configFile);
    }
    catch(Config_File_Exception& e) {
    	_logger->log(Logger::LOG_LEV_ERROR, "idl_setup_wrap: %s", e.what().c_str());
    	return IDL_GettmpLong(IDL_INVALID_FILE);
    }

    // Retrieve useful values form config file
    string copy = Utils::deepCopy(configFile.c_str());
    clientName = basename((char*)copy.c_str());
    if (clientName.substr( clientName.size()-5, 5) == ".conf")
       clientName = clientName.substr(0, clientName.size()-5);

     serverIp = (string)_adsecConfig["Server"];
     logLevelAsString = (string)_adsecConfig["LogLevel"];
     logLevel = Logger::stringToLevel(logLevelAsString);

    _logger->log(Logger::LOG_LEV_INFO, "idl_setup_wrap: client name is %s", clientName.c_str());
    _logger->log(Logger::LOG_LEV_INFO, "idl_setup_wrap: server is %s", serverIp.c_str());
    _logger->setLevel(logLevel);
    _logger->printStatus();
    
    // Used to init thrdlib and as logfile name
    string myFullName = clientName + "." + Utils::getAdoptSide();

	// Set the correct log file name
    Logger::setParentName(myFullName);
	char* logPath = getenv("ADOPT_LOG");
	if(logPath == NULL) {
		logPath = (char*)"/tmp";
	}
        _logger->rename( myFullName, string(logPath));

    // Initialize thrdlib
    if(!_thLib_initialized) {
	    ErrC = thInit((char*)myFullName.c_str());
	    if (IS_ERROR(ErrC)){
	        _logger->log(Logger::LOG_LEV_WARNING, "idl_setup_wrap: thInit failed: %s (errcode %d)",lao_strerror(ErrC), ErrC);
	        return IDL_GettmpLong(ErrC);
	    }
	    else {
	    	_thLib_initialized = true;
	    	_logger->log(Logger::LOG_LEV_DEBUG, "idl_setup_wrap: thInit succesfully completed");
	    }
	}

    ErrC = thStart((char*)serverIp.c_str(), retryNum);
    if (IS_ERROR(ErrC)){
        _logger->log(Logger::LOG_LEV_WARNING, "idl_setup_wrap: thStart failed: %s (errcode %d)",lao_strerror(ErrC), ErrC);
        return IDL_GettmpLong(ErrC);
    }
    _logger->log(Logger::LOG_LEV_DEBUG, "idl_setup_wrap: thStart succesfully completed");

    _idl_connected=1;
    _logger->log(Logger::LOG_LEV_INFO, "idl_setup_wrap: CONNECTED TO MSGD AS %s", clientName.c_str());

    return IDL_GettmpLong(NO_ERROR);
}


IDL_VPTR idl_close_wrap(int /*lArgc*/, IDL_VPTR /*lArgv[]*/)
{
    int ErrC = 0;
    ErrC = thClose();
   	if (IS_ERROR(ErrC)) {
   		_logger->log(Logger::LOG_LEV_WARNING, "idl_setup_wrap: thClose failed: %s (errcode %d)",lao_strerror(ErrC), ErrC);
        return IDL_GettmpLong(ErrC);
    }
    _logger->log(Logger::LOG_LEV_DEBUG, "idl_setup_wrap: thClose succesfully completed");

	ErrC = thCleanup();
   	if (IS_ERROR(ErrC)) {
   		_logger->log(Logger::LOG_LEV_WARNING, "idl_setup_wrap: thCleanup failed: %s (errcode %d)",lao_strerror(ErrC), ErrC);
        return IDL_GettmpLong(ErrC);
    }
    _logger->log(Logger::LOG_LEV_DEBUG, "idl_setup_wrap: thCleanup succesfully completed");

    //Detach from shared memory buffers
    if ((Info != NULL) && (info_size>0))
    {
        free(Info);
        Info = NULL;
        info_size=0;
    }

    _thLib_initialized = false;
    _idl_connected = 0;
     _logger->log(Logger::LOG_LEV_INFO, "idl_setup_wrap: DISCONNECTED FROM MSGD AS %s", ((string)_adsecConfig["MyName"]).c_str());

    return IDL_GettmpLong(NO_ERROR);
}
//---------------------------------------------------------------------------------------------------------
IDL_STRUCT_TAG_DEF value_tags[] = {
          { (char*)"LOG_LEV_ALWAYS", 0,    (void *) IDL_TYP_LONG, 0},
          { (char*)"LOG_LEV_FATAL", 0,     (void *) IDL_TYP_LONG, 0},
          { (char*)"LOG_LEV_ERROR", 0,     (void *) IDL_TYP_LONG, 0},
          { (char*)"LOG_LEV_WARNING", 0,   (void *) IDL_TYP_LONG, 0},
          { (char*)"LOG_LEV_INFO", 0,      (void *) IDL_TYP_LONG, 0},
          { (char*)"LOG_LEV_DEBUG", 0,     (void *) IDL_TYP_LONG, 0},
          { (char*)"LOG_LEV_TRACE", 0,     (void *) IDL_TYP_LONG, 0},
          { (char*)"LOG_LEV_DISABLE", 0,  (void *) IDL_TYP_LONG, 0},
          { 0, 0, NULL, 0 }
         };


typedef struct value_struct {
           IDL_LONG always;
           IDL_LONG fatal;
           IDL_LONG error;
           IDL_LONG warning;
           IDL_LONG info;
           IDL_LONG debug;
           IDL_LONG trace;
           IDL_LONG disabled;
} IDL_Value;


//-------------------------------------------------------------------------------------------------------

IDL_VPTR idl_log_wrap(int lArgc, IDL_VPTR Argv[])
{
   switch (lArgc)
    {
      case 2: {

       _logger->log(IDL_LongScalar(Argv[0]), "%s", IDL_VarGetString(Argv[1]));
       return IDL_GettmpLong(NO_ERROR);

      }
      case 1: {
       _logger->log(Logger::LOG_LEV_INFO, "%s", IDL_VarGetString(Argv[0]));
       return IDL_GettmpLong(NO_ERROR);

      }
      case 0: {

         //IDL_VPTR varvalueOut; 	// Output parameter
         IDL_MEMINT  varSize = 1;
         IDL_Value *s_value;
         int varType = IDL_TYP_STRUCT;

         // create the structure definition
         void *p = IDL_MakeStruct(0, value_tags);
         s_value   = (IDL_Value*)malloc(sizeof(IDL_Value));

         s_value->always=Logger::LOG_LEV_ALWAYS;
         s_value->fatal=Logger::LOG_LEV_FATAL;
         s_value->error=Logger::LOG_LEV_ERROR;
         s_value->warning=Logger::LOG_LEV_WARNING;
         s_value->info=Logger::LOG_LEV_INFO;
         s_value->debug=Logger::LOG_LEV_DEBUG;
         s_value->trace=Logger::LOG_LEV_TRACE;
         s_value->disabled=Logger::LOG_LEV_DISABLED;
         IDL_VPTR v_pa = IDL_ImportArray(1, &varSize, varType, (UCHAR *) s_value, idl_free_cb, (_idl_structure*)p);

       return v_pa;

      }
    }
    return IDL_GettmpLong(IDL_PAR_NUM_ERROR);
}


// ********************************************************************************************************************* //
// ******************************************** GENERAL INFO ************************************************ //

IDL_VPTR idl_getadoptside(int /* lArgc */, IDL_VPTR /* lArgv[] */) {
    string side = Utils::getAdoptSide();
    return IDL_StrToSTRING((char*)side.c_str());
}

IDL_VPTR idl_getadoptsubsystem(int /* lArgc */, IDL_VPTR /* lArgv[] */) {
    string subsystem = Utils::getAdoptSubsystem();
    return IDL_StrToSTRING((char*)subsystem.c_str());
}

IDL_VPTR idl_getadoptlog(int /* lArgc */, IDL_VPTR /* lArgv[] */) {
    string log = Utils::getAdoptLog();
    return IDL_StrToSTRING((char*)log.c_str());
}



// ********************************************************************************************************************* //
// ******************************************** BCU INTERNAL USE SECTION ************************************************ //

CommandSender*  _comSender = NULL;

IDL_INT sendMultiBcuCommand(int priority, int firstBcu, int lastBcu,
							int firstDsp, int lastDsp, 	int opcode,
							int address, int datalenBytes, unsigned char *data,
							int timeout_ms, int flags) {

	static BcuIdMap 	   _bcuIdMap;

	_logger->log(Logger::LOG_LEV_DEBUG, "Addressing BCU %d-%d", firstBcu, lastBcu);

	try {
		// Is able to address:
		// 	- BcuSwitch stand alone
		//	- A subset of contiguous BcuMirror
		if(!((firstBcu == lastBcu && firstBcu == _bcuIdMap["BCU_SWITCH"]) ||
		   (firstBcu >= _bcuIdMap["BCU_MIRROR_0"] && lastBcu <= _bcuIdMap["BCU_MIRROR_5"] && firstBcu <= lastBcu))) {
		    _logger->log(Logger::LOG_LEV_ERROR, "Impossible to address BCU %d-%d", firstBcu, lastBcu);
            return PLAIN_ERROR(VALUE_OUT_OF_RANGE_ERROR);
        }
	}
	catch (BcuIdMapException &e){
		return PLAIN_ERROR(VALUE_OUT_OF_RANGE_ERROR);
	}

	int ErrC;
    try {
		if(_comSender == NULL) {
     		_comSender = new CommandSender();
     	}

     	// --- Particular case: addressing SIGGEN --- //
     	if(firstDsp == BcuMirror::SIGGEN_BOARD_ID) {
     		_logger->log(Logger::LOG_LEV_WARNING, "SIGGEN request found: calling multiple sendMultiBcuCommand...");
     	   	// Split the multi bcu command in (lastBcu-firstBcu+1) bcu commands.
     	   	// Compute the new dataLen only if is a SEQ WRITE/READ operation
     	   	int subDataLenBytes = datalenBytes;	// By default consider a SAME operation
     	   	if(OpCodes::isSeq(opcode)) {
     	   		subDataLenBytes = datalenBytes/(lastBcu-firstBcu+1); // Fix for SEQ operation
     	   	}
     	   	int sysFirstCrate = _bcuIdMap["BCU_MIRROR_0"];
     		for(int crate = firstBcu; crate<=lastBcu; crate++) {
     			int sigGenDsp = BcuMirror::SIGGEN_BOARD_ID; // Default value
     			// Check if is a crate with 'alternative' siggen id
     			_logger->log(Logger::LOG_LEV_TRACE, "Checking alternative SIGGEN... (crate = %d, firstSysCrate = %d, ACCELER_CRATE_NEXT = %d, ACCELER_CRATE_FIRST = %d)", crate, sysFirstCrate, BcuMirror::ACCELER_CRATE_NEXT, BcuMirror::ACCELER_CRATE_FIRST);
                if((crate-sysFirstCrate) % BcuMirror::ACCELER_CRATE_NEXT == BcuMirror::ACCELER_CRATE_FIRST) {
                	sigGenDsp = BcuMirror::SIGGEN_BOARD_ID_ALT;
                }
     		    // Send the single-crate bcu command
     			_comSender->sendMultiBcuCommand(priority, crate, crate, sigGenDsp, sigGenDsp,
	     							   	        opcode, address, subDataLenBytes,
	     							   		    &(data[(crate-firstBcu)*subDataLenBytes]), timeout_ms, flags);
     		}
     	}
     	// --- Normal case ---//
     	else {
     		_logger->log(Logger::LOG_LEV_DEBUG, "Calling sendMultiBcuCommand...");
	     	_comSender->sendMultiBcuCommand(priority, firstBcu, lastBcu, firstDsp, lastDsp,
	     							   	    opcode, address, datalenBytes,
	     							   		data, timeout_ms, flags);
     	}
     	ErrC = NO_ERROR;
     }
     catch(CommandSenderTimeoutException& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "%s", e.what().c_str());
		delete _comSender;	// !!! see BcuCommon.h->Bcu::CommandSender::sendMultiBcuCommand(...) !!!
		_comSender = NULL;
		ErrC = TIMEOUT_ERROR;
	}
	catch(CommandSenderException& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "%s", e.what().c_str());
		ErrC = COMMUNICATION_ERROR;
	}

	return PLAIN_ERROR(ErrC);
}

bool isIdlArray(IDL_VPTR vptr) {
	_logger->log(Logger::LOG_LEV_TRACE, "IDL_VPTR flags: %d (IDL_ARRAY = %d) ", vptr->flags, IDL_V_ARR);
	if((vptr->flags & IDL_V_ARR) == 0) {
		return false;
	}
	else {
		return true;
	}
}

void dumpCommand(int opcode, IDL_INT firstBCU, IDL_INT lastBCU, IDL_INT firstDSP, IDL_INT lastDSP,
				  IDL_LONG dspAddress, IDL_LONG dataLength, IDL_LONG time,
				  int flags, UCHAR* /*dataBuffer*/) {

	_logger->log(Logger::LOG_LEV_TRACE, "> OpCode  : %d", opcode);
    _logger->log(Logger::LOG_LEV_TRACE, "> FirtsBcu : %d", firstBCU);
    _logger->log(Logger::LOG_LEV_TRACE, "> LastBcu : %d", lastBCU);
    _logger->log(Logger::LOG_LEV_TRACE, "> FirstDsp: %d", firstDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "> LastDsp : %d", lastDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "> DspAddress : %ld", dspAddress);
    _logger->log(Logger::LOG_LEV_TRACE, "> DataLength : %ld", dataLength);
    _logger->log(Logger::LOG_LEV_TRACE, "> Timeout : %ld", time);
    _logger->log(Logger::LOG_LEV_TRACE, "> Flags: %d", flags);
}


// ********************************************************************************************************************* //
// ******************************************** SHM INTERNAL USE SECTION ************************************************ //

// Function to check if we are already attached to a buffer.
// Returns a pointer to the buffer Info struct or NULL if we are
// not attached
BufInfo* is_attached(char *bufname)
{
    char *pos, *onlyname;
    int i;

    pos = strchr(bufname, ':');
    *pos = 0;   // Zero-terminate after the producer
    onlyname = pos+1; // points to the buffer name

    if ((info_size>0) && (Info != NULL))
        for (i=0; i< info_size; i++)
            {
            _logger->log(Logger::LOG_LEV_TRACE, "name: %s - onlyname: %s - prod: %s - bufname; %s",
                    Info[i].com.name, onlyname, Info[i].com.producer, bufname);
            if (strncmp( Info[i].com.name, onlyname, VAR_NAME_LEN) == 0)
                if (strncmp( Info[i].com.producer, bufname, PROC_NAME_LEN) ==0)
                    {
                    *pos = ':';    // Restore the corret name
                    return Info+i;
                    }
            }

    *pos = ':';    // Restore the corret name
    return NULL;
}


// Function to resize the Info vector destroying the 'bufname' element
void resize_Info(char *bufname)
{
    char *pos, *onlyname;
    int i;

    pos = strchr(bufname, ':');
    *pos = 0;   // Zero-terminate after the producer
    onlyname = pos+1; // points to the buffer name
    _logger->log(Logger::LOG_LEV_TRACE, "Resizing buffer queue...");

    if ((info_size>0) && (Info != NULL))
        for (i=0; i< info_size; i++)
            {
                _logger->log(Logger::LOG_LEV_TRACE, "name: %s - onlyname: %s - prod: %s - bufname; %s",
                    Info[i].com.name, onlyname, Info[i].com.producer, bufname);
            if (strncmp( Info[i].com.name, onlyname, VAR_NAME_LEN) == 0)
                if (strncmp( Info[i].com.producer, bufname, PROC_NAME_LEN) ==0)
                    {
                    *pos = ':';    // Restore the corret name
                    memmove(Info+i,Info+i+1,(info_size-i-1)*sizeof(BufInfo));
                    info_size--;
                    _logger->log(Logger::LOG_LEV_TRACE, "Resized Info array, deleted elements %d, new info_size %d", i, info_size);
                    return;
                    }
            }

    _logger->log(Logger::LOG_LEV_TRACE, "Buffer not found...");
    *pos = ':';    // Restore the corret name
    return;
}

// Function to attach to a new shared buffer
int idl_attach( char *bufname, int type, BufInfo **extern_info)
{
	char MyName[PROC_NAME_LEN];
    BufInfo *local_info;

   // Allocate a new vector element
   if (!Info)
        {
        Info = (BufInfo *) malloc( sizeof(BufInfo));
        info_size =1;
        }
   else
        {
        info_size++;
        Info = (BufInfo *) realloc( Info, sizeof(BufInfo) * info_size);
        }

    if (!Info)
        {
            _logger->log(Logger::LOG_LEV_ERROR, "Malloc error");
        info_size=0;
        return MEMORY_ALLOC_ERROR;
        }

    local_info = &(Info[info_size-1]);

    _logger->log(Logger::LOG_LEV_INFO, "Attacching to %s as %s...", bufname, MyName);
    int ErrC = bufRequest(MyName, bufname, type, local_info);
    _logger->log(Logger::LOG_LEV_TRACE, "shm name: %s, producer: %s", Info[info_size-1].com.name, Info[info_size-1].com.producer );
    if (PLAIN_ERROR(ErrC) != 0) {
        info_size--;
        Info = (BufInfo *) realloc( Info, sizeof(BufInfo) * info_size);
        _logger->log(Logger::LOG_LEV_WARNING, "BufRequest failed (errno %d), realloc to infosize: %d", errno, info_size);
        return PLAIN_ERROR(ErrC);
    }
    _logger->log(Logger::LOG_LEV_TRACE, "BufRequest returned %s (error code %d)", lao_strerror(ErrC), ErrC);
    _logger->log(Logger::LOG_LEV_TRACE, "Data readable length: %d", local_info->com.lng);
    _logger->log(Logger::LOG_LEV_TRACE, "Attacched. Info_size: %d", info_size);

    if (extern_info)
        *extern_info = local_info;

    return NO_ERROR;
}



// ********************************************************************************************************************* //
// ********************************* UDPCONNECTION ***PRIVATE*** LIB WRAPPER SECTION *********************************** //
// Used by "Generic UdpConnection" wrapper and "Adam UdpConnection" wrapper

UdpConnection* udpConnectionInit(int lArgc, IDL_VPTR lArgv[]) {

	// Check if the number of argument is correct
    if (lArgc != 4) {
    	_logger->log(Logger::LOG_LEV_ERROR, "udpConnectionInit: wrong parameters number (4 required, %d received)", lArgc);
    	return NULL;
    }

	// Get parameters
	int localPort 		  = IDL_LongScalar(lArgv[0]);
	string remoteIp 	  = IDL_VarGetString(lArgv[1]);
	int remotePort 		  = IDL_LongScalar(lArgv[2]);
	int receiveTimeout_ms = IDL_LongScalar(lArgv[3]);

	try {
		UdpConnection* udpConn = new UdpConnection(localPort, remoteIp, remotePort, receiveTimeout_ms);
		_logger->log(Logger::LOG_LEV_INFO, "udpConnectionInit: UDP connection succesfully created");
		return udpConn;
	}
	catch(UdpFatalException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, "udpConnectionInit: %s", e.what().c_str());
		return NULL;
	}
}

void udpConnectionDestroy(UdpConnection* udpConn) {
	delete udpConn;
	_logger->log(Logger::LOG_LEV_INFO, "udpConnectionDestroy: UDP connection succesfully destroyed");
}

IDL_VPTR udpConnectionSend(UdpConnection* udpConn, int lArgc, IDL_VPTR lArgv[]) {

	// Check if the number of argument is correct
    if (lArgc != 2) {
    	_logger->log(Logger::LOG_LEV_ERROR, "udpConnectionSend: wrong parameters number (2 required, %d received)", lArgc);
    	return IDL_GettmpLong(IDL_PAR_NUM_ERROR);
    }

    // Check if the UdpConnection is initialized
	if(udpConn != NULL) {

		 BYTE* buffer = (BYTE*)IDL_VarGetString(lArgv[0]);
		 int bufferLen = IDL_LongScalar(lArgv[1]);
		 try {
			udpConn->send((BYTE*)buffer, bufferLen);
			_logger->log(Logger::LOG_LEV_DEBUG, "udpConnectionSend: buffer succesfully sent %s", buffer);
			return IDL_GettmpLong(NO_ERROR);
		 }
		 catch(UdpFatalException& e) {
			_logger->log(Logger::LOG_LEV_ERROR, "udpConnectionSend: %s", e.what().c_str());
			return IDL_GettmpLong(IDL_GENERIC_ERROR);
		}
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "udpConnectionSend: uninitialized UdpConnection");
		return IDL_GettmpLong(IDL_GENERIC_ERROR);
	}
}

IDL_VPTR udpConnectionReceive(UdpConnection* udpConn, int lArgc, IDL_VPTR lArgv[]) {
	// Check if the number of argument is correct
    if (lArgc != 1) {
    	_logger->log(Logger::LOG_LEV_ERROR, "udpConnectionReceive: wrong parameters number (1 required, %d received)", lArgc);
    	return IDL_StrToSTRING((char*)"");
    }

    // Check if the UdpConnection is initialized
	if(udpConn != NULL) {

		 int bufferLen = IDL_LongScalar(lArgv[0]);
		 BYTE buffer[bufferLen+1];	// Add null terminator
		 memset(&buffer, '\0', bufferLen+1);
		 try {
			udpConn->receive((BYTE*)(&buffer), bufferLen);
			_logger->log(Logger::LOG_LEV_DEBUG, "udpConnectionReceive: buffer succesfully received %s", &buffer);
			return IDL_StrToSTRING((char*)buffer);
		 }
		 catch(UdpTimeoutException& e) {
			_logger->log(Logger::LOG_LEV_ERROR, "udpConnectionReceive: %s", e.what().c_str());
			return IDL_StrToSTRING((char*)"");
		 }
		 catch(UdpFatalException& e) {
			_logger->log(Logger::LOG_LEV_ERROR, "udpConnectionReceive: %s", e.what().c_str());
			return IDL_StrToSTRING((char*)"");
		 }

	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "udpConnectionReceive: uninitialized UdpConnection");
		return IDL_StrToSTRING((char*)"");
	}
}


// ************************************************************************************************************* //
// ********************************** GENERIC UDPCONNECTION LIB SECTION **************************************** //


UdpConnection* _udpConn = NULL;


IDL_VPTR udpConnectionInit_wrap(int lArgc, IDL_VPTR lArgv[]) {
	_udpConn = udpConnectionInit(lArgc, lArgv);
	if(_udpConn != NULL) {
		return IDL_GettmpLong(NO_ERROR);
	}
	else {
		return IDL_GettmpLong(IDL_GENERIC_ERROR);
	}
}


IDL_VPTR udpConnectionDestroy_wrap() {
	udpConnectionDestroy(_udpConn);
	_udpConn = NULL;
	return IDL_GettmpLong(NO_ERROR);
}


IDL_VPTR udpConnectionSend_wrap(int lArgc, IDL_VPTR lArgv[]) {
	return udpConnectionSend(_udpConn, lArgc, lArgv);
}

IDL_VPTR udpConnectionReceive_wrap(int lArgc, IDL_VPTR lArgv[]) {
	return udpConnectionReceive(_udpConn, lArgc, lArgv);
}


// ********************************************************************************************************************* //
// ******************************************** ADAM COMMUNICATION LIBRARY ********************************************* //

Adam* _adam;
AdamModbus* _adamModbus;
//Adam* _adam2;

IDL_VPTR adamConnect_wrap(int Argc, IDL_VPTR Argv[], char *Argk) {
    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        IDL_STRING adam_name;
	int adam_name_there;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"ADAM_NAME"  , IDL_TYP_STRING, 1, 0, (int*)IDL_KW_OFFSETOF(adam_name_there), (char*)IDL_KW_OFFSETOF(adam_name) },
        { NULL, 0, 0, 0, NULL, NULL }
    };
    KW_RESULT           kw;
    string		adam_name;
    
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw);
    adam_name = kw.adam_name_there ? IDL_STRING_STR(&kw.adam_name) : "";
    IDL_KW_FREE;
    
    //Adam* adam = _adam; 
    //if (adam_name.compare("") == 0)  adam = _adam;
    //if (adam_name.compare("2") == 0) adam = _adam2;

    if(_adam == NULL) {
	try {
	    _adam = new Adam(); //, adam_name);
	}
	catch(AOException& e) {
	    _logger->log(Logger::LOG_LEV_ERROR, "adamConnect_wrap: %s", e.what().c_str());
	    return IDL_GettmpLong(IDL_GENERIC_ERROR);
	}
	_logger->log(Logger::LOG_LEV_INFO, "adamConnect_wrap: adam succesfully connected");
    }
    else {
	_logger->log(Logger::LOG_LEV_INFO, "adamConnect_wrap: adam was already connected... ok!");
    }
    return IDL_GettmpLong(NO_ERROR);
}
IDL_VPTR adamModbusInit_wrap(int Argc, IDL_VPTR Argv[], char *Argk) {
    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        IDL_STRING adam_name;
	int adam_name_there;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"ADAM_NAME"  , IDL_TYP_STRING, 1, 0, (int*)IDL_KW_OFFSETOF(adam_name_there), (char*)IDL_KW_OFFSETOF(adam_name) },
        { NULL, 0, 0, 0, NULL, NULL }
    };
    KW_RESULT           kw;
    string		adam_name;
    
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw);
    adam_name = kw.adam_name_there ? IDL_STRING_STR(&kw.adam_name) : "";
    IDL_KW_FREE;
    
    //Adam* adam = _adam; 
    //if (adam_name.compare("") == 0)  adam = _adam;
    //if (adam_name.compare("2") == 0) adam = _adam2;

    if(_adamModbus == NULL) {
	try {
	    _adamModbus = new AdamModbus("ADAM-MODBUS", 0); //, adam_name);
	}
	catch(AOException& e) {
	    _logger->log(Logger::LOG_LEV_ERROR, "adamModbusInit_wrap: %s", e.what().c_str());
	    return IDL_GettmpLong(IDL_GENERIC_ERROR);
	}
	_logger->log(Logger::LOG_LEV_INFO, "adamModbusInit_wrap: adam succesfully connected");
    }
    else {
	_logger->log(Logger::LOG_LEV_INFO, "adamModbusInit_wrap: adam was already connected... ok!");
    }
    return IDL_GettmpLong(NO_ERROR);
}


IDL_VPTR adamDisconnect_wrap(int Argc, IDL_VPTR Argv[], char *Argk) {
    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        IDL_STRING adam_name;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"ADAM_NAME"  , IDL_TYP_STRING, 1, 0, 0, (char*)IDL_KW_OFFSETOF(adam_name) },
        { NULL, 0, 0, 0, NULL, NULL }
    };
    KW_RESULT           kw;
    string		adam_name;
    
    IDL_StrStore(&kw.adam_name,(char *)"");
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw);
    adam_name = IDL_STRING_STR(&kw.adam_name);
    IDL_KW_FREE;
    
    //Adam* adam; 
    //if (adam_name.compare("") == 0)  adam = _adam;
    //if (adam_name.compare("2") == 0) adam = _adam2;


    delete _adam;
    _adam = NULL;
    _logger->log(Logger::LOG_LEV_INFO, "adamConnect_wrap: adam succesfully disconnected");
    return IDL_GettmpLong(NO_ERROR);
}
IDL_VPTR adamModbusDisconnect_wrap(int Argc, IDL_VPTR Argv[], char *Argk) {
    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        IDL_STRING adam_name;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"ADAM_NAME"  , IDL_TYP_STRING, 1, 0, 0, (char*)IDL_KW_OFFSETOF(adam_name) },
        { NULL, 0, 0, 0, NULL, NULL }
    };
    KW_RESULT           kw;
    string		adam_name;
    
    IDL_StrStore(&kw.adam_name,(char *)"");
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw);
    adam_name = IDL_STRING_STR(&kw.adam_name);
    IDL_KW_FREE;
    
    //Adam* adam; 
    //if (adam_name.compare("") == 0)  adam = _adam;
    //if (adam_name.compare("2") == 0) adam = _adam2;

    if (_adamModbus != NULL) {
       delete _adamModbus;
       _adamModbus = NULL;
    }
    _logger->log(Logger::LOG_LEV_INFO, "adamConnect_wrap: adam succesfully disconnected");
    return IDL_GettmpLong(NO_ERROR);
}




IDL_VPTR adamCmd_wrap(int lArgc, IDL_VPTR lArgv[], char *Argk) {
    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        IDL_STRING adam_name;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"ADAM_NAME"  , IDL_TYP_STRING, 1, 0, 0, (char*)IDL_KW_OFFSETOF(adam_name) },
        { NULL, 0, 0, 0, NULL, NULL }
    };
    KW_RESULT           kw;
    string		adam_name;
    
    IDL_StrStore(&kw.adam_name,(char *)"");
    (void) IDL_KWProcessByOffset(lArgc, lArgv, Argk, kw_pars, NULL, 1, &kw);
    adam_name = IDL_STRING_STR(&kw.adam_name);
    IDL_KW_FREE;
    
    //Adam* _adam; 
    //if (adam_name.compare("") == 0)  adam = _adam;
    //if (adam_name.compare("2") == 0) adam = _adam2;

    UdpConnection* adamConn = _adam->getUdpConnection();

    // Check if the number of argument is correct: cmd, cmdLen, ansLen
    if (lArgc != 3) {
    	_logger->log(Logger::LOG_LEV_ERROR, "adamCmd_wrap: wrong parameters number (3 required, %d received)", lArgc);
    	return IDL_StrToSTRING((char*)"");
    }

    IDL_VPTR sendArgv[2];
    sendArgv[0] = lArgv[0];
    sendArgv[1] = lArgv[1];
    int ret = IDL_LongScalar(udpConnectionSend(adamConn, 2, sendArgv));

    if (IS_ERROR(ret))
    	_logger->log(Logger::LOG_LEV_ERROR, "adamCmd_wrap: udpConnectionSend error (%d) %s", ret, lao_strerror(ret));
    else
      _logger->log(Logger::LOG_LEV_DEBUG, "adamCmd_wrap: sent '%s', ret=%d", IDL_VarGetString(sendArgv[0]), ret);

    IDL_VPTR receiveArgv[1];
    receiveArgv[0] = lArgv[2];
    IDL_VPTR s = udpConnectionReceive(adamConn, 1, receiveArgv);
    _logger->log(Logger::LOG_LEV_DEBUG, "adamCmd_wrap: received '%s'", IDL_VarGetString(s));
    return s;
}

IDL_VPTR adamModbusReadInput_wrap(int lArgc, IDL_VPTR lArgv[], char *Argk) {
    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        IDL_STRING adam_name;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"ADAM_NAME"  , IDL_TYP_STRING, 1, 0, 0, (char*)IDL_KW_OFFSETOF(adam_name) },
        { NULL, 0, 0, 0, NULL, NULL }
    };
    KW_RESULT           kw;
    string		adam_name;
    
    IDL_StrStore(&kw.adam_name,(char *)"");
    (void) IDL_KWProcessByOffset(lArgc, lArgv, Argk, kw_pars, NULL, 1, &kw);
    adam_name = IDL_STRING_STR(&kw.adam_name);
    IDL_KW_FREE;
    
    if(_adamModbus == NULL)
          return IDL_GettmpLong(ADAM_INIT_ERROR);

    int bitmask=0;
    if (!(_adamModbus->ReadInputStatus(1, 0, 8, &bitmask))) {
        _logger->log(Logger::LOG_LEV_DEBUG, "adamModbusReadInput_wrap found TCP connection not initialized ...");
        bitmask=0;
        try {
	        _logger->log(Logger::LOG_LEV_DEBUG, "adamModbusReadInput_wrap is starting TCP...");
            _adamModbus->clearTcp();
            _adamModbus->initTcp();
        } catch(AOException& e) {
	        _logger->log(Logger::LOG_LEV_ERROR, "adamModbusReadInput_wrap: %s", e.what().c_str());
   	        return IDL_GettmpLong(e._errcode);
   	    }
        if (!(_adamModbus->ReadInputStatus(1, 0, 8, &bitmask))) {
	        _logger->log(Logger::LOG_LEV_ERROR, "adamModbusReadInput_wrap tried to read again but failed.");
            return IDL_GettmpLong(TCP_GENERIC_ERROR);
        } else {
	        _logger->log(Logger::LOG_LEV_DEBUG, "adamModbusReadInput_wrap read at second shot.");
            return IDL_GettmpLong(bitmask & 0xFF);
        }
       
    } else {
         _logger->log(Logger::LOG_LEV_DEBUG, "adamModbusReadInput_wrap read at first shot.");
         return IDL_GettmpLong(bitmask & 0xFF);
    }
}

IDL_VPTR adamModbusReadOutput_wrap(int lArgc, IDL_VPTR lArgv[], char *Argk) {
    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        IDL_STRING adam_name;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"ADAM_NAME"  , IDL_TYP_STRING, 1, 0, 0, (char*)IDL_KW_OFFSETOF(adam_name) },
        { NULL, 0, 0, 0, NULL, NULL }
    };
    KW_RESULT           kw;
    string		adam_name;
    
    IDL_StrStore(&kw.adam_name,(char *)"");
    (void) IDL_KWProcessByOffset(lArgc, lArgv, Argk, kw_pars, NULL, 1, &kw);
    adam_name = IDL_STRING_STR(&kw.adam_name);
    IDL_KW_FREE;
    
    //TCP connection
    if(_adamModbus == NULL)
          return IDL_GettmpLong(ADAM_INIT_ERROR);

    int bitmask=0;
    if (!(_adamModbus->ReadCoilStatus(1, 16, 8, &bitmask))) {
        _logger->log(Logger::LOG_LEV_INFO, "adamModbusReadOutput_wrap found TCP connection not initialized ...");
        bitmask=0;
        try {
	        _logger->log(Logger::LOG_LEV_DEBUG, "adamModbusReadOutput_wrap is starting TCP...");
            _adamModbus->clearTcp();
            _adamModbus->initTcp();
        } catch(AOException& e) {
	        _logger->log(Logger::LOG_LEV_ERROR, "adamModbusReadOutput_wrap: %s", e.what().c_str());
   	        return IDL_GettmpLong(e._errcode);
   	    }
        if (!(_adamModbus->ReadCoilStatus(1, 16, 8, &bitmask))) {
	        _logger->log(Logger::LOG_LEV_ERROR, "adamModbusReadOutput_wrap tried to read again but failed.");
            return IDL_GettmpLong(TCP_GENERIC_ERROR);
        } else {
	        _logger->log(Logger::LOG_LEV_DEBUG, "adamModbusReadOutput_wrap read at second shot.");
            return IDL_GettmpLong(bitmask & 0xFF);
        }
       
    } else {
         _logger->log(Logger::LOG_LEV_DEBUG, "adamModbusReadOutput_wrap read at first shot.");
         return IDL_GettmpLong(bitmask & 0xFF);
    }

}



IDL_VPTR adamModbusWriteStructure_wrap(int Argc, IDL_VPTR Argv[], char *Argk) {
 

   IDL_VPTR            adam_params, s_tag;
   IDL_MEMINT          ret=0;
   bool                res=0;
   BYTE                val=0;
   BYTE c[8]={0,0,0,0,0,0,0,0};

//   if (Argc ==0 || Argc > 1) return IDL_GettmpLong(IDL_INPUT_TYPE);
   adam_params  = Argv[0];
   if(adam_params->type != IDL_TYP_STRUCT) return IDL_GettmpLong(IDL_INPUT_TYPE);

   ret = IDL_StructTagInfoByName(adam_params->value.s.sdef, (char*)"TCS_SYS_RST_N", 0,  &s_tag);
   if (ret != -1 && s_tag->type == IDL_TYP_INT)
       c[0] = (BYTE)(*(IDL_INT*)(adam_params->value.s.arr->data + ret)) ;
   else return IDL_GettmpLong(IDL_INPUT_TYPE);

   ret = IDL_StructTagInfoByName(adam_params->value.s.sdef, (char*)"TCS_BOOT_SELECT_N", 0,  &s_tag);
   if (ret != -1 && s_tag->type == IDL_TYP_INT)
       c[1] = (BYTE)(*(IDL_INT*)(adam_params->value.s.arr->data + ret)) ;
   else return IDL_GettmpLong(IDL_INPUT_TYPE);

   ret = IDL_StructTagInfoByName(adam_params->value.s.sdef, (char*)"TCS_FPGA_CLR_N", 0,  &s_tag);
   if (ret != -1 && s_tag->type == IDL_TYP_INT)
       c[2] = (BYTE)(*(IDL_INT*)(adam_params->value.s.arr->data + ret)) ;
   else return IDL_GettmpLong(IDL_INPUT_TYPE);

   ret = IDL_StructTagInfoByName(adam_params->value.s.sdef, (char*)"TCS_DRIVER_ENABLE", 0,  &s_tag);
   if (ret != -1 && s_tag->type == IDL_TYP_INT)
       c[3] = (BYTE)(*(IDL_INT*)(adam_params->value.s.arr->data + ret)) ;
   else return IDL_GettmpLong(IDL_INPUT_TYPE);

   ret = IDL_StructTagInfoByName(adam_params->value.s.sdef, (char*)"TSS_DISABLE", 0,  &s_tag);
   if (ret != -1 && s_tag->type == IDL_TYP_INT)
       c[4] = (BYTE)(*(IDL_INT*)(adam_params->value.s.arr->data + ret)) ;
   else return IDL_GettmpLong(IDL_INPUT_TYPE);

   ret = IDL_StructTagInfoByName(adam_params->value.s.sdef, (char*)"OUT5", 0,  &s_tag);
   if (ret != -1 && s_tag->type == IDL_TYP_INT)
       c[5] = (BYTE)(*(IDL_INT*)(adam_params->value.s.arr->data + ret)) ;
   else return IDL_GettmpLong(IDL_INPUT_TYPE);

   ret = IDL_StructTagInfoByName(adam_params->value.s.sdef, (char*)"OUT6", 0,  &s_tag);
   if (ret != -1 && s_tag->type == IDL_TYP_INT)
       c[6] = (BYTE)(*(IDL_INT*)(adam_params->value.s.arr->data + ret)) ;
   else return IDL_GettmpLong(IDL_INPUT_TYPE);

   ret = IDL_StructTagInfoByName(adam_params->value.s.sdef, (char*)"MAIN_POWER", 0,  &s_tag);
   if (ret != -1 && s_tag->type == IDL_TYP_INT)
       c[7] = (BYTE)(*(IDL_INT*)(adam_params->value.s.arr->data + ret)) ;
   else return IDL_GettmpLong(IDL_INPUT_TYPE);

    _logger->log(Logger::LOG_LEV_DEBUG, "vettore c %x,%x,%x,%x,%x,%x,%x,%x", c[0], c[1], c[2], c[3],c[4],c[5],c[6],c[7]);
   for (int i=0; i<8; i++) {
      val = val + (((BYTE)c[i]) << i);
   }

   if(_adamModbus == NULL)
          return IDL_GettmpLong(ADAM_INIT_ERROR);

    int bitmask = 0;
    if (!(_adamModbus->SetMultiCoils(val))) {
        _logger->log(Logger::LOG_LEV_DEBUG, "adamModbusWriteStructure_wrap found TCP connection not initialized ...");
        bitmask=0;
        try {
	        _logger->log(Logger::LOG_LEV_DEBUG, "adamModbusWriteStructure_wrap is starting TCP...");
            _adamModbus->clearTcp();
            _adamModbus->initTcp();
        } catch(AOException& e) {
	        _logger->log(Logger::LOG_LEV_ERROR, "adamModbusWriteStructure_wrap: %s", e.what().c_str());
   	        return IDL_GettmpLong(e._errcode);
   	    }
        if (!(_adamModbus->SetMultiCoils(val))) {
	        _logger->log(Logger::LOG_LEV_ERROR, "adamModbusWriteStructure_wrap tried to write again but failed.");
            return IDL_GettmpLong(TCP_GENERIC_ERROR);
        } else {
	        _logger->log(Logger::LOG_LEV_DEBUG, "adamModbusWriteStructure_wrap wrote at second shot.");
            return IDL_GettmpLong(NO_ERROR);
        }
       
    } else {
         _logger->log(Logger::LOG_LEV_DEBUG, "adamModbusWriteStructure_wrap wrote at first shot.");
         return IDL_GettmpLong(NO_ERROR);
    }
    



   return IDL_GettmpLong(NO_ERROR);
}

IDL_VPTR adamLed(int Argc, IDL_VPTR Argv[], char *Argk) {
    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        int on;
        int off;
	int on_there;
	int off_there;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"OFF"  , IDL_TYP_INT, 1, 0, (int*)IDL_KW_OFFSETOF(off_there), (char*)IDL_KW_OFFSETOF(off) },
        { (char*)"ON"   , IDL_TYP_INT, 1, 0, (int*)IDL_KW_OFFSETOF(on_there),  (char*)IDL_KW_OFFSETOF(on) },
        { NULL, 0, 0, 0, NULL, NULL }
    };
    KW_RESULT           kw;
    int	led;
    bool on;
    
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw);
    on = kw.on_there ? true : false;
    if   (kw.off_there) on=false;
    IDL_KW_FREE;
    
    led = IDL_LongScalar(Argv[0]);

    Adam* adam;
    bool ret = false;
    try {
	_logger->log(Logger::LOG_LEV_INFO, "adamLed: led=%d on=%d [%s:%d]", led, on, __FILE__, __LINE__);
	adam = new Adam("2");
	if (led==0 && on)  ret = adam->Led0On();
	if (led==0 && !on) ret = adam->Led0Off();
	if (led==1 && on)  ret = adam->Led1On();
	if (led==1 && !on) ret = adam->Led1Off();
	if (led==2 && on)  ret = adam->Led2On();
	if (led==2 && !on) ret = adam->Led2Off();
	if (led==3 && on)  ret = adam->Led3On();
	if (led==3 && !on) ret = adam->Led3Off();
	delete adam;
    }
    catch(AOException& e) {
	_logger->log(Logger::LOG_LEV_ERROR, "adamLed error: %s", e.what().c_str());
	delete adam;
	return IDL_GettmpLong(IDL_GENERIC_ERROR);
    }
    return IDL_GettmpLong( (ret==true) ? NO_ERROR : IDL_GENERIC_ERROR);
}


// ********************************************************************************************************************* //
// ****************************************** BCU COMMON LIB WRAPPER SECTION ******************************************* //


IDL_VPTR getBcuId_wrap(int lArgc, IDL_VPTR Argv[]) {
 	if(lArgc == 1) {
 		static BcuIdMap _bcuIdMap;
 		string bcuName = IDL_VarGetString(Argv[0]);
 		_logger->log(Logger::LOG_LEV_DEBUG, "Retrieving ID for %s", bcuName.c_str());
 		try {
 			int bcuId = _bcuIdMap[bcuName];
 			return IDL_GettmpLong(bcuId);
 		}
 		catch (BcuIdMapException& e) {
 			_logger->log(Logger::LOG_LEV_ERROR, "ID for %s is not existing !", bcuName.c_str());
 		}
 	}
 	return IDL_GettmpLong(-1);
}


IDL_VPTR getBcuIp_wrap(int lArgc, IDL_VPTR Argv[]) {
 	if(lArgc == 1) {
 		static BcuIpMap _bcuIpMap;
 		string bcuName  = IDL_VarGetString(Argv[0]);
 		_logger->log(Logger::LOG_LEV_INFO, "Retrieving IP port for %s", bcuName.c_str());
 		try {
 			string bcuIp = _bcuIpMap[bcuName];
 			return IDL_StrToSTRING((char*)bcuIp.c_str());
 		}
 		catch (BcuIpMapException& e) {
 			_logger->log(Logger::LOG_LEV_ERROR, "IP port for %s is not existing !", bcuName.c_str());
 		}
 	}
 	return IDL_StrToSTRING((char*)"");
}

IDL_VPTR lao_strerror_wrap(int /* lArgc */, IDL_VPTR Argv[]) {
    int code =  IDL_LongScalar(Argv[0]);
    _logger->log(Logger::LOG_LEV_DEBUG, "Retrieving ERROR CODE TEXT for %d", code);
    return IDL_StrToSTRING((char*)lao_strerror(code));
}



IDL_VPTR getBcuMasterUdpPort_wrap(int lArgc, IDL_VPTR Argv[]) {
	if(lArgc == 1) {
		static BcuMasterUdpMap _bcuMasterUdpMap((string)_adsecConfig["MasterDiagnosticConfFile"]);
 		string bcuName = IDL_VarGetString(Argv[0]);
 		_logger->log(Logger::LOG_LEV_DEBUG, "Retrieving UDP MASTER port for %s", bcuName.c_str());
 		try {
 			int bcuUdpMasterPort = _bcuMasterUdpMap[bcuName];
 			return IDL_GettmpLong(bcuUdpMasterPort);
 		}
 		catch (BcuUdpMapException& e) {
 			_logger->log(Logger::LOG_LEV_ERROR, "UDP MASTER port for %s is not existing !", bcuName.c_str());
 		}
 	}
 	return IDL_GettmpLong(-1);
}


IDL_VPTR read_seq_multi_wrap(int Argc, IDL_VPTR Argv[])
{

    IDL_INT firstBCU, lastBCU, firstDSP, lastDSP;
    IDL_LONG dspAddress, dataLength;
    UCHAR *dataBuffer, setPD;
    IDL_LONG time;
    int opcode;
	int flags = Constants::WANTREPLY_FLAG;

    // Check if the number of argument is correct
    if (Argc != 9) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

    // Check if we are connected with the Supervisor
    if (!_idl_connected) return IDL_GettmpLong(MISSING_CLIENTNAME_ERROR);

    firstBCU      = IDL_LongScalar(Argv[0]);
    lastBCU    	  = IDL_LongScalar(Argv[1]);
    firstDSP  	  = IDL_LongScalar(Argv[2]);
    lastDSP   	  = IDL_LongScalar(Argv[3]);
    dspAddress 	  = IDL_LongScalar(Argv[4]);
    dataLength 	  = IDL_LongScalar(Argv[5]);
    if(isIdlArray(Argv[6])) {
    	dataBuffer = Argv[6]->value.arr->data;
    }
    else {
    	dataBuffer = (UCHAR*)(&(Argv[6]->value.l));
    }
    time 		  = IDL_LongScalar(Argv[7]);
    setPD 		  = Argv[8]->value.c;

    // Data compatibility check
    if (setPD >= 6 || setPD == 1) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

    // verifica il valore di data length
    if (dataLength<=0) return IDL_GettmpLong(IDL_DATALENGTH_ERROR);

    // verifica la congruenza dei dati
    // last DSP > first DSP
    if (lastDSP<firstDSP) return IDL_GettmpLong(IDL_WRONG_DSP_ERROR);

    switch (setPD)
    {
       	case 0 : opcode = OpCodes::MGP_OP_RDSEQ_DSP;
         	break;
      	case 2 : opcode = OpCodes::MGP_OP_RDSEQ_SDRAM;
         	break;
       	case 3 : opcode = OpCodes::MGP_OP_RDSEQ_SRAM;
         	break;
       	case 4 : opcode = OpCodes::MGP_OP_RDSEQ_FLASH;
         	break;
       	case 5 : opcode = OpCodes::MGP_OP_RDSEQ_DIAGBUFF;
         	break;
    }

    dumpCommand(opcode, firstBCU, lastBCU, firstDSP, lastDSP, dspAddress, dataLength, time,  flags, dataBuffer);

    return IDL_GettmpLong(sendMultiBcuCommand(Priority::LOW_PRIORITY, firstBCU, lastBCU, firstDSP, lastDSP,
     						   				  opcode, dspAddress, dataLength*Constants::DWORD_SIZE,
     						   				  dataBuffer, time, flags));
}


IDL_VPTR write_seq_multi_wrap(int Argc, IDL_VPTR Argv[])
{
 	IDL_INT firstBCU, lastBCU, firstDSP, lastDSP;
 	IDL_LONG dspAddress, dataLength;
 	UCHAR *dataBuffer, setPD;
 	IDL_LONG time, flags;
 	int opcode;

    // Check if the number of argument is correct
    if (Argc != 10) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

    // Check if we are connected with the Supervisor
    if (! _idl_connected) return IDL_GettmpLong(MISSING_CLIENTNAME_ERROR);

    firstBCU      = IDL_LongScalar(Argv[0]);
    lastBCU    	  = IDL_LongScalar(Argv[1]);
    firstDSP  	  = IDL_LongScalar(Argv[2]);
    lastDSP   	  = IDL_LongScalar(Argv[3]);
    dspAddress 	  = IDL_LongScalar(Argv[4]);
    dataLength 	  = IDL_LongScalar(Argv[5]);
    if(isIdlArray(Argv[6])) {
    	dataBuffer = Argv[6]->value.arr->data;
    }
    else {
    	dataBuffer = (UCHAR*)(&(Argv[6]->value.l));
    }
    time  = IDL_LongScalar(Argv[7]);
    setPD = Argv[8]->value.c;
    flags = IDL_LongScalar(Argv[9]) & Constants::WANTREPLY_FLAG;

    // Data compatibility check
    if (setPD >= 7 || setPD == 1) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

    // verifica il valore di data length
    if (dataLength<=0){
    	_logger->log(Logger::LOG_LEV_TRACE, "dataLength : %ld", dataLength);
       	return IDL_GettmpLong(IDL_DATALENGTH_ERROR);
    }

    // verifica la congruenza dei dati
    // last DSP > first DSP
    if (lastDSP<firstDSP)
    	return IDL_GettmpLong(IDL_WRONG_DSP_ERROR);

    switch (setPD) {
    	case 0 : opcode =  OpCodes::MGP_OP_WRSEQ_DSP;
         	break;
      	 case 1 : // return an error code; Note by AR
         	break;
       	case 2 : opcode = OpCodes::MGP_OP_WRSEQ_SDRAM;
         	break;
       	case 3 : opcode = OpCodes::MGP_OP_WRSEQ_SRAM;
         	break;
       	case 4 : opcode = OpCodes::MGP_OP_WRITE_FLASH;
         	break;
       	case 5 : opcode = OpCodes::MGP_OP_WRSEQ_DIAGBUFF;
         	break;
       	case 6 : opcode = OpCodes::MGP_OP_WRITE_SIGGEN_RAM;
         	break;
	}

    dumpCommand(opcode, firstBCU, lastBCU, firstDSP, lastDSP, dspAddress, dataLength, time,  flags, dataBuffer);

    return IDL_GettmpLong(sendMultiBcuCommand(Priority::LOW_PRIORITY, firstBCU, lastBCU, firstDSP, lastDSP,
     						   				  opcode, dspAddress, dataLength*Constants::DWORD_SIZE,
     						  				  dataBuffer, time, flags));
}


IDL_VPTR write_same_multi_wrap(int Argc, IDL_VPTR Argv[])
{

	IDL_INT firstBCU, lastBCU, firstDSP, lastDSP;
	IDL_LONG dspAddress, dataLength;
	UCHAR *dataBuffer, setPD;
	IDL_LONG time, flags;
  	int opcode;

  	// Check if the number of argument is correct
  	if (Argc != 10) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

  	// Check if we are connected with the Supervisor
  	if (!_idl_connected) return IDL_GettmpLong(MISSING_CLIENTNAME_ERROR);

	firstBCU      = IDL_LongScalar(Argv[0]);
    lastBCU    	  = IDL_LongScalar(Argv[1]);
    firstDSP  	  = IDL_LongScalar(Argv[2]);
    lastDSP   	  = IDL_LongScalar(Argv[3]);
    dspAddress 	  = IDL_LongScalar(Argv[4]);
    dataLength 	  = IDL_LongScalar(Argv[5]);
    if(isIdlArray(Argv[6])) {
    	dataBuffer = Argv[6]->value.arr->data;
    }
    else {
    	dataBuffer = (UCHAR*)(&(Argv[6]->value.l));
    }
    time  = IDL_LongScalar(Argv[7]);
    setPD = Argv[8]->value.c;
    flags = IDL_LongScalar(Argv[9]) & Constants::WANTREPLY_FLAG;

   	// Data compatibility check
   	if (setPD >= 7 || setPD == 1 || setPD == 4) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

  	 // verifica il valore di data length
     if (dataLength<=0) return IDL_GettmpLong(IDL_DATALENGTH_ERROR);

   	// verifica la congruenza dei dati
   	// last DSP > first DSP
    if (lastDSP<firstDSP) return IDL_GettmpLong(IDL_WRONG_DSP_ERROR);

     switch (setPD)  {
     	case 0: opcode = OpCodes::MGP_OP_WRSAME_DSP;
        	break;
      	case 1: //error
        	break;
      	case 2: opcode = OpCodes::MGP_OP_WRSAME_SDRAM;
        	break;
      	case 3: opcode = OpCodes::MGP_OP_WRSAME_SRAM;
        	break;
      	case 4: //error
        	break;
      	case 5: opcode = OpCodes::MGP_OP_WRSAME_DIAGBUFF;
        	break;
      	case 6: opcode = OpCodes::MGP_OP_WRITE_SIGGEN_RAM;
        	break;
    }

   	dumpCommand(opcode, firstBCU, lastBCU, firstDSP, lastDSP, dspAddress, dataLength, time,  flags, dataBuffer);

    return IDL_GettmpLong(sendMultiBcuCommand(Priority::LOW_PRIORITY, firstBCU, lastBCU, firstDSP, lastDSP,
     										  opcode, dspAddress, dataLength*Constants::DWORD_SIZE,
     										  dataBuffer, time, flags));
}


IDL_VPTR read_seq_wrap(int Argc, IDL_VPTR Argv[])
{

    IDL_INT BCU_num, firstDSP, lastDSP;
    IDL_LONG dspAddress, dataLength;
    UCHAR *dataBuffer, setPD;
    IDL_LONG time;
    int opcode;
    int flags = Constants::WANTREPLY_FLAG;

    // Check if the number of argument is correct
    if (Argc != 8) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

    // Check if we are connected with the Supervisor
    if (!_idl_connected) return IDL_GettmpLong(MISSING_CLIENTNAME_ERROR);

    BCU_num       = IDL_LongScalar(Argv[0]);
    firstDSP  	  = IDL_LongScalar(Argv[1]);
    lastDSP   	  = IDL_LongScalar(Argv[2]);
    dspAddress 	  = IDL_LongScalar(Argv[3]);
    dataLength 	  = IDL_LongScalar(Argv[4]);
    if(isIdlArray(Argv[5])) {
    	dataBuffer = Argv[5]->value.arr->data;
    }
    else {
    	dataBuffer = (UCHAR*)(&(Argv[5]->value.l));
    }
    time 		  = IDL_LongScalar(Argv[6]);
    setPD 		  = Argv[7]->value.c;

    // Data compatibility check
    if (setPD >= 6 || setPD == 1) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

    // verifica il valore di data length
    if (dataLength<=0) return IDL_GettmpLong(IDL_DATALENGTH_ERROR);

    // verifica la congruenza dei dati
    // last DSP > first DSP
    if (lastDSP<firstDSP) return IDL_GettmpLong(IDL_WRONG_DSP_ERROR);

    switch (setPD)
    {
       	case 0 : opcode = OpCodes::MGP_OP_RDSEQ_DSP;
         	break;
      	case 2 : opcode = OpCodes::MGP_OP_RDSEQ_SDRAM;
         	break;
       	case 3 : opcode = OpCodes::MGP_OP_RDSEQ_SRAM;
         	break;
       	case 4 : opcode = OpCodes::MGP_OP_RDSEQ_FLASH;
         	break;
       	case 5 : opcode = OpCodes::MGP_OP_RDSEQ_DIAGBUFF;
         	break;
    }

    dumpCommand(opcode, BCU_num, BCU_num, firstDSP, lastDSP, dspAddress, dataLength, time,  flags, dataBuffer);

    return IDL_GettmpLong(sendMultiBcuCommand(Priority::LOW_PRIORITY, BCU_num, BCU_num, firstDSP, lastDSP,
     						   				  opcode, dspAddress, dataLength*Constants::DWORD_SIZE,
     						   				  dataBuffer, time, flags));
}


IDL_VPTR write_seq_wrap(int Argc, IDL_VPTR Argv[])
{
    IDL_INT BCU_num, firstDSP, lastDSP;
    IDL_LONG dspAddress, dataLength;
    UCHAR *dataBuffer, setPD;
    IDL_LONG time, flags;
    int opcode;

    // Check if the number of argument is correct
    if (Argc != 9) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

    // Check if we are connected with the Supervisor
    if (! _idl_connected) return IDL_GettmpLong(MISSING_CLIENTNAME_ERROR);

    BCU_num       = IDL_LongScalar(Argv[0]);
    firstDSP  	  = IDL_LongScalar(Argv[1]);
    lastDSP   	  = IDL_LongScalar(Argv[2]);
    dspAddress 	  = IDL_LongScalar(Argv[3]);
    dataLength 	  = IDL_LongScalar(Argv[4]);
	if(isIdlArray(Argv[5])) {
    	dataBuffer = Argv[5]->value.arr->data;
    }
    else {
    	dataBuffer = (UCHAR*)(&(Argv[5]->value.l));
    }
    time  = IDL_LongScalar(Argv[6]);
    setPD = Argv[7]->value.c;			// [todo] Check !!!
    flags = IDL_LongScalar(Argv[8]) & Constants::WANTREPLY_FLAG;

     // Data compatibility check
     if (setPD >= 7 || setPD == 1) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

     // verifica il valore di data length
     if (dataLength<=0){
       _logger->log(Logger::LOG_LEV_TRACE, "dataLength : %ld", dataLength);
       return IDL_GettmpLong(IDL_DATALENGTH_ERROR);
     }
     // verifica la congruenza dei dati
     // last DSP > first DSP
     if (lastDSP<firstDSP)
       return IDL_GettmpLong(IDL_WRONG_DSP_ERROR);

     switch (setPD) {
       case 0 : opcode =  OpCodes::MGP_OP_WRSEQ_DSP;
         break;
       case 1 : // return an error code; Note by AR
         break;
       case 2 : opcode = OpCodes::MGP_OP_WRSEQ_SDRAM;
         break;
       case 3 : opcode = OpCodes::MGP_OP_WRSEQ_SRAM;
         break;
       case 4 : opcode = OpCodes::MGP_OP_WRITE_FLASH;
         break;
       case 5 : opcode = OpCodes::MGP_OP_WRSEQ_DIAGBUFF;
         break;
       case 6 : opcode = OpCodes::MGP_OP_WRITE_SIGGEN_RAM;
         break;
       }

     dumpCommand(opcode, BCU_num, BCU_num, firstDSP, lastDSP, dspAddress, dataLength, time,  flags, dataBuffer);

     return IDL_GettmpLong(sendMultiBcuCommand(Priority::LOW_PRIORITY, BCU_num, BCU_num, firstDSP, lastDSP,
     										   opcode, dspAddress, dataLength*Constants::DWORD_SIZE,
     										   dataBuffer, time, flags));
}


IDL_VPTR write_same_wrap(int Argc, IDL_VPTR Argv[])
{
	IDL_INT BCU_num, firstDSP, lastDSP;
	IDL_LONG dspAddress, dataLength;
	UCHAR *dataBuffer, setPD;
	IDL_LONG time, flags;
  	int opcode;

  	// Check if the number of argument is correct
  	if (Argc != 9) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

  	// Check if we are connected with the Supervisor
  	if (!_idl_connected) return IDL_GettmpLong(MISSING_CLIENTNAME_ERROR);

	BCU_num       = IDL_LongScalar(Argv[0]);
    firstDSP  	  = IDL_LongScalar(Argv[1]);
    lastDSP   	  = IDL_LongScalar(Argv[2]);
    dspAddress 	  = IDL_LongScalar(Argv[3]);
    dataLength 	  = IDL_LongScalar(Argv[4]);
    if(isIdlArray(Argv[5])) {
    	dataBuffer = Argv[5]->value.arr->data;
    }
    else {
    	dataBuffer = (UCHAR*)(&(Argv[5]->value.l));
    }
    time  = IDL_LongScalar(Argv[6]);
    setPD = Argv[7]->value.c;
    flags = IDL_LongScalar(Argv[8]) & Constants::WANTREPLY_FLAG;

   	// Data compatibility check
   	if (setPD >= 7 || setPD == 1 || setPD == 4) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

  	 // verifica il valore di data length
     if (dataLength<=0) return IDL_GettmpLong(IDL_DATALENGTH_ERROR);

   	// verifica la congruenza dei dati
   	// last DSP > first DSP
    if (lastDSP<firstDSP) return IDL_GettmpLong(IDL_WRONG_DSP_ERROR);

      switch (setPD)
      {
      case 0 : opcode = OpCodes::MGP_OP_WRSAME_DSP;
               break;
      case 1 : //error
               break;
      case 2 : opcode = OpCodes::MGP_OP_WRSAME_SDRAM;
               break;
      case 3 : opcode = OpCodes::MGP_OP_WRSAME_SRAM;
               break;
      case 4 : //error
               break;
      case 5 : opcode = OpCodes::MGP_OP_WRSAME_DIAGBUFF;
               break;
      case 6 : opcode = OpCodes::MGP_OP_WRITE_SIGGEN_RAM;
               break;
      }

   	dumpCommand(opcode, BCU_num, BCU_num, firstDSP, lastDSP, dspAddress, dataLength, time,  flags, dataBuffer);

     return IDL_GettmpLong(sendMultiBcuCommand(Priority::LOW_PRIORITY, BCU_num, BCU_num, firstDSP, lastDSP,
     										   opcode, dspAddress, dataLength*Constants::DWORD_SIZE,
     										   dataBuffer, time, flags));
}


IDL_VPTR reset_devices_wrap(int Argc, IDL_VPTR Argv[])
{
	_logger->printStatus();
	_logger->log(Logger::LOG_LEV_DEBUG, ">>> IDL resetting devices (with reply wait)...");


   	IDL_INT BCU_num, firstDSP, lastDSP;
   	IDL_LONG dspAddress, dataLength;
   	UCHAR *dataBuffer;  //, setPD;
    int opcode;
    int timeout = 200;

	// Check if the number of argument is correct
   	if (Argc != 6) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

	// Check if we are connected with the Supervisor
   	if (!_idl_connected) return IDL_GettmpLong(MISSING_CLIENTNAME_ERROR);

   	BCU_num   	= IDL_LongScalar(Argv[0]);
   	firstDSP  	= IDL_LongScalar(Argv[1]);
   	lastDSP   	= IDL_LongScalar(Argv[2]);
   	dspAddress 	= IDL_LongScalar(Argv[3]);
   	dataLength	= IDL_LongScalar(Argv[4]);
   	if(isIdlArray(Argv[5])) {
    	dataBuffer = Argv[5]->value.arr->data;
    }
    else {
    	dataBuffer = (UCHAR*)(&(Argv[5]->value.l));
    }

	// verifica la congruenza dei dati
	// last DSP > first DSP
   	if (lastDSP < firstDSP ) return IDL_GettmpLong(IDL_WRONG_DSP_ERROR);

    opcode = OpCodes::MGP_OP_RESET_DEVICES;

	_logger->log(Logger::LOG_LEV_TRACE, "> OpCode:     %d (Reset devices)", opcode);
	_logger->log(Logger::LOG_LEV_TRACE, "> Bcu:        %d", BCU_num);
	_logger->log(Logger::LOG_LEV_TRACE, "> First Dsp:  %d", firstDSP);
	_logger->log(Logger::LOG_LEV_TRACE, "> Last Dsp:   %d", lastDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "> Timeout:    %d", timeout);
    _logger->log(Logger::LOG_LEV_TRACE, "> DataLength: %d", dataLength);
    _logger->log(Logger::LOG_LEV_TRACE, "> Reset mask: %d %d", (int)(((int*)dataBuffer)[0]), (int)(((int*)dataBuffer)[1]));

    return IDL_GettmpLong(sendMultiBcuCommand(Priority::LOW_PRIORITY, BCU_num, BCU_num, firstDSP, lastDSP,
     						   				  opcode, dspAddress,
     						   				  dataLength*Constants::DWORD_SIZE,
     						  				  dataBuffer,
     						   				  timeout,
     						   				  Constants::WANTREPLY_FLAG));
}


IDL_VPTR reset_devices_multi_wrap(int Argc, IDL_VPTR Argv[])
{
	_logger->printStatus();
	_logger->log(Logger::LOG_LEV_DEBUG, ">>> IDL resetting MULTI devices (with reply wait)...");

   	IDL_INT firstBcu, lastBcu, firstDSP, lastDSP;
   	IDL_LONG dspAddress, dataLength;
   	UCHAR *dataBuffer;  //, setPD;
   	int opcode;
   	int timeout = 200;

	// Check if the number of argument is correct
   	if (Argc != 7) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

	// Check if we are connected with the Supervisor
   	if (! _idl_connected) return IDL_GettmpLong(MISSING_CLIENTNAME_ERROR);

   	firstBcu   	= IDL_LongScalar(Argv[0]);
   	lastBcu   	= IDL_LongScalar(Argv[1]);
   	firstDSP  	= IDL_LongScalar(Argv[2]);
   	lastDSP   	= IDL_LongScalar(Argv[3]);
   	dspAddress 	= IDL_LongScalar(Argv[4]);
   	dataLength	= IDL_LongScalar(Argv[5]);
   	if(isIdlArray(Argv[6])) {
    	dataBuffer = Argv[6]->value.arr->data;
    }
    else {
    	dataBuffer = (UCHAR*)(&(Argv[6]->value.l));
    }

	// verifica la congruenza dei dati
	// last DSP > first DSP
   	if (lastDSP < firstDSP ) return IDL_GettmpLong(IDL_WRONG_DSP_ERROR);

    opcode = OpCodes::MGP_OP_RESET_DEVICES;

	_logger->log(Logger::LOG_LEV_TRACE, "> OpCode :    %d (Reset devices)", opcode);
	_logger->log(Logger::LOG_LEV_TRACE, "> First Bcu:  %d", firstBcu);
	_logger->log(Logger::LOG_LEV_TRACE, "> Last Bcu:   %d", lastBcu);
	_logger->log(Logger::LOG_LEV_TRACE, "> First DSP:  %d", firstDSP);
	_logger->log(Logger::LOG_LEV_TRACE, "> Last DSP:   %d", lastDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "> Timeout:    %d", timeout);
    _logger->log(Logger::LOG_LEV_TRACE, "> DataLength: %d", dataLength);
    _logger->log(Logger::LOG_LEV_TRACE, "> Reset mask: %d %d", (int)(((int*)dataBuffer)[0]), (int)(((int*)dataBuffer)[1]));

   return IDL_GettmpLong(sendMultiBcuCommand(Priority::LOW_PRIORITY, firstBcu, lastBcu, firstDSP, lastDSP,
     						   				 opcode, dspAddress,
     						   				 dataLength*Constants::DWORD_SIZE,
     						  				 dataBuffer,
     						   				 timeout,
     						   				 Constants::WANTREPLY_FLAG));
}

IDL_VPTR reset_devices_multi_check_wrap(int Argc, IDL_VPTR Argv[])
{
	_logger->printStatus();
	_logger->log(Logger::LOG_LEV_DEBUG, ">>> IDL fully resetting MULTI devices (checking reset completed)...");

   	IDL_INT firstBcu, lastBcu, firstDSP, lastDSP;
   	IDL_LONG dspAddress, dataLength;
   	UCHAR *dataBuffer;  //, setPD;
   	double timeout_s;
   	int opcode;

	// Check if the number of argument is correct
   	if (Argc != 8) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

	// Check if we are connected with the Supervisor
   	if (!_idl_connected) return IDL_GettmpLong(MISSING_CLIENTNAME_ERROR);

   	firstBcu   = IDL_LongScalar(Argv[0]);
   	lastBcu    = IDL_LongScalar(Argv[1]);
   	firstDSP   = IDL_LongScalar(Argv[2]);
   	lastDSP    = IDL_LongScalar(Argv[3]);
   	dspAddress = IDL_LongScalar(Argv[4]);
   	dataLength = IDL_LongScalar(Argv[5]);
   	if(isIdlArray(Argv[6])) {
    	dataBuffer = Argv[6]->value.arr->data;
    }
    else {
    	dataBuffer = (UCHAR*)(&(Argv[6]->value.l));
    }
   	timeout_s  = IDL_DoubleScalar(Argv[7]);

	// verifica la congruenza dei dati
	// last DSP > first DSP
   	if (lastDSP < firstDSP ) return IDL_GettmpLong(IDL_WRONG_DSP_ERROR);

    opcode = OpCodes::MGP_OP_RESET_DEVICES;

    _logger->log(Logger::LOG_LEV_TRACE, "> OpCode :    %d (Reset devices)", opcode);
	_logger->log(Logger::LOG_LEV_TRACE, "> First Bcu:  %d", firstBcu);
	_logger->log(Logger::LOG_LEV_TRACE, "> Last Bcu:   %d", lastBcu);
	_logger->log(Logger::LOG_LEV_TRACE, "> First DSP:  %d", firstDSP);
	_logger->log(Logger::LOG_LEV_TRACE, "> Last DSP:   %d", lastDSP);
	_logger->log(Logger::LOG_LEV_TRACE, "> Timeout:    %d s", timeout_s);
	_logger->log(Logger::LOG_LEV_TRACE, "> Reset mask: %d %d", (int)(((int*)dataBuffer)[0]), (int)(((int*)dataBuffer)[1]));

	// --- 1) Send the RESET message, without any timeout ---
	int ret;
	ret = sendMultiBcuCommand(Priority::LOW_PRIORITY, firstBcu, lastBcu, firstDSP, lastDSP,
	     					  opcode, dspAddress,
	     					  dataLength*Constants::DWORD_SIZE,
     						  dataBuffer,
     						  0,  	// Timeout: doesn't wait the reply !!!
     						  0);   // Flags: doesn't want the reply !!!
	// Note that it can't be a Timeout
	if(IS_ERROR(ret)) {
		_logger->log(Logger::LOG_LEV_WARNING, "Reset request failed");
		return IDL_GettmpLong(ret);
	}

	// --- 2) Poll the BCU to check the reset completion ---
	if (isBcuReady(firstBcu, lastBcu, timeout_s)) {
		_logger->log(Logger::LOG_LEV_INFO, "Reset completed !");
		return IDL_GettmpLong(NO_ERROR);
	}
	else {
		_logger->log(Logger::LOG_LEV_WARNING, "Timeout %2.1f s expired: reset request failed", timeout_s);
		return IDL_GettmpLong(TIMEOUT_ERROR);
	}
}


//
// isBcuReady
//
// This function checks if one or more BCU are ready
//
// ret = isBcuReady (firstBcu, [lastBcu], timeout=timeout)
//
// INPUTS
//      firstBcu     ID of first BCU to check (use GETBCUID_WRAP to get the ID)
//      lastBcu      ID of last BCU to check (optional)
//
// OUTPUT
//
// KEYWORDS
//      TIMEOUT      timeout [s]
//


IDL_VPTR isBcuReady_wrap(int Argc, IDL_VPTR Argv[], char *Argk)
{
    typedef struct { 
         IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure 
         int timeout;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"TIMEOUT"  , IDL_TYP_LONG, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },
        { NULL , 0, 0, 0, NULL, NULL}
    };

    KW_RESULT           kw;
    kw.timeout = 60;
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw);
    int timeout_s = kw.timeout;
    IDL_KW_FREE;

    int firstBcu, lastBcu;
    firstBcu = IDL_LongScalar(Argv[0]);
	lastBcu  = (Argc > 1) ? IDL_LongScalar(Argv[1]) : firstBcu;

    bool ret = isBcuReady(firstBcu, lastBcu, timeout_s);
    return IDL_GettmpLong((ret==true) ? 0 : NETWORK_ERROR);
}

// ********************************************************************************************************************* //
// ************************************************* SHM WRAPPER SECTION *********************************************** //

IDL_VPTR shm_readbuf_wrap(int Argc, IDL_VPTR Argv[])
{
   char *ptrBufName;
   int ErrC1 = 0;
   int ErrC2 = 0;
   int ErrC3 = 0;
   int retry, n_retry=1;
   int TypeBuf;
   IDL_INT type;
   unsigned char *dataptr;
   BufInfo *local_info;
   char MyName[PROC_NAME_LEN];
   IDL_LONG timeout = 0;

   if (Argc !=4) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

   ptrBufName = IDL_VarGetString(Argv[0]);
   type = IDL_LongScalar(Argv[1]);
   timeout = IDL_LongScalar(Argv[2]);

   if(isIdlArray(Argv[3])) {
      dataptr = Argv[3]->value.arr->data;
    }
    else {
      return IDL_GettmpLong(-1);
    }


   _logger->log(Logger::LOG_LEV_TRACE, "smreadbuf argv: %s %d %d", ptrBufName, type, timeout);

   if (type <= 0 || type >=3 ) return IDL_GettmpLong(IDL_DATATYPE_ERROR);
   switch (type)
   {
      case 1 : TypeBuf=BUFCONT;
      	break;
      case 2 : TypeBuf=BUFSNAP;
      	break;
   }

   for (retry=0; retry <= n_retry; retry++)
   {
      if (! (local_info = is_attached(ptrBufName)))
      {
          _logger->log(Logger::LOG_LEV_INFO, "I'm attaching with idl_attach (# retry  %d)...",retry);
         ErrC1 = idl_attach(ptrBufName, TypeBuf, &local_info);

         _logger->log(Logger::LOG_LEV_TRACE, "BufRequest returned %s (errcode %d)", lao_strerror(ErrC1), ErrC1);
         if (IS_ERROR(ErrC1)) return IDL_GettmpLong(PLAIN_ERROR(ErrC1));
      }

      ErrC2 = bufRead (local_info, dataptr, timeout);
      _logger->log(Logger::LOG_LEV_TRACE, "bufRead returned %s (errcode %d, errno %d)", lao_strerror(ErrC2), ErrC2, errno);

      if (PLAIN_ERROR(ErrC2) == SEM_LOCK_ERROR) {

         ErrC3 = bufRelease(MyName, local_info);
         if (IS_ERROR(ErrC3)) return IDL_GettmpLong(PLAIN_ERROR(ErrC3));

         memset(local_info, 0, sizeof(local_info));
         _logger->log(Logger::LOG_LEV_TRACE, "Memset done, old shared buffer cleared.");
         resize_Info(ptrBufName);

      } else {
          _logger->log(Logger::LOG_LEV_TRACE, "Memset not done!");
           break;
      }
   }

   if (IS_ERROR(ErrC2)) return IDL_GettmpLong(PLAIN_ERROR(ErrC2));
   //ErrC3 = bufRelease (MyName, &Info);
   //printf("ErrC3 from BufRelease %d\n", PLAIN_ERROR(ErrC3));
   return IDL_GettmpLong(PLAIN_ERROR(ErrC2));
}


//////////////////////////////////////////////////////////////////////////////

//
// SetLogLevel
//
// SetLogLevel(AOApp, logger, level, /time)
//
// INPUT
//      AOApp           AO application name ('FASTDGN', 'TTCTRL', ...)
//                      '' means this library
//      Logger          Logger name. "MAIN" is always present in every AOApp
//      Level
//
// KEYWORD
//      increase        increase verbosity
//      decrease        decrease verbosity
//      timeout         timeout [s] (def. 1s)
//
// RETURN
//      NO_ERROR
//
#include "aoapplib.h"
IDL_VPTR IDLSetLogLevel(int Argc, IDL_VPTR Argv[], char *Argk)
{

	typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        UCHAR  increase;
        UCHAR  decrease;
        double timeout;
        int    increase_there;
        int    decrease_there;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"DECREASE" , IDL_TYP_BYTE,    1, 0,  (int*)IDL_KW_OFFSETOF(decrease_there),
             (char*)IDL_KW_OFFSETOF(decrease) },
        { (char*)"INCREASE" , IDL_TYP_BYTE,    1, 0,  (int*)IDL_KW_OFFSETOF(increase_there),
             (char*)IDL_KW_OFFSETOF(increase) },
        { (char*)"TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },
        { NULL, 0, 0, 0, NULL, NULL }
    };

    KW_RESULT           kw;
    string              aoapp;
    string              logger;
    int                 tmout;
    char*               strLevel;

    IDL_VPTR            c_args[3];       //contains argv after keyword processing

    // set to NULL to check if params struct has been passed as a plain
    // argument
    c_args[2] = NULL;


    kw.timeout = 1.0;
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, c_args, 1, &kw);
    tmout = (int) (kw.timeout * 1000);

    // c_args and Argv are the same, because the previous call
    // sorts the plain arguments into the front of Argv.
    // But c_args[4] is not null iif the level value has been
    // passed as a plain argument

    aoapp  = string(IDL_VarGetString(Argv[0]));
    logger = string(IDL_VarGetString(Argv[1]));

    if(c_args[2] !=NULL ){   // level passed as a plain argument, ignore increase, decrease keywords
		strLevel = IDL_VarGetString(Argv[2]);
		if (aoapp.compare("")==0) {
			if ( Logger::exists(logger) ) {
				Logger::get(logger)->setLevel(logNumLevel(strLevel));
				_logger->log(Logger::LOG_LEV_INFO, "log level of myself [%s] auto-changed to %s [%s:%d]",
						logger.c_str(), logDescr( Logger::get(logger)->getLevel() ), __FILE__, __LINE__);
			}
		} else {
			AOAppLib::setLoggerLevel(aoapp, string (logger), logNumLevel(strLevel));
			_logger->log(Logger::LOG_LEV_INFO, "log level of %s:%s asked to change to %s [%s:%d]",
					aoapp.c_str(), logger.c_str(), strLevel, __FILE__, __LINE__);
		}
	} else {
		if (kw.increase_there){
            if (aoapp.compare("")==0) {
                if ( Logger::exists(logger) ) {
                    Logger::get(logger)->setLevel( Logger::get(logger)->getLevel() + 1 );
                    _logger->log(Logger::LOG_LEV_INFO, "log level of myself [%s] increased to %s by myself [%s:%d]",
                            logger.c_str(), logDescr( Logger::get(logger)->getLevel() ), __FILE__, __LINE__);
                }
			} else {
			}
		}
		if (kw.decrease_there){
            if (aoapp.compare("")==0) {
                if ( Logger::exists(logger) ) {
                    Logger::get(logger)->setLevel( Logger::get(logger)->getLevel() - 1 );
                    _logger->log(Logger::LOG_LEV_INFO, "log level of myself [%s] increased to %s by myself [%s:%d]",
                            logger.c_str(), logDescr( Logger::get(logger)->getLevel() ), __FILE__, __LINE__);
                }
			} else {
			}
		}
	}
    IDL_KW_FREE;
    return IDL_GettmpLong(NO_ERROR);
}

//////////////////////////////////////////////////////////////////////

static IDL_STRUCT_TAG_DEF namelevel_tags[] = {
    { (char*)"NAME", 0, (void *) IDL_TYP_STRING, 0},
    { (char*)"LEVEL", 0, (void *) IDL_TYP_STRING, 0},
    { 0, 0, NULL, 0 }
};

typedef struct namelevel_struct {
    IDL_STRING name;
    IDL_STRING level;
} IDL_NameLevel;

static IDL_NameLevel    *s_namelevel;
//
// GetLogLevel
//
// GetLogLevel(AOApp, map, /time)
//
// INPUT
//      AOApp           AO application name ('FASTDGN', 'TTCTRL', ...)
//                      '' means this library
//      map             Array of structs {name:name, level:level}
//
// KEYWORD
//      timeout         timeout [s] (def. 1s)
//
// RETURN
//      error code or NO_ERROR
//
#include "aoapplib.h"
IDL_VPTR IDLGetLogLevel(int Argc, IDL_VPTR Argv[], char *Argk)
{

    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        double timeout;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },
        { NULL, 0, 0, 0, NULL, NULL }
    };

    KW_RESULT           kw;
    string              aoapp;
    int                 tmout;
    int                 i;
    IDL_VPTR            namelevels;
    NameLevelDict       dict;
    static IDL_MEMINT   n_ele = 0;

    kw.timeout = 1.0;
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw);
    tmout = (int) (kw.timeout * 1000);
    IDL_KW_FREE;

    aoapp  = string(IDL_VarGetString(Argv[0]));
    namelevels  = Argv[1];

    try{
        dict = AOAppLib::getLoggerNamesAndLevels(aoapp);
    } catch (AOException &e) {
        _logger->log(Logger::LOG_LEV_ERROR, e.what());
        return IDL_GettmpLong(e._errcode);
    }


    // Create the structure definition
    void *p = IDL_MakeStruct(0, namelevel_tags);

    // Allocate memory for struct array
    n_ele=dict.size();
    s_namelevel   = (IDL_NameLevel*)malloc(n_ele*sizeof(IDL_NameLevel));

    // Fill structure fields
    i=0;
    for (NameLevelDict::iterator it = dict.begin(); it != dict.end();  it++,i++){
        string name    = (*it).first;
        char*  level   = logDescr( (*it).second ) ;

        IDL_StrStore(&s_namelevel[i].name,  (char*)( name.c_str() ) );
        IDL_StrStore(&s_namelevel[i].level, level  );

        _logger->log(Logger::LOG_LEV_TRACE, "GetLogLevel %s  %s - %s [%s:%d]",
                aoapp.c_str(), name.c_str(), level , __FILE__,__LINE__);
    }

    // Import the data area s_matched into an IDL structure, note that no data are moved.
    IDL_VPTR v_va = IDL_ImportArray( 1, &n_ele, IDL_TYP_STRUCT, (UCHAR *) s_namelevel  , idl_free_cb, (_idl_structure*)p);
    IDL_VarCopy(v_va, namelevels);

    return IDL_GettmpLong(NO_ERROR);
}

#include "aoapplib.h"
IDL_VPTR IDLLogDescr(int /*Argc*/, IDL_VPTR Argv[])
{
    int level = IDL_LongScalar(Argv[0]);
    return(IDL_StrToSTRING(logDescr(level)));
}


// ********************************************************************************************************************* //
// *********************************************** RTDB VAR MANAGEMENT ************************************************* //


/*
 * @Function: readVar
 * Retrieve a var from an RTDB var.
 *
 * Required arguments:
 * 1) Variable name (string)
 * 2) Variable value (multi-type) [Output parameter, allocated by function]
 * 3) Timeout in milliseconds (long)
 *
 * Returns an integer error code.
 * @
 */
IDL_VPTR readVar(int Argc, IDL_VPTR Argv[]) {

    string varname;
    IDL_LONG timeout;
    IDL_VPTR varvalueOut; 	// Output parameter

    MsgBuf*	msgb;

    Variable*	var;
    IDL_MEMINT  varSize = 0;
    int    		varType;
    UCHAR*  	varData;

    // Check if the number of arguments is correct
    if (Argc != 3) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

    // Check if we are connected with the Supervisor
    if (!_idl_connected) return IDL_GettmpLong(MISSING_CLIENTNAME_ERROR);

	// Retrieve parameters
    varname = IDL_VarGetString(Argv[0]);
    varvalueOut = Argv[1];
    timeout = IDL_LongScalar(Argv[2]);

    _logger->log(Logger::LOG_LEV_DEBUG, "Reading var: %s (timeout=%d ms)", varname.c_str(), timeout);

	int ErrC = NO_ERROR;
    msgb = thGetVar((char*)varname.c_str(), timeout, &ErrC);
    if (msgb == NULL) return IDL_GettmpLong(ErrC);
    var = thValue(msgb);
    varSize = var->H.NItems;
    IDL_VPTR v_pa;

    // Retrieve the var type and get the value(s)
    switch(var->H.Type) {
    	case INT_VARIABLE:
            varType = IDL_TYP_LONG;
            varData = (UCHAR*) malloc(varSize * sizeof(IDL_LONG) );
            memcpy(varData,  &(var->Value), varSize * sizeof(IDL_LONG) );
    		_logger->log(Logger::LOG_LEV_DEBUG, "Retrieved %d int (32 bit) var value: %d [%s:%d]",
                    varSize, var->Value.Lv[0], __FILE__,__LINE__);
            v_pa = IDL_ImportArray( 1, &varSize, varType, (UCHAR *) varData, idl_free_cb, NULL);
    		break;
		case REAL_VARIABLE:
            varType = IDL_TYP_DOUBLE;
            varData = (UCHAR*) malloc(varSize * sizeof(double) );
            memcpy(varData,  &(var->Value), varSize * sizeof(double) );
			_logger->log(Logger::LOG_LEV_DEBUG, "Retrieved %d double (64 bit) var value: %f [%s:%d]",
                    varSize, var->Value.Dv[0],__FILE__,__LINE__ );
            v_pa = IDL_ImportArray( 1, &varSize, varType, (UCHAR *) varData, idl_free_cb, NULL);
    		break;
		case CHAR_VARIABLE:
	 		varType = IDL_TYP_BYTE;
         if (varSize == 0) {
            varSize++;
            varData = (UCHAR*) malloc(varSize * sizeof(UCHAR) );
            varData[0] = 0;
         } else {
            varData = (UCHAR*) malloc((varSize+1) * sizeof(UCHAR) );
            memcpy(varData,  &(var->Value), varSize * sizeof(UCHAR) );
            varData[varSize]=0;
         }
    		_logger->log(Logger::LOG_LEV_DEBUG, "Retrieved string var value: %d %s [%s:%d]",
                    varSize, var->Value.Sv[0], __FILE__,__LINE__);
            v_pa = IDL_StrToSTRING((char *) varData);
   			break;
        default:
    		_logger->log(Logger::LOG_LEV_DEBUG, "Wrapper (still) does not support variable type %d [%s:%d]",
                    var->H.Type,__FILE__,__LINE__ );
            thRelease(msgb);
            return IDL_GettmpLong(VAR_TYPE_ERROR);

    }

//    IDL_VarCopy(v_pa,varvalueOut);
    IDL_VarCopy(v_pa,varvalueOut);

    // Clean up
    thRelease(msgb);

    return IDL_GettmpLong(NO_ERROR);
}

/*
 * @Function: writeVar
 *
 * Required arguments:
 * 1) variable name (string)
 * 2) variable value (multi-type)
 * 3) variable type (see dlm for available types)
 * 4) timeout in milliseconds (long)
 *
 * Returns an integer error code.
 * @
 */
IDL_VPTR writeVar(int Argc, IDL_VPTR Argv[]) {
	_logger->log(Logger::LOG_LEV_TRACE, "Writing var...");

    string varname;
    IDL_LONG timeout;
    IDL_VPTR var;
    IDL_LONG vartype;

    // Check if the number of arguments is correct
    if (Argc != 4) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

    // Check if we are connected with the Supervisor
    if (!_idl_connected) return IDL_GettmpLong(MISSING_CLIENTNAME_ERROR);

	varname = IDL_VarGetString(Argv[0]);
    var = Argv[1];		// Is an IDL_VPTR, a pointer to IDL_VARIABLE, that can contain a scalar or an array
    int arraySize = 0;		// If zero, is a scalar (default)
    vartype = IDL_LongScalar(Argv[2]);
    timeout = IDL_LongScalar(Argv[3]);

    // Check if array
    _logger->log(Logger::LOG_LEV_TRACE, "Checking var (scalar or array)...");
    if(var->flags & IDL_V_ARR) {
    	_logger->log(Logger::LOG_LEV_TRACE, "Array found!");
    	arraySize = var->value.arr->n_elts;
    	_logger->log(Logger::LOG_LEV_DEBUG, "Writing ARRAY var: %s (type=%d, timeout=%d ms)", varname.c_str(), vartype, timeout);
    }
    else {
    	_logger->log(Logger::LOG_LEV_TRACE, "Scalar found!");
   		_logger->log(Logger::LOG_LEV_DEBUG, "Writing SCALAR var: %s (type=%d, timeout=%d ms)", varname.c_str(), vartype, timeout);
    }

    // Retrieve the var type and set the value
    int ErrC = NO_ERROR;
    switch(vartype) {

    	// The IDL_VPTR can contains data as int or long:
		// find the stored value and extract it as a long (RTDB INT_VARIABLE - 32 bits)
    	case INT_VARIABLE:
    		// --- Scalar --- //
    		if(arraySize == 0) {
    			IDL_LONG longValue;
    			if(var->type == IDL_TYP_INT) {
    			_logger->log(Logger::LOG_LEV_DEBUG, "Found int (16 bit)");
    				longValue = var->value.i;
    			}
    			else {
    				_logger->log(Logger::LOG_LEV_DEBUG, "Found long (32 bit)");
    				longValue = var->value.l;
    			}
	    		ErrC = thWriteVar((char*)varname.c_str(), INT_VARIABLE, 1, (void*)(&longValue), timeout);
	    		_logger->log(Logger::LOG_LEV_DEBUG, "Written long (32 bit) var value to RTDB: %d", longValue);
    		}
    		// --- Array --- //
    		else {
    			IDL_LONG* longArray = NULL;
    			IDL_LONG longData[arraySize];
    			if(var->type == IDL_TYP_INT) {
    				_logger->log(Logger::LOG_LEV_DEBUG, "Found array of int (16 bit)");
    				IDL_INT* intArray = (IDL_INT*)(var->value.arr->data);
    				for(int i=0; i<arraySize; i++) {
    					longData[i] = (IDL_LONG)intArray[i];
    					_logger->log(Logger::LOG_LEV_TRACE, "Converted int %d to long %d", intArray[i], longData[i]);
    				}
    				longArray = (IDL_LONG*)longData;
    			}
    			else {
    				_logger->log(Logger::LOG_LEV_DEBUG, "Found array of long (32 bit)");
    				longArray = (IDL_LONG*)(var->value.arr->data);
    			}
    			for(int i=0; i<arraySize; i++) _logger->log(Logger::LOG_LEV_DEBUG, "%d ", longArray[i]);
    			ErrC = thWriteVar((char*)varname.c_str(), INT_VARIABLE, arraySize, (void*)longArray, timeout);
	    		_logger->log(Logger::LOG_LEV_DEBUG, "Written array of %d int (32 bit) values to RTDB", arraySize);
    		}
    		break;

    	// The IDL_VPTR can contains data as double or float:
		// find the stored value and extract it as a double (RTDB REAL_VARIABLE - 64 bits)
		case REAL_VARIABLE:
			if(arraySize == 0) {
				double doubleVal;
				if(var->type == IDL_TYP_FLOAT) {
					_logger->log(Logger::LOG_LEV_DEBUG, "Found float (32 bit)");
					doubleVal = var->value.f;
				}
				else {
					_logger->log(Logger::LOG_LEV_DEBUG, "Found double (64 bit)");
					doubleVal = var->value.d;
				}
				ErrC = thWriteVar((char*)varname.c_str(), REAL_VARIABLE, 1, (void*)(&doubleVal), timeout);
	    		_logger->log(Logger::LOG_LEV_DEBUG, "Written double (64 bit) var value: %f", doubleVal);
			}
			else {
				double* doubleArray = NULL;
    			double doubleData[arraySize];
    			if(var->type == IDL_TYP_FLOAT) {
    				_logger->log(Logger::LOG_LEV_DEBUG, "Found array of float (32 bit)");
    				float* floatArray = (float*)(var->value.arr->data);
    				for(int i=0; i<arraySize; i++) {
    					doubleData[i] = (double)floatArray[i];
    					_logger->log(Logger::LOG_LEV_TRACE, "Converted float %f to double %f", floatArray[i], doubleData[i]);
    				}
    				doubleArray = (double*)doubleData;
    			}
    			else {
    				_logger->log(Logger::LOG_LEV_DEBUG, "Found array of double (64 bit)");
    				doubleArray = (double*)(var->value.arr->data);
    			}
    			for(IDL_LONG i=0; i<arraySize; i++) _logger->log(Logger::LOG_LEV_DEBUG, "%f ", doubleArray[i]);
    			ErrC = thWriteVar((char*)varname.c_str(), REAL_VARIABLE, arraySize, (void*)(doubleArray), timeout);
	    		_logger->log(Logger::LOG_LEV_DEBUG, "Written array of %d double (64 bit) values", arraySize);
    		}
    		break;

		case CHAR_VARIABLE:
			ErrC = thWriteVar((char*)varname.c_str(), CHAR_VARIABLE, var->value.str.slen, (void*)(var->value.str.s), timeout);
    		_logger->log(Logger::LOG_LEV_DEBUG, "Written string var value: %s", var->value.str.s);
    		break;

    	 default:
    		_logger->log(Logger::LOG_LEV_DEBUG, "Wrapper (still) does not support variable type %d [%s:%d]",
                    vartype,__FILE__,__LINE__ );
            ErrC = VAR_TYPE_ERROR;
            break;
    }

    return IDL_GettmpLong(ErrC);
}

/*
 * @Function: readScalarVar
 * Retrieve a scalar value from an RTDB var.
 *
 * Required arguments:
 * 1) Variable name (string)
 * 2) Variable scalar value (multi-type) [Output parameter, allocated by function]
 * 3) Timeout in milliseconds (long)
 *
 * Returns an integer error code.
 * @
 */
IDL_VPTR readScalarVar(int Argc, IDL_VPTR Argv[]) {

    string varname;
    IDL_LONG timeout;
    IDL_VPTR varvalueOut; 	// Output parameter

    MsgBuf *msgb;
    Variable *var;

    // Check if the number of arguments is correct
    if (Argc != 3) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

    // Check if we are connected with the Supervisor
    if (!_idl_connected) return IDL_GettmpLong(MISSING_CLIENTNAME_ERROR);

	// Retrieve parameters
    varname = IDL_VarGetString(Argv[0]);
    varvalueOut = Argv[1];
    timeout = IDL_LongScalar(Argv[2]);

    _logger->log(Logger::LOG_LEV_DEBUG, "Reading scalar var: %s (timeout=%d ms)", varname.c_str(), timeout);

	int ErrC = NO_ERROR;
    msgb = thGetVar((char*)varname.c_str(), timeout, &ErrC);
    if (msgb == NULL) return IDL_GettmpLong(ErrC);
    var = thValue(msgb);

    IDL_VPTR result;
    result = IDL_Gettmp();

    // Retrieve the var type and set the value
    switch(var->H.Type) {
    	case INT_VARIABLE:
    		result->type = IDL_TYP_LONG;
    		result->value.l = var->Value.Lv[0];
    		_logger->log(Logger::LOG_LEV_DEBUG, "Retrieved int (32 bit) var value: %d", result->value.l);
    		break;
		case REAL_VARIABLE:
			result->type = IDL_TYP_DOUBLE;
    		result->value.d = var->Value.Dv[0];
    		_logger->log(Logger::LOG_LEV_DEBUG, "Retrieved double (64 bit) var value: %f", result->value.d);
    		break;
		case CHAR_VARIABLE:
    		result->type = IDL_TYP_STRING;
    		result->value.str.s = var->Value.Sv;
    		result->value.str.slen = var->H.NItems;
    		_logger->log(Logger::LOG_LEV_DEBUG, "Retrieved string var value: %s (size=%d)", result->value.str.s, result->value.str.slen);
    		break;
    }
    IDL_VarCopy(result, varvalueOut);

    // Clean up
    thRelease(msgb);

    return IDL_GettmpLong(NO_ERROR);
}

/*
 * @Function: writeScalarVar
 * Write a scalar value to an RTDB var.
 * Creates it if doesn't exists.
 *
 * Required arguments:
 * 1) variable name (string)
 * 2) variable scalar value (multi-type)
 * 3) variable type (see dlm for available types)
 * 4) timeout in milliseconds (long)
 *
 * Returns an integer error code.
 * @
 */
IDL_VPTR writeScalarVar(int Argc, IDL_VPTR Argv[]) {

    string varname;
    IDL_LONG timeout;
    IDL_VPTR varvalue;
    IDL_LONG vartype;

    // Check if the number of arguments is correct
    if (Argc != 4) return IDL_GettmpLong(IDL_PAR_NUM_ERROR);

    // Check if we are connected with the Supervisor
    if (!_idl_connected) return IDL_GettmpLong(MISSING_CLIENTNAME_ERROR);

	varname = IDL_VarGetString(Argv[0]);
    varvalue = Argv[1];
    vartype = IDL_LongScalar(Argv[2]);
    timeout = IDL_LongScalar(Argv[3]);

   _logger->log(Logger::LOG_LEV_DEBUG, "Writing scalar var: %s (type=%d, timeout=%d ms)", varname.c_str(), vartype, timeout);

    // Retrieve the var type and set the value
    int ErrC = NO_ERROR;
    double doubleVal;
    switch(vartype) {
    	case INT_VARIABLE:
    		ErrC = thWriteVar((char*)varname.c_str(), INT_VARIABLE, 1, (void*)(&varvalue->value.l), timeout);
    		_logger->log(Logger::LOG_LEV_DEBUG, "Written int (32 bit) var value: %d", varvalue->value.l);
    		break;
		case REAL_VARIABLE:
			// The IDL_VPTR can contains data as double or float:
			// find the value and extract it as a double (REAL_VARIABLE)
			doubleVal = varvalue->value.d==0;
			if(doubleVal == 0) {
				doubleVal = varvalue->value.f;
			}
			ErrC = thWriteVar((char*)varname.c_str(), REAL_VARIABLE, 1, (void*)(&doubleVal), timeout);
    		_logger->log(Logger::LOG_LEV_DEBUG, "Written double (64 bit) var value: %f", doubleVal);
    		break;
		case CHAR_VARIABLE:
			ErrC = thWriteVar((char*)varname.c_str(), CHAR_VARIABLE, varvalue->value.str.slen, (void*)(varvalue->value.str.s), timeout);
    		_logger->log(Logger::LOG_LEV_DEBUG, "Written string var value: %s", varvalue->value.str.s);
    		break;
    }

    return IDL_GettmpLong(ErrC);
}


IDL_VPTR setGain_wrap(int lArgc, IDL_VPTR Argv[]) {
    char cmd[1024];
    char CtrlName[] = "idlctrl.R";
    int stat;
    MsgBuf *msgb;
    idl_answer answer;

   if(lArgc == 1) {
      string gainFile  = IDL_VarGetString(Argv[0]);
      _logger->log(Logger::LOG_LEV_DEBUG, "Setting gain file %s", gainFile.c_str());
      sprintf(cmd, "print, fsm_set_gain('%s')", gainFile.c_str());

      stat = thSendMsg( strlen(cmd), CtrlName, IDLCMD, 0, cmd);
      if (IS_ERROR(stat))
         _logger->log(Logger::LOG_LEV_ERROR, "Cannot send command to IDL ctrl: (%d) %s", stat, lao_strerror(stat));
      else {
            unsigned int seqnum = (unsigned)stat;
            int idl_reply_received;
            PrepareIDLAnswer(&answer);

            // Waits for the answer
            do {
               idl_reply_received=0;
               msgb = thWaitMsg( IDLREPLY, CtrlName, 0, 1000, &stat);
               if (IS_ERROR(stat)) {
                  fprintf( stderr, "Error: (%d) %s\n", stat, lao_strerror(stat));
                  return IDL_StrToSTRING((char*)"");
                  }

               // Output and errors from IDL, packed into the same messagge
               if ((HDR_SEQN(msgb)==seqnum) && (HDR_CODE(msgb)==IDLREPLY))
                  if (GetIDLAnswer(msgb, &answer) == NO_ERROR) {
                     printf("%s", answer.error);
                     printf("%s", answer.output);
                     idl_reply_received = 1;
                     }
               }
            while (!idl_reply_received);

            thRelease(msgb);
            FreeIDLAnswer(&answer);
            }
        }
     return IDL_StrToSTRING((char*)"");
}


IDL_VPTR getIfSpec_wrap(int /* lArgc */, IDL_VPTR Argv[]) {

    string ifName = IDL_VarGetString(Argv[0]);

    _logger->log(Logger::LOG_LEV_DEBUG, "Retrieving MAC address and IP for interface for %s", ifName.c_str());

    string mac, ip, netmask;
    int stat = Utils::getMacAddr( ifName, mac, ip, netmask);
    if (!IS_ERROR(stat)) {
        ostringstream oss (ostringstream::out);
        oss << ifName << " ";
        if (mac.compare(""))
            oss << mac << " ";
        else
            oss << "-1 ";

        if (ip.compare(""))
            oss << ip << " ";
        else
            oss << "-1 ";

        if (netmask.compare(""))
            oss << netmask << " ";
        else
            oss << "-1 ";

        return IDL_StrToSTRING((char*)(oss.str().c_str()));

    } else {
        _logger->log(Logger::LOG_LEV_DEBUG, "Unable to open socket for retrieving info to the interface %s: (%d) %s", ifName.c_str(), stat, lao_strerror(stat));
        return IDL_StrToSTRING((char*)"");
   }

}


// ********************************************************************************************************************* //
// ************************************************ DLM INIT SECTION *************************************************** //

int IDL_Load(void) {

	// TODO retrieve in a better way!
	int logLevel = Logger::LOG_LEV_TRACE;

	// Set a temporary log file (before reading conf file)
	_logger = Logger::get(logLevel); // get the MAIN logger!
        _logger->rename("IDL_TEMP", Utils::getAdoptLog());
	_logger->log(Logger::LOG_LEV_INFO, "Loading DLM: IDL_WRAPLIB...");
	_logger->printStatus();

  	// These tables contain information on the functions and procedures
	// that make up the TESTMODULE DLM. The information contained in these
	// tables must be identical to that contained in testmodule.dlm.
	//
   	static IDL_SYSFUN_DEF2 function_addr[] = {

		{ {(IDL_SYSRTN_GENERIC) idl_setup_wrap}, (char*)"IDL_SETUP_WRAP", 1, 1, 0, 0},
		{ {(IDL_SYSRTN_GENERIC) idl_close_wrap}, (char*)"IDL_CLOSE_WRAP", 0, 0, 0, 0},
		{ {(IDL_SYSRTN_GENERIC) idl_log_wrap}, (char*)"IDL_LOG_WRAP", 0, 2, 0, 0},

		{ {(IDL_SYSRTN_GENERIC) idl_getadoptlog}, (char*)"IDL_GETADOPTLOG", 0, 0, 0, 0},
		{ {(IDL_SYSRTN_GENERIC) idl_getadoptside}, (char*)"IDL_GETADOPTSIDE", 0, 0, 0, 0},
		{ {(IDL_SYSRTN_GENERIC) idl_getadoptsubsystem}, (char*)"IDL_GETADOPTSUBSYSTEM", 0, 0, 0, 0},
		
        { {(IDL_SYSRTN_GENERIC) udpConnectionInit_wrap}, 	(char*)"UDPCONNECTION_INIT_WRAP", 	  4, 4, 0, 0},
		{ {(IDL_SYSRTN_GENERIC) udpConnectionDestroy_wrap}, (char*)"UDPCONNECTION_DESTROY_WRAP", 0, 0, 0, 0},
		{ {(IDL_SYSRTN_GENERIC) udpConnectionSend_wrap}, 	(char*)"UDPCONNECTION_SEND_WRAP", 	  2, 2, 0, 0},
		{ {(IDL_SYSRTN_GENERIC) udpConnectionReceive_wrap}, (char*)"UDPCONNECTION_RECEIVE_WRAP", 1, 1, 0, 0},

		{ {(IDL_SYSRTN_GENERIC) adamConnect_wrap}, 	  (char*)"ADAM_CONNECT_WRAP", 	  0, 1, IDL_SYSFUN_DEF_F_KEYWORDS, 0},
		{ {(IDL_SYSRTN_GENERIC) adamDisconnect_wrap}, (char*)"ADAM_DISCONNECT_WRAP", 	  0, 1, IDL_SYSFUN_DEF_F_KEYWORDS, 0},
		{ {(IDL_SYSRTN_GENERIC) adamModbusInit_wrap}, 	  (char*)"ADAM_MODBUS_INIT_WRAP", 	  0, 1, IDL_SYSFUN_DEF_F_KEYWORDS, 0},
		{ {(IDL_SYSRTN_GENERIC) adamModbusDisconnect_wrap}, (char*)"ADAM_MODBUS_DISCONNECT_WRAP", 	  0, 1, IDL_SYSFUN_DEF_F_KEYWORDS, 0},
		{ {(IDL_SYSRTN_GENERIC) adamCmd_wrap}, 	      (char*)"ADAM_CMD_WRAP", 	  3, 3, IDL_SYSFUN_DEF_F_KEYWORDS, 0},
		{ {(IDL_SYSRTN_GENERIC) adamModbusReadInput_wrap}, 	      (char*)"ADAM_MODBUS_RI_WRAP", 	  0, 0, IDL_SYSFUN_DEF_F_KEYWORDS, 0},
		{ {(IDL_SYSRTN_GENERIC) adamModbusReadOutput_wrap}, 	      (char*)"ADAM_MODBUS_RO_WRAP", 	  0, 0, IDL_SYSFUN_DEF_F_KEYWORDS, 0},
		{ {(IDL_SYSRTN_GENERIC) adamModbusWriteStructure_wrap}, 	      (char*)"ADAM_MODBUS_WS_WRAP", 	  1, 1, IDL_SYSFUN_DEF_F_KEYWORDS, 0},
		{ {(IDL_SYSRTN_GENERIC) adamLed}, 	      (char*)"ADAM_LED", 	  1, 1, IDL_SYSFUN_DEF_F_KEYWORDS, 0},

		{ {(IDL_SYSRTN_GENERIC) reset_devices_wrap},   			 (char*)"RESET_DEVICES_WRAP", 			 6, 6, 0, 0},
      	{ {(IDL_SYSRTN_GENERIC) reset_devices_multi_wrap}, 	     (char*)"RESET_DEVICES_MULTI_WRAP", 	 7, 7, 0, 0},
      	{ {(IDL_SYSRTN_GENERIC) reset_devices_multi_check_wrap}, (char*)"RESET_DEVICES_MULTI_CHECK_WRAP", 8, 8, 0, 0},

		{ {(IDL_SYSRTN_GENERIC) shm_readbuf_wrap},  (char*)"SHM_READBUF_WRAP", 4, 4, 0, 0},

      	{ {(IDL_SYSRTN_GENERIC) IDLGetDiagnParam}, 	         (char*)"GETDIAGNPARAM" , 5, 5, IDL_SYSFUN_DEF_F_KEYWORDS , 0},
      	{ {(IDL_SYSRTN_GENERIC) IDLSetDiagnParam}, 	         (char*)"SETDIAGNPARAM" , 4, 5, IDL_SYSFUN_DEF_F_KEYWORDS , 0},
      	{ {(IDL_SYSRTN_GENERIC) IDLGetDiagnValue}, 	         (char*)"GETDIAGNVALUE" , 5, 5, IDL_SYSFUN_DEF_F_KEYWORDS , 0},
      	{ {(IDL_SYSRTN_GENERIC) IDLGetDiagnBuffer},	         (char*)"GETDIAGNBUFFER", 5, 5, IDL_SYSFUN_DEF_F_KEYWORDS , 0},
      	{ {(IDL_SYSRTN_GENERIC) IDLDumpDiagnParams},  	     (char*)"DUMPDIAGNPARAMS", 2, 2, IDL_SYSFUN_DEF_F_KEYWORDS , 0},
      	{ {(IDL_SYSRTN_GENERIC) IDLMastdiagnDumpHistory},    (char*)"DUMP_DIAGN_HISTORY", 1, 1, 0, 0},
      	{ {(IDL_SYSRTN_GENERIC) IDLLoadOfflineDiagnFrames},  (char*)"LOAD_OFFLINE_DIAG_FRAMES", 2, 2, 0 , 0},

      	{ {(IDL_SYSRTN_GENERIC) IDLSetLogLevel},   	(char*)"SETLOGLEVEL", 	  2, 3, IDL_SYSFUN_DEF_F_KEYWORDS , 0},
      	{ {(IDL_SYSRTN_GENERIC) IDLGetLogLevel},   	(char*)"GETLOGLEVEL", 	  2, 2, IDL_SYSFUN_DEF_F_KEYWORDS , 0},

      	{ {(IDL_SYSRTN_GENERIC) getBcuId_wrap},            (char*)"GETBCUID_WRAP",  		   1, 1, 0 , 0},
      	{ {(IDL_SYSRTN_GENERIC) getBcuIp_wrap},   		   (char*)"GETBCUIP_WRAP",            1, 1, 0 , 0},
      	{ {(IDL_SYSRTN_GENERIC) getIfSpec_wrap},   		   (char*)"GETIFSPEC_WRAP",            1, 1, 0 , 0},
      	{ {(IDL_SYSRTN_GENERIC) getBcuMasterUdpPort_wrap}, (char*)"GETBCUMASTERUDPPORT_WRAP", 1, 1, 0 , 0},
      	{ {(IDL_SYSRTN_GENERIC) isBcuReady_wrap}, (char*)"ISBCUREADY", 1, 2, IDL_SYSFUN_DEF_F_KEYWORDS , 0},

      	{ {(IDL_SYSRTN_GENERIC) read_seq_multi_wrap},   (char*)"READ_SEQ_MULTI_WRAP",   9, 9, 0 , 0},
      	{ {(IDL_SYSRTN_GENERIC) write_same_multi_wrap}, (char*)"WRITE_SAME_MULTI_WRAP", 10, 10, 0 , 0},
      	{ {(IDL_SYSRTN_GENERIC) write_seq_multi_wrap},  (char*)"WRITE_SEQ_MULTI_WRAP",  10, 10, 0 , 0},

      	{ {(IDL_SYSRTN_GENERIC) read_seq_wrap},   (char*)"READ_SEQ_WRAP",   8, 8, 0 , 0},
      	{ {(IDL_SYSRTN_GENERIC) write_same_wrap}, (char*)"WRITE_SAME_WRAP", 9, 9, 0 , 0},
      	{ {(IDL_SYSRTN_GENERIC) write_seq_wrap},  (char*)"WRITE_SEQ_WRAP",  9, 9, 0 , 0},

      	{ {(IDL_SYSRTN_GENERIC) readScalarVar}, (char*)"READ_SCALAR_VAR_WRAP", 3, 3, 0 , 0},
      	{ {(IDL_SYSRTN_GENERIC) writeScalarVar},  (char*)"WRITE_SCALAR_VAR_WRAP",  4, 4, 0 , 0},
      	{ {(IDL_SYSRTN_GENERIC) readVar}, (char*)"READ_VAR_WRAP", 3, 3, 0 , 0},
      	{ {(IDL_SYSRTN_GENERIC) writeVar}, (char*)"WRITE_VAR_WRAP", 4, 4, 0 , 0},

	    { {(IDL_SYSRTN_GENERIC) IDL_hexmoveto}, (char*)"HEXMOVETO", 1, 1, IDL_SYSFUN_DEF_F_KEYWORDS, 0},
	    { {(IDL_SYSRTN_GENERIC) IDL_hexmoveby}, (char*)"HEXMOVEBY", 1, 1, IDL_SYSFUN_DEF_F_KEYWORDS, 0},
	    { {(IDL_SYSRTN_GENERIC) IDL_hexmovsph}, (char*)"HEXMOVESPH",1, 1, IDL_SYSFUN_DEF_F_KEYWORDS, 0},
	    { {(IDL_SYSRTN_GENERIC) IDL_hexbrake},  (char*)"HEXBRAKE",  0, 0, IDL_SYSFUN_DEF_F_KEYWORDS, 0},
	    { {(IDL_SYSRTN_GENERIC) IDL_hexhome},   (char*)"HEXHOME",  0, 0, 0, 0},

	    { {(IDL_SYSRTN_GENERIC) setGain_wrap},   (char*)"SET_AO_GAIN",  1, 1, 0, 0},

	    { {(IDL_SYSRTN_GENERIC) idl_4D_init}, (char*)"IDL_4D_INIT", 0, 0, 0, 0},
	    { {(IDL_SYSRTN_GENERIC) idl_4D_finalize}, (char*)"IDL_4D_FINALIZE", 0, 0, 0, 0},
	    { {(IDL_SYSRTN_GENERIC) idl_4D_getMeasure}, (char*)"IDL_4D_GET_MEASURE", 1, 1, 0, 0},
   		{ {(IDL_SYSRTN_GENERIC) idl_4D_getBurst}, (char*)"IDL_4D_GET_BURST", 1, 2, 0, 0},
   		{ {(IDL_SYSRTN_GENERIC) idl_4D_setTrigger}, (char*)"IDL_4D_SET_TRIGGER", 1, 1, 0, 0},
   		{ {(IDL_SYSRTN_GENERIC) idl_4D_setPostProc}, (char*)"IDL_4D_SET_POSTPROC", 1, 1, 0, 0},
		{ {(IDL_SYSRTN_GENERIC) idl_4D_trigger}, (char*)"IDL_4D_TRIGGER", 0, 0, 0, 0},
		{ {(IDL_SYSRTN_GENERIC) idl_4D_postProcess}, (char*)"IDL_4D_POSTPROCESS", 2, 3, 0, 0},
   		{ {(IDL_SYSRTN_GENERIC) idl_4D_capture}, (char*)"IDL_4D_CAPTURE", 1, 2, 0, 0},
	    { {(IDL_SYSRTN_GENERIC) idl_4D_produce}, (char*)"IDL_4D_PRODUCE", 1, 1, 0, 0},

		{ {(IDL_SYSRTN_GENERIC) lao_strerror_wrap}, (char*)"LAO_STRERROR_WRAP", 1, 1, 0, 0},
   	};

   	static IDL_SYSFUN_DEF2 procedure_addr[] = {
   	//   { (IDL_SYSRTN_GENERIC) testpro, "TESTPRO", 0, IDL_MAX_ARRAY_DIM, 0, 0},
  	 };

   	// Create a message block to hold our messages. Save its handle where
   	// the other routines can access it.
	if (!(msg_block = IDL_MessageDefineBlock((char*)"idlwraplib", IDL_CARRAY_ELTS(msg_arr), msg_arr))) return IDL_FALSE;

   	// Register our routine. The routines must be specified exactly the same
   	// as in testmodule.dlm.
   	return IDL_SysRtnAdd(function_addr, TRUE, IDL_CARRAY_ELTS(function_addr)) &&
      IDL_SysRtnAdd(procedure_addr, FALSE, IDL_CARRAY_ELTS(procedure_addr));
}

