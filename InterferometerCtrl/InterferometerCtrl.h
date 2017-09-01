
#ifndef INTERFEROMETERCONTROLLER_H_INCLUDE
#define INTERFEROMETERCONTROLLER_H_INCLUDE

#include "AOApp.h"

#include "AbstractFactory.h"
#include "AbstractMeasurement.h"
#include "AbstractMeasurementBurst.h"

namespace Arcetri {

/*
 * @ Class: InterferometerCtrl
 * Controller for a generic interferometer.
 * 
 * Defines a set of operations supported by a generic interferometer, and the 
 * type of informations returned
 * 
 * The actual interferometer initialization is specified in the Create() method,
 * assigning an concrete instance to following field:
 * 
 * 	AbstractFactory* _measFactory
 * 
 * @
 */
class InterferometerCtrl: public AOApp {

	public:
		
		InterferometerCtrl(int argc, char **argv) throw (AOException);
		
		void Create() throw (AOException);
              			 
        ~InterferometerCtrl();  
        
	
	private: // AOAPP FEATURES
	
		/*
		 * 
		 */
		void SetupVars();
		
		/*
		 * 
		 */
		void InstallHandlers();
		
		/*
		 * Perform some post init:

		 *  - ...
		 */
		void PostInit();
		
		/*
		 * 
		 */
		static int commandsHndlr(MsgBuf *msgb, void *argp, int hndlrQueueSize);
	
		 
		 /*
		  * Overridden AOApp run method
		  */
		 void Run();
		 
		 
	private:	// GENERIC INTERFEROMETER FEATURES
	
		/*
		 * Enable or disable the interferometer hw trigger
		 */
		bool enableTrigger(bool enable);

		/*
		 * Set the type of the post-processing
		 * By default disable the post processing
		 * Available post-processing types: NONE, HDF5
		 */
		bool setPostProcessing(string ppType = "NONE");

		/*
		 * Acquire a new measurement and returns it
		 */
		AbstractMeasurement* getMeasurement(string measName);

		/*
		 * Acquire a burst of new measurements and return it
		 *
		 * 'burstName' is automatically generated if not given
		 */
		AbstractMeasurementBurst* getMeasurementBurst(int measNum, string burstName = "");

		/*
		 * Perform the post processing previously set with setPostProcessing(ppType)
		 * Return the destination path (for N measurements) or the destination
		 * file (for a single measurement)
		 * 
		 * *** NOTE ****
		 * The remotePath is relative to a ROOT set by the specific interferometer
		 * implementation (i.e. Constants.I4D_DATA_PATH defined in PyModules/I4D/Commons.py)
		 */
		string postProcess(string remotePath, string measPrefix, int measNum = 1);
		
                //
                string capture(int n, string name);
                
                //
                string produce(string name);

	//----------------------- FIELDS ----------------------//
	
	private:
	
		bool   _enableExtTrigger;
		string _postProcType;
	
	
		/*
		 * This field defines the actually implemented controller.
		 */
		AbstractFactory* _measFactory;

};

}

#endif //INTERFEROMETERCONTROLLER_H_INCLUDE

