#include "AdSecArbitrator.h"
#include "AdSecFsm.h"
#include "AdSecCommandImplFactory.h"
#include "IdlSystem.h"
#include "HousekeeperSystem.h"
#include "FastDiagnSystem.h"
#include "MastDiagnSystem.h"
#include "AdamSystem.h"
#include "arblib/base/Alerts.h"
#include "NaNutils.h"
#include "aoslib/aoscodes.h"
#include "Paths.h"
#include <math.h>

//#define USE_ADAM 1

//#ifdef USE_ADAM
//#include "AdamLib.h"
//#endif

using namespace Arcetri::Arbitrator;
using namespace Arcetri::AdSec_Arbitrator;

AdSecArbitrator::AdSecArbitrator(int argc, char* argv[]):AbstractArbitrator(argc, argv) {
	create();
}

void AdSecArbitrator::create() {

    RequireKeyword("HOUSEKEEPERLogLev");
    RequireKeyword("FASTDIAGNLogLev");
    RequireKeyword("IDLLogLev");
    RequireKeyword("MASTDIAGNLogLev");
    RequireKeyword("IdlCtrlProcess");
    RequireKeyword("maxWindSpeed");
    RequireKeyword("hystWindSpeed");
    RequireKeyword("minElevationRest");
    RequireKeyword("minElevationSet");
    CheckKeywords(true, true, true);


	// Set logger levels for specific components: the levels are got
	// from config file
	setLoggerLevel("HOUSEKEEPER");
	setLoggerLevel("FASTDIAGN");
	setLoggerLevel("IDL");
	setLoggerLevel("MASTDIAGN");

   string sMode;
   if(isSimulation()) sMode="(SIMULATION)";

   _logger->log(Logger::LOG_LEV_INFO, "AdSecArbitrator version %s %s", GetVersionID(),sMode.c_str());

	// Create a CommandImplFactory, able to implement AO commands
	_cmdImplFactory = new AdSecCommandImplFactory();

	// Create the AOFsm (with the given CommandImplFactory), able
	// to process AO commands
	_cmdHandler = new AdSecFsm(_commandHistorySize, MyName());

   // Set starting state
   int initialSt=((AdSecFsm *)_cmdHandler)->stateCode( (std::string)ConfigDictionary()["INITIAL_STATE"]);
   if(initialSt>=0) {
      ((AdSecFsm *)_cmdHandler)->forceState((AdSecFsm::States)initialSt);
      _logger->log(Logger::LOG_LEV_INFO, "AdSecArbitrator starting from state %s (from configuration file)", ((AdSecFsm *)_cmdHandler)->currentStateAsString().c_str());
   } else {
      _logger->log(Logger::LOG_LEV_INFO, "AdSecArbitrator starting from default state %s", ((AdSecFsm *)_cmdHandler)->currentStateAsString().c_str());
   }

   _tss_enabled=false;
   _labmode=false;
   _coilsEnabled=false;

   _varDoOffloadHighOrder = NULL;

   try {
      _anem_AOS=(int)ConfigDictionary()["anemometerFromAOS"];
   }
   catch (Config_File_Exception &e) {
      _anem_AOS=1;
   }

   double maxWindSpeed, hystWindSpeed;
   try {
       maxWindSpeed=(double)ConfigDictionary()["maxWindSpeed"];
   }
   catch (Config_File_Exception &e) {
       maxWindSpeed = 7;
       _logger->log(Logger::LOG_LEV_WARNING, "No max windspeed defined in config file. Using default value of %f m/s",maxWindSpeed);
   }

 
   try {
      hystWindSpeed=(double)ConfigDictionary()["hystWindSpeed"];
   }
   catch (Config_File_Exception &e) {
       _logger->log(Logger::LOG_LEV_WARNING, "No hysteresis windspeed defined in config file. Using default value of 1m/s");
       hystWindSpeed = 1;
   }
   _upperWindSpeed=maxWindSpeed+hystWindSpeed;
   _lowerWindSpeed=maxWindSpeed-hystWindSpeed;

   _logger->log(Logger::LOG_LEV_INFO, "Wind lower,upper thresholds: %f,%f m/s",_lowerWindSpeed,_upperWindSpeed);

   try {
       _minElevationRest=(double)ConfigDictionary()["minElevationRest"];
   }
   catch (Config_File_Exception &e) {
       _minElevationRest=25;
       _logger->log(Logger::LOG_LEV_WARNING, "No elevation threshold for resting defined in config file. Using default value of %f", _minElevationRest);
   }
   _logger->log(Logger::LOG_LEV_INFO, "Elevation threshold for resting: %f deg",_minElevationRest);


   try {
       _minElevationSet=(double)ConfigDictionary()["minElevationSet"];
   }
   catch (Config_File_Exception &e) {
       _minElevationSet=25;
       _logger->log(Logger::LOG_LEV_WARNING, "No minimum elevation for setting defined in config file. Using default value of %f", _minElevationSet);
   }
   _logger->log(Logger::LOG_LEV_INFO, "Min elevation threshold for setting the shell: %f deg",_minElevationSet);

   _safeSkipCounter=0;
   _safeSkipPercent=0;
   _skipping = false;
   _LOOffloadReq= false;

   _last_elevation_step = (-1);     //initialize to impossible value 



   // read led state map
  
   _ledStateMap.clear();
   for (Config_File::iterator it = ConfigDictionary().begin(); it != ConfigDictionary().end(); it++) {
      if (it->first.find(ADSECARBITRATOR_LEDSTATEMAP_STR) == 0) {
          string state = it->first.substr( it->first.rfind(".")+1 );
          int value = it->second;
          _ledStateMap[state] = value;
          _logger->log(Logger::LOG_LEV_DEBUG, "_ledStateMap['%s'] = %d", state.c_str(), value);
      }
   }


   //_last_tss = false;
 
   /* NOTE: Adam init moved to postinit

   if(! _simulation) {
   // create connection to Adam
      try {
         _adam = new Adam();
      } catch (AOException& e) {
         _logger->log(Logger::LOG_LEV_FATAL, "AdSecArbitrator connection to ADAM failed %s", e.what().c_str());
         throw(e);
      }
   }
   enableTSS(true,true);  // Initially enable TSS
   */

   // END

}


