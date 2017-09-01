#include "AOCommandsImpl.h"
#include "AOGlobals.h"
#include "Paths.h"

using namespace Arcetri::Arbitrator;

// Define this to have a simulated atmospheric disturbance during acquire ref and co.

#undef SIMULATE_ATMOSPHERE
#define CL_CENTER

///////////////////////////// POWER ON WFS

bool PowerOnWfsImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<PowerOnWfs Command has an empty (extra) validation>");
   return true;
}

AbstractSystemCommandResult PowerOnWfsImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   PowerOnWfs *cmd = ((PowerOnWfs*)getCommand());
   string wfsSpec = cmd->getParams().wfsid;

   WfsArbSystem* wfs = getWfs(wfsSpec);

   if(!wfs) {
      _logger->log(Logger::LOG_LEV_WARNING, "No such WFS: %s in PowerOnWfs",wfsSpec.c_str());
      return SysCmdRetry;
   }

   wfs->powerOn();

   getCommand()->setErrorString( wfs->getErrorString());

   checkCanceled(); // Can throw a CommandCanceledException

   // Check the result
   AbstractSystemCommandResult wfsRes = wfs->getCommandResult();
   if(wfsRes != SysCmdSuccess) _logger->log(Logger::LOG_LEV_WARNING, "PowerOnWfs failed");
   return wfsRes;
}


///////////////////////////// POWER OFF WFS

bool PowerOffWfsImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<PowerOffWfs Command has an empty (extra) validation>");
   return true;
}

AbstractSystemCommandResult PowerOffWfsImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   PowerOffWfs *cmd = ((PowerOffWfs*)getCommand());
   string wfsSpec = cmd->getParams().wfsid;
   WfsArbSystem* wfs = getWfs(wfsSpec);

   if(!wfs) {
      _logger->log(Logger::LOG_LEV_WARNING, "No such WFS: %s in PowerOffWfs",wfsSpec.c_str());
      return SysCmdRetry;
   }
   wfs->powerOff();
   getCommand()->setErrorString( wfs->getErrorString());

   checkCanceled(); // Can throw a CommandCanceledException

   // Check the result
   AbstractSystemCommandResult wfsRes = wfs->getCommandResult();
   if(wfsRes != SysCmdSuccess) _logger->log(Logger::LOG_LEV_WARNING, "PowerOff on Wfs failed");
   return wfsRes;
}


///////////////////////////// POWER ON ADSEC

bool PowerOnAdSecImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<PowerOnAdSec Command has an empty (extra) validation>");
   return true;
}

AbstractSystemCommandResult PowerOnAdSecImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   AdsecArbSystem* adsec = getAdSec();

   adsec->powerOn();
   getCommand()->setErrorString( adsec->getErrorString());

   checkCanceled(); // Can throw a CommandCanceledException

   // Check the result
   AbstractSystemCommandResult adsecRes = adsec->getCommandResult();
   if(adsecRes != SysCmdSuccess) _logger->log(Logger::LOG_LEV_WARNING, "PowerOn on AdSec failed");
   return adsecRes;
}


///////////////////////////// POWER OFF ADSEC

bool PowerOffAdSecImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<PowerOffAdSec Command has an empty (extra) validation>");
   return true;
}

AbstractSystemCommandResult PowerOffAdSecImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   AdsecArbSystem* adsec = getAdSec();

   adsec->powerOff();

   getCommand()->setErrorString( adsec->getErrorString());

   checkCanceled(); // Can throw a CommandCanceledException

   // Check the result
   AbstractSystemCommandResult adsecRes = adsec->getCommandResult();
   if(adsecRes != SysCmdSuccess) _logger->log(Logger::LOG_LEV_WARNING, "PowerOff on AdSec failed");
   return adsecRes;
}

///////////////////////////// MIRROR SET

bool MirrorSetImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<MirrorSet Command has an empty (extra) validation>");
   return true;
}

AbstractSystemCommandResult MirrorSetImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   AdsecArbSystem* adsec = getAdSec();

   adsec->mirrorSet();

   getCommand()->setErrorString( adsec->getErrorString());

   checkCanceled(); // Can throw a CommandCanceledException

   // Check the result
   AbstractSystemCommandResult adsecRes = adsec->getCommandResult();
   if(adsecRes != SysCmdSuccess) _logger->log(Logger::LOG_LEV_WARNING, "MirrorSet on AdSec failed");
   return adsecRes;

}


///////////////////////////// MIRROR REST

bool MirrorRestImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<MirrorRest Command has an empty (extra) validation>");
   return true;
}

AbstractSystemCommandResult MirrorRestImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   AdsecArbSystem* adsec = getAdSec();

   adsec->mirrorRest();

   getCommand()->setErrorString( adsec->getErrorString());

   checkCanceled(); // Can throw a CommandCanceledException

   // Check the result
   AbstractSystemCommandResult adsecRes = adsec->getCommandResult();
   if(adsecRes != SysCmdSuccess) _logger->log(Logger::LOG_LEV_WARNING, "MirrorRest on AdSec failed");
   return adsecRes;

}


///////////////////////////// FAULT RECOVERY

bool FaultRecoveryImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<FaultRecovery Command has an empty (extra) validation>");
   return true;
}

AbstractSystemCommandResult FaultRecoveryImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   AdsecArbSystem* adsec = getAdSec();

   adsec->recoverFailure();
   getCommand()->setErrorString( adsec->getErrorString());

   checkCanceled(); // Can throw a CommandCanceledException

   // Check the result
   AbstractSystemCommandResult adsecRes = adsec->getCommandResult();
   if(adsecRes != SysCmdSuccess) _logger->log(Logger::LOG_LEV_WARNING, "RecoverFailure on AdSec failed");
   return adsecRes;

}

///////////////////////////// STANDALONE FAIL

bool StandaloneFailImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<StandaloneFail Command has an empty (extra) validation>");
   return true;
}

AbstractSystemCommandResult StandaloneFailImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   _logger->log(Logger::LOG_LEV_WARNING, "StandaloneFail not implemented yet");

   return SysCmdSuccess;
}


