//@File: AOArbirator.cpp
//
//@
#include <algorithm>

#include "aoarbitrator/AOArbitrator.h"
#include "aoarbitrator/AOCommandImplFactory.h"
#include "aoarbitrator/WfsArbSystem.h"
#include "aoarbitrator/AdsecArbSystem.h"
#include "aoarbitrator/AOArbFsm.h"
#include "arblib/base/Alerts.h"
#include "arblib/base/Commands.h"

using namespace Arcetri::Arbitrator;


AOArbitrator::AOArbitrator( int argc, char **argv):AbstractArbitrator( argc, argv),
                                                   _activeWFS(NULL),
                                                   _FLAOwfsStateVar(NULL),
                                                   _MAGwfsStateVar(NULL),
                                                   _LBTIwfsStateVar(NULL),
                                                   _adsecStateVar(NULL),
                                                   _activeWfsStateVar(NULL)
{
   _adsecWasConnected=false;
   _adsecConnected=false;
   _FLAOwfsConnected=false;
   _MAGwfsConnected=false;
   _LBTIwfsConnected=false;

   create();
}

void AOArbitrator::create() {

    RequireKeyword("MaxOffsetXY");
    RequireKeyword("MaxOffsetZ");
    RequireKeyword("SplitOffsetXY");
    RequireKeyword("SplitOffsetZ");
    RequireKeyword("FLAOWFSLogLev");
    RequireKeyword("MAGWFSLogLev");
    RequireKeyword("LBTIWFSLogLev");
    RequireKeyword("ADSECLogLev");
    CheckKeywords(true, true, true);

	// Set logger levels for specific components: the levels are got
	// from config file
	setLoggerLevel(AO_WFS_FLAO);
	setLoggerLevel(AO_WFS_MAG);
	setLoggerLevel(AO_WFS_LBTI);
	setLoggerLevel(AO_ADSEC_FLAO);

    _largeOffset = false;
    _adcTracking = true;

	// --- Intialize the AbstractArbitrator with specific modules ---//

	// Create a CommandImplFactory, able to implement AO commands
	_cmdImplFactory = new AOCommandImplFactory();

	// Create the AOArbFsm (with the given CommandImplFactory), able
	// to process AO commands
	_cmdHandler = new AOArbFsm(_commandHistorySize, MyName());

}

AOArbitrator::~AOArbitrator() {
       _aoServStat.Set(0);

       if (_FLAOwfsStateVar != NULL) delete _FLAOwfsStateVar;
       if (_MAGwfsStateVar != NULL) delete _MAGwfsStateVar;
       if (_LBTIwfsStateVar != NULL) delete _LBTIwfsStateVar;
       if (_adsecStateVar != NULL) delete _adsecStateVar;
}

AOModeType AOArbitrator::getAOMode() {
    _aoMode.Update();
    string modeStr = _aoMode.Get();
	return AOMode::fromString(modeStr);
}

void AOArbitrator::setAOMode(AOModeType mode) {
   string modeStr = AOMode::toString(mode);
	_aoMode.Set(modeStr.c_str());
	_logger->log(Logger::LOG_LEV_INFO, "AOSYSTEM mode: %s\n", modeStr.c_str());
// printf("AOSYSTEM mode: %s\n", modeStr.c_str());
}

void AOArbitrator::setActiveWFS(string wfsSpec) {

   if(wfsSpec==AO_WFS_FLAO)
       _activeWfsStateVar=_FLAOwfsStateVar;
   if(wfsSpec==AO_WFS_MAG)
       _activeWfsStateVar=_MAGwfsStateVar;
   else if(wfsSpec==AO_WFS_LBTI)
       _activeWfsStateVar=_LBTIwfsStateVar;
   else {
       _activeWfsStateVar=NULL;
	   _logger->log(Logger::LOG_LEV_WARNING, "No such WFS: %s in setActiveWFS",wfsSpec.c_str());
       return;
   }
   _wfsSource.Set(wfsSpec);
   _activeWFS=(WfsArbSystem *)_systems[wfsSpec];
   
   _logger->log(Logger::LOG_LEV_INFO, "Active WFS now: %s\n", wfsSpec.c_str());
}



