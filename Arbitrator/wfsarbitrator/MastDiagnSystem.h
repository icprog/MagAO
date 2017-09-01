#ifndef MASTDIAGNSYSTEM_H_INCLUDE
#define MASTDIAGNSYSTEM_H_INCLUDE

#include "framework/AbstractSystem.h"

using namespace Arcetri::Arbitrator;

namespace Arcetri{
namespace Wfs_Arbitrator {
	
class MastDiagnSystem: public AbstractSystem {
	
	public:
	
		MastDiagnSystem();
		
		virtual ~MastDiagnSystem();
		
		AbstractSystemCommandResult openLoop();
		
		AbstractSystemCommandResult closeLoop();
		
		void cancelCommand();

   protected:
      string _varname;

      AbstractArbitrator* _arbitrator;

};

}
}

#endif /*MASTDIAGNSYSTEM_H_INCLUDE*/