///////////////////////////// SEEINGLMTD FAIL

bool SeeingLmtdFailImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<SeeingLmtdFail Command has an empty (extra) validation>");
   return true;
}

AbstractSystemCommandResult SeeingLmtdFailImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   _logger->log(Logger::LOG_LEV_WARNING, "SeeingLmtdFail not implemented yet");

   return SysCmdSuccess;
}




///////////////////////////// PRESET FLAT

bool PresetFlatImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<PresetFlat Command has an empty (extra) validation>");
   return true;
}

AbstractSystemCommandResult PresetFlatImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   AOArbitrator* aoArb = ((AOArbitrator*)getArbitrator());
   AdsecArbSystem* adsec = getAdSec();
   PresetFlat *cmd = ((PresetFlat*)getCommand());

   // Retrieve parameters
   string flatSpec = cmd->getParams().flatSpec;

   // Set default shape if nothing was specified
//   if (flatSpec == "")
//      flatSpec = "default";
//
//   flatSpec += ".sav";


   adsec->presetFlat( flatSpec);

   // progress test
   //updateProgress(80);

   checkCanceled(); // Can throw a CommandCanceledException

   // Check the result
   AbstractSystemCommandResult adSecRes = adsec->getCommandResult();

   // Set the output shape (the same as the input for now)
   presetFlatParams result;
   if (adSecRes == SysCmdSuccess) 
      result.flatSpec = flatSpec;
   else {
      _logger->log(Logger::LOG_LEV_WARNING, "PresetFlat on AdSec failed");
      getCommand()->setErrorString( adsec->getErrorString());
      result.flatSpec = "NO_FLAT";
      aoArb->updateRtdb(&result);
   }

   cmd->setResult(result);

   aoArb->setAOMode( AOMode::FIX_AO);

   return adSecRes;
}



///////////////////////////// PRESET AO

bool PresetAOImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<PresetAO Command has an empty (extra) validation>");
   return true;
}

static string fwpos2name( int fwNum, int fwPos) {

   fwNum = 0;   // Avoid warning

   // Should lookup in the conf files....
   ostringstream oss;
   oss << "Filter #" << fwPos;
   return oss.str();
}


AbstractSystemCommandResult PresetAOImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {


   PresetAO *cmd = ((PresetAO*)getCommand());
   presetAOParams params = cmd->getParams();

   string wfsSpec = params.wfsSpec;

   WfsArbSystem* wfs = getWfs(wfsSpec);

   AOArbitrator* aoArb = ((AOArbitrator*)getArbitrator());

   if(!wfs) {
      _logger->log(Logger::LOG_LEV_WARNING, "No such WFS: '%s' in PresetAO",wfsSpec.c_str());
      return SysCmdRetry;
   }
   
   aoArb->setActiveWFS(wfsSpec);

   AdsecArbSystem* adsec = getAdSec();

   // Retrieve parameters for Wfs
   Wfs_Arbitrator::prepareAcquireRefParams prepParams;
   prepParams.Instr = params.instr;
   prepParams.AOMode = params.aoMode;
   prepParams.ROCoords[0]  = params.roCoord[0];
   prepParams.ROCoords[1]  = params.roCoord[1];
   prepParams.SOCoords[0]  = params.soCoord[0];
   prepParams.SOCoords[1]  = params.soCoord[1];
   prepParams.Elevation = params.elevation;
   prepParams.RotAngle = params.rotAngle;
   prepParams.GravAngle = params.gravAngle;
   prepParams.Mag = params.mag;
   prepParams.Color = params.color;
   prepParams.r0 = params.r0;

   _logger->log(Logger::LOG_LEV_INFO, "Preset AO: seeing mode is %f", params.r0);

   Wfs_Arbitrator::prepareAcquireRefResult *wfsResult;
   wfs->presetAO( &prepParams, &wfsResult);

   updateProgress(80);
   
   if ((!wfsResult) || (wfs->getCommandResult() != SysCmdSuccess)) {
      _logger->log(Logger::LOG_LEV_WARNING, "PresetAO on Wfs failed");
      getCommand()->setErrorString( wfs->getErrorString());
      return wfs->getCommandResult();
   }

   if (wfsResult->freq == 0.0) {
      string errstr = "acquireRefResult from WFS has zero frequency! Cannot go on";
      _logger->log(Logger::LOG_LEV_ERROR, errstr);
      getCommand()->setErrorString( errstr);
      return SysCmdError;
   }

   wfsResult->Dump();

   try {
      aoArb->updateRtdb(wfsResult);
   } catch (AOException *e) {
      _logger->log(Logger::LOG_LEV_ERROR, "updateRtdb() failed: %s\n", e->what().c_str());
   }

   // Transfer parameters into result structure
   acquireRefAOResult result;
   result.deltaXY[0] = 0.0;
   result.deltaXY[1] = 0.0;
   result.slNull = "";
   result.nModes = wfsResult->nModes;
   result.freq = wfsResult->freq;
   result.nBins = wfsResult->binning;
   result.ttMod = wfsResult->TTmod;
   result.f1spec = fwpos2name( 1, wfsResult->Fw1Pos);
   result.f2spec = fwpos2name( 2, wfsResult->Fw2Pos);
   result.strehl = 0.0;
   result.starMag = 0.0;
   result.r0 = 0.0;
   memset( result.mSNratio, 0, sizeof(result.mSNratio));
   memset( result.TVframe, 0, sizeof(result.TVframe));

   cmd->setResult(result);

   aoArb->setAOMode( AOMode::ACE_AO);

   aoArb->setAdcTracking( params.adcTracking);

   updateProgress(90);

   // Set diagnostic data path
   adsec->selectFocalStation( params.focStation);
   if (adsec->getCommandResult() != SysCmdSuccess) {
       _logger->log(Logger::LOG_LEV_WARNING, "SelectFocalStation on AdSec failed");
       getCommand()->setErrorString(adsec->getErrorString());
       return SysCmdError;
   }

   return SysCmdSuccess;
}


///////////////////////////// ACQUIRE_REF_AO

