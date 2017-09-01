/*
 * @File: HexapodCtrl.h
 * 
 * \HexapodCtrl\ class declaration
 * 
 * This file contains the declaration of \HexapodCtrl\ class, an AOApp
 * which controls the hexapod. 
 * HexapodCtrl implements the commands of hexapodlib as the AOS subsystem. 
 * Use HexapodCtrl in place of AOS in case you don't want to use the TCS software 
 * @
 */

#ifndef HEXAPODCTRL_H_INCLUDE
#define HEXAPODCTRL_H_INCLUDE


#include "AOApp.h"
#include "Hexapod.h"
#include "RTDBvar.h"
using namespace Arcetri;


/*
 * @Class{API}: HexapodCtrl
 *
 * Controller managing the communication with the Hexapod.
 * 
 * !!!!!!!! This is only for the lab tests when using the whole AOS-TCS-OSS path is unwanted !!!!!!!!!
 * 
 * At the telescope, the hexapod will be managed by the TCS/OSS and the AO software will
 * ask the AOS to move the hexapod.
 * AO clients must link the aoslib to ask hexapod service to either the AOS or the HexapodCtrl.
 * 
 * @
 */ 
class HexapodCtrl: public AOApp {

	//----------------------- METHODS ----------------------//

	public:
		
		HexapodCtrl(int argc, char **argv) throw (AOException);
              			 
        ~HexapodCtrl();
        
	
	private:
	
        void Create() throw (AOException);

		/*
		 * Setup RTDB variables
		 */
		void SetupVars();
		
		/*
		 * Installa handlers fos synch and asynch (getPos) commands
		 */
		void InstallHandlers();
		
		/*
		 * Perform some post init:
		 * 	- Set the default position values
		 *  - ...
		 */
		void PostInit();
		
		/*
		 * Thlib handler for commands
		 */
		static const int HNDLR_QUEUE_LIMIT = 10;
		static int commandsHandler(MsgBuf *msgb, void *argp, int hndlrQueueSize);
		
		/*
		 * Updates the RTDB variables that store the current hexapod 
		 * position and log 
    	 */
		 void updateHexapodPositionVars();
		 
		/*
		 * Get and log the current hexapod status
		 */
		 void updateHexapodStatus();
		 
		 /*
		  * Overridden AOApp run method
		  */
		 void Run();
		
	//----------------------- FIELDS ----------------------//
	
	private:
	
		static Hexapod* _hexapod;
		
		int _posUpdatePeriod_us;
		
		RTDBvar _hexaPos;

};

#endif //HEXAPODCTRL_H_INCLUDE

