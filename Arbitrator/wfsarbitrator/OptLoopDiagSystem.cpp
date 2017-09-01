#include "wfsarbitrator/OptLoopDiagSystem.h"
#include "wfsarbitrator/WfsArbitrator.h"
#include "../../OptLoopDiagnostic/OptLoopDiagnosticStructs.h"


using namespace Arcetri::Wfs_Arbitrator;


OptLoopDiagSystem::OptLoopDiagSystem(): AbstractSystem("OPTLOOPDIAG") {

   _arbitrator = AbstractArbitrator::getInstance();
   int stat;;
   _logger->log(Logger::LOG_LEV_INFO, "Installing custom handlers ...");
   if((stat=thHandler(OPTSAVE_REPLY, (char *)"*", 0, optsavereply_handler, (char *)"optsavereply_handler", this))<0) {
      _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
//      _arbitrator->SetTimeToDie(true);
   } 

   _target = "optloopdiag."+AOApp::Side();
   _framesToSave = 0;

   pthread_mutex_init( &_replyMutex, NULL);
   pthread_cond_init( &_replyCond, NULL);

   pthread_mutex_init( &_firstReplyMutex, NULL);
   pthread_cond_init( &_firstReplyCond, NULL);

   pthread_mutex_init( &_dataMutex, NULL);

   _saving = false;
   _senderClient="";

}

int OptLoopDiagSystem::optsavereply_handler(MsgBuf *msgb, void *argp, int /* hndlrQueueSize */) {

   OptLoopDiagSystem* myself = (OptLoopDiagSystem*)argp;
   optsave_reply *reply =  ((optsave_reply *) MSG_BODY(msgb));

   pthread_mutex_lock(&myself->_dataMutex);
   myself->_filename = reply->outfile;
   myself->_tracking = myself->_filename.substr( myself->_filename.size() - 16, 15);
   pthread_mutex_unlock(&myself->_dataMutex);

   // Bounce the message to the original sender, if any
   if (myself->_senderClient.compare("") != 0)
      thSendMsg( sizeof(optsave_reply), myself->_senderClient.c_str(), OPTSAVE_REPLY, 0, MSG_BODY(msgb));

   // Always unblock firstReply on any reply received
   printf("Received reply (%d of %d frames saved)\n", reply->saved, myself->_framesToSave);
   pthread_cond_signal( &myself->_firstReplyCond);

   // If the acquisition is finished, release the command thread
   if (reply->saved == myself->_framesToSave) {
      pthread_cond_signal( &myself->_replyCond);
      myself->_saving = false;
   }

   thRelease(msgb);
   return NO_ERROR;
}


OptLoopDiagSystem::~OptLoopDiagSystem() {
	
}

AbstractSystemCommandResult OptLoopDiagSystem::save( saveOptLoopDataParams params, bool join, string senderClient) {

   if (!thHaveYou(_target.c_str())) {
       setErrorString("Process "+_target+" not found");
       return SysCmdRetry;
   }
     
   _logger->log(Logger::LOG_LEV_INFO, "Executing save...");

   optsave_cmd cmd;
   cmd.saveFrames = params._saveFrames;
   cmd.saveSlopes = params._saveSlopes;
   cmd.saveModes = params._saveModes;
   cmd.saveFFCommands = params._saveCommands;
   cmd.saveDistAverages = params._savePositions;
   cmd.nFrames = params._nFrames;
   cmd.saveWfsStatus = false;    // Can't receive a SaveStatus command while we are executing another command...
   cmd.saveAdSecStatus = true;

   AbstractSystemCommandResult cmdRes = SysCmdSuccess;
   string errString;

   _saving = true;
   _senderClient = senderClient;

   int stat = thSendMsg( sizeof(cmd), _target.c_str(), OPTSAVE, 0, (void*)&cmd);
   if (IS_ERROR(stat)) {
      errString= lao_strerror(stat);
      cmdRes = SysCmdError;
      return cmdRes;
   }

   _framesToSave = params._nFrames;

   // Wait for the first reply
   pthread_mutex_lock(&_firstReplyMutex);
   pthread_cond_wait( &_firstReplyCond, &_firstReplyMutex);
   pthread_mutex_unlock( &_firstReplyMutex);

   if (join)
      this->join();

   return cmdRes;
}


void OptLoopDiagSystem::join() {


   // The order of these checks should prevent race conditions with optsavereply_handler() which is unblocking us.

   // If already done, exit immediately
   if (!_saving)
      return;

   // Otherwise for the acquisition to finish.
   pthread_mutex_lock(&_replyMutex);
   pthread_cond_wait( &_replyCond, &_replyMutex);
   pthread_mutex_unlock( &_replyMutex);

}

void OptLoopDiagSystem::cancelCommand() {

   // Unblocks any pending condition variable
   pthread_cond_signal( &_firstReplyCond);
   pthread_cond_signal( &_replyCond);
}


string OptLoopDiagSystem::getTrackingNum() {

   pthread_mutex_lock(&_dataMutex);
   string s = _tracking;
   pthread_mutex_unlock(&_dataMutex);
   return s;
}

string OptLoopDiagSystem::getFilename() {

   pthread_mutex_lock(&_dataMutex);
   string s = _filename;
   pthread_mutex_unlock(&_dataMutex);
   return s;
}

void OptLoopDiagSystem::enableAntiDrift( bool enable) {

   int intzero=0;
   int intuno =1;

   _logger->log(Logger::LOG_LEV_INFO, "Executing enableAntiDrift(%s)...", enable ? "true":"false");
   AbstractSystemCommandResult cmdRes;
   string errString="";

   string varname =  _target+".DRIFT.ENABLE.REQ";
  _logger->log(Logger::LOG_LEV_TRACE, "Varname is %s\n", varname.c_str());

   cmdRes = SysCmdSuccess;
   int stat = thWriteVar( varname.c_str(), INT_VARIABLE, 1, enable? (&intuno) : (&intzero), 2000);
   if (IS_ERROR(stat)) {
      _logger->log(Logger::LOG_LEV_ERROR, "Error in thWriteVar: (%d) %s", stat, lao_strerror(stat));
      errString = "Error in enableAntiDrift()";
      cmdRes = SysCmdRetry;
   }

}

bool OptLoopDiagSystem::isAntiDriftEnabled() {

   string varname = _target+".DRIFT.ENABLE.CUR";
  _logger->log(Logger::LOG_LEV_TRACE, "Varname is %s\n", varname.c_str());
   int stat;
   bool ret = false;
   MsgBuf *msgb = thGetVar( varname.c_str(), 0, &stat);
   if (msgb) {
      Variable *var = thValue(msgb);
      ret = var->Value.Lv[0] != 0;
      thRelease(msgb);
   } else {
      _logger->log(Logger::LOG_LEV_ERROR, "Error in thGetVar: (%d) %s", stat, lao_strerror(stat));
   }

   return ret;
}

