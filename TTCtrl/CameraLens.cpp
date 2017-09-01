#include "CameraLens.h"
#include "Logger.h"
using namespace Arcetri;

extern "C" {
    #include "tt_matlab.h"
}

CameraLens::CameraLens(TTCtrl *ctrl, int direction) {

	_myCtrl = ctrl;
	_direction = direction;

	_valid = false;	// CameraLens default values are not loaded !

	Logger::get()->log(Logger::LOG_LEV_DEBUG, "CameraLens created");	
} 

void CameraLens::setupVars() {
    try {
        _pos_x = RTDBvar(_myCtrl->MyFullName(),"POSX", _direction, REAL_VARIABLE, 1);
        _pos_y = RTDBvar(_myCtrl->MyFullName(),"POSY", _direction, REAL_VARIABLE, 1);
    } catch (AOVarException &e) { cout << e.what() << endl; }

	Logger::get()->log(Logger::LOG_LEV_DEBUG, "CameraLens vars setup completed");
} 

void CameraLens::initVars() throw (Config_File_Exception) {
	Config_File cfg = _myCtrl->ConfigDictionary();

	//Load req default values and save them to RTDB
	try { 
		setPos_x(cfg["DEFAULT_POSX"]); 
		setPos_y(cfg["DEFAULT_POSY"]); 
	} 
	catch(Config_File_Exception e) { 
		Logger::get()->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
		throw Config_File_Exception("Incomplete config file: please specify default CameraLens parameters !"); 
	}
	save();
	_valid = true;

	Logger::get()->log(Logger::LOG_LEV_DEBUG, "CameraLens vars initialization completed");
}

void CameraLens::notifyVars() {
	if(_direction == REQ_VAR) 
	{	
		_myCtrl->Notify(_pos_x, _myCtrl->CameraLensReqChanged);
		_myCtrl->Notify(_pos_y, _myCtrl->CameraLensReqChanged);
	}	

	Logger::get()->log(Logger::LOG_LEV_DEBUG, "CameraLens vars notify request completed");
}

void CameraLens::copy(CameraLens *another) {
	setPos_x(another->Pos_x());
	setPos_y(another->Pos_y());
}

double CameraLens::Pos_x() throw (AOVarException) {
	double x;
	_pos_x.Get(&x);
	return x;
}

double CameraLens::Pos_y() throw (AOVarException) {
	double y;
	_pos_y.Get(&y);
	return y;
}

void CameraLens::setPos_x(const double pos_x) throw (AOVarException) {
	_pos_x.Set(pos_x, 0, NO_SEND);
}

void CameraLens::setPos_y(const double pos_y) throw (AOVarException) {
	_pos_y.Set(pos_y, 0, NO_SEND);
}

bool CameraLens::update(Variable *var) throw (AOVarException) {
   return _pos_x.MatchAndSet(var) ||
          _pos_y.MatchAndSet(var);
}

void CameraLens::save() throw (AOVarException) {
    _pos_x.Send();
    _pos_y.Send();
	Logger::get()->log(Logger::LOG_LEV_DEBUG, "Saving to RTDB of CameraLens done !");
}

void CameraLens::apply() {

	//Send to BCU
        matlab_setCL(_myCtrl->engine(), Pos_x(), Pos_y());
}
