//@File: Logger.cpp
//
// Logger class implementation 
//@

#include "Logger.h"
#include "AOExcept.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace Arcetri;

// [TODO] revise references to logPath and corresponding argument.

//-------------------------------- LOGGER ----------------------------------------//

//#define ALLOW_PRINTF

void log_printf(const char *fmt, ...)
{
#ifdef ALLOW_PRINTF
    va_list args;
    va_start( args, fmt);
    vprintf(fmt, args);
    va_end(args);
#else
#endif
}


// -- INIT STATIC PUBLIC FIELDS --- //
string Logger::LOG_PATH_DEFAULT = (Utils::getAdoptLog() != "") ? Utils::getAdoptLog() :  LOGGER_TMP_DIRECTORY;


// --- INIT STATIC PRIVATE FIELDS --- //
vector<Logger*> Logger::_loggers;
map<FILE *, int> Logger::_counters;

// The _parentName is used as default log file name
string   Logger::_parentName   = LOGGER_DEFAULT_PARENT;
bool     Logger::_fileDisabled = false;     
int      Logger::_global_level = LOG_LEV_DEFAULT;

pthread_mutex_t Logger::_logMutex = PTHREAD_MUTEX_INITIALIZER;;

//
//
bool Logger::exists(string loggerName, string extraName) {
    return (findByName(loggerName, extraName) != NULL);
}

//
//
Logger* Logger::get(int level) throw(LoggerException) {
	return get("MAIN", level);
}

//
//
Logger* Logger::get(string loggerName, int level, string extraName) throw(LoggerException) {

	// Instance the singleton
    Logger* logger = findByName(loggerName, extraName);
	if(logger == NULL) {
		logger = new Logger(loggerName, level, extraName);
		_loggers.push_back(logger);
	}
	
	return logger;
}

//
//
Logger::~Logger() {
    archiveLogFile();
}

//
//
void Logger::destroy() {

	vector<Logger*>::iterator iter = _loggers.begin();
	while(iter != _loggers.end()) {
		delete (*iter);
		iter++;
	}
	_loggers.clear();
	
    log_printf("[LOGGER-POOL] All loggers succesfully destroyed !!!\n");
}


//
//
Logger::Logger(string loggerName, int level, string extraName) :
    _logFile(""),
    _logExt(LOGGER_LOG_EXTENSION),
    _logPath(LOG_PATH_DEFAULT),
    _logFp(NULL),
    _name(loggerName),
    _extname(extraName),
    _level(level)
{
    //   log_printf("[LOGGER-POOL] Logger::Logger(): loggername=%s   filename=%s  parentname=%s\n", loggerName.c_str(), extraName.c_str(), _parentName.c_str());


    // check if it's a telemetry logger
    if (extraName.find(LOGGER_TELEMETRY) != string::npos)
        _logExt = LOGGER_TEL_EXTENSION;

    // In case this is the main logger, open the main log file   
    // and set the level.
    //
    if (loggerName == "MAIN") {
        setLogFile(_parentName, _logPath);
        _global_level = level;
    } else {
       
        string filename;

        // If no filename given, use the main file, otherwise use the main file as a prefix anyway
        if (extraName == "") {
            if (!findByName("MAIN")) {
                // A secondary logger opened before the main! Create the main
                Logger::get("MAIN");
            }
            filename = findByName("MAIN")->getLogFile();
        } else {
            filename = _parentName + "_" + extraName;
        }

        // If there is already a logger with that filename, copy the fp
        if ((_logFp = findFp(filename)) != NULL) {
            _logFile = filename;
        } else {
            // No other loggers with this filename, open a new one
            setLogFile(filename, _logPath);
        }
    }
}

//
//
FILE *Logger::findFp( string fileName)
{
    // Search for an existing logger with the given filename. If so, grab its Fp

    vector<Logger*>::iterator iter = _loggers.begin();
    while(iter != _loggers.end()) {
        if((*iter)->getLogFile() == fileName) {
            return (*iter)->getLogFp();
        }
        iter++;
    }
    return NULL;
}


//
//
void Logger::setLevel(int level) {

    if(level<LOG_LEV_WARNING) {
        _level = LOG_LEV_WARNING;
    }
    else if(level>LOG_LEV_TRACE) {
        _level = LOG_LEV_TRACE;
    }
    else {
        _level = level;
    }

    if(getName() == "MAIN")
        _global_level = level; // default for for new loggers
}


