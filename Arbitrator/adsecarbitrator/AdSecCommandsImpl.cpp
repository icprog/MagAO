#include "AdSecCommandsImpl.h"
#include "Utils.h"
#include "Paths.h"

// Includes global constants ADSEC_ON, ADSEC_SAFE and ADSEC_SET
#include "../../lib/arblib/aoArb/AOArbConst.h"

#include <iomanip>

using namespace Arcetri::AdSec_Arbitrator;

//////////////////////////////

bool AoToChopImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<AoToChop Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult AoToChopImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<AoToChop Command>");

	IdlSystem* idl = getIdl();

   idl->aoToChop( getReducedTimeout());

	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool AoToChopImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<AoToChop Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

/* L.F. removed

bool AoToSlImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<AoToSl Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult AoToSlImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<AoToSl Command>");

	IdlSystem* idl = getIdl();

   idl->aoToSl( getReducedTimeout());

	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool AoToSlImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<AoToSl Command undo has an empty implementation>");
	return false;
}
  end of remove */
//////////////////////////////

bool ApplyCommandsImpl::validateImpl() throw(CommandValidationException) {
   applyCommandParams params = ((ApplyCommands*)getCommand())->getParams();

   // Must be a .fits
   if (!Utils::hasExtension( params.commandsFile, ".fits")) {
      _logger->log(Logger::LOG_LEV_ERROR, "ApplyCommands validation failed: file does not have .fits extension");
      return false;
   }
   _logger->log(Logger::LOG_LEV_DEBUG, "ApplyCommands  validation ok (.fits extension present)");
   return true;

}

AbstractSystemCommandResult ApplyCommandsImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<ApplyCommands Command>");

	IdlSystem* idl = getIdl();

	applyCommandParams params = ((ApplyCommands*)getCommand())->getParams();

   idl->applyCommands( getReducedTimeout(), params);

	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool ApplyCommandsImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<ApplyCommands Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

//////////////////////////////

bool RecoverSkipFrameImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<RecoverSkipFrame Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult RecoverSkipFrameImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<RecoverSkipFrame Command>");

	IdlSystem* idl = getIdl();

   idl->recoverSkipFrame( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool RecoverSkipFrameImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<RecoverSkipFrameImpl Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool ChopToAoImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<ChopToAoImpl Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult ChopToAoImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<ChopToAoImpl Command>");

	IdlSystem* idl = getIdl();

   idl->chopToAo( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool ChopToAoImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<ChopToAoImpl Command undo has an empty implementation>");
	return false;
}

//////////////////////////////
/* L.F. removed

bool ChopToSlImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<ChopToSl Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult ChopToSlImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<ChopToSl Command>");

	IdlSystem* idl = getIdl();

   idl->chopToSl( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool ChopToSlImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<ChopToSl Command undo has an empty implementation>");
	return false;
}
  end of remove */

//////////////////////////////

bool LoadProgramImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<Load Program Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult LoadProgramImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Load Program Command>");

	IdlSystem* idl = getIdl();

   idl->loadProgram( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
   else {
      AdSecArbitrator* adSecArb = ((AdSecArbitrator*)getArbitrator());
      adSecArb->_varLed->Set(ADSEC_SAFE);
   }
	return idl->getCommandResult();
}

bool LoadProgramImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<Load Program Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool OffImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<Off Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult OffImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Off Command>");
	IdlSystem* idl = getIdl();

   idl->powerOff( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
   else {
      AdSecArbitrator* adSecArb = ((AdSecArbitrator*)getArbitrator());
      adSecArb->_varLed->Set(ADSEC_OFF);
   }
	return idl->getCommandResult();
}

bool OffImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<Off Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool OnImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<On Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult OnImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<On Command>");
	IdlSystem* idl = getIdl();

   idl->powerOn( getReducedTimeout());

	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool OnImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<On Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool RecoverFailureImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<RecoverFailure Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult RecoverFailureImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<RecoverFailure Command has an empty doExecuteImpl>");

	IdlSystem* idl = getIdl();

   idl->recoverFailure( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool RecoverFailureImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<RecoverFailure Command undo has an empty implementation>");
	return true;
}

//////////////////////////////

bool ResetImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<Reset Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult ResetImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Reset Command>");

	IdlSystem* idl = getIdl();

   idl->reset( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool ResetImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<Reset Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool RestImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<Rest Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult RestImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Rest Command>");

	IdlSystem* idl = getIdl();
    FastDiagnSystem* fastdiag = getFD();
    AdamSystem* adam = getAdam();
try {

    // change diagnostics thresholds
    fastdiag->setDiagThresholdsSet(FASTDIAGNSYSTEM_RIPPING_SET);

    adam->disableCoils();
    idl->rest( getReducedTimeout());

    // restore thresholds
    fastdiag->setDiagThresholdsSet(FASTDIAGNSYSTEM_PREVIOUS_SET);

    checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
   else {
      AdSecArbitrator* adSecArb = ((AdSecArbitrator*)getArbitrator());
      adSecArb->_varLed->Set(ADSEC_SAFE);
   }

    } catch (AOException &e) {
       Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "handleCommand(): Exception: %s", e.what().c_str());
       return SysCmdError;
    }

	return idl->getCommandResult();
}

