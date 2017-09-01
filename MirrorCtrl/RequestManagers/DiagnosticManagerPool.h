#ifndef DIAGNOSTICMANAGERPOOL_H_INCLUDE
#define DIAGNOSTICMANAGERPOOL_H_INCLUDE

#include "RequestManagers/DiagnosticManager.h"

/*
 * @Class: DiagnosticManagerGroups
 * Defines different groups to control a set
 * of mutually exclusive diagnostic managers.
 */
class DiagnosticManagerGroups {
	
	public:
		static const unsigned int FASTDIAGN_ID = 0;	
		static const unsigned int SLOPE_COMP_ID = 1;
		static const unsigned int TECH_VIEW_ID = 2;
};


/*
 * @Class: DiagnosticManagerPool
 * Control a pool of diagnostic managers.
 * @
 */
class DiagnosticManagerPool {

	public:
	
		DiagnosticManagerPool(int logLevel);
		
		~DiagnosticManagerPool();
		
		/*
		 * Returns the number of existing diagnostic managers
		 */
		unsigned int size() { return _diagnManagers.size(); }
		
		/*
		 * Add a diagnostic manager to the pool.
		 * An added diagnostic manager isn't started: it can be started/stoped
		 * but can't be removed from the pool (not useful).
		 * Return true if the object has been added (doesn't exceed the
		 * max allowed number)
		 */
		bool add(DiagnosticManager* dm);
			
		/*
		 * Start a DiagnosticManager thread by groupId and frame size, stopping
		 * the others diagnostic managers of the same group.
		 * If frameSize is zero it isn't considered, because it means that the group
		 * contains only one diagnostic manager.
		 * Returns the id of the started diagnostic manager, that can be used
		 * to wait for it to finish.
		 */										    
	    unsigned int startDiagnosticManager(unsigned int groupId, 
	     									unsigned int frameSizePx = 0) throw (UnexistingDiagnosticManagerException,
						 											     		 DiagnosticManagerPoolStartException);
		
		
		/*
		 * 
		 */
		void stopDiagnosticManager(unsigned int id);
		
		
		/*
		 * 
		 */
		void stopDiagnosticManagersByGroup(unsigned int groupId);
		
		
		/*
		 * Wait the diagostic manager thread to finish.
		 */				
		void waitDiagnosticManager(unsigned int id);
						 											   						 
	
	private:
	
		// The maximum number of allowed disgnostic managers.
		// This is usefull to index the threads using a simple array.
		static const unsigned int _MAX_DIAGN_MAN_NUM = 10;
	
		vector<DiagnosticManager*> _diagnManagers;
		pthread_t 		   		   _threads[_MAX_DIAGN_MAN_NUM];
		
		/*
		 * Start a DiagnosticManager thread by index
		 * Throw DiagnosticManagerPoolStartException if the thread isn't started,
		 * i.e. if is not enabled.
		 * Throw UnexistingDiagnosticManagerException if the diagnostic manager doesn't exist in the pool.
		 */
		void startDiagnosticManagerInternal(unsigned int index) throw (UnexistingDiagnosticManagerException);
		
		Logger* _logger;
		
		

};

#endif /*DIAGNOSTICMANAGERPOOL_H_INCLUDE*/
