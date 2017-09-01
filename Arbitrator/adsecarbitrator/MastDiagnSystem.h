#ifndef MASTDIAGNSYSTEM_H_INCLUDE
#define MASTDIAGNSYSTEM_H_INCLUDE

#include "framework/AbstractSystem.h"
#include "arblib/adSecArb/AdSecPTypes.h"


using namespace Arcetri::Arbitrator;
using namespace Arcetri::AdSec_Arbitrator;

namespace Arcetri{
namespace AdSec_Arbitrator {
	
class MastDiagnSystem: public AbstractSystem {
	
	public:
	
		MastDiagnSystem(bool simulation=false);
		
		virtual ~MastDiagnSystem();
		
		void cancelCommand();

      void setDecimation( int timeout_ms, int decimation);
      void setDestination( int timeout_ms, string addr);

   protected:
      string _mastdClient;

      AbstractArbitrator* _arbitrator;

   private:
      bool _simulation;
};

}
}

#endif /*MASTDIAGNSYSTEM_H_INCLUDE*/