AdSecArbitrator::~AdSecArbitrator() {

    //AdamSystem * adam;
    if (_simulation) 
        return;
    
    // Enable TSS just to be sure 
    // NOTE: arbitrator systems are destroyed at the base class destructor
    AdamSystem * adam = (AdamSystem*)(_systems[ADAMSYSTEM_NAME]);
    if ((adam != NULL) && (!_labmode))
        adam->enableTss();
    
    // TO BE REMOVED AFTER AOS FUNCTIONALITIES CHECK
    //_adam->disableTss();
    // END
    //delete _adam;
}



void AdSecArbitrator::SetupVars() {
    AbstractArbitrator::SetupVars();

    _logger->log(Logger::LOG_LEV_DEBUG, "AdSecArbitrator setting up RTDB vars...");
  

    try {
	// Request notification for Wind and Elevation variables
	string doOffloadHighOrderVarname = "adsecarb."+Side()+".DOOFFLOADHIGHORDER";
	Notify( doOffloadHighOrderVarname, doOffloadHighOrderHandler);

   // Request notification for demo mode check
   string demoModeVarname = Side()+".IDL_STAT";
   Notify( demoModeVarname, demoModeHandler);
    }
    catch (AOVarException& e) {
	_logger->log(Logger::LOG_LEV_ERROR, "SetupVars: %s", e.what().c_str());
	throw ArbitratorException("Error while setting up RTDB vars", ARB_INIT_ERROR);
    }

    _logger->log(Logger::LOG_LEV_DEBUG, "AdSecArbitrator RTDB vars setup done.");
}

int AdSecArbitrator::doOffloadHighOrderHandler( void *pt, Variable *var) {

   AdSecArbitrator *adsecarb = (AdSecArbitrator *)pt;

	adsecarb->_logger->log(Logger::LOG_LEV_DEBUG, "AdSecArbitrator doOffloadHighOrderHandler [%s:%d]", __FILE__, __LINE__);
   if (!adsecarb->varDoOffloadHighOrder())
      adsecarb->varDoOffloadHighOrder() = new RTDBvar(var);
   else
      adsecarb->varDoOffloadHighOrder()->Set(var);
 
   return NO_ERROR;
}

int AdSecArbitrator::demoModeHandler( void *pt, Variable *var) {

   AdSecArbitrator *adsecarb = (AdSecArbitrator *)pt;

   adsecarb->_logger->log(Logger::LOG_LEV_DEBUG, "AdSecArbitrator demoModeHandler [%s:%d]", __FILE__, __LINE__);
   if (!var->Value.Lv[0]) {

      // [TODO] IDL is in demo mode. RIP mirror?
      adsecarb->_logger->log(Logger::LOG_LEV_WARNING, "IDL in demo mode!");
   }

   return NO_ERROR;
}


int AdSecArbitrator::adsecStateChanged( string /* state */) {
   return NO_ERROR;
}  

