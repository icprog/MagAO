#include "wfsarbitrator/WfsCommandsImpl.h"
#include "Paths.h"

// Includes to send command to AdSecArbitrator
#include "arblib/adSecArb/AdSecCommandsExport.h"

// Includes global constants WFS_ON and WFS_OFF
#include "../../lib/arblib/aoArb/AOArbConst.h"


using namespace Arcetri::Wfs_Arbitrator;

//////////////////////////////// OPERATE

bool OperateImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Operate Command has an empty (extra) validation>");
   powerOnParams params =  ((Operate*)getCommand())->getParams();
	_logger->log(Logger::LOG_LEV_DEBUG, " Config: %s", params.config.c_str());
	_logger->log(Logger::LOG_LEV_DEBUG, " Board setup: %s", params.boardSetup.c_str());
	_logger->log(Logger::LOG_LEV_DEBUG, " Optical Setup: %s", (params.opticalSetup ? "YES" :"NO"));
	return true;
}

AbstractSystemCommandResult OperateImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Operate Command>");

	WfsSystem* wfs = getWfs();

   OptLoopDiagSystem *opt = getOptLoopDiag();
   opt->enableAntiDrift(false);

	powerOnParams params;
	// The wrapped command is 'Operate': it carry the config
	if(getCommand()->getCode() == WfsOpCodes::OPERATE_CMD) {
		params =  ((Operate*)getCommand())->getParams();
	}
	// The wrapped command is 'RecoverFailure': use the last configuration!
	else {
		params.config = "LAST";
	}

   WfsArbitrator *arb =  (WfsArbitrator*)getArbitrator();

   // An empty config string causes a dummy Operate command which goes to Operating state without changing anything

   if (params.config != "") {
      wfs->operate(params);
      getCommand()->setErrorString( wfs->getErrorString());
      checkCanceled(); // Can throw a CommandCanceledException
      if (wfs->getCommandResult() == SysCmdSuccess)
         arb->_led.Set(WFS_ON);
      return wfs->getCommandResult();
   } else {
      getCommand()->setErrorString("No error");
      arb->_led.Set(WFS_ON);
      return SysCmdSuccess;
   }


}


//////////////////////////////// OFF

bool OffImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Off Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult OffImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Off Command>");

	WfsSystem* wfs = getWfs();

   WfsArbitrator *arb =  (WfsArbitrator*)getArbitrator();
   arb->enableRerotTracking(false);
   arb->enableAdcTracking(false);
   arb->enableLensTracking(false);

   OptLoopDiagSystem *opt = getOptLoopDiag();
   opt->enableAntiDrift(false);

   wfs->off();
   getCommand()->setErrorString( wfs->getErrorString());

	checkCanceled(); // Can throw a CommandCanceledException

   if (wfs->getCommandResult() == SysCmdSuccess) {
      WfsArbitrator *arb =  (WfsArbitrator*)getArbitrator();
      arb->_led.Set(WFS_OFF);
   }

	return wfs->getCommandResult();
}


//////////////////////////////// PREPARE ACQUIRE REF

bool PrepareAcquireRefImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PrepareAcquireRef Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult PrepareAcquireRefImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PrepareAcquireRef Command>");

   WfsSystem* wfs = getWfs();
   WfsArbitrator *arb =  (WfsArbitrator*)getArbitrator();
   arb->enableRerotTracking( true);


   OptLoopDiagSystem *opt = getOptLoopDiag();
   opt->enableAntiDrift(false);

   // Retrieve parameters
   prepareAcquireRefParams params = ((PrepareAcquireRef*)getCommand())->getParams();
   wfs->setInstrument(params.Instr);

   prepareAcquireRefResult *outputParams =  ((PrepareAcquireRef*)getCommand())->getOutputParams();

   double x,y;
   arb->rotate( params.ROCoords[0], params.ROCoords[1], &x, &y);
   params.ROCoords[0] = x;
   params.ROCoords[1] = y;

   wfs->prepareAcquireRef( params, outputParams);

   checkCanceled(); // Can throw a CommandCanceledException
   getCommand()->setErrorString( wfs->getErrorString());
   return wfs->getCommandResult();
}


//////////////////////////////// GetTVSnap

