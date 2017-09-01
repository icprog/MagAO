#include "wfsarbitrator/WfsCommandImpl.h"

using namespace Arcetri::Wfs_Arbitrator;

WfsCommandImpl::WfsCommandImpl(Command* command, map<string, AbstractSystem*> wfsSystems): CommandImpl(command, wfsSystems) {
	_thExecWfs = NULL;
	_thCancelWfs = NULL;
}

WfsCommandImpl::~WfsCommandImpl() {
	
}

AbstractSystemCommandResult WfsCommandImpl::executeImpl() throw(CommandExecutionException, CommandCanceledException) {

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

