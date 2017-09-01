//@File: Logger.h
//
// Definition of the Logger class.
//@

#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#define MAXLOGLINELEN   16384
#define LOGGER_TMP_DIRECTORY  "/tmp"
#define LOGGER_LOG_EXTENSION  ".log"
#define LOGGER_TEL_EXTENSION  ".tel"
#define LOGGER_TELEMETRY      "TELEMETRY"

//#define LOGGER_DEFAULT_PARENT "UNKNOWN-PROCESS"
#define LOGGER_DEFAULT_PARENT (Utils::uniqueFileName())

#include "stdarg.h"
#include "stdio.h"
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#include <pthread.h>

#include "AOExcept.h"
#include "Utils.h"
extern "C" {
#include "base/thrdlib.h"
}

using namespace std;
using namespace Arcetri;



// ------- Debug
// Define this to allow local printf on stdout

// #define ALLOW_PRINTF


void log_printf(const char *fmt, ...);

// Root namespace
namespace Arcetri {
	
/*
 * @Class: LoggerException
 * Base class for all the Logegr exceptions
 */
    class LoggerException: public AOException {
	
      public:
		
		LoggerException(string id,
                        string message, 
		                int errcode = LOGGER_GENERIC_ERROR, 
		                string file = "", 
		                int line = 0): AOException(message, errcode, file, line) { exception_id = id; }
		                
		/*
		 * Generic LoggerException
		 */
		LoggerException(string message, 
		                int errcode = LOGGER_GENERIC_ERROR, 
		                string file = "", 
		                int line = 0): AOException(message, errcode, file, line) { exception_id = "LoggerException"; }
		                
		virtual ~LoggerException() {};
    };

    class LoggerFatalException: public LoggerException {
      public: 
        LoggerFatalException(string message): LoggerException("LoggerFatalException", message, LOGGER_FATAL_ERROR) {}
			
        virtual ~LoggerFatalException() {};
	};

 
//@Class: Logger 
//
// LBT Logging support class 
//
// \Logger\ defines a pool of configurable "named" logger
//
// The pool is defined by:
//	- A parent name, usually the name of the process using it.
//  - A log file
//
// Logger is a Multi-Singleton class: each named logger can be instantiated only 
// once (usually by the AOApp). The default name is "MAIN", automatically used.
// A reference to the "MAIN" Logger can be obtained by a call to Logger::get(). 
// Only the first Logger::get(name) will actually create the Logger; a client
// can only change the logging level using setLevel().
// 
// The logger pool must be destroyed using the destroy() method. Repeated calls to
// destroy() doesn't have any effect
//
// The class provides a set of log-levels to change the graining of the logging.
// It must be initialized with a desired log-level (LOG_LEV_ERROR by default): 
// it only logs requests with req-level<=log-level. Obviously it doesn't log
// requests with req-level = DISABLED.
//
    class Logger {
	
      public: // AVAILABLE SETTINGS
    
        // Log-levels (for each named logger)
        static const int LOG_LEV_VTRACE   = MASTER_LOG_LEV_VTRACE;
        static const int LOG_LEV_TRACE    = MASTER_LOG_LEV_TRACE;
        static const int LOG_LEV_DEBUG    = MASTER_LOG_LEV_DEBUG;
        static const int LOG_LEV_INFO     = MASTER_LOG_LEV_INFO;
        static const int LOG_LEV_WARNING  = MASTER_LOG_LEV_WARNING;
        static const int LOG_LEV_ERROR    = MASTER_LOG_LEV_ERROR;
        static const int LOG_LEV_FATAL    = MASTER_LOG_LEV_FATAL;
        static const int LOG_LEV_ALWAYS   = MASTER_LOG_LEV_ALWAYS;   // DEPRECATED
        static const int LOG_LEV_DISABLED = MASTER_LOG_LEV_DISABLED; // DEPRECATED


        // Default log path (initialized in cpp)
        static string LOG_PATH_DEFAULT;
       
       
      public:	// DEFAULT VALUES
	
        // Change this if needed
        static const int LOG_LEV_DEFAULT = LOG_LEV_WARNING;
		
        
      private:
    
    	static const unsigned int MAX_LINES_PER_LOGFILE = 100000; //(consider 100 Bytes/line)
    
    
      public:
        
        // -------------- CREATORS & DESTRUCTORS ------------- //
        