void AOArbitrator::serviceRequest(Request* req, MsgBuf* /* requestMsgBuf */) {

	static Logger* logger = Logger::get("REQUESTHANDLER");

	// Check the type of request
	switch(req->getCode()) {
		case AOOpCodes::ADJUST_GAIN: {
			logger->log(Logger::LOG_LEV_INFO, "Received  AdjustGain request");
			// TODO
			req->setCompleted(true);
			break;
		}
		case AOOpCodes::ADJUST_INT_TIME: {
			logger->log(Logger::LOG_LEV_INFO, "Received  AdjustIntTime request");
			// TODO
			req->setCompleted(true);
			break;
		}

      case AOOpCodes::REQUEST_AO_STATUS_CMD: {
         RequestAOStatus *statusReq = (RequestAOStatus*)req;
         statusReq->setArbitratorStatus(this->fsmState());
         Command* lastCmd = this->lastCommand();
         if(lastCmd != NULL) {
            statusReq->setLastCommandDescription(lastCmd->getDescription());
            statusReq->setLastCommandResult(lastCmd->getStatusAsString());
         }
         statusReq->setAOMode( AOMode::toString( getAOMode()));


         if (_activeWFS != NULL) {
            statusReq->setWfsPower( _activeWFS->isPowerOn());
            statusReq->setWfsStatus( _activeWFS->fsmState());
            statusReq->setWfsClStatReady( _activeWFS->getClStatReady());
         } else {
             statusReq->setWfsPower( false);
             statusReq->setWfsStatus( "Disconnected");
             statusReq->setWfsClStatReady( false);
         }


         AdsecArbSystem *adsec = (AdsecArbSystem *)(_systems[AO_ADSEC_FLAO]);
         if (_adsecConnected) {
            statusReq->setAdSecPower( adsec->isPowerOn());
            statusReq->setAdSecSet( adsec->isSet());
            statusReq->setAdsecStatus( adsec->getStatus());
            statusReq->setAdSecClStatReady( adsec->getClStatReady());
         } else {
            statusReq->setAdSecPower( false);
            statusReq->setAdSecSet( false);
            statusReq->setAdsecStatus( "Disconnected");
            statusReq->setAdSecClStatReady( false);
         }

         statusReq->setDone();
         break;
      }

		default:
			logger->log(Logger::LOG_LEV_WARNING, "Unexpected request (code: %)", req->getCode());

	}

	// --- Send back the request as reply (the "completion status" is now changed!)
//	int ret = sendReply(req, requestMsgBuf, logger); // This also release msgBuf with thReplyMsg
//	delete req;
}


void AOArbitrator::InstallHandlers() {
	// TODO Add handlers here if needed
    AbstractArbitrator::InstallHandlers();

}
/*
inline string AOArbitrator::fsmState() {
        return ((AOArbFsm *)_cmdHandler)->currentStateAsString();
}
*/

ConnectionStatus AOArbitrator::updateAdsecConnectionStatus() {
   AdsecArbSystem *adsec = (AdsecArbSystem *)_systems[AO_ADSEC_FLAO];
   ConnectionStatus stat;
   static struct timeval lastStartTime = {0L, 0L};
   static int skipAlert=0;

   bool adsecIsConnected = adsec->sync(); 

   if (adsecIsConnected) {
      if (adsec->getSafeSkipPercent() > 0.50) {
         if (skipAlert==0) {
            doSpeak("Voice_Skipping");
            skipAlert=1;
         }
      } else skipAlert=0;
   }


   if (adsecIsConnected) {
       if (!_adsecWasConnected) {
           Logger::get()->log(Logger::LOG_LEV_INFO, "ADSEC subsystem connected");
           adsec->FeedbackRequest();
           stat=JustConnected;
           adsec->getArbStartTime(lastStartTime);
       } else {
           struct timeval startTime;
           adsec->getArbStartTime(startTime);
           if ((startTime.tv_sec != lastStartTime.tv_sec) || (startTime.tv_usec != lastStartTime.tv_usec)) {
               Logger::get()->log(Logger::LOG_LEV_INFO, "ADSEC subsystem restarted");
               adsec->FeedbackRequest();
               stat=JustConnected;
               lastStartTime = startTime;
           }
           else {
               stat=IsConnected;
           }
       }
       _adsecWasConnected=true;
   } else {
       if (_adsecWasConnected) {
           Logger::get()->log(Logger::LOG_LEV_INFO, "ADSEC subsystem disconnected");
           stat=JustDisconnected;
       } else
           stat=IsDisconnected;
       _adsecWasConnected = false;
   }
   _adsecHealth.Set( adsec->getClStatReady() ? 1 : 0);

   return stat;
}


