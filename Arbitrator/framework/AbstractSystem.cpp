#include "framework/AbstractSystem.h"
#include "framework/AbstractArbitrator.h"

using namespace Arcetri::Arbitrator;

AbstractSystem::AbstractSystem(string systemName) {
	_logger = Logger::get(systemName);
	_logger->log(Logger::LOG_LEV_DEBUG, "Creating system %s...", systemName.c_str());
	_name = systemName;
   _cmdErrorString="";
}

AbstractSystem::~AbstractSystem() {
	_logger->log(Logger::LOG_LEV_DEBUG, "Destroying AbstractSystem %s...", _name.c_str());
}

void AbstractSystem::setCommandResult(AbstractSystemCommandResult cmdRes, string errorString) {
	_cmdResult = cmdRes;
   _cmdErrorString = errorString;
   string errStr=".";
   if (!errorString.empty()) errStr = ": "+errorString;

   switch(cmdRes) {
      case SysCmdSuccess:
	       _logger->log(Logger::LOG_LEV_INFO, "Command returns success %s", errStr.c_str());
          break;
      case SysCmdRetry:
	       _logger->log(Logger::LOG_LEV_WARNING, "Command returns retryable error %s", errStr.c_str());
          break;
      case SysCmdError:
	       _logger->log(Logger::LOG_LEV_ERROR, "Command returns error %s", errStr.c_str());
          break;
   }
}


void AbstractSystem::setCommandResultFromReply( Command *reply) {
   if (reply) {
       if (reply->isSuccess()) {
           setCommandResult(SysCmdSuccess, reply->getErrorString());
       }
       else if (reply->isNotValidated()) {
           setCommandResult( SysCmdRetry, reply->getErrorString());
       }
       else if (reply->isRetry()) {
           setCommandResult( SysCmdRetry, reply->getErrorString());
       }
       else {
           setCommandResult( SysCmdError, reply->getErrorString());
       }
   } else {
       setCommandResult( SysCmdError, "NULL reply");
   }
}




