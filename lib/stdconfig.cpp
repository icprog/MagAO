//@File: stdconfig.cpp
//
// C++ functions and classes implementations to read standard configuration files
//

//@

#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "errno.h"
using namespace std;

#include "stdconfig.h"

int Config_File::_logLevel = Logger::LOG_LEV_INFO;
Logger* Config_File::_logger;

Config_File::Config_File() {
   _cfgFile = "";
   _delimiters = "";
}

Config_File::Config_File(string filename,
						 string namePrefix,
						 string delimiters)
						 throw(Config_File_Exception) {

	_logger = Logger::get("CONFIG-LOADER");
	_logger->setLevel(_logLevel);
	_logger->printStatus();

	_cfgFile = filename;
    _delimiters = delimiters;
    char _commentChar = '#';

    // Check file existance and get the input stream
    ifstream* f = NULL;
    f = openConfigFile(filename); // Can throw Config_File_Exception (fatal)

    // Prepare prefix for recursion
    string recurse_name = filename;
    if (recurse_name.rfind('/') != string::npos)
        recurse_name = recurse_name.substr(0, recurse_name.rfind('/'));

    // Read the configuration and create the Config_File object (dictionary)
    string line, name, type, value;
    size_t start, end, comment;
    
    while (!f->eof())
    {
    	getline(*f,line);
		_logger->log(Logger::LOG_LEV_TRACE, "-->%s<--", line.c_str());

      // Check for comment directive
      if (line.find("#Comment: ") == 0)
         _commentChar = line[10];
		//Checks if comment line
		comment = line.find(_commentChar);
		if (comment != string::npos)
		{
			//Remove comment ad the end of the line
			line = line.substr(0, comment);
		}

		// Found only comment or empty line
		if (line.size() <1) {
			continue;
		}

		// -- Get first token (par name)
        start = line.find_first_not_of(_delimiters);
        
        end = line.find_first_of(_delimiters, start);
        
        if ((start == string::npos) || (end == string::npos))
        {
            continue;
        }
        name = line.substr( start, end-start);
		_logger->log(Logger::LOG_LEV_TRACE, "Found name: %s", name.c_str());

		//Prefix the name if is called recursively
		if (namePrefix.size() != 0) {
			name = namePrefix + "." + name;
			_logger->log(Logger::LOG_LEV_TRACE, "Found inner name: %s", name.c_str());
		}

		// -- Get second token (par type)
        start = line.find_first_not_of(_delimiters,end);
        end = line.find_first_of(_delimiters, start);
        if ((start == string::npos) || (end == string::npos))
        {
            continue;
        }
        type = line.substr( start, end-start);
		_logger->log(Logger::LOG_LEV_TRACE, "Found type: %s", type.c_str());

		// -- Get thirdh token (par value)
        start = line.find_first_not_of(_delimiters, end);
        end = line.size();
        if (start == string::npos)
        {
            continue;
        }
        value = line.substr(start, end-start);
		//Remove trailing whitespace
		while (value.find_first_of(_delimiters, value.size()-1) != string::npos)
		{
			value = value.substr(0, value.size()-1);
		}
		_logger->log(Logger::LOG_LEV_TRACE, "Found value: -->%s<--", value.c_str());


        if (value[0] == '"')
		{
            value = value.substr(1);
		}
        if (value[ value.size()-1] == '"')
		{
            value = value.substr(0, value.size()-1);
		}
		_logger->log(Logger::LOG_LEV_TRACE, "Converted value: %s", value.c_str());

		// Fill the dictionary
        if ((type == "int") || (type == "int32"))
        {
            insert(name, (int32)atoi(value.c_str()));
        }
        if ((type == "int64"))
        {
            insert(name, (int64)atoll(value.c_str()));
        }
        if ((type == "uint") || (type == "uint32") || (type == "ulong"))
        {
            insert(name, (uint32)strtoul(value.c_str(), NULL, 0));
        }
        if ((type == "uint64"))
        {
            insert(name, (uint64)strtoull(value.c_str(), NULL, 0));
        }
        if ((type == "float") || (type == "float32"))
        {
            insert(name, (float32)atof(value.c_str()));
        }
        if ((type == "double") || (type == "float64"))
        {
            insert(name, (float64)atof(value.c_str()));
        }
        if (type == "string")
        {
			//insert(name, value); // Ambiguous with map::insert(InputIterator, InputIterator)
            insert(name, Config_Value(value));
        }
        if (type == "structure")
        {
        	// Recursively parse the file with name found in "value"
        	// and copy it in current dictionary
            string newname = recurse_name+"/"+value;
            Config_File innerConfig = Config_File(newname, name);
            insert(innerConfig.begin(), innerConfig.end());
        }
        if (type == "array")
        {
        	insert(name, parseArray((string)value));
        }
    }

    f->close();
    delete f;
    _logger->log(Logger::LOG_LEV_INFO, "%d parameters read from config file", this->size());

   iterator it;
   for( it = this->begin(); it != this->end(); it++)
      _logger->log(Logger::LOG_LEV_INFO, " - %s: %s", (it->first).c_str(), ((string)(it->second)).c_str());
   // TODO IT SHOULD BE LIKE THIS if ConfigValue implements <<
   //for( it = this->begin(); it != this->end(); it++){
   //   ostringstream ostr;
   //   ostr << it->first << ": " << it->second;
   //   _logger->log(Logger::LOG_LEV_INFO, " - %s", ostr.str().c_str());
   //}
}

