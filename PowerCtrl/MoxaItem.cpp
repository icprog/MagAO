//@File: MoxaItem.cpp
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

#include "MoxaItem.h"
#include "MoxaCtrl.h"

#include "mxio.h"

//@MoxaItem (default constructor)
//
//@

MoxaItem::MoxaItem( AOApp &app, Config_File &cfg) : AbstractPowerItem( app, cfg)
{
   this->app = &app;
   this->checkTT = false;
   clientname = app.MyFullName();
   int check;

   try   
      {
      num = cfg["bitnum"];
      }
   catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s (%s:%d)", e.what().c_str(), __FILE__, __LINE__);
   }
   try   
      {
      check = cfg["checkTT"];
      this->checkTT = (check)?true:false;
      }
   catch (AOException &e) {
   }
}

//@~MoxaItem
//
//@

MoxaItem::~MoxaItem()
{
}

void MoxaItem::SwitchOn()
{

   if (checkTT) {
      if (((MoxaCtrl *)app)->ttAmp() >0) {
         Logger::get()->log( Logger::LOG_LEV_ERROR, "Cannot switch %s on: tip/tilt modulation amplitude is not zero", item_name.c_str());
         return;
      }
   }
      

   Logger::get()->log( Logger::LOG_LEV_INFO, "Switching ON %s", item_name.c_str());
   int ret = E1K_DO_Writes( ((MoxaCtrl *)app)->handle(),
                            num, 1, 1);
   if (ret == MXIO_OK)
       var_status.Set(1);
}

void MoxaItem::SwitchOff()
{
   Logger::get()->log( Logger::LOG_LEV_INFO, "Switching OFF %s", item_name.c_str());
   int ret = E1K_DO_Writes( ((MoxaCtrl *)app)->handle(),
                            num, 1, 0);
   if (ret == MXIO_OK)
       var_status.Set(0);
}


int MoxaItem::Status()
{
   DWORD value;

   int ret = E1K_DO_Reads( ((MoxaCtrl *)app)->handle(),
                            num, 1, &value);

   return (value != 0) ? 1: 0;
}