bool GetTVSnapImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<GetTVSnap Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult GetTVSnapImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<GetTVSnap Command>");

	WfsSystem* wfs = getWfs();

	// Retrieve parameters
   snapParams *outputParams =  ((GetTVSnap*)getCommand())->getOutputParams();

   wfs->getTVSnap(outputParams);

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}

//////////////////////////////// CheckRef

bool CheckRefImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CheckRef Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult CheckRefImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CheckRef Command>");

	WfsSystem* wfs = getWfs();

	// Retrieve parameters
   checkRefResult *outputParams =  ((CheckRef*)getCommand())->getOutputParams();

   wfs->checkRef(outputParams);

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}

//////////////////////////////// SetSource

bool SetSourceImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetSource Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult SetSourceImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetSource Command>");

	WfsSystem* wfs = getWfs();

	// Retrieve parameters
   setSourceParams params =  ((SetSource*)getCommand())->getParams();

   wfs->setSource( params);

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}



//////////////////////////////// SAVE STATUS

bool SaveStatusImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SaveStatus Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult SaveStatusImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SaveStatus Command>");

	WfsSystem* wfs = getWfs();

	// Retrieve parameters
	saveStatusParams params = ((SaveStatus*)getCommand())->getParams();

   wfs->saveStatus(params);

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}


//////////////////////////////// SAVE OPT LOOP DATA

bool SaveOptLoopDataImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SaveOptLoopData Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult SaveOptLoopDataImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SaveOptLoopData Command>");

	OptLoopDiagSystem* loopdiag = getOptLoopDiag();
   WfsSystem *wfs = getWfs();

	// Retrieve parameters
	saveOptLoopDataParams params = ((SaveOptLoopData*)getCommand())->getParams();

   AbstractSystemCommandResult res = loopdiag->save(params, false, getCommand()->getSender());
   if (res != SysCmdSuccess)
      return res;

   string trackingNum = loopdiag->getTrackingNum();
   string path = Paths::AdSecDataDir( true, trackingNum, trackingNum);

   saveStatusParams statusParams;
   statusParams._outputFile = path + "wfs.fits";
   wfs->saveStatus(statusParams);
   res = wfs->getCommandResult();
   if (res != SysCmdSuccess)
      return res;

   boost::thread* irtc = NULL;
   boost::thread* pisces = NULL;
   boost::thread* psf = NULL;

   if (params._saveIrtc) {
      string irtcFilename = path + "irtc.fits";
      irtc = new boost::thread(boost::bind(&WfsSystem::saveIrtc, wfs, irtcFilename, params._nFramesIrtc));
   }

   if (params._savePisces) {
      string piscesFilename = path + "pisces.fits";
      pisces = new boost::thread(boost::bind(&WfsSystem::savePisces, wfs, piscesFilename, params._nFramesPisces));
   }

   if (params._savePsf) {
      string psfFilename = path + "psf.fits";
      psf = new boost::thread(boost::bind(&WfsSystem::savePsf, wfs, psfFilename, params._nFramesPsf));
   }

   loopdiag->join();

   if (irtc)
	irtc->join();
   if (pisces)
	pisces->join();
   if (psf)
	psf->join();

   ((SaveOptLoopData*)getCommand())->setTrackingNum( trackingNum);

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}

bool SaveOptLoopDataImpl::cancelImpl() throw(CommandTerminationException) {

   // Unblock any joins()

	OptLoopDiagSystem* loopdiag = getOptLoopDiag();
   loopdiag->cancelCommand();
   return true;
}


//////////////////////////////// ACQUIRE REF

bool AcquireRefImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<AcquireRef Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult AcquireRefImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<AcquireRef Command>");

	WfsSystem* wfs = getWfs();

   acquireRefResult *outputParams =  ((AcquireRef*)getCommand())->getOutputParams();
   wfs->acquireRef(outputParams);

   checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}


//////////////////////////////// CLOSE LOOP

bool CloseLoopImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CloseLoop Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult CloseLoopImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CloseLoop Command>");

	WfsSystem* wfs = getWfs();

	AbstractSystemCommandResult res = wfs->closeLoop();
   if (res == SysCmdSuccess) {
	   MastDiagnSystem* mastDiagn = getMastDiagn();
      mastDiagn->closeLoop();
   }

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}

//////////////////////////////// PREPARE ADSEC

