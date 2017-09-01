/************************************************************
*    VisAOFilterWheel.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* Implementation of the VisAOFilterWheel class defined in 
* VisAOFilterWheel.h.
* 
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file VisAOFilterWheel.cpp
  * \author Jared R. Males
  * \brief Implementation of the VisAOFilterWheel class.
  * 
*/

#include "VisAOFilterWheel.h"

namespace VisAO
{

VisAOFilterWheel::VisAOFilterWheel( AOApp *app, Config_File &cfg) : FilterWheel( app, cfg)
{
   Logger::get()->log( Logger::LOG_LEV_DEBUG, "VisAOFilterwheel constructor");

   SetupVars();
}

int VisAOFilterWheel::SetupVars()
{
   

   try
   {
      std::cout << "setting up vars.\n";
      var_pos_local_req = RTDBvar( app->MyFullName(), "POS_LOCAL", REQ_VAR, REAL_VARIABLE, 1);   
   
      var_pos_script_req = RTDBvar( app->MyFullName(), "POS_SCRIPT", REQ_VAR, REAL_VARIABLE, 1);
   }
   catch (AOVarException &e)
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }

   //notifies done by VisAOFilterWheel

   return NO_ERROR;
}

int VisAOFilterWheel::GetSwitchStatus()
{
   Logger::get()->log( Logger::LOG_LEV_DEBUG, "VisAOFilterwheel GetSwitchStatus()");
   
   //Don't know what to do yet, and I don't think this is used anymore.  Just return 0.
   return 0;
}

}//namespace VisAO
