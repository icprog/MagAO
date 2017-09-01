#include "aoarbitrator/WfsArbSystem.h"
#include "arblib/wfsArb/WfsCommands.h"
#include "arblib/base/Commands.h"

#include "aoarbitrator/AOArbitrator.h"
#include "Paths.h"

using namespace Arcetri::Arbitrator;
using namespace Arcetri::Wfs_Arbitrator;

// Define this to avoid sending any commands to WFS and always reply OK
#undef TEST

WfsArbSystem::WfsArbSystem(string wfsSpec,bool simulation): AbstractSystem(wfsSpec)
{
   _simulation=simulation;
   _msgdId=Utils::getClientname("wfsarb", true, wfsSpec);
   _msgdDomain="M_"+wfsSpec;
   _ident=wfsSpec;
	_arbitrator = AbstractArbitrator::getInstance();
	_wfsArbIntf = NULL;
   _powerOn=false;
   _fsmState = "Unknown";
   _clStatReady = false;
   _lensTrackingTarget = false;
   _wasConnected=false;
   _wfsArbHealth=RTDBvar(_arbitrator->MyFullName()+"."+wfsSpec+".HEALTH",INT_VARIABLE,1);

// _fsm_stat_varname=(string)ConfigDictionary()[AOARBITRATOR_WFS_STATEVAR]+"@"+_msgdName;
// if ((p = fsm_statevarname.find("*")) != string::npos)
//    _fsm_stat_varname[p] = _arbitrator->Side()[0];

}

void WfsArbSystem::setArbInterface(ArbitratorInterface* intf)
{
	_wfsArbIntf = intf;
}


WfsArbSystem::~WfsArbSystem() {

}

bool WfsArbSystem::sync() {

   RequestWfsStatus request(1000);
   RequestWfsStatus *reply = NULL;

   if(_simulation) {
      _clStatReady=1;
      _powerOn=true;
      return true;
   }

   try {
      reply = (RequestWfsStatus *)_wfsArbIntf->requestCommand(&request);
      Logger::get()->log( Logger::LOG_LEV_DEBUG, "thGetStartT(%s)", _wfsArbIntf->getTargetArbitrator().c_str()); 
      if (thGetStartT(_wfsArbIntf->getTargetArbitrator().c_str(), &_arbStartTime) != NO_ERROR) {
          Logger::get()->log(Logger::LOG_LEV_WARNING, "Cannot get arbitrator start time for %s", 
                             _wfsArbIntf->getTargetArbitrator().c_str());
      }
   } catch (AOException &e) {
       Logger::get()->log(Logger::LOG_LEV_DEBUG, "Error requesting status from "+_ident);
       _clStatReady = false;
       return false;
   }
   
   if (!reply->isSuccess()) {
       _clStatReady = false;
       return false;
   }
   
   // Get on/off status
   _fsmState = reply->getArbitratorStatus();
   if (_fsmState == "PowerOff")
      _powerOn = false;
   else
      _powerOn = true;

   _clStatReady = reply->getClStatReady();
   _lensTrackingTarget = reply->getLensTrackingTarget();

   return true;
}

ConnectionStatus WfsArbSystem::updateConnectionStatus() {
  ConnectionStatus stat;
  static struct timeval lastStartTime = {0L, 0L};

  bool connected=sync();

  if(connected) {
     if(!_wasConnected) {
        Logger::get()->log(Logger::LOG_LEV_INFO, _ident+" subsystem connected");
        FeedbackRequest();
        stat=JustConnected;
        getArbStartTime(lastStartTime);
     } else {
         struct timeval startTime;
         getArbStartTime(startTime);
         if ((startTime.tv_sec != lastStartTime.tv_sec) || (startTime.tv_usec != lastStartTime.tv_usec)) {
             Logger::get()->log(Logger::LOG_LEV_INFO,  _ident+" subsystem restarted");
             FeedbackRequest();
             stat=JustConnected;
             lastStartTime = startTime;
         }
         else {
             stat=IsConnected;
         }
     }
     _wasConnected=true;
  } else {
     if(_wasConnected) {
        Logger::get()->log(Logger::LOG_LEV_INFO, _ident+" subsystem disconnected");
        stat=JustDisconnected;
     } else
        stat=IsDisconnected;
     _wasConnected=false;
  }
  _wfsArbHealth.Set(_clStatReady ? 1 : 0);
  return stat;
}