int AdSecArbitrator::adsecStateHandler( void *pt, Variable *msgb) {
   AdSecArbitrator *arb = (AdSecArbitrator *)pt;

   arb->adsecStateChanged(string(msgb->Value.Sv, msgb->H.NItems));
   return NO_ERROR;
}


void AdSecArbitrator::PostInit() {
	AbstractArbitrator::PostInit();

	if(!TimeToDie()) {                      // Setup safety variables
		_logger->log(Logger::LOG_LEV_DEBUG, "AdSecArbitrator PostInit checks and initializations...");

        if(_anem_AOS)
           _varWind = new RTDBvar("AOS.AMB.WINDSPEED",REAL_VARIABLE,1,false);
        else
           _varWind = new RTDBvar("ANEM."+Side()+".CKSPEED",REAL_VARIABLE,1,false);

        _varGuiding = new RTDBvar("AOS.TEL.ISGUIDING",REAL_VARIABLE,1,false);
        _varElev = new RTDBvar("AOS.TEL.EL",REAL_VARIABLE,1,false);
        _varSWA  = new RTDBvar("AOS."+Side()+".SWA.DEPLOYED",INT_VARIABLE,1,false);
        _varLabmode  = new RTDBvar(Side()+".LAB.MODE",INT_VARIABLE,1,true);
        _varLed  = new RTDBvar(MyFullName()+".LED",INT_VARIABLE,1);
        _varLed->Set(0);
        _varSafeSkipPercent  = new RTDBvar("adsecarb."+Side()+".SAFESKIP_PERCENT",REAL_VARIABLE,1);
        _varSafeSkipPercent->Set(0.0);

        _varFocalStation = new RTDBvar(MyFullName()+".FOCAL_STATION", CHAR_VARIABLE, 50);
        _varFocalStation->Set("null\0");
        _varCoilsAdam = new RTDBvar("adamhousekeeper."+Side()+".COIL_STATUS", INT_VARIABLE, 1);
        _varTSSEnabled = new RTDBvar("adamhousekeeper."+Side()+".TSS_ENABLED", INT_VARIABLE, 1);
        _varTTOffloadEnable = new RTDBvar(MyFullName()+".DOOFFLOADTT_ENABLED", INT_VARIABLE, 1);
        _varHOOffloadTime = new RTDBvar(MyFullName()+".DOOFFLOADHO_LASTTIME", CHAR_VARIABLE, 20);
    }

    // Wait for idl_ctrl control process to become ready
    while(!TimeToDie()) {
        int ret;
        
        _logger->log( Logger::LOG_LEV_INFO, "Waiting for idl controller to become ready....");
        if(_simulation) {
            _logger->log(Logger::LOG_LEV_INFO, "SIMULATION: I don't actually wait.");
            break;
        } else {
            if ((ret = thWaitReady( (((string)ConfigDictionary()["IdlCtrlProcess"])+"."+Side()).c_str(), 5000)) == NO_ERROR) break;
            if (ret != TIMEOUT_ERROR)
                throw AOException("Error waiting for Idl controller to become ready", ret, __FILE__, __LINE__);
        }
    }

    if (TimeToDie()) return;

    // Attach to current IDL status, if any 
    try {
        int errCode = NO_ERROR;
        string fsmVarName= "ADSEC."+Side()+".FSM_STATE";

        MsgBuf *buf = thGetVar( fsmVarName.c_str(), 1000, &errCode);
        if (errCode != NO_ERROR)
            throw AOException("Error reading variable "+fsmVarName, errCode, __FILE__, __LINE__);

        ((AdSecFsm*)_cmdHandler)->forceState( ((AdSecFsm*)_cmdHandler)->stateCode(thValue(buf)->Value.Sv));
        _logger->log(Logger::LOG_LEV_INFO, "AdSecArbitrator forcing state %s (from IDL status)", ((AdSecFsm *)_cmdHandler)->currentStateAsString().c_str());
        _varLed->Set( _ledStateMap[((AdSecFsm *)_cmdHandler)->currentStateAsString()]);
    } catch (AOException &e) { _logger->log( Logger::LOG_LEV_ERROR, "%s", e.what().c_str()); }

	// Initialize the AbstractSystems: WFS and AdSec
	// Can't do it before the thrdlib is started!
    bool simulation=isSimulation();
	_systems["IDL"] = new IdlSystem(simulation);
	_systems["HOUSEKEEPER"] = new HousekeeperSystem(simulation);
	_systems["FASTDIAGN"] = new FastDiagnSystem(simulation);
	_systems["MASTDIAGN"] = new MastDiagnSystem(simulation);
	_systems[ADAMSYSTEM_NAME] = new AdamSystem(simulation);
	// ...complete initialization of systems!

    // Initialize interface to ourselves
    addArbitratorInterface(MyFullName());

    getCoilsEnabled();
    _logger->log(Logger::LOG_LEV_INFO, "Detected coils status ----> %d", _coilsEnabled);

    getTSSEnabled();
	_logger->log(Logger::LOG_LEV_INFO, "Detected TSS enabled  ----> %d", _tss_enabled);

    if (!checkLabMode()) // if not in lab mode force enable tss
        enableTSS(true,true);  
    else           // otherwise force disable it
        enableTSS(false, true); 

    refreshFlatList();
	_logger->log(Logger::LOG_LEV_DEBUG, "AdSecArbitrator PostInit checks and initializations done.");
}