bool AcquireRefAOImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<AcquireRefAO Command has an empty (extra) validation>");
   return true;
}

void AcquireRefAOImpl::startSeeingLimited() {

   WfsArbSystem* wfs = getWfs();
   AdsecArbSystem* adsec = getAdSec();

   try {
      adsec->setDisturbance("/towerdata/adsec_calib/CMD/disturb/dist_mag585_atm_s0.8_L040.0_v15.0_ovfreq1000.00_sd1983.fits");
      adsec->setGain(0);
      adsec->startAO();
      wfs->enableDisturb( true, true);
      wfs->startAO(true);
      msleep(1*1000);
      wfs->stop();
   } catch (AOException &e) {
      _logger->log( Logger::LOG_LEV_WARNING, "Failed to start seeing limited disturbance");
   }
}

void AcquireRefAOImpl::stopSeeingLimited() {

   WfsArbSystem* wfs = getWfs();
   AdsecArbSystem* adsec = getAdSec();

   try {
      wfs->enableDisturb( false, false);
      wfs->startAO(true);
      msleep(1*1000);
      wfs->stop();
      adsec->stop();
   } catch (AOException &e) {
      _logger->log( Logger::LOG_LEV_WARNING, "Failed to stop seeing limited disturbance");
   }
}

void StartAOImpl::stopSeeingLimited() {

   WfsArbSystem* wfs = getWfs();
   AdsecArbSystem* adsec = getAdSec();

   try {
      wfs->enableDisturb( false, false);
      wfs->startAO(true);
      msleep(1*1000);
      wfs->stop();
      adsec->stop();
   } catch (AOException &e) {
      _logger->log( Logger::LOG_LEV_WARNING, "Failed to stop seeing limited disturbance");
   }
}


// AcquireRefAO sequence:
//
// 1. start AcquireRef on the WFS:
//    - check star position on ccd47
//    - move wfs stages to center it on the hotspot
//    - repeat until position is good
// 2. Exit immediately to give the operator a chance of seeing what's going on
//    and fix the centering if needed. The rest of the preparation is done in StartAO

// External function to perform an acquire ref from a different command
AbstractSystemCommandResult acquireRef( WfsArbSystem *wfs, AdsecArbSystem *adsec, Command *command) {

   acquireRefAOResult result;
   AbstractSystemCommandResult wfsRes;

   try {
      wfs->acquireRefAO( &result);
      wfsRes = wfs->getCommandResult();
      if (wfsRes != SysCmdSuccess) {
         command->setErrorString( wfs->getErrorString());
         return wfsRes;
      }
   } catch (AOException &e) {
      throw;
   }

   adsec->setAcquireRefResult(result);

   return SysCmdSuccess;
}

AbstractSystemCommandResult AcquireRefAOImpl::doExecuteImpl() throw (Arcetri::Arbitrator::CommandExecutionException, Arcetri::Arbitrator::CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException)
{

   AbstractSystemCommandResult wfsRes;
   WfsArbSystem* wfs = getWfs();
   AdsecArbSystem* adsec = getAdSec();
   AcquireRefAO *cmd = (AcquireRefAO*)getCommand();


   // Simulate a seeing-limited star using disturbance
#ifdef SIMULATE_ATMOSPHERE
   startSeeingLimited();
#endif

   acquireRefAOResult result;

   updateProgress(1,"Starting source acquisition on WFS");

   try {
      wfs->acquireRefAO( &result);
      wfsRes = wfs->getCommandResult();
      if (wfsRes != SysCmdSuccess) {
         _logger->log(Logger::LOG_LEV_WARNING, "AcquireRefAO on Wfs failed");
         getCommand()->setErrorString( wfs->getErrorString());
#ifdef SIMULATE_ATMOSPHERE
         stopSeeingLimited();
#endif
         return wfsRes;
      }
   } catch (AOException &e) {
#ifdef SIMULATE_ATMOSPHERE
      stopSeeingLimited();
#endif
      throw;
   }

   adsec->setAcquireRefResult(result);


   // Set command result
   cmd->setResult( result);
   return SysCmdSuccess;
}


///////////////////////////// CHECKREF_AO

bool CheckRefAOImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<CheckRefAO has empty validation");
   return true;
}

AbstractSystemCommandResult CheckRefAOImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   WfsArbSystem* wfs = getWfs();
   CheckRefAO *cmd = (CheckRefAO*)getCommand();

   checkRefAOResult result;

   wfs->checkRefAO(&result);

   checkCanceled(); // Can throw a CommandCanceledException

   // Check the result
   AbstractSystemCommandResult wfsRes = wfs->getCommandResult();

   if(wfsRes == SysCmdSuccess) {

      try {
         AOArbitrator* aoArb = ((AOArbitrator*)getArbitrator());
         aoArb->updateRtdb(&result);
      } catch (AOException *e) {
         _logger->log(Logger::LOG_LEV_ERROR, "updateRtdb() failed: %s\n", e->what().c_str());
      }
      // Set command result
      cmd->setResult( result);
      return SysCmdSuccess;
   }
   else {
      _logger->log(Logger::LOG_LEV_WARNING, "CheckRefAO on Wfs failed");
      getCommand()->setErrorString( wfs->getErrorString());
      return wfsRes;
   }
}

///////////////////////////// SET ZERNIKES

bool SetZernikesImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<SetZernikes has empty validation");
   return true;
}

AbstractSystemCommandResult SetZernikesImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   string curState = ((AOArbitrator*)getArbitrator())->fsmState();

   if (curState == "Ready") {
       AdsecArbSystem* adsc = getAdSec();
       SetZernikes *cmd = (SetZernikes*)getCommand();

       setZernikesParams params = cmd->getParams();

       adsc->setZernikes(params);

       checkCanceled(); // Can throw a CommandCanceledException

       // Check the result
       AbstractSystemCommandResult adscRes = adsc->getCommandResult();

       if(adscRes == SysCmdSuccess)
          return SysCmdSuccess;
       else {
          _logger->log(Logger::LOG_LEV_WARNING, "SetZernikes on AdSec failed");
          getCommand()->setErrorString( adsc->getErrorString());
          return adscRes;
       }
   }

   _logger->log(Logger::LOG_LEV_WARNING, "Unsupported state %s for SetZernikes command", curState.c_str());
   getCommand()->setErrorString("AOArbitrator: Illegal command for state "+curState);
   return SysCmdError;

}