bool PrepareAdsecImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PrepareAdsec Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult PrepareAdsecImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PrepareAdsec Command>");

	WfsSystem* wfs = getWfs();
	AdSecSystem* adSec = getAdSec();
   int decimation = wfs->getDecimation();

   if (!adSec->check()) {
      getCommand()->setErrorString("No connection with AdSec");
      return SysCmdRetry;
   }

   // Inform the AdSecArbitrator that we are closing the loop and set the decimation
   AdSec_Arbitrator::dataDecimationParams params;
   params._decimation = decimation;

   try {
      AbstractSystemCommandResult res = adSec->setDataDecimation(params);
      if (res != SysCmdSuccess) {
         getCommand()->setErrorString( adSec->getErrorString());
         return res;
      }

      AdSec_Arbitrator::runAoParams runao_params;
      runao_params._decimation = decimation;
      runao_params._loopFreq = wfs->getLoopFreq();

      res = adSec->closeLoop( runao_params);
      getCommand()->setErrorString( adSec->getErrorString());

      return res;

   } catch (ArbitratorInterfaceException &e) {
      getCommand()->setErrorString( e.what());
      return SysCmdRetry;
   }

}


//////////////////////////////// PAUSE LOOP

bool PauseLoopImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PauseLoopImpl Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult PauseLoopImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PauseLoopImpl Command>");

   WfsSystem* wfs = getWfs();

#ifdef FAKE_PAUSE
   WfsArbitrator *arb =  (WfsArbitrator*)getArbitrator();
   arb->enableLensTracking(false);
#else

   wfs->pauseLoop();

   if (wfs->getCommandResult() == SysCmdSuccess) {
      MastDiagnSystem* mastDiagn = getMastDiagn();
      mastDiagn->openLoop();
   }
#endif

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}


//////////////////////////////// ENABLE DISTURB

bool EnableDisturbImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<EnableDisturb Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult EnableDisturbImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<EnableDisturb Command>");

	WfsSystem* wfs = getWfs();

	// Retrieve parameters
	enableDisturbParams params = ((EnableDisturb*)getCommand())->getParams();

   wfs->enableDisturb(params);

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}


//////////////////////////////// REFINE AO

bool ModifyAOImpl::validateImpl() throw(CommandValidationException) {
   modifyAOparams params =  ((ModifyAO*)getCommand())->getParams();
   Config_File &cfg = AbstractArbitrator::getInstance()->ConfigDictionary();

   // Check parameters
   if (params.freq != -1)
      if ((params.freq < (float) cfg["MinLoopFreq"])  || (params.freq > (float) cfg["MaxLoopFreq"])) {
         string errstr = "Loop frequency "+Utils::itoa( params.freq, 5, 1)+" is out of range (valid values: "+Utils::itoa( (float)cfg["MinLoopFreq"], 5, 1) +" - " + Utils::itoa( (float) cfg["MaxLoopFreq"], 5, 1);
	 _logger->log(Logger::LOG_LEV_ERROR, errstr.c_str());
         getCommand()->setErrorString( errstr);
         return false;
      }

   // [TODO] Move limits to configuration file
   if (params.Binning != -1) 
      if ((params.Binning < 1) || (params.Binning > 5)) {
         string errstr = "Binning "+Utils::itoa(params.Binning)+" is out of range (valid values: 1-5)";
         _logger->log(Logger::LOG_LEV_ERROR, errstr.c_str());
         getCommand()->setErrorString( errstr);
         return false;
      }


   // [TODO] Move limits to configuration file
      if ((params.TTmod < 0) || (params.TTmod > 45)) {
         string errstr = "TT modulation "+Utils::itoa(params.TTmod, 5, 1)+" is out of range (valid values: 0-20)";
	 _logger->log(Logger::LOG_LEV_ERROR, errstr.c_str());
         getCommand()->setErrorString( errstr);
         return false;
      }


	return true;
}

AbstractSystemCommandResult ModifyAOImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {

   _logger->log(Logger::LOG_LEV_DEBUG, "<ModifyAO Command>");
   WfsSystem* wfs = getWfs();

   // Retrieve parameters
   modifyAOparams params = ((ModifyAO*)getCommand())->getParams();
   modifyAOparams *result = ((ModifyAO*)getCommand())->getOutputParams();

   if (params.TTmod > 20)
      params.TTmod =20;

   wfs->modifyAO( params, result);

   checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
   return wfs->getCommandResult();
}


