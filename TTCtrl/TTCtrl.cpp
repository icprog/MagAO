#include "TTCtrl.h"
#include "Logger.h"
#include "Utils.h"
#include "AOStates.h"

#include <vector>

extern "C" {
   #include "tt_matlab.h"
}

using namespace Arcetri;


pthread_mutex_t	TTCtrl::_updatingMutex;

TTCtrl::TTCtrl(int argc, char **argv) throw (AOException) : AOApp(argc, argv) {

  _engine = NULL;
   CreateTT();
}

TTCtrl::~TTCtrl() {
	
    restTT();
    delete _tiptilt_cur;
    delete _tiptilt_req;
    delete _cameralens_cur;
    delete _cameralens_req;
    delete _waves_cur;
    delete _waves_req;
    _logger->log(Logger::LOG_LEV_INFO, "TTCtrl succesfully cleaned up");
}


void TTCtrl::CreateTT() throw (AOException) {
	
	pthread_mutex_init(&_updatingMutex, NULL);
	
	try {
		_actNum = ConfigDictionary()["ACT_NUM"];
	} 
	catch(Config_File_Exception& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "Using default actuators number (3)");
		_actNum = 3;
	}
	
	_ccd39CtrlFullName = (string)ConfigDictionary()["ccd39Process"]+"."+Side();
	_ccd39Ready = NULL;  // This could remain uninitialized if CCD39 ctrl is off !!!
	_ccd39Status = NULL; // This could remain uninitialized if CCD39 ctrl is off !!!
	_ccd39Freq = NULL; // This could remain uninitialized if CCD39 ctrl is off !!!
   _pingerPrefix = (string)ConfigDictionary()["pingerProcess"]+"."+Side();

	//Create TipTilt Request and Current
	_tiptilt_cur = new TipTilt(this, CUR_VAR);
	_tiptilt_req = new TipTilt(this, REQ_VAR);
	_cameralens_cur = new CameraLens(this, CUR_VAR);
	_cameralens_req = new CameraLens(this, REQ_VAR);
	
	//Create Waves Request and Current
	try {
		_waves_cur = new Waves(this, CUR_VAR);
		_waves_req = new Waves(this, REQ_VAR);
	} catch(WavesException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
		throw AOException("Error creating TTCtrl");
	}

	_logger->log(Logger::LOG_LEV_DEBUG, "TTCtrl for TipTilt Mirror with %d actuators created", _actNum);

}

//--- Virtual AOApp methods ---//

void TTCtrl::SetupVars() {

	_var_go = RTDBvar( MyFullName(), "GO", NO_DIR, INT_VARIABLE, 1);
	Notify(_var_go, GoChanged);
	
	_rotAngleReq = RTDBvar( MyFullName(), "AXYSROT", REQ_VAR, REAL_VARIABLE, 1);
	_rotAngleCur = RTDBvar( MyFullName(), "AXYSROT", CUR_VAR, REAL_VARIABLE, 1);
	Notify(_rotAngleReq, RotationAngleReqChanged);

	_setReq = RTDBvar( MyFullName(), "MIRROR_SET", REQ_VAR, INT_VARIABLE, 1);
	_setCur = RTDBvar( MyFullName(), "MIRROR_SET", CUR_VAR, INT_VARIABLE, 1);
	Notify(_setReq, SetReqChanged);

   // Wait for bcu39 pinger variable to exist, otherwise we can't attach to it.

   // The last "false" is "create=false" to avoid creating the variable when it doesn't exists
   _bcu39Status = RTDBvar( _pingerPrefix + ".bcu39", REAL_VARIABLE, 1, false );
   Notify(_bcu39Status, Bcu39StatusChanged);

   while(1) {
     int ret;
     _logger->log( Logger::LOG_LEV_INFO, "Waiting for ccd39 controller (%s) to become ready....", _ccd39CtrlFullName.c_str());
     if ((ret = thWaitReady( _ccd39CtrlFullName.c_str(), 60 * 1000)) == NO_ERROR)
        break;
     if (ret != TIMEOUT_ERROR)
     throw AOException("Error waiting for ccd39 to become ready", ret, __FILE__, __LINE__);
    }
     _logger->log( Logger::LOG_LEV_INFO, "ccd39 is ready");


	
	_tiptilt_cur->setupVars();
	_tiptilt_req->setupVars();
	
	_cameralens_cur->setupVars();
	_cameralens_req->setupVars();
	
	_waves_cur->setupVars();
	_waves_req->setupVars();
	
	attachJoeCtrl(); 

	_logger->log(Logger::LOG_LEV_DEBUG, "TTCtrl vars setup completed");
}

