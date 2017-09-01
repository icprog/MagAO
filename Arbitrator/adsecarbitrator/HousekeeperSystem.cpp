#include "adsecarbitrator/HousekeeperSystem.h"

using namespace Arcetri::AdSec_Arbitrator;

HousekeeperSystem::HousekeeperSystem(bool simulation): AbstractSystem("HOUSEKEEPER") {
   _simulation=simulation;

}

HousekeeperSystem::~HousekeeperSystem() {
	
}
