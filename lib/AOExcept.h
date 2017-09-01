//@File: AOExcept.h
//
// \AOException\ class and derivate classes declaration
//
// This file contains the declaration of the generic \AOException\ class
// and of other specialized exception classes derived from it.
//@

#ifndef AOEXCEPT_H_INCLUDED
#define AOEXCEPT_H_INCLUDED

#include <sstream>
#include <stdio.h>
#include <sys/time.h>

#include "RTDBvar.h"


//@Class: AOException
//
// LBT-AO base exception class
//
// \AOException\ is the base exception class for LBT-AO components.
//
// \subsection*{Writing exception classes}
//
// An LBT-AO component wishing to define a specialized exception must:
// \begin{itemize}
// \item derive the class from AOException or another derived class
// \item in the constructor, set the \exception_id\ member to the name of the specialized class
// \item provide a str() method returning a descriptive string of the error
// \item call the base class constructor with the appropriate parameters (message and errcode for AOException).
// \end{itemize}
//
// The derived class may add any member variables needed to hold information about the exception, which
// can be examined by application programs catching the exception. However, general AOExcept methods
// will not be able to access those members. Currently, the only way to give information about them is
// the str() function.
//
// \subsection*{Using exception classes}
//
// When throwing, choose the most specific class available.
//
// When catching, catch either a specific class or a generic AOExcept object. Use the what() method to have a
// descriptive string of what happened. 
//@

typedef enum {
	Verbose,  // Returns full message string
	Terse,    // Returns message without Exception ID and time
        Hmi       // Returns only the explicit message
} Verbosity;


class AOException
{
public:
   AOException( std::string message, int errcode = NO_ERROR, std::string file = "", int line=0)
      {
      _message = message;
      _errcode = errcode;
      _file    = file;
      _line    = line;
      exception_id = "AOException";
      }

   virtual ~AOException() {}

   virtual std::string str()
      { 
      return "";
      }

   virtual std::string what(Verbosity verb=Verbose)
   {
      std::ostringstream s;

      switch(verb) {
      case Verbose:
          s << "[" << exception_id << "] " << _message;
          if (_errcode <0)
             s << " (code " << PLAIN_ERROR(_errcode) << ") " << lao_strerror(_errcode);
    
          if (_file != "")
             s << " File " << _file;
          if (_line >0)
             s << " line " << _line;
          s << " " << str();
          break;
      case Terse:
          s << _message;
          if(IS_ERROR(_errcode)) 
              s << " - " << PLAIN_ERROR(_errcode) << ": " << lao_strerror(_errcode);
          break;
      case Hmi:
          s << _message;
      }
      return s.str();
   }

public:
   std::string _message;
   std::string exception_id;
   int         _errcode;
   std::string _file;
   int         _line;

};

class AOVarException : public AOException
{
public:
  AOVarException( std::string message,
                  RTDBvar &var,
                  int errcode = NO_ERROR,
                  std::string file="",
                  int line = 0) : AOException( message, errcode, file, line)
    {
    exception_id = "AOVarException";
    _varname = var.complete_name();
    _varstrtype = RTDBvar::strtype(var.type());
    _vartype = var.type();
    _varlen  = var.len();
    }
  
  std::string str()
     {
     std::ostringstream s;
     s << "Variable: " << _varname << " (type " << _varstrtype << ", len " << _varlen << ")";//std::endl;
     return s.str();
     }

public:
    string _varname;
    string _varstrtype;
    int    _varlen;
    int    _vartype;
};

class AOVarTypeException : public AOVarException
{
public:
   AOVarTypeException( RTDBvar &var,
                       int type,
                       std::string message ="",
                       std::string file="",
                       int line=0) : AOVarException( message, var, VAR_TYPE_MISMATCH_ERROR, file, line)
     {
     _type = type;
     exception_id = "AOVarTypeException";
     }

   std::string str()
   {
      std::ostringstream s;
      s << AOVarException::str();
      s << "Wrong variable type: expected " << _varstrtype
        << " (" << _vartype << "), got " << RTDBvar::strtype(_type) << " (" << _type << ")" << std::endl;
      return s.str();
   }
public:
   int _type;
};

