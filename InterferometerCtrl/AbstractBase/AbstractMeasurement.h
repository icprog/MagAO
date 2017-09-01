/*
 * @File: AbstractMeasurement.h
 * Abstract class for a Measurement object related to a generic interferometer.
 * 
 * This class must be derived and implemented (see virual methods) to use a 
 * specific interferometer
 */ 
 
#include <string>

#include "Logger.h"

using namespace std;
using namespace Arcetri;

#ifndef ABSTRACTMEASUREMENT_H_INCLUDE
#define ABSTRACTMEASUREMENT_H_INCLUDE

namespace Arcetri {
		
	/*
	 * Defines a Measurement acquired with a generic interferometer.
	 * 
	 * --- NOTE ---
	 * Usually a measurement is stored in the remote interferometer 
	 * workstation, so this class is a PROXY, basically providing only
	 * the remote file name
	 */
	class AbstractMeasurement {
		
		public:
			
			/*
			 * Remove the measurement object
			 */
			virtual ~AbstractMeasurement();
			
			/*
			 * Returns the remote relative path of
			 * the burst, without the measure filename.
			 * 
			 * The filenames (with a sequence number) depends
			 * on the specific interferometer implementation.
			 */
			string getRemoteFilename() { return _remoteFilename; } 

		
		protected:
		
			/*
			 * Creates a measurement object
			 */
			AbstractMeasurement(string remoteFilename);
			
			
		protected: // FIELDS
		
			Logger* _logger;
			
			string _remoteFilename;
			
	};
	
}

#endif /*ABSTRACTMEASUREMENT_H_INCLUDE*/
