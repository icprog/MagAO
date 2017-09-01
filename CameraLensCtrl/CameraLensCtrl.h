/*
 * @File: CameraLensCtrl.h
 * 
 * \CameraLensCtrl\ class declaration
 * 
 * This file contains the declaration of \CameraLensCtrl\ class, an AOApp
 * which controls the position of the camera lens
 * @
 */

#ifndef CAMERALENSCTRL_H_INCLUDE
#define CAMERALENSCTRL_H_INCLUDE


#include "AOApp.h"
#include "BcuLib/BcuCommon.h"
#include "RTDBvar.h"
using namespace Arcetri::Bcu;


/*
 * @Class{API}: CameraLensCtrl
 * 
 * Controls the position X,Y of the camera lens. 
 * 
 * The input positions are got by way of 2 REQ RTDB variables, named 
 * CAMERALENSCTRL.POS_X.REQ and CAMERALENSCTRL.POS_Y.REQ.
 * The current lens position is stored in the 2 corresponding CUR 
 * RTDB variable, named CAMERALENSCTRL.POS_X.CUR and CAMERALENSCTRL.POS_Y.CUR.
 * @
 */ 
class CameraLensCtrl: public AOApp {

	//----------------------- METHODS ----------------------//

	public:
		
		CameraLensCtrl( int argc, char **argv)
              			 throw (AOException);

              			 
        ~CameraLensCtrl();
        
	
	private:
	
      void Create() throw (AOException);

		/*
		 * Setup RTDB variables
		 */
		void SetupVars();
		
		/*
		 * Perform some post init:
		 * 	- Set the default position values
		 *  - ...
		 */
		void PostInit();
		
		/*
		 * Updates locally requested values of the lens position
		 */
        void changeLensPosition(Variable *var);
		
		/*
		 * Req "position" variables change handler: forward to changeLensPosition
		 */
		static int changeLensPosition_hdlr(void *pt, Variable *var);
		
		/*
		 * Perform the sending of the position to BCU47
		 */
		void sendPositionsToBcu(double posX, double posY);
		
		
		
	//----------------------- FIELDS ----------------------//
	
	private:
	
		// Maximum and minimum values for X and Y positions, in um (micron)
		double _MAX_X_UM, _MAX_Y_UM, _MIN_X_UM, _MIN_Y_UM;
		
		// Maximum value for the lens voltage in BCU units (checked for safety!)
		uint32 _MIN_V_BCU_U, _MAX_V_BCU_U;
		
		// Conversion um to V factors
		double _MICRON_PER_VOLT_X, _MICRON_PER_VOLT_Y;

      	// Conversion from Volts to BCU units
      	double _BCUU_PER_VOLT_X, _BCUU_PER_VOLT_Y;
	
		// RTDB variables for requested and current positions X and Y of the lens
		RTDBvar _posX_req, _posY_req, _posX_cur, _posY_cur;
		
		CommandSender* _bcuCommandSender;
		
		// BCU parameters to set the camera lens
		int    _BCU_ID;	
		uint32 _BCU_DSP;
		uint32 _BCU_ADDRESS;
		int    _BCU_TIMEOUT_MS;	
		
		// BCU parameters to enable te outputs
		uint32  _enableOutputsAddress;
		uint32 _enableOutputsValue;
                int    _invertXY;
		
		// Structure defining the data to send to BCU47 for each position
		struct LensPosition
		{
			uint32 dummy1;
			uint16 pos;
			uint16 dummy2;
		};

};

#endif
