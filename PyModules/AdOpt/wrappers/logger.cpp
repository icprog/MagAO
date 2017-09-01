//@File: logger.cpp
//
// Python module to interface to Logger module
//
// Only limited functionality is available: a single logger with configurable method, level and output file.
//-

#include <Python.h>
#include <string>
#include "Logger.h"


extern "C" {
// methods callable from Python
void initlogger(void);
static PyObject *PyLoggerSetLevel( PyObject *, PyObject *);
static PyObject *PyLoggerSetLogFile( PyObject *, PyObject *);
static PyObject *PyLoggerSetName( PyObject *, PyObject *);
static PyObject *PyLoggerLog( PyObject *, PyObject *);
static PyObject *PyLoggerArchive( PyObject *, PyObject *);
}


// Registration of methods callable from Python

static struct PyMethodDef logger_methods[] = {
    {"setLevel", PyLoggerSetLevel, METH_VARARGS, "Sets the logging level"},
    {"setLogFile", PyLoggerSetLogFile, METH_VARARGS, "Sets the logger output file"},
    {"setName", PyLoggerSetName, METH_VARARGS, "Sets the logger name"},
    {"log", PyLoggerLog, METH_VARARGS, "Logs a string"},
    {"archive", PyLoggerArchive, METH_VARARGS, "Archives the log file"},
    {NULL, NULL, 0, NULL}
};

// C functions which do the actual work


//+Function: initlogger

// Function called by Python import mechanism

// This function is the equivalent of Python classes' __init__, and is
// automaticaly called by the interpreter when importing the module.
//-

void initlogger()
{
    PyObject *m = Py_InitModule("logger", logger_methods);


     PyModule_AddIntConstant(m,"LOG_LEV_TRACE", Logger::LOG_LEV_TRACE);
     PyModule_AddIntConstant(m,"LOG_LEV_DEBUG", Logger::LOG_LEV_DEBUG);
     PyModule_AddIntConstant(m,"LOG_LEV_INFO", Logger::LOG_LEV_INFO);
     PyModule_AddIntConstant(m,"LOG_LEV_WARNING", Logger::LOG_LEV_WARNING);
     PyModule_AddIntConstant(m,"LOG_LEV_ERROR", Logger::LOG_LEV_ERROR);
     PyModule_AddIntConstant(m,"LOG_LEV_FATAL", Logger::LOG_LEV_FATAL);
/* L.F. removed.  L'asterisco serve per imbrogliare grep
     PyModule_AddIntConstant(m,"LOG_LEV_*DISABLED", Logger::LOG_LEV_*DISABLED);
     PyModule_AddIntConstant(m,"LOG_LEV_*ALWAYS", Logger::LOG_LEV_*ALWAYS);
*/

}


//+Function: PyLoggerSetLevel

static PyObject *PyLoggerSetLevel( PyObject *self, PyObject *args)
{
    int level;

    if (!PyArg_ParseTuple( args, "i", &level))
       return NULL;

    Logger::get()->setLevel(level);

    return Py_BuildValue("i", 0);
}

//+Function: PyLoggerSetLogFile

static PyObject *PyLoggerSetLogFile( PyObject *self, PyObject *args)
{
    char *logfile, *logpath;

    if (!PyArg_ParseTuple( args, "ss", &logfile, &logpath))
       return NULL;

    try {
      Logger::get()->setLogFile( logfile, logpath);
    } catch (Arcetri::LoggerException &e) {
       printf("Logger exception: %s\n", e.what().c_str());
    }


    return Py_BuildValue("i", 0);
}


//+Function: PyLoggerSetName

static PyObject *PyLoggerSetName( PyObject *self, PyObject *args)
{
    char *name;

    if (!PyArg_ParseTuple( args, "s", &name))
       return NULL;

    try {
      Logger::get()->rename(name);
    } catch (Arcetri::LoggerException &e) {
       printf("Logger exception: %s\n", e.what().c_str());
    }


    return Py_BuildValue("i", 0);
}

static PyObject *PyLoggerArchive(  PyObject *self, PyObject *args)
{
   Logger::get()->archiveLogFile();
   return Py_BuildValue("i", Py_None);
}


//+Function: PyLoggerLog
//
// Logs the message. Returns either None or, if the message
// must be sent to the MsgD too, a tuple of arguments to the thrdlib.thLogMsg() function.

int to_msgd;
int _level,_seqnum;
char _message[256];

static PyObject *PyLoggerLog( PyObject *self, PyObject *args)
{
    char *str;
    int level = Logger::LOG_LEV_INFO;

    if (!PyArg_ParseTuple( args, "s|i", &str, &level))
       return NULL;

    to_msgd=0;
    Logger::get()->log( level, str);

    if (to_msgd == 0)
      return Py_BuildValue("i", Py_None);
    else
      return Py_BuildValue("(iis)", _level, _seqnum, _message);
}

// Function used to save thLogMsg parameters.

int thLogMsg( int level, int seqnum, const char *message)
{
   _level = level;
   _seqnum = seqnum;
   strncpy( _message, message, 256);
   _message[255] =0; // Ensure null-termination

   return 0;
}

