//@File: AOApp.cpp
//
// Implementation of the \aoapp class
//@

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <signal.h>
#include <stdio.h>


extern "C"{
#include "base/thrdlib.h"
}

#include "AOApp.h"
#include "AOStates.h"
#include "AOExcept.h"

#include "AOStates.c"



// ------------------ DEFAULT VALUES ---------------- //
// These values can safely be changed
int    AOApp::CONFIG_LOADER_LOG_LEVEL = Logger::LOG_LEV_INFO;
int    AOApp::DEFAULT_LOG_LEVEL       = Logger::LOG_LEV_INFO;
string AOApp::DEFAULT_LOG_PATH        = "/tmp";   // Warning: can't use Logger::LOG_PATH_DEFAULT since it isn't
                                                  // const, and will cause a static initialization "fiasco" !!!


// ----------------- STATIC FIELDS ------------------ //

AOApp* AOApp::_instance = NULL;


RTDBvar AOApp::_VarStatus;
RTDBvar AOApp::_VarErrmsg;

bool 	AOApp::_timeToDie;
bool  AOApp::_simulation;
Logger* AOApp::_logger = NULL;

Config_File* AOApp::_cfg = NULL;

std::string  AOApp::_configFile;
std::string  AOApp::_logFile;
std::string  AOApp::_myName;
std::string  AOApp::_logdir;
std::string  AOApp::_myFullName;
std::string  AOApp::_server;
std::string  AOApp::_side;
bool		 AOApp::_ready;

bool AOApp::_install_sighand=true;  // Flag to enable installation of signal handlers

std::map<std::string, AOApp::v_hndl> AOApp::_var_handlers;
std::vector<string> AOApp::_keywords;


// -------------------------------------------------- //


//@Member: Usage

// Print usage message to stdout and exit
//@

void AOApp::usage()
{
   string vId;
   vId=GetVersionID();
   cout << endl << "AOApp: " << _myName << " " << vId << endl << endl;
   cout << endl << "Standard options:" << endl << endl;
   cout << "   -i <identity>: MsgD identity" << endl;
   cout << "   -f <file>    : configuration file (if not specified, takes the identity as prefix)" << endl;
   cout << "   -s <server>  : server (overrides configuration file specification)" << endl;
   cout << "   -l <side>    : side (R or L) (overrides ADOPT_SIDE environment variable)" << endl;
   cout << "   -S           : set simulation mode. The only effect is that method isSimulation()" << endl;
   cout << "                  returns true." << endl;
   cout << "   -n           : Do not open a log file" << endl;
   cout << "   -v           : increase verbosity over default level (can appear multiple times)" << endl;
   cout << "   -q           : decrease verbosity from default level (can appear multiple times)" << endl;
   cout << endl;
}


//@Member: AOApp
//
// Constructor
//
// Initialize members with basic parameters.
// The log level is set to default
//
//@
AOApp::AOApp(string name,
             string side,
             string serverIp,
             int logLevel,
             bool inst_sighand,
             string logDir     )   // with "" uses default mechanism
                                   // Use "NONE" to disable logging
    throw (AOException) {

    // Before all, temporary initialize the singleton logger (named "MAIN")
    initTempLog(logDir);
    
    // Init the AOApp status
    if (_cfg != NULL) delete _cfg;
    _cfg             = NULL;
    _myName          = name;
    _side            = side;
    _server          = serverIp;
    _install_sighand = inst_sighand;

    _simulation=false;

    // Complete the initialization
    CreateApp(logLevel);
}


//@Member: AOApp
//
// Constructor
//
// Initialize members with specified values.
// Note that the "logLevel" parameter is used only to set the logging level
// for the stdconfig library, and then ignored if the config file specify a
// "LogLevel" entry.
//@
AOApp::AOApp(string name, const std::string& conffile)
    throw (AOException) {

    _myName          = name;

    // Before all, temporary initialize the singleton logger (named "MAIN")
    initTempLog("");
    
    _simulation=false;
    _server="";
    
    // Init the AOApp status using the configuration file
    if (_cfg != NULL) delete _cfg;
    _cfg = NULL;
    try {
	SetConfigFile(conffile);
    }
    catch(Config_File_Exception &e) {
        // This is a fatal error
        throw;
    }
    _side = Utils::getAdoptSide();
    
    // Get the log level
    int logLevel;

    try {
        // Try to get the level from config file...
        if(_cfg != NULL)  {
	    logLevel = Logger::stringToLevel((*_cfg)["LogLevel"]);
	}
    }
    catch(Config_File_Exception& e) {
        //...otherwise use a default value
        logLevel=Logger::LOG_LEV_INFO;
    }
    // Complete the initialization
    CreateApp(logLevel);
}