///////////////////////////// CORRECT MODES

bool CorrectModesImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<CorrectModes has empty validation");
   return true;
}

AbstractSystemCommandResult CorrectModesImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   string curState = ((AOArbitrator*)getArbitrator())->fsmState();

   if (curState == "LoopClosed") {
       WfsArbSystem* wfs = getWfs();
       CorrectModes *cmd = (CorrectModes*)getCommand();

       correctModesParams params = cmd->getParams();

       wfs->correctModes(params);

       checkCanceled(); // Can throw a CommandCanceledException

       // Check the result
       AbstractSystemCommandResult wfsRes = wfs->getCommandResult();

       if(wfsRes == SysCmdSuccess)
          return SysCmdSuccess;
       else {
          _logger->log(Logger::LOG_LEV_WARNING, "CorrectModes on Wfs failed");
          getCommand()->setErrorString( wfs->getErrorString());
          return wfsRes;
       }
   }

   _logger->log(Logger::LOG_LEV_WARNING, "Unsupported state %s for CorrectModes command", curState.c_str());
   return SysCmdError;
}




///////////////////////////// MODIFY AO

bool ModifyAOImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<ModifyAO has empty validation");
   return true;
}

// MODIFY AO

AbstractSystemCommandResult ModifyAOImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   WfsArbSystem* wfs = getWfs();
   AdsecArbSystem* adsec = getAdSec();
   ModifyAO *cmd = (ModifyAO*)getCommand();

   modifyAOParams params = cmd->getParams();
   acquireRefAOResult result;

   wfs->modifyAO( &params, &result);
   adsec->modifyAO();   // At the moment does nothing

   checkCanceled(); // Can throw a CommandCanceledException

   // Check the result
   AbstractSystemCommandResult wfsRes = wfs->getCommandResult();
   AbstractSystemCommandResult adSecRes = adsec->getCommandResult();

   if(wfsRes == SysCmdSuccess && adSecRes == SysCmdSuccess) {
      AOArbitrator* aoArb = ((AOArbitrator*)getArbitrator());
      aoArb->updateRtdb(&result);
      cmd->setResult( result);
      return SysCmdSuccess;
   }
   else {
      if(wfsRes != SysCmdSuccess) {
         _logger->log(Logger::LOG_LEV_WARNING, "ModifyAO on Wfs failed");
         getCommand()->setErrorString( wfs->getErrorString());
      }
      if(adSecRes != SysCmdSuccess) _logger->log(Logger::LOG_LEV_WARNING, "ModifyAO on AdSec failed");
      return wfsRes;
   }
}


///////////////////////////// OFFSET XY

bool OffsetXYImpl::validateImpl() throw(CommandValidationException) {

   OffsetXY *cmd = (OffsetXY*)getCommand();
   offsetXYParams params = cmd->getParams();
   Config_File &cfg = AbstractArbitrator::getInstance()->ConfigDictionary();

   // Only check nominal offset range.
   // Out-of-FoV condition is checked at WFS arbitrator level.
   // Split of big offsets into smaller offsets is done during command execution.

   if ((fabs(params.deltaXY[0]) > (float)cfg["MaxOffsetXY"]) || ((fabs(params.deltaXY[1]) > (float)cfg["MaxOffsetXY"]))) {
      if ((int)cfg["AllowLargeOffset"] == 1) {
          ((AOArbitrator*)getArbitrator())->toggleLargeOffset();
          _skip = true;
          return true;
      }
      string errstr = "OffsetXY out of range (max is "+Utils::itoa((float)cfg["MaxOffsetXY"],5,2)+" mm)";
      _logger->log(Logger::LOG_LEV_ERROR, errstr.c_str());
      getCommand()->setErrorString( errstr);
      return false;
    }

   return true;

}

AbstractSystemCommandResult OffsetXYImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

    WfsArbSystem* wfs = getWfs();
    AdsecArbSystem* adsec = getAdSec();
    OffsetXY *cmd = (OffsetXY*)getCommand();
    Config_File &cfg = AbstractArbitrator::getInstance()->ConfigDictionary();
   AOArbitrator* aoArb = ((AOArbitrator*)getArbitrator());

    if (_skip)
        return SysCmdSuccess;

    offsetXYParams params = cmd->getParams();

    aoArb->doSpeak( "Voice_Nodding");

   // If we aren't in closed loop, the offset can be executed in one shot
   string curState = ((AOArbitrator*)getArbitrator())->fsmState();
   if (curState != "LoopClosed") {
        wfs->offsetXY(params);
        getCommand()->setErrorString( wfs->getErrorString());
        return wfs->getCommandResult();
   }

    // Reduce gain before starting
    adsec->reduceGain();



    // Split big offsets into smaller offsets
    offsetXYParams myoffsetxy;
    double xoff = params.deltaXY[0];
    double yoff = params.deltaXY[1];
    int xsign = (xoff>0)?1:-1;
    int ysign = (yoff>0)?1:-1;
    double epsilon = 1e-3;
    while ((fabs(xoff) > epsilon) || (fabs(yoff) > epsilon)) {

        // Wait for adsec to be in good condition (no big tt offloads)
        time_t start = time(NULL);
        int timeout=40;
        while (1) {
            sleep(1);
            double tip = adsec->getTipOffload();
            double tilt = adsec->getTiltOffload();
            if ((tip < 3e-6) && (tilt < 3e-6))
                break;
            if (time(NULL)-start > timeout) {
                string errstr = "secondary mirror tip/tilt offload did not converge after "+Utils::itoa(timeout)+" seconds.";
                _logger->log(Logger::LOG_LEV_ERROR, "OffsetXY failed: %s", errstr.c_str());
                 getCommand()->setErrorString(errstr);
                 adsec->restoreGain();
                 return SysCmdError;
            }
            if (getArbitrator()->getStopFlag()) {
                _logger->log(Logger::LOG_LEV_ERROR, "Command stopped by external signal");
                return SysCmdError;
            }
        }
                 
        // Watch out: cfg values are coming as float, not double, and cannot be casted to double
        myoffsetxy.deltaXY[0] = min( (float)fabs(xoff), (float)cfg["SplitOffsetXY"]) * xsign;
        myoffsetxy.deltaXY[1] = min( (float)fabs(yoff), (float)cfg["SplitOffsetXY"]) * ysign;


        xoff -= myoffsetxy.deltaXY[0];
        yoff -= myoffsetxy.deltaXY[1];
        _logger->log(Logger::LOG_LEV_INFO, "Moving by %4.1f %4.1f, remaining offset %4.1f %4.1f", myoffsetxy.deltaXY[0], myoffsetxy.deltaXY[1], xoff, yoff);

        bool brake = true;
        if ((fabs(xoff) > epsilon) || (fabs(yoff) > epsilon))
            brake = false;

        wfs->offsetXY(myoffsetxy, brake);

        // Check whether this step has been successful
        AbstractSystemCommandResult wfsRes = wfs->getCommandResult();
        if (wfsRes != SysCmdSuccess) {
           _logger->log(Logger::LOG_LEV_ERROR, "OffsetXY on Wfs failed");
           getCommand()->setErrorString( wfs->getErrorString());
           adsec->restoreGain();
           return wfsRes;
        }
    }

    checkCanceled(); // Can throw a CommandCanceledException
  
    adsec->restoreGain();
    return SysCmdSuccess;
}