void TTCtrl::attachJoeCtrl() {

        return; /// Ignore ccd39 for now


          /////////////////////////////////

	
	try {
		// --- Check if CCD39 controller is registered and ready
		// RTDBvar can throw AOVarException if var doesn't exists

		// --- Ask to be notified for CCD39 status
		// This will never thorw AOVarException, if the previous step didn't throw it !
		if(_ccd39Status == NULL) {

        // Wait for ccd39 control process to become ready, otherwise there's no variable to attach!
        while(1) {
           int ret;
           _logger->log( Logger::LOG_LEV_INFO, "Waiting for ccd39 controller to become ready....", _ccd39CtrlFullName.c_str());
           if ((ret = thWaitReady( _ccd39CtrlFullName.c_str(), 60 * 1000)) == NO_ERROR)
                 break;
           if (ret != TIMEOUT_ERROR)
                 throw AOException("Error waiting for ccd39 to become ready", ret, __FILE__, __LINE__);
        }

			_ccd39Status = new RTDBvar(_ccd39CtrlFullName + ".STATUS");	// Also get the value
			Notify(*_ccd39Status, Ccd39StatusChanged);
		}


		if(_ccd39Ready == NULL) {
			_ccd39Ready = new RTDBvar(_ccd39CtrlFullName + ".CLSTAT" );
                        printf("ccd39 Ready var: %s\n", (_ccd39CtrlFullName + ".CLSTAT").c_str());
			Notify(*_ccd39Ready, Ccd39ReadyChanged);
		}

		if(_ccd39Freq == NULL) {
			_ccd39Freq = new RTDBvar(_ccd39CtrlFullName + ".FRMRT.CUR" );
                        printf("ccd39 Freq var: %s\n", (_ccd39CtrlFullName + ".FRMRT.CUR").c_str());
			Notify(*_ccd39Freq, Ccd39FreqChanged);
		}

		
		if(isCcd39CtrlReady() && isCcd39Operating()) {
			_logger->log(Logger::LOG_LEV_INFO, "CCD39 CONTROLLER IS READY AND OPERATING: can attach to CCD frequency");
		}
		else if(!isCcd39CtrlReady()) {
			_logger->log(Logger::LOG_LEV_WARNING, "CCD39 CONTROLLER ISN'T READY: impossible to attach to CCD frequency");
		}
		else if(!isCcd39Operating()) {
			_logger->log(Logger::LOG_LEV_WARNING, "CCD39 CONTROLLER IS READY BUT ISN'T OPERATING: impossible to attach to CCD frequency");
		}
	}
	catch (AOVarException &e) {
		_logger->log(Logger::LOG_LEV_WARNING, "CCD39 CONTROLLER NOT READY: impossible to attach to CCD frequency");
	}
}

void TTCtrl::PostInit() {
	// Initialize the parameters converter
	_converter = new Converter(this, ConfigDictionary());
	_rotAngleCur.Set(_converter->getRotationAngle());

   // Start with no connection and wait for notification from BCU39 status
   setCurState( STATE_NOCONNECTION);


	// Initialize default values for TipTilt Req
	// ant ask to be notified on change
	_tiptilt_req->initVars();
	_tiptilt_req->notifyVars();
	_tiptilt_req->setValid(false); 	// Avoid the default settings are applied: want to apply
								 	// the default low-level settings !!!

	_cameralens_req->initVars();
	_cameralens_req->notifyVars();
	_cameralens_req->setValid(false); 	// Avoid the default settings are applied: want to apply
	
	// Initialize default values for TipTilt Req
	// ant ask to be notified on change
	_waves_req->initVars();
	_waves_req->notifyVars();
	
}

bool TTCtrl::isCcd39CtrlReady() {
	int ccd39Ready = STATE_UNDEFINED;
	if(_ccd39Ready != NULL) {
		_ccd39Ready->Get(&ccd39Ready);
	}
	return (ccd39Ready == 2);
}

bool TTCtrl::isCcd39Operating() {
	int ccd39Status = STATE_UNDEFINED;
	if(_ccd39Status != NULL) {
		_ccd39Status->Get(&ccd39Status);
	}
	return (ccd39Status == STATE_OPERATING);
}

bool TTCtrl::isBcu39Operating() {
	int bcu39Status;

   _bcu39Status.Get(&bcu39Status);
	return (bcu39Status == STATE_OPERATING);
}


//--- Start/Stop methods ---//

