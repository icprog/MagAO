#ifndef MEASUREMENT4D_H_INCLUDE
#define MEASUREMENT4D_H_INCLUDE

#include "AbstractMeasurement.h"

using namespace Arcetri;

namespace Arcetri {
	
		
	// Forwarded definition to declare freindship avoiding
	// include loops
	class Factory4D;
	
	/*
	 * Defines a Measurement acquired with the 4D PhaseCam 4020
	 * interferometer.
	 * 
	 * It knows how to acquire and get the measurements, that is
	 * using the I4D class (lib for the PhaseCam 4020)
	 */
	class Measurement4D: public AbstractMeasurement {
		
		public:
			
			/*
			 * --- NOTE ---
			 * Must not destroy the remotely wrapped measurement!!!
			 */
			~Measurement4D();
			
			
		private:
		
			/*
			 * Create a measurement burst
			 */
			friend class Factory4D;
			Measurement4D(string remoteFileName);
		
	};
	
}

#endif /*MEASUREMENT4D_H_INCLUDE*/
