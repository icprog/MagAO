//@File: stdconfig.h
//
// C++ classes and functions to read standard configuration files.
//
// The utility function "ReadConfigFile" was preserved to
// guarantee backward compatibility.
//
//
// Type conversion:
// -------------------------------------------------------------------
// Config file					aotypes				(Config_Value)
// -------------------------------------------------------------------
// int, int32					int32				(int_value)
// uint, uint32, ulong			uint32				(uint_value)
// int64						int64				(int64_value)
// uint64						uint64				(uint64_value)
// float, float32				float32				(float_value)
// double, float64				float64				(float64_value)
// string						string*				(char_value)
// structure**					string*				(-)
// array***						vector<string>*		(vector<string>)
// -------------------------------------------------------------------
// [* not an aotype]
// [** reference to a configuration filename]
// -------------------------------------------------------------------
//
// *** Array example (in config file):
//
//		OP_MODES	array	 [debug, alignement, complete]
//
// Note that:
// 	- Array CAN be delimited by '[' and ']', but this is not mandatory
//  - Spaces doesn't have any effect
//  - Elements must be delimited by ','
//
// HISTORY
//
// 26/01/2007  Alfio Puglisi added extract()
// 12/07/2006	Fabio Tosetti	Now consider "structure" type variables and recursively
//								parses files referred.
//								Created "Config_File class" instead of "config_file typedef"
//								for a better extensibility.
//								Added some types to Config_Value class: int64, uint32, uint64,
//								float32, float64.
//								Defined Config_File_Exception calls and exception throw in case
//								of not existing configuration file.
//								Overridden Config_File::operator[](key) to raise exception
//                              key is not found. Added method Config_File::insert(key,value)
//								to correctly perform insertion (cant' use operator[] !!!)
//								Renamed class value_type to Config_Value to avoid name conflict
//								with "value_type" parameter used in std::map<...> definition.
//
//@

#ifndef STDCONFIG_H_INCLUDED
#define STDCONFIG_H_INCLUDED


extern "C" {
#include "aotypes.h"
}

#include "AOExcept.h"
#include "Logger.h"
using namespace Arcetri;

#include <map>
#include <vector>
#include <string>
#include <cstring>
using namespace std;

#include <libgen.h>   // dirname()

// Root namespace
namespace Arcetri {


// Forward definitions
class Config_File;
class Config_Value;
class Config_File_Exception;



// ************************ CLASSES ************************ //


//@Class: Config_File_Exception
//
// LBT Configuration support class
//
// \config_file_excepiton\ defines a configuration exception
// thrown by the Config_File class methods
//@
class Config_File_Exception: public AOException {

	public:
		explicit Config_File_Exception(string m): AOException(m) { exception_id = "Config_File_Exception"; }
		explicit Config_File_Exception(string m, int errcode, string file, int line): AOException(m, errcode, file, line) { exception_id = "Config_File_Exception"; }
		virtual ~Config_File_Exception() throw() {}
		//const char * str() const throw() { return _message.c_str(); }
};


//@Class: Config_File
//
// LBT Configuration support class
//
// \Config_File\ defines a configuration dictionary got
// from a configuration file
//
// The dictionary is provided as a class for a better extendibility.
//
// ATTENTION: see overriding of operator[] to use this class !!!
//@
class Config_File: public map<string, Config_Value> {

	public:

		//@Member: setLogLevel
		//
		//Set a log level different from the default one
		//@
		static void setLogLevel(int logLevel) { _logLevel = logLevel; }

		//@Member: Config_File
		//
		// Constructor
		//
		// Create an empty Config_File. (It's implicitly called for
		// declaration like "Config_File cf;")
		//@
		Config_File();

		//@Member: Config_File
		//
		//Constructor
		//
		//Create a Config_File basing on a configuration
		//file.
		//@
		Config_File(string filename, 		     //@P{filename}: configuration file path relative
										         //				to "ADOPT_CONF/" directory; usually
											     //				it specify a "config" directory and
											     //				a "subsystem" subdirectory.
												 //				i.e. conf/w2/ccd.ext, conf/w1/tt.ext,
											     // 				conf/adsec/mirror.ext
				 	string namePrefix = "",      //@P{namePrefix}: the prefix for the variables name
											     //				 (dictionary keys) found in config file
					string delimiters = " \t")   //@P{delimiters}: delimiters allowed in config file

					throw (Config_File_Exception);


		//@Member operator[]
		//
		// Return the element corresponding to given key, or raise an exception
		// if key not found.
		//
		// ATTENTION: use it only when extracting an element ! When
		// inserting an element you have to use use insert(key,value)
		//@
		Config_Value& operator[](const string& key) throw (Config_File_Exception);

		//@Member Insert
		//
		// Utility overload to replace operator[] in the left side of assignment.
		// Return true only if element have been inserted, that is if it wasn't
		// already present
		//@
		bool insert(string key, Config_Value value);
		using map<string, Config_Value>::insert; //Guarantee visibility for base
												 //class insert methods

        bool has_key( const string &key);