//@Member: AOApp
//
// Constructor overload
//
// Initializes an AOApp taking standard parameters from command line:
//
//
//@
AOApp::AOApp( int argc,
              char **argv)
    throw (AOException)
{
    char c;

    // Init the AOApp status using the command line
    std::string conffile = "";
    std::string server = "";
    int log_modifier = 0;
    string logDir = "";
    _simulation=false;
   _side = Utils::getAdoptSide();

    opterr = 0;
    while ((c = getopt (argc, argv, "i:l:f:s:hSvqn")) != -1)
	switch (c) {
   case 'i':
       _myName = optarg;
       break;
	case 'l':
	    logDir = optarg;
	    break;
	case 'f':
	    conffile = optarg;
	    break;
	case 's':
	    server = optarg;
	    break;
	case 'S':
	    _simulation=true;
	    break;
	case 'v':
	    log_modifier++;
	    break;
	case 'q':
	    log_modifier--;
	    break;
	case 'n':
	    logDir="NONE";
	    break;
   case 'h':
       usage();
       exit(0);
	case '?':
	    // Do not want to exit in case of unknown option: can be a valid
	    // option for a subclass!!!
	    //if (isprint (optopt))
	    //    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
	    //else
	    //    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
	    //exit(0);
	    break;
	   
	    // Do not want to exit in case of unknown option: can be a valid
	    // option for a subclass!!!
	    //default:
	    //exit(0);
	}
   
    // Before all, instantiate the singleton logger (named "MAIN")
    initTempLog(logDir);
   
    _server = server;

   // If no config. file specified, use the identity to find it
   if (conffile == "")
      conffile = getConffile(_myName);


	if (server != "")
      _server = server;

	try {
		SetConfigFile(conffile);
	}
	catch(Config_File_Exception &e) {
		// This is a fatal error
	    throw;
	}

    // Get the logging level
    int llevel=Logger::LOG_LEV_INFO;
    if(log_modifier > 0) llevel=Logger::LOG_LEV_INFO + log_modifier;

    // Complete the initialization
    CreateApp(llevel);
}

void AOApp::setLogFile(string fileName, bool renaming) {
    // Find the log path
    if(_logdir=="NONE") {
        Logger::disableFileLog();
        return;
    }
    if(_logdir=="") _logdir = Utils::getAdoptLog();
    if(_logdir == "") _logdir = DEFAULT_LOG_PATH;
  
    if (renaming) {
    	_logger->rename( fileName, _logdir);
    }
    else {
    	_logger->change( fileName, _logdir);
    }
}

string AOApp::getConffile( string identity) {
    return Utils::getConffile(identity);
}


void AOApp::initTempLog( string logDir ) {

    _logdir=logDir;

    if(_logdir=="NONE")
       Logger::disableFileLog();
    else
       Logger::enableFileLog();

    // Before configuration read, the logging file name is unknown:
    // a temp name is used, and then the method CreateApp(), which
    // know the correct name will rename it

    _logger = Logger::get();
    //setLogFile(Utils::uniqueFileName("aoapp_"), true);
    setLogFile(Utils::uniqueFileName(), false);
}


void AOApp::CreateApp(int logLevel) throw (AOException)
{
    _ready = false;
    _timeToDie = false;

    // Connection retry parameters
    _ntry=1;
    _incr=1;
    _looptime=1000000;

    SetMyName(_myName);

    Logger::setParentName(_myFullName);

    // This way the log file is unique
    _logFile = _myFullName;

    ////////////////// INIT THE LOGGER /////////////////

    // --- Setup the LOGGER POOL --- //

    // Setup the logging path
    setLogFile(_logFile, true);


    // --- Setup the logger MAIN --- //

    _logger->setGlobalLevel(logLevel);

    ////////////////// INIT OTHERS FEATURES /////////////////

    if(_install_sighand) {
      signal(SIGINT, sighand);
      signal(SIGTERM, sighand);
    }

    // Set the singleton
    _instance = this;
}