// Send a message to the AOS with the current flat list

void AdSecArbitrator::refreshFlatList() {

   vector<string> files;
   int errcode = Utils::readDir( Paths::FlatsDir(true), files);
   if (errcode) {
	   _logger->log(Logger::LOG_LEV_ERROR, "Cannot refresh flat list: (%d) %s", errcode, lao_strerror(errcode));
      return;
   }
   std::sort( files.begin(), files.end());

   string flatlist="";
   vector<string>::iterator iter;
   bool first = true;
   for (iter = files.begin(); iter != files.end(); iter++)
      if (Utils::hasExtension(*iter, ".sav")) {
         if (!first)
            flatlist += ";";
         first=false;
         flatlist += *iter; 
      }

   thSendMsgPl( flatlist.size()+1, ("AOS."+Side()).c_str(), AOS_HOUSKEEP, AOS_FLAT_LIST, 0, (void*)&flatlist[0]);
} 

// Determine whether TT offloading can be turned on or off, depending on the various flags.

bool AdSecArbitrator::checkTTOffload() {

    IdlSystem* idl = (IdlSystem*)(_systems["IDL"]);
    FastDiagnSystem *fd = (FastDiagnSystem*)(_systems["FASTDIAGN"]);
    AdSecFsm::States curStat=((AdSecFsm *)_cmdHandler)->currentState();

    bool loopClosed = (curStat == AdSecFsm::AORunning);
    bool gainZero   = idl->isGainZero();
    bool safeSkip   = (fd->getSafeSkipPercent() > 0.5) ? true : false;
    bool coilsEnabled = getCoilsEnabled();
    bool isOpening = false;

    Command* lastCmd = this->lastCommand();
    if (lastCmd != NULL)
        if ((lastCmd->getStatus() == Command::CMD_EXECUTING) &&
            (lastCmd->getCode() == AdSecOpCodes::STOP_AO_CMD))
            isOpening=true;


    if (_LOOffloadReq && loopClosed && (!gainZero) && (!safeSkip) && coilsEnabled && (!isOpening))  {
        fd->startTTOffload();
        return true;
    } else {
        fd->stopTTOffload();
        return false;
    }
}

double AdSecArbitrator::getElevation() {
    return _elevation;
}

bool AdSecArbitrator::getCoilsEnabled() {
    try {
        _varCoilsAdam->Update();
        _varCoilsAdam->Get(&_coilsEnabled);
    }
    catch (AOException &e) {
        _logger->log(Logger::LOG_LEV_WARNING, "Cannot read coils status (%s)", e.what().c_str());
    }
        
    return _coilsEnabled;
}

bool AdSecArbitrator::getTSSEnabled() {
    try {
        _varTSSEnabled->Update();
        _varTSSEnabled->Get(&_tss_enabled);
    }
    catch (AOException &e) {
        _logger->log(Logger::LOG_LEV_WARNING, "Cannot read TSS enabled status (%s)", e.what().c_str());
    }
        
    return _tss_enabled;
}


void AdSecArbitrator::setCoilsEnabled( bool enabled) {
    _coilsEnabled = enabled;
}


ElevState AdSecArbitrator::checkElevation(ElevMode elevMode) {    // Perform checks on elevation
   struct timeval vtime, now;

   bool elevOK=true;
   _elevation=NaNutils::dNaN();

   try {
      _varElev->Update();
   } catch (...) {
      elevOK=false;
   }
   if(elevOK) {
      _varElev->Get(&_elevation);
      gettimeofday(&now, NULL);
      vtime = _varElev->mTime();
      if (Utils::timediff(&now, &vtime) > EXPIRE_TIME || NaNutils::isNaN(_elevation)) elevOK=false;
   }
   if(!elevOK) return ElevationNotAvailable;

   double elevThrsh;

   switch(elevMode) {
      case ElevSet:
         elevThrsh=_minElevationSet;
         break;
      case ElevRest:
         elevThrsh=_minElevationRest;
         break;
   }

   if(_elevation < elevThrsh) return ElevationBelowThreshold;

   return ElevationOk;
}