        //@Member: get
        //
        // Returns the current singleton main logger (named "MAIN")
        // Attention: rember to call destroy()
        //@
        static Logger* get(int level = _global_level) throw(LoggerException);
        
        //@Member: get
        //
        // Returns the singleton logger by name.
        // If the logger doesn't exist, create it with the specified
        // log level; else retrieve it without changing log level.
        //@
        static Logger* get(string loggerName, 
                           int level = _global_level,
                           string extraName = "") throw(LoggerException);
	
        //@Member: destroy
        //
        // Destroy the loggers pool.
        //@
        static void destroy();
        
        
        // -------------- QUERIES ------------- //
        
        //@Member: exists
        //
        // Returns true only if the specified logger exists.
        //@
        static bool exists(string loggerName, string extraName = "");
      	
      	
      	// -------------- MODIFIERS ------------- //
        
        //@Member: setParentName
        //
        // Sets the parent name, that is the name of
        // the application using the loggers pool
        //@
        static void setParentName(string name);

        //@Member: rename
        //
        // Renames the current log file on the fly
        //@
        void rename( string newname, string newpath = "") throw (LoggerFatalException);

        //@Member: change
        //
        // Changes the current log file, archiving the current one and opening a new one.
        //@
        void change( string newname, string newpath = "") throw (LoggerFatalException);
 
        
        //@Member: setLogFile
        //
        // Sets file name and path for the LOG_METHOD_FILE method. 
        //
        // This function uses the global logMutex to avoid concurrency with the
        // log() method. If called from the log() method, the 'nomutex' flag
        // must be set to avoid deadlocks.
        //@
        void setLogFile( string fileName,       // The extension .log is automatically added
                         string filePath,       // Without trailing "/"
                         bool nomutex = false)	// Avoid locking the log() mutex
            throw(LoggerFatalException);

        //@Member: setGlobalLevel
        //
        // Sets the log level for all the loggers belonging to the pool
        //@
        static void setGlobalLevel(int level);

        //@Member: disableFileLog()
        //
        // Disables logging on file (MsgD messages are still sent)
        //@
        static void disableFileLog() { _fileDisabled = true; }
        
        //@Member: enableFileLog()
        //
        // Enables logging on file
        //@
        static void enableFileLog() { _fileDisabled = false; }
        
        
        
        // -------------- ACCESSORS ------------- //
        
        //@Member: getParentName
        //
        // Returns the parent name
        //@
        static string getParentName() { return _parentName;}

        
        // ------------------- UTILITIES -------------------- //

		//@Memeber: stringToLevel
        //
        // Convert a log level from a string to an int valid
        // as log level.
        //@   
        static int stringToLevel(string levelAsString) {
        	return logNumLevel(levelAsString.c_str());
        }
        
        //@Member: printPoolStatus
        //
        // Print the status of the pool to stdout, since the info
        // printed are unuseful into a log file.
        //@
        static void printPoolStatus();
        
        //@Member: levelDescription
        //
        // Return the description of a log-level
        //@
        static const char* levelDescription(int message_level) {
            return logDescr(message_level);
        }

        //@Member: getNamesAndLevels
        // 
        // Return a dictionary of names and levels of the pool
        //@
        static map<string,int> getNamesAndLevels();


        static int getCounter( FILE *fp) {
            if (_counters.find(fp) == _counters.end())
                return -1;
            return _counters[fp];
        }

        static void setCounter( FILE *fp, int counter) {
            _counters[fp] = counter;
        }

        static void eraseCounter( FILE *fp) {
            _counters.erase(fp);
        }

        static void incrCounter( FILE *fp) {
            _counters[fp]++;
        }


      public: // DINAMIC SECTION
	
		//@Member: setLevel
        //
        // Set the current logging level
        //@
        void setLevel(int level);
        
        //@Member: getName
        //
        // Returns the name of the logger
        //@
        string getName() { return _name;};
        
        //@Member: getExtraName
        //
        // Returns the name of the logger
        //@
        string getExtraName() { return _extname;};

        //@Member: getLevel
        //
        // Returns the current the logging level
        //@
        int getLevel() { return _level;};

        //@Member: getLogFile
        //
        // Gets the output file name for the FILE method
        //@
        string getLogFile() { return _logFile; }

        //@Member: getLogPath
        //
        // Gets the output path for the FILE method
        //@
        string getLogPath() { return _logPath; }
    
