//@File: RTDBvar.cpp
//
// Implementation of the RTDBvar class
//@

#include <cstring>
#include <string>

#include <iostream>
#include <sstream>

#include <cstdlib>

#include "Logger.h"
#include "RTDBvar.h"
#include "AOExcept.h"

extern "C" {
	#include "base/thrdlib.h"
}

#define ROUNDTRIP_TIMEOUT (1000)

// Uitlity functions

/*
string getStringRTDB(Variable *var) //@P{var}: RTDB Variable structure
{
   if(var->H.Type != CHAR_VARIABLE) return "";
   return string(var->Value.Sv,var->H.NItems);
}


int getIntRTDB(Variable *var, int ix) //@P{var}: RTDB Variable structure
                                       //@P{ix}: Variable index (default:0)
{
   if(var->H.Type != INT_VARIABLE) return Arcetri::NaNutils::lNaN(); 
   if(ix<0 || ix>=var->H.NItems) return Arcetri::NaNutils::lNaN();

   return var->Value.Lv[ix];
}


double getRealRTDB(Variable *var, int ix) //@P{var}: RTDB Variable structure
                                          //@P{ix}: Variable index (default:0)
{
   if(var->H.Type != REAL_VARIABLE) return Arcetri::NaNutils::dNaN(); 
   if(ix<0 || ix>=var->H.NItems) return Arcetri::NaNutils::dNaN();
   return var->Value.Lv[0];
}

*/





//********************************* PUBLIC **************************************** //

// Deprecated Constructors

/*
RTDBvar::RTDBvar() {
	_buffer = NULL;
    _own = false;
}

RTDBvar::RTDBvar(string prefix, string name, int dir, bool global, int type, int len, bool create) {
	 string fullname = RTDBvar::varname(prefix, dir, name);
    init(fullname, dir, type, len, create);
}

RTDBvar::RTDBvar( string prefix, const char *name, int dir, bool global, int type, int len, bool create) {
	 string fullname = RTDBvar::varname(prefix, dir, name);
    init(fullname, dir, type, len, create);
}

RTDBvar::RTDBvar(string fullname, bool global, int type, int len, bool create) {
   init( fullname, NO_DIR, type, len, create);
}

RTDBvar::RTDBvar(Variable *TheVar) {
	_buffer = NULL;
   CreateFrom(TheVar);
}

RTDBvar::RTDBvar(const RTDBvar &from) {
	_buffer = NULL;
    CopyConstructor(from);
}
*/

// Constructors

RTDBvar::RTDBvar() {
    _buffer = NULL;
    _own = false;
    pthread_mutex_init(&_mutex, NULL);
}

RTDBvar::RTDBvar(string prefix, string name, int dir, int type, int len, bool create) {
    string fullname = RTDBvar::varname(prefix, dir, name);
    pthread_mutex_init(&_mutex, NULL);
    init(fullname, dir, type, len, create);
}

RTDBvar::RTDBvar(string fullname, int type, int len, bool create) {
    pthread_mutex_init(&_mutex, NULL);
    init( fullname, NO_DIR, type, len, create);
}

RTDBvar::RTDBvar(Variable *TheVar) {
    _buffer = NULL;
    pthread_mutex_init(&_mutex, NULL);
    CreateFrom(TheVar);
}


RTDBvar::RTDBvar(const RTDBvar &from) {
    _buffer = NULL;
    pthread_mutex_init(&_mutex, NULL);
    CopyConstructor(from);
}


void RTDBvar::Create() {
	
	// Create a variable in the RTDB
   	CreateInternal();
   	Set_internal();

/*
   	if (_global)
      	if (IS_ERROR(stat = thVarOp( (char *)_complete_name.c_str(), GIVEVAR, 0, ROUNDTRIP_TIMEOUT)))
         	throw AOVarException("Error releasing ownership of the variable", *this, stat);
*/
}