///////////////////////////// OFFSET Z

bool OffsetZImpl::validateImpl() throw(CommandValidationException) {

   OffsetZ *cmd = (OffsetZ*)getCommand();
   offsetZParams params = cmd->getParams();
   Config_File &cfg = AbstractArbitrator::getInstance()->ConfigDictionary();

   // Only check nominal offset range.
   // Out-of-FoV condition is checked at WFS arbitrator level.
   // Split of big offsets into smaller offsets is done during command execution.

   if (fabs(params.deltaZ) > (float)cfg["MaxOffsetZ"]) {
       string errstr = "OffsetZ out of range (max is "+Utils::itoa((float)cfg["MaxOffsetZ"],5,2)+" mm)";
       _logger->log(Logger::LOG_LEV_ERROR, errstr.c_str());
       getCommand()->setErrorString( errstr);
       return false;
    }

   return true;

}

AbstractSystemCommandResult OffsetZImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   WfsArbSystem* wfs = getWfs();
   AdsecArbSystem* adsec = getAdSec();
   OffsetZ *cmd = (OffsetZ*)getCommand();
   Config_File &cfg = AbstractArbitrator::getInstance()->ConfigDictionary();

   offsetZParams params = cmd->getParams();

   // Reduce gain before starting
   adsec->reduceGain();


   // Split big offsets into smaller offsets
   offsetZParams myoffsetz;
   double zoff = params.deltaZ;
   int zsign = (zoff>0)?1:-1;
   double epsilon = 1e-3;
   double th = 5e-7;
   while (fabs(zoff) > epsilon) {

       // Wait for adsec to be in good condition (no big focus offloads)
       time_t start = time(NULL);
       int timeout=20;
       while (1) {
           sleep(1);
           double focus = adsec->getFocusOffload();
           if (focus < th)
               break;
           if (time(NULL)-start > timeout) {
               string errstr = "secondary mirror focus offload did not converge after "+Utils::itoa(timeout)+" seconds.";
               _logger->log(Logger::LOG_LEV_ERROR, "OffsetZ failed: %s", errstr.c_str());
               getCommand()->setErrorString(errstr);
               adsec->restoreGain();
               return SysCmdError;
           }
           if (getArbitrator()->getStopFlag()) {
               _logger->log(Logger::LOG_LEV_ERROR, "Command stopped by external signal");
               return SysCmdError;
           }
       }

       // Watch out: cfg values are coming as float, not double, and cannot be casted to double
       myoffsetz.deltaZ = min( (float)fabs(zoff), (float)cfg["SplitOffsetZ"]) * zsign;

       wfs->offsetZ(myoffsetz);

       zoff -= myoffsetz.deltaZ;
       _logger->log(Logger::LOG_LEV_INFO, "Moved by %4.1f, remaining offset %4.1f", myoffsetz.deltaZ, zoff);

       // Check whether this step has been successful
       AbstractSystemCommandResult wfsRes = wfs->getCommandResult();
       if (wfsRes != SysCmdSuccess) {
           _logger->log(Logger::LOG_LEV_ERROR, "OffsetZ on Wfs failed");
           getCommand()->setErrorString( wfs->getErrorString());
           adsec->restoreGain();
           return wfsRes;
       }
   }

   adsec->restoreGain();
   checkCanceled(); // Can throw a CommandCanceledException
   return SysCmdSuccess;

}


///////////////////////////// PAUSE

bool PauseImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<Pause has empty validation");
   return true;
}

// Pause command sequence:
//
// 1. Disable TT offloading on AdSec
// 2. Pause loop on WFS side
// 3. Pause loop on AdSec side


AbstractSystemCommandResult PauseImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   AdsecArbSystem* adsec = getAdSec();
   AOArbitrator* aoArb = ((AOArbitrator*)getArbitrator());

   if (aoArb->largeOffset())
       return SysCmdSuccess;


   adsec->enableTTOffload(false);

#ifdef FAKE_PAUSE
   adsec->zeroGain();

#else

   wfs->pause();
   wfsRes = wfs->getCommandResult();

   if (wfsRes != SysCmdSuccess) {
      _logger->log(Logger::LOG_LEV_WARNING, "Pause on Wfs failed");
      getCommand()->setErrorString( wfs->getErrorString());
      return wfsRes;
   }



   adsec->pause();
   adsecRes = adsec->getCommandResult();


   if (adsecRes != SysCmdSuccess) {
      _logger->log(Logger::LOG_LEV_WARNING, "Pause on AdSec failed");
      getCommand()->setErrorString( adsec->getErrorString());

      // Try to resume WFS!!
      wfs->resume();

      return adsecRes;
   }