void AOArbitrator::Periodic() {
   WfsArbSystem *wfs;

   
   switch(updateAdsecConnectionStatus()) {
      case JustConnected: 
         eNotify(*_adsecStateVar, StateChange, true);
         _adsecConnected=true;
         break;
      case IsConnected:
         _adsecConnected=true;
         break;
      default:
         _adsecConnected=false;
   }

   wfs = (WfsArbSystem*)(_systems[AO_WFS_FLAO]);
   switch(wfs->updateConnectionStatus()) {
      case JustConnected:
         if (_activeWfsStateVar == NULL)
             setActiveWFS(AO_WFS_FLAO);
         eNotify(*_FLAOwfsStateVar,StateChange, true);
         _FLAOwfsConnected=true;
         break;
      case IsConnected:
         _FLAOwfsConnected=true;
         break;
      default:
         _FLAOwfsConnected=false;
   }

   wfs = (WfsArbSystem*)(_systems[AO_WFS_LBTI]);
   switch(wfs->updateConnectionStatus()) {
      case JustConnected:
         if (_activeWfsStateVar == NULL)
             setActiveWFS(AO_WFS_LBTI);
         eNotify(*_LBTIwfsStateVar,StateChange, true);
         _LBTIwfsConnected=true;
         break;
      case IsConnected:
         _LBTIwfsConnected=true;
         break;
      default:
         _LBTIwfsConnected=false;
   }

   wfs = (WfsArbSystem*)(_systems[AO_WFS_MAG]);
   switch(wfs->updateConnectionStatus()) {
      case JustConnected:
         if (_activeWfsStateVar == NULL)
             setActiveWFS(AO_WFS_MAG);
         eNotify(*_MAGwfsStateVar,StateChange, true);
         _MAGwfsConnected=true;
         break;
      case IsConnected:
         _MAGwfsConnected=true;
         break;
      default:
         _MAGwfsConnected=false;
   }
}



void AOArbitrator::SetupVars() {
	AbstractArbitrator::SetupVars();

	_logger->log(Logger::LOG_LEV_DEBUG, "AOArbitrator setting up RTDB vars...");

   /*    Fatto piu' sotto e senza usare il conmfigdictionary
   string adsec_statevar = (string)ConfigDictionary()[AOARBITRATOR_ADSEC_STATEVAR];

   size_t p;
   if ((p = adsec_statevar.find("*")) != string::npos)
      adsec_statevar[p] = Side()[0];
   try {
      _adsecStateVar = new RTDBvar(adsec_statevar, CHAR_VARIABLE, 50);
   } catch (AOException &e) {
      _adsecStateVar = NULL;
   }
   */

	try {

		// 'AO' variables
		_aoMode      = RTDBvar(MyFullName()+".MODE", CHAR_VARIABLE, 50);
		_aoServStat  = RTDBvar(MyFullName()+".ServStat", INT_VARIABLE, 1);
		_wfsSource   = RTDBvar(MyFullName()+".WFS_SOURCE", CHAR_VARIABLE, 50);
		_aoLabMode   = RTDBvar(MyFullName()+".LAB_MODE", INT_VARIABLE, 1);
//_wfsCLstat   = RTDBvar(MyFullName()+".WFS.HEALTH", INT_VARIABLE, 1);       // Got from M_WFS
		_adsecHealth = RTDBvar(MyFullName()+".ADSEC.HEALTH", INT_VARIABLE, 1);
        _loopOn      = RTDBvar(MyFullName()+".LOOPON", INT_VARIABLE, 1);
        _stopCmd     = RTDBvar(MyFullName()+".STOPCMD", INT_VARIABLE, 1);
        clearStopCmd();


      // AOLoop params
      _aoparamNModes  = RTDBvar( MyFullName()+".CORRECTEDMODES", INT_VARIABLE);
      _aoparamStrehl =  RTDBvar( MyFullName()+".STREHL", REAL_VARIABLE);

      // Alert message variables
      _aoMsg       = RTDBvar(MyFullName()+".MSG", CHAR_VARIABLE, 50);
//    _aoMsgLev    = RTDBvar(MyFullName()+".MSGLEV", INT_VARIABLE);

      // substate string
      _fsmSubStateVar = RTDBvar( MyFullName()+".FSM_SUBSTATE", CHAR_VARIABLE, 50);
      _fsmSubStateVar.Set("");

      _SLReady = RTDBvar( MyFullName()+".SL_READY", INT_VARIABLE);
      _SLReady.Set(0);
      _AOReady = RTDBvar( MyFullName()+".AO_READY", INT_VARIABLE);
      _AOReady.Set(0);

      _clearAOI = RTDBvar("MagAOI.cmds.clear", INT_VARIABLE, 1);
      _clearAOI.Set(1);

      _aoServStat.Set(1);    // Enable control from AOS

	}
	catch (AOVarException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, "SetupVars: %s", e.what().c_str());
		throw ArbitratorException("Error while setting up RTDB vars", ARB_INIT_ERROR);
	}
	_logger->log(Logger::LOG_LEV_DEBUG, "AOArbitrator RTDB vars setup done.");
}