void WfsArbSystem::powerOn() {
	_logger->log(Logger::LOG_LEV_INFO, "Executing PowerOn...");

#ifdef TEST
	setCommandResult(SysCmdSuccess);
   return;
#endif

	try {
      Wfs_Arbitrator::powerOnParams params; // TODO obtain params from somewhere!!
      params.config = "WFS_complete_(with_ccd47)";
      params.boardSetup= "";
      params.sourceSetup = "CALIBRATION";
      params.opticalSetup= false;

		Operate operate(360*1000, params);
		_logger->log(Logger::LOG_LEV_INFO, "Requesting Operate to WfsArbitrator...");
		Command* operateReply = _wfsArbIntf->requestCommand(&operate);

      setCommandResultFromReply(operateReply);
	}
	catch(ArbitratorInterfaceException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, "Operate failed: %s", e.what().c_str());
      setCommandResult(SysCmdError, e.what(Hmi));
	}
}

void WfsArbSystem::backToOperate() {
	_logger->log(Logger::LOG_LEV_INFO, "Executing backToOperate...");

#ifdef TEST
	setCommandResult(SysCmdSuccess);
   return;
#endif

	try {
      Wfs_Arbitrator::powerOnParams params;
      params.config = "";
      params.boardSetup= "";
      params.opticalSetup= false;

		Operate operate(300000, params);
		_logger->log(Logger::LOG_LEV_INFO, "Requesting Operate to WfsArbitrator...");
		Command* operateReply = _wfsArbIntf->requestCommand(&operate);

      setCommandResultFromReply(operateReply);
	}
	catch(ArbitratorInterfaceException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, "Operate failed: %s", e.what().c_str());
      setCommandResult(SysCmdError, e.what());
	}
}

void WfsArbSystem::powerOff() {
	_logger->log(Logger::LOG_LEV_INFO, "Executing PowerOff...");
	Off off(60*1000);		// TODO obtain timeout from somewhere!!!

#ifdef TEST
	setCommandResult(SysCmdSuccess);
   return;
#endif

   simpleCommand(&off);
}

void WfsArbSystem::presetAO( Wfs_Arbitrator::prepareAcquireRefParams *params, Wfs_Arbitrator::prepareAcquireRefResult **result) {


//#ifdef TEST
//	setCommandResult(SysCmdSuccess);
//   Wfs_Arbitrator::prepareAcquireRefResult res;
//   *result = &res;
//   return;
//#endif

   Command *reply;

   if (!sync()) {
      setCommandResult(SysCmdError, "WFS subsytem is not available");
      return;
   }

   try {

	   _logger->log(Logger::LOG_LEV_INFO, "Executing PresetAO...");
      PrepareAcquireRef prepAcquireRef(150*1000, *params);		// TODO obtain timeout from somewhere!!!
      _logger->log(Logger::LOG_LEV_INFO, "Requesting PrepareAcquireRef to WfsArbitrator...");
      reply = _wfsArbIntf->requestCommand(&prepAcquireRef);

   } catch (ArbitratorInterfaceException &e) {
      _logger->log(Logger::LOG_LEV_ERROR, "PresetAO failed: %s", e.what().c_str());
      setCommandResult(SysCmdError, e.what());
      return;
   }

   if(reply->isSuccess()) {
      *result =  ((PrepareAcquireRef *)reply)->getOutputParams();
      (*result)->Dump();
   } else {
    *result = NULL;
   }

   setCommandResultFromReply(reply);

}