#endif

   checkCanceled(); // Can throw a CommandCanceledException

   aoArb->_loopOn.Set(LOOP_PAUSED);
   return SysCmdSuccess;
}


///////////////////////////// REFINEAO

bool RefineAOImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<RefineAO has empty validation");
   return true;
}

AbstractSystemCommandResult RefineAOImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   RefineAO *cmd = (RefineAO*)getCommand();

   refineAOParams params = cmd->getParams();
   acquireRefAOResult result;

   // Not implemented
/*
   wfs->refineAO(params, &result);
   adsec->refineAO();

   checkCanceled(); // Can throw a CommandCanceledException

   // Check the result
   AbstractSystemCommandResult wfsRes = wfs->getCommandResult();
   AbstractSystemCommandResult adSecRes = adsec->getCommandResult();
*/
   AbstractSystemCommandResult wfsRes = SysCmdSuccess;
   AbstractSystemCommandResult adSecRes = SysCmdSuccess;


   if(wfsRes == SysCmdSuccess && adSecRes == SysCmdSuccess) {
      AOArbitrator* aoArb = ((AOArbitrator*)getArbitrator());
      aoArb->updateRtdb(&result);
      cmd->setResult( result);
      return SysCmdSuccess;
   }
   else {
      if(wfsRes != SysCmdSuccess) _logger->log(Logger::LOG_LEV_WARNING, "RefineAO on Wfs failed");
      if(adSecRes != SysCmdSuccess) _logger->log(Logger::LOG_LEV_WARNING, "RefineAO on AdSec failed");
      return SysCmdError;
   }
}


///////////////////////////// RESUME

bool ResumeImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<Resume command has empty validation");
   return true;
}

// Resume command sequence:
//
// 1. Resume loop on AdSec side
// 2. Resume loop on WFS side
// 3. Enable TT offloading on AdSec


AbstractSystemCommandResult ResumeImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   AdsecArbSystem* adsec = getAdSec();
   AOArbitrator* aoArb = ((AOArbitrator*)getArbitrator());
 
   if (aoArb->largeOffset())
       return SysCmdSuccess;

#ifdef FAKE_PAUSE
   adsec->setGain(0.1);
   sleep(1);
   adsec->restoreGain();
#else

   adsec->reduceGain();
   adsec->resume();
   adsecRes = adsec->getCommandResult();

   sleep(1);
   adsec->restoreGain();



   if (adsecRes != SysCmdSuccess) {
      _logger->log(Logger::LOG_LEV_WARNING, "Resume on AdSec failed");
      getCommand()->setErrorString( adsec->getErrorString());
      return adsecRes;
   }


   wfs->resume();
   wfsRes = wfs->getCommandResult();

   if (wfsRes != SysCmdSuccess) {
      _logger->log(Logger::LOG_LEV_WARNING, "Resume on Wfs failed");
      getCommand()->setErrorString( wfs->getErrorString());

      // Try to re-pause adsec!
      adsec->pause();

      return wfsRes;
   }

#endif

   adsec->enableTTOffload(true);

   checkCanceled(); // Can throw a CommandCanceledException

   aoArb->_loopOn.Set(LOOP_ON);
   return SysCmdSuccess;
}



bool StartAOImpl::checkStop() { 

    bool stop=false;
    AOArbitrator* aoArb = ((AOArbitrator*)getArbitrator());

    stop = aoArb->checkStopCmd();
    if (stop) {
        getCommand()->setErrorString( "Stopped by the AOI");
        aoArb->doSpeak( "Voice_StartAOAborted");
    }

    return stop;
}


///////////////////////////// START AO

bool StartAOImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<StartAO command has empty validation");
   return true;
}

// StartAO command sequence:
//
// 1. Check the magnitude on the ccd39 and re-configure if needed
// 1. Start a 10 modes loop to get rid of low-order aberrations
// 2. Enable WFS camera lens tracking and wait for the tracking to lock in position
// 3. Stop the 10 modes loop
// 4. Prepare AdSec for closed loop
// 5. Close loop on WFS side
// 6. Enable TT offloading on AdSec
// 7. Enable cameralens centering on WFS
// 8. Tell TCS we are in closed loop (LOOPON variable)

AbstractSystemCommandResult StartAOImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   WfsArbSystem* wfs = getWfs();
   AdsecArbSystem* adsec = getAdSec();
   AOArbitrator* aoArb = ((AOArbitrator*)getArbitrator());

   AbstractSystemCommandResult adSecRes, wfsRes, acqRes;

   acqRes = acquireRef( wfs, adsec, getCommand());
   if (acqRes != SysCmdSuccess)
      return acqRes;

   acquireRefAOResult result  = adsec->getAcquireRefResult();
   aoArb->clearStopCmd(); 