void AOArbitrator::PostInit() {
	AbstractArbitrator::PostInit();

   bool simulation=isSimulation();

	if(!TimeToDie()) {
		_logger->log(Logger::LOG_LEV_DEBUG, "AOArbitrator PostInit checks and initializations...");

      string msgdId;
      string stVarName;

		_systems[AO_WFS_FLAO] = new WfsArbSystem(string(AO_WFS_FLAO),simulation);
		_systems[AO_WFS_LBTI] = new WfsArbSystem(string(AO_WFS_LBTI),simulation);
		_systems[AO_WFS_MAG] = new WfsArbSystem(string(AO_WFS_MAG),simulation);
		_systems[AO_ADSEC_FLAO] = new AdsecArbSystem(simulation);

		// Install interfaces for sub-arbitrators
      msgdId = ((WfsArbSystem *)_systems[AO_WFS_FLAO])->getMsgDIdent();
		addArbitratorInterface(msgdId);
      ((WfsArbSystem *)_systems[AO_WFS_FLAO])->setArbInterface(getArbitratorInterface(msgdId));
      stVarName="wfsarb."+Side()+".FSM_STATE@"+((WfsArbSystem *)_systems[AO_WFS_FLAO])->getDomain();
      _FLAOwfsStateVar =  new RTDBvar(stVarName,CHAR_VARIABLE,50,false);

      msgdId = ((WfsArbSystem *)_systems[AO_WFS_LBTI])->getMsgDIdent();
		addArbitratorInterface(msgdId);
      ((WfsArbSystem *)_systems[AO_WFS_LBTI])->setArbInterface(getArbitratorInterface(msgdId));
      stVarName="wfsarb."+Side()+".FSM_STATE@"+((WfsArbSystem *)_systems[AO_WFS_LBTI])->getDomain();
      _LBTIwfsStateVar =  new RTDBvar(stVarName,CHAR_VARIABLE,50,false);

      msgdId = ((WfsArbSystem *)_systems[AO_WFS_MAG])->getMsgDIdent();
		addArbitratorInterface(msgdId);
      ((WfsArbSystem *)_systems[AO_WFS_MAG])->setArbInterface(getArbitratorInterface(msgdId));
      stVarName="wfsarb."+Side()+".FSM_STATE@"+((WfsArbSystem *)_systems[AO_WFS_MAG])->getDomain();
      _MAGwfsStateVar =  new RTDBvar(stVarName,CHAR_VARIABLE,50,false);

      msgdId = ((AdsecArbSystem *)_systems[AO_ADSEC_FLAO])->getMsgDIdent();
		addArbitratorInterface(msgdId);
      ((AdsecArbSystem *)_systems[AO_ADSEC_FLAO])->setArbInterface(getArbitratorInterface(msgdId));
      _adsecStateVar = new RTDBvar("adsecarb."+Side()+".FSM_STATE",CHAR_VARIABLE,50,false);
        
      // read wfs states
      statesWfs = ConfigDictionary()[AOARBITRATOR_STATESWFS_STR];
      statesAdsec = ConfigDictionary()[AOARBITRATOR_STATESADSEC_STR];

      // read states sync configuration
      _statesSyncMap.clear();
      for (Config_File::iterator it = ConfigDictionary().begin(); it != ConfigDictionary().end(); it++) {
          if (it->first.find(AOARBITRATOR_STATESSYNC_STR) == 0) {
              AOArbStatesSyncStruct syncStruct;
              string state = it->first.substr( it->first.rfind(".")+1 );;
              vector<string> params = it->second;
              vector<string>::iterator wfs_from, wfs_to, adsec_from, adsec_to;

              Logger::get()->log(Logger::LOG_LEV_DEBUG,"checking [%s][%s] -> %s (%s)",
                                 params[0].c_str(), params[1].c_str(), state.c_str(), params[2].c_str());

              // check for wfs states range
              if (params[0][0] == '<') {
                  wfs_from = statesWfs.begin();
                  wfs_to = find(statesWfs.begin(), statesWfs.end(), params[0].substr(1))-1;
              }
              else if (params[0][0] == '>') {
                  wfs_from = find(statesWfs.begin(), statesWfs.end(), params[0].substr(1))+1;
                  wfs_to = statesWfs.end()-1;
              }
              else {
                  wfs_from = wfs_to = find(statesWfs.begin(), statesWfs.end(), params[0]);
              }

              // check for adsec range
              if (params[1][0] == '<') {
                  adsec_from = statesAdsec.begin();
                  adsec_to = find(statesAdsec.begin(), statesAdsec.end(), params[1].substr(1))-1;
              }
              else if (params[1][0] == '>') {
                  adsec_from = find(statesAdsec.begin(), statesAdsec.end(), params[1].substr(1))+1;
                  adsec_to = statesAdsec.end()-1;
              }
              else {
                  adsec_from = adsec_to = find(statesAdsec.begin(), statesAdsec.end(), params[1]);
              }
              // fill in states map table
              Logger::get()->log(Logger::LOG_LEV_INFO,"States sync [%s .. %s][%s .. %s] -> %s (%s)",
                                 wfs_from->c_str(), wfs_to->c_str(), adsec_from->c_str(), adsec_to->c_str(),
                                 state.c_str(), params[2].c_str());
              for ( vector<string>::iterator wfs = wfs_from; wfs <= wfs_to; wfs++ ) {
                  for ( vector<string>::iterator adsec = adsec_from; adsec <= adsec_to; adsec++ ) {
                      string states_couple = *wfs + "+" + *adsec;
                      Logger::get()->log(Logger::LOG_LEV_DEBUG,"Adding states sync rule %s -> %s (%s)",
                                         states_couple.c_str(), state.c_str(), params[2].c_str());
                      _statesSyncMap[states_couple].state = state;
                      _statesSyncMap[states_couple].substate = params[2];
                  }
              }
          }
      }
      
      // check if the current subsytem is stored in the database
      _wfsSource.Update();
      if (_wfsSource.Get().compare("") != 0)
          setActiveWFS(_wfsSource.Get());

      _logger->log(Logger::LOG_LEV_DEBUG, "AOArbitrator PostInit checks and initializations done.");
      Periodic();
   }
}

