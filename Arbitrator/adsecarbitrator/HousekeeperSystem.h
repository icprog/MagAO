#ifndef HOUSEKEEPERSYSTEM_H_INCLUDE
#define HOUSEKEEPERSYSTEM_H_INCLUDE

#include "framework/AbstractSystem.h"

using namespace Arcetri::Arbitrator;


namespace Arcetri{
namespace AdSec_Arbitrator {
	
class HousekeeperSystem: public AbstractSystem {
	
   private:
      bool _simulation;

	public:
	
		HousekeeperSystem(bool simulation=false);
		
		virtual ~HousekeeperSystem();
		
};

}
}

#endif /*HOUSEKEEPERSYSTEM_H_INCLUDE*/