SWAState AdSecArbitrator::checkSWA() {    // Perform checks on swing arm
   int swastatus;
   bool swaOK=true;

   try {
      _varSWA->Update();
   } catch (...) {
      swaOK=false;
   }
   if(!swaOK) return SwaNotAvailable;

   _varSWA->Get(&swastatus);
   if(swastatus!=1) return SwaNotDeployed;

   return SwaDeployed;
}

void AdSecArbitrator::putToRest(string reason) {

   _logger->log(Logger::LOG_LEV_TRACE, "putToRest: "+reason);
   if(_simulation) {
      _logger->log(Logger::LOG_LEV_INFO, "SIMULATION: putToRest - no action done");
   } else {
       bool isSetting=false;
       AdSecFsm::States curStat=((AdSecFsm *)_cmdHandler)->currentState();
       Command* lastCmd = this->lastCommand();
       if(lastCmd != NULL)
           if ((lastCmd->getStatus() == Command::CMD_EXECUTING) &&
               (lastCmd->getCode() == AdSecOpCodes::SET_FLAT_AO_CMD))
               isSetting=true;

       if ( isSetting                       ||
            curStat == AdSecFsm::AOSet      || 
            curStat == AdSecFsm::AORunning  || 
            curStat == AdSecFsm::AOPause    ||
            curStat == AdSecFsm::ChopSet    ||
            curStat == AdSecFsm::ChopRunning  ) {
          _logger->log(Logger::LOG_LEV_ERROR, reason);
          ((AdSecFsm *)_cmdHandler)->forceRest();
          _varLed->Set(ADSEC_SAFE);
        }
   }
}


void AdSecArbitrator::controlShellSafety() {  // Verifies wind speed and elevation
                                              // to apply proper safety measures
   double windspeed;
   float step_deg=4.86;   // Chosen to avoid an integer number of steps at 90 degrees of elevation
   struct timeval vtime, now;
                                     // Control wind speed
   _logger->log(Logger::LOG_LEV_TRACE, "Checking windspeed");
   gettimeofday(&now, NULL);
   bool windOK=true;
   try {
      _varWind->Update();
   } catch (...) {
      windOK=false;
   }
   if(windOK) {
      _varWind->Get(&windspeed);
      vtime = _varWind->mTime();
      if (Utils::timediff(&now,&vtime) > EXPIRE_TIME || NaNutils::isNaN(windspeed)) windOK=false;
   }
   
   // update tss status
   getTSSEnabled();

   if(!windOK) {
	   _logger->log(Logger::LOG_LEV_WARNING, "Windspeed not available!");
       if (!_tss_enabled )
           enableTSS(true);
   } else {
      if (_tss_enabled ) {
         if (windspeed<_lowerWindSpeed) {
	          _logger->log(Logger::LOG_LEV_INFO, "Windspeed %fm/s, below %fm/s",windspeed,_lowerWindSpeed);
             enableTSS(false);
         } else {
	          _logger->log(Logger::LOG_LEV_DEBUG, "Windspeed %fm/s, not below %fm/s",windspeed,_lowerWindSpeed);
         }
      } else {
         if (windspeed>_upperWindSpeed) {
	          _logger->log(Logger::LOG_LEV_WARNING, "Windspeed %fm/s, above %fm/s",windspeed,_upperWindSpeed);
             enableTSS(true);
         } else {
	          _logger->log(Logger::LOG_LEV_DEBUG, "Windspeed %fm/s - OK",windspeed);
         }
      }
   }
                                     // Control swing arm status
   bool goOn=true;
   _logger->log(Logger::LOG_LEV_TRACE, "Checking swing arm status");
   switch(checkSWA()) {
      case SwaNotAvailable:
         putToRest("Swing arm status not available! Resting the shell.");
         goOn=false;
         break;
      case SwaNotDeployed:
	      putToRest("Swing arm not deployed! Resting the shell.");
         goOn=false;
         break;
      case SwaDeployed:
         _logger->log(Logger::LOG_LEV_TRACE, "Swing arm - OK.");
         goOn=true;
         break;
   }
   if(!goOn) return;   // No need to control elevation if swingarm not deployed
                                     // Control telescope elevation
   _logger->log(Logger::LOG_LEV_TRACE, "Checking elevation");
   ostringstream o;
   switch(checkElevation(ElevRest)) {
      case ElevationNotAvailable:
         putToRest("Elevation not available! Resting the shell.");
         goOn=false;
         break;
      case ElevationBelowThreshold:
         o << "Elevation (" << _elevation << " deg) below " << _minElevationRest << " Resting the shell.";
	      putToRest(o.str());
         goOn=false;
         break;
      case ElevationOk:
         _logger->log(Logger::LOG_LEV_TRACE, "Elevation %f deg - OK",_elevation);
         goOn=true;
         break;
   }
   if(!goOn) return;   // No need of further controls if elevation is not OK

   int elevation_step=(int)(_elevation/step_deg);
   bool setPiston=false;

   _logger->log(Logger::LOG_LEV_TRACE, "Checking elevation step %d (last %d)",elevation_step,_last_elevation_step);

   if (elevation_step!=_last_elevation_step) { // Check if elevation has changed
          setPiston=true;                      // more that 5 degrees
   }
   if ( ((AdSecFsm *)_cmdHandler)->currentState() == AdSecFsm::PowerOff || 
        ((AdSecFsm *)_cmdHandler)->currentState() == AdSecFsm::Failure  || 
        ((AdSecFsm *)_cmdHandler)->currentState() == AdSecFsm::Panic       )
       setPiston = false;
   if(setPiston) {
        _last_elevation_step=elevation_step;
        setPistonCurrent(_elevation);
   }
}