// Updates AO arbitrator status. Must be called when either the
// AdSec or the Wfs status changes

int AOArbitrator::updateStatus() {

 
    vector<string>::iterator _wfs = statesWfs.begin();
    vector<string>::iterator _adsec = statesAdsec.begin();
       
    try {
        _adsecStateVar->Update();             // Update adsec status variable
    } catch(AOVarException) {
        _logger->log(Logger::LOG_LEV_WARNING, "Error reading AdSec state variable. Cannot update status");
        return 0;
    }

    if ( _activeWfsStateVar ) {
        try {
            _activeWfsStateVar->Update();      // Update wfs status variable
        } catch(AOVarException) {
            _logger->log(Logger::LOG_LEV_WARNING, "Error reading WFS state variable. Cannot update status");
            return 0;
        }
    
        string statesPair = _activeWfsStateVar->Get() + "+" + _adsecStateVar->Get();

        if (_statesSyncMap.find(statesPair) == _statesSyncMap.end()) {
            // states pair not found, do nothing
            _logger->log(Logger::LOG_LEV_WARNING, "States pair not found for synchronization (%s)", statesPair.c_str());
        }
        else {
            string new_state = _statesSyncMap[statesPair].state;
            string new_substate = _statesSyncMap[statesPair].substate;

            _logger->log(Logger::LOG_LEV_DEBUG,"States sync: [%s] -> %s (%s)", statesPair.c_str(), new_state.c_str(), new_substate.c_str());
            AOArbFsmCore::States newstate = ((AOArbFsm*)_cmdHandler)->stateCode(new_state);
        
            if (newstate == AOArbFsmCore::_Unknown) {
                _logger->log(Logger::LOG_LEV_WARNING,"Unknown new state, no state change (%s)", new_state.c_str());
            }
            else {
                if (_busy) {
                    _logger->log(Logger::LOG_LEV_DEBUG,"Arbitrator busy, no state change (%s)",  new_state.c_str());
                }
                else {
                    if (((AOArbFsm*)_cmdHandler)->currentState() != newstate) {
                        _logger->log(Logger::LOG_LEV_INFO,"Updating state [%s] -> %s/%s", 
                                     statesPair.c_str(), new_state.c_str(), new_substate.c_str());
                        ((AOArbFsm*)_cmdHandler)->forceState(newstate);
                        _fsmStateVar->Set( fsmState());
                        _fsmSubStateVar.Set(new_substate);
                    }
                }
                if ( _fsmSubStateVar.Get() != new_substate) {
                    _logger->log(Logger::LOG_LEV_INFO,"Updating substate [%s] -> %s/%s",
                                 statesPair.c_str(), new_state.c_str(), new_substate.c_str());
                    _fsmSubStateVar.Set(new_substate);
                }
            }
        }
        _wfs = find(statesWfs.begin(), statesWfs.end(), _activeWfsStateVar->Get());
    }
    _adsec = find(statesAdsec.begin(), statesAdsec.end(), _adsecStateVar->Get());
        
    // set SLReady/AOReady
    if (_adsec >= find(statesAdsec.begin(), statesAdsec.end(), "AOSet")) {
        _SLReady.Set(1);
        if (_wfs >= find(statesWfs.begin(), statesWfs.end(), "Operating")) {
            _AOReady.Set(1);
        }
        else {
            _AOReady.Set(0);
        }
    }
    else {
        _SLReady.Set(0);
        _AOReady.Set(0);
    }
    return 0;
}


