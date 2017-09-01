#ifndef FACTORY4D_H_INCLUDE
#define FACTORY4D_H_INCLUDE

#include "AbstractFactory.h"
#include "Measurement4D.h"
#include "MeasurementBurst4D.h"

#include "Python.h"

using namespace Arcetri;

namespace Arcetri {


	/*
	 * Factory class for Mask4D, Measurement4D and MeasurementBurst4D.
	 * objects.
	 */
	class Factory4D: public AbstractFactory {

		public:

			/*
			 * Return the singleton instance of the Factory object
			 */
			static Factory4D* instance();

			~Factory4D();

			bool enableTrigger(bool enable);

			bool setPostProcessing(string ppType);

			/*
			 * Acquires a measurement and returns it
			 * In case of failure throws an exception
			 */
			AbstractMeasurement* getMeasurement(string measName) throw (AOException);

			/*
			 * Acquires a burst and returns it
			 * In case of failure throws an exception
			 */
			AbstractMeasurementBurst* getMeasurementBurst(int measNum, string burstName) throw (AOException);

			/*
			 * Post-process measurements and returns the remote path, or an empty
			 * string if post-processing is set to NONE
			 * In case of failure throws an exception
			 */
			string postProcess(string remotePath, string measPrefix, int measNum) throw (AOException);

			/*
             * capture
			 */
            string capture(int n, string name) throw (AOException);
	                
			/*
             * produce
			 */
            string produce(string name) throw (AOException);

		private:

			Factory4D() throw (AOException, Config_File_Exception);

			// Reference to the Client class of the python I4D.Client module
			static PyObject* _remote4DClient;

	};

}

#endif /*FACTORY4D_H_INCLUDE*/