vector<string> Config_File::parseArray(string arrayValue) {
	vector<string> array;

	// Remove leading and trailing delimiters, '[' and ']'
	arrayValue.replace(arrayValue.find("["), 1, "");
	arrayValue.replace(arrayValue.find("]"), 1, "");

	// Remove all spaces
	while(arrayValue.find(" ") != string::npos){
		arrayValue.replace(arrayValue.find(" "), 1, "");
	}

	// Parse tokens delimited by ','
	char* s = (char*)arrayValue.c_str();
	char* token = strtok(s, ",");
	while(token != NULL) {
		array.push_back(string(token));
		token = strtok(NULL, ",");
	}

	return array;
}

ifstream* Config_File::openConfigFile(string filename) throw (Config_File_Exception) {

   string absoluteFilename = filename;
   if (absoluteFilename[0] != '/') {
      // Create absolute path filename
	   absoluteFilename = Utils::getAdoptRoot() + "/" + filename;
   }

    _logger->log(Logger::LOG_LEV_INFO, "Getting configuration file '%s'", absoluteFilename.c_str());

    struct stat fileStat;
    stat(absoluteFilename.c_str(), &fileStat);

    // Check file existance
    ifstream* f = new ifstream(absoluteFilename.c_str());
    if (S_ISDIR(fileStat.st_mode) || !f->is_open())
    {
        throw Config_File_Exception("Unable to find configuration file: " + absoluteFilename);
    }
    return f;
}


Config_Value& Config_File::operator[](const string& key) throw (Config_File_Exception) {
	iterator it = this->find(key);
	if (it != this->end())
    	return it->second;
	else {
		throw Config_File_Exception("Trying to access a non existing key: " + key + ". To add a new element, use insert(string,Conf_Value) instead");
	}
}

bool Config_File::has_key( const string& key) {
    iterator it = this->find(key);
    return (it != this->end());
}

bool Config_File::insert(string key, Config_Value value) {
	bool inserted = insert(make_pair(key,value)).second;
	return inserted;
}


Config_File Arcetri::ReadConfigFile(string filename, string namePrefix) throw (Config_File_Exception)
{
    return Config_File(filename, namePrefix);
}


Config_File *Config_File::extract(const string &prefix)
{
   Config_File *f = new Config_File();
   iterator it;
   for( it = this->begin(); it != this->end(); it++)
      {
      if ((it->first).find(prefix) == 0)
         f->insert( (it->first).substr(prefix.size()+1), it->second);
      }

   return f;
}

void Config_File::dump()
{
   iterator it;
   for( it = this->begin(); it != this->end(); it++)
      printf("%s: %s\n", (it->first).c_str(), ((std::string)(it->second)).c_str());
}

string Config_File::getDir()
{
   string copy = Utils::deepCopy(getCfgFile());
   return string(dirname((char*)copy.c_str()));
}