bool AdSecArbitrator::checkLabMode() {     // Check the labmode variable
    static bool cannot_read = false; // local
    bool isLabMode=false;
    bool maybe;

    try {
        _varLabmode->Update();
        maybe=true;
        cannot_read = false;
    } catch (...) {
        if (cannot_read) {
            // second time in a row that cannot read variable, disable lab mode
            isLabMode = false;
            maybe=false;
        } else
            cannot_read = true;
    }
    
    if(maybe) {
        int labmode;
        _varLabmode->Get(&labmode);
        if(labmode==1) isLabMode = true;
    }
    if(isLabMode != _labmode) {
      _labmode=isLabMode;
      if(_labmode)
	       _logger->log(Logger::LOG_LEV_WARNING, "LabMode ENABLED!");
      else
	       _logger->log(Logger::LOG_LEV_INFO, "LabMode DISABLED!");
   }
   return isLabMode;
}


void AdSecArbitrator::Periodic() {

   static bool first=true;
   static bool skipping_all = false;

   if(!first) {    
       // Skip the check first time, because not everything is properly initialized
       bool labmode=checkLabMode();
       getTSSEnabled(); // update tss status
       
       if(!labmode) 
           controlShellSafety();   // Control all safety aspects of the shell
       else {
           enableTSS(false);       // disable TSS when entering labMode
	       _logger->log(Logger::LOG_LEV_DEBUG, "LabMode: skipping shell safety tests");
       }
   }

   // Check safe skip counter
   int newCounter = ( (FastDiagnSystem*)(_systems["FASTDIAGN"]))->getSafeSkipCounter();
   if (newCounter >= 0) {
       _logger->log(Logger::LOG_LEV_DEBUG, "Safe skip frame counter: %d", newCounter);
       if ((newCounter > _safeSkipCounter) && (!first)) {
           // Skip condition!!!
           _skipping = true;
           _logger->log(Logger::LOG_LEV_WARNING, "Skip frame detected!");
           // [TODO]
       } else {
           _skipping = false;
       }
       _safeSkipCounter = newCounter;
   }

   // Also get skip percentage and stop/restart offload if needed
   _safeSkipPercent = ( (FastDiagnSystem*)(_systems["FASTDIAGN"]))->getSafeSkipPercent();
   if (!NaNutils::isNaN(_safeSkipPercent))
       _varSafeSkipPercent->Set(_safeSkipPercent);
   else
       _varSafeSkipPercent->Set(1.0);

   if (_safeSkipPercent >= 0.99) {
       // not anymore in control, send alert so the aoarbitrator can open the loop
       // check that it is like that at least for two check periods
       if (!skipping_all)
           skipping_all = true;
       else {
           string currentState = ((AdSecFsm *)_cmdHandler)->currentStateAsString();
           if ( currentState.compare("AORunning") == 0 || currentState.compare("AOPause")==0 ) {
               Error error("Maximum skip reached", true);
               notifyClients((Alert*)&error);
           }
       }
   }
   else
       skipping_all = false;

   checkTTOffload();
   
   first = false;
}