string fwpos2name( int /* fwNum */, int fwPos) {

   // Should lookup in the conf files....
   ostringstream oss;
   oss << "Filter #" << fwPos;
   return oss.str();
}

void WfsArbSystem::acquireRefAO( acquireRefAOResult *result) {

	 setCommandResult(SysCmdError);

//#ifdef TEST
//   setCommandResult(SysCmdSuccess);
//   return;
//#endif

   _logger->log(Logger::LOG_LEV_DEBUG, "Executing AcquireRef...");

   try {

       AcquireRef acquireRef(300*1000);		// TODO obtain timeout from somewhere!!!
       Command* reply = _wfsArbIntf->requestCommand(&acquireRef); 
       if (reply)
           if(reply->isSuccess()) {
               
               Wfs_Arbitrator::acquireRefResult *wfsResult = ((AcquireRef *)reply)->getOutputParams();

               result->deltaXY[0] = wfsResult->deltaXY[0];
               result->deltaXY[1] = wfsResult->deltaXY[1];
               result->slNull = "";
               result->nModes = wfsResult->nModes;
               result->freq = wfsResult->freq;
               result->nBins = wfsResult->bin;
               result->ttMod = wfsResult->TTmod;
               result->f1spec = fwpos2name( 1, wfsResult->Fw1Pos);
               result->f2spec = fwpos2name( 2, wfsResult->Fw2Pos);
               result->gain  = wfsResult->gain;
               result->rec   = wfsResult->rec;
               result->filtering = wfsResult->filtering;
               result->pupils = wfsResult->pupils;
               result->base   = wfsResult->base;
               result->decimation = wfsResult->decimation;
               result->starMag = wfsResult->starMag;
               result->strehl = 0;
               result->r0 = 0;
               memset( result->mSNratio, 0, sizeof(double)*672);
               memcpy( result->TVframe, wfsResult->TVframe, sizeof(result->TVframe));
               result->CLbase = wfsResult->CLbase;
               result->CLrec  = wfsResult->CLrec;
               result->CLfreq = wfsResult->CLfreq;
           }

       setCommandResultFromReply(reply);
   }
   catch(ArbitratorInterfaceException& e) {
       _logger->log(Logger::LOG_LEV_ERROR, "acquireRefAO failed: %s", e.what().c_str());
   }

}

void WfsArbSystem::checkRefAO( checkRefAOResult *result) {

	_logger->log(Logger::LOG_LEV_INFO, "Executing CheckRefAO...");
   try {
       
       CheckRef checkRef(120*1000);  // [TODO] obtain timeout from somewhere!!!
       Command *reply = _wfsArbIntf->requestCommand(&checkRef);

       if (reply)
         if (reply->isSuccess()) {
             
             Wfs_Arbitrator::checkRefResult *wfsResult = ((CheckRef*)reply)->getOutputParams();
             
             result->deltaXY[0] = wfsResult->deltaXY[0];
             result->deltaXY[1] = wfsResult->deltaXY[1];
             result->starMag    = wfsResult->starMag;

         }
       setCommandResultFromReply(reply);
   }
   catch(ArbitratorInterfaceException& e) {
       _logger->log(Logger::LOG_LEV_ERROR, "CheckRefAO failed: %s", e.what().c_str());
   }
}

void WfsArbSystem::recoverFailure() {
}

void WfsArbSystem::correctModes( correctModesParams /* params */) {
#ifdef TEST
	setCommandResult(SysCmdSuccess);
   return;
#endif

	_logger->log(Logger::LOG_LEV_INFO, "WFS Executing CorrectModes");
   Arcetri::Wfs_Arbitrator::correctModesParams wfsParams;
   // [TODO] transfer params to wfs structure
   CorrectModes correctModes( 10000, wfsParams);   // [TODO] obtain timeout from somewhere
   simpleCommand( &correctModes);
}