class AOVarSideException : public AOVarException
{
public:
   AOVarSideException( RTDBvar &var,
                       std::string side,
                       std::string message ="",
                       std::string file="",
                       int line = 0) : AOVarException( message, var, VAR_SIDE_ERROR, file, line)
     {
     _side = side;
     exception_id = "AOVarSideException";
     }

   std::string str()
   {
      std::ostringstream s;
      s << AOVarException::str();
      s << "Wrong variable side: " << _side << std::endl;
      return s.str();
   }
public:
   std::string _side;
};

class AOVarRangeException : public AOVarException
{
public:
   AOVarRangeException( RTDBvar &var,
                        int index,
                        std::string message = "",
                        std::string file="",
                        int         line=0) : AOVarException( message, var, VAR_RANGE_ERROR, file, line)
   {
      _index = index;
      exception_id = "AOVarRangeException";
   }

   std::string str()
    {
    std::ostringstream s;
    s << AOVarException::str();
    s << "accessing index " << _index << " (range is 0-" << _varlen-1 << ")" << std::endl;
    return s.str();
    }

public:
   int _index;
};

class AOVarLenException : public AOVarException
{
public:
   AOVarLenException( RTDBvar &var,
                      int len,
                      std::string message = "",
                      std::string file="",
                      int line=0) : AOVarException( message, var, VAR_SIZE_ERROR, file, line)
   {
      _len = len;
      exception_id = "AOVarLenException";
   }

   std::string str()
      {
      std::ostringstream s;
      s << AOVarException::str();
      s << "expected " << _varlen << ", got " << _len << std::endl;
      return s.str();
      }

public:
   int _len;
};


//
// 
//
//

class AOShMemException : public AOException
{
public:
    AOShMemException( 
            std::string message,
            BufInfo &info,
            int errcode = NO_ERROR,
            std::string file="",
            int line = 0) : AOException( message, errcode, file, line)
    {
        _info = (BufInfo*)&info;
        exception_id = "AOShMemException";
    }
  
    std::string str()
    {
        std::ostringstream s;
        s << "Shared Buffer: " << _info->com.producer << ":" << _info->com.name << std::endl;
        return s.str();
    }

public:
    BufInfo* _info;
};



class AOTimeoutException : public AOException
{
public:
    AOTimeoutException( 
            std::string message,
            int milliseconds,
            int errcode = TIMEOUT_ERROR,
            std::string file="",
            int line = 0) : AOException( message, errcode, file, line)
    {
        _milliseconds = milliseconds;
        exception_id = "AOTimeoutException";
    }
  
    std::string str()
    {
        std::ostringstream s;
        s << AOException::str();
        s << " after " << _milliseconds << " milliseconds." << std::endl;
        return s.str();
    }

public:
    int _milliseconds;
};


//@
//
//@
class AOInvalidNumberException : public AOException
{
public:
    AOInvalidNumberException( 
            std::string message,
            int errcode = INVALID_NUMBER_ERROR,
            std::string file="",
            int line = 0) : AOException( message, errcode, file, line)
    {
        exception_id = "AOInvalidNumberException";
    }
  
    std::string str()
    {
        std::ostringstream s;
        s << AOException::str();
        s << std::endl;
        return s.str();
    }

public:
};

//@
//
//@
class AOOutOfRangeException : public  AOInvalidNumberException
{
public:
    AOOutOfRangeException(
            std::string message,
            std::string file="",
            int line =0 
            ) : AOInvalidNumberException(message, VALUE_OUT_OF_RANGE_ERROR, file, line)
    {
        exception_id = "AOOutOfRangeException";
    }
};

//@
//
//@
class AONaNException : public AOInvalidNumberException
{
    public:
    AONaNException(
            std::string message,
            std::string file="",
            int line =0 
            ) : AOInvalidNumberException(message, NAN_ERROR, file, line)
    {
        exception_id = "AONaNException";
    }
};

//@
//
//@
class AOBadConversionException : public AOInvalidNumberException
{
    public:
    AOBadConversionException(
            std::string message,
            std::string file="",
            int line =0 
            ) : AOInvalidNumberException(message, BAD_CONVERSION_ERROR, file, line)
    {
        exception_id = "AOBadConversionException";
    }
};









#endif // AOEXCEPT_H_INCLUDED