void RTDBvar::Update() {
	int stat;
	
   if(!_created) CreateInternal();

	// Retrieve the value from RTDB
    MsgBuf* msgb = thGetVar((char*)_complete_name.c_str(), ROUNDTRIP_TIMEOUT, &stat);
    if (!msgb) {
        throw AOVarException("thGetVar error", *this, stat);
    }
  	Variable* var = thValue(msgb);

    pthread_mutex_lock(&_mutex);
    try {  	
        unsigned char* bufferOld = _buffer;
        // Update the local object
        if ((_type == CHAR_VARIABLE) && (var->H.Type == CHAR_VARIABLE)) {
            _len = var->H.NItems;
        }
        unsigned char* bufferTmp = new unsigned char [_len*_bpe];
        _buffer = bufferTmp;
        memcpy(_buffer, &(var->Value.Sv[0]), _len*_bpe);
        if(memcmp(bufferOld, _buffer, _len*_bpe) != 0) {
            Logger::get()->log(Logger::LOG_LEV_DEBUG, "Value of RTDB var %s updated", _complete_name.c_str());
       	} 
        else {
            Logger::get()->log(Logger::LOG_LEV_TRACE, "Value of RTDB var %s not changed (from last update)", _complete_name.c_str());
        }
    
        memcpy(&_mTime, &var->H.MTime, sizeof(struct timeval));
        // Clean up
        if (bufferOld) delete[] bufferOld;
        }
    catch (...) {
        pthread_mutex_unlock(&_mutex);
        throw;
    }
    pthread_mutex_unlock(&_mutex);

    thRelease(msgb);
}


RTDBvar::~RTDBvar()
{
	  Delete();
}


RTDBvar& RTDBvar::operator=(const RTDBvar& other) {
	CopyConstructor(other);
    return *this;
}


string RTDBvar::Get() {
    if (_type != CHAR_VARIABLE)  
        throw AOVarTypeException(*this, CHAR_VARIABLE, "string RTDBvar::Get() called on non-char variable", __FILE__, __LINE__);

    string retval;
    pthread_mutex_lock(&_mutex);
    try {
        if (!_buffer)
          throw AOVarException("Variable uninitialized" , *this, NOT_INIT_ERROR, __FILE__, __LINE__);

        int pos=0;
        while ( (pos<_len) && (((char*)_buffer)[pos]))
           pos++;

        if (pos==_len)
          retval = string((char*)_buffer, _len);
    else 
          retval = string((char*)_buffer);
    }
    catch (...) {
        pthread_mutex_unlock(&_mutex);
        throw;
    }
    pthread_mutex_unlock(&_mutex);
    return retval;
}


int RTDBvar::MatchAndSet(Variable *TheVar) {
   if (strcmp( TheVar->H.Name, _complete_name.c_str()) == 0)
      {  
      try {
         Set(TheVar);
      } catch ( AOVarException e) 
         {
         return e._errcode;
         }
      return 1;
      }
   return 0;
}


void RTDBvar::Set(Variable *TheVar) {
   pthread_mutex_lock(&_mutex);
   try {

   switch(_type) {
      case INT_VARIABLE:
	      memcpy( _buffer, TheVar->Value.Lv, _len*_bpe);
	      break;

      case REAL_VARIABLE:
	      memcpy( _buffer, TheVar->Value.Dv, _len*_bpe);
	      break;

      case CHAR_VARIABLE:
         CreateFrom(TheVar);
	      break;

      case BIT8_VARIABLE:
	      memcpy( _buffer, TheVar->Value.B8, _len*_bpe);
	      break;

      case BIT16_VARIABLE:
	      memcpy( _buffer, TheVar->Value.B16, _len*_bpe);
	      break;

      case BIT32_VARIABLE:
	      memcpy( _buffer, TheVar->Value.B32, _len*_bpe);
	      break;

      case BIT64_VARIABLE:
	      memcpy( _buffer, TheVar->Value.B64, _len*_bpe);
	      break;

      default:
      	throw AOVarException("Unexpected variable type", *this, VAR_TYPE_ERROR, __FILE__, __LINE__);
      }
    memcpy(&_mTime, &TheVar->H.MTime, sizeof(struct timeval));

    }
    catch (...) {
        pthread_mutex_unlock(&_mutex);
        throw;
    }
    pthread_mutex_unlock(&_mutex);

}


void RTDBvar::Set( string value, send_actions send) {
   if (value.size()<1)
      return;

   check_type( value[0]);
   if ( _len != (int)value.size()+1)
      {
      _len = (int)value.size()+1;
      CreateInternal();
      }

   Update_contents( (char*)value.c_str(), send);   // This is the correct way to get the buffer address
}


void RTDBvar::Set( char *value, send_actions send) {
   string s = value;
   RTDBvar::Set( s, send);
}



