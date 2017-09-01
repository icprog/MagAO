//@File: PowerBoardItem.cpp
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
#include <stdlib.h>  // itoa()


extern "C" {
#include "aotypes.h"
#include "commlib.h"
#include "base/errlib.h"
}

#include "stdconfig.h"

#include "PowerBoardItem.h"
#include "AbstractRelayCtrl.h"

//@PowerBoardItem (default constructor)
//
//@

PowerBoardItem::PowerBoardItem( AOApp *app, Config_File &cfg) : AbstractPowerItem( *app, cfg)
{
   this->app = app;
   clientname = app->MyFullName();

   try   
      {
      cmd_on     = (std::string) cfg["cmd_on"];
      cmd_off    = (std::string) cfg["cmd_off"];

      cmd_reply_on  = (std::string) cfg["cmd_reply_on"];
      cmd_reply_off = (std::string) cfg["cmd_reply_off"];

      cmd_on = ProcessEscapes(cmd_on);
      cmd_off = ProcessEscapes(cmd_off);

      }
   catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s (%s:%d)", e.what().c_str(), __FILE__, __LINE__);
   }
}

std::string PowerBoardItem::ProcessEscapes( std::string str)
{ 
   for (unsigned int i=0; i<str.size(); i++)
      if (str[i] == '\\')
            {
            str[i] = atoi( str.substr(i+1,2).c_str());
            str = str.substr(0,i) + str[i] + str.substr(i+3);
            }

   return str;
}


//@~PowerBoardItem
//
// Standard deconstructor
//@

PowerBoardItem::~PowerBoardItem()
{
}

void PowerBoardItem::SwitchOn()
{
   Logger::get()->log( Logger::LOG_LEV_INFO, "Switching ON %s", item_name.c_str());
   ((AbstractRelayCtrl *)app)->SendCommandWaitAnswer( cmd_on, cmd_reply_on);
   var_status.Set(1);
   ((AbstractRelayCtrl *)app)->_lastCmdOk = true;
}

void PowerBoardItem::SwitchOff()
{
   Logger::get()->log( Logger::LOG_LEV_INFO, "Switching OFF %s", item_name.c_str());
   ((AbstractRelayCtrl *)app)->SendCommandWaitAnswer( cmd_off, cmd_reply_off);
   var_status.Set(0);
   ((AbstractRelayCtrl *)app)->_lastCmdOk = true;
}



