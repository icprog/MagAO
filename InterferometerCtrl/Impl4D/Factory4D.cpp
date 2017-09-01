
#include "Logger.h"

#include "Factory4D.h"

// Python objects interface
#include "abstract.h"

using namespace Arcetri;

// Static fields
PyObject* Factory4D::_remote4DClient = NULL;

Factory4D* Factory4D::instance() {
	// Instance the singleton only once
	if(_factory == NULL) {
		try {
			_factory = new Factory4D();
		}
		catch(Config_File_Exception& e) {
			return NULL;
		}
	}
	return (Factory4D*)(AbstractFactory::_factory);
}

Factory4D::Factory4D() throw (AOException, Config_File_Exception): AbstractFactory("conf/adsec/current/processConf/phasecam4020.conf")  {

	// The logger is installed by AbstractFactory
	_logger->log(Logger::LOG_LEV_DEBUG, "Initializing python library...");
	Py_Initialize();

	PyObject* clientModuleName = PyString_FromString("AdOpt.I4D.Client");
	_logger->log(Logger::LOG_LEV_TRACE, "PyString_FromString... done! (%s)", PyString_AsString(clientModuleName));

	PyObject* clientModule = PyImport_Import(clientModuleName);
	_logger->log(Logger::LOG_LEV_TRACE, "PyImport_Import... done! (%d)", (int)clientModule);

	PyObject* clientDict = PyModule_GetDict(clientModule);
	_logger->log(Logger::LOG_LEV_TRACE, "PyModule_GetDict... done!");

	PyObject* clientClass = PyDict_GetItemString(clientDict, "Client");
	_logger->log(Logger::LOG_LEV_TRACE, "PyDict_GetItemString... done!");

	// At the end get the wanted object reference :-)
	if (PyCallable_Check(clientClass)) {

		// Retrieve and test remote controller
		_logger->log(Logger::LOG_LEV_DEBUG, "Retrieving and testing remote controller instance...");
		_remote4DClient = PyObject_CallObject(clientClass, NULL);
		if(PyErr_Occurred()) {
			_logger->log(Logger::LOG_LEV_ERROR, "Can't communicate with 4D Server (down or not reachable)");
			throw AOException("Error in python library initialization!", INTERFEROMETER_PY_INIT_ERROR, __FILE__, __LINE__);
		}

		// Use it to initialize the controller
		_logger->log(Logger::LOG_LEV_DEBUG, "Initializing remote controller...");
		PyObject_CallMethod(_remote4DClient, "setConfigFile", "s", ((string)_config["4DConfigFile"]).c_str());

		_logger->log(Logger::LOG_LEV_DEBUG, "Python library succesfully initialized!");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Can't bind python class (I4D.Client)");
		throw AOException("Error in python library initialization!", INTERFEROMETER_GENERIC_ERROR, __FILE__, __LINE__);
	}

}

Factory4D::~Factory4D() {
	_logger->log(Logger::LOG_LEV_DEBUG, "Finalizing python library...");
	Py_Finalize();
	_logger->log(Logger::LOG_LEV_DEBUG, "Python library succesfully finalized!");
	_factory = NULL;
}

bool Factory4D::enableTrigger(bool enable) {
	_logger->log(Logger::LOG_LEV_INFO, "SETTING TRIGGER TO %d...", enable);
	PyObject_CallMethod(_remote4DClient, "enableTrigger", "i", enable);

	// Check python/pyro errors (usually in remote communication)
	if(PyErr_Occurred()) {
		_logger->log(Logger::LOG_LEV_ERROR, "Error occurred at Python/Pyro level");
		_logger->log(Logger::LOG_LEV_INFO, "SETTING TRIGGER  FAILED");
		return false;
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "TRIGGER SET!");
		return true;
	}
}

bool Factory4D::setPostProcessing(string ppType) {
	_logger->log(Logger::LOG_LEV_INFO, "SETTING POST-PROCESSING TO %s...", ppType.c_str());
	PyObject_CallMethod(_remote4DClient, "setPostProcessing", "s", ppType.c_str());

	// Check python/pyro errors (usually in remote communication)
	if(PyErr_Occurred()) {
		_logger->log(Logger::LOG_LEV_ERROR, "Error occurred at Python/Pyro level");
		_logger->log(Logger::LOG_LEV_INFO, "SETTING POST-PROCESSING FAILED");
		return false;
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "POST-PROCESSING SET!");
		return true;
	}

}

AbstractMeasurement* Factory4D::getMeasurement(string measName) throw (AOException) {

	// Do remote acquisition
	_logger->log(Logger::LOG_LEV_INFO, "ACQUIRING MEASUREMENT...");
	PyObject* destFile = NULL;
	destFile = PyObject_CallMethod(_remote4DClient, "getMeasurementToDisk", "s", measName.c_str());

	// Check python/pyro errors (usually in remote communication)
	if(PyErr_Occurred()) {
		_logger->log(Logger::LOG_LEV_ERROR, "Error occurred at Python/Pyro level");
		_logger->log(Logger::LOG_LEV_INFO, "MEASUREMENT ACQUISITION FAILED");
		throw AOException("Error in measurement acquisition", INTERFEROMETER_GENERIC_ERROR, __FILE__, __LINE__);
	}

	// Check acquisition result and return the Measurement (or NULL)
	if(destFile != NULL && PyString_Check(destFile)) {
		string destFileStr = string(PyString_AsString(destFile));
		_logger->log(Logger::LOG_LEV_INFO, "MEASUREMENT ACQUISITION DONE!");
		return new Measurement4D(destFileStr);
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "MEASUREMENT ACQUISITION FAILED!");
		throw AOException("Error in measurement acquisition", INTERFEROMETER_GENERIC_ERROR, __FILE__, __LINE__);
	}
}