void AOApp::SetMyName( const std::string& name) {
    _myName = name;

    if(!_side.empty())
 	   _myFullName = _myName + "." + _side;
    else
   	   _myFullName = _myName;
}


//@Member: $\sim$ AOApp
//
//Standard Destructor
//
//Completely clean the application:
// - Close MsgD communication
// - Destroy the loggers pool
//@
AOApp::~AOApp()
{
    // Note: it doesn't set its status, because the RTDBAVar 'STATUS' wouldn't
    //        have any meaning if the AOApp is dead! The clients must consider
    //		  the RTDBVar 'CLSTAT' instead.

    // Close MsgD communication
    thClose();
    _logger->log(Logger::LOG_LEV_INFO, "Communication with MsgD stopped");

    // Clean up all thLib handlers
    thCleanup();
    _logger->log(Logger::LOG_LEV_DEBUG, " *** %s SUCCESFULLY CLEANED-UP AND TERMINATED ***", _myName.c_str());

    // Destroy the loggers pool
    Logger::destroy();

    // 
    if (_cfg != NULL) {
	delete _cfg;
	_cfg = NULL;
    }

}

//@Member: sighand
//
//Signal handler for terminating the program:
// - SIGINT: Ctrl-C
// - SIGTERM: kill [-15]
// - SIGUSR1: GUI stop process (kill -30)
//@
void AOApp::sighand(int thesig)
{
    if (thesig==SIGINT || thesig==SIGTERM) {
        _logger->log(Logger::LOG_LEV_INFO, "Signal %d received. It's timeToDie.", thesig);

        // Tell to all threads (including the main one that will destroy
        // the AOApp) to terminate
        SetTimeToDie(true);
    }
}


//@Member: SetConfigFile
//
// Set the config file and read it, creating the config dictionary.
//
// At least \verb+Server+ and must be defined in
// \verb+_config_file+. See \stdconfig\ documentation for how to
// write a config file.
//@
void AOApp::SetConfigFile(const std::string& conffile) throw (Config_File_Exception)
{

    _configFile = conffile;

    if (_cfg != NULL) delete _cfg;

    try {
      _cfg         = new Config_File(_configFile);
      _server      = (std::string)(*_cfg)["Server"];
    }
    catch (Config_File_Exception &e) {
        Logger::get()->log(Logger::LOG_LEV_ERROR, e.what().c_str());
        SetTimeToDie(true);	// Since this method is called in the constructor, this is not useful
        throw;
    }
    // Now get optional parameters
    try {
        _logdir      = (std::string)(*_cfg)["LogDir"];
    }
    catch (Config_File_Exception &e) {
        _logdir = std::string();
    }
    try {
        int isSimul=(int)(*_cfg)["Simulation"];
        if(isSimul==1) _simulation=true;
    }
    catch (Config_File_Exception &e) {
    }
    if(_server.empty()) {
        try {
            _server      = (std::string)(*_cfg)["Server"];
        }
        catch (Config_File_Exception &e) {
	    _server="127.0.0.1";
        }
    }
}


//@Member: InitBase
//
// Initialize the \aoapp\ application
//
// This routine initializes an \aoapp\ application.
// Prior to calling this routine, \verb+_myName+ must be set.
//
//@
void AOApp::InitBase()
{
    _logger->log(Logger::LOG_LEV_DEBUG, "Init thrdlib, myName is %s ...", _myFullName.c_str());
    int stat=thInit(_myFullName.c_str());

    if (IS_ERROR(stat)) {
        if (stat == THRD_INITIALIZE_ERROR)
            _logger->log(Logger::LOG_LEV_WARNING, "thrdlib seems to be already initialized ");
        else {
            _logger->log(Logger::LOG_LEV_ERROR, "Error %d (%s) from thInit(): time to die", stat, lao_strerror(stat));
            SetTimeToDie(true);
        }
    }

}

