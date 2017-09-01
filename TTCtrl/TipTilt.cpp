#include "TipTilt.h"
#include "Logger.h"
using namespace Arcetri;

TipTilt::TipTilt(TTCtrl *ctrl, int direction) {

	_myCtrl = ctrl;
	_direction = direction;

	_valid = false;	// TipTilt default values are not loaded !

	Logger::get()->log(Logger::LOG_LEV_DEBUG, "TipTilt created");	
} 

void TipTilt::setupVars() {
    try {
        _freq = RTDBvar(_myCtrl->MyFullName(), "FREQ", _direction, REAL_VARIABLE, 1);
        _amp = RTDBvar(_myCtrl->MyFullName(),"AMP", _direction, REAL_VARIABLE, 1);
        _offset_x = RTDBvar(_myCtrl->MyFullName(),"OFFX", _direction, REAL_VARIABLE, 1);
        _offset_y = RTDBvar(_myCtrl->MyFullName(),"OFFY", _direction, REAL_VARIABLE, 1);
    } catch (AOVarException &e) { cout << e.what() << endl; }

	Logger::get()->log(Logger::LOG_LEV_DEBUG, "TipTilt vars setup completed");
} 

void TipTilt::initVars() throw (Config_File_Exception) {
	Config_File cfg = _myCtrl->ConfigDictionary();

	//Load req default values and save them to RTDB
	try { 
		setFreq(cfg["DEFAULT_FREQ"]); 
		setAmp(cfg["DEFAULT_AMP"]); 
		setOffset_x(cfg["DEFAULT_OFFSET_X"]); 
		setOffset_y(cfg["DEFAULT_OFFSET_Y"]); 
	} 
	catch(Config_File_Exception e) { 
		Logger::get()->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
		throw Config_File_Exception("Incomplete config file: please specify default TipTilt parameters !"); 
	}
	save();
	_valid = true;

	Logger::get()->log(Logger::LOG_LEV_DEBUG, "TipTilt vars initialization completed");
}

void TipTilt::notifyVars() {
	if(_direction == REQ_VAR) 
	{	
		_myCtrl->Notify(_freq, _myCtrl->TipTiltReqChanged);
		_myCtrl->Notify(_amp, _myCtrl->TipTiltReqChanged);
		_myCtrl->Notify(_offset_x, _myCtrl->TipTiltReqChanged);
		_myCtrl->Notify(_offset_y, _myCtrl->TipTiltReqChanged);
	}	

	Logger::get()->log(Logger::LOG_LEV_DEBUG, "TipTilt vars notify request completed");
}

void TipTilt::copy(TipTilt *another) {
	setFreq(another->Freq());
	setAmp(another->Amp());
	setOffset_x(another->Offset_x());
	setOffset_y(another->Offset_y());
}

double TipTilt::Freq() throw (AOVarException) {
	double f;
	_freq.Get(&f);
	return f; 
}

double TipTilt::Amp() throw (AOVarException) {
	double a;
	_amp.Get(&a);
	return a;
}

double TipTilt::Offset_x() throw (AOVarException) {
	double ox;
	_offset_x.Get(&ox);
	return ox;
}

double TipTilt::Offset_y() throw (AOVarException) {
	double oy;
	_offset_y.Get(&oy);
	return oy;
}

void TipTilt::setFreq(const double freq) throw (AOVarException) {
	_freq.Set(freq, 0, NO_SEND);
}

void TipTilt::setAmp(const double amp) throw (AOVarException) {
	_amp.Set(amp, 0, NO_SEND);
}

void TipTilt::setOffset_x(const double offset_x) throw (AOVarException) {
	_offset_x.Set(offset_x, 0, NO_SEND);
}

void TipTilt::setOffset_y(const double offset_y) throw (AOVarException) {
	_offset_y.Set(offset_y, 0, NO_SEND);
}

bool TipTilt::update(Variable *var) throw (AOVarException) {
   return _freq.MatchAndSet(var) ||
          _amp.MatchAndSet(var) ||
          _offset_x.MatchAndSet(var) ||
          _offset_y.MatchAndSet(var);
}

void TipTilt::save() throw (AOVarException) {
    _freq.Send();
    _amp.Send();
    _offset_x.Send();
    _offset_y.Send();
	Logger::get()->log(Logger::LOG_LEV_DEBUG, "Saving to RTDB of TipTilt done !");
}
