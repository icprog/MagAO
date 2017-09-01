#include "AbstractFactory.h"

using namespace Arcetri;

// Static fields
Config_File 	 AbstractFactory::_config;
Logger* 	     AbstractFactory::_logger = NULL;
AbstractFactory* AbstractFactory::_factory = NULL;

AbstractFactory::AbstractFactory(string configFile) throw (Config_File_Exception) { 
	
	_factory = NULL;
	
	try {
		_config = Config_File(configFile);
		
		string intfName = _config["InterferometerName"]; 
		int	   logLevel = Logger::stringToLevel(_config["LogLevel"]);
		
		_logger = Logger::get(intfName, logLevel);
		_logger->printStatus();
	
		_logger->log(Logger::LOG_LEV_DEBUG, "Factory for interferometer '%s' created", intfName.c_str());
	}
	catch(Config_File_Exception& e) {
		Logger::get("GENERIC-INTERFEROMETER")->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
		throw Config_File_Exception("Unexisting or wrong configuration file");
	}
}				

AbstractFactory::~AbstractFactory() {
	_logger->log(Logger::LOG_LEV_DEBUG, "Factory for interferometer '%s' destroyed", ((string)_config["InterferometerName"]).c_str());	
}
