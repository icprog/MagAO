#ifndef ADSECSYSTEM_H_INCLUDE
#define ADSECSYSTEM_H_INCLUDE

#include "framework/AbstractSystem.h"
#include "arblib/base/Alerts.h"

#include "arblib/base/ArbitratorInterface.h"
#include "arblib/adSecArb/AdSecPTypes.h"

using namespace Arcetri;
using namespace Arcetri::Arbitrator;

namespace Arcetri{
namespace Wfs_Arbitrator {

class AdSecSystem: public AbstractSystem {

	public:

		AdSecSystem();

		virtual ~AdSecSystem();

		AbstractSystemCommandResult setDataDecimation( AdSec_Arbitrator::dataDecimationParams params);
		AbstractSystemCommandResult closeLoop( AdSec_Arbitrator::runAoParams params);
		AbstractSystemCommandResult openLoop( bool hold);

                bool check();


	private:

		AbstractArbitrator* _arbitrator;

		// Used to control AdSec subsystem
		ArbitratorInterface* _adSecArbIntf;
};

}
}

#endif /*ADSECSYSTEM_H_INCLUDE*/
