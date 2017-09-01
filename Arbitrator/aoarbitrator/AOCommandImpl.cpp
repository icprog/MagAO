#include "aoarbitrator/AOCommandImpl.h"

AOCommandImpl::AOCommandImpl(Command* command, map<string, AbstractSystem*> wfsAndAdSec): CommandImpl(command, wfsAndAdSec) {
	
	_thExecWfs = NULL;
	_thExecAdSec = NULL;
	_thCancelWfs = NULL;
	_thCancelAdSec = NULL;
}

AOCommandImpl::~AOCommandImpl() {
	
}


WfsArbSystem* AOCommandImpl::getWfs(string wfsSpec) { 
   if(wfsSpec.empty()) { 
      AOArbitrator* aoArb = ((AOArbitrator*)getArbitrator());
      return aoArb->getActiveWFS();
   } else
      return (WfsArbSystem*)(_systems[wfsSpec]); 
}
	

AbstractSystemCommandResult AOCommandImpl::executeImpl() throw(CommandExecutionException, CommandCanceledException) {

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