//////////////////////////////// REFINE LOOP

bool RefineLoopImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<RefineLoop Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult RefineLoopImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<RefineLoop Command>");

	WfsSystem* wfs = getWfs();

	// Retrieve parameters
	wfsLoopParams wfsLoopPar = ((RefineLoop*)getCommand())->getWfsLoopParameters();

	_thExecWfs = new boost::thread(boost::bind(&WfsSystem::refineLoop, wfs, wfsLoopPar));

	// This waits for the command to terminate.
	// There are 2 possibilities:
	// 	- Execution terminates with success/error
	//	- Execution terminates in advance because of a "cancel" command
	_thExecWfs->join();
	delete _thExecWfs;

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}


//////////////////////////////// RESUME LOOP

bool ResumeLoopImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<ResumeLoopImpl Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult ResumeLoopImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<ResumeLoopImpl Command>");


   WfsSystem* wfs = getWfs();

#ifdef FAKE_PAUSE
   WfsArbitrator *arb =  (WfsArbitrator*)getArbitrator();
   arb->enableLensTracking(true);
#else

   wfs->resumeLoop();
  
   if (wfs->getCommandResult() == SysCmdSuccess) { 
      MastDiagnSystem* mastDiagn = getMastDiagn();
      mastDiagn->closeLoop();
   }

#endif

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}



//////////////////////////////// STOP LOOP

bool StopLoopImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<StopLoop Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult StopLoopImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<StopLoop Command>");

	WfsSystem* wfs = getWfs();

   AbstractSystemCommandResult res = wfs->stopLoop();

   if (res == SysCmdSuccess) {
	   MastDiagnSystem* mastDiagn = getMastDiagn();
      mastDiagn->openLoop();
   }

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return res;
}




//////////////////////////////// STOP LOOP ADSEC

bool StopLoopAdsecImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<StopLoopAdsec Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult StopLoopAdsecImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<StopLoopAdsec Command>");

   AdSecSystem* adSec = getAdSec();
   stopLoopAdsecParams params = ((StopLoopAdsec*)getCommand())->getParams();

   if (!adSec->check()) {
      getCommand()->setErrorString("No connection with AdSec");
      return SysCmdRetry;
   }


   AbstractSystemCommandResult res1 = adSec->openLoop( params.hold);
   getCommand()->setErrorString( adSec->getErrorString());

	checkCanceled(); // Can throw a CommandCanceledException
	return res1;
}



////////////////////////////////  CALIBRATE HO DARK

bool CalibrateHODarkImpl::validateImpl() throw(CommandValidationException) {
   int nframes =  ((CalibrateHODark*)getCommand())->getNframes();
   Config_File &cfg = AbstractArbitrator::getInstance()->ConfigDictionary();
   if ((nframes < (int) cfg["MinHODark"])  || (nframes > (int) cfg["MaxHODark"])) {
      _logger->log(Logger::LOG_LEV_ERROR, "CalibrateHODark Command validation failed: value %d is out of accepted bounds %d-%d", nframes, (int) cfg["MinHODark"], (int) cfg["MaxHODark"]);
      return false;
   }

   _logger->log(Logger::LOG_LEV_DEBUG, "CalibrateHODark validation OK - nframes = %d", nframes);

	return true;
}

AbstractSystemCommandResult CalibrateHODarkImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CalibrateHODark Command>");

   WfsSystem* wfs = getWfs();

   // Retrieve parameters
   int nframes = ((CalibrateHODark*)getCommand())->getNframes();

   wfs->calibrateHODark( nframes);

   checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
   return wfs->getCommandResult();
}

////////////////////////////////  CALIBRATE TV DARK

bool CalibrateTVDarkImpl::validateImpl() throw(CommandValidationException) {

   int nframes =  ((CalibrateTVDark*)getCommand())->getNframes();
   Config_File &cfg = AbstractArbitrator::getInstance()->ConfigDictionary();
   if ((nframes < (int) cfg["MinTVDark"])  || (nframes > (int) cfg["MaxTVDark"])) {
      _logger->log(Logger::LOG_LEV_ERROR, "CalibrateTVDark Command validation failed: value %d is out of accepted bounds %d-%d", nframes, (int) cfg["MinTVDark"], (int) cfg["MaxTVDark"]);
      return false;
   }

   _logger->log(Logger::LOG_LEV_DEBUG, "CalibrateTVDark validation OK - nframes = %d", nframes);

	return true;
}