// Variable change notification handler
//
int AOArbitrator::StateChange( void *_this, Variable *var) {

    AOArbitrator * me = (AOArbitrator *)_this;

    _logger->log(Logger::LOG_LEV_DEBUG,"Variable notification callback (%s)", var->H.Name); 

    me->updateStatus();

    return 0;
}

void AOArbitrator::updateRtdb( checkRefAOResult * /* params */) { 

   try {
       // [nothing do to]
   } catch (AOException &e) {
		_logger->log(Logger::LOG_LEV_ERROR, "Exception updateRtdb(): %s", e.what().c_str());
   }

}

void AOArbitrator::updateRtdb( presetFlatParams * /* params */) {

   try {
       //[nothing to do]
      //_adsecShape.Set( params->flatSpec);
   } catch (AOException &e) {
		_logger->log(Logger::LOG_LEV_ERROR, "Exception updateRtdb(): %s", e.what().c_str());
   }
}



void AOArbitrator::updateRtdb( Wfs_Arbitrator::prepareAcquireRefResult *params) {

   ostringstream f1,f2;
   f1 << "Filter " << params->Fw1Pos;
   f2 << "Filter " << params->Fw2Pos;

   try {

      _aoparamNModes.Set( params->nModes); 

      // [no more to do]

      //_aoparamFilter1.Set(f1.str()); 
      //_aoparamFilter2.Set(f2.str()); 
      //_aoparamFreq.Set( params->freq);
      //_aoparamNBins.Set( params->binning);
      //_aoparamTTmod.Set( params->TTmod);

   // [TODO]

      //_aoparamR0.Set(0.0);
      //_aoparamSnMode.Set(0.0);
      //_aoparamStrehl.Set(0.0);
   } catch (AOException &e) {
		_logger->log(Logger::LOG_LEV_ERROR, "Exception updateRtdb(): %s", e.what().c_str());
   }
}