		string getCfgFile() { return _cfgFile; }
		string getDelimiters() { return _delimiters;  }
      string getDir(); 

      //@Member extract
      //
      // Extracts a sub-tree from the configuration file and
      // returns it as a new object
      //@

      Config_File *extract(const string &prefix);

      void dump();

	private:

		// Logging
		static const int LOG_LEVEL_DEFAULT = Logger::LOG_LEV_INFO;
		static int _logLevel;
		static Logger* _logger;

		// Parsed config file (path relative to $ADOPT_ROOT)
		string _cfgFile;
		// Allowed whitespace characters
		string _delimiters;

			//@ Member: getAbsolutePath
		//
		// Get the configuration file from the filename  and return an input stream.
		// The filename is a path relative to configuration root (env var $(ADOPT_ROOT)) and contains:
	    //  - "config" directory (usually "conf")
	    //  - "subsystem" sub-directory ("adsec", "w2" etc)
		// If the $(ADOPT_ROOT) or file doesn't exist throw a Config_File_Exception
		//
		// ATTENTION: the client MUST take care of deleting the returned object !!!
		//@
		static ifstream* openConfigFile(string filename) throw (Config_File_Exception);

		// Takes a config line and parse it to a vector of strings
		vector<string> parseArray(string arrayValue);
};



//@Class: Config_Value
//
// LBT Configuration support class
//
// \Config_Value\ defines a configuration value type, that is the type
// corresponding to a single configuration parameter written in a
// config file.
//
// A configuration parameter have the following format:
// \item\ name: name of the parameter
// \item\ type: type of the parameter, wrapped in \Config_Value\ class
// \item\ value: value of the parameter
//@
class Config_Value {

public:

	// --- CONSTRUCTORS --- //
	// When a new_type is added, the corresponding constructor should be
	// defined to allow expressions like "Config_Value v = new_type_value",
	// since this doesn't calls assignement operator, but the constructor !

	Config_Value() { zero(); }

 	Config_Value(const int32 &value) {
 		zero();
 		int_value = (int32)value;
		int64_value = (int64)value;
		uint_value = (uint32)value;
		uint64_value = (uint64)value;
		float_value = (float32)value;
		float64_value = (double)value;
      ostringstream ostr;
      ostr<<value;
      char_value = ostr.str();
 	}

 	Config_Value(const int64 &value) {
 		zero();
 		int_value = (int32)value;
		int64_value = (int64)value;
		uint_value = (uint32)value;
		uint64_value = (uint64)value;
		float_value = (float32)value;
		float64_value = (double)value;
      ostringstream ostr;
      ostr<<value;
      char_value = ostr.str();
 	}

 	Config_Value(const uint32 &value) {
 		zero();
 		int_value = (int32)value;
		int64_value = (int64)value;
		uint_value = (uint32)value;
		uint64_value = (uint64)value;
		float_value = (float32)value;
		float64_value = (double)value;
      ostringstream ostr;
      ostr<<value;
      char_value = ostr.str();
 	}

 	Config_Value(const uint64 &value) {
 		zero();
 		int_value = (int32)value;
		int64_value = (int64)value;
		uint_value = (uint32)value;
		uint64_value = (uint64)value;
		float_value = (float32)value;
		float64_value = (double)value;
      ostringstream ostr;
      ostr<<value;
      char_value = ostr.str();
 	}

 	Config_Value(const float32 &value) {
 		zero();
 		int_value = (int32)value;
		int64_value = (int64)value;
		uint_value = (uint32)value;
		uint64_value = (uint64)value;
		float_value = (float32)value;
		float64_value = (double)value;
      ostringstream ostr;
      ostr<<value;
      char_value = ostr.str();
 	}

 	Config_Value(const float64 &value) {
 		zero();
 		int_value = (int32)value;
		int64_value = (int64)value;
		uint_value = (uint32)value;
		uint64_value = (uint64)value;
		float_value = (float32)value;
		float64_value = (double)value;
      ostringstream ostr;
      ostr<<value;
      char_value = ostr.str();
 	}

 	Config_Value(const string &value) {
 		zero();
 		int_value = (int32)atoi(value.c_str());
		int64_value = (int64)atol(value.c_str());
		uint_value = (uint32)atoi(value.c_str());
		uint64_value = (uint64)atol(value.c_str());
		float_value = (float32)atof(value.c_str());
		float64_value = (double)atof(value.c_str());
 		char_value = value;
 	}

 	Config_Value(vector<string> array) {
 		array_value = array;
 	}

    Config_Value(const Config_Value &other)
    {
        CopyConstructor(other);
    }

	// --- UTILITY METHODS --- //

	void zero()
    {
    	int_value = 0;
    	int64_value = 0;
    	uint_value = 0;
    	uint64_value = 0;
    	float_value = 0;
    	float64_value = 0;
    	char_value = "";
    	array_value.clear();
    }

    // --- ASSIGNMENT OVERRIDING --- //

