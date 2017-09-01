//@File: WfsArbitrator.cpp
//
// Implementation of the WfsArbitrator class
//@

#include "wfsarbitrator/WfsArbitrator.h"
#include "wfsarbitrator/WfsFsm.h"
#include "wfsarbitrator/WfsCommandImplFactory.h"
#include "wfsarbitrator/WfsSystem.h"
#include "wfsarbitrator/AdSecSystem.h"
#include "wfsarbitrator/MastDiagnSystem.h"
#include "wfsarbitrator/OptLoopDiagSystem.h"
#include "arblib/base/Alerts.h"
#include "Paths.h"
#include "AOStates.h"

using namespace Arcetri::Arbitrator;
using namespace Arcetri::Wfs_Arbitrator;


WfsArbitrator::WfsArbitrator(int argc, char* argv[]):AbstractArbitrator(argc, argv) {
	create();
}

void WfsArbitrator::create() {

    RequireKeyword("MinLoopFreq");
    RequireKeyword("MaxLoopFreq");
    RequireKeyword("MaxOvsFreq");
    RequireKeyword("MinHODark");
    RequireKeyword("MaxHODark");
    RequireKeyword("MinTVDark");
    RequireKeyword("MaxTVDark");
    RequireKeyword("MinIRTCDark");
    RequireKeyword("MaxIRTCDark");
    RequireKeyword("MinSlopenull");
    RequireKeyword("MaxSlopenull");
    RequireKeyword("MaxSlopenull");
    RequireKeyword("TelElevationVar");
    RequireKeyword("TelRotatorVar");
    RequireKeyword("RerotVar");
    RequireKeyword("Adc1Var");
    RequireKeyword("Adc2Var");
    RequireKeyword("WfsSpec");
    RequireKeyword("CLtrackingMaxX");
    RequireKeyword("CLtrackingMaxY");
    RequireKeyword("CLtrackingMinX");
    RequireKeyword("CLtrackingMinY");
    RequireKeyword("RotOffsetMax");
    CheckKeywords(true, true, true);

    // Set WFS spec. Can be AO_WFS_FLAO, LBTI, etc.
    // It will be propagated to the WFS interface
    // and checked against incoming prepareAcquireRef commands

    _wfsSpec = (std::string)ConfigDictionary()["WfsSpec"];

	// Set logger levels for specific components: the levels are got
	// from config file
	setLoggerLevel("WFS");
	setLoggerLevel("MASTDIAGN");
	setLoggerLevel("ASYNCHCOMMANDHANDLER");
	setLoggerLevel("ADSEC");
	setLoggerLevel("OPTLOOPDIAG");

	// --- Intialize the AbstractArbitrator with specific modules ---//

	// Create a CommandImplFactory, able to implement AO commands
	_cmdImplFactory = new WfsCommandImplFactory();

	// Create the AOFsm (with the given CommandImplFactory), able
	// to process AO commands
	_cmdHandler = new WfsFsm(_commandHistorySize, MyName());

   // Set starting state
   try {
      ((WfsFsm *)_cmdHandler)->forceState( ((WfsFsm *)_cmdHandler)->stateCode( (std::string)ConfigDictionary()["INITIAL_STATE"]));
	   _logger->log(Logger::LOG_LEV_INFO, "WfsArbitrator starting from state %s (from configuration file)", ((WfsFsm *)_cmdHandler)->currentStateAsString().c_str());
   }
   catch (AOException &e) {
	   _logger->log(Logger::LOG_LEV_INFO, "WfsArbitrator starting from default state %s", ((WfsFsm *)_cmdHandler)->currentStateAsString().c_str());
   }


   _rerotTracking=false;
   _adcTracking=false;
   _lensTracking=false;
   _lensTrackingTarget=false;
   _lensTrackingOutOfRange = false;
   _lensTrackingAlert = false;
   _lensTrackingTH = 2.4;  // default to 0.1 pixels

   _rotatorChangeOffset[0]=0.0;
   _rotatorChangeOffset[1]=0.0;
   _rotatorChangeOffset[2]=0.0;
   _rotatorChangeOffset[3]=0.0;
   _rotatorChangeOffset[4]=0.0;

   _adcOffset1 =0.0;
   _adcOffset2 =0.0;


    for (int i=1; i<=5; i++) {
       try {
          ostringstream oss;
          oss << "RotatorOffsetBin" << i;
          _rotatorOffset[i] = (double)ConfigDictionary()[oss.str()];
      }
      catch (AOException &e) {
          _logger->log(Logger::LOG_LEV_INFO, "RotatorOffset keyword not defined in config file. Using default offset=0");
          _rotatorOffset[i]=0;
      }
    }
    try {
       _rotatorSign = (double)ConfigDictionary()["RotatorSign"];
   }
   catch (AOException &e) {
       _logger->log(Logger::LOG_LEV_INFO, "RotatorSign keyword not defined in config file. Using default sign=-1");
       _rotatorSign=-1;
   }

    try {
       _maxMasterSpeed = (double)ConfigDictionary()["MaxMasterSpeed"];
   }
   catch (AOException &e) {
       _logger->log(Logger::LOG_LEV_WARNING, "MaxMasterSpeed keyword not defined in config file. Using default speed=400Hz");
       _maxMasterSpeed=400;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set master diagnostic speed: %5.1f Hz", _maxMasterSpeed);

    try {
       _focalPlaneRotation = (double)ConfigDictionary()["focalPlaneRotation"];
   }
   catch (AOException &e) {
       _logger->log(Logger::LOG_LEV_WARNING, "focalPlaneRotation keyword not defined in config file. Using default value of 0 (no rotation)");
       _focalPlaneRotation=0;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set coordinate system rotation: %5.1f Hz", _focalPlaneRotation);

   _adcLutValid=false;
   string filename = Paths::WfsCalibDir() + "/adc/lut.txt";
   //adcTest(filename);
   try {
     readAdcLut(filename, _adc1, _adc2);
     _adcLutValid=true;
     _logger->log(Logger::LOG_LEV_INFO, "Read ADC lookup table, %d elements", _adc1.size());
   } catch (...) {
      _logger->log(Logger::LOG_LEV_WARNING, "Cannot read ADC lut file: %s", filename.c_str());
      _logger->log(Logger::LOG_LEV_WARNING, "ADC tracking will not work");
   }


   try {
      _adc1Limits = (int)ConfigDictionary()["Adc1Limits"];
      _adc1Min = (int)ConfigDictionary()["Adc1Min"];
      _adc1Max = (int)ConfigDictionary()["Adc1Max"];
   } catch (...){
      _adc1Limits = 0;
      _adc1Min = -10000;
      _adc1Max =  10000;
   }


   try {
      _voiceHost = (std::string) ConfigDictionary()["Voice_Host"];
      _voicePort = (int) ConfigDictionary()["Voice_Port"];
   } catch (...) {
      //If we don't have these, then we can't talk
      _voiceHost = "";
      _voicePort = -1;
   }

   try {
      _voiceName = (std::string) ConfigDictionary()["Voice_Name"];
   } catch (...) {
      _voiceName = "none";
   }

   try {
      _voiceCameraLensWarning = (std::string) ConfigDictionary()["Voice_CameraLensWarning"];
   } catch (...) {
      _voiceCameraLensWarning = "none";
   }

   try {
      _voiceADCTrackingEnabled = (std::string) ConfigDictionary()["Voice_ADCTrackingEnabled"];
   } catch (...) {
      _voiceADCTrackingEnabled = "none";
   }

   try {
      _voiceADCZeroTrackEnabled = (std::string) ConfigDictionary()["Voice_ADCZeroTrackEnabled"];
   } catch (...) {
      _voiceADCZeroTrackEnabled = "none";
   }

   try {
      _voiceADCMinimumWarning  = (std::string) ConfigDictionary()["Voice_ADCMinimumWarning"];
   } catch (...) {
      _voiceADCMinimumWarning = "none";
   }

   try {
      _voiceADCMaximumWarning  = (std::string) ConfigDictionary()["Voice_ADCMaximumWarning"];
   } catch (...) {
      _voiceADCMaximumWarning = "none";
   }

}


WfsArbitrator::~WfsArbitrator() {

}

string WfsArbitrator::getCalibUnitState() {
    WfsSystem *wfs = (WfsSystem*)(_systems["WFS"]);
    return wfs->getSourceName();
}

void WfsArbitrator::serviceRequest(Request* req, MsgBuf* /* requestMsgBuf */) {

   switch(req->getCode()) {
      case WfsOpCodes::REQUEST_WFS_STATUS_CMD:

         RequestWfsStatus* statusReq = (RequestWfsStatus*)req;
         statusReq->setArbitratorStatus(this->fsmState());
         Command* lastCmd = this->lastCommand();
         if(lastCmd != NULL) {
            statusReq->setLastCommandDescription(lastCmd->getDescription());
            statusReq->setLastCommandResult(lastCmd->getStatusAsString());
         }

         statusReq->setClStatReady( this->clStatOK());

         statusReq->setDone();
         statusReq->setSourceName( this->getCalibUnitState());
         statusReq->setRerotTracking( this->_rerotTracking);
         statusReq->setAdcTracking( this->_adcTracking);
         statusReq->setLensTracking( this->_lensTracking);
         statusReq->setLensTrackingTarget( this->_lensTrackingTarget);
         statusReq->setLensTrackingOutOfRange( this->_lensTrackingOutOfRange);
         statusReq->setLensTrackingAlert( this->_lensTrackingAlert);

         break;

   }

}


inline string WfsArbitrator::fsmState() {
      return ((WfsFsm *)_cmdHandler)->currentStateAsString();
}



void WfsArbitrator::SetupVars() {
	AbstractArbitrator::SetupVars();

	_logger->log(Logger::LOG_LEV_DEBUG, "WfsArbitrator setting up RTDB vars...");
	try {

      _ccdFreqLimits = RTDBvar( MyFullName()+".CCDFREQ_LIMITS", REAL_VARIABLE, 2);
      _modAmpl       = RTDBvar( MyFullName()+".MOD_AMPL", REAL_VARIABLE);
      _msg           = RTDBvar( MyFullName()+".MSG", CHAR_VARIABLE, 50);
      _pyramidPos    = RTDBvar( MyFullName()+".PYRAMID_POS", INT_VARIABLE, 2);
      _pupils        = RTDBvar( MyFullName()+".PUPILS", CHAR_VARIABLE, 16);

      _offsetBin1Cur = RTDBvar( MyFullName(), "OFFSETBIN1", CUR_VAR, REAL_VARIABLE, 1);
      _offsetBin1Req = RTDBvar( MyFullName(), "OFFSETBIN1", REQ_VAR, REAL_VARIABLE, 1);
      _offsetBin2Cur = RTDBvar( MyFullName(), "OFFSETBIN2", CUR_VAR, REAL_VARIABLE, 1);
      _offsetBin2Req = RTDBvar( MyFullName(), "OFFSETBIN2", REQ_VAR, REAL_VARIABLE, 1);
      _offsetBin3Cur = RTDBvar( MyFullName(), "OFFSETBIN3", CUR_VAR, REAL_VARIABLE, 1);
      _offsetBin3Req = RTDBvar( MyFullName(), "OFFSETBIN3", REQ_VAR, REAL_VARIABLE, 1);
      _offsetBin4Cur = RTDBvar( MyFullName(), "OFFSETBIN4", CUR_VAR, REAL_VARIABLE, 1);
      _offsetBin4Req = RTDBvar( MyFullName(), "OFFSETBIN4", REQ_VAR, REAL_VARIABLE, 1);
      _offsetBin5Cur = RTDBvar( MyFullName(), "OFFSETBIN5", CUR_VAR, REAL_VARIABLE, 1);
      _offsetBin5Req = RTDBvar( MyFullName(), "OFFSETBIN5", REQ_VAR, REAL_VARIABLE, 1);

      _offsetAdc1Cur = RTDBvar( MyFullName(), "ADC1.OFFSET", CUR_VAR, REAL_VARIABLE, 1);
      _offsetAdc1Req = RTDBvar( MyFullName(), "ADC1.OFFSET", REQ_VAR, REAL_VARIABLE, 1);
      _offsetAdc2Cur = RTDBvar( MyFullName(), "ADC2.OFFSET", CUR_VAR, REAL_VARIABLE, 1);
      _offsetAdc2Req = RTDBvar( MyFullName(), "ADC2.OFFSET", REQ_VAR, REAL_VARIABLE, 1);

      _adcZeroTrack = RTDBvar( MyFullName(), "ADC.ZEROTRACK", NO_DIR, INT_VARIABLE, 1);

      _led       = RTDBvar( MyFullName()+".LED", INT_VARIABLE, 1);
      _led.Set(0);
      _bcu39sx   = RTDBvar( "pinger."+Side()+".bcu39", REAL_VARIABLE, 1);
      _flEnabled = RTDBvar( "slopecompctrl."+Side()+".FASTLINK.ENABLE.CUR", INT_VARIABLE, 1);


      // Set _led and current state according to current conditions
      sync();




      _clRefX   = RTDBvar( MyFullName(), "CL_REF_X", NO_DIR, REAL_VARIABLE, 1);
      _clRefY   = RTDBvar( MyFullName(), "CL_REF_Y", NO_DIR, REAL_VARIABLE, 1);
      _clRefRot = RTDBvar( MyFullName(), "CL_REF_ROT", NO_DIR, REAL_VARIABLE, 1);

      _clXErr = RTDBvar( MyFullName(), "CL_ERR_X", NO_DIR, REAL_VARIABLE, 1);
      _clYErr = RTDBvar( MyFullName(), "CL_ERR_Y", NO_DIR, REAL_VARIABLE, 1);
      _clErrCorrecting = RTDBvar( MyFullName(), "CL_ERR_CORRECTING", NO_DIR, INT_VARIABLE, 1);
      _clTrackReq = RTDBvar( MyFullName(), "CL_TRACK", REQ_VAR, INT_VARIABLE, 1);


      // Cameralens tracking
      _ccd39Pup0 = RTDBvar( "pupilcheck39."+Side()+".PUP0", REAL_VARIABLE, 6);
      _ccd39Pup1 = RTDBvar( "pupilcheck39."+Side()+".PUP1", REAL_VARIABLE, 6);
      _ccd39Pup2 = RTDBvar( "pupilcheck39."+Side()+".PUP2", REAL_VARIABLE, 6);
      _ccd39Pup3 = RTDBvar( "pupilcheck39."+Side()+".PUP3", REAL_VARIABLE, 6);
      _clXcur    = RTDBvar( "ttctrl."+Side()+".POSX.CUR", REAL_VARIABLE, 1);
      _clXreq    = RTDBvar( "ttctrl."+Side()+".POSX.REQ", REAL_VARIABLE, 1);
      _clYcur    = RTDBvar( "ttctrl."+Side()+".POSY.CUR", REAL_VARIABLE, 1);
      _clYreq    = RTDBvar( "ttctrl."+Side()+".POSY.REQ", REAL_VARIABLE, 1);
      _clGo     = RTDBvar( "ttctrl."+Side()+".CLGO", INT_VARIABLE, 1);


      // Pupil tracking
      _slopeCompPupils = RTDBvar( "slopecompctrl."+AOApp::Side()+".PIXELLUT.CUR", CHAR_VARIABLE, 200);

	}
	catch (AOVarException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, "SetupVars(): %s", e.what().c_str());
		throw ArbitratorException("Error while setting up RTDB vars", ARB_INIT_ERROR);
	}

    // These variable groups have their own try/catch handlers because they usually refer to
    // a different msgd, so they may not exist or be unreachable.
    // The WFS unit can work standalone without the functionality from these variables,
    // so errors here are not fatal.

    _telElOk = false;
    try {
        // Rerotator & ADC Tracking
        _telElevation = RTDBvar( ReplaceSide( ConfigDictionary()["TelElevationVar"]), REAL_VARIABLE);
        _telDerotator = RTDBvar( ReplaceSide( ConfigDictionary()["TelRotatorVar"]), REAL_VARIABLE);
        _telElOk = true;
    } catch (AOVarException& e) {
        _logger->log(Logger::LOG_LEV_ERROR, "SetupVars(): %s", e.what().c_str());
        _logger->log(Logger::LOG_LEV_ERROR, "Derotation/elevation tracking will not work");
    }

    _slopenullOk = false;
    try {
        // Slopenull rotation tracking
        _curSlopenull = RTDBvar("slopecompctrl."+Side()+".SLOPENULL.CUR", CHAR_VARIABLE, 50); 
        _reqSlopenull = RTDBvar("slopecompctrl."+Side()+".SLOPENULL.REQ", CHAR_VARIABLE, 50); 
        _slopenullOk = true;
    } catch (AOVarException& e) {
        _logger->log(Logger::LOG_LEV_ERROR, "SetupVars(): %s", e.what().c_str());
        _logger->log(Logger::LOG_LEV_ERROR, "Slopenull rotation tracking will not work");
    }

    _safeSkipOk = false;
    try {
      _safeSkipPercent = RTDBvar( "adsecarb."+AOApp::Side()+".SAFESKIP_PERCENT@M_ADSEC", REAL_VARIABLE, 1);
      _safeSkipOk = true;
    } catch (AOVarException& e) {
        _logger->log(Logger::LOG_LEV_ERROR, "SetupVars(): %s", e.what().c_str());
        _logger->log(Logger::LOG_LEV_ERROR, "Safeskip detection will not work");
    }

    _adsecGainOk = false;
    try {
      _adsecGain = RTDBvar( "ADSEC."+AOApp::Side()+".G_GAIN_A@M_ADSEC", CHAR_VARIABLE, 256);
      _adsecGainOk = true;
    } catch (AOVarException& e) {
        _logger->log(Logger::LOG_LEV_ERROR, "SetupVars(): %s", e.what().c_str());
        _logger->log(Logger::LOG_LEV_ERROR, "Adsec gain detection will not work");
    }
 
    _aoLabMode   = RTDBvar(MyFullName()+".LAB_MODE", INT_VARIABLE, 1);

    _logger->log(Logger::LOG_LEV_DEBUG, "WfsArbitrator RTDB vars setup done.");
}

int WfsArbitrator::clTrackReqChanged( void *pt, Variable *var)
{
   WfsArbitrator *arbitrator = (WfsArbitrator *)pt;
   arbitrator->enableLensTracking( var->Value.Lv[0]);
   return NO_ERROR;
}

int WfsArbitrator::labModeHandler( void *pt, Variable *var)
{
   WfsArbitrator *arbitrator = (WfsArbitrator *)pt;
   arbitrator->_labmode = var->Value.Lv[0];
   return NO_ERROR;
}


int WfsArbitrator::offsetReqChanged( void *pt, Variable *var)
{
   WfsArbitrator *arbitrator = (WfsArbitrator *)pt;
   arbitrator->_offsetBin1Req.MatchAndSet(var);
   arbitrator->_offsetBin2Req.MatchAndSet(var);
   arbitrator->_offsetBin3Req.MatchAndSet(var);
   arbitrator->_offsetBin4Req.MatchAndSet(var);
   arbitrator->_offsetBin5Req.MatchAndSet(var);
   arbitrator->_offsetAdc1Req.MatchAndSet(var);
   arbitrator->_offsetAdc2Req.MatchAndSet(var);

   arbitrator->_offsetBin1Req.Get( arbitrator->_rotatorChangeOffset+1);
   arbitrator->_offsetBin2Req.Get( arbitrator->_rotatorChangeOffset+2);
   arbitrator->_offsetBin3Req.Get( arbitrator->_rotatorChangeOffset+3);
   arbitrator->_offsetBin4Req.Get( arbitrator->_rotatorChangeOffset+4);
   arbitrator->_offsetBin5Req.Get( arbitrator->_rotatorChangeOffset+5);
   arbitrator->_offsetAdc1Req.Get( & arbitrator->_adcOffset1);
   arbitrator->_offsetAdc2Req.Get( & arbitrator->_adcOffset2);

   arbitrator->_offsetBin1Cur.Set( arbitrator->_rotatorChangeOffset+1);
   arbitrator->_offsetBin2Cur.Set( arbitrator->_rotatorChangeOffset+2);
   arbitrator->_offsetBin3Cur.Set( arbitrator->_rotatorChangeOffset+3);
   arbitrator->_offsetBin4Cur.Set( arbitrator->_rotatorChangeOffset+4);
   arbitrator->_offsetBin5Cur.Set( arbitrator->_rotatorChangeOffset+5);

   arbitrator->_offsetAdc1Cur.Set( arbitrator->_adcOffset1);
   arbitrator->_offsetAdc2Cur.Set( arbitrator->_adcOffset2);

   return NO_ERROR;
}


void WfsArbitrator::InstallHandlers() {
	AbstractArbitrator::InstallHandlers();

	_logger->log(Logger::LOG_LEV_DEBUG, "WfsArbitrator installing custom handlers...");
	int stat = NO_ERROR;

	if(!IS_ERROR(stat = thHandler(WFS_ARB_SET_HO_POLICY, "*", 0, handleSetHoArmPolicy, "setHoArmPolicyHandler", this))) {
		_logger->log(Logger::LOG_LEV_DEBUG, "Handler for SetHoArmPolicy successfully installed!");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Error (code %d) while installing SetHoArmPolicy handler.", stat);
		throw ArbitratorException("Impossible to install SetHoArmPolicy handler", ARB_INIT_ERROR);
	}

	if(!IS_ERROR(stat = thHandler(WFS_ARB_SET_TV_POLICY, "*", 0, handleSetTvArmPolicy, "setTvPolicyHanlder", this))) {
		_logger->log(Logger::LOG_LEV_DEBUG, "Handler for SetTvArmPolicy successfully installed!");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Error (code %d) while installing SetHoArmPolicy handler.", stat);
		throw ArbitratorException("Impossible to install SetTvArmPolicy handler", ARB_INIT_ERROR);
	}

	if(!IS_ERROR(stat = thHandler(WFS_ARB_SET_AND_POINT_SOURCE_POLICY, "*", 0, handleSetAndPointSourcePolicy, "SetAndPointSourcePolicyHandler", this))) {
		_logger->log(Logger::LOG_LEV_DEBUG, "Handler for SetAndPointSourcePolicy successfully installed!");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Error (code %d) while installing SetHoArmPolicy handler.", stat);
		throw ArbitratorException("Impossible to install SetAndPointSourcePolicy handler", ARB_INIT_ERROR);
	}

   // This system must be initialized before the thStart because it uses thHandler

		_systems["OPTLOOPDIAG"] = new OptLoopDiagSystem();

	_logger->log(Logger::LOG_LEV_DEBUG, "WfsArbitrator message handlers installation done.");
}

void WfsArbitrator::PostInit() {
	AbstractArbitrator::PostInit();

	if(!TimeToDie()) {
		_logger->log(Logger::LOG_LEV_DEBUG, "WfsArbitrator PostInit checks and initializations...");

		// Initialize the AbstractSystems: WFS and AdSec
		// Can't do it before the thrdlib is started!
		_systems["WFS"] = new WfsSystem();
		_systems["MASTDIAGN"] = new MastDiagnSystem();
		_systems["ADSEC"] = new AdSecSystem();
		// ...complete initialization of systems!

		_logger->log(Logger::LOG_LEV_DEBUG, "WfsArbitrator PostInit checks and initializations done.");
		
		try {
		
		      Notify( _offsetBin1Req, offsetReqChanged);
		      Notify( _offsetBin2Req, offsetReqChanged);
		      Notify( _offsetBin3Req, offsetReqChanged);
		      Notify( _offsetBin4Req, offsetReqChanged);
		      Notify( _offsetBin5Req, offsetReqChanged);
		      Notify( _offsetAdc1Req, offsetReqChanged);
		      Notify( _offsetAdc2Req, offsetReqChanged);
		      Notify( _clTrackReq, clTrackReqChanged);
		      Notify(_ccd39Pup0, pupTrackingHandler);
		      Notify(_ccd39Pup1, pupTrackingHandler);
		      Notify(_ccd39Pup2, pupTrackingHandler);
		      Notify(_ccd39Pup3, pupTrackingHandler);
		      Notify(_clXcur, pupTrackingHandler);
		      Notify(_clYcur, pupTrackingHandler);
		      Notify( _slopeCompPupils, pupChangedHandler, 1);
                      if (_slopenullOk) {             
		          Notify( _curSlopenull, trackingHandler);
                      }
                      if (_safeSkipOk) {
		          eNotify( _safeSkipPercent, safeSkipPercentHandler, 1);
                      }
                      if (_adsecGainOk) {
		          eNotify( _adsecGain, adsecGainHandler, 1);
                      }
		      Notify (_aoLabMode, labModeHandler, true);
		}
		catch (AOVarException& e) {
		      _logger->log(Logger::LOG_LEV_ERROR, "PostInit(): %s", e.what().c_str());
		      throw ArbitratorException("Error RTDB vars notifications", ARB_INIT_ERROR);
		}
	}
}

int WfsArbitrator::pupChangedHandler( void *pt, Variable *var) {

   WfsArbitrator *arbitrator = (WfsArbitrator *)pt;
   arbitrator->_slopeCompPupils.MatchAndSet(var);

   string path = arbitrator->_slopeCompPupils.Get();
   size_t pos = path.find("LUTs/");
   if (pos != string::npos) {
	string pupils = path.substr(pos+4,21);
	arbitrator->_pupils.Set(pupils);
   }
   return NO_ERROR;
}

int WfsArbitrator::safeSkipPercentHandler( void *pt, Variable *var) {

   WfsArbitrator *arbitrator = (WfsArbitrator *)pt;
   arbitrator->_safeSkipPercent.MatchAndSet(var);
   return NO_ERROR;
}

int WfsArbitrator::adsecGainHandler( void *pt, Variable *var) {

   WfsArbitrator *arbitrator = (WfsArbitrator *)pt;
   arbitrator->_adsecGain.MatchAndSet(var);
   return NO_ERROR;
}
  
int cmpdiam( const void *p1, const void *p2) {
	double d1 = * (double*) p1;
	double d2 = * (double*) p2;
        if (d1==d2)
		return 0;
	if (d1>d2)
		return 1;
	else
		return -1;
}

int WfsArbitrator::pupTrackingHandler( void *pt, Variable *var) {

   static int cloopCounter=0;

   static bool lensAlert=false;

   WfsArbitrator *arbitrator = (WfsArbitrator *)pt;
   arbitrator->_clXcur.MatchAndSet(var);
   arbitrator->_clYcur.MatchAndSet(var);
   arbitrator->_ccd39Pup0.MatchAndSet(var);
   arbitrator->_ccd39Pup1.MatchAndSet(var);
   arbitrator->_ccd39Pup2.MatchAndSet(var);
   int set = arbitrator->_ccd39Pup3.MatchAndSet(var);

   if (set !=1)
	return NO_ERROR;

   _logger->log(Logger::LOG_LEV_INFO, "step1");

   WfsSystem* wfs = (WfsSystem*)_systems["WFS"];
   if (!wfs)
        return NO_ERROR;
   _logger->log(Logger::LOG_LEV_INFO, "step2");

   int ccdBin = wfs->getBinning();

   if (ccdBin<1)
	return NO_ERROR;
   _logger->log(Logger::LOG_LEV_INFO, "step3");

   float pix_micron=24.0;	// Pixel size in micron (bin 1x1)
   float gain=0.2;

   double diffx[4];
   double diffy[4];
   arbitrator->_ccd39Pup0.Get(4,diffx);
   arbitrator->_ccd39Pup1.Get(4,diffx+1);
   arbitrator->_ccd39Pup2.Get(4,diffx+2);
   arbitrator->_ccd39Pup3.Get(4,diffx+3);
   arbitrator->_ccd39Pup0.Get(5,diffy);
   arbitrator->_ccd39Pup1.Get(5,diffy+1);
   arbitrator->_ccd39Pup2.Get(5,diffy+2);
   arbitrator->_ccd39Pup3.Get(5,diffy+3);

   double dx = (diffx[0]+diffx[1]+diffx[2]+diffx[3])/4.0;
   double dy = (diffy[0]+diffy[1]+diffy[2]+diffy[3])/4.0;

   // Some basic safety check on the acq pupils.

   // If we get exactly zero, the pupilcheck process is just now starting!
   if ((dx ==0) && (dy==0)) {
	_logger->log(Logger::LOG_LEV_INFO, "Acq pupil not yet started, skipping camera lens iteration");
	return NO_ERROR;
   }
   _logger->log(Logger::LOG_LEV_INFO, "step4");

   // Diameter the same for all pupils
/*
   double d[4];
   arbitrator->_ccd39Pup0.Get(0,d);
   arbitrator->_ccd39Pup0.Get(0,d+1);
   arbitrator->_ccd39Pup0.Get(0,d+2);
   arbitrator->_ccd39Pup0.Get(0,d+3);
   qsort(d, sizeof(double), 4, cmpdiam);
   if (d[3]==0)
	return NO_ERROR;
   double factor = d[0]/d[3];
   if (factor < 0.8) {
	_logger->log(Logger::LOG_LEV_INFO, "Acq pupil gives pupils with different diameters, skipping camera lens iteration");
	return NO_ERROR;
   }
*/

   // Calculate current error

   double xerr = dx*pix_micron*ccdBin;
   double yerr = dy*pix_micron*ccdBin;
   _logger->log(Logger::LOG_LEV_INFO, "cameraLensTracking(): xerr=%5.1f yerr=%5.1f (pixel)", dx, dy);

   arbitrator->_clXErr.Set(xerr);
   arbitrator->_clYErr.Set(yerr);

   if ((fabs(xerr)< arbitrator->_lensTrackingTH*ccdBin) && (fabs(yerr) < arbitrator->_lensTrackingTH*ccdBin))
	arbitrator->_lensTrackingTarget = true;
   else
   	arbitrator->_lensTrackingTarget=false;

   bool correcting = true;
   if (!arbitrator->_lensTracking)
       correcting = false;

   _logger->log(Logger::LOG_LEV_INFO, "step5 - correcting = %d", (correcting) ? 1:0);

   // Check whether the lens tracking must be used or not

    string arbStatus, cmdStatus="", cmdName="";
    OpCode opcode;
 
    arbStatus = arbitrator->fsmState();
    Command* lastCmd = arbitrator->lastCommand();
    if(lastCmd != NULL) {
        cmdStatus = lastCmd->getStatusAsString();
        cmdName   = lastCmd->getDescription();
        opcode    = lastCmd->getCode();
    }

   _logger->log(Logger::LOG_LEV_INFO, "step6 - correcting = %d", (correcting) ? 1:0);

    // Do not move cameralens while any command is executing, except saving of optical loop data
    if (cmdStatus.compare("EXECUTING") ==0) 
        if (opcode != WfsOpCodes::SAVE_OPTLOOPDATA_CMD) 
            correcting = false;

   _logger->log(Logger::LOG_LEV_INFO, "step7 - correcting = %d", (correcting) ? 1:0);

    // Do not move cameralens if safe skip if active
    if (arbitrator->_safeSkipOk) {
        double safeSkipPercent;
        arbitrator->_safeSkipPercent.Get(&safeSkipPercent);
        if (safeSkipPercent > 0.5) {
	    _logger->log(Logger::LOG_LEV_INFO, "AdSec is skipping frames, camera lens not moving\n");
            correcting = false;
        }
    }
   _logger->log(Logger::LOG_LEV_INFO, "step8 - correcting = %d", (correcting) ? 1:0);

    // Do not move cameralens if gain is zero
    if (arbitrator->_adsecGainOk) {
        string gainfile = arbitrator->_adsecGain.Get();
        if (gainfile.find("gain0.fits") != string::npos) {
	    _logger->log(Logger::LOG_LEV_INFO, "AdSec has zero gain, camera lens not moving\n");
            correcting = false;
        }
    }
   _logger->log(Logger::LOG_LEV_INFO, "step9 - correcting = %d", (correcting) ? 1:0);

    // Do not move cameralens in open loop
    if (arbStatus.compare("LoopClosed")) {
        cloopCounter=0;
        correcting = false;
    }

   _logger->log(Logger::LOG_LEV_INFO, "step10 - correcting = %d", (correcting) ? 1:0);



    // Wait for second update when the loop is closed
    _logger->log(Logger::LOG_LEV_INFO, "Camera lens: cloopCounter = %d", cloopCounter);
    if (cloopCounter<2)
        correcting = false;

    // Slow down to avoid quueing camera lens commands
//    if (cloopCounter % 4)
//        correcting  =false;

    cloopCounter++;

   _logger->log(Logger::LOG_LEV_INFO, "step11 - correcting = %d", (correcting) ? 1:0);

   if (correcting)
       arbitrator->_clErrCorrecting.Set(1);
   else
       arbitrator->_clErrCorrecting.Set(0);

   _logger->log(Logger::LOG_LEV_INFO, "step12 - correcting = %d", (correcting) ? 1:0);
  
   if (!correcting)
       return NO_ERROR; 

   // Do not move if we are inside the target threshold
   if (arbitrator->_lensTrackingTarget)
       return NO_ERROR;

   double xmov = xerr*gain; 
   double ymov = yerr*gain;

   _logger->log(Logger::LOG_LEV_INFO, "cameraLensTracking(): moving by x=%5.1f y=%5.1f (um)", xmov, ymov);

   double xpos,ypos;
   arbitrator->_clXcur.Get(&xpos);
   arbitrator->_clYcur.Get(&ypos);
   double xtarget = xpos-ymov/1e6;
   double ytarget = ypos+xmov/1e6;
   double xmax = ((double)ConfigDictionary()["CLtrackingMaxX"]) / 1e6;
   double ymax = ((double)ConfigDictionary()["CLtrackingMaxY"]) / 1e6;
   double xmin = ((double)ConfigDictionary()["CLtrackingMinX"]) / 1e6;
   double ymin = ((double)ConfigDictionary()["CLtrackingMinY"]) / 1e6;
   bool rangeAlert=false;
   if (xtarget > xmax) { xtarget = xmax; rangeAlert=true; }
   if (ytarget > ymax) { ytarget = ymax; rangeAlert=true; }
   if (xtarget < xmin) { xtarget = xmin; rangeAlert=true; }
   if (ytarget < ymin) { ytarget = ymin; rangeAlert=true; }
   arbitrator->_lensTrackingAlert = rangeAlert;
   if (rangeAlert) {
       if (!lensAlert) {
          try {
             lensAlert=true;
             arbitrator->speak( arbitrator->_voiceCameraLensWarning);
          } catch(...) {}
       }
   } else {
       lensAlert=false;
   }

   _logger->log(Logger::LOG_LEV_INFO, "cameraLensTracking(): target is x=%g y=%g", xtarget, ytarget);
   _logger->log(Logger::LOG_LEV_INFO, "cameraLensTracking(): target is x=%5.1f y=%5.1f (um)", xtarget*1e6, ytarget*1e6);
   arbitrator->_clXreq.Set( xtarget);
   arbitrator->_clYreq.Set( ytarget);
   arbitrator->_clGo.Set(1, 0, FORCE_SEND);
   printf("Xmov: %g + %g = %g\n", xpos, xmov/1e6, xpos+xmov/1e6);

   return NO_ERROR;
}

int WfsArbitrator::trackingHandler( void *pt, Variable *var) {

   WfsArbitrator *arbitrator = (WfsArbitrator *)pt;
   arbitrator->_telElevation.MatchAndSet(var);
   arbitrator->_curSlopenull.MatchAndSet(var);


   return NO_ERROR;
}

double WfsArbitrator::derot2rerot( double derotPos) {

   WfsSystem* wfs = (WfsSystem*)_systems["WFS"];
   int ccdBin = wfs->getBinning();
   if (ccdBin<0)
      return 0;

   double pos = derotPos * _rotatorSign/2.0 + _rotatorOffset[ccdBin] + _rotatorChangeOffset[ccdBin];
   int periods = int(pos/360.0);
   pos -= periods*360;
   while (pos<0)
	pos += 360.0;

   return pos; 
}

double WfsArbitrator::elev2adc( double elevPos) {
	return 90.0-elevPos;
}

void WfsArbitrator::rerotTrack() {

   double derotPos;
   static int first=1;
   static double prevPos=0;
   WfsSystem* wfs = (WfsSystem*)_systems["WFS"];

   if (!_telElOk)
      return;

   _telDerotator.Get(&derotPos);

   double rerotPos = derot2rerot(derotPos);

   int skip=0;
   if (isLoopClosed(this)) {
      if (first) {
         first=0;
      } else {
         if (fabs(rerotPos - prevPos) > (double)ConfigDictionary()["RotOffsetMax"]) {
            _logger->log(Logger::LOG_LEV_WARNING, "Rotator tracking: rejected offset of %5.2f degrees in closed loop", fabs(rerotPos - prevPos));
            skip=1;
         }
      }
   } else first=1;

   if (skip)
       return;

   prevPos = rerotPos;


   if (!_labmode)
   	thWriteVar( ReplaceSide( ((string)ConfigDictionary()["RerotVar"])).c_str(), REAL_VARIABLE, 1, &rerotPos, 1000);

   if (!_labmode)
        updateSlopenull( derotPos);

   if (!_labmode) {
        float rangemin = wfs->getRangeMin();
        float rangemax = wfs->getRangeMax();
        if ((derotPos < rangemin) || (derotPos > rangemax)) {
           // Camera lens range exceeded, warn user
          _logger->log(Logger::LOG_LEV_WARNING, "Camera lens range exceeded.");
	  _lensTrackingOutOfRange = true;
        } else {
	  _lensTrackingOutOfRange = false;
        }
   }
}

void WfsArbitrator::updateSlopenull( double derotPos) {

   // Update the current slopenull searching for a file which has a better match for the current rotation
   if (!_slopenullOk)
      return;

   string curSlopenull = _curSlopenull.Get();

   WfsSystem* wfs = (WfsSystem*)_systems["WFS"];
   int ccdBin = wfs->getBinning();
   _logger->log(Logger::LOG_LEV_DEBUG, "updateSlopenull(): binning is %d", ccdBin);
   if (ccdBin<1)
       return;

   string tracknum = curSlopenull.substr(0,15);
   string dir = Paths::slopenullDir( ccdBin);
   vector<string> files;
   Utils::readDir( dir, files);
   vector<string>::iterator iter;
 

   double derot = fmod(derotPos, 360.0);
   double diff = 1000;
   string found("");
   for (iter = files.begin(); iter != files.end(); iter++)
      if (iter->size() >21)
         if (iter->substr(0,15).compare(tracknum) == 0) {
             double angle = atof(iter->substr(17).c_str());
             double d = fabs(angle-derot);
             if (d < diff) {
                 diff = d;
                 found = *iter;
             }
         }

   if (found.compare("")) {
      _reqSlopenull.Set(found);
      _logger->log(Logger::LOG_LEV_INFO, "Loading slopenull: %s", found.c_str());
   }


}

void WfsArbitrator::adcTrack() {

   int zeroTrack;

   double derotPos, elevPos;
   if (!_telElOk)
      return;

   static double lastadc1=-1;  //change to static by JRM 2014.10.31
   static double lastadc2=-1; //change to static by JRM 2014.10.31
   double threshold=0.3;

   _adcZeroTrack.Get(&zeroTrack);

   _telDerotator.Get(&derotPos);
   _telElevation.Get(&elevPos);
  
   double zenithangle = elev2adc(elevPos);

   //std::cout << "derotPos: " << derotPos << "\n";
   //std::cout << "zenith angle: " << zenithangle << "\n";

   double adc1, adc2;
   if(zeroTrack)
   {
      adc1 = (double)ConfigDictionary()["Adc1Offset"] + _adcOffset1;
      adc2 = (double)ConfigDictionary()["Adc2Offset"] + _adcOffset2;
   }
   else
   {
      // We need a valid LUT
      if (!_adcLutValid) return;

      adc1 = -1*adcInterpolate( _adc1, zenithangle);  //added -1 by JRM 2014.10.31
      adc2 = -1*adcInterpolate( _adc2, zenithangle);  //added -1 by JRM 2014.10.31
   
      adc1 += (double)ConfigDictionary()["Adc1Offset"] + derotPos +  _adcOffset1;
      adc2 += (double)ConfigDictionary()["Adc2Offset"] - derotPos +  _adcOffset2;
   }

   //std::cout << "adc1-abs: " << adc1 << " adc2-abs: " << adc2 << "\n";

   if(_adc1Limits)
   {
      if( adc1 < _adc1Min ) 
      {
         adc1 = _adc1Min;
         speak(_voiceADCMinimumWarning);
      }

      if( adc1 > _adc1Max )
      {
         adc1 = _adc1Max;
         speak(_voiceADCMaximumWarning);
      }
   }

   if ( fabs(lastadc1-adc1) > threshold || fabs(lastadc2-adc2) > threshold) {  //added fabs() by JRM 2014.10.31

       thWriteVar( ReplaceSide( ((string)ConfigDictionary()["Adc1Var"])).c_str(), REAL_VARIABLE, 1, &adc1, 1000);
       thWriteVar( ReplaceSide( ((string)ConfigDictionary()["Adc2Var"])).c_str(), REAL_VARIABLE, 1, &adc2, 1000);

       lastadc1= adc1;
       lastadc2= adc2;
   }
}

void WfsArbitrator::enableLensTracking(bool enable) {

 _lensTracking = enable;
}


void WfsArbitrator::Periodic() {

   static int last_adcTracking = -1;

    // Retorator tracking with polling
    try {
        if (_rerotTracking) {
            _telDerotator.Update();
            rerotTrack();
        }
    } catch (AOException &e) {
       _logger->log(Logger::LOG_LEV_ERROR, "Cannot read telescope rotator position: %s", e.what().c_str());
    } catch (...) {
       _logger->log(Logger::LOG_LEV_ERROR, "Cannot read telescope rotator position");
    }

   // Elevation tracking with polling
   try {
       if (_adcTracking) {
          _telElevation.Update();
          _telDerotator.Update();
          _adcZeroTrack.Update();
          adcTrack();

          if( last_adcTracking != _adcTracking)
          {
             int zeroTrack;
             _adcZeroTrack.Get(&zeroTrack);
             if(zeroTrack)
             {
                speak( _voiceADCZeroTrackEnabled);
             }
             else
             {
               speak( _voiceADCTrackingEnabled);
             }
          }

       }
    } catch (AOException &e) {
       _logger->log(Logger::LOG_LEV_ERROR, "Cannot read telescope elevation: %s", e.what().c_str());
    } catch (...) {
       _logger->log(Logger::LOG_LEV_ERROR, "Cannot read telescope elevation");
    }

   last_adcTracking = _adcTracking;

    return;

    // Re-read config file
    //printf("Re-reading config. file\n");
    //string cfgfile = ConfigDictionary().getCfgFile();
    //Config_File *cfg = new Config_File( cfgfile);
    //_rotatorOffset = (double)ConfigDictionary()["RotatorOffset"];
    //_rotatorSign = (double)ConfigDictionary()["RotatorSign"];
    //printf("Offset: %f   - sign: %f\n", _rotatorOffset, _rotatorSign);
    //delete cfg;

}



int WfsArbitrator::handleSetHoArmPolicy(MsgBuf* requestMsgBuf, void *argp, int hndlrQueueSize) {
	// Check the queue of this handler: if it's becoming full
	// react in the appropriate way, i.e. ignoring the current buffer
	// (remember thRelease()!!!)
	if(hndlrQueueSize > HNDLR_QUEUE_MESSAGES_LIMIT) {
		thRelease(requestMsgBuf);
		return THRD_QUEUE_OVERRUN_ERROR;
	}

	static Logger* logger = Logger::get("ASYNCHCOMMANDHANDLER");

	WfsArbitrator* myself = (WfsArbitrator*)argp;
	if(!myself->IsReady()) {
		logger->log(Logger::LOG_LEV_ERROR, "setHOPolicy request refused: arbitrator not read!");
		thRelease(requestMsgBuf);
		return NOT_INIT_ERROR;
	}


	logger->log(Logger::LOG_LEV_INFO, "Processing SetHoArmPolicy...");

	SetHoArmPolicy hoArmPolicy = *((SetHoArmPolicy*)MSG_BODY(requestMsgBuf));

	WfsSystem* wfs = (WfsSystem*)_systems["WFS"];
	wfs->setHoArmPolicy(hoArmPolicy);
	AbstractSystemCommandResult res = wfs->getCommandResult();

	if(res == SysCmdSuccess) {
		thReplyMsg(ACK, 0, NULL, requestMsgBuf);
		logger->log(Logger::LOG_LEV_INFO, "SetHoArmPolicy succesfully executed!");
	}
	else {
		thReplyMsg(NAK, 0, NULL, requestMsgBuf);
		logger->log(Logger::LOG_LEV_WARNING, "SetHoArmPolicy failed!");
	}

	return NO_ERROR;
}

int WfsArbitrator::handleSetTvArmPolicy(MsgBuf* requestMsgBuf, void *argp, int hndlrQueueSize) {
	// Check the queue of this handler: if it's becoming full
	// react in the appropriate way, i.e. ignoring the current buffer
	// (remember thRelease()!!!)
	if(hndlrQueueSize > HNDLR_QUEUE_MESSAGES_LIMIT) {
		thRelease(requestMsgBuf);
		return THRD_QUEUE_OVERRUN_ERROR;
	}

	static Logger* logger = Logger::get("ASYNCHCOMMANDHANDLER");

	WfsArbitrator* myself = (WfsArbitrator*)argp;
	if(!myself->IsReady()) {
		logger->log(Logger::LOG_LEV_ERROR, "setTVPolicy request refused: arbitrator not read!");
		thRelease(requestMsgBuf);
		return NOT_INIT_ERROR;
	}

	logger->log(Logger::LOG_LEV_INFO, "Processing SetTvArmPolicy...");

	SetTvArmPolicy tvArmPolicy = *((SetTvArmPolicy*)MSG_BODY(requestMsgBuf));

	WfsSystem* wfs = (WfsSystem*)_systems["WFS"];
	wfs->setTvArmPolicy(tvArmPolicy);
	AbstractSystemCommandResult res = wfs->getCommandResult();

	if(res == SysCmdSuccess) {
		thReplyMsg(ACK, 0, NULL, requestMsgBuf);
		logger->log(Logger::LOG_LEV_INFO, "SetTvArmPolicy succesfully executed!");
	}
	else {
		thReplyMsg(NAK, 0, NULL, requestMsgBuf);
		logger->log(Logger::LOG_LEV_WARNING, "SetTvArmPolicy failed!");
	}

	return NO_ERROR;
}

int WfsArbitrator::handleSetAndPointSourcePolicy(MsgBuf* requestMsgBuf, void *argp, int hndlrQueueSize) {
	// Check the queue of this handler: if it's becoming full
	// react in the appropriate way, i.e. ignoring the current buffer
	// (remember thRelease()!!!)
	if(hndlrQueueSize > HNDLR_QUEUE_MESSAGES_LIMIT) {
		thRelease(requestMsgBuf);
		return THRD_QUEUE_OVERRUN_ERROR;
	}

	static Logger* logger = Logger::get("ASYNCHCOMMANDHANDLER");

	WfsArbitrator* myself = (WfsArbitrator*)argp;
	if(!myself->IsReady()) {
		logger->log(Logger::LOG_LEV_ERROR, "setPointAndSourcePolicy request refused: Arbitrator not ready!");
		thRelease(requestMsgBuf);
		return NOT_INIT_ERROR;
	}

	logger->log(Logger::LOG_LEV_INFO, "Processing PointAndSourcePolicy...");

	PointAndSourcePolicy setPointAndSourceArmPolicy = *((PointAndSourcePolicy*)MSG_BODY(requestMsgBuf));

	WfsSystem* wfs = (WfsSystem*)_systems["WFS"];
	wfs->setPointAndSourcePolicy(setPointAndSourceArmPolicy);
	AbstractSystemCommandResult res = wfs->getCommandResult();

	if(res == SysCmdSuccess) {
		thReplyMsg(ACK, 0, NULL, requestMsgBuf);
		logger->log(Logger::LOG_LEV_INFO, "PointAndSourcePolicy succesfully executed!");
	}
	else {
		thReplyMsg(NAK, 0, NULL, requestMsgBuf);
		logger->log(Logger::LOG_LEV_WARNING, "PointAndSourcePolicy failed!");
	}

	return NO_ERROR;
}

void WfsArbitrator::updateRtdb() {

   // WfsSystem* wfs = (WfsSystem*)_systems["WFS"];
/*   
   _ccdFreqLimits.Set( 0, 50.0);
   _ccdFreqLimits.Set( 1, 1000.0);

   int hoBin, tvBin, fw1pos, nSubap;
   double ttMod, hoFreq, tvExptime;

   _ttmodVar.Get(&ttMod);
   _modAmpl.Set( ttmod);

   _msg.Set("")  // [TODO]

   _pyramidPos.Set(0,0);    // [TODO] from cfg file.
   _pyramidPos.Set(1,0);

*/

}

void WfsArbitrator::processAlert(Alert* alert) {

   string state;
   string sender = alert->getSender();
   string desc = alert->getShortDescription();
   string message = alert->getTextMessage();
   bool propagateAlert = true;



   _logger->log(Logger::LOG_LEV_INFO, "Processing alert %d (%s) from %s: %s", alert->getCode(), desc.c_str(), sender.c_str(), message.c_str());

   switch(alert->getCode()) {
      case BasicOpCodes::WARNING:
         // [Todo] something?
      break;

      case BasicOpCodes::ERROR:
	// TODO: 
        // Open the loop if currently closed. Since the bcu39 may be out of reach,
         // we can try to reset it using the power board.

       state = ((WfsFsm *)_cmdHandler)->currentStateAsString().c_str();
       if ((state == "LoopClosed") || (state=="LoopSuspended")) {

           ((WfsSystem*)_systems["WFS"])->stopLoop();
           ((MastDiagnSystem*)_systems["MASTDIAGN"])->openLoop();

           ((WfsFsm*)_cmdHandler)->forceState(WfsFsm::AOSet);
       }

       // An alert is expected from SlopeCompCtrl when we are turning off the system. The alert can be discarded
       if (sender == "slopecompctrl."+AOApp::Side()) {
           Command* lastCmd = this->lastCommand();
           if (lastCmd != NULL)
               if ((state == "PowerOff") || 
                   ((lastCmd->getStatus() == Command::CMD_EXECUTING) &&
                    (lastCmd->getCode() == WfsOpCodes::OFF_CMD)))
                  propagateAlert = false;      
       }

       break;

       case BasicOpCodes::PANIC:
          // [Todo]
       break;

       default:
          _logger->log(Logger::LOG_LEV_ERROR, "Unknown Alert opcode %d received from %s", alert->getCode(), sender.c_str());
       break;
   }

	// Notify all clients registered
    if (propagateAlert)
        notifyClients(alert);
}

void WfsArbitrator::rotate( double x, double y, double *newx, double *newy) {

    double ang = _focalPlaneRotation * 3.1415 / 180.;
    if (newx)
        *newx = x*cos(ang) - y*sin(ang);
    if (newy)
        *newy = x*sin(ang) + y*cos(ang);
}


int WfsArbitrator::isLoopClosed( WfsArbitrator *arb) {

    string arbStatus = arb->fsmState();
    if (arbStatus.compare("LoopClosed"))
       return 0;
    return 1;
}

void WfsArbitrator::sync() {

    double t;
    int fl;

    _bcu39sx.Update();
    _bcu39sx.Get(&t);
    _flEnabled.Update();
    _flEnabled.Get(&fl);

    if (t>0) {
        _led.Set(WFS_ON);
        if (fl>0)
            ((WfsFsm *)_cmdHandler)->forceState( ((WfsFsm *)_cmdHandler)->stateCode("LoopClosed"));
        else
            ((WfsFsm *)_cmdHandler)->forceState( ((WfsFsm *)_cmdHandler)->stateCode("Operating"));
    }
    else {
        _led.Set(WFS_OFF);
        ((WfsFsm *)_cmdHandler)->forceState( ((WfsFsm *)_cmdHandler)->stateCode("PowerOff"));
    }
    _fsmStateVar->Set( ((WfsFsm *)_cmdHandler)->currentStateAsString());
}

void WfsArbitrator::speak( string msg) {

  if(_voicePort == -1) return;

  char str[1024];

  if (msg.compare("none") == 0)
      return;

  snprintf(str, 1024, "echo \"%s %s\" | nc %s %d &", _voiceName.c_str(), msg.c_str(), _voiceHost.c_str(), _voicePort);
  system(str);

}
    

