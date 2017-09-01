/*
 * @File: AbstractFactory.h
 * Abstract class for Factory objects related to a generic interferometer.
 * 
 * This class must be derived and implemented to use a specific interferometer.
 */ 

#ifndef MEASUREMENTFACTORY_H_INCLUDE
#define MEASUREMENTFACTORY_H_INCLUDE

#include <string>

#include "Logger.h"
#include "stdconfig.h"

#include "AbstractMeasurement.h"
#include "AbstractMeasurementBurst.h"

using namespace std;
using namespace Arcetri;


namespace Arcetri {
	
	/*
	 * Factory class able to: 
	 *  - Enable/disable hw trigger
	 * 	- Create AbstractMeasurement
	 * 	- Create AbstractMeasurementBurst
	 *  - ...
	 * 
	 * This class is a singleton and must be derived to actually get 
	 * the needed instance
	 * The derived class must initialize the interferometer and provides
	 * implemnetations of pure abstract methods
	 * 
	 * i.e. see Impl4D/Factory4D
	 */
	class AbstractFactory {
	
		public:
		
			/*
			 * Destroy the abstract factory
			 */
			virtual ~AbstractFactory();
			
			/*
			 * Returns the interferometer name
			 */
			static string getInterferometerName() { return _config["InterferometerName"]; }
			
			/*
			 * Returns the installed logger
			 */
			static Logger* getLogger() { return _logger; }
		
			/*
			 * Enable or disable the interferometer hw trigger
			 */
			virtual bool enableTrigger(bool enable) = 0;
			
			/*
			 * Set the wanted type of post processing for data
			 * By default disable post processing
			 */
			 virtual bool setPostProcessing(string ppType) { return false; }
		
			/*
			 * Acquire a measurement from the interferometer and returns
			 * the corresponding object
			 */
			virtual AbstractMeasurement* getMeasurement(string measName) = 0;
			
			/*
			 * Acquire a measurement burst from the interferometer and returns
			 * the corresponding object
			 */
			virtual AbstractMeasurementBurst* getMeasurementBurst(int measNum, string burstName) = 0;
			
			/*
			 * Perform the post processing set with setPostProcessing(ppType)
			 */
			virtual string postProcess(string remotePath, string measPrefix, int measNum) {}
			

			/*
             * capture
			 */
            virtual string capture(int n, string name) {};
	                
			/*
             * produce
			 */
            virtual string produce(string name) {};


		protected:
		
			/*
			 * Create an abstract factory from a configFile
			 */
			AbstractFactory(string configFile) throw (Config_File_Exception);
			
		protected:	// FIELDS
			
			/*
			 * Interferometer .conf file
			 */
			static Config_File _config;
			
			/*
			 * Singleton object
			 */
			static AbstractFactory* _factory;
			
			/*
			 * Main logger (name got from config file)
			 */
			static Logger* _logger;		
		
	};
	
}

#endif /*ABSTRACTMEASUREMENTFACTORY_H_INCLUDE*/