void WfsArbSystem::autoTrack( Wfs_Arbitrator::autoTrackParams params) {

#ifdef TEST
	setCommandResult(SysCmdSuccess);
   return;
#endif

	_logger->log(Logger::LOG_LEV_INFO, "WFS Executing AutoTrack");
   AutoTrack autoTrack( 10000, params);   // [TODO] obtain timeout from somewhere
   simpleCommand( &autoTrack);


}

void WfsArbSystem::modifyAO( Arcetri::Arbitrator::modifyAOParams *params, acquireRefAOResult *result) {
   _logger->log(Logger::LOG_LEV_INFO, "Executing ModifyAO...");

//#ifdef TEST
//	setCommandResult(SysCmdSuccess);
//   return;
//#endif

   Arcetri::Wfs_Arbitrator::modifyAOparams wfsParams;

   wfsParams.freq = params->freq;
   wfsParams.Binning = params->binning;
   wfsParams.TTmod = params->TTmod;
   wfsParams.Fw1Pos = atoi(params->f1spec.c_str());
   wfsParams.Fw2Pos = atoi(params->f2spec.c_str());
   wfsParams.checkCameralens = false;

   Arcetri::Wfs_Arbitrator::ModifyAO modifyAo(80000, wfsParams);		// TODO obtain timeout from somewhere!!!
   Command *reply =simpleCommand( &modifyAo);
   if (reply)
      if(reply->isSuccess()) {

         Wfs_Arbitrator::modifyAOparams *wfsResult = ((Arcetri::Wfs_Arbitrator::ModifyAO*)reply)->getOutputParams();

         result->deltaXY[0] = 0;
         result->deltaXY[1] = 0;
         result->slNull = "";
         result->nModes = 0;
         result->freq = wfsResult->freq;
         result->nBins = wfsResult->Binning;
         result->ttMod = wfsResult->TTmod;
         result->f1spec = wfsResult->Fw1Pos;
         result->f2spec = wfsResult->Fw2Pos;
         result->strehl = 0;
         result->r0 = 0;
         memset( result->mSNratio, 0, sizeof(double)*672);
         memset( result->TVframe, 0, 256*256);

      }
   setCommandResultFromReply(reply);
}


void WfsArbSystem::offsetXY( offsetXYParams params, bool brake) {
#ifdef TEST
	setCommandResult(SysCmdSuccess);
   return;
#endif

	_logger->log(Logger::LOG_LEV_INFO, "Executing OffsetXY...");
   Arcetri::Wfs_Arbitrator::offsetXYparams wfsParams;
   wfsParams.offsetX = params.deltaXY[0];
   wfsParams.offsetY = params.deltaXY[1];
   wfsParams.brake = brake;
   OffsetXY offsetXY( 80*1000, wfsParams);   // [TODO] obtain timeout from somewhere
   simpleCommand( &offsetXY);
}

void WfsArbSystem::offsetZ( offsetZParams params) {
#ifdef TEST
	setCommandResult(SysCmdSuccess);
   return;
#endif

	_logger->log(Logger::LOG_LEV_INFO, "Executing OffsetZ...");
   Arcetri::Wfs_Arbitrator::offsetZparams wfsParams;
   wfsParams.offsetZ = params.deltaZ;
   OffsetZ offsetZ( 80*1000, wfsParams);   // [TODO] obtain timeout from somewhere
   simpleCommand( &offsetZ);
}


void WfsArbSystem::optimizeGain() {
#ifdef TEST
	setCommandResult(SysCmdSuccess);
   return;
#endif

	_logger->log(Logger::LOG_LEV_INFO, "Executing OptimizeGain...");
   OptimizeGain optimizeGain( 160*1000);   // [TODO] obtain timeout from somewhere
   simpleCommand( &optimizeGain);
}