//
//
Logger* Logger::findByName(string name, string extname) {
	
	vector<Logger*>::iterator iter = _loggers.begin();
	while(iter != _loggers.end()) {
	    if (((*iter)->getName() == name) && ((*iter)->getExtraName() == extname)) {
            return (*iter);
	    }
	    iter++;
	}
	return NULL;
}

//
//
void Logger::setGlobalLevel(int level) {
    
    _global_level = level;
    // Loop over all loggers
    vector<Logger*>::iterator iter = _loggers.begin();
    while(iter != _loggers.end()) {
        (*iter)->setLevel(level);
        iter++;
    }
}

//
//
map<string, int> Logger::getNamesAndLevels() {
	map<string, int> mappa;
    // Loop over all loggers
   	vector<Logger*>::iterator iter = _loggers.begin();
	while(iter != _loggers.end()) {
		mappa.insert( pair<string, int>( (*iter)->getName(), (*iter)->getLevel()) );
		iter++;
	}
    return mappa;
}

//
//
void Logger::setParentName(string name) { 
    if(_parentName == LOGGER_DEFAULT_PARENT) {
        _parentName = name; 
        log_printf("[LOGGER-POOL] Logger parent is %s\n", _parentName.c_str()); 
    }
    else {
        log_printf("[LOGGER-POOL] Logger parent is already set (%s): impossible to change it!\n", _parentName.c_str()); 
    }
}

//
//
void Logger::printPoolStatus() {
   	map<string,int> loggers = getNamesAndLevels();
    map<string,int>::iterator leggersIter;
    log_printf("[LOGGER-POOL] Available loggers (name|level)\n");
    for(leggersIter = loggers.begin(); leggersIter != loggers.end(); leggersIter++ ) {
        log_printf("[LOGGER-POOL] %s|%s\n", leggersIter->first.c_str(), levelDescription(leggersIter->second));
	}
}

//
// Change current log file 
void Logger::change( string newname, string newpath) throw (LoggerFatalException) {

    log_printf("[LOGGER-POOL] Changing log file (%s %s)\n", newname.c_str(), newpath.c_str());

    setLogFile(newname, newpath);
}

//
// Rename current log file on the fly.
void Logger::rename(string newname, string newpath) throw (LoggerFatalException) {

    string oldfile = _logFile;
    string oldpath = _logPath;


    if (_fileDisabled)
        return;
    

    string oldfilename = getFullPath();

    _logFile = newname;
    if ((newpath != "") && (Utils::fileExists(newpath)))
        _logPath = newpath;

    string newfilename = getFullPath(); 

    // if it's the same name just return
    if (oldfilename != newfilename) {
        FILE *oldFp= getLogFp();

        // Check existance of the target file. If so, archive
        if (Utils::fileExists(newfilename)) {
            // if locked, rename it
            struct flock fl = { 0, 0, SEEK_SET, 0, 0};
            int hf = open(newfilename.c_str(), O_RDONLY);
            if ((fcntl(hf, F_GETLK, &fl) != -1) && (fl.l_type == F_WRLCK))  {
                // destination file is locked, add pid to the name
                char mypid[16];
                snprintf(mypid, 15, "%d", getpid());
                _logFile = _logFile + "_" + mypid;
                newfilename = getFullPath(); 
                log(LOG_LEV_WARNING, "Specified log file %s is locked, using %s", newname.c_str(), newfilename.c_str());
            }
            else {
                log_printf("[LOGGER-POOL] Rename: archiving old file %s\n", getFullPath().c_str());
                archiveLogFile();
            }
        }
        // mutex
        pthread_mutex_lock(&_logMutex);
 
        // close old file;
        closeLogFile();
	
        // move to new name
        log_printf("[LOGGER-POOL] Renaming log file (%s -> %s)\n", oldfilename.c_str(), newfilename.c_str());

        ostringstream cmd;
        // temporally just using system call (and redirecting stderr to null)
        cmd << "mv " << oldfilename << " " << newfilename << " 2> /dev/null";
        if (system(cmd.str().c_str()) != 0) 	{
            // rename failed, try to restore
            _logFile = oldfile;
            _logPath = oldpath;
            log_printf("[LOGGER-POOL] Rename: failed to rename log file\n");
        }
    
        /*
          FILE *src = fopen(oldfilename.c_str(), "rb");
          FILE *dst = fopen(newfilename.c_str(), "wb");
          
          if ((src == NULL) || (dst == NULL)) {
          fclose(src);
          fclose(dst);
          // rename failed, try to restore
          _logFile = oldfile;
          _logPath = oldpath;
          log_printf("[LOGGER-POOL] Rename: failed to rename log file\n");
          }
          else {
          #define BUFFSIZE 10240
	  
          char buff[BUFFSIZE];
          int  len;
	  
          while ( (len = fread(buff, BUFFSIZE, 1, src)) > 0)
          fwrite(buff, len, 1, dst);
	  
          fclose(src);
          fclose(dst);
	
          std::remove(oldfilename.c_str());
          }
        */
        
        // reopen file (true -> append)
        openLogFile(true);

        vector<Logger*>::iterator iter = _loggers.begin();
        while(iter != _loggers.end()) {
            if((*iter)->getLogFp() == oldFp) {
                (*iter)->setLogFp( _logFp);
                (*iter)->setLogFilePath(_logFile, _logPath);
            }
            iter++;
        }
        pthread_mutex_unlock(&_logMutex); 
    }
}

