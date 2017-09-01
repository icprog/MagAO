#include "adsecarbitrator/AdSecCommandImpl.h"

using namespace Arcetri::AdSec_Arbitrator;

AdSecCommandImpl::AdSecCommandImpl(Command* command, map<string, AbstractSystem*> adSecSystems): CommandImpl(command, adSecSystems) {
	_thExecHK = NULL;
	_thExecFD = NULL;
	_thExecIdl = NULL;
	_thExecMD = NULL;
	_thCancelHK = NULL;
	_thCancelFD = NULL;
	_thCancelIdl = NULL;
	_thCancelMD = NULL;
}

AdSecCommandImpl::~AdSecCommandImpl() {
	
}

AbstractSystemCommandResult AdSecCommandImpl::executeImpl() throw(CommandExecutionException, CommandCanceledException) {

   try {
      return doExecuteImpl();
   } catch (AOException &e) {
      _logger->log(Logger::LOG_LEV_ERROR,"%s failed: %s\n", getCommand()->getDescription().c_str(), e.what().c_str());
      getCommand()->setErrorString(e.what());
      return SysCmdError;
   } catch (...) {
      _logger->log(Logger::LOG_LEV_ERROR,"%s failed: unknown exception", getCommand()->getDescription().c_str());
      return SysCmdError;
   }

}

