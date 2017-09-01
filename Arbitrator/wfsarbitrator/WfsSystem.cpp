#include "wfsarbitrator/WfsSystem.h"
#include "wfsarbitrator/WfsArbitrator.h"

extern "C" {
#include "base/timelib.h"
}


using namespace Arcetri::Wfs_Arbitrator;


//#define EXTERNAL


/********************** PUBLIC *********************/

string WfsSystem::OP_MODE_DEFAULT_CONFIG = "Debug";	// See wfs-arb.conf (this value is actually never used,
													// but set only for safety


WfsSystem::WfsSystem(): AbstractSystem("WFS") {

	_opModeConfig = OP_MODE_DEFAULT_CONFIG;

	_arbitrator = AbstractArbitrator::getInstance();

	#ifdef EXTERNAL
	_wfsInterface.init( _logger, 0, "WfsArb200");
	#else
   _wfsInterface.init( _logger);
   #endif

   setInstrument("");	// Start without instrument, we'll get one at the preset
}

WfsSystem::~WfsSystem() {

}

void WfsSystem::setInstrument( string instrument) {
   _wfsInterface.setSensorInstr( ((WfsArbitrator *)_arbitrator)->_wfsSpec, instrument);
}


void WfsSystem::operate( powerOnParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing operate...");
	AbstractSystemCommandResult cmdRes;
	string errString;

   if (params.config == "LAST") 
      params.config = _opModeConfig;
    else
		_opModeConfig = params.config; // Saved to be used in case of failure: RecoverFailure is
								// implemented as an Operate without parameter!

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.setOperating( params);
	errString = _wfsInterface.getErrorString();
	#else
	cmdRes = SysCmdSuccess;
	errString = "dummy_warning";
	msleep(5000);
	Warning warning("Operate is executing...");
	_arbitrator->notifyClients(&warning);
	msleep(5000);
	#endif

	setCommandResult(cmdRes, errString);
}


void WfsSystem::off() {
	_logger->log(Logger::LOG_LEV_INFO, "Executing off...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.setPowerOff();
	errString = _wfsInterface.getErrorString();
	#else
	cmdRes = SysCmdSuccess;
	errString = "dummy_warning";
	msleep(2000);
	Warning warning("Off is executing...");
	_arbitrator->notifyClients(&warning);
	msleep(2000);
	#endif

	setCommandResult(cmdRes, errString);
}


void WfsSystem::prepareAcquireRef(prepareAcquireRefParams params, prepareAcquireRefResult *outputParams) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing prepareAcquireRef...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.prepareAcquireRef(&params, outputParams);
	errString = _wfsInterface.getErrorString();
	#else
	cmdRes = SysCmdSuccess;
	errString = "dummy_warning";
	msleep(1000);
	Warning warning("PrepareAcquireRef is executing...");
	_arbitrator->notifyClients(&warning);
	msleep(1000);
	#endif

	setCommandResult(cmdRes, errString);
}



void WfsSystem::enableDisturb(enableDisturbParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing EnableDisturb...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.enableDisturb(params);
	errString = _wfsInterface.getErrorString();
	#else
	cmdRes = SysCmdSuccess;
	errString = "dummy_warning";
	msleep(1000);
	Warning warning("EnableDisturb is executing...");
	_arbitrator->notifyClients(&warning);
	msleep(1000);
	#endif

	setCommandResult(cmdRes, errString);
}



void WfsSystem::getTVSnap( snapParams *outputParams) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing GetTVSnap...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.getTVSnap( outputParams);
	errString = _wfsInterface.getErrorString();
	#else
	cmdRes = SysCmdSuccess;
	errString = "dummy_warning";
	msleep(1000);
	Warning warning("GetTVSnap is executing...");
	_arbitrator->notifyClients(&warning);
	msleep(1000);
	#endif

	setCommandResult(cmdRes, errString);
}


void WfsSystem::checkRef( checkRefResult *outputParams) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing checkRef...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.checkRef( outputParams);
	errString = _wfsInterface.getErrorString();
	#else
	cmdRes = SysCmdSuccess;
	errString = "dummy_warning";
	msleep(1000);
	Warning warning("checkRef is executing...");
	_arbitrator->notifyClients(&warning);
	msleep(1000);
	#endif

	setCommandResult(cmdRes, errString);
}






void WfsSystem::acquireRef(acquireRefResult *outputParams) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing acquireRef...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.acquireRef(outputParams);
	errString = _wfsInterface.getErrorString();

	#else
	cmdRes = SysCmdSuccess;
	errString = "dummy_warning";
	msleep(3000);
	Warning warning("AcquireRef is executing...");
	_arbitrator->notifyClients(&warning);
	msleep(3000);
	#endif

	setCommandResult(cmdRes, errString);
}


AbstractSystemCommandResult WfsSystem::closeLoop() {
	_logger->log(Logger::LOG_LEV_INFO, "Executing closeLoop...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.closeLoop();
	errString = _wfsInterface.getErrorString();
	#else
	cmdRes = SysCmdSuccess;
	errString = "dummy_warning";
	msleep(5000);
	Warning warning("CloseLoop is executing...");
	_arbitrator->notifyClients(&warning);
	msleep(5000);
	#endif

	setCommandResult(cmdRes, errString);
   return cmdRes;
}


AbstractSystemCommandResult WfsSystem::stopLoop() {
	_logger->log(Logger::LOG_LEV_INFO, "Executing stopLoop...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.stopLoop();
	errString = _wfsInterface.getErrorString();
	#else
	cmdRes = SysCmdSuccess;
	errString = "dummy_warning";
	msleep(5000);
	Warning warning("StopLoop is executing...");
	_arbitrator->notifyClients(&warning);
	msleep(5000);
	#endif

	setCommandResult(cmdRes, errString);
   return cmdRes;
}

void WfsSystem::saveStatus( saveStatusParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing saveStatus...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.saveStatus( params);
	errString = _wfsInterface.getErrorString();
	#else
	cmdRes = SysCmdSuccess;
	errString = "dummy_warning";
	msleep(5000);
	Warning warning("SaveStatus is executing...");
	_arbitrator->notifyClients(&warning);
	msleep(5000);
	#endif

	setCommandResult(cmdRes, errString);
}



void WfsSystem::modifyAO(modifyAOparams params, modifyAOparams *result) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing modifyAO...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.modifyAO(params, result);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);
}