//
//
void Logger::setLogFile(string fileName, string filePath, bool nomutex) throw (LoggerFatalException) {

    // Handle concurrency with log(...) method
    if (!nomutex)
        pthread_mutex_lock(&_logMutex); 

    FILE *oldFp= getLogFp();

    log_printf("[LOGGER-POOL] Setting log file: %s\n", fileName.c_str());

    // Archive temporary log file
    if (oldFp) {
        closeLogFile();
        archiveLogFile();
    }

    // 2b) Select filename
    _logFile = fileName;
    if ((filePath != "") && (Utils::fileExists(filePath)))
        _logPath = filePath;

    // check existance of an old file left by a crashed program. If so, archive it.
    if (Utils::fileExists(getFullPath())) {
        // check if 
        log_printf("[LOGGER-POOL] setLogFile: archiving old file %s\n", getFullPath().c_str());
        archiveLogFile();
    }

    openLogFile();

    // Search for other loggers with the same fp and change them to the new one
    vector<Logger*>::iterator iter = _loggers.begin();
    while(iter != _loggers.end()) {
        if((*iter)->getLogFp() == oldFp) {
            (*iter)->setLogFp( _logFp);
            (*iter)->setLogFilePath(_logFile, _logPath);
        }
        iter++;
    }
    
    if (!nomutex)
        pthread_mutex_unlock(&_logMutex); 
}

//
//
string Logger::getFullPath( string filename) {

    if (filename == "")
        return _logPath + "/" + _logFile + _logExt; 
    else 
        return _logPath + "/" + filename + _logExt;
}


//
//
void Logger::archiveLogFile() {

    if (_fileDisabled)
        return;

    if(Utils::fileExists(getFullPath())) {
        int stat;
        // Add a timestamp to the current name   
        string archiveName = _logPath + "/" + _logFile + "." + Utils::timeAsString() + _logExt;
        log_printf("[LOGGER-POOL] Archiving %s to %s\n", getFullPath().c_str(), archiveName.c_str());
        if (( stat = ::rename(getFullPath().c_str(), archiveName.c_str())) != 0) {
            perror("rename") ;
            throw LoggerFatalException("Can't archive log file");
        }
    }

}

//
//
void Logger::openLogFile(bool append) throw (LoggerException) {

    if (_fileDisabled)
        return;

    if(_logFp == NULL) {
        _logFd = open(getFullPath().c_str(), O_WRONLY |  O_CREAT | ((append) ? O_APPEND : 0));
        _logFp = fdopen(_logFd, (append) ? "a" : "w");
        //_logFp = fopen(getFullPath().c_str(), (append) ? "a" : "w");
        if (_logFp == NULL) {
            log_printf("[LOGGER-POOL] Can't open file %s (may be directory not writable)\n", getFullPath().c_str());
            throw LoggerFatalException("Can't open log file");
        }
        struct flock fl;
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;
        fl.l_pid = getpid();

        fcntl(_logFd, F_SETLK, &fl);
        
    }
    else {
        throw LoggerException("A log file is already opened: close it before!");
    }
    fchmod(fileno(_logFp),0644);    // Set file permissions

    setCounter(_logFp, 0);
}

