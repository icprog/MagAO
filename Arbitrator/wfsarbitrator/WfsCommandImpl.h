#ifndef WFSCOMMANDIMPL_H_INCLUDE
#define WFSCOMMANDIMPL_H_INCLUDE

#include "framework/CommandImpl.h"

#include "wfsarbitrator/WfsArbitrator.h"
#include "wfsarbitrator/WfsSystem.h"
#include "wfsarbitrator/MastDiagnSystem.h"
#include "wfsarbitrator/OptLoopDiagSystem.h"
#include "wfsarbitrator/AdSecSystem.h"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace Wfs_Arbitrator {

class WfsCommandImpl: public CommandImpl {
	
	public:
		
		WfsCommandImpl(Command* command, map<string, AbstractSystem*> wfsSystems);
		
		virtual ~WfsCommandImpl();

      AbstractSystemCommandResult executeImpl() throw(CommandExecutionException, CommandCanceledException);

      virtual AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, ArbitratorInterfaceException)
            { return SysCmdSuccess; }

		
		WfsSystem* getWfs() { return (WfsSystem*)(_systems["WFS"]); }
		MastDiagnSystem* getMastDiagn() { return (MastDiagnSystem*)(_systems["MASTDIAGN"]); }
		AdSecSystem* getAdSec() { return (AdSecSystem*)(_systems["ADSEC"]); }
		OptLoopDiagSystem* getOptLoopDiag() { return (OptLoopDiagSystem*)(_systems["OPTLOOPDIAG"]); }
		
	protected:
		
		boost::thread* _thExecWfs;
		boost::thread* _thExecOpt;
		boost::thread* _thCancelWfs;
};

}
}

#endif /*WFSCOMMANDIMPL_H_INCLUDE*/