void TTCtrl::start() throw (AOVarException) {
	// Compute requested Waves form TipTilt if it is valid
	if(_tiptilt_req->Valid()) {
		_logger->log(Logger::LOG_LEV_DEBUG, "TipTilt Req IS VALID !!!");
		// Check TipTilt parameters and fix them if needed
		_converter->checkAndFix(_tiptilt_req);
		// Waves computed from tiptilt will be ok: they won't need checkAndFix !
		_converter->convert(_tiptilt_req, _waves_req);
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "TipTilt Req ISN'T VALID !!!");
		// Check Waves parameters and fix them if needed
		_converter->checkAndFix(_waves_req);	
	}

	// Apply requested settings
	try {
		setTT();
                setCameraLens();
      setCurState( STATE_OPERATING);
		_logger->log(Logger::LOG_LEV_INFO, "TipTilt succesfully started/restarted");
	}
	catch(WavesException& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "%s", e.what().c_str());
		_logger->log(Logger::LOG_LEV_ERROR, "TipTilt start/restart failed");
	}
}

void TTCtrl::stop() throw (AOVarException) {
	
	// If the TipTilt was valid, it remains valid (that is, corresponding to
	// Waves) by setting its amplitude to 0
	if(_tiptilt_req->Valid()) {
		_tiptilt_req->setAmp(0);
	}

	// Send Waves with Amp = 0
	double amps[3];
	for(int i=0; i<3; i++) {
		amps[i] = 0;
	}
	_waves_req->setAmps(amps);
	_waves_req->apply();
	_waves_cur->copy(_waves_req);
	_waves_cur->save();
	
	try {
		setTT();
      setCurState( STATE_READY);
		_logger->log(Logger::LOG_LEV_INFO, "TipTilt succesfully stopped");
	}
	catch(WavesException& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "%s", e.what().c_str());
		_logger->log(Logger::LOG_LEV_ERROR, "TipTilt stop failed");
	}
	
}

void TTCtrl::updateRotationAngle(double rotAngle) {
	_converter->setRotationAngle(rotAngle);
	_rotAngleCur.Set(rotAngle);
	_logger->log(Logger::LOG_LEV_INFO, "TipTilt axys rotation angle succesfully updated to %.5f", rotAngle);
}

void TTCtrl::updateCcd39Ready(Variable* var) {
        printf("Updating ccd39 ready var\n");
	_ccd39Ready->Set(var);
}

void TTCtrl::updateCcd39Status(Variable* var) {
        printf("Updating ccd39 status var\n");
	_ccd39Status->Set(var);
}

void TTCtrl::updateCcd39Freq(Variable* var) {
        printf("Updating ccd39 freq var\n");
	_ccd39Freq->Set(var);
}

void TTCtrl::updateBcu39Status(Variable *var) {
   _bcu39Status.Set(var);
}

void TTCtrl::setTT() throw(AOVarException, WavesException) {
	
	// Try to attach JoeCtrl to enable CCD39 freq synch.
	// Do nothing if already attached.
	attachJoeCtrl();
	
	_waves_req->apply();	// Can throw WavesException if fails
		
	// Update current Waves in RTDB 
	_waves_cur->copy(_waves_req);
	_waves_cur->save();   

	// Update current TipTilt in RTDB 
	if(_tiptilt_req->Valid()) {
		_tiptilt_cur->copy(_tiptilt_req);
		_tiptilt_cur->save();
	}
	else {
		// [todo] Should update TipTiltCur on RTDB to mark it as INVALID
		_logger->log(Logger::LOG_LEV_WARNING, "TipTilt Cur not updated on RTDB");
	}
}

void TTCtrl::setCameraLens() {
    _cameralens_req->apply();
    _cameralens_cur->copy( _cameralens_req);
    _cameralens_cur->save();
}

void TTCtrl::initTT() {

    _setCur.Set(-1);
    _engine = matlab_init();
    _setCur.Set(1);

    // Send Waves with Amp = 0
    double amps[3];
    for(int i=0; i<3; i++) {
        amps[i] = 0;
    }
    _waves_req->setAmps(amps);
    _waves_req->apply();
    _waves_cur->copy(_waves_req);
    _waves_cur->save();
}

void TTCtrl::restTT() {
    matlab_setMod(_engine, 10, 0); // stop modulation
    matlab_rip(_engine);
    _setCur.Set(0);
}

// --- Handlers --- //

int TTCtrl::SetReqChanged(void *pt, Variable *var) {

	pthread_mutex_lock(&_updatingMutex);
	TTCtrl* TTC = (TTCtrl*)pt;
	int setRequest = var->Value.Lv[0];
	_logger->log(Logger::LOG_LEV_INFO, "Set variable changed, value = %d", setRequest);
        if (setRequest)
            TTC->initTT();
        else
            TTC->restTT();

 	pthread_mutex_unlock(&_updatingMutex);
        return NO_ERROR;
}
         
