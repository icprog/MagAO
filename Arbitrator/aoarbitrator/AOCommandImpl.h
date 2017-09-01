#ifndef AOCOMMANDIMPL_H_INCLUDE
#define AOCOMMANDIMPL_H_INCLUDE

#include "arblib/aoArb/AOCommands.h"

#include "framework/CommandImpl.h"
#include "aoarbitrator/AOArbitrator.h"
#include "aoarbitrator/WfsArbSystem.h"
#include "aoarbitrator/AdsecArbSystem.h"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

using namespace Arcetri::Arbitrator;
using namespace Arcetri::AdSec_Arbitrator;
using namespace Arcetri::Wfs_Arbitrator;

class AOCommandImpl: public CommandImpl {
	
	public:
		
		AOCommandImpl(Command* command, map<string, AbstractSystem*> wfsAndAdSec);
		
		virtual ~AOCommandImpl();

      AbstractSystemCommandResult executeImpl() throw(CommandExecutionException, CommandCanceledException);

      virtual AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, ArbitratorInterfaceException)
         { return SysCmdSuccess; }

		
		WfsArbSystem* getWfs(string wfsSpec="");
		AdsecArbSystem* getAdSec() { return (AdsecArbSystem*)(_systems[AO_ADSEC_FLAO]); }
		
	protected:
		
		boost::thread* _thExecWfs;
		boost::thread* _thExecAdSec;
		
		boost::thread* _thCancelWfs;
		boost::thread* _thCancelAdSec;
		
};

#endif /*AOCOMMANDIMPL_H_INCLUDE*/