#ifdef CL_CENTER
   // Camera lens centering
   if ((result.CLbase.compare("no")) != 0) {

        // Set WFS and AdSec to CL centering settings
        acquireRefAOResult myResult;
        modifyAOParams modifyaoparams;
        modifyaoparams.freq = result.CLfreq;
        modifyaoparams.binning = -1;
        modifyaoparams.TTmod= 40;
        modifyaoparams.f1spec="-1";
        modifyaoparams.f2spec="-1";

        updateProgress(30,"Setting parameters for camera lens centering");

        wfs->modifyAO(&modifyaoparams, &myResult);
	// gain for camera lens
        adsec->presetAO( "gain0.1_10modi.fits", result.CLbase, result.CLrec, "pureIntegrator", result.CLfreq, 0);

        if (wfs->getCommandResult() != SysCmdSuccess) {
            getCommand()->setErrorString( wfs->getErrorString());
            return wfs->getCommandResult();
        }
        if (adsec->getCommandResult() != SysCmdSuccess) {
            getCommand()->setErrorString( adsec->getErrorString());
            return adsec->getCommandResult();
        }

        updateProgress(40,"Centering camera lens");

        // Close loop and enable TT offloading
        adsec->startAO();
        if (adsec->getCommandResult() != SysCmdSuccess) {
            getCommand()->setErrorString( adsec->getErrorString());
            return adsec->getCommandResult();
        }


        wfs->startAO();
        if (wfs->getCommandResult() != SysCmdSuccess) {
            adsec->stop();
            getCommand()->setErrorString( wfs->getErrorString());
            return wfs->getCommandResult();
        }

        adsec->enableTTOffload(true);
        if (adsec->getCommandResult() != SysCmdSuccess) {
            adSecRes = adsec->getCommandResult();
            getCommand()->setErrorString( adsec->getErrorString());
            wfs->stop();
            adsec->stop();
            return adSecRes;
        }

        // Enable lens tracking
        Wfs_Arbitrator::autoTrackParams autotrackparams;
        autotrackparams.rerotTrack = true;
        autotrackparams.adcTrack = aoArb->getAdcTracking();
        autotrackparams.lensTrack = true;
        wfs->autoTrack(autotrackparams);

        if (wfs->getCommandResult() != SysCmdSuccess) {
            wfsRes = wfs->getCommandResult();
            getCommand()->setErrorString( wfs->getErrorString());
            wfs->stop();
            adsec->stop();
            return wfsRes;
        }

        // Wait for lens centering
        int timeout=60*3;
        time_t start = time(NULL);
        sleep(5);
        while (!wfs->getLensTrackingTarget()) {
            if (time(NULL) - start > timeout) {
                ostringstream oss;
                oss << "Camera lens centering did not converge after " << timeout << " seconds";
                _logger->log(Logger::LOG_LEV_ERROR, oss.str());
                getCommand()->setErrorString(oss.str());
                adsec->enableTTOffload(false);
                wfs->stop();
                adsec->stop();
                return SysCmdRetry;
            }
            if (getArbitrator()->getStopFlag()) {
                _logger->log(Logger::LOG_LEV_ERROR, "Command stopped by external signal");
                return SysCmdError;
            }
            if (aoArb->checkStopCmd())
                break;
            sleep(1);
        }

        if (checkStop()) {
             wfs->stop();
             adsec->stop();
            return SysCmdRetry;
        }

        if (wfs->getCommandResult() != SysCmdSuccess) {
            wfsRes = wfs->getCommandResult();
            getCommand()->setErrorString( wfs->getErrorString());
            wfs->stop();
            adsec->stop();
            return wfsRes;
        }

        // Open loop
        adsec->enableTTOffload(false);
        if (adsec->getCommandResult() != SysCmdSuccess) {
            adSecRes = adsec->getCommandResult();
            getCommand()->setErrorString( adsec->getErrorString());
            wfs->stop();
            adsec->stop();
            return adSecRes;
        }
        
        wfs->stop();
        sleep(1);
        adsec->stop(true);

        if (wfs->getCommandResult() != SysCmdSuccess) {
            getCommand()->setErrorString( wfs->getErrorString());
            return wfs->getCommandResult();
        }
        if (adsec->getCommandResult() != SysCmdSuccess) {
            getCommand()->setErrorString( adsec->getErrorString());
            return adsec->getCommandResult();
        }

        
        // Remove lens tracking
        autotrackparams.rerotTrack = true;
        autotrackparams.adcTrack = aoArb->getAdcTracking();
        autotrackparams.lensTrack = true;
        wfs->autoTrack(autotrackparams);

        if (wfs->getCommandResult() != SysCmdSuccess) {
            aoArb->doSpeak( "Voice_StartAOAborted");
            wfsRes = wfs->getCommandResult();
            getCommand()->setErrorString( wfs->getErrorString());
            wfs->stop();
            adsec->stop();
            return wfsRes;
        }
        
        updateProgress(80,"Setting AO loop parameters on WFS");

       if (checkStop()) return SysCmdRetry;

        // Restore WFS settings
        modifyaoparams.freq = result.freq;
        modifyaoparams.binning = result.nBins;
        modifyaoparams.TTmod= result.ttMod;
        modifyaoparams.f1spec="-1";
        modifyaoparams.f2spec="-1";
        wfs->modifyAO(&modifyaoparams, &myResult);
        
        if (wfs->getCommandResult() != SysCmdSuccess) {
            getCommand()->setErrorString( wfs->getErrorString());
            return wfs->getCommandResult();
        }


   }
#endif // CL_CENTER



   // Seeing limited shutdown
#ifdef SIMULATE_ATMOSPHERE
   stopSeeingLimited();