void AOArbitrator::updateRtdb( acquireRefAOResult *params) {

   try {
      _aoparamNModes.Set( params->nModes); 
      _aoparamStrehl.Set( params->strehl);

      //_aoparamFilter1.Set( params->f1spec); 
      //_aoparamFilter2.Set( params->f2spec);
      //_aoparamFreq.Set( params->freq);
      //_aoparamNBins.Set( params->nBins);
      //_aoparamTTmod.Set( params->ttMod);
      //_aoparamR0.Set( params->r0);

   // [TODO]
      //_aoparamSnMode.Set(0.0);
   } catch (AOException &e) {
		_logger->log(Logger::LOG_LEV_ERROR, "Exception updateRtdb(): %s", e.what().c_str());
   }
}



// Writes error message from Alert to TCS variables.

void AOArbitrator::TCSNotify(Alert *alert) {

    int level;

    switch(alert->getCode()) 
        {
        case BasicOpCodes::WARNING:
            level = MASTER_LOG_LEV_WARNING;
            break;
        case BasicOpCodes::ERROR:
            level = MASTER_LOG_LEV_ERROR;
            break;
        case BasicOpCodes::PANIC:
            level = MASTER_LOG_LEV_FATAL;
            break;
        default:
            level = MASTER_LOG_LEV_INFO;
        }
    
    char msg[50];

    snprintf(msg, 50, "Alert from %s: %s - %s", alert->getSender().c_str(),
             alert->getShortDescription().c_str(), alert->getTextMessage().c_str());

    TCSNotify(alert->getTextMessage(), level);
}

void AOArbitrator::TCSNotify(string msg, int level) {
    
    ostringstream _msg;
    static Logger* logger = Logger::get("ALERTHANDLER", Logger::LOG_LEV_INFO);

    _msg << level << ':' << msg;
    _aoMsg.Set(_msg.str());
    
    logger->log(level, msg);
}

void AOArbitrator::toggleLargeOffset() {
    _largeOffset = !_largeOffset;
}

bool AOArbitrator::largeOffset() {
    return _largeOffset;
}