string RTDBvar::strtype( int type)
{
   switch(type)
   {
      case INT_VARIABLE:
         return "INT_VARIABLE";
      case REAL_VARIABLE:
         return "REAL_VARIABLE";
      case CHAR_VARIABLE:
         return "CHAR_VARIABLE";
      case BIT8_VARIABLE:
         return "BIT8_VARIABLE";
      case BIT16_VARIABLE:
         return "BIT16_VARIABLE";
      case BIT32_VARIABLE:
         return "BIT32_VARIABLE";
      case BIT64_VARIABLE:
         return "BIT64_VARIABLE";

      default:
         return "Invalid type";
   }
}


void RTDBvar::Delete() {
//	int stat;

   if (_buffer) delete[] _buffer;
   _buffer = NULL;

	// Can't do this! See note in the method prototype.
//	if (_own) {
//		if (IS_ERROR(stat = thVarOp( (char *)_complete_name.c_str(), DELVAR,0,ROUNDTRIP_TIMEOUT))) {
//        	throw AOVarException( "Error deleting variable", *this, stat);
//		}
//    }
}


void RTDBvar::Send() {
   Set_internal();
}


string RTDBvar::varname(string device_type, int direction, string name) {
   string separator = ".";
   string dir;
   ostringstream s;

   if (direction == NO_DIR)
      dir = "";
   else if (direction == CUR_VAR)
      dir = separator + "CUR";
   else if (direction == REQ_VAR)
      dir = separator + "REQ";

   s << device_type;

   s << separator << name << dir;

   return s.str();
}


//********************************* PROTECTED **************************************** //

int RTDBvar::TypeToBpe( int type)
{
   switch(type)
      {
      case INT_VARIABLE:
         return sizeof(int);
      case REAL_VARIABLE:
         return sizeof(double);
      case CHAR_VARIABLE:
         return sizeof(char);
      case BIT8_VARIABLE:
         return sizeof(uint8);
      case BIT16_VARIABLE:
         return sizeof(uint16);
      case BIT32_VARIABLE:
         return sizeof(uint32);
      case BIT64_VARIABLE:
         return sizeof(uint64);
      default:
         return -1;
      }
}

void RTDBvar::CreateInternal() {

   pthread_mutex_lock(&_mutex);
   try{

   if (_buffer)
   delete[] _buffer;

   switch( _type) {
      case INT_VARIABLE:
	      _buffer = (unsigned char *) new int[_len];
	      _bpe = sizeof(int);
	      break;

      case REAL_VARIABLE:
	      _buffer = (unsigned char *) new double[_len];
	      _bpe = sizeof(double);
	      break;

      case CHAR_VARIABLE:
	      _buffer = new unsigned char[_len];
	      _bpe = sizeof(unsigned char);
	      break;

      case BIT8_VARIABLE:
	      _buffer = new uint8[_len];
	      _bpe = sizeof(uint8);
	      break;

      case BIT16_VARIABLE:
	      _buffer = (unsigned char *) new uint16[_len];
	      _bpe = sizeof(uint16);
	      break;

      case BIT32_VARIABLE:
	      _buffer = (unsigned char *) new uint32[_len];
	      _bpe = sizeof(uint32);
	      break;

      case BIT64_VARIABLE:
	      _buffer = (unsigned char *) new uint64[_len];
	      _bpe = sizeof(uint64);
	      break;

      default:
	      _buffer = NULL;
	      throw AOVarException( "Unexpected variable type", *this, VAR_TYPE_ERROR, __FILE__, __LINE__);
      }

   memset( _buffer, 0, _len*_bpe);

   memset(&_mTime, 0, sizeof(struct timeval));
   _created=true;

   }
   catch (...) {
       pthread_mutex_unlock(&_mutex);
       throw;
   }
   pthread_mutex_unlock(&_mutex);
}


void RTDBvar::Update_contents( void *value, int send)
{
   int modified;
   pthread_mutex_lock(&_mutex);
   try{
       modified = memcmp( _buffer, value, _bpe*_len);
       if (modified)
          memcpy( _buffer, value, _bpe*_len);
   }
   catch (...) {
       pthread_mutex_unlock(&_mutex);
       throw;
   }
   pthread_mutex_unlock(&_mutex);

   if ((send == FORCE_SEND) || ((modified) && (send == CHECK_SEND)))
       Set_internal();
}


void RTDBvar::Set_internal() {
   	int stat;
 	if (IS_ERROR(stat = thWriteVar( (char *)_complete_name.c_str(), _type, _len, _buffer, ROUNDTRIP_TIMEOUT))) {
		throw AOVarException("Error writing variable", *this, stat, __FILE__, __LINE__);
	}
}