bool RestImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<Rest Command undo has an empty implementation>");
	return false;
}

//////////////////////////////  RUNAO

bool RunAoImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<RunAo Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult RunAoImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<RunAo Command>");

    IdlSystem* idl = getIdl();
    MastDiagnSystem* md = getMD();
    AdSecArbitrator* adSecArb = ((AdSecArbitrator*)getArbitrator());

    string focalStation = adSecArb->varFocalStation()->Get();
    if (focalStation.compare("null") == 0) {
        getCommand()->setErrorString("No focal station selected");
        return SysCmdRetry;
    }

    if (adSecArb->isGuiding()) {
      _logger->log(Logger::LOG_LEV_WARNING, "Telescope is guiding");
      //getCommand()->setErrorString("Telescope guiding is active. Cannot close loop");
      //  return SysCmdRetry;
    }
 
    runAoParams params = ((RunAo*)getCommand())->getParams();

    _logger->log(Logger::LOG_LEV_INFO, "RunAoImpl(): decimation = %d", params._decimation);

    md->setDecimation( getReducedTimeout(), params._decimation);
    idl->runAo( getReducedTimeout(), params);
    checkCanceled(); // Can throw a CommandCanceledException

    if (idl->getCommandResult() != SysCmdSuccess)
        getCommand()->setErrorString( idl->getErrorString());
    return idl->getCommandResult();
}

bool RunAoImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<RunAo Command undo has an empty implementation>");
	return false;
}

//////////////////////////////  SETZERNIKES

bool SetZernikesImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetZernikes Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult SetZernikesImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetZernikes Command>");

	IdlSystem* idl = getIdl();

	setZernikesParams params = ((SetZernikes*)getCommand())->getParams();

   idl->setZernikes( getReducedTimeout(), params);
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool SetZernikesImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetZernikes Command undo has an empty implementation>");
	return false;
}


//////////////////////////////  PAUSE_AO

bool PauseAoImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<PauseAo Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult PauseAoImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PauseAo Command>");

   AdSecArbitrator* adSecArb = ((AdSecArbitrator*)getArbitrator());

	IdlSystem* idl = getIdl();

   idl->pauseAo( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   adSecArb->checkTTOffload();

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool PauseAoImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<PauseAo Command undo has an empty implementation>");
	return false;
}

//////////////////////////////  RESUME AO

