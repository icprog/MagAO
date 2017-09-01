
//
// Class for a single power-controlling item.
//
// The class knows how to switch on and off the item power, owns
// a status variable and a request variable in the MsgD-RTBD and knows 
// how to answer to power switch requests.
//@

#include <stdio.h>
#include <string.h>
#include <memory.h>


extern "C" {
#include "aotypes.h"
#include "commlib.h"
#include "base/errlib.h"
#include "hwlib/netseriallib.h"
}

#include "BcuLib/bcucommand.h"
#include "stdconfig.h"

#include "AbstractPowerItem.h"


//@AbstractPowerItem (default constructor)
//
//@

AbstractPowerItem::AbstractPowerItem( AOApp &app, Config_File &cfg)
{
   try {
	   CreateVars( app, app.MyFullName(), (std::string) cfg["code"], (std::string) cfg["name"]);
      item_name = (std::string) cfg["name"];
   } catch (AOException &e) {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error in AbstractPowerItem constructor");
   }
}

//@~AbstractPowerItem
//
// Standard deconstructor
//@

AbstractPowerItem::~AbstractPowerItem()
{
	DeleteVars();
}

void AbstractPowerItem::CreateVars( AOApp &app, std::string prefix, std::string item_code, std::string item_name)
{
   try {
      var_status = RTDBvar( prefix, item_code, CUR_VAR, INT_VARIABLE, 1);
      var_request = RTDBvar( prefix, item_code, REQ_VAR, INT_VARIABLE, 1);
      std::string varname = item_code + ".NAME";
      var_name = RTDBvar( prefix, varname, NO_DIR, CHAR_VARIABLE, varname.size());
      var_name.Set(item_name);

      app.Notify(var_request);
   } catch (AOVarException &e) {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }
}

void AbstractPowerItem::DeleteVars()
{
	return;
}

void AbstractPowerItem::HandleRequest( Variable *TheVar)
{
	if (var_request.MatchAndSet(TheVar))
		{
		int value;
		var_request.Get(&value);

      try {
		   if (value)
	  	   	SwitchOn();
		   else
			   SwitchOff();
      } catch (AOException &e) {
         Logger::get()->log( Logger::LOG_LEV_ERROR, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
      }
		}
}

void AbstractPowerItem::forceStatus( int state)
{
   var_status.Set(state, 0, FORCE_SEND);
}

// Try to re-set the status after a communication error

void AbstractPowerItem::Retry()
{
   int status;

   var_request.Get(&status);

   if (status)
      SwitchOn();
   else
      SwitchOff();
}

		