void AOArbitrator::processAlert(Alert* alert) {
	static Logger* logger = Logger::get("ALERTHANDLER", Logger::LOG_LEV_INFO);
    AdsecArbSystem *adsec = (AdsecArbSystem *)_systems[AO_ADSEC_FLAO];   
    WfsArbSystem *wfs = _activeWFS;
    AbstractSystemCommandResult res = SysCmdSuccess;
    AbstractSystemCommandResult res2 = SysCmdSuccess;
    string sender = alert->getSender();
    string desc = alert->getShortDescription();
    string message = alert->getTextMessage();
    string state = ((AOArbFsm *)_cmdHandler)->currentStateAsString();

    logger->log(Logger::LOG_LEV_DEBUG, "Processing alert %d (%s) from %s: %s", 
                alert->getCode(), desc.c_str(), sender.c_str(), message.c_str());

    switch(alert->getCode()) {
    case BasicOpCodes::WARNING:
        TCSNotify(alert);
        break;

    case BasicOpCodes::ERROR:
        TCSNotify(alert);
        setStopFlag(true);
        try {
           speak( (std::string) ConfigDictionary()["Voice_LoopOpen"]);
        } catch(...) {}
    // If WFS registers an error, open the loop on AdSec side.
        if ((state == "LoopClosed") || (state == "LoopSuspended")) {

            //adsec->enableTTOffload(false);
            
            TCSNotify("Stopping WFS...");
            wfs->stop();
            msleep(100);
            res = wfs->getCommandResult();
            if (res == SysCmdSuccess)
                logger->log(Logger::LOG_LEV_INFO, "WFS successfuly stopped");
            else
                logger->log(Logger::LOG_LEV_ERROR, "Problem stopping WFS!");
        }
            
        adsec->sync();
        if ((sender == adsec->getMsgDIdent()) && (adsec->getStatus() == "Failure")) {
            // do a recover if adsec is in failure and sent the alert
            try {
               speak( (std::string) ConfigDictionary()["Voice_ShellRip"]);
            } catch(...) {}
            //TCSNotify("Recovering AdSec failure");
            //adsec->recoverFailure();
            adsec->mirrorRest();
            res2 = adsec->getCommandResult();
            if (res2 == SysCmdSuccess) 
                logger->log(Logger::LOG_LEV_INFO, "AdSec successfuly recovered");
            else
                logger->log(Logger::LOG_LEV_ERROR, "Cannot recover AdSec failure!");
        }
        else {
            TCSNotify("Stopping AdSec");
            _loopOn.Set(LOOP_OFF);
            //adsec->enableTTOffload(false);
            adsec->stop();
            res2 = adsec->getCommandResult();
            if (res2 == SysCmdSuccess)
                logger->log(Logger::LOG_LEV_INFO, "AdSec successfuly stopped");
            else 
                logger->log(Logger::LOG_LEV_ERROR, "Problem stopping adsec!");
        }
        if ((res != SysCmdSuccess) || (res2 != SysCmdSuccess))  {
            // And now what we do?
            //((AOArbFsm*)_cmdHandler)->forceState(AOArbFsmCore::Failure);
            }
        else {
            _loopOn.Set(LOOP_OFF);
            ((AOArbFsm*)_cmdHandler)->forceState(AOArbFsmCore::Operational);
        }
        break;

    case BasicOpCodes::PANIC:
        TCSNotify(alert);
        setStopFlag(true);
        // If WFS registers an error, open the loop on AdSec side.
        if (_activeWFS && (sender == _activeWFS->getMsgDIdent())) {
            if ((state == "LoopClosed") || (state == "LoopSuspended")) {
                TCSNotify("Stopping AdSec");
                _loopOn.Set(LOOP_OFF);
                adsec->enableTTOffload(false);
                adsec->stop();
                res = adsec->getCommandResult();
                if (res == SysCmdSuccess) 
                    logger->log(Logger::LOG_LEV_INFO, "AdSec successfully stopped");
                else
                    logger->log(Logger::LOG_LEV_ERROR, "Problem stopping adsec!");
            }
        } 
	    logger->log(Logger::LOG_LEV_INFO, "PANIC alert! forcing Failure state.");
        ((AOArbFsm*)_cmdHandler)->forceState(AOArbFsmCore::Failure);
        break;

        // HO offload - no action to do
    case BasicOpCodes::HOOFFLOAD:
        break;

        // Dump saved - no action to do
    case BasicOpCodes::DUMPSAVED:
        break;

    default:
        logger->log(Logger::LOG_LEV_ERROR, "Unknown Alert opcode %d received from %s", alert->getCode(), sender.c_str());
        break;
    }
   
    // Notify all registered clients
    notifyClients(alert);
}

void AOArbitrator::speak( string msg) {

  char str[1024];

  if (msg.compare("none") == 0)
      return;

  string host =  (std::string) ConfigDictionary()["Voice_Host"];
  int port = (int) ConfigDictionary()["Voice_Port"];
  snprintf(str, 1024, "echo \"%s\" | nc %s %d &", msg.c_str(), host.c_str(), port);
  system(str);

}

void AOArbitrator::clearStopCmd() {
   _stopCmd.Set(0);
}

bool AOArbitrator::checkStopCmd() {
   int v;
   _stopCmd.Update();
   _stopCmd.Get(&v);
   return v!=0;
}

void AOArbitrator::doSpeak( string key) {

   Config_File &cfg = AbstractArbitrator::getInstance()->ConfigDictionary();
   AOArbitrator *arb = (AOArbitrator*) AbstractArbitrator::getInstance();

   try {
      ((AOArbitrator*)arb)->speak( (std::string) cfg[key]);
   } catch(...) {}

}

