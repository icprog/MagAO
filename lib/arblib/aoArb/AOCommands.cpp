#include "arblib/aoArb/AOCommands.h"

using namespace Arcetri::Arbitrator;

// ------------------------ PowerOnWfs ----------------------- //

void PowerOnWfs::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> WFS sel: "+_params.wfsid);
}


bool PowerOnWfs::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Power On WFS command has an empty (basic) validation>");
	return true;
}

// ------------------------ PowerOffWfs ----------------------- //


void PowerOffWfs::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> WFS sel: "+_params.wfsid);
}

bool PowerOffWfs::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PowerOffWfs command has an empty (basic) validation>");
	return true;
}

// ------------------------ PowerOnAdSec ----------------------- //


bool PowerOnAdSec::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PowerOnAdSec command has an empty (basic) validation>");
	return true;
}

// ------------------------ PowerOffAdSec ----------------------- //


bool PowerOffAdSec::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PowerOffAdSec command has an empty (basic) validation>");
	return true;
}

// ------------------------ MirrorRest -------------------------- //


bool MirrorRest::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<MirrorRest command has an empty (basic) validation>");
	return true;
}

// ------------------------ MirrorSet --------------------------- //


bool MirrorSet::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<MirrorSet command has an empty (basic) validation>");
	return true;
}


// ------------------------ FaultRecovery ----------------------- //


bool FaultRecovery::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<FaultRecovery command has an empty (basic) validation>");
	return true;
}


// ------------------------ StandaloneFault --------------------- //


bool StandaloneFail::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<StandaloneFail command has an empty (basic) validation>");
	return true;
}


// ------------------------ SeeingLmtdFail --------------------- //


bool SeeingLmtdFail::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SeeingLmtdFail command has an empty (basic) validation>");
	return true;
}


// ------------------------ SetAOMode ----------------------- //


bool SetAOMode::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<SetAOMode command has an empty (basic) validation>");
	return true;
}

// ------------------------- AdjustGain ---------------------- //

bool AdjustGain::validateImpl() throw(CommandValidationException) {
   _logger->log(Logger::LOG_LEV_DEBUG, "<AdjustGain command has an empty (basic) validation>");
   return true;
}

void AdjustGain::log() const {
	Command::log();
}

// ------------------------- AdjustIntTime ---------------------- //

bool AdjustIntTime::validateImpl() throw(CommandValidationException) {
   _logger->log(Logger::LOG_LEV_DEBUG, "<AdjustIntTime command has an empty (basic) validation>");
   return true;
}

void AdjustIntTime::log() const {
	Command::log();
}


// ------------------------ AcquireRefAO ----------------------- //

void AcquireRefAO::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Output parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      slopeNull: %s", _result.slNull.c_str());
	_logger->log(Logger::LOG_LEV_DEBUG, "      DXY: %d, %d", _result.deltaXY[0], _result.deltaXY[1]);
	_logger->log(Logger::LOG_LEV_DEBUG, "      NMOdes: %d", _result.nModes);
	_logger->log(Logger::LOG_LEV_DEBUG, "      freq: %f", _result.freq);
	_logger->log(Logger::LOG_LEV_DEBUG, "      NBins: %f", _result.nBins);
	_logger->log(Logger::LOG_LEV_DEBUG, "      TTMode: %f", _result.ttMod);
	_logger->log(Logger::LOG_LEV_DEBUG, "      FSpec1: %s", _result.f1spec.c_str());
	_logger->log(Logger::LOG_LEV_DEBUG, "      FSpec2: %s", _result.f2spec.c_str());
	_logger->log(Logger::LOG_LEV_DEBUG, "      Sthrel: %f", _result.strehl);
	_logger->log(Logger::LOG_LEV_DEBUG, "      R0: %f", _result.r0);
	_logger->log(Logger::LOG_LEV_DEBUG, "      MSNRatio: %f, %f", _result.mSNratio[0],  _result.mSNratio[2]);
//      _logger->log(Logger::LOG_LEV_DEBUG, "      TVFrame: data omitted");
}

bool AcquireRefAO::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<AcquireRefAO command has an empty (basic) validation>");
	return true;
}


// ------------------------ CheckRefAO ----------------------- //

void CheckRefAO::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_TRACE, " >> Output parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      DXY: %d, %d", _result.deltaXY[0], _result.deltaXY[1]);
}

bool CheckRefAO::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<CheckRefAO command has an empty (basic) validation>");
	return true;
}


// ------------------------ CorrectModes ----------------------- //

void CorrectModes::log() const {
	Command::log();
}

bool CorrectModes::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Correct Modes command has an empty (basic) validation>");
	return true;
}


// ------------------------ SetZernikes ----------------------- //

void SetZernikes::log() const {
	Command::log();
}

bool SetZernikes::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Set Zernikes command has an empty (basic) validation>");
	return true;
}


// ------------------------ ModifyAO ----------------------- //