void AdSecArbitrator::serviceRequest(Request* req, MsgBuf* /* requestMsgBuf */) {

   switch(req->getCode()) {
      case AdSecOpCodes::REQUEST_ADSEC_STATUS_CMD:

         RequestAdSecStatus* statusReq = (RequestAdSecStatus*)req;
         statusReq->setArbitratorStatus(this->fsmState());
         Command* lastCmd = this->lastCommand();
         if(lastCmd != NULL) {
            statusReq->setLastCommandDescription(lastCmd->getDescription());
            statusReq->setLastCommandResult(lastCmd->getStatusAsString());
         }

         statusReq->setClStatReady( this->clStatOK());

         statusReq->setDone();
         statusReq->setSkipping( _safeSkipCounter, _skipping, _safeSkipPercent);
         statusReq->setCoilsEnabled(getCoilsEnabled());
         statusReq->setTssEnabled(getTSSEnabled());
         statusReq->setTipOffload( ((FastDiagnSystem*)(_systems["FASTDIAGN"]))->getTipOffload());
         statusReq->setTiltOffload( ((FastDiagnSystem*)(_systems["FASTDIAGN"]))->getTiltOffload());
         statusReq->setFocusOffload( ((FastDiagnSystem*)(_systems["FASTDIAGN"]))->getFocusOffload());
      break;

   }

}

inline string AdSecArbitrator::fsmState() {
     return ((AdSecFsm *)_cmdHandler)->currentStateAsString();
}


void AdSecArbitrator::processAlert(Alert* alert) {
	static Logger* logger = Logger::get("ALERTHANDLER");

    //logger->log(Logger::LOG_LEV_INFO, "Processing alert %d (%s)...", alert->getCode(), alert->getShortDescription().c_str());
	IdlSystem* idl; 
	applyCommandParams param;
	string currentState;
	int doOffloadHighOrder;


   if(_simulation) {
        logger->log(Logger::LOG_LEV_INFO, "Processing alert %d (%s) from %s..", alert->getCode(), alert->getShortDescription().c_str(), alert->getSender().c_str());
		logger->log(Logger::LOG_LEV_INFO, "SIMULATION: doing nothing. '%s' with message %s", alert->getShortDescription().c_str(), alert->getTextMessage().c_str());
   } else {
	   switch(alert->getCode()) {
	    case BasicOpCodes::WARNING:
		   logger->log(Logger::LOG_LEV_WARNING, "Nothing to do for a '%s' with message %s from %s", alert->getShortDescription().c_str(), alert->getTextMessage().c_str(), alert->getSender().c_str());
		   // [Todo] something?
		   break;

	    case BasicOpCodes::ERROR:
		   logger->log(Logger::LOG_LEV_ERROR, "Received an ERROR alert! Forcing Failure state.");
		   logger->log(Logger::LOG_LEV_ERROR, "Alert message: %s from %s", alert->getTextMessage().c_str(), alert->getSender().c_str());
         currentState = ((AdSecFsm *)_cmdHandler)->currentStateAsString();
         if ( currentState.compare("Panic") == 0 ) {
            break;
         }
         else {
            if (((Error*)alert)->requestRip()) {
		         ((AdSecFsm*)_cmdHandler)->forceRest();
            }
            _varLed->Set(ADSEC_SAFE);
		      ((AdSecFsm*)_cmdHandler)->forceState(AdSecFsm::Failure);
   //		logger->log(Logger::LOG_LEV_ERROR, "Nothing to do for a '%s' with message '%s' :-(", alert->getShortDescription().c_str(), alert->getTextMessage().c_str());

		   // TODO: Cancel the current command (always needed?)
		   // logger->log(Logger::LOG_LEV_WARNING, "Canceling the current command...");
		   // _cmdHandler->doCancel();
		   // [todo] ...
		      break;
         }

         // In case of PANIC notification:
         // 1) disable coils
         // 2) Go to Panic state
	    case BasicOpCodes::PANIC:
		   logger->log(Logger::LOG_LEV_FATAL, "Received a PANIC alert! Forcing Panic state.");
		   logger->log(Logger::LOG_LEV_FATAL, "Alert message: %s from %s", alert->getTextMessage().c_str(), alert->getSender().c_str());
		   ((AdSecFsm*)_cmdHandler)->forceRest();
		   _varLed->Set(ADSEC_SAFE);
		   ((AdSecFsm*)_cmdHandler)->forceState(AdSecFsm::Panic);
		   break;

	    case BasicOpCodes::HOOFFLOAD:
         if (!varDoOffloadHighOrder()) {
            doOffloadHighOrder=false;
         } else {
            varDoOffloadHighOrder()->Get(&doOffloadHighOrder);
         }
         currentState = ((AdSecFsm *)_cmdHandler)->currentStateAsString();
         if ( currentState.compare("Panic") == 0 ) break;
         if (doOffloadHighOrder) {
		      logger->log(Logger::LOG_LEV_INFO, "Received an High Order Offload %s", ((HighOrderOffload*)alert)->getHighOrderOffloadFilename().c_str() );
		       // CALL IDL WHEN NEEDED
		       if ( currentState.compare("AOSet") == 0 || currentState.compare("AORun")==0 ){
                  _varHOOffloadTime->Set(Utils::timeAsIsoString()); // set timestamp on rtdb
			      param.commandsFile = ((HighOrderOffload*)alert)->getHighOrderOffloadFilename();
			      idl = (IdlSystem*)(_systems["IDL"]);
			      idl->HOOffload(AdSecArbitrator::HIGHORDEROFFLOAD_TIMEOUT, param);
		       }
		   }
         break;

       case BasicOpCodes::DUMPSAVED:
		      logger->log(Logger::LOG_LEV_INFO, "Received from %s a DumpSaved alert with filename %s", alert->getSender().c_str(), ((DumpSaved*)alert)->filename().c_str() );
             idl = (IdlSystem*)(_systems["IDL"]);
             
             // AP 20161116 Removed because it stops command processing for a long time and everything times out

             //idl->processDump(AdSecArbitrator::PROCESSDUMP_TIMEOUT, ((DumpSaved*)alert)->filename());
             break;

       default:
          _logger->log(Logger::LOG_LEV_ERROR, "Unknown Alert opcode %d received from %s", alert->getCode(), alert->getSender().c_str());
       break;
	   }

   }

	// Notify all registered clients
    notifyClients(alert);
}