#endif

   updateProgress(90,"Setting AO loop parameters on AdSec");
        
   adsec->presetAO( result.gain, result.base, result.rec, result.filtering, result.freq, result.decimation);

   adSecRes = adsec->getCommandResult();
   if (adSecRes != SysCmdSuccess) {
      if(adSecRes != SysCmdSuccess) _logger->log(Logger::LOG_LEV_WARNING, "PresetAO on AdSec failed");
      getCommand()->setErrorString( adsec->getErrorString());
      return adSecRes;
   }


   try {
      aoArb->updateRtdb(&result);
   } catch (AOException *e) {
      _logger->log(Logger::LOG_LEV_ERROR, "updateRtdb() failed: %s\n", e->what().c_str());
   } 

   if (checkStop()) return SysCmdRetry;

   adsec->startAO();
   adSecRes =  adsec->getCommandResult();
   
   checkCanceled(); // Can throw a CommandCanceledException

   if (adSecRes != SysCmdSuccess) {
      _logger->log(Logger::LOG_LEV_WARNING, "StartAO on AdSec failed");
      getCommand()->setErrorString( adsec->getErrorString());
      return adSecRes;
   }

   bool failed=false;

   try {
      wfs->startAO();
      checkCanceled(); // Can throw a CommandCanceledException
      wfsRes = wfs->getCommandResult();
      
      if (wfsRes != SysCmdSuccess) {
         failed=true;
         _logger->log(Logger::LOG_LEV_WARNING, "StartAO on WFS failed");
         getCommand()->setErrorString( wfs->getErrorString());
      }
   } catch (AOException &e) {
      // Catch everything here because we need to stop the AdSec
      failed=true;
      _logger->log(Logger::LOG_LEV_WARNING, "StartAO on WFS failed: %s", e.what().c_str());
      wfsRes = SysCmdError;
      getCommand()->setErrorString( e.what());
   }


   if (failed) {
      // Try to stop the AdSec!
      adsec->stop();
   } else {
      adsec->enableTTOffload(true);
      aoArb->_loopOn.Set(LOOP_ON);

      sleep(10);

      // Wait for lens centering
      int timeout=60*3;
      time_t start = time(NULL);
      while (!wfs->getLensTrackingTarget()) {
          if (time(NULL) - start > timeout) {
              ostringstream oss;
              oss << "Camera lens centering did not converge after " << timeout << " seconds";
              _logger->log(Logger::LOG_LEV_ERROR, oss.str());
              getCommand()->setErrorString(oss.str());
              adsec->enableTTOffload(false);
              wfs->stop();
              adsec->stop();
              return SysCmdRetry;
          }
          if (getArbitrator()->getStopFlag()) {
              _logger->log(Logger::LOG_LEV_ERROR, "Command stopped by external signal");
              return SysCmdError;
          }
          sleep(1);
           if (aoArb->checkStopCmd())
               break;
      }

        if (checkStop()) {
             wfs->stop();
             adsec->stop();
            return SysCmdRetry;
        }

      // Disable lens tracking
      Wfs_Arbitrator::autoTrackParams autotrackparams;
      autotrackparams.rerotTrack = true;
      autotrackparams.adcTrack = aoArb->getAdcTracking();
      autotrackparams.lensTrack = false;
      wfs->autoTrack(autotrackparams);

   
      wfs->optimizeGain();
      checkCanceled(); // Can throw a CommandCanceledException
      wfsRes = wfs->getCommandResult();
     
      if (wfsRes != SysCmdSuccess) {
         _logger->log(Logger::LOG_LEV_WARNING, "OptimizeGain on WFS failed");
         getCommand()->setErrorString( wfs->getErrorString());
         failed=true;
      }

      // Re-enable lens tracking
      autotrackparams.rerotTrack = true;
      autotrackparams.adcTrack = aoArb->getAdcTracking();
      autotrackparams.lensTrack = true;
      wfs->autoTrack(autotrackparams);

   }

   if (!failed)
      aoArb->doSpeak( "Voice_LoopIsLocked");

   return wfsRes;
}

///////////////////////////// STOP AO

bool StopImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<Stop Command has an empty (extra) validation>");
   return true;
}

// StopAO command sequence:
//
// 1. Disable TT offloading on AdSec
// 2. Open loop on WFS side
// 3. Open loop on AdSec side


AbstractSystemCommandResult StopImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   WfsArbSystem* wfs = getWfs();
   AdsecArbSystem* adsec = getAdSec();
   AOArbitrator* aoArb = ((AOArbitrator*)getArbitrator());

   string curState = ((AOArbitrator*)getArbitrator())->fsmState();

   AbstractSystemCommandResult adSecRes, wfsRes;

   if ((curState == "LoopClosed") || (curState == "LoopSuspended")
       || (curState == "RefAcquired")) {

      adsec->enableTTOffload(false);

      wfs->stop();
      wfsRes = wfs->getCommandResult();

      checkCanceled(); // Can throw a CommandCanceledException

      if (wfsRes != SysCmdSuccess) {
         _logger->log(Logger::LOG_LEV_WARNING, "Stop on WFS failed");
         getCommand()->setErrorString( wfs->getErrorString());
         return wfsRes;
      }

      msleep(100);

      adsec->stop();
      adSecRes = adsec->getCommandResult();
   
      // We should be now in open loop
      aoArb->_loopOn.Set(LOOP_OFF);

      checkCanceled(); // Can throw a CommandCanceledException

      if (adSecRes != SysCmdSuccess) {
         _logger->log(Logger::LOG_LEV_WARNING, "Stop on AdSec failed");
         getCommand()->setErrorString( adsec->getErrorString());
         return adSecRes;
      }

      return SysCmdSuccess;

   }
   else if (curState == "Ready") {
      _logger->log(Logger::LOG_LEV_DEBUG, "Nothing to Stop in mode Ready");
      return SysCmdSuccess;
   }
   else if (curState == "ReadyToAcquire") {
      wfs->backToOperate();
      wfsRes = wfs->getCommandResult();

      if (wfsRes != SysCmdSuccess) {
         _logger->log(Logger::LOG_LEV_WARNING, "Stop on WFS failed");
         getCommand()->setErrorString( wfs->getErrorString());
         return wfsRes;
      }
      return SysCmdSuccess;
   }
   else {
      _logger->log(Logger::LOG_LEV_INFO, "Nothing to do for Stop command in mode %s", curState.c_str());
      return SysCmdSuccess;
   }

}




///////////////////////////// USER PANIC

bool UserPanicImpl::validateImpl() throw(CommandValidationException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<UserPanic Command has an empty (extra) validation>");
   return true;
}

AbstractSystemCommandResult UserPanicImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException) {

   WfsArbSystem* wfs = getWfs();
   AdsecArbSystem* adsec = getAdSec();

   _thExecWfs = new boost::thread(boost::bind(&WfsArbSystem::userPanic, wfs));
   _thExecAdSec = new boost::thread(boost::bind(&AdsecArbSystem::userPanic, adsec));   // At the moment does nothing

   _thExecWfs->join();
   _thExecAdSec->join();

   delete _thExecWfs;
   delete _thExecAdSec;

   checkCanceled(); // Can throw a CommandCanceledException

   // Check the result
   AbstractSystemCommandResult wfsRes = wfs->getCommandResult();
   AbstractSystemCommandResult adSecRes = adsec->getCommandResult();

   if(wfsRes == SysCmdSuccess && adSecRes == SysCmdSuccess) {
      return SysCmdSuccess;
   }
   else {
      if(wfsRes != SysCmdSuccess) _logger->log(Logger::LOG_LEV_WARNING, "UserPanic on Wfs failed");
      if(adSecRes != SysCmdSuccess) _logger->log(Logger::LOG_LEV_WARNING, "UserPanic on AdSec failed");
      return SysCmdError;
   }
}