void ModifyAO::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      NMOdes: %d", _params.nModes);
	_logger->log(Logger::LOG_LEV_DEBUG, "      freq: %f", _params.freq);
	_logger->log(Logger::LOG_LEV_DEBUG, "      NBins: %f", _params.binning);
	_logger->log(Logger::LOG_LEV_DEBUG, "      TTMode: %f", _params.TTmod);
	_logger->log(Logger::LOG_LEV_DEBUG, "      FSpec1: %s", _params.f1spec.c_str());
	_logger->log(Logger::LOG_LEV_DEBUG, "      FSpec2: %s", _params.f2spec.c_str());
}

bool ModifyAO::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<ModifyAO command has an empty (basic) validation>");
	return true;
}


// ------------------------ OffsetXY ----------------------- //

void OffsetXY::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "     Offset XY: %f, %f", _params.deltaXY[0], _params.deltaXY[1]);
}

bool OffsetXY::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Offset XY command has an empty (basic) validation>");
	return true;
}


// ------------------------ OffsetZ ----------------------- //

void OffsetZ::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "     Offset Z: %f", _params.deltaZ);
}

bool OffsetZ::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Offset Z command has an empty (basic) validation>");
	return true;
}


// ------------------------ PauseAO ----------------------- //

bool Pause::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Pause command has an empty (basic) validation>");
	return true;
}


// ------------------------ PresetAO ----------------------- //

void PresetAO::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_INFO, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_INFO, "      AOMode:     %s", _params.aoMode.c_str());
	_logger->log(Logger::LOG_LEV_INFO, "      focStation: %s", _params.focStation .c_str());
	_logger->log(Logger::LOG_LEV_INFO, "      Instr:      %s", _params.instr.c_str());
	_logger->log(Logger::LOG_LEV_INFO, "      wfsSpec:    %s", _params.wfsSpec.c_str());
	_logger->log(Logger::LOG_LEV_INFO, "      SOCoord:    %f mm, %f mm", _params.soCoord[0], _params.soCoord[1]);
	_logger->log(Logger::LOG_LEV_INFO, "      ROCoord:    %f mm, %f mm", _params.roCoord[0], _params.roCoord[1]);
	_logger->log(Logger::LOG_LEV_INFO, "      Elevation:  %f rad", _params.elevation);
	_logger->log(Logger::LOG_LEV_INFO, "      rotAngle:   %f rad", _params.rotAngle);
	_logger->log(Logger::LOG_LEV_INFO, "      gravAngle:  %f rad", _params.gravAngle);
	_logger->log(Logger::LOG_LEV_INFO, "      mag:        %f", _params.mag);
	_logger->log(Logger::LOG_LEV_INFO, "      color:      %f", _params.color);
	_logger->log(Logger::LOG_LEV_INFO, "      skyBrgt:    %f", _params.skyBrgt);
	_logger->log(Logger::LOG_LEV_INFO, "      r0:         %f", _params.r0);
	_logger->log(Logger::LOG_LEV_INFO, "      windSpeed:  %f rad", _params.windSpeed);
	_logger->log(Logger::LOG_LEV_INFO, "      windDir:    %f rad", _params.windDir);
	_logger->log(Logger::LOG_LEV_INFO, "      adcTracking:%d", _params.adcTracking);
}

bool PresetAO::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<PresetAO command has an empty (basic) validation>");
	return true;
}

// ------------------------ PresetFlat ----------------------- //

void PresetFlat::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      FlatSpec: %s", _params.flatSpec.c_str());
}

bool PresetFlat::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Preset Flat command has an empty (basic) validation>");
	return true;
}


// ------------------------ RefineAO ----------------------- //

void RefineAO::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      Method: %s", _params.method.c_str());
}

bool RefineAO::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Refine AO command has an empty (basic) validation>");
	return true;
}


// ------------------------ ResumeAO ----------------------- //

bool Resume::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Resume command has an empty (basic) validation>");
	return true;
}


// ------------------------ StartAO ----------------------- //

bool StartAO::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Start AO command has an empty (basic) validation>");
	return true;
}

// ------------------------ RecoverWfsFailure ----------------------- //

bool RecoverWfsFailure::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<RecoverWfsFailure command has an empty (basic) validation>");
	return true;
}

// ------------------------ RecoverAdSecFailure ----------------------- //

bool RecoverAdSecFailure::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<RecoverAdSecFailure command has an empty (basic) validation>");
	return true;
}


// ------------------------ Stop ----------------------- //

void Stop::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      Reason: %s", _params.msg.c_str());
}

bool Stop::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<Stop command has an empty (basic) validation>");
	return true;
}


// ------------------------ UserPanic ----------------------- //

void UserPanic::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      Message: %s", _params.msg.c_str());
}

bool UserPanic::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<User Panic command has an empty (basic) validation>");
	return true;
}

// ------------------------ RequestAOStatus ----------------------- //

bool RequestAOStatus::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "<RequestAOStatus command has an empty (basic) validation>");
	return true;
}