void AdSecArbitrator::setPistonCurrent(double elevation){

    _logger->log(Logger::LOG_LEV_INFO, "Setting piston current for elevation %f degrees",elevation);
    if(_simulation) {
        _logger->log(Logger::LOG_LEV_INFO, "SIMULATION: setPistonCurrent - no action done.");
    } else {
        IdlSystem* idl = (IdlSystem*)(_systems["IDL"]);
    
        idl->setPistonCurrent(elevation, SETPISTCURR_TIMEOUT);
    }
}

void AdSecArbitrator::enableTSS(bool enable, bool force)
{
    if(_simulation) {
       string senable="disable";
       string sforce="no force";
       if(enable) senable="enable";
       if(force) sforce="force";

       _tss_enabled=enable;
       _logger->log(Logger::LOG_LEV_INFO, "SIMULATION: enableTSS(%s,%s) - no action done",senable.c_str(),sforce.c_str());
    } else {
       if(enable) {
          if(! _tss_enabled || force ) {
           _logger->log(Logger::LOG_LEV_INFO, "Enabling TSS");
           ((AdamSystem*)_systems[ADAMSYSTEM_NAME])->enableTss();
          }
          _tss_enabled=true;
       } else {
           if(_tss_enabled || force ) {
               _logger->log(Logger::LOG_LEV_INFO, "Disabling TSS");
               ((AdamSystem*)_systems[ADAMSYSTEM_NAME])->disableTss();
           }
           _tss_enabled=false;
       }
    }
}

bool AdSecArbitrator::isGuiding()
{
    int guiding;
    try {
        _varGuiding->Update();
        _varGuiding->Get(&guiding);
    } catch (...) {
        return false;
    }  
    return (guiding == 1);
}

/* L.F. removed
 
void AdSecArbitrator::controlTSS(bool force){

    bool ontss=false;
    double wspeed;

    if (!varWind()) {

      wspeed= NaNutils::dNaN();  

    } else {

      struct timeval now;
      varWind()->Get(&wspeed);
      windTime = varWind()->mTime();
      gettimeofday(&now, NULL);
      if ((now.tv_sec - windTime.tv_sec) > EXPIRE_TIME || NaNutils::isNaN(wspeed)) 
         ontss = true;
      else {

         if (_last_tss) {
            ontss = (wspeed>(_maxWindSpeed-_hystWindSpeed));
         }
         else {
            ontss = (wspeed>_maxWindSpeed);
         }

      }
         
    }
	 
    _logger->log(Logger::LOG_LEV_INFO, "Windspeed is %f, hysteresis is %f, threshold is %f", wspeed, _hystWindSpeed, _maxWindSpeed);


//#ifdef USE_ADAM

    //if ((ontss != _last_tss) || force )
    if (force) {
        _adam->enableTss();
         _last_tss = true;
    }
    else {
       if ( ontss ){
           if (!_last_tss) _adam->enableTss();
           _last_tss = true;
       }
       else {
           if (_last_tss) _adam->disableTss();
           _last_tss = false;
       }
    }
//#else
//    bool on = (ontss != _last_tss) || force;
//    idl->setTSS(on, SETTSS_TIMEOUT);
//#endif

    //_last_tss=ontss;
}
  end of remove */
