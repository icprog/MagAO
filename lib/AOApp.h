//@File: AOApp.h
//
// \aoapp\ class declaration
//
// This file contains the declaration of \aoapp\ class
//@

#ifndef AOAPP_H_INCLUDED
#define AOAPP_H_INCLUDED

extern "C" {
}

#include "RTDBvar.h"
#include "stdconfig.h"
#include "Logger.h"
using namespace Arcetri;

#include <vector>

//@Class: AOApp
//
// LBT-AO components' base class
//
// \aoapp\ implements an LBT-AO component
// with basic functionality common to all the complex LBT-AO components.
//
// A generic \aoapp\ application consists of the following blocks, executed in this order
// and described in details below:
// \begin{itemize}
// \item Read the configuration file, if specified (performed in the constructor)
// \item Initialize the application.
// \item Install handlers for asynchronous events.
// \item Connect to \msgd.
// \item Create and initialize \RTDB\ variables.
// \item Post initialization step.
// \item Inform the other LBT-AO components that we are ready
// \item Run the active loop
// \end{itemize}
//
// \subsection*{Configuration file}
//
// To be created, a generic \aoapp\ requires a few basic parameters: name, side, id, serverIP.
// These parameters can be passed directly in a constructor or written in a configuration file.
// The configuration file can be specified in the constructor or by calling the corresponding
// SetConfigFile member function.
// Here follows an example of a minimal configuration file:
//
//\begin{verbatim}
// # generic AOApp configuration file
// # Parameters are in the form <name> <value>
//
// # MsgD IP
// Server string 127.0.0.1
// \end{verbatim}
//
//
//
//
// \subsection*{Inherited functionalities}
// The \aoapp\ class provides to the derived classes some functionalities:
// \begin{itemize}
// \item Parse the configuration file and provides a dictionary of the configuration data.
// \item Connects to \msgd\ through the \thrdl\ library.
// \item Creates two \RTDB\ variables ({\tt STATUS} and {\tt ERRMSG}).
// \item Installs a generic handler to react to the {\tt TERMINATE}, {\tt LOGINCR} and {\tt LOGDECR} messages.
// \item Installs a generic handler for the {\tt VARCHANGD} message, which can be overriden by specific handlers.
// \end{itemize}
//
//
// \subsection*{Usage}
// An LBT-AO component should derive from \aoapp.
// Some members functions are declared virtual and can be overridden by
// the derived application.
//
// \subsection*{Duties of a well-behaved derived application}
// An LBT-AO component derived by \aoapp\ must use variables {\tt STATUS} and {\tt ERRMSG}
// created by the base class to reflect in the \RTDB\ its internal status.
//
// Each thread of the derived application must periodically call {\tt TimeToDie()} to properly stop
// the application in case of a {\tt TERMINATE} message.
//
// An LBT-AO component derived by \aoapp\ must make use of the configuration file and use
// ConfigDictionary() to retrieve the configuration dictionary.
//
// An LBT-AO component derived by \aoapp\ must make use of {\tt Verbosity()} to determine the verbosity
// requested by the user via the {\tt LOGINCR} and {\tt LOGDECR} messages .
//
// \subsection*{Dependences}
// \aoapp\ depends on core LBT-AO libraries (\msgl, \rtdbl, \thrdl )
// and on classes \RTDBvar\ and \stdconfig.
//@
class AOApp
{

	private:

		// --- Some default settings --- //
		// see AOApp.cpp for initialization
		static int 	  CONFIG_LOADER_LOG_LEVEL;	// Used to log config file loading

		static int 	  DEFAULT_LOG_LEVEL;
		static string DEFAULT_LOG_PATH;
		
		//  The following parameters define how the connection to MsgD is managed
		//  Values different from default can be set with waitForMsgD()
      int     _ntry;     // Number of connection attempts to be made
      int     _looptime; // Delay between retries in microseconds
      int     _incr;     // Auxiliary variable


	public:

      /*
       * Retrieve the singleton instance
       */
       static AOApp* getInstance() { return _instance; }


      /*
       * Return configuration file path based on identity
       */

      static string getConffile( string identity);

      /*
       * typedef for variable handlers
       */

		 typedef int (*v_hndl)(void *pt, Variable *msgb);


    public:

       	//////////// construction ///////////

       	// This constructor defines the basic required parameters.
       	//
       	// The name of the logfile is name+id, and the location
       	// is defined by ADOPT_LOG environment variable
       	AOApp( string name,
       	       string side,
       	       string serverIp,
       	       int logLevel = Logger::LOG_LEV_INFO,
                bool inst_sighand = true,
                string logDir="" )
               throw (AOException);

       	// This constructor load parameters from a config file.
       	//
    	// Note that the LogLevel specified in config file has the priority
    	// on "logLevel" parameter
        AOApp( string name,
               const std::string& conffile)
               throw (AOException);


		// Note that command line parameters will modify the LogLevel specified
      	// in the configuration file
        AOApp( int argc,
               char **argv)
               throw (AOException);

        virtual ~AOApp();

    	// ------------------------ ACCESSORS ----------------------- //

        static const std::string &     ConfigFile() {return _configFile;}
        static const std::string &     MyName() {return _myName;}
        static const std::string &     MyFullName() {return _myFullName;}
        static const std::string &     Side() {return _side;}
        static const std::string &     Server() {return _server;}
        static Config_File &           ConfigDictionary() {return (*_cfg);}
        static bool             	   TimeToDie() {return _timeToDie;}
        static bool                 isSimulation() { return _simulation; };
        static int              	   Verbosity() {return _logger->getLevel();}