//@Member: Connect
//
// Connect to \msgd.
//
// This routine creates the connection with the \msgd.
// Prior to calling this routine, \verb+_server+  must be set.
//
//@
int AOApp::ConnectBase()
{
    int stat,n;
    _logger->log(Logger::LOG_LEV_INFO, "Starting thrdlib. Server is @ %s",(char*)_server.c_str());
    if (_ntry== 0)
	throw AOException( "AOApp::ConnectBase():  _ntry is zero (and shouldn't be), cannot connect", NO_ERROR);

    for(n=0;n<_ntry;n+=_incr) {
        if(TimeToDie()) { stat=NO_ERROR; break; }
        if (IS_NOT_ERROR(stat=thStart((char*)_server.c_str(),1))) break;
        if(_ntry>1) nusleep(_looptime);
    }
    if (IS_ERROR(stat)) {
        if(_ntry>1) stat=TIMEOUT_ERROR;
        _logger->log(Logger::LOG_LEV_ERROR, "Error %d (%s) from thStart()", stat, lao_strerror(stat));
        SetTimeToDie(true);
    }
    return stat;
}

//@Member: SetupVars
//
// Create and initialize some standard \RTDB\ variables.
//
// This routine creates and initialize some standard \RTDB\  variables common to all \aoapp s.
//
// Prior to calling this routine, \verb+_myName+ and \verb+_VarPrefix+ must be set.
// The following \RTDB\ variables are created:
//
//
// \begin{tabular}{llll}
// ``STATUS'' & char & private & ``Undefined'' \\*
// ``ERRMSG'' & char & private & ``OK'' \\*
// \end{tabular}
//
//@
void AOApp::SetupVarsBase()
{
    _logger->log(Logger::LOG_LEV_DEBUG, "Setting up rtdb variables ...");
    std::string errmsg="OK";

    try{
        _VarStatus = RTDBvar( _myFullName, string("STATUS"), NO_DIR);
        setCurState(STATE_UNDEFINED);

        _VarErrmsg = RTDBvar( _myFullName, string("ERRMSG"), NO_DIR, CHAR_VARIABLE, errmsg.length());
        _VarErrmsg.Set(errmsg);
    } catch (AOException &e) {
        _logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
        throw(e);
    }
}




//@Member: InstallHandlers
//
// Install common AOApp handlers.
//
// This routine installs in \thrdl\ some default message handlers
// whose functionality is supposed to be common to every \aoapp\ application.
//
// The following message handlers are installed:
// %\begin{itemize}
// %\item terminate_handler
// %\item logincr_handler
// %\item logdecr_handler
// %\item varchangd_handler
// %\end{itemize}
//
// NOTE: The behaviour of the default handler can be overridden by registering
// another handler that matches the same messages than the default handler and
// implements a different reaction. When a message is received, \thrdl\ executes
// the first handler that matches the message, hence it is sufficient to register
// the new handler overriding in the derived application
// \verb+InstallHandlers()+  that is called before \verb+InstallHandlersBase()+
// by \verb+Exec()+.
//
//
// See \thrdl\ documentation for the details.
//@