//Start or stop the TipTilt
int TTCtrl::GoChanged(void *pt, Variable *var) {

	pthread_mutex_lock(&_updatingMutex);

	TTCtrl* TTC = (TTCtrl*)pt;

	_logger->log(Logger::LOG_LEV_INFO, "Go variable changed !!!");

	int goRequest = (var->Value.Lv[0] != 0) ? 1 : 0;
        TTC->_var_go.Set(goRequest);		// Save value for later use

	//--- Starting or Restarting (already running) ---// 
	// Note: it doesn't matter if the TipTilt is already running or not:
	// in both cases parameters are set and command is sent to BCU
	try {
    	if(goRequest) {
			TTC->start();
		}
		else {
			TTC->stop();
		}
	}
	catch(AOVarException& e) {
 		_logger->log(Logger::LOG_LEV_FATAL, "TTCtrl.GoChanged(): %s", e.what().c_str());
 		pthread_mutex_unlock(&_updatingMutex);
 		return VAR_WRITE_ERROR;
 	}
 	pthread_mutex_unlock(&_updatingMutex);
	return NO_ERROR;
}

int TTCtrl::RotationAngleReqChanged(void *pt, Variable *var) {
	_logger->log(Logger::LOG_LEV_INFO, "RotationAngle variable changed !!!");
	TTCtrl *TTC = (TTCtrl*)pt;
	try {
		TTC->updateRotationAngle(var->Value.Dv[0]);
	}
	catch(AOVarException& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "TipTilt axys rotation angle not changed.");
		return VAR_WRITE_ERROR;
	}
	return NO_ERROR;
}

int TTCtrl::Ccd39StatusChanged(void *pt, Variable *var) {
	_logger->log(Logger::LOG_LEV_INFO, "Ccd39 status variable changed to value %d !!!", var->Value.Lv[0]);
	TTCtrl *TTC = (TTCtrl*)pt;
	try {
		TTC->updateCcd39Status(var);
	}
	catch(AOVarException& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "Local Ccd39 status updating failed");
		return VAR_WRITE_ERROR;
	}
	return NO_ERROR;
}

int TTCtrl::Ccd39ReadyChanged(void *pt, Variable *var) {
	_logger->log(Logger::LOG_LEV_INFO, "Ccd39 ready variable changed to value %d !!!", var->Value.Lv[0]);
	TTCtrl *TTC = (TTCtrl*)pt;
	try {
		TTC->updateCcd39Ready(var);
	}
	catch(AOVarException& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "Local Ccd39 ready updating failed");
		return VAR_WRITE_ERROR;
	}
	return NO_ERROR;
}

int TTCtrl::Ccd39FreqChanged(void *pt, Variable *var) {
	_logger->log(Logger::LOG_LEV_INFO, "Ccd39 framerate variable changed to value %5.2f !!!", var->Value.Dv[0]);
	TTCtrl *TTC = (TTCtrl*)pt;
	try {
		TTC->updateCcd39Freq(var);
	}
	catch(AOVarException& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "Local Ccd39 framerate updating failed");
		return VAR_WRITE_ERROR;
	}
	return NO_ERROR;
}

int TTCtrl::Bcu39StatusChanged(void *pt, Variable *var) {

    static int counter=0;
    TTCtrl *TTC = (TTCtrl*)pt;
    try {
       TTC->updateBcu39Status(var);

       float value = var->Value.Dv[0];
       if ((value == -1.0) && (TTC->getCurState() != STATE_NOCONNECTION))
       {
          // Allow up to 3 failed pings before giving up
          counter++;
          if (counter<3) return NO_ERROR;
          counter=0;

 		   _logger->log(Logger::LOG_LEV_INFO, "BCU39 unreachable! TT fate unknown");
         TTC->setCurState( STATE_NOCONNECTION);
       }

       if ((value > 0) && (TTC->getCurState() == STATE_NOCONNECTION)) {
	      pthread_mutex_lock(& TTC->_updatingMutex);
 	   _logger->log(Logger::LOG_LEV_INFO, "BCU39 is online. Turn on TT mirror");

         // Turn on TT mirror
        TTC->initTT();

	   _logger->log(Logger::LOG_LEV_INFO, "Setting last TT parameters");

         int goRequest;
         TTC->_var_go.Get(&goRequest);

	      try {
    	      if(goRequest)
			      TTC->start();
		      else
			      TTC->stop();
	      }
	      catch(AOVarException& e) {
 		      _logger->log(Logger::LOG_LEV_FATAL, "TTCtrl.GoChanged(): %s", e.what().c_str());
 		      pthread_mutex_unlock(&_updatingMutex);
 		      return VAR_WRITE_ERROR;
 	      }
 	   pthread_mutex_unlock(&_updatingMutex);
      }
    }
    catch(AOVarException& e) {
      _logger->log(Logger::LOG_LEV_WARNING, "Local bcu39 status updating failed");
      return VAR_WRITE_ERROR;
   }
   return NO_ERROR;
}


