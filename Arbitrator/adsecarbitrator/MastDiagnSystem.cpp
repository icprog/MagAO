#include "adsecarbitrator/MastDiagnSystem.h"
#include "adsecarbitrator/AdSecArbitrator.h"

using namespace Arcetri::AdSec_Arbitrator;

MastDiagnSystem::MastDiagnSystem(bool simulation): AbstractSystem("MASTDIAGN") {

   _mastdClient = "masterdiagnostic."+AOApp::Side();
   _arbitrator = AbstractArbitrator::getInstance();

   _simulation=simulation;
}

MastDiagnSystem::~MastDiagnSystem() {
	
}

void MastDiagnSystem::setDecimation( int timeout_ms, int decimation) {

   if(_simulation) {
      _logger->log(Logger::LOG_LEV_INFO, "SIMULATION: Executing setDecimation, client %s", _mastdClient.c_str());
      setCommandResult(SysCmdSuccess);
      return;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Executing setDecimation, client: %s dec: %d", _mastdClient.c_str(), decimation);
   AbstractSystemCommandResult cmdRes;

   int seq = thSendMsg( sizeof(int), _mastdClient.c_str(), MASTDIAGN_SET_DECIMATION, 0, &decimation);
   _logger->log(Logger::LOG_LEV_INFO, "Sequence number: %d", seq);
   if (IS_ERROR(seq)) {
      cmdRes = SysCmdRetry;
   } else {
      int errc;
      MsgBuf *msgb = thWaitMsg(MASTDIAGN_SET_DECIMATION, "*", seq, timeout_ms, &errc);
      if (!msgb) {
         Error error("SetDecimation: No answer from MasterDiagnostic", false);
         _arbitrator->notifyClients(&error);
         cmdRes = SysCmdError;
      }
      else if ( ((int *)MSG_BODY(msgb))[0] != 0) {
         ostringstream s;
         s << "SetDecimation answer:" << ((int *)MSG_BODY(msgb))[0];
         Error error(s.str(), false);
         _arbitrator->notifyClients(&error);
         cmdRes = SysCmdError;
      }
      else
         cmdRes = SysCmdSuccess;
   }

   if(cmdRes == SysCmdRetry) {
      Warning warning("SetDecimation not executed (retry)");
      _arbitrator->notifyClients(&warning);
   }
   else if(cmdRes == SysCmdError) {
      Error error("SetDecimation failed", false);
      _arbitrator->notifyClients(&error);
   }
   setCommandResult(cmdRes);
}



void MastDiagnSystem::setDestination( int timeout_ms, string addr) {

   if(_simulation) {
      _logger->log(Logger::LOG_LEV_INFO, "SIMULATION: Executing setDestination, client %s", _mastdClient.c_str());
      setCommandResult(SysCmdSuccess);
      return;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Executing setDestination, client: %s dest: %s", _mastdClient.c_str(), addr.c_str());
   AbstractSystemCommandResult cmdRes;

   int seq = thSendMsg( addr.size()+1, _mastdClient.c_str(), MASTDIAGN_SET_BOUNCER_ADDR, 0, addr.c_str());

   _logger->log(Logger::LOG_LEV_TRACE, "Sequence number: %d", seq);
   if (IS_ERROR(seq)) {
      cmdRes = SysCmdRetry;
   } else {
      int errc;
      MsgBuf *msgb = thWaitMsg(MASTDIAGN_SET_BOUNCER_ADDR, "*", seq, timeout_ms, &errc);
      if (!msgb) {
         Error error("setDestination: No answer from MasterDiagnostic", false);
         _arbitrator->notifyClients(&error);
         cmdRes = SysCmdError;
      }
      else if ( ((int *)MSG_BODY(msgb))[0] != 0) {
          if (((int *)MSG_BODY(msgb))[0] == NOT_INIT_ERROR) {
              // adsec bouncer not enabled, return warning
              cmdRes = SysCmdRetry;
              _logger->log(Logger::LOG_LEV_INFO, "AdSecBouncer seems to be disabled (NOT_INIT_ERROR)");
          }
          else {
              ostringstream s;
              s << "setDestination answer:" << ((int *)MSG_BODY(msgb))[0];
              Error error(s.str(), false);
              _arbitrator->notifyClients(&error);
              cmdRes = SysCmdError;
          }
      }
      else
         cmdRes = SysCmdSuccess;
   }

   if(cmdRes == SysCmdRetry) {
      Warning warning("setDestination not executed");
      _arbitrator->notifyClients(&warning);
   }
   else if(cmdRes == SysCmdError) {
      Error error("setDestination failed", false);
      _arbitrator->notifyClients(&error);
   }
   setCommandResult(cmdRes);
}





