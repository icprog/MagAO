#include "WfsCommands.h"

using namespace Arcetri::Wfs_Arbitrator;


//////////////////////////// OPERATE

bool Operate::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "Operate Command has an empty (basic) validation>");
	return true;
}

void Operate::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Input parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "       Config:  %s", _params.config.c_str() );
	_logger->log(Logger::LOG_LEV_DEBUG, "  Board setup:  %s", _params.boardSetup.c_str() );
	_logger->log(Logger::LOG_LEV_DEBUG, "Optical setup:  %s", _params.opticalSetup ? "Yes" : "No" );
}

//////////////////////////// SAVESTATUS

bool SaveStatus::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "SaveStatus Command has an empty (basic) validation>");
	return true;
}

void SaveStatus::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Input parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "       Filename:  %s", _params._outputFile.c_str() );
}


//////////////////////////// SAVEOPTLOOPDATA

bool SaveOptLoopData::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "SaveOptLoopData Command has an empty (basic) validation>");
	return true;
}

void SaveOptLoopData::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Input parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "       nFrames:  %d", _params._nFrames );
}


//////////////////////////// OFF

bool Off::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "Off Command has an empty (basic) validation>");
	return true;
}

//////////////////////////// REQUEST WFS STATUS

bool RequestWfsStatus::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "RequestWfsStatus Command has an empty (basic) validation>");
	return true;
}

//////////////////////////// PREPARE ACQUIRE REF

bool PrepareAcquireRef::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "PrepareAcquireRef Command has an empty (basic) validation>");
	// TODO
	// Validate parameters
	return true;
}

void PrepareAcquireRef::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Input parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      AOMode:     %5s", _params.AOMode.c_str() );
	_logger->log(Logger::LOG_LEV_DEBUG, "      SOCoords:   %f,%f", _params.SOCoords[0], _params.SOCoords[1]);
	_logger->log(Logger::LOG_LEV_DEBUG, "      ROCoords:   %f,%f", _params.ROCoords[0], _params.ROCoords[1]);
	_logger->log(Logger::LOG_LEV_DEBUG, "      Elevation:  %f", _params.Elevation);
	_logger->log(Logger::LOG_LEV_DEBUG, "      RotAngle:   %f", _params.RotAngle);
	_logger->log(Logger::LOG_LEV_DEBUG, "      GravAngle:  %f", _params.GravAngle);
	_logger->log(Logger::LOG_LEV_DEBUG, "      Magnitude:  %f", _params.Mag);
	_logger->log(Logger::LOG_LEV_DEBUG, "      Color:      %f", _params.Color);
	_logger->log(Logger::LOG_LEV_DEBUG, "      r0:         %f", _params.r0);
}


//////////////////////////// ACQUIRE REF

bool AcquireRef::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "AcquireRef Command has an empty (basic) validation>");
	// TODO
	// Validate parameters
	return true;
}

void AcquireRef::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Output parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      [TODO]");
}


//////////////////////////// ENABLE DISTURB

bool EnableDisturb::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "EnableDisturb Command has an empty (basic) validation>");
	return true;
}

void EnableDisturb::log() const {
	Command::log();
}

//////////////////////////// SET SOURCE

bool SetSource::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "SetSource Command has an empty (basic) validation>");
	return true;
}

void SetSource::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Input parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      Source:      %s", _params.source.c_str() );
	_logger->log(Logger::LOG_LEV_DEBUG, "      Magnitude:   %f", _params.magnitude);
}

//////////////////////////// MODIFY AO

bool ModifyAO::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "ModifyAO Command has an empty (basic) validation>");
	return true;
}

void ModifyAO::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Input parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      freq:       %f", _params.freq );
	_logger->log(Logger::LOG_LEV_DEBUG, "      Binning:    %d", _params.Binning);
	_logger->log(Logger::LOG_LEV_DEBUG, "      TT mod:     %f", _params.TTmod);
	_logger->log(Logger::LOG_LEV_DEBUG, "      Fw1Pos:     %d", _params.Fw1Pos);
	_logger->log(Logger::LOG_LEV_DEBUG, "      Fw2Pos:     %d", _params.Fw2Pos);
}


//////////////////////////// CLOSE LOOP

bool CloseLoop::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "CloseLoop Command has an empty (basic) validation>");
	return true;
}


//////////////////////////// PREPARE ADSEC

bool PrepareAdsec::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "PrepareAdsec Command has an empty (basic) validation>");
	return true;
}