    Config_Value& operator=(const int32& value)
    {
    	zero();
 		int_value = (int32)value;
		int64_value = (int64)value;
		uint_value = (uint32)value;
		uint64_value = (uint64)value;
		float_value = (float32)value;
		float64_value = (double)value;
      ostringstream ostr;
      ostr<<value;
      char_value = ostr.str();
     	return *this;
    }

    Config_Value& operator=(const int64& value)
    {
    	zero();
 		int_value = (int32)value;
		int64_value = (int64)value;
		uint_value = (uint32)value;
		uint64_value = (uint64)value;
		float_value = (float32)value;
		float64_value = (double)value;
      ostringstream ostr;
      ostr<<value;
      char_value = ostr.str();
    	return *this;
    }

    Config_Value& operator=(const uint32& value)
    {
    	zero();
 		int_value = (int32)value;
		int64_value = (int64)value;
		uint_value = (uint32)value;
		uint64_value = (uint64)value;
		float_value = (float32)value;
		float64_value = (double)value;
      ostringstream ostr;
      ostr<<value;
      char_value = ostr.str();
    	return *this;
    }

	Config_Value& operator=(const uint64& value)
    {
    	zero();
 		int_value = (int32)value;
		int64_value = (int64)value;
		uint_value = (uint32)value;
		uint64_value = (uint64)value;
		float_value = (float32)value;
		float64_value = (double)value;
      ostringstream ostr;
      ostr<<value;
      char_value = ostr.str();
    	return *this;
    }

    Config_Value& operator=(const float32& value)
    {
        zero();
 		int_value = (int32)value;
		int64_value = (int64)value;
		uint_value = (uint32)value;
		uint64_value = (uint64)value;
		float_value = (float32)value;
		float64_value = (double)value;
        ostringstream ostr;
        ostr<<value;
        char_value = ostr.str();
        return *this;
    }

    Config_Value& operator=(const float64& value)
    {
        zero();
 		int_value = (int32)value;
		int64_value = (int64)value;
		uint_value = (uint32)value;
		uint64_value = (uint64)value;
		float_value = (float32)value;
		float64_value = (double)value;
        ostringstream ostr;
        ostr<<value;
        char_value = ostr.str();
        return *this;
    }

    Config_Value& operator=(const string& value)
    {
        zero();
 		int_value = (int32)atoi(value.c_str());
		int64_value = (int64)atol(value.c_str());
		uint_value = (uint32)atoi(value.c_str());
		uint64_value = (uint64)atol(value.c_str());
		float_value = (float32)atof(value.c_str());
		float64_value = (double)atof(value.c_str());
        char_value = value;
        return *this;
    }

    Config_Value& operator=(const vector<string>& value)
	{
		zero();
		array_value = value;
		return *this;
	}

    Config_Value& operator=(const Config_Value& other)
    {
        CopyConstructor(other);
        return *this;
    }


	 // --- RELATIONAL OPERATORS OVERRIDING --- //

    bool operator<(const Config_Value& other)
    {
		return ((int_value < other.int_value) ||
				(int64_value < other.int64_value) ||
				(uint_value < other.uint_value) ||
				(uint64_value < other.uint64_value) ||
				(float_value < other.float_value) ||
				(float64_value < other.float64_value) ||
				(strcmp(char_value.c_str(),other.char_value.c_str()) < 0));
    }

    bool operator==(Config_Value& other)
    {
        return (!(*this < other) && !(other < *this));
    }

	// --- CASTING OVERRIDING --- //
	// Overrides cast for aotypes supported
    operator  int32()   { return int_value; }
    operator  int64()   { return int64_value; }
    operator  uint32()  { return uint_value; }
    operator  uint64()  { return uint64_value; }
    operator  float32() { return float_value; }
	 operator  float64()  { return float64_value; }
	 //operator  double()   { return float64_value; }
	 operator  string()   { return char_value; }
	 operator  vector<string>() { return array_value; }

	protected:
		// Supported aotypes
		int32 	int_value;
		int64 	int64_value;
		uint32 	uint_value;
		uint64 	uint64_value;
		float32  float_value;
		double 	float64_value;
		string 	char_value;
		vector<string> array_value;

	private:

		void CopyConstructor(const Config_Value &other)
	    {
	    	int_value = other.int_value;
	    	int64_value = other.int64_value;
	    	uint_value = other.uint_value;
	      uint64_value = other.uint64_value;
	      float_value = other.float_value;
	      float64_value = other.float64_value;
	      char_value = other.char_value;
	      array_value = other.array_value;
	    }
};



// ************************ FUNCTIONS ************************ //


//@Function(API): ReadConfigFile
//
// Read the specified configuration file and return a dictionary
// containing <par_name, par_value>.
//
// This function was preserve for backward compatibility of the library.
//@
Config_File ReadConfigFile(string filename, 	           // @P{filename}: name of config file to parse
						   string namePrefix="")           // @P{namePrefix}: (optional) prefix for dictionary keys.
						   						           //				  The keys will be inserted in the dictionary
						   						           //				  in "prefix.key" format
						   throw (Config_File_Exception);

} //End Root namespace

#endif  // STDCONFIG_H_INCLUDED