bool ResumeAoImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<ResumeAo Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult ResumeAoImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<ResumeAo Command>");

	IdlSystem* idl = getIdl();
   idl->resumeAo( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool ResumeAoImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<ResumeAo Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool RunChopImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<RunChop Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult RunChopImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<RunChop Command>");

	IdlSystem* idl = getIdl();

   idl->runChop( getReducedTimeout());

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool RunChopImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<RunChop Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool SetFlatAoImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetFlatAo Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult SetFlatAoImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetFlatAo Command>");
   AdSecArbitrator* adSecArb = ((AdSecArbitrator*)getArbitrator());

   double elev=90.0;   // Note: in lab mode we use this value

   if(!adSecArb->checkLabMode()) {  // If in lab mode do not perform checks
                                    // Check swing arm
      switch(adSecArb->checkSWA()) {
         case SwaNotAvailable:
            getCommand()->setErrorString( "No swing arm status value! Cannot set shell");
            return SysCmdRetry;
         case SwaNotDeployed:
            getCommand()->setErrorString( "Swing arm not deployed! Cannot set shell");
            return SysCmdRetry;
         case SwaDeployed:
            break;
      }
                                    // Check elevation
      switch(adSecArb->checkElevation(ElevSet)) {
         case ElevationNotAvailable:
            getCommand()->setErrorString( "No elevation value! Cannot set shell");
            return SysCmdRetry;
         case ElevationBelowThreshold:
            getCommand()->setErrorString( "Elevation value below threshold! Cannot set shell");
            return SysCmdRetry;
         case ElevationOk:
            break;
      }
      elev=adSecArb->getElevation();
   }

	IdlSystem* idl = getIdl();
    adSecArb->varFocalStation()->Set("null");

   idl->setFlatAo( elev, getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
   else {
      AdSecArbitrator* adSecArb = ((AdSecArbitrator*)getArbitrator());
      adSecArb->_varLed->Set(ADSEC_SET);
      adSecArb->setCoilsEnabled(true);
   }

   // For this command, any kind of error is a FAIL that needs the recover failure!
   if (idl->getCommandResult() != SysCmdSuccess)
      return SysCmdError;
   else
      return SysCmdSuccess;
}

bool SetFlatAoImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetFlatAo Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool SetFlatChopImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetFlatChop Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult SetFlatChopImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetFlatChop Command>");

	IdlSystem* idl = getIdl();

	// Retrieve parameters
	flatParams params = ((SetFlatChop*)getCommand())->getParams();

   idl->setFlatChop( getReducedTimeout(), params);
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
   else {
      AdSecArbitrator* adSecArb = ((AdSecArbitrator*)getArbitrator());
      adSecArb->_varLed->Set(ADSEC_SET);
   }
	return idl->getCommandResult();
}

bool SetFlatChopImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetFlatChop Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

/* L.F. removed
bool SetFlatSlImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetFlatSl Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult SetFlatSlImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetFlatSetFlatSlChop Command>");

	IdlSystem* idl = getIdl();

	// Retrieve parameters
	flatParams params = ((SetFlatSl*)getCommand())->getParams();

   idl->setFlatChop( getReducedTimeout(), params);
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool SetFlatSlImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetFlatSl Command undo has an empty implementation>");
	return false;
}
  end of remove */

//////////////////////////////

bool SetGainImpl::validateImpl() throw(CommandValidationException) {

   gainParams params = ((SetGain*)getCommand())->getParams();

   if (!Utils::hasExtension( params._gainFile, ".fits")) {
      _logger->log(Logger::LOG_LEV_ERROR, "SetGain validation failed: file does not have .fits extension");
      return false;
   }
   _logger->log(Logger::LOG_LEV_DEBUG, "SetGain validation ok (.fits extension present)");
   return true;
}

AbstractSystemCommandResult SetGainImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetGain Command>");

   AdSecArbitrator* adSecArb = ((AdSecArbitrator*)getArbitrator());

	IdlSystem* idl = getIdl();

   // Retrieve parameters
   gainParams params = ((SetGain*)getCommand())->getParams();

   idl->setGain( getReducedTimeout(), params);
	checkCanceled(); // Can throw a CommandCanceledException

   adSecArb->checkTTOffload();

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool SetGainImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetGain Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool SaveSlopesImpl::validateImpl() throw(CommandValidationException) {

   saveSlopesParams params = ((SaveSlopes*)getCommand())->getParams();

   if (!Utils::hasExtension( params._filename, ".sav")) {
      _logger->log(Logger::LOG_LEV_ERROR, "SaveSlopes validation failed: file does not have .sav extension");
      return false;
   }
   _logger->log(Logger::LOG_LEV_DEBUG, "SaveSlopes validation ok (.sav extension present)");
	return true;
}

AbstractSystemCommandResult SaveSlopesImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SaveSlopes Command>");

	IdlSystem* idl = getIdl();

   // Retrieve parameters
   saveSlopesParams params = ((SaveSlopes*)getCommand())->getParams();

   idl->saveSlopes( getReducedTimeout(), params);
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool SaveSlopesImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SaveSlopes Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool SaveStatusImpl::validateImpl() throw(CommandValidationException) {

   saveStatusParams params = ((SaveStatus*)getCommand())->getParams();

   if (!Utils::hasExtension( params._filename, ".sav")) {
      _logger->log(Logger::LOG_LEV_ERROR, "SaveStatus validation failed: file does not have .sav extension");
      return false;
   }
   _logger->log(Logger::LOG_LEV_DEBUG, "SaveStatus validation ok (.sav extension present)");
	return true;
}

