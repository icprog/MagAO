//+File: PicCtrl.cpp
//
// Control program for the relay and temperature board
//-

#include "PicCtrl.h"
#include "Logger.h"
#include "AOStates.h"

#ifdef _WIN32
#include "winsock2.h"
typedef unsigned int socklen_t;
#undef NO_ERROR
#endif

extern "C" {
#include <stdio.h>
#include <string.h>		// strncmp()
#include <stdlib.h>		// atoi(), atof()
#include <stdarg.h>
#include <time.h>		// time(), localtime()
#include <unistd.h>		// sleep()
#include <pthread.h>

#include "hwlib/netseriallib.h"


// General Supervisor libs
#include "base/msglib.h"
}

#include "RTDBvar.h"
#include "stdconfig.h"
#include "AOStates.h"

// Program identification

int VersMajor = 1;
int VersMinor = 0;
const char *Date = "Sep 2005";

//+Entry  help
//
// help prints an usage message
//

void help()
{
        printf("\nPicCtrl - Vers. %d.%d     A. Puglisi, %s\n\n", VersMajor, VersMinor, Date);
        printf("Usage: PicCtrl [-v] [-h] [-i] [-f config file] [<server>]\n\n");
        printf("       -f       configuration file (defaults to \"config\")\n");
        printf("       -v       verbose\n");
        printf("       -i       interactive mode\n");
        printf("       <server> Server numeric address (default: \"127.0.0.1\")\n");
        printf("       -h       prints this message\n\n");
}

PicCtrl::PicCtrl( int argc, char **argv) throw (AOException): AbstractRelayCtrl( argc, argv)
{
   Create();
}


void PicCtrl::PostInit() throw (AOException)
{
   AbstractRelayCtrl::PostInit();
   var_lamp_cur.Set(_lampIntCur);
}

void PicCtrl::Create() throw (AOException)
{
    try {
        t_num         = ConfigDictionary()["temps.num_items"];
        h_num         = ConfigDictionary()["humidity.num_items"];
        d_num         = ConfigDictionary()["dews.num_items"];
        _lampUpCmd    = (std::string) ConfigDictionary()["lamp.upCmd"];
        _lampDownCmd  = (std::string) ConfigDictionary()["lamp.downCmd"];
        _lampUpAnswer    = (std::string) ConfigDictionary()["lamp.upAnswer"];
        _lampDownAnswer  = (std::string) ConfigDictionary()["lamp.downAnswer"];
        _readSensirion = (std::string) ConfigDictionary()["readSensirion"];
    } catch (Config_File_Exception &e) {
       _logger->log( Logger::LOG_LEV_FATAL, e.what());
       throw AOException("Fatal: missing configuration data");
    }

    _lampIntCur=0;

}

// Called at regular intervals from DoFSM()

void PicCtrl::updateLoop(void)
{

   int prev_cur = _lampIntCur;

   if (_lampIntCur != _lampIntReq)
      _logger->log( Logger::LOG_LEV_DEBUG, "Current lamp: %d - request: %d", _lampIntCur, _lampIntReq);

   // Give up to 100 updates at a time to have a fast lamp reaction
   for (int i=0; i< 10; i++)
      {
      if (_lampIntReq > _lampIntCur)
         {
         SendCommandWaitAnswer( _lampUpCmd, _lampUpAnswer, 3000);
         _lampIntCur++;
         }
      if (_lampIntReq < _lampIntCur)
         {
         SendCommandWaitAnswer( _lampDownCmd, _lampDownAnswer, 3000);
         _lampIntCur--;
         }

      // Continuous update
      if (prev_cur != _lampIntCur)
         {
         var_lamp_cur.Set(_lampIntCur);
         prev_cur = _lampIntCur;
         }
      }

   if (_readSensirion == "on")
      updateTemps();
   else
      TestLink();

}

void PicCtrl::updateTemps(void)
{
   if (_readSensirion != "on")
      return;

   static int counter =0;
   double value;

   if (counter <t_num)
      {
      value = AskValue( "temps", counter+1);
      _logger->log( Logger::LOG_LEV_DEBUG, "Temp %d - %f", counter, value);
      var_temps.Set( value, counter);
      }
   else if (counter < (t_num+h_num))
      {
      int n = counter - t_num;
      value = AskValue( "humidity", n+1);
      _logger->log( Logger::LOG_LEV_DEBUG, "Hum %d - %f", n, value);
      var_humidity.Set( value, n);
      }
   else
      {
      int n = counter - t_num - h_num;
      value = AskValue( "dews", n+1);
      _logger->log( Logger::LOG_LEV_DEBUG, "Dew %d - %f", n, value);
      var_dewpoints.Set( value, n);
      }

   counter++;
   counter %= (t_num + h_num + d_num);
}

//+Function
//
// TestLink         test serial communication with CPE board

int PicCtrl::TestLink(void)
{

   // Can't test without Sensirion 
   if (_readSensirion != "on")
      return NO_ERROR;

   try {
      AskValue("temps", 1);
      _logger->log( Logger::LOG_LEV_TRACE, "TestLink() successful");
   } catch (AOException &e) {
      _logger->log( Logger::LOG_LEV_TRACE, "TestLink() failed");
   }

   if (_lastCmdOk)
      return NO_ERROR;
   else {
      _lampIntCur =0;
      var_lamp_cur.Set(_lampIntCur);
      return NETWORK_ERROR;
   }
}



float PicCtrl::AskValue( std::string category, int num)
{
   char buf[64];

   _logger->log( Logger::LOG_LEV_TRACE, "AskValue: %s %d", category.c_str(), num);

   sprintf( buf, "%s.readCmd%d", category.c_str(), num);
   std::string cmd = (std::string)ConfigDictionary()[buf];

   sprintf( buf, "%s.prefix%d", category.c_str(), num);
   std::string answ = (std::string)ConfigDictionary()[buf];

   std::string a = SendCommandWaitAnswer( cmd, answ, 3000, 35);
   return atof( a.substr(answ.size()).c_str());
}



// +Entry
//
// SetupVars      create the MsgD-RTDB variables for this CCD
//
// Creates all the variables needed for CCD control.
// Register for notification for all "global" variables

void PicCtrl::SetupVars()
{
   try {

	var_temps = RTDBvar( MyFullName(), "TEMPS", NO_DIR, REAL_VARIABLE, t_num);
	var_humidity = RTDBvar( MyFullName(), "HUM", NO_DIR, REAL_VARIABLE, h_num);
	var_dewpoints = RTDBvar( MyFullName(), "DEW", NO_DIR, REAL_VARIABLE, d_num);

   var_lamp_cur = RTDBvar( MyFullName(), "LampIntensity", CUR_VAR);
   var_lamp_req = RTDBvar( MyFullName(), "LampIntensity", REQ_VAR);

   Notify( var_lamp_req, LampReqChanged);

   } catch (AOVarException &e) {
     _logger->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
     throw AOException("Error creating RTDB variables");
   }

}

int PicCtrl::LampReqChanged( void *pt, Variable *var)
{
   PicCtrl *ctrl = (PicCtrl *)pt;

   int req = var->Value.Lv[0];

   // It's a single int, no need for locking...
   ctrl->_lampIntReq = req;

   _logger->log( Logger::LOG_LEV_INFO, "Lamp request: %d\n", req);

   return NO_ERROR;
}

int debug;
int main( int argc, char **argv) {

   debug = 0;


   SetVersion(VersMajor,VersMinor);


   try {
      PicCtrl *c;

      c = new PicCtrl( argc, argv);

      c->Exec();

      delete c;

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}

