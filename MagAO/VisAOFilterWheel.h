/************************************************************
*    VisAOFilterWheel.h
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Motor control for the MCBL3006S based filter wheels in the
* VisAO system.  Essentially re-uses the MCBL2805 based
* FilterWheel code of the adopt base, with some tweaks for working
* in the VisAO architecture.
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOFilterWheel.h
  * \author Jared R. Males
  * \brief Motor control for the MCBL3006S based filter wheels on VisAO.
  *  
*/

#ifndef __VisAOFilterWheel_h__
#define __VisAOFilterWheel_h__

#include "../SimpleMotorCtrl/FilterWheel.h"

namespace VisAO
{
   
/// Control of the MCBL3006S based filter wheels.
/** Re-uses almost all of FilterWheel,  but overrides GetSwitchStatus
  * and provides a public accessor for var_pos_req so we can Notify on it.
  * Also adds position req RTDBvars for LOCAL and SCRIPT control modes.
  * 
*/ 
class VisAOFilterWheel: public FilterWheel
{
public:
   /// Constructor called by \ref VisAOSimpleMotorCtrl.
   VisAOFilterWheel( AOApp *app, Config_File &cfg);

   int SetupVars();
   
   /// Get the limit switch status.
   /** Right now this doesn't do anything, and isn't really  used for anything important.
    * We override it, however, to avoid the "GAST" command which isn't supported by 3006.
    */
   virtual int GetSwitchStatus();
   
   int IsHoming(){return homing;}
   
   /// Provides access to the protected RTDB variable var_pos_req for resetting Notify.
   /** We need to intercept any notifications on this variable from RTDB to first decide
    * if control mode allows the operation.  This is done by \ref VisAOSimpleMotorCtrl.
    */
   RTDBvar * get_var_pos_req(){return &var_pos_req;}

   RTDBvar var_pos_local_req;
   RTDBvar var_pos_script_req;
};

} //namespace VisAO

#endif // __VisAOFilterWheel_h__
	