AbstractSystemCommandResult SaveStatusImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SaveStatus Command>");

	IdlSystem* idl = getIdl();

   // Retrieve parameters
   saveStatusParams params = ((SaveStatus*)getCommand())->getParams();

   idl->saveStatus( getReducedTimeout(), params);
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool SaveStatusImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SaveStatus Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool SetDisturbImpl::validateImpl() throw(CommandValidationException) {
   
   disturbParams params = ((SetDisturb*)getCommand())->getParams();

   // Must be a .fits
   if (!Utils::hasExtension( params._disturbFile, ".fits")) {
      _logger->log(Logger::LOG_LEV_ERROR, "SetDisturb Command validation failed: file does not have .fits extension");
      return false;
   }
   _logger->log(Logger::LOG_LEV_DEBUG, "SetDisturb Command validation ok (.fits extension present)");
   return true;
}

AbstractSystemCommandResult SetDisturbImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetDisturb Command>");

	IdlSystem* idl = getIdl();

   // Retrieve parameters
   disturbParams params = ((SetDisturb*)getCommand())->getParams();

   idl->setDisturb( getReducedTimeout(), params);

	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool SetDisturbImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetDisturb Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool SetDataDecimationImpl::validateImpl() throw(CommandValidationException) {
   
   dataDecimationParams params = ((SetDataDecimation*)getCommand())->getParams();

   if ((params._decimation <0) || (params._decimation>10)) {
      _logger->log(Logger::LOG_LEV_ERROR, "SetDataDecimation Command validation failed (decimation outside allowed bounds)");
      return false;
      }
   _logger->log(Logger::LOG_LEV_DEBUG, "SetDataDecimation Command validation ok (decimation inside allowed bounds)");
   return true;
}

AbstractSystemCommandResult SetDataDecimationImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetDataDecimation Command>");

	MastDiagnSystem* md = getMD();

   // Retrieve parameters
   dataDecimationParams params = ((SetDataDecimation*)getCommand())->getParams();
   md->setDecimation( getReducedTimeout(), params._decimation);
	checkCanceled(); // Can throw a CommandCanceledException

	return md->getCommandResult();
}

bool SetDataDecimationImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetDataDecimation Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool SelectFocalStationImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SelectFocalStation Command has an empty (extra) validation>");
   return true;
}

AbstractSystemCommandResult SelectFocalStationImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SelectFocalStation Command>");

   AdSecArbitrator* adSecArb = ((AdSecArbitrator*)getArbitrator());
	MastDiagnSystem* md = getMD();
   IdlSystem *idl = getIdl();

   // Retrieve parameters
   selectFocalStationParams params = ((SelectFocalStation*)getCommand())->getParams();
   string key;
   try {
    key = "FocalStation_"+params._focalStation;
    string addr = (string) adSecArb->ConfigDictionary()[key]; 
    size_t pos = addr.find(",");
    int input=0;
    if (pos>0) {
      input = atoi(addr.substr(pos+1).c_str());
      addr = addr.substr(0,pos);
    }
    md->setDestination( getReducedTimeout(), addr.c_str());
    idl->selectWfsPort( getReducedTimeout(), input);
    adSecArb->varFocalStation()->Set(params._focalStation);
   } catch (Config_File_Exception &e) {
       getCommand()->setErrorString("key "+key+" not found");
       return SysCmdRetry;
   }

	checkCanceled(); // Can throw a CommandCanceledException

	AbstractSystemCommandResult res = md->getCommandResult();
    getCommand()->setErrorString( md->getErrorString());
    if (res != SysCmdSuccess)
        return SysCmdRetry;
    else
        return SysCmdSuccess;
}

bool SelectFocalStationImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SelectFocalStation Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool SetRecMatImpl::validateImpl() throw(CommandValidationException) {

   recMatParams params = ((SetRecMat*)getCommand())->getParams();

   bool v = true;

   if (!Utils::hasExtension( params._m2cFile, ".fits")) {
      _logger->log(Logger::LOG_LEV_ERROR, "SetRecMat Command validation failed: m2c file does not have .fits extension");
      v = false;
   }
   if (!Utils::hasExtension( params._recMatFile, ".fits")) {
      _logger->log(Logger::LOG_LEV_ERROR, "SetRecMat Command validation failed: rec file does not have .fits extension");
      v = false;
   }
   if (!Utils::hasExtension( params._aDelayFile, ".fits")) {
      _logger->log(Logger::LOG_LEV_ERROR, "SetRecMat Command validation failed: A Delay file does not have .fits extension");
      v = false;
   }
   if (!Utils::hasExtension( params._bDelayFile, ".fits")) {
      _logger->log(Logger::LOG_LEV_ERROR, "SetRecMat Command validation failed: B Delay file does not have .fits extension");
      v = false;
   }

   if (v == true)
      _logger->log(Logger::LOG_LEV_DEBUG, "SetDisturb Command validation ok (.fits extension present)");

   return v;
}

