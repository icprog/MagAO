#ifndef INTERFEROMETERINTERFACE_H_INCLUDE
#define INTERFEROMETERINTERFACE_H_INCLUDE

#include "Logger.h"

//#include "parapin.h"
#include "../../contrib/parapin-1.5.1-beta1/parapin.h"

// Parapin: pins used
#define TRIGGER_OUT		LP_PIN01	// in-out pin: to be configured
#define FRAME_READY_IN	LP_PIN11	// in pin

namespace Arcetri {

/*
 * @ Class: InterferometerInterface
 * Interface to the interferometer controlled by the InterferometerCtrl
 *
 * NOTE: the trigger is implemented using a parallel port pin.
 */
class InterferometerInterface {

	public:

		InterferometerInterface(int LogLevel);

		virtual ~InterferometerInterface();

		/*
		 * Enable/disable the external trigger
		 */
		bool setTrigger(bool enable);

		/*
		 * Set the type of post processing applied to measurements
		 * Available types are NONE and HDF5
		 */
		bool setPostProcessing(string type="NONE");

		/*
		 * Acquire a single measurement
		 * Returns the relative path of the remote file
		 */
		string getMeasurement(string measPrefix);

		/*
		 * Acquire a burst of measurements
		 * Returns the relative path of the remote files
		 */
		string getMeasurementBurst(int measNum, string burstName);

		/*
		 * Trigger the interferometer using the parallel port
		 */
		void trigger();

		/*
		 * Remotely post-process the existing measurements
		 * Returns:
		 * 	- SUCCESS: the relative path of the processed files
		 *  - FAILURE: an empty string
		 *  - NOT DONE, because post-processing type is NONE: throws AOException
		 */
		string postProcess(string remotePath, string measPrefix, int measNum) throw (AOException);

		/*
		 *
		 */
        string capture(int n, string name);
    
		/*
		 *
		 */
        string produce(string folder);

    
	private:

		string _interferometerCtrlMsgdIdentity;

		// The external trigger is implemented using the parallel port
		bool _parallelPortInitialized;

		Logger* _logger;

};

}

#endif /*INTERFEROMETERINTERFACE_H_INCLUDE*/