void AOApp::InstallHandlersBase()
{
    int stat;;
    _logger->log(Logger::LOG_LEV_DEBUG, "Installing default handlers ...");
    if((stat=thHandler(TERMINATE,(char *)"*",0,terminate_handler,(char *)"terminate", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
        SetTimeToDie(true);
    }
    if((stat=thHandler(AO_SETLOGLEV, (char *)"*", 0, setloglev_handler, (char *)"setloglev", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
        SetTimeToDie(true);
    }
    if((stat=thHandler(AO_GETLOGNAMELEV, (char *)"*", 0, getlognamelev_handler, (char *)"getlognamelev", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
        SetTimeToDie(true);
    }
    if((stat=thHandler(VARCHANGD, (char *)"*", 0, varchangd_handler, (char *)"varchangd", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
        SetTimeToDie(true);
    }
}

//@Function: terminate_handler
//
// Function called by \thrdl\ in response to a TERMINATE message
//
// Set \verb+_timeToDie+ true. The threads must periodically call {\tt TimeToDie()} to properly stop
// the application.
//
// It is declared static since it's a C function, as needed by thHandler.
// See @see{InstallHandlers}.
//@
int AOApp::terminate_handler(MsgBuf *msgb, void *argp, int /*hndlrQueueSize*/)
// @R: NO_ERROR
{
    AOApp *pt = (AOApp*)argp;
    Logger::get()->log(Logger::LOG_LEV_FATAL, "%s terminated by %s", pt->_myFullName.c_str(), HDR_FROM(msgb));
    thRelease(msgb);
    SetTimeToDie(true);
    return NO_ERROR;
}

//@Function: setloglev_handler
//
// Function called by \thrdl\ in response to a AO_SETLOGLEV message
//
// Modify the logging level of a logger.
//
// It is declared static since it's a C function, as needed by thHandler.
// See @see{InstallHandlers}.
//@

#include <boost/archive/binary_iarchive.hpp>
#include <sstream>
#include <string>
#include "aoapplib.h"
int AOApp::setloglev_handler(MsgBuf *msgb, void * /*argp*/, int /*hndlrQueueSize*/)
// @R: NO_ERROR
{
    LogLevelModifier llm;

    _logger->log(Logger::LOG_LEV_DEBUG, "received AO_SETLOGLEV message [%s:%d]");

    unsigned int len = HDR_LEN(msgb);
    std::string sbuf( (const char *)(MSG_BODY(msgb)), len);
    std::istringstream iss(sbuf);
    boost::archive::binary_iarchive ia(iss);
    ia >> llm;
    if ( Logger::exists(llm.Logger()) ) {
        Logger::get(llm.Logger())->setGlobalLevel(llm.Level());
        _logger->log(Logger::LOG_LEV_INFO, "log level of %s changed to %s by %s",
		     llm.Logger().c_str(), logDescr( Logger::get(llm.Logger())->getLevel() ), HDR_FROM(msgb));
    }
    thRelease(msgb);
    return NO_ERROR;
}

//@Function: getlognamelev_handler
//
// Function called by \thrdl\ in response to a AO_GETLOGNAMELEV message
//
// Provides to the remote client a dictionary of names and levels of the logger pool.
//
// It is declared static since it's a C function, as needed by thHandler.
// See @see{InstallHandlers}.
//@

#include <boost/archive/binary_oarchive.hpp>
#include <sstream>
#include <string>
#include "aoapplib.h"
int AOApp::getlognamelev_handler(MsgBuf *msgb, void * /*argp*/, int /*hndlrQueueSize*/)
// @R: NO_ERROR
{
    _logger->log(Logger::LOG_LEV_DEBUG, "received AO_GETLOGNAMELEV message [%s:%d]", __FILE__, __LINE__);
    NameLevelDict mappa = Logger::getNamesAndLevels();


    std::ostringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << mappa;

    std::string outbuf = oss.str();
    int  lungh = outbuf.size();
    const char* buffer = outbuf.c_str();

    _logger->log(Logger::LOG_LEV_DEBUG, "AO_GETLOGNAMELEV reply to %s (len %d) [%s:%d]",HDR_FROM(msgb), lungh, __FILE__, __LINE__ );

    int stat = thReplyMsg(ACK, lungh, (void*)(buffer), msgb);
    if(IS_ERROR(stat)){
        _logger->log(Logger::LOG_LEV_ERROR, "Error in getlognamelev_handler(): %s [%s:%d]",lao_strerror(stat), __FILE__, __LINE__);
        return stat;
        //Dont throw any exception and simply go on
    }
    _logger->log(Logger::LOG_LEV_DEBUG, "AO_GETLOGNAMELEV all done [%s:%d]", __FILE__, __LINE__ );
    return NO_ERROR;
}


//@Function: varchangd_handler
//
// Function called by \thrdl\ in response to a VARCHANGD message
//
// Call the appropriate variable handler, or the default \verb+VariableHandler()+ if no handler
// was specified for the variable. Handlers are installed with the \verb+Notify()+ function.
//
// It is declared static since it's a C function, as needed by thHandler.
// See @see{InstallHandlers}.
//@
int AOApp::varchangd_handler(MsgBuf *msg, void *argp, int /*hndlrQueueSize*/)
{
    Variable *TheVar;
    std::string varname;
    AOApp *pt = (AOApp*)argp;

    TheVar = (Variable *)MSG_BODY(msg);
    varname = TheVar->H.Name;

    Logger::get()->log(Logger::LOG_LEV_DEBUG, "Calling variable handler for %s", varname.c_str() );
    int ret;
    try {
	if (pt->_var_handlers.find(varname) != pt->_var_handlers.end()) {
	    ret = (pt->_var_handlers[varname])(argp, TheVar);
	}
	else {
	    // Test wildcards
//         std::map<std::string, AOApp::v_hndl>::iterator iter;
//         for (iter = pt->_var_handlers.begin(); iter != pt->_var_handlers.end(); iter++) {
//            if (iter->first.compare( iter->first.size()-1, 1, "*"));

	    ret = pt->VariableHandler(TheVar);
	}
    } catch (AOException &e) {
	Logger::get()->log(Logger::LOG_LEV_ERROR, "Variable handler error: %s", e.what().c_str());
	ret = TBD_ERROR;
    }

    thRelease(msg);
    return ret;
}

//@Member: eNotify
//
// Request notification on a variable
//
// Ask MsgD-RTDB to be notified when a variable is changed, and optionally
// install a handler for the message. Returns error code.
//
// Use Notify for an exception based error management.

// See Notify for a description.
//
// @

int AOApp::eNotify( RTDBvar &var, v_hndl f, bool getNow)
{
    return eNotify(var.complete_name(), f, getNow);
}

int AOApp::eNotify( string varname, v_hndl f, bool getNow)
{
   int stat=NO_ERROR;

   _logger->log(Logger::LOG_LEV_DEBUG, "AOApp::Notify %s",(char *)varname.c_str() );
   if (IS_ERROR(stat = thVarOp( (char *)varname.c_str(), VARNOTIF, 1, 1000))) {
      _logger->log(Logger::LOG_LEV_ERROR, "Error asking for variable notification: %s",varname.c_str());
      return stat;
   }

   if (f) _var_handlers[varname] = f;

   if (getNow) {
      MsgBuf *msgb = thGetVar(varname.c_str(), 1000, &stat);
      if (msgb) 
         f(dynamic_cast<AOApp*>(this), (Variable*) MSG_BODY(msgb));
         thRelease(msgb);
   }
   return stat;
}


//@Member: Notify
//
// Request notification on a variable
//
// Ask MsgD-RTDB to be notified when a variable is changed, and optionally
// install a handler for the message.
//
// A handler may be a static member function, or an ordinary C function:
//
//   static int DerivedApp::handler( void *pt, MsgBuf *msg);
//   int handler( void *pt, MsgBuf *msg);
//
// In both cases, the first argument is a pointer to the AOApp (or derived) object
// that installed the handler.
//
// If no handler is specified, or if it is NULL, no handler is installed and the
// default function \verb+VariableHandler\+ is called instead.
//
// @
void AOApp::Notify( RTDBvar &var, v_hndl f, bool getNow)
{
    Notify(var.complete_name(), f, getNow);
}

void AOApp::Notify( string varname, v_hndl f, bool getNow)
{
   int stat;

   stat=eNotify(varname,f,getNow);

   if (IS_ERROR(stat)) throw AOException("Error asking for variable notification "+varname, stat);

}


//@Member: waitForMsgD
//
//  Modifies the mode of connection to the MSGD
//
//  Default connection mode is to attempt a connection with the MsgD and return
//  an error if the connection fails.

//  By calling  waitForMsgD, the connection will be retried a given number of times
//  before returning an error.

// Default values correspond to waiting forever, attempting to connect once per second.
//@

void AOApp::waitForMsgD(int ntry,     // @P{ntry}: Number of connection retries
                                      // Default=0: loop forever
                        int looptime) // @P{looptime}: Loop period in milliseconds. Default=1000
{
    if(ntry<=0) { 
	_ntry=2; 
	_incr=0; 
    } else {
	_ntry=ntry;
	_incr=1;
    }
    _looptime=looptime*1000;
    if(_looptime<200000) _looptime=200000;
}

//@Member: ImReady
//
// Inform the rest of the world that we are ready and
// set a flag, that can be read using isReady() (see).
//
// This routine sends a \tt{SETREADY} command to \msgd.
// @
void AOApp::SetReady()
{
    while(!_ready) {
	if(!IS_ERROR(thSendMsg(0, (char *)"", SETREADY, 0, NULL))) {
	    _ready = true;
	}
    }
}


//@Member: Run
//
// Do real stuff in here
//
// Server-type operations (reacting to messages) should be done in the Listening thread
// @
void AOApp::Run()
{
    while((TimeToDie()==false) && (thInfo()->connected))
	{
	    msleep(3000);
	    setCurState( getCurState());
	}
}


//@Member: Exec
//
// Exec an \aoapp\ application
//
// After setting up parameters with Set members and having overridden
// the needed functions in the derived class, call Exec() to run the LBT-AO component.
//
// This function calls the \aoapp\ member functions in the good order.
// 
// @
void AOApp::Exec( bool retryClientname) // @P{retryClientname}: if True, the function will try to change
// the clientname if the MsgD-RTDB connection is refused.
{
    string myname = MyName();

    bool loop =true;
    int count=0, ntry=50;
    while(loop && (count++ < ntry)) {

	//////////// Initialize
	if (TimeToDie() == false) InitBase();

	//////////// Install handlers
	if (TimeToDie() == false) InstallHandlers();
	if (TimeToDie() == false) InstallHandlersBase(); //keep it after user derived handlers

	//////////// Connect to MsgD-RTDB
	if (TimeToDie() == false) {
	    int stat = ConnectBase();

	    if ( TimeToDie() && retryClientname && (stat == DUPLICATE_CLIENT_ERROR) ) { // ConnectBase() failed with duplicated name
            thCleanup();
            ostringstream oss;
            oss << myname << count;
            SetMyName( oss.str());
            SetTimeToDie(false);
        }
	    else
            loop = false;
	}
    }


    ///////////////////   Create variables
    if (TimeToDie() == false) SetupVarsBase();
    if (TimeToDie() == false) SetupVars();

    ///////////////////   Post init
    if (TimeToDie() == false) PostInit();

    /////////////////  Inform the rest of the world that we are ready
    if (TimeToDie() == false) SetReady();

    /////////////// Now we are up and running. Do our stuff here
    if (TimeToDie() == false) {
    	Run();
    }
    else {
    	_logger->log(Logger::LOG_LEV_FATAL, "Some problem occurred (i.e. MsgD offline): AOApp not started (run not executed)");
    }
}

void AOApp::setCurState( int state)
{
    static int old_state = STATE_INVALID;

    if (state != old_state)
	{
	    _logger->log( Logger::LOG_LEV_DEBUG, "State change from %s to %s",
			  lao_strtable( old_state, AOStatesTable, AOStatesTabLen),
			  lao_strtable( state, AOStatesTable, AOStatesTabLen));
	    //// Can override this to do automatic actions at state entering
	    StateChange( old_state, state);
	    old_state = state;
	}

    try {
	_VarStatus.Set(state, 0, FORCE_SEND);
    } catch (AOVarException &e) {
	_logger->log( Logger::LOG_LEV_ERROR, "Error: Cannot write variable %s: %s",
		      _VarStatus.complete_name().c_str(), e.what().c_str());
    }

}

/*
 * ***SAFETY CHECK***
 * This method could be called by an handler BEFORE the variable _VarStatus
 * is initialized: in this case _ready is false (will be set to true only by
 * the SetReady method) and the variable is not read
 */
int AOApp::getCurState()
{
    int state = STATE_INVALID;	// Default state id AOApp hasn't set up _VarStatus
    if(_ready) {
	_VarStatus.Get(&state);
    }
    return state;
}

void AOApp::RequireKeyword( string keyword) {
    _keywords.push_back(keyword);
}

/* Returns True if all required keywords are present in cfg file */

bool AOApp::CheckKeywords( bool exit, bool log, bool print) {

    vector<string>::iterator iter;
    vector<string> missing;
    for (iter = _keywords.begin(); iter != _keywords.end(); iter++) 
        if (!ConfigDictionary().has_key(*iter))
            missing.push_back(*iter);

    bool ok=true;
    for (iter = missing.begin(); iter != missing.end(); iter++) {
        if (log)
            Logger::get()->log( Logger::LOG_LEV_ERROR, "Required keyword %s missing from configuration file", iter->c_str());
        if (print)
            printf("Required keyword %s missing from configuration file\n", iter->c_str());
        ok = false;
    }

    if (exit && (!ok))
        SetTimeToDie(true);

    return ok;

}