//
// NOTE: some other logger could be pointing to this same file handle, 
//       if it's like that is managed case by case after this call as
//       otherwise gets complicate to know .
void Logger::closeLogFile() {
	
    if(_logFp != NULL) {
        fclose(_logFp);
        close(_logFd);

        // remove it if has no logs
        if (getCounter(_logFp) == 0)
            remove(getFullPath().c_str());

        eraseCounter(_logFp);
        _logFp = NULL;
    }
}


//
//
void Logger::log(int message_level, const char* fmt, ...) {
    static int seqNumber;					

    if (!_fileDisabled) {

        if (_level >= message_level) {  
            	
            // Handle concurrency with setLogFile(...) and setMethod(...) methods
            pthread_mutex_lock(&_logMutex); 

            int startCounter = getCounter(_logFp);

            // Create log message 
            char message[MAXLOGLINELEN];   
            memset(message, '\0', MAXLOGLINELEN);// Message will have two '\0' characters as final guard
            va_list argp;
            va_start(argp, fmt);
            vsnprintf(message, MAXLOGLINELEN-1, fmt, argp);  // See comment above
            va_end(argp);

                
            // Create log header with the space for message at the end
            char header[MASTER_LOG_HEADER_LEN];
            seqNumber = logString((char*)_parentName.c_str(), message_level, NULL, header);   

            // --- FILE --- //
            // The header ends with a space
            // Add the name of the logger before the message
            if (_logFp) {
                // Manage multiline log
                try {
                    char *pt=strchr(message,'\n');
                    if(pt) *pt++ = '\0';
                    if (getLogExt() != LOGGER_TEL_EXTENSION)
                        fprintf( _logFp, "%s%16s > %s\n", header, _name.substr(0,16).c_str(), message);
                    else
                        fprintf( _logFp, "%s%30s > %s\n", header, _name.substr(0,30).c_str(), message);

                    incrCounter(_logFp);
                    while(pt) {
                        char *str=pt;
                        char *end=strchr(pt,'\n');
                        if(end) {
                            *end='\0';
                            pt=end+1;
                        } else pt=NULL;
                        if (getLogExt() != LOGGER_TEL_EXTENSION)
                            fprintf( _logFp, "%s%16s >  . %s\n", header, _name.substr(0,16).c_str(), str);
                        else
                            fprintf( _logFp, "%s%30s >  . %s\n", header, _name.substr(0,30).c_str(), str);
                        incrCounter(_logFp);
                    }
                
                    fflush( _logFp);
                } catch (...) {
                    log_printf("[LOGGER-POOL] Log: Failed to log\n");
                }
                
                int endCounter = getCounter(_logFp);
                int sect1 = startCounter/MAX_LINES_PER_LOGFILE;
                int sect2 = endCounter/MAX_LINES_PER_LOGFILE;
                
                // Save the log archive if necessary
                if(sect1 != sect2) {
                    try {
                        FILE *oldFp = _logFp;

                        closeLogFile();
                        archiveLogFile();
                        openLogFile();

                        // Search for other loggers with the same fp and change them to the new one
                        vector<Logger*>::iterator iter = _loggers.begin();
                        while(iter != _loggers.end()) {
                            if((*iter)->getLogFp() == oldFp) {
                                (*iter)->setLogFp( _logFp);
                            }
                            iter++;
                        }
 
                        setCounter( _logFp, endCounter);

                    } catch (AOException *e) {
                        log_printf("Exception while rotating log file: %s\n", e->what().c_str());
                    }
                }
            
            }
            // --- MSGD --- //
            // If level is LOG_LEV_ERROR/LOG_LEV_FATAL
            if (message_level == LOG_LEV_ERROR || message_level == LOG_LEV_FATAL) {
                thLogMsg(message_level, seqNumber, message);
            }
        
            pthread_mutex_unlock(&_logMutex);
        } 
    }
}

//@Member: Log
//
// Log a message, given as string
//@
void Logger::log(int message_level, const string & s) {
    log(message_level, s.c_str());
}

void Logger::printStatus() {
    log( LOG_LEV_DEBUG, "My level is %d (%s)", _level, levelDescription(_level));
}

