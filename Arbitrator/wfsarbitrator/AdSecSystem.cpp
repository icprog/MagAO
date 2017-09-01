#include "wfsarbitrator/AdSecSystem.h"
#include "wfsarbitrator/WfsArbitrator.h"
#include "arblib/adSecArb/AdSecCommands.h"

using namespace Arcetri::Arbitrator;
using namespace Arcetri::Wfs_Arbitrator;

int adsec_timeout = 30000;

AdSecSystem::AdSecSystem() : AbstractSystem("ADSEC") {
	_arbitrator = AbstractArbitrator::getInstance();
	_adSecArbIntf = new ArbitratorInterface( "adsecarb."+AOApp::Side()+"@M_ADSEC", Logger::LOG_LEV_DEBUG);
}

AdSecSystem::~AdSecSystem() {

}

// Check whether the adsec arbitrator exists and is reachable
bool AdSecSystem::check() {
	return _adSecArbIntf->check();
}

AbstractSystemCommandResult AdSecSystem::setDataDecimation( AdSec_Arbitrator::dataDecimationParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing setDataDecimation...");
   AbstractSystemCommandResult ret = SysCmdError;

	Command* reply = NULL;

	try {
      AdSec_Arbitrator::SetDataDecimation cmd(adsec_timeout, params);	// TODO obtain from somewhere!!!
		_logger->log(Logger::LOG_LEV_INFO, "Requesting SetDataDecimation to AdSecArbitrator...");
		reply = _adSecArbIntf->requestCommand(&cmd);
		setCommandResultFromReply(reply);
		if(reply->isSuccess()) 
         ret = SysCmdSuccess;
      else if ( reply->getStatus() != Command::CMD_EXECUTION_FAILED)
         ret = SysCmdRetry;
      else
         ret = SysCmdError;
		}
	catch(ArbitratorInterfaceException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, "SetDataDecimation failed: %s", e.what().c_str());
	}

	delete reply;
   return ret;
}

AbstractSystemCommandResult AdSecSystem::closeLoop( AdSec_Arbitrator::runAoParams params) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing closeLoop...");
   AbstractSystemCommandResult ret = SysCmdError;
   _arbitrator = AbstractArbitrator::getInstance();

	Command* reply = NULL;


   // Calculate oversampling frequency
   float maxOvsFreq;
   try {
      maxOvsFreq = _arbitrator->ConfigDictionary()["MaxOvsFreq"];
   } catch(Config_File_Exception &e) {
       _logger->log(Logger::LOG_LEV_INFO, "Max OVS frequency not specified in config file. Using default value");
       maxOvsFreq = 810.0;
   }

   if ((params._loopFreq <1) || (params._loopFreq>2050)) {
		_logger->log(Logger::LOG_LEV_ERROR, "AdSecSystem::runAO() rejected loop frequency %f", params._loopFreq);
      return SysCmdError;
   }

   float ovsFreq = params._loopFreq;
   if (ovsFreq > maxOvsFreq)
      ovsFreq = maxOvsFreq;
   int maxiter=100;
   while((ovsFreq+ params._loopFreq < maxOvsFreq) && (maxiter-- >=0))
      ovsFreq += params._loopFreq;

   if (maxiter <=0) {
		_logger->log(Logger::LOG_LEV_ERROR, "AdSecSystem::runAO() Unexpected problem searching for the OVS frequency for loop frequency %f", params._loopFreq);
      return SysCmdError;
   }

   ovsFreq += 40.0; // Adjust to avoid problems with OVS recalc. onboard the secondary

   params._ovsFreq = ovsFreq;

   
   _logger->log(Logger::LOG_LEV_INFO, "Freq %f - dec %d - ovs %f\n", params._loopFreq, params._decimation, params._ovsFreq);

	try {
      AdSec_Arbitrator::RunAo cmd(adsec_timeout, params);	// TODO obtain from somewhere!!!
		_logger->log(Logger::LOG_LEV_INFO, "Requesting RunAO to AdSecArbitrator...");
		reply = _adSecArbIntf->requestCommand(&cmd);
		setCommandResultFromReply(reply);
		if(reply->isSuccess()) 
         ret = SysCmdSuccess;
      else if ( reply->getStatus() != Command::CMD_EXECUTION_FAILED)
         ret = SysCmdRetry;
      else
         ret = SysCmdError;
		}
	catch(ArbitratorInterfaceException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, "RunAO failed: %s", e.what().c_str());
	}

	delete reply;
   return ret;
}

AbstractSystemCommandResult AdSecSystem::openLoop( bool hold) {
	_logger->log(Logger::LOG_LEV_INFO, "Executing openLoop...");
   AbstractSystemCommandResult ret = SysCmdError;

	Command* reply = NULL;

	try {
      AdSec_Arbitrator::stopAoParams params;
      params._restoreShape = !hold;
      AdSec_Arbitrator::StopAo cmd(adsec_timeout, params);	// TODO obtain from somewhere!!!
		_logger->log(Logger::LOG_LEV_INFO, "Requesting StopAo to AdSecArbitrator...");
		reply = _adSecArbIntf->requestCommand(&cmd);
		setCommandResultFromReply(reply);
		if(reply->isSuccess()) 
         ret = SysCmdSuccess;
      else if ( reply->getStatus() != Command::CMD_EXECUTION_FAILED)
         ret = SysCmdRetry;
      else
         ret = SysCmdError;
		}
	catch(ArbitratorInterfaceException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, "StopAo failed: %s", e.what().c_str());
	}

	delete reply;
   return ret;
}

