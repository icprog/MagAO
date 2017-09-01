#include "AdSecCommands.h"

using namespace Arcetri::AdSec_Arbitrator;

//////////////////////

SetMirror::SetMirror(unsigned int timeout_ms, OpCode code, string description, flatParams params, string progressVar):
    Command(code, description, timeout_ms, progressVar) {

	_params = params;
}

void SetMirror::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      dummy: %s", _params._dummy.c_str());
}

//////////////////////

bool AoToChop::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Ao to Chop Command has an empty (basic) validation>");
	return true;
}

//////////////////////

/*  L.F removed
bool AoToSl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Ao to Sl Command has an empty (basic) validation>");
	return true;
}
  end of remove */

//////////////////////

bool RequestAdSecStatus::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<RequestAdSecStatus Command has an empty (basic) validation>");
	return true;
}

//////////////////////

void SetDataDecimation::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "     Decimation: %d", _params._decimation);
}

bool SetDataDecimation::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetDataDecimation has an empty (basic) validation>");
   return true;
}


void SelectFocalStation::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "     Focal Station: %s", _params._focalStation.c_str());
}

bool SelectFocalStation::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SelectFocalStation has an empty (basic) validation>");
   return true;
}


//////////////////////

void ApplyCommands::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "     Commands file: %s", _params.commandsFile.c_str());
}



bool ApplyCommands::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<ApplyCommands has an empty (basic) validation>");
   return true;
}

//////////////////////

bool RecoverSkipFrame::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<RecoverSkipFrame Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool ChopToAo::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Chop to Ao Command has an empty (basic) validation>");
	return true;
}

//////////////////////

/*  L.F removed
bool ChopToSl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Chop to Sl Command has an empty (basic) validation>");
	return true;
}
  end of remove */

//////////////////////

bool LoadProgram::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Load Program Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool Off::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Power Off Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool On::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Power On Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool Reset::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Reset Command has an empty (basic) validation>");
	return true;
}

//////////////////////


bool Rest::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Rest Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool RunAo::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Run AO Command has an empty (basic) validation>");
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "     Loop freq.: %5.3f", _params._loopFreq);
	_logger->log(Logger::LOG_LEV_DEBUG, "      OVS freq.: %5.3f", _params._ovsFreq);
	_logger->log(Logger::LOG_LEV_DEBUG, "     Decimation: %d", _params._decimation);
	return true;
}

bool SetZernikes::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Set Zernikes Command has an empty (basic) validation>");
	return true;
}


//////////////////////

bool TTOffload::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<TT Offload Command has an empty (basic) validation>");
	return true;
}

void TTOffload::log() const {
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "     enable: %s", _params.enable ? "true" : "false");
}

//////////////////////

bool RunChop::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Run Chop Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool PauseAo::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PauseAo Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool ResumeAo::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<ResumeAo Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool SetFlatAo::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Set Flat AO Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool SetFlatChop::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Set Flat Chop Command has an empty (basic) validation>");
	return true;
}

//////////////////////

/*  L.F removed
bool SetFlatSl::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Set Flat Sl Command has an empty (basic) validation>");
	return true;
}
  end of remove */

//////////////////////

bool SetGain::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Set Gain Command has an empty (basic) validation>");
	return true;
}

void SetGain::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      gainFile: %s", _params._gainFile.c_str());
}

//////////////////////

bool SaveSlopes::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Save Slopes Command has an empty (basic) validation>");
	return true;
}

void SaveSlopes::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      filename: %s", _params._filename.c_str());
	_logger->log(Logger::LOG_LEV_DEBUG, "      n frames: %d", _params._nFrames);
}

//////////////////////

bool SaveStatus::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Save Status Command has an empty (basic) validation>");
	return true;
}

void SaveStatus::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      filename: %s", _params._filename.c_str());
}

//////////////////////

bool SetDisturb::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Set Disturb Command has an empty (basic) validation>");
	return true;
}

void SetDisturb::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      disturbFile: %s", _params._disturbFile.c_str());
   _logger->log(Logger::LOG_LEV_INFO, "   > disturbActive:  %s", _params._on ? "ON": "OFF");
}

//////////////////////

bool LoadShape::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Load Shape Command has an empty (basic) validation>");
	return true;
}

void LoadShape::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      shapeFile: %s", _params._shapeFile.c_str());
}

//////////////////////

bool SaveShape::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Save Shape Command has an empty (basic) validation>");
	return true;
}

void SaveShape::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      shapeFile: %s", _params._shapeFile.c_str());
}

//////////////////////

bool SetRecMat::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Set Rec Mat Command has an empty (basic) validation>");
	return true;
}

void SetRecMat::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      M2c : %s", _params._m2cFile.c_str());
	_logger->log(Logger::LOG_LEV_DEBUG, "      Rec mat file: %s", _params._recMatFile.c_str());
	_logger->log(Logger::LOG_LEV_DEBUG, "      aDelay file: %s", _params._aDelayFile.c_str());
	_logger->log(Logger::LOG_LEV_DEBUG, "      bDelay file: %s", _params._bDelayFile.c_str());
}

//////////////////////


/*  L.F removed
bool SlToAo::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Sl to Ao Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool SlToChop::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Sl to Chop Command has an empty (basic) validation>");
	return true;
}

  end of remove */
//////////////////////

bool StopAo::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Stop Ao Command has an empty (basic) validation>");
	return true;
}

void StopAo::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      restoreShape: %d", _params._restoreShape);
}

//////////////////////

bool StopChop::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Stop Chop Command has an empty (basic) validation>");
	return true;
}

/////////////////////

bool AdSec_Arbitrator::RecoverFailure::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Recover Failure Command has an empty (basic) validation>");
	return true;
}

//////////////////////

/*  L.F removed
bool SetFlatFF::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetFlat Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool IfAcquire::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<IfAcquire Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool IfReduce::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<IfReduce Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool DustTest::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<DustTest Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool CoilTest::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CoilTest Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool CapsensTest::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CapsensTest Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool FFAcquire::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<FFAcquire Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool FFReduce::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<FFReduce Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool WdTest::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<WdTest Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool FlTest::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<FlTest Command has an empty (basic) validation>");
	return true;
}

//////////////////////

bool PsTest::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PsTest Command has an empty (basic) validation>");
	return true;
}

  end of remove */