//////////////////////////// PAUSE LOOP

bool PauseLoop::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "PauseLoop Command has an empty (basic) validation>");
	return true;
}


//////////////////////////// REFINE LOOP

bool RefineLoop::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "RefineLoop Command has an empty (basic) validation>");
	return true;
}

void RefineLoop::log() const {
	RefineLoop::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Input parameters:");
	for(int i=0; i<wfsNUM_ZERNIKE; i++) {
		_logger->log(Logger::LOG_LEV_DEBUG, "      Zernike mode %d:  %f", i, _wfsLoopPar.zmodes[i] );
	}
}


//////////////////////////// RESUME LOOP

bool ResumeLoop::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "ResumeLoop Command has an empty (basic) validation>");
	return true;
}


//////////////////////////// STOP LOOP

bool StopLoop::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "StopLoop Command has an empty (basic) validation>");
	return true;
}

//////////////////////////// STOP LOOP ADSEC

bool StopLoopAdsec::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "StopLoopAdsec Command has an empty (basic) validation>");
	return true;
}

//////////////////////////// CALIBRATE HO DARK

bool CalibrateHODark::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "CalibrateHODark Command has an empty (basic) validation>");
	return true;
}

//////////////////////////// CALIBRATE IRTC DARK

bool CalibrateIRTCDark::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "CalibrateIRTCDark Command has an empty (basic) validation>");
	return true;
}

//////////////////////////// CALIBRATE PISCES DARK

bool CalibratePISCESDark::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "CalibratePISCESDark Command has an empty (basic) validation>");
	return true;
}

//////////////////////////// CALIBRATE TV DARK

bool CalibrateTVDark::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "CalibrateTVDark Command has an empty (basic) validation>");
	return true;
}


//////////////////////////// CALIBRATE MOVEMENTS

bool CalibrateMovements::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "CalibrateMovements Command has an empty (basic) validation>");
	return true;
}


//////////////////////////// CALIBRATE SLOPENULL

bool CalibrateSlopenull::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "CalibrateSlopenull Command has an empty (basic) validation>");
	return true;
}


//////////////////////////// GET TV SNAP

bool GetTVSnap::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "GetTVSnap Command has an empty (basic) validation>");
	return true;
}
//////////////////////////// CHECK REF

bool CheckRef::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "CheckRef Command has an empty (basic) validation>");
	return true;
}
//////////////////////////// CORRECT MODES

bool CorrectModes::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "CorrectModes Command has an empty (basic) validation>");
	return true;
}

//////////////////////////// CORRECT MODES

bool AntiDrift::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "AntiDrift Command has an empty (basic) validation>");
	return true;
}

//////////////////////////// EMERGENCY OFF

bool EmergencyOff::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "EmergencyOff Command has an empty (basic) validation>");
	return true;
}

//////////////////////////// OFFSET XY

bool OffsetXY::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "OffsetXY Command has an empty (basic) validation>");
	return true;
}

void OffsetXY::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Input parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      offset x:  %5.3f mm", _params.offsetX);
	_logger->log(Logger::LOG_LEV_DEBUG, "      offset y:  %5.3f mm", _params.offsetY);
	_logger->log(Logger::LOG_LEV_DEBUG, "         brake:  %s", (_params.brake)?"True":"False");
}

//////////////////////////// AutoTrack

bool AutoTrack::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "AutoTrack Command has an empty (basic) validation>");
	return true;
}

void AutoTrack::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Input parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      Rerot tracking: %s", _params.rerotTrack?"True":"False");
	_logger->log(Logger::LOG_LEV_DEBUG, "      Adc   tracking: %s", _params.adcTrack?"True":"False");
	_logger->log(Logger::LOG_LEV_DEBUG, "      Lens  tracking: %s", _params.lensTrack?"True":"False");
}

//////////////////////////// OFFSET Z

bool OffsetZ::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "OffsetZ Command has an empty (basic) validation>");
	return true;
}

void OffsetZ::log() const {
	Command::log();
	_logger->log(Logger::LOG_LEV_DEBUG, " >> Input parameters:");
	_logger->log(Logger::LOG_LEV_DEBUG, "      offset z:  %5.3f mm", _params.offsetZ);
}

//////////////////////////// OPTIMIZEGAIN

bool OptimizeGain::validateImpl() throw(CommandValidationException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "OptimizeGain Command has an empty (basic) validation>");
	return true;
}

void OptimizeGain::log() const {
	Command::log();
}