void WfsArbSystem::pause() {
#ifdef TEST
	setCommandResult(SysCmdSuccess);
   return;
#endif

   _logger->log(Logger::LOG_LEV_INFO, "Executing Pause...");
   PauseLoop pause( 5000);   // [TODO] obtain timeout from somewhere
   simpleCommand( &pause);
}


void WfsArbSystem::resume() {
#ifdef TEST
	setCommandResult(SysCmdSuccess);
   return;
#endif

   _logger->log(Logger::LOG_LEV_INFO, "Executing Resume...");
   ResumeLoop resume( 5000);   // [TODO] obtain timeout from somewhere
   simpleCommand( &resume);
}

Command *WfsArbSystem::simpleCommand( Command *cmd) {

   try {
      _logger->log(Logger::LOG_LEV_INFO, "Requesting %s to WfsArbitrator...", cmd->getDescription().c_str());
      Command* reply = _wfsArbIntf->requestCommand(cmd);
      setCommandResultFromReply(reply);
      return reply;
   }
   catch(ArbitratorInterfaceException& e) {
      _logger->log(Logger::LOG_LEV_ERROR, "%s failed: %s", cmd->getDescription().c_str(), e.what().c_str());
      setCommandResult(SysCmdError, e.what());
      return NULL;
   }
}


void WfsArbSystem::startAO(bool nocheck) {
#ifdef TEST
	setCommandResult(SysCmdSuccess);
   return;
#endif

   _logger->log(Logger::LOG_LEV_INFO, "Executing StartAO...");
   CloseLoop close( 60*1000);   // [TODO] obtain timeout from somewhere
   simpleCommand( &close);
}

void WfsArbSystem::stop() {
#ifdef TEST
	setCommandResult(SysCmdSuccess);
   return;
#endif

   _logger->log(Logger::LOG_LEV_INFO, "Executing Stop...");
   StopLoop stop( 5000);   // [TODO] obtain timeout from somewhere
   simpleCommand( &stop);
}



void WfsArbSystem::userPanic() {
#ifdef TEST
	setCommandResult(SysCmdSuccess);
   return;
#endif

	_logger->log(Logger::LOG_LEV_INFO, "Executing UserPanic...");
   Wfs_Arbitrator::EmergencyOff panic( 5000);   // [TODO] obtain timeout from somewhere
   simpleCommand( &panic);

}

bool WfsArbSystem::enableDisturb( bool wfs, bool ovs)
{
    try {
        enableDisturbParams enableParams;
        enableParams._enableWFS = wfs;
        enableParams._enableOVS = ovs;
        EnableDisturb enableDisturb(20000, enableParams);
        Command* reply = _wfsArbIntf->requestCommand(&enableDisturb);
        bool ret = reply->isSuccess();
        delete reply;
        return ret;
    } catch (AOException &e) {
        _logger->log(Logger::LOG_LEV_ERROR, "enableDisturb failed: %s", e.what().c_str());
        return false;
    }
}



bool WfsArbSystem::FeedbackRequest() {

    RequestFeedback feedbackReq(500);
    Command* feedbackReqReply;
    bool res = false;

    if(_simulation) {
       _logger->log(Logger::LOG_LEV_INFO, "Simulation mode. I do not try to register for alerts from WFS arbitrator");
       return true;
    }
    _logger->log(Logger::LOG_LEV_INFO, "Trying to register for alerts notification from Wfs arbitrator...");
    
    try {
        // --- Request for feedback ---
        feedbackReqReply = _wfsArbIntf->requestCommand(&feedbackReq);
		if(feedbackReqReply->isSuccess()) {
			_logger->log(Logger::LOG_LEV_DEBUG, "Alerts notifications succesfully requested!");
            res = true;
		}
		else {
			_logger->log(Logger::LOG_LEV_WARNING, "Alerts notifications request failed");
		}
		delete feedbackReqReply;
    }
    catch(...) {
		_logger->log(Logger::LOG_LEV_ERROR, "Connection to Wfs arbitrator failed");
	}
   return res;
}