AbstractMeasurementBurst* Factory4D::getMeasurementBurst(int measNum, string burstName) throw (AOException) {

	// Do remote acquisition
	_logger->log(Logger::LOG_LEV_INFO, "ACQUIRING BURST...");
	PyObject* destPath = NULL;
	if(burstName != "") {
		destPath = PyObject_CallMethod(_remote4DClient, "getBurstToDisk", "is", measNum, burstName.c_str());
	}
	else {
		destPath = PyObject_CallMethod(_remote4DClient, "getBurstToDisk", "i", measNum);
	}

	// Check python/pyro errors (usually in remote communication)
	if(PyErr_Occurred()) {
		_logger->log(Logger::LOG_LEV_ERROR, "Error occurred at Python/Pyro level");
		_logger->log(Logger::LOG_LEV_INFO, "BURST ACQUISITION FAILED");
		throw AOException("Error in burst acquisition", INTERFEROMETER_GENERIC_ERROR, __FILE__, __LINE__);
	}

	// Check acquisition result and return the MeasurementBurst (or NULL)
	if(destPath != NULL && PyString_Check(destPath)) {
		string destPathStr = string(PyString_AsString(destPath));
		_logger->log(Logger::LOG_LEV_INFO, "BURST ACQUISITION DONE!");
		return new MeasurementBurst4D(measNum, destPathStr);
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "BURST ACQUISITION FAILED!");
		throw AOException("Error in burst acquisition", INTERFEROMETER_GENERIC_ERROR, __FILE__, __LINE__);
	}
}

string Factory4D::postProcess(string remotePath, string measPrefix, int measNum) throw (AOException){
	_logger->log(Logger::LOG_LEV_INFO, "PROCESSING...");
	PyObject* dest = NULL;
	dest = PyObject_CallMethod(_remote4DClient, "postProcess", "ssi", remotePath.c_str(), measPrefix.c_str(), measNum);

	// Check python/pyro errors (usually in remote communication)
	if(PyErr_Occurred()) {
		_logger->log(Logger::LOG_LEV_ERROR, "Error occurred at Python/Pyro level");
		_logger->log(Logger::LOG_LEV_INFO, "PROCESSING FAILED");
		throw AOException("Error in post-processing", INTERFEROMETER_GENERIC_ERROR, __FILE__, __LINE__);
	}

	if(dest != NULL && PyString_Check(dest)) {
		if(string(PyString_AsString(dest)) != "") {
			_logger->log(Logger::LOG_LEV_INFO, "PROCESSING DONE!");
		}
		else {
			_logger->log(Logger::LOG_LEV_INFO, "PROCESSING NOT DONE (post-proc type is NONE)");
		}
		return string(PyString_AsString(dest));
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "PROCESSING FAILED");
		throw AOException("Error in post-processing", INTERFEROMETER_GENERIC_ERROR, __FILE__, __LINE__);
	}
}

//
//
string Factory4D::capture(int n, string name) throw (AOException) {

    _logger->log(Logger::LOG_LEV_INFO, "CAPTURING...");

    PyObject* destPath = NULL;
    if(name != "") {
        destPath = PyObject_CallMethod(_remote4DClient, "capture", "is", n, name.c_str());
	}
    else {
        destPath = PyObject_CallMethod(_remote4DClient, "capture", "i", n);
	}

    // Check python/pyro errors (usually in remote communication)
    if(PyErr_Occurred()) {
        _logger->log(Logger::LOG_LEV_ERROR, "Error occurred at Python/Pyro level");
        _logger->log(Logger::LOG_LEV_INFO, "CAPTURE ACQUISITION FAILED");
        throw AOException("Error in capture", INTERFEROMETER_GENERIC_ERROR, __FILE__, __LINE__);
    }

    if (destPath != NULL && PyString_Check(destPath)) {
        _logger->log(Logger::LOG_LEV_INFO, "CAPTURE ACQUISITION DONE (%s)", PyString_AsString(destPath));
        return string(PyString_AsString(destPath));
    }
    else {
        _logger->log(Logger::LOG_LEV_INFO, "CAPTURE ACQUISITION FAILED");
        return "";
    }
}
	                
//
//
string Factory4D::produce(string name) throw (AOException) {

    _logger->log(Logger::LOG_LEV_INFO, "PRODUCING...");

    PyObject* destPath = NULL;
    if(name != "") {
        destPath = PyObject_CallMethod(_remote4DClient, "produce", "s", name.c_str());
	}
    else {
        destPath = PyObject_CallMethod(_remote4DClient, "produce", "");
	}

    // Check python/pyro errors (usually in remote communication)
    if(PyErr_Occurred()) {
        _logger->log(Logger::LOG_LEV_ERROR, "Error occurred at Python/Pyro level");
        _logger->log(Logger::LOG_LEV_INFO, "PRODUCE ACQUISITION FAILED");
        throw AOException("Error in produce", INTERFEROMETER_GENERIC_ERROR, __FILE__, __LINE__);
    }
    
    if (destPath != NULL && PyString_Check(destPath)) {
        _logger->log(Logger::LOG_LEV_INFO, "PRODUCE ACQUISITION DONE (%s)", PyString_AsString(destPath));
        return string(PyString_AsString(destPath));
    }
    else {
        _logger->log(Logger::LOG_LEV_INFO, "PRODUCE ACQUISITION FAILED");
        return "";
    }
}



