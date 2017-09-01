/*
 * @File: AbstractMeasurementBurst.h
 * Abstract class for all AbstractMeasurementBurst objects related to a generic interferometer.
 * 
 * This class must be derived and implemented to use a specific interferometer.
 */ 

#ifndef ABSTRACTMEASUREMENTBURST_H_INCLUDE
#define ABSTRACTMEASUREMENTBURST_H_INCLUDE

#include <vector>

#include "AbstractMeasurement.h"

using namespace std;
using namespace Arcetri;

namespace Arcetri {
		
	/*
	 * Defines a burst of Measurement acquired with a generic 
	 * interferometer.
	 * 
	 * --- NOTE ---
	 * Usually a measurement burst is stored in the remote interferometer 
	 * workstation, so this class is a PROXY, basically providing only
	 * the remote path
	 */
	class AbstractMeasurementBurst {
		
		public:
			
			/*
			 * Remove the measurement burst object
			 */
			virtual ~AbstractMeasurementBurst();
			
			/*
			 * Returns the number of measurements in the burst
			 */
			int size() { return _size; }
			
			/*
			 * Returns the remote relative path of
			 * the burst, without the measure filename.
			 * 
			 * The filenames (with a sequence number) depends
			 * on the specific interferometer implementation.
			 */
			string getRemotePath() { return _remotePath; }
			
		protected:
		
			/*
			 * Creates a measurement burst object
			 */
			AbstractMeasurementBurst(int measNum, string remotePath);
			
		protected: // FIELDS
		
			Logger* _logger;
			
			int _size;
			
			string _remotePath;
			
	};
	
}

#endif /*ABSTRACTMEASUREMENTBURST_H_INCLUDE*/