AbstractSystemCommandResult SetRecMatImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetRecMat Command>");

	IdlSystem* idl = getIdl();

   // Retrieve parameters
   recMatParams params = ((SetRecMat*)getCommand())->getParams();

   idl->setRecMat( getReducedTimeout(), params);
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool SetRecMatImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetRecMat Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool LoadShapeImpl::validateImpl() throw(CommandValidationException) {

   shapeParams params = ((LoadShape*)getCommand())->getParams();

   _logger->log(Logger::LOG_LEV_DEBUG, "LoadShape Command validation ok (.sav extension present)");
   return true;
}

AbstractSystemCommandResult LoadShapeImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<LoadShape Command>");

	IdlSystem* idl = getIdl();

   // Retrieve parameters
   shapeParams params = ((LoadShape*)getCommand())->getParams();

   // If a relative path, add the flat directory
   string path;
   path = params._shapeFile;
   if (path.substr(0,1) != "/") 
      path = Paths::FlatsDir(true)+"/"+path;

   // If not a .sav, add the extension
   if (!Utils::hasExtension( path, ".sav"))
      path = Utils::addExtension( path, ".sav");

   // Check that the file exists
   if (!Utils::fileExists(path)) {
      getCommand()->setErrorString("File not found: "+ params._shapeFile);
      return SysCmdRetry;
   }

   params._shapeFile = path;
   idl->loadShape( getReducedTimeout(), params);

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool LoadShapeImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<LoadShape Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool SaveShapeImpl::validateImpl() throw(CommandValidationException) {

   shapeParams params = ((SaveShape*)getCommand())->getParams();

   if (!Utils::hasExtension( params._shapeFile, ".sav")) {
      _logger->log(Logger::LOG_LEV_ERROR, "SaveShape Command validation failed: file does not have .sav extension");
      return false;
   }
   _logger->log(Logger::LOG_LEV_DEBUG, "SaveShape Command validation ok (.sav extension present)");
   return true;
}

AbstractSystemCommandResult SaveShapeImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SaveShape Command>");

	IdlSystem* idl = getIdl();

   // Retrieve parameters
   shapeParams params = ((SaveShape*)getCommand())->getParams();

   idl->saveShape( getReducedTimeout(), params);
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());

   AdSecArbitrator* adSecArb = ((AdSecArbitrator*)getArbitrator());
   adSecArb->refreshFlatList();

	return idl->getCommandResult();
}

bool SaveShapeImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SaveShape Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool TTOffloadImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<StopChop Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult TTOffloadImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<TTOffloadImpl Command>");

   AdSecArbitrator* adSecArb = ((AdSecArbitrator*)getArbitrator());

   // Retrieve parameters
   ttOffloadParams params = ((TTOffload*)getCommand())->getParams();
   bool enable = params.enable;

   adSecArb->_varTTOffloadEnable->Set(adSecArb->_LOOffloadReq = enable);
   adSecArb->checkTTOffload();

	checkCanceled(); // Can throw a CommandCanceledException

	return SysCmdSuccess;
}

bool TTOffloadImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<TTOffloadImpl Command undo has an empty implementation>");
	return false;
}






//////////////////////////////

/* L.F. removed
bool SlToAoImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SlToAoImpl Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult SlToAoImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SlToAoImpl Command>");

	IdlSystem* idl = getIdl();

   idl->slToAo( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool SlToAoImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SlToAoImpl Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool SlToChopImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SlToChop Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult SlToChopImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SlToChop Command>");

	IdlSystem* idl = getIdl();

   idl->slToChop( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool SlToChopImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SlToChop Command undo has an empty implementation>");
	return false;
}

  end of remove */
//////////////////////////////