// The "value=0" instruction is only to avoid a warning at compile time.
void RTDBvar::check_type( uint8 value)  { value=0; if (_type != BIT8_VARIABLE)   throw AOVarTypeException( *this, BIT8_VARIABLE, "Type check failed", __FILE__, __LINE__); }
void RTDBvar::check_type( uint16 value) { value=0; if (_type != BIT16_VARIABLE)  throw AOVarTypeException( *this, BIT16_VARIABLE, "Type check failed", __FILE__, __LINE__); }
void RTDBvar::check_type( uint32 value) { value=0; if (_type != BIT32_VARIABLE)  throw AOVarTypeException( *this, BIT32_VARIABLE, "Type check failed", __FILE__, __LINE__); }
void RTDBvar::check_type( uint64 value) { value=0; if (_type != BIT64_VARIABLE)  throw AOVarTypeException( *this, BIT64_VARIABLE, "Type check failed", __FILE__, __LINE__); }
void RTDBvar::check_type( long value)   { value=0; if (_type != INT_VARIABLE)    throw AOVarTypeException( *this, INT_VARIABLE, "Type check failed", __FILE__, __LINE__); }
void RTDBvar::check_type( double value) { value=0; if (_type != REAL_VARIABLE)   throw AOVarTypeException( *this, REAL_VARIABLE, "Type check failed", __FILE__, __LINE__); }
void RTDBvar::check_type( char  value)  { value=0; if (_type != CHAR_VARIABLE)   throw AOVarTypeException( *this, CHAR_VARIABLE, "Type check failed", __FILE__, __LINE__); }
void RTDBvar::check_type( int  value)   { value=0; if (_type != INT_VARIABLE)    throw AOVarTypeException( *this, INT_VARIABLE, "Type check failed", __FILE__, __LINE__); }
void RTDBvar::check_type( string value) { value=""; if (_type != CHAR_VARIABLE)   throw AOVarTypeException( *this, CHAR_VARIABLE, "Type check failed", __FILE__, __LINE__); }

void RTDBvar::check_element( int wanted_element) {
   if ((wanted_element >= _len) || (wanted_element < 0))
      throw AOVarRangeException( *this, wanted_element, "", __FILE__, __LINE__);
}


void RTDBvar::check_len( int wanted_len) {
   if (_len != wanted_len)
      throw AOVarLenException( *this, wanted_len, "", __FILE__, __LINE__);
}


void RTDBvar::check_pointer( void *pointer) {
   if (pointer == NULL)
      throw AOException("NULL pointer", NULL_BUFFER_ERROR, __FILE__, __LINE__);
}

//********************************* PRIVATE **************************************** //


void RTDBvar::init(string name, int dir, int type, int len, bool create) {

	// --- Try to attach an RTDB existing variable --- //
	 _complete_name = name;
	try {
		_buffer = NULL;
     	_own = false;
		CreateFrom(_complete_name);
		Logger::get()->log(Logger::LOG_LEV_DEBUG, "Existing RTDB var %s attached", _complete_name.c_str());
	}
	catch(AOVarException& e) {
		
		// --- Create ex-novo --- //
	   _type   = type;
	   _len    = len;
	   _dir    = dir;
      memset(&_mTime, 0, sizeof(struct timeval));
	
	   _own = true;
	   _buffer = NULL;
	
	   if(create) {
	   		Create();	
	   		Logger::get()->log(Logger::LOG_LEV_DEBUG, "Created RTDB var %s", _complete_name.c_str());
	   }
	   else {
	   		Logger::get()->log(Logger::LOG_LEV_DEBUG, "Initialized RTDB var %s. Please create it!", _complete_name.c_str());
	   }
	}

}

void RTDBvar::CreateFrom(Variable *var) {
    unsigned char *bufferOld;
    unsigned char *bufferTmp;
    _own = false;
    _buffer = NULL;

   _type = var->H.Type;
   _len  = var->H.NItems;

   _complete_name = var->H.Name;
   _bpe = TypeToBpe(_type);
   if (_bpe == -1)
      throw AOVarException("Invalid variable type", *this, VAR_TYPE_ERROR, __FILE__, __LINE__);

   //@C
   // this is a kind of realloc to prevent 
   // beeing interrupted after \verb+delete[]+ 
   // and before \verb+new+
   //@ 
   bufferOld = _buffer;
   bufferTmp = new unsigned char [_len*_bpe];
   _buffer = bufferTmp;
   if (bufferOld) delete[] bufferOld;

   memcpy(_buffer, &(var->Value.Sv[0]), _len*_bpe);   
   memcpy(&_mTime, &var->H.MTime, sizeof(struct timeval));
 
   _created=true;
}