// Updates Requested TipTilt when var change is notified
int TTCtrl::TipTiltReqChanged(void *pt, Variable *var) {
	
	_logger->log(Logger::LOG_LEV_INFO, "TipTilt variable changed !!!");
	pthread_mutex_lock(&_updatingMutex);
	
	TTCtrl *TTC = (TTCtrl*)pt;
	
	try {
		if(TTC->TipTiltReq()->update(var)) {
			TTC->TipTiltReq()->setValid(true);
		}
	}
	catch(AOVarException& e) {
		_logger->log(Logger::LOG_LEV_FATAL, "TipTilt.update(): %s", e.what().c_str());
		pthread_mutex_unlock(&_updatingMutex);
 		return VAR_WRITE_ERROR;
	}

	pthread_mutex_unlock(&_updatingMutex);
	return NO_ERROR;
}

// Updates Requested Waves when var change is notified
int TTCtrl::WavesReqChanged(void *pt, Variable *var) {

	_logger->log(Logger::LOG_LEV_INFO, "Waves variable changed !!!");
	pthread_mutex_lock(&_updatingMutex);
	
	TTCtrl* TTC = (TTCtrl*)pt;
	
	try {
		if(TTC->WavesReq()->update(var)) {
			// TipTilt can't be updated from a change to Waves,
			// so it is now invalid
			TTC->TipTiltReq()->setValid(false);
		}
	}
	catch(AOVarException& e) {
		_logger->log(Logger::LOG_LEV_FATAL, "Waves.update(): %s", e.what().c_str());
		pthread_mutex_unlock(&_updatingMutex);
 		return VAR_WRITE_ERROR;
	}
	pthread_mutex_unlock(&_updatingMutex);
	return NO_ERROR;
}

// Updates Requested CameraLens when var change is notified
int TTCtrl::CameraLensReqChanged(void *pt, Variable *var) {
	
	_logger->log(Logger::LOG_LEV_INFO, "CameraLens variable changed !!!");
	pthread_mutex_lock(&_updatingMutex);
	
	TTCtrl *TTC = (TTCtrl*)pt;
	
	try {
		if(TTC->CameraLensReq()->update(var)) {
			TTC->CameraLensReq()->setValid(true);
		}
	}
	catch(AOVarException& e) {
		_logger->log(Logger::LOG_LEV_FATAL, "CameraLensReq.update(): %s", e.what().c_str());
		pthread_mutex_unlock(&_updatingMutex);
 		return VAR_WRITE_ERROR;
	}

	pthread_mutex_unlock(&_updatingMutex);
	return NO_ERROR;
}

double TTCtrl::checkAmp( double freq, double amp) {

   double maxProduct =170* 1e-3;
   if (freq*amp > maxProduct) {
      printf("checkAmp(): amplitude reduced from %g to %g rad\n", amp, maxProduct/freq);
      amp = maxProduct / freq;
   }

   return amp;

   /////////////////

/*
   std::vector<double> limits_f;
   std::vector<double> limits_a;

   count = limits_f.size();

   if (count<1)
       return amp;
   if (freq <= limits_f[0])
       return limits_a[0];

   int i=0;
   while ((freq>limits_f[i]) && (i<count))
       i++;
   if (i=> count)
       return 0;

   double x1 = limits_f[i-1];
   double x2 = limits_f[i];
   double y1 = limits_a[i-1];
   double y2 = limits_a[i];
   double dx = x2-x1;
   double dy = y2-y1;
   double maxamp = dy / ((freq-x1)/dx);
   
   if (amp>maxamp)
       return maxamp;
   else
       return amp; 
*/
}



//*** Testing main ***//
int main( int argc, char **argv) {
	
	TTCtrl* TTC = NULL;
	try {
      TTC = new TTCtrl(argc, argv);
    	TTC->Exec();
    	
	} 
  	catch (LoggerFatalException &e) {
		// In this case the logger can't log!!!
      	printf("%s\n", e.what().c_str());
   	} 
   	catch (AOException &e) {
      	Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
   	} 
   	
   	// Clear the TTCtrl object (release the shm attached to Waves->_comSender)
    delete TTC;
    return 0;
}