bool StopAoImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<StopAo Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult StopAoImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<StopAo Command>");


   AdSecArbitrator* adSecArb = ((AdSecArbitrator*)getArbitrator());
	IdlSystem* idl = getIdl();
    stopAoParams params = ((StopAo*)getCommand())->getParams();

    _logger->log(Logger::LOG_LEV_INFO,"StopAoImpl(): restoreShape = %d", params._restoreShape);

  idl->stopAo( getReducedTimeout(), params._restoreShape);
	checkCanceled(); // Can throw a CommandCanceledException

   adSecArb->checkTTOffload();

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool StopAoImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<StopAo Command undo has an empty implementation>");
	return false;
}


//////////////////////////////

bool StopChopImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<StopChop Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult StopChopImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<StopChop Command>");

	IdlSystem* idl = getIdl();

   idl->stopChop( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool StopChopImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<StopChop Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

/* L.F. removed
bool SetFlatFFImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetFlatFF Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult SetFlatFFImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetFlatFF Command>");

	IdlSystem* idl = getIdl();

   idl->setFlatFF( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool SetFlatFFImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<SetFlatFF Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool IfAcquireImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<IfAcquire Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult IfAcquireImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<IfAcquire Command>");

	IdlSystem* idl = getIdl();

   idl->ifAcquire( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool IfAcquireImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<IfAcquire Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool IfReduceImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<IfReduce Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult IfReduceImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<IfReduce Command>");

	IdlSystem* idl = getIdl();

   idl->ifReduce( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool IfReduceImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<IfReduce Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool DustTestImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<DustTest Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult DustTestImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<DustTest Command>");

	IdlSystem* idl = getIdl();

   idl->dustTest( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool DustTestImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<DustTest Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool CoilTestImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<CoilTest Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult CoilTestImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CoilTest Command>");

	IdlSystem* idl = getIdl();

   idl->coilTest( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool CoilTestImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<CoilTest Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool CapsensTestImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<CapsensTest Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult CapsensTestImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CapsensTest Command>");

	IdlSystem* idl = getIdl();

   idl->capsensTest( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool CapsensTestImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<CapsensTest Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool FFAcquireImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<FFAcquire Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult FFAcquireImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<FFAcquire Command>");

	IdlSystem* idl = getIdl();

   idl->ffAcquire( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool FFAcquireImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<FFAcquire Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool FFReduceImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<FFReduce Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult FFReduceImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<FFReduce Command>");

	IdlSystem* idl = getIdl();

   idl->ffReduce( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool FFReduceImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<FFReduce Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool WdTestImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<WdTest Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult WdTestImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<WdTest Command>");

	IdlSystem* idl = getIdl();

   idl->wdTest( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool WdTestImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<WdTest Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool FlTestImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<FlTest Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult FlTestImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<FlTest Command>");

	IdlSystem* idl = getIdl();

   idl->flTest( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool FlTestImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<FlTest Command undo has an empty implementation>");
	return false;
}

//////////////////////////////

bool PsTestImpl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<PsTest Command has an empty (extra) validation>");
	return true;
}

AbstractSystemCommandResult PsTestImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PsTest Command>");

	IdlSystem* idl = getIdl();

   idl->psTest( getReducedTimeout());
	checkCanceled(); // Can throw a CommandCanceledException

   if (idl->getCommandResult() != SysCmdSuccess)
      getCommand()->setErrorString( idl->getErrorString());
	return idl->getCommandResult();
}

bool PsTestImpl::undoImpl() throw(CommandUndoingException) {
	_logger->log(Logger::LOG_LEV_TRACE, "<PsTest Command undo has an empty implementation>");
	return false;
}

  end of remove */


//////////////////////////////

bool CalibrateImpl::validateImpl() throw(CommandValidationException) {
	return true;
}

AbstractSystemCommandResult CalibrateImpl::doExecuteImpl() throw(CommandExecutionException, CommandCanceledException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Calibration Procedure Command>");

	IdlSystem* idl = getIdl();
    calibrateParams params = ((CalibrateCmd*)getCommand())->getParams();


    string idl_cmd = "collect_" + params._procname;

    idl->simpleCommand("calibrate",  getReducedTimeout(), idl_cmd);

	checkCanceled();

    if (idl->getCommandResult() != SysCmdSuccess)
        getCommand()->setErrorString( idl->getErrorString());

	return idl->getCommandResult();
}