AbstractSystemCommandResult CalibrateTVDarkImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CalibrateTVDark Command>");

	WfsSystem* wfs = getWfs();

	// Retrieve parameters
	int nframes = ((CalibrateTVDark*)getCommand())->getNframes();

   wfs->calibrateTVDark(nframes);

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}

////////////////////////////////  CALIBRATE IRTC DARK

bool CalibrateIRTCDarkImpl::validateImpl() throw(CommandValidationException) {

   int nframes =  ((CalibrateIRTCDark*)getCommand())->getNframes();
   Config_File &cfg = AbstractArbitrator::getInstance()->ConfigDictionary();
   if ((nframes < (int) cfg["MinIRTCDark"])  || (nframes > (int) cfg["MaxIRTCDark"])) {
      _logger->log(Logger::LOG_LEV_ERROR, "CalibrateIRTCDark Command validation failed: value %d is out of accepted bounds %d-%d", nframes, (int) cfg["MinIRTCDark"], (int) cfg["MaxIRTCDark"]);
      return false;
   }

   _logger->log(Logger::LOG_LEV_DEBUG, "CalibrateIRTCDark validation OK - nframes = %d", nframes);

	return true;
}

AbstractSystemCommandResult CalibrateIRTCDarkImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CalibrateIRTCDark Command>");

	WfsSystem* wfs = getWfs();

	// Retrieve parameters
	int nframes = ((CalibrateIRTCDark*)getCommand())->getNframes();

   wfs->calibrateIRTCDark(nframes);

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}

////////////////////////////////  CALIBRATE PISCES DARK

bool CalibratePISCESDarkImpl::validateImpl() throw(CommandValidationException) {

   int nframes =  ((CalibratePISCESDark*)getCommand())->getNframes();
   Config_File &cfg = AbstractArbitrator::getInstance()->ConfigDictionary();
   if ((nframes < (int) cfg["MinPISCESDark"])  || (nframes > (int) cfg["MaxPISCESDark"])) {
      _logger->log(Logger::LOG_LEV_ERROR, "CalibratePISCESDark Command validation failed: value %d is out of accepted bounds %d-%d", nframes, (int) cfg["MinPISCESDark"], (int) cfg["MaxPISCESDark"]);
      return false;
   }

   _logger->log(Logger::LOG_LEV_DEBUG, "CalibratePISCESDark validation OK - nframes = %d", nframes);

	return true;
}

AbstractSystemCommandResult CalibratePISCESDarkImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CalibratePISCESDark Command>");

	WfsSystem* wfs = getWfs();

	// Retrieve parameters
	int nframes = ((CalibratePISCESDark*)getCommand())->getNframes();

   wfs->calibratePISCESDark(nframes);

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}

////////////////////////////////  CALIBRATE MOVEMENTS

bool CalibrateMovementsImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CalibrateMovements Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult CalibrateMovementsImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CalibrateMovements Command>");

	WfsSystem* wfs = getWfs();

	_thExecWfs = new boost::thread(boost::bind(&WfsSystem::calibrateMovements, wfs));

	// This waits for the command to terminate.
	// There are 2 possibilities:
	// 	- Execution terminates with success/error
	//	- Execution terminates in advance because of a "cancel" command
	_thExecWfs->join();
	delete _thExecWfs;

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}

////////////////////////////////  CALIBRATE SLOPENULL

bool CalibrateSlopenullImpl::validateImpl() throw(CommandValidationException) {
   int nframes =  ((CalibrateSlopenull*)getCommand())->getNframes();
   Config_File &cfg = AbstractArbitrator::getInstance()->ConfigDictionary();
   if ((nframes < (int) cfg["MinSlopenull"])  || (nframes > (int) cfg["MaxSlopenull"])) {
      _logger->log(Logger::LOG_LEV_ERROR, "CalibrateSlopenull Command validation failed: value %d is out of accepted bounds %d-%d", nframes, (int) cfg["MinSlopenull"], (int) cfg["MaxSlopenull"]);
      return false;
   }

   _logger->log(Logger::LOG_LEV_DEBUG, "CalibrateSlopenull validation OK - nframes = %d", nframes);

	return true;
}