        void            Exec( bool retryClientname = false);

        void            setCurState( int state);
        int             getCurState();

        // ---------------------- UTILITIES -------------------- //

        void usage();

        int     eNotify(RTDBvar &var, v_hndl = NULL, bool getNow=false);
        int     eNotify(string varname, v_hndl = NULL, bool getNow=false);
        void    Notify(RTDBvar &var, v_hndl = NULL, bool getNow=false);
        void    Notify(string varname, v_hndl = NULL, bool getNow=false);
        void    waitForMsgD(int ntry=0, int looptime=1000);


        static void     SetTimeToDie(bool t) { _timeToDie = t; }

        // ---------------- CONFIG FILE CHECK -------------//
        void    RequireKeyword(string keyword);
        bool    CheckKeywords( bool exit = true, bool log = true, bool print = false);

    protected:

    	// --------------------------- QUERIES --------------------------------//

    	// This method should be called in EVERY handler before doing something!!!
    	static bool IsReady() { return _ready; }


    	// --------------------------- MODIFIERS ---------------------------- //

        static void     SetMyName(const std::string& name);
        static void     SetConfigFile(const std::string& conffile) throw (Config_File_Exception);
        static void     SetServer(const std::string& server) {_server=server;}

        // ---------------------- BASE METHODS - to be overridden ------------------- //

        virtual void     SetupVars(){};
        virtual void     InstallHandlers(){};
        virtual void     PostInit(){};
        virtual void     Run();
        virtual int      VariableHandler(Variable *){return NO_ERROR;};
        virtual void     StateChange( int /*oldstate */, int /*state*/){};

	private:

		/*
		 * Set the log file for the owned Logger, using the correct
		 * log path ($ADOPT_LOG or the Logger-default)
		 */
		void 	setLogFile(string fileName, bool renaming = false);

		/*
		 * Intialize a temporary log file, before the configuration
		 * is read; the log file will be renamed by CreateApp(...) method
		 * which know about the correct log filename. If logFileEnabled is false,
       * further log file writing is disabled.
		 */
		void 	initTempLog(string logDir);

    	/*
    	 * Complete the AOApp construction
    	 */
    	void 	CreateApp(int logLevel) throw (AOException);

        void    InitBase();
        int     ConnectBase();
        void    InstallHandlersBase();
        void    SetupVarsBase();
        void    SetReady();

        //must be static since it's a C function
        static int      terminate_handler(MsgBuf *msgb, void *argp, int hndlrQueueSize);
        static int      setloglev_handler(MsgBuf *msgb, void *argp, int hndlrQueueSize);
        static int      getlognamelev_handler(MsgBuf *msgb, void *argp, int hndlrQueueSize);
        static int      varchangd_handler(MsgBuf *msgb, void *argp, int hndlrQueueSize);
        static void     sighand(int);


    ////////////// variables ////////////
    protected:

		//@C
		//\verb+_VarStatus+ must be used to reflect in the {\tt RTDB} the status of the application.
		//@
		        static RTDBvar     _VarStatus;

		//@C
		//\verb+_VarErrmsg+ must be used to reflect in the {\tt RTDB} the last error message of the application.
		//@
		        static RTDBvar     _VarErrmsg;

		//@C
		//\verb+_timeToDie+ is used to signal to the threads that they have to exit.
		//@
		        static bool    _timeToDie;

		//@C
		//\verb+_simulation+ is true only if the AOApp has been started with
      // simulation option.
		//@
		        static bool _simulation;

		//@C
		//\verb+_logger+  control the level of verbosity.
		//@
		        static Logger*  _logger;


    private:

		//@C
		//\verb+_configFile+ is the name of the configuration file
		//@
		      	static std::string    _configFile;

		//@C
		//\verb+_logFile+ is the name of the configuration file
		//@
		      	static std::string    _logFile;

		//@C
		//\verb+_myName+ is the name of the application
		//as used by \msgd.
		//@
		        static std::string    _myName;

		//@C
      // Logging directory
		//@
                	static std::string  _logdir;   // Logging directory

		//@C
		//\verb+_myFullName+ is the name of the application with the side attached (unique for MsgD)
		//as used by \msgd.
		//@
		        static std::string    _myFullName;

		//@C
		//\verb+_server+ is the IP number of the machine on which
		//\msgd\ is running.
		//@
		        static std::string    _server;

		//@C
		//\verb+_side+ can be one of \verb+C+,  \verb+L+ or \verb+R+ (common, left or right)
		//and is passed as a command line argument or taken from ADOPT_SIDE
		//@
		        static std::string    _side;


		//@C
		//\verb+_install_sighand+
		//@

		        static bool     _install_sighand;  // Flag to enable installation of signal handlers

		//@C
		//\verb+_cfg+ holds the configuration dictionary read from an optional config file.
		//Can be NULL if config file doesn't exist.
		//@
		        static Config_File*    _cfg;

		//@C
		//\verb+var_handlers+ is a map of function pointers for RTDBvar notify messages
		//@
		        static std::map<std::string, v_hndl> _var_handlers;

		//@C
		//\verb+_ready+ is true only if the AOApp is succesfully started
		//@
		        static bool _ready;

		//@C
		//\verb+_keywords+ is a vector of keywords to be checked
		//@
		        static std::vector<string> _keywords;

      //@C
      // Singleton instance, set by constructor
      //@
             static AOApp *_instance; 
};

#endif // AOAPP_H_INCLUDED