        //@Member: getLogExt
        //
        // Gets the output path for the FILE method
        //@
        string getLogExt() { return _logExt; }
    
        //@Member: setLogFilePath
        //
        //@
        void setLogFilePath(string name, string path) { _logFile = name; _logPath = path; }

        //@Member getFullPath
        //
        // Returns the complete path of the current log file.
        // If filename is not empty, returns the complete path
        // for that filename instead of the current one.
        //@
        string getFullPath( string filename = "");

        //@Member: getLogFp
        //
        // Gets the output fp for the FILE method
        //@
        FILE *getLogFp() { return _logFp; }
        
        //@Member: setLogFp
        //
        // Set the output fp for the FILE method
        //@
        void setLogFp( FILE *fp) { _logFp = fp; }

        //@Member: log_telemetry
        //
        // Specialized function to log telemetry data
        //@
        template <class T> void log_telemetry(int message_level, T* data, unsigned int n_elem) {
            ostringstream oss;
            unsigned int i;

            Logger *log = this;

            if (_logExt != LOGGER_TEL_EXTENSION) {
                log = Logger::get(_name, _level, LOGGER_TELEMETRY);
            }

            if ((log != NULL) && (log->getLogExt() == LOGGER_TEL_EXTENSION)) {
                for (i=0; i<n_elem; i++) oss << data[i] << " ";
                log->log(message_level, "%s", oss.str().c_str());
            }
        }

        //@Member: log_telemetry
        //
        // Specialized function to log telemetry data
        //@
        template <class T> void log_telemetry(int message_level, vector<T> &data) {
            log_telemetry(message_level, &data[0], data.size());
        }

        
        //@Member: log
        //
        // Log a message, adding a newline at the end.
        //@
        void log(int message_level, const char* fmt, ...);

        //@Member: Log
        //
        // Log a message, adding a newline at the end.
        //@
        void log(int message_level, const string & s);

        
        //@Member: printStatus
        //
        // Print the status of the logger
        //@
        void printStatus();
        

      private: 	// STATIC SECTION

        //@C
        //\verb+_loggers+ store the vector of singleton logger
        //@
        static vector<Logger*> _loggers;

        //\verb+_counters+ store the line counters for each log file (using the file descriptor)
        static map<FILE *, int> _counters;
        
        //@
        //\verb+_parentName+ store the name of the parent process (optional)
        //@
        static string _parentName;

        //@C
        //\verb+_global__level+ store the current global level
        //@
        static int _global_level;    // All loggers share the same level
        
        //@
        //\verb+_fileDisabled+ flag to disable any file writing operations
        //@
        static bool _fileDisabled;
        
        // Avoid concurrent operations that may conflict,
        // i.e. change method or file archive while log(...) 
        // is executing
        static pthread_mutex_t _logMutex;

        
        
      private:	// DYNAMIC SECTION

        //\verb+_logFile+ filename (without path) for FILE logging method
        //@
        string _logFile;
 
        //\verb+_logExt+ extension
        //@
        string _logExt;
        
        //\verb+_logPath+ path for FILE logging method
        //@
        string _logPath;
        
        //@
        //\verb+_myLogFp+ file descriptor for FILE logging method
        //@
        FILE* _logFp;
        int   _logFd;
    
        //@C
        //\verb+_name+ store the name of the logger
        //@
        string _name;

        //@C
        //\verb+_extname+ store the name of the logger
        //@
        string _extname;

        //@C
        //\verb+_level+ store the current logging level
        //@
        int _level;   

    	//@Member: Logger
        // 
        // Constructor 
        //
        // Initialize the logger with the desired logging-level (LOG_LEV_ERROR by default)
        //@
        Logger(string loggerName, int level = LOG_LEV_ERROR, string fileName="");   
        
        //@Member: Logger
        //
        // Destructor
        //
        //@
        ~Logger();
	       
	  
	    //@Member: findByName
        //
        // Find a logger by name
        //@ 
        static Logger* findByName(string loggerName, string extraName = "");

        FILE *findFp( string fileName);

        //@Member: setName
        //
        // Set logger name
        //@

	    
	    /*
	     * Archive the current logging file, adding a timestamp to the name.
	     */
      public:
	    void archiveLogFile();
	
      private:    
	    /*
	     * Open/Close the currently set logFile
	     */
	    void openLogFile(bool append = false) throw (LoggerException);
	    void closeLogFile();

    };


} //End Root namespace

#endif
