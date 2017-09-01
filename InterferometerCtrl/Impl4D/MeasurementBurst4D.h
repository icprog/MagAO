#ifndef MEASUREMENTBURST4D_H_INCLUDE
#define MEASUREMENTBURST4D_H_INCLUDE

#include <vector>

#include "AbstractMeasurementBurst.h"

using namespace std;
using namespace Arcetri;

namespace Arcetri {
		
	// Forwarded definition to declare freindship avoiding
	// include loops
	class Factory4D;
	
	/*
	 * Defines a burst of Measurement acquired with the 4D PhaseCam 4020
	 * interferometer.
	 */
	class MeasurementBurst4D: public AbstractMeasurementBurst {
		
		public:
	
			
			
			/*
			 * --- NOTE ---
			 * Must not destroy the remotely wrapped measurement burst!!!
			 */
			~MeasurementBurst4D();
			
		private:
		
			/*
			 * Create a measurement burst
			 */
			friend class Factory4D;
			MeasurementBurst4D(int measNum, string remotePath);
		
	};
	
}

#endif /*MEASUREMENTBURST4D_H_INCLUDE*/
