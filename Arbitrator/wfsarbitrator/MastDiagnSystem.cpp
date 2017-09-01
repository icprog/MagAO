#include "wfsarbitrator/MastDiagnSystem.h"
#include "wfsarbitrator/WfsArbitrator.h"

using namespace Arcetri::Wfs_Arbitrator;

MastDiagnSystem::MastDiagnSystem(): AbstractSystem("MASTDIAGN") {

   _arbitrator = AbstractArbitrator::getInstance();

   _varname = "masterdiagnostic."+AOApp::Side() + ".CLOSELOOP.REQ";

}

MastDiagnSystem::~MastDiagnSystem() {
	
}

AbstractSystemCommandResult MastDiagnSystem::openLoop() {

   int intzero=0;

   _logger->log(Logger::LOG_LEV_INFO, "Executing openLoop...");
   AbstractSystemCommandResult cmdRes;
   string errString;

   cmdRes = SysCmdSuccess;
   int stat = thWriteVar( _varname.c_str(), INT_VARIABLE, 1, &intzero, 2000);
   if (IS_ERROR(stat))
      cmdRes = SysCmdRetry;

   return cmdRes;
}

AbstractSystemCommandResult MastDiagnSystem::closeLoop() {

   int intuno=1;

   _logger->log(Logger::LOG_LEV_INFO, "Executing closeLoop...");
   AbstractSystemCommandResult cmdRes;
   string errString;

   cmdRes = SysCmdSuccess;
   int stat = thWriteVar( _varname.c_str(), INT_VARIABLE, 1, &intuno, 2000);
   if (IS_ERROR(stat))
      cmdRes = SysCmdRetry;

   return cmdRes;
}