void RTDBvar::CreateFrom(string varname) {
    MsgBuf *msgb;
    int stat;
    _buffer = NULL;

    msgb = thGetVar((char *)varname.c_str(), ROUNDTRIP_TIMEOUT,&stat);
    if (!msgb) throw AOVarException("thGetVar error", *this, stat, __FILE__, __LINE__);
  
    CreateFrom( thValue(msgb));

    thRelease(msgb);
}

void RTDBvar::CopyConstructor( const RTDBvar &from)
{
//  _prefix     = from._prefix;
//  _name       = from._name;
//  _global     = from._global;
    _type       = from._type;
    _bpe        = from._bpe;
    _len        = from._len;
    _own        = from._own;
    _complete_name = from._complete_name;
    _created    = from._created;
    _dir        = from._dir;
    _mTime      = from._mTime;

    if (from._buffer)
    {
    	_buffer = new unsigned char[_len*_bpe];
        memcpy( _buffer, from._buffer, _len*_bpe);
    }
    else 
    {
    	_buffer = NULL;
    }
}

// **************** TEMPLATES INSTANTIATION ********************* //


template void RTDBvar::Set<uint8>( uint8, int, send_actions);
template void RTDBvar::Set<uint16>( uint16, int, send_actions);
template void RTDBvar::Set<uint32>( uint32, int, send_actions);
template void RTDBvar::Set<uint64>( uint64, int, send_actions);
template void RTDBvar::Set<int>( int, int, send_actions);
template void RTDBvar::Set<float>( float, int, send_actions);
template void RTDBvar::Set<double>( double, int, send_actions);
template void RTDBvar::Set<char>( char, int, send_actions);

template void RTDBvar::Set<uint8>( uint8 *, send_actions);
template void RTDBvar::Set<uint16>( uint16 *, send_actions);
template void RTDBvar::Set<uint32>( uint32 *, send_actions);
template void RTDBvar::Set<uint64>( uint64 *, send_actions);
template void RTDBvar::Set<int>( int *, send_actions);
template void RTDBvar::Set<float>( float *, send_actions);
template void RTDBvar::Set<double>( double *, send_actions);


template void RTDBvar::Set<uint8>( vector<uint8> &, send_actions);
template void RTDBvar::Set<uint16>( vector<uint16> &, send_actions);
template void RTDBvar::Set<uint32>( vector<uint32> &, send_actions);
template void RTDBvar::Set<uint64>( vector<uint64> &, send_actions);
template void RTDBvar::Set<int>( vector<int> &, send_actions);
template void RTDBvar::Set<float>( vector<float> &, send_actions);
template void RTDBvar::Set<double>( vector<double> &, send_actions);

template void RTDBvar::Get<uint8>( int, uint8 *);
template void RTDBvar::Get<uint16>( int, uint16 *);
template void RTDBvar::Get<uint32>( int, uint32 *);
template void RTDBvar::Get<uint64>( int, uint64 *);
template void RTDBvar::Get<int>( int, int *);
template void RTDBvar::Get<double>( int, double *);
template void RTDBvar::Get<char>( int, char *);

template void RTDBvar::Get<uint8>( uint8 *);
template void RTDBvar::Get<uint16>( uint16 *);
template void RTDBvar::Get<uint32>( uint32 *);
template void RTDBvar::Get<uint64>( uint64 *);
template void RTDBvar::Get<int>( int *);
template void RTDBvar::Get<double>( double *);
template void RTDBvar::Get<char>( char *);

template void RTDBvar::Get<uint8>( uint8 **);
template void RTDBvar::Get<uint16>( uint16 **);
template void RTDBvar::Get<uint32>( uint32 **);
template void RTDBvar::Get<uint64>( uint64 **);
template void RTDBvar::Get<int>( int **);
template void RTDBvar::Get<double>( double **);

template <>
void RTDBvar::Set( float value, int index, send_actions send) {
   double v = value;
   Set(v, index, send);
}

template <>
void RTDBvar::Set( float * /* value*/, int /* index */, send_actions /* send */) {
      throw AOException("RTDBvar::Set not implemented for float array type. Use double instead.");
}

template <>
void RTDBvar::Set( vector<float>, int /* index */, send_actions /* send */) {
      throw AOException("RTDBvar::Set not implemented for float vector type. Use double instead.");
}