AbstractSystemCommandResult CalibrateSlopenullImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CalibrateSlopenull Command>");

	WfsSystem* wfs = getWfs();

	// Retrieve parameters
	int nframes = ((CalibrateSlopenull*)getCommand())->getNframes();

	_thExecWfs = new boost::thread(boost::bind(&WfsSystem::calibrateSlopenull, wfs, nframes));

	// This waits for the command to terminate.
	// There are 2 possibilities:
	// 	- Execution terminates with success/error
	//	- Execution terminates in advance because of a "cancel" command
	_thExecWfs->join();
	delete _thExecWfs;

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}


//////////////////////////////// OFFSETXY

bool OffsetXYImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<OffsetXY Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult OffsetXYImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<OffsetXY Command>");

	// Retrieve parameters
	offsetXYparams params = ((OffsetXY*)getCommand())->getParams();

// Removed because we are using the FAKE_PAUSE flag on AOArbitrator

   // Check offset request in closed loop
//   string curState = ((WfsArbitrator*)getArbitrator())->fsmState();
//   if (curState == "LoopClosed") {
//      float maxOff = getArbitrator()->ConfigDictionary()["MaxOffsetXYCLoop"];
//      float offset = sqrt( params.offsetX*params.offsetX + params.offsetY*params.offsetY);
//      if ( offset > maxOff) {
//         string errStr = "Offset too large. Max offset in closed loop is " + Utils::itoa( maxOff, 5, 1) + " mm (combined XY), requested " + Utils::itoa(offset, 5, 1);
//         getCommand()->setErrorString( errStr);
//         return SysCmdRetry;
//      }
//   }

   WfsSystem *wfs = getWfs();
   WfsArbitrator *arb =  (WfsArbitrator*)getArbitrator();

   double x,y;
   arb->rotate( params.offsetX, params.offsetY, &x, &y);
   params.offsetX = x;
   params.offsetY = y;

   wfs->offsetXY(params);

   checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}


//////////////////////////////// OFFSETZ

bool OffsetZImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<OffsetZ Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult OffsetZImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<OffsetZ Command>");

	// Retrieve parameters
	offsetZparams params = ((OffsetZ*)getCommand())->getParams();
   WfsSystem *wfs = getWfs();

	wfs->offsetZ(params);

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}

////////////////////////////////  OPTIMIZE GAIN

bool OptimizeGainImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<OptimizeGainImpl Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult OptimizeGainImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<OptimizeGainImpl Command>");

   WfsSystem *wfs = getWfs();

   wfs->optimizeGain();

   checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
   return wfs->getCommandResult();
}


//////////////////////////////// ANTI DRIFT

bool AntiDriftImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<AntiDrift Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult AntiDriftImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<AntiDrift Command>");

   // Retrieve parameters
   antiDriftParams params = ((AntiDrift*)getCommand())->getParams();

   OptLoopDiagSystem *opt = getOptLoopDiag();

   opt->enableAntiDrift( params.enable);

   checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( opt->getErrorString());
   return opt->getCommandResult();
}


//////////////////////////////// AUTO TRACK

bool AutoTrackImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<AutoTrack Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult AutoTrackImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<AutoTrack Command>");

	// Retrieve parameters
   autoTrackParams params = ((AutoTrack*)getCommand())->getParams();

   WfsArbitrator *arb =  (WfsArbitrator*)getArbitrator();

   arb->enableRerotTracking( params.rerotTrack);
   arb->enableAdcTracking( params.adcTrack);
   arb->enableLensTracking( params.lensTrack);

   checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString("");
   return SysCmdSuccess;
}


/////////////////////////////// CORRECTMODES

bool CorrectModesImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CorrectModes Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult CorrectModesImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CorrectModes Command>");

	WfsSystem* wfs = getWfs();

	// Retrieve parameters
	correctModesParams params = ((CorrectModes*)getCommand())->getParams();

   wfs->correctModes( params);

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}


//////////////////////////////// EmergencyOff

bool EmergencyOffImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<EmergencyOff Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult EmergencyOffImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<EmergencyOff Command>");

	WfsSystem* wfs = getWfs();

   wfs->emergencyOff();

	checkCanceled(); // Can throw a CommandCanceledException

   getCommand()->setErrorString( wfs->getErrorString());
	return wfs->getCommandResult();
}


