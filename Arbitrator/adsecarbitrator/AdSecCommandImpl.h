#ifndef ADSECCOMMANDIMPL_H_INCLUDE
#define ADSECCOMMANDIMPL_H_INCLUDE

#include "framework/CommandImpl.h"

#include "adsecarbitrator/AdSecArbitrator.h"
#include "adsecarbitrator/HousekeeperSystem.h"
#include "adsecarbitrator/FastDiagnSystem.h"
#include "adsecarbitrator/MastDiagnSystem.h"
#include "adsecarbitrator/IdlSystem.h"
#include "adsecarbitrator/AdamSystem.h"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace AdSec_Arbitrator {

class AdSecCommandImpl: public CommandImpl {
	
	public:
		
		AdSecCommandImpl(Command* command, map<string, AbstractSystem*> adSecSystems);
		
		virtual ~AdSecCommandImpl();

      AbstractSystemCommandResult executeImpl() throw(CommandExecutionException, CommandCanceledException);

      virtual AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, ArbitratorInterfaceException)
         { return SysCmdSuccess; }

		
		HousekeeperSystem* getHK() { return (HousekeeperSystem*)(_systems["HOUSEKEEPER"]); }
		FastDiagnSystem* getFD()   { return (FastDiagnSystem*)(_systems["FASTDIAGN"]); }
		IdlSystem* getIdl() 	{ return (IdlSystem*)(_systems["IDL"]); }
		MastDiagnSystem* getMD() 	{ return (MastDiagnSystem*)(_systems["MASTDIAGN"]); }
		AdamSystem* getAdam() 	{ return (AdamSystem*)(_systems[ADAMSYSTEM_NAME]); }
		
	protected:
		
		boost::thread* _thExecHK;
		boost::thread* _thExecFD;
		boost::thread* _thExecIdl;
		boost::thread* _thExecMD;
		
		boost::thread* _thCancelHK;
		boost::thread* _thCancelFD;
		boost::thread* _thCancelIdl;
		boost::thread* _thCancelMD;
};

}
}

#endif /*ADSECCOMMANDIMPL_H_INCLUDE*/