int WfsSystem::getDecimation() {
   return _wfsInterface.getDecimation();
}

float WfsSystem::getLoopFreq() {
   return _wfsInterface.getLoopFreq();
}

int WfsSystem::getBinning() {
   return _wfsInterface.getBinning();
}

float WfsSystem::getModulation() {
   return _wfsInterface.getModulation();
}

float WfsSystem::getRangeMin() {
   return _wfsInterface.getRangeMin();
}

float WfsSystem::getRangeMax() {
   return _wfsInterface.getRangeMax();
}

string WfsSystem::getSourceName() {
   return _wfsInterface.getSourceName();
}

float WfsSystem::getSourceMag() {
   return _wfsInterface.getSourceMag();
}

void WfsSystem::refineLoop(wfsLoopParams wfsLoopPar) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing refineLoop...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.refineLoop(wfsLoopPar);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}

void WfsSystem::pauseLoop() {
	_logger->log(Logger::LOG_LEV_INFO, "Executing pauseLoop...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.pauseLoop();
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);
}

void WfsSystem::resumeLoop() {
	_logger->log(Logger::LOG_LEV_INFO, "Executing resumeLoop...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.resumeLoop();
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);
}


void WfsSystem::setHoArmPolicy(SetHoArmPolicy hoArmPolicy) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing setHoArmPolicy with timeout %d ms...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.setHOarmPolicy(hoArmPolicy);
	errString = _wfsInterface.getErrorString();
	#else
	cmdRes = SysCmdSuccess;
	errString = "setHoArmPolicy :-s";
	#endif

	setCommandResult(cmdRes, errString);
}

void WfsSystem::setTvArmPolicy(SetTvArmPolicy tvArmPolicy) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing setTvArmPolicy with timeout %d ms...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.setTVarmPolicy(tvArmPolicy);
	errString = _wfsInterface.getErrorString();
	#else
	cmdRes = SysCmdSuccess;
	errString = "setTvArmPolicy :-s";
	#endif

	setCommandResult(cmdRes, errString);
}

void WfsSystem::setPointAndSourcePolicy(PointAndSourcePolicy pointSourcePolicy) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing setPointAndSourcePolicy with timeout %d ms...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	cmdRes = _wfsInterface.setPointAndSourcePolicy(pointSourcePolicy);
	errString = _wfsInterface.getErrorString();
	#else
	cmdRes = SysCmdSuccess;
	errString = "SetPointAndSourcePolicy :-s";
	#endif

	setCommandResult(cmdRes, errString);
}

void WfsSystem::setSource( setSourceParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing setSource...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.setSource( params.source, params.magnitude);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}


void WfsSystem::calibrateHODark( int nframes) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing calibrateHODark...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.calibrateHODark( nframes);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}
void WfsSystem::calibrateIRTCDark( int nframes) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing calibrateIRTCDark...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.calibrateIRTCDark( nframes);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}

void WfsSystem::calibratePISCESDark( int nframes) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing calibratePISCESDark...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.calibratePISCESDark( nframes);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}

void WfsSystem::calibrateTVDark( int nframes) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing calibrateTVDark...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.calibrateTVDark( nframes);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}

void WfsSystem::calibrateMovements() {
	_logger->log(Logger::LOG_LEV_INFO, "Executing calibrateMovements...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.calibrateMovements();
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}

void WfsSystem::calibrateSlopenull( int nframes) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing calibrateSlopenull...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.calibrateSlopenull( nframes);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}

void WfsSystem::savePsf( string filename, int nFrames) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing savePsf...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.savePsf( filename, nFrames);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}

void WfsSystem::saveIrtc( string filename, int nFrames) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing saveIrtc...");
	AbstractSystemCommandResult cmdRes;
	string errString;


	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.saveIrtc( filename, nFrames);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}

void WfsSystem::savePisces( string filename, int nFrames) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing savePisces...");
	AbstractSystemCommandResult cmdRes;
	string errString;


	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.savePisces( filename, nFrames);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}

void WfsSystem::offsetXY(offsetXYparams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing offsetXY...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.offsetXY(params);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}
void WfsSystem::offsetZ(offsetZparams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing offsetZ...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.offsetZ(params);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}
void WfsSystem::optimizeGain() {
	_logger->log(Logger::LOG_LEV_INFO, "Executing optimizeGain...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.optimizeGain();
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}

void WfsSystem::correctModes(correctModesParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing correctModes...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.correctModes(params);
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}

void WfsSystem::emergencyOff() {
	_logger->log(Logger::LOG_LEV_INFO, "Executing emergencyOff...");
	AbstractSystemCommandResult cmdRes;
	string errString;

	#ifndef TEST_ONLY
	//_wfsInterface.setTimeout(timeout_ms); // TODO scalare il timeout!!!
	cmdRes = _wfsInterface.emergencyOff();
	errString = _wfsInterface.getErrorString();
	#endif

	setCommandResult(cmdRes, errString);

}




