//+File: RelayCtrl.cpp
//
// Control program for the relay and temperature board
//-

#include "RelayCtrl.h"
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
//#include <unistd.h>		// sleep()
#include <pthread.h>

#include "hwlib/netseriallib.h"


// General Supervisor libs
#include "base/msglib.h"
}

#include "RTDBvar.h"
#include "stdconfig.h"
#include "AOStates.h"


// Overtemp enable
#define USE_OVERTEMP

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
        printf("\nRelayCtrl - Vers. %d.%d     A. Puglisi, %s\n\n", VersMajor, VersMinor, Date);
        printf("Usage: RelayCtrl [-v] [-h] [-i] [-f config file] [<server>]\n\n");
        printf("       -f       configuration file (defaults to \"config\")\n");
        printf("       -v       verbose\n");
        printf("       -i       interactive mode\n");
        printf("       <server> Server numeric address (default: \"127.0.0.1\")\n");
        printf("       -h       prints this message\n\n");
}

RelayCtrl::RelayCtrl( int argc, char **argv) throw (AOException): AbstractRelayCtrl( argc, argv)
{
   Create();
}

void RelayCtrl::Create() throw (AOException)
{
    try {
        t_num         = ConfigDictionary()["t_num"];

#ifdef USE_OVERTEMP
       _th1up     = (std::string) ConfigDictionary()["threshold1.up"];
       _th1down   = (std::string) ConfigDictionary()["threshold1.down"];
       _th1answer = (std::string) ConfigDictionary()["threshold1.answer"];

       _th2up     = (std::string) ConfigDictionary()["threshold2.up"];
       _th2down   = (std::string) ConfigDictionary()["threshold2.down"];
       _th2answer = (std::string) ConfigDictionary()["threshold2.answer"];

       _overtempEnable = (std::string) ConfigDictionary()["overtemp.enable"];
       _overtempEnableAnswer = (std::string) ConfigDictionary()["overtemp.enableAnswer"];
       _overtempDisable = (std::string) ConfigDictionary()["overtemp.disable"];
       _overtempDisableAnswer = (std::string) ConfigDictionary()["overtemp.disableAnswer"];
#endif

    } catch (Config_File_Exception &e) {
       _logger->log( Logger::LOG_LEV_FATAL, e.what());
       throw AOException("Fatal: missing configuration data");
    }

    _overtempEnabled = false;
}


// Called at regular intervals from DoFSM()

void RelayCtrl::updateLoop(void)
{
#ifdef USE_OVERTEMP

   double th1Req, th2Req, th1Cur, th2Cur;

   // Give up to 100 updates at a time to have a fast reaction
   for (int i=0; i< 100; i++) {
      bool modified=false;
      var_overtemp_th1_req.Get(&th1Req);
      var_overtemp_th1_cur.Get(&th1Cur);
      var_overtemp_th2_req.Get(&th2Req);
      var_overtemp_th2_cur.Get(&th2Cur);

      if (th1Req - th1Cur >= 0.5) { 
          IncTh1();
          modified = true;
          }
      if (th1Req - th1Cur <= -0.5) { 
          DecTh1();
          modified = true;
      }
      if (th2Req - th2Cur >= 0.5) { 
          IncTh2();
          modified = true;
      }
      if (th2Req - th2Cur <= -0.5) { 
          DecTh2();
          modified = true;
      }

      if (!modified)
         break;
   }

   if (_overtempEnabled) 
      EnableOvertemp();

#endif

   _lastCmdOk = false;
   updateTemps();
   if (_readTemps == t_num)
      _lastCmdOk = true;
}

// Fake testlink

int RelayCtrl::TestLink(void)
{
   updateTemps();
   if (_readTemps==t_num)
       {
       _lastCmdOk = true;
       return NO_ERROR;
       }
   else
       {
       _lastCmdOk = false;
        return COMMUNICATION_ERROR;
       }
}

void RelayCtrl::updateTemps(void)
{
    int counter;

    pthread_mutex_lock( &serial_mutex);

    SendCommand("b\n\n", 3);
    _readTemps = 0;
    counter = 0;   // Counter to avoid infinite blocking
    while ((_readTemps < t_num) && (counter<5))
       {
       EmptySerial(1, 1000, false);
       msleep(1000);
       counter++;
       }

    if (_overtempAlarm) {
      // Overtemp detected. Signal error and re-enable the loop
      var_overtemp_alarm.Set(1);
      _VarErrmsg.Set("Overtemperature alarm");
      _logger->log(Logger::LOG_LEV_WARNING, "Overtemperature alarm ACTIVE");
      }
   else {
      var_overtemp_alarm.Set(0);
      _VarErrmsg.Set("No error");
   }

    pthread_mutex_unlock( &serial_mutex);

    if ((_overtempEnabled) && (_overtempAlarm)) {
      _overtempAlarm = false;    // Will be refreshed to True, if the alarm persist, in the next call to EmptySerial
      EnableOvertemp();
    }
}


// +Entry
//
// SetupVars      create the MsgD-RTDB variables for this CCD
//
// Creates all the variables needed for CCD control.
// Register for notification for all "global" variables

void RelayCtrl::SetupVars()
{
   try {

#ifdef USE_OVERTEMP
      var_overtemp_active_cur = RTDBvar( MyFullName(), "Overtemp.active", CUR_VAR);
      var_overtemp_active_req = RTDBvar( MyFullName(), "Overtemp.active", REQ_VAR);

      var_overtemp_th1_cur = RTDBvar( MyFullName(), "Overtemp.th1", CUR_VAR, REAL_VARIABLE);
      var_overtemp_th1_req = RTDBvar( MyFullName(), "Overtemp.th1", REQ_VAR, REAL_VARIABLE);

      var_overtemp_th2_cur = RTDBvar( MyFullName(), "Overtemp.th2", CUR_VAR, REAL_VARIABLE);
      var_overtemp_th2_req = RTDBvar( MyFullName(), "Overtemp.th2", REQ_VAR, REAL_VARIABLE);

      var_overtemp_alarm = RTDBvar( MyFullName(), "Overtemp.alarm", NO_DIR, INT_VARIABLE);

      Notify( var_overtemp_th1_req, OvertempTh1Changed);
      Notify( var_overtemp_th2_req, OvertempTh2Changed);
      Notify( var_overtemp_active_req, OvertempActiveChanged);
#endif


	   var_temps = RTDBvar(MyFullName(), "TEMPS", NO_DIR, REAL_VARIABLE, t_num);
   } catch (AOVarException &e) {
     _logger->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
     throw AOException("Error creating RTDB variables");
   }

}

int RelayCtrl::OvertempTh1Changed( void *pt, Variable *var)
{
   RelayCtrl *ctrl = (RelayCtrl *)pt;
   ctrl->var_overtemp_th1_req.Set(var);
   return NO_ERROR;
}

int RelayCtrl::OvertempTh2Changed( void *pt, Variable *var)
{
   RelayCtrl *ctrl = (RelayCtrl *)pt;
   ctrl->var_overtemp_th2_req.Set(var);
   return NO_ERROR;
}

int RelayCtrl::OvertempActiveChanged( void *pt, Variable *var)
{
   RelayCtrl *ctrl = (RelayCtrl *)pt;

   int req = var->Value.Lv[0];

   if (req)
       ctrl->EnableOvertemp();
   else
       ctrl->DisableOvertemp();

   return NO_ERROR;
}


// Parses the temperature readout
// Returns the # of temperatures parsed

int RelayCtrl::parseTemps( std::string s)
{
   std::string::size_type pos;
   char buf[20], buf2[20];
   int counter = 0;

   if (s.find("limit !!! Power off") != std::string::npos) {
      _overtempAlarm = true;
      printf("Overtemp detected !!\n");
   }


   for (int temp_num=1; temp_num<t_num+1; temp_num++)
   {
      if (temp_num < 10)
         sprintf(buf, "T%d=", temp_num);
      else
         sprintf(buf, "T%c=", temp_num-10+'a');

      if ((pos = s.find(buf)) != std::string::npos)
         {
         double temp;

         temp = atof( s.substr(strlen(buf)+pos).c_str());

         sprintf(buf2, "temp%d_offset", temp_num);

         float offset = ConfigDictionary()[buf2];
         temp += offset;

         _logger->log( Logger::LOG_LEV_DEBUG, "Temperature sensor %d: %f (with offset %5.2f)", temp_num, temp, offset);
         printf("Temperature sensor %d: %f (with offset %5.2f)\n", temp_num, temp, offset);
         var_temps.Set( temp, temp_num-1, NO_SEND);

         counter++;
         }
      }

   var_temps.Send();

   return counter;
}

// ----------- Overtemp protection ---------------

double RelayCtrl::parseOvertemp( std::string answ)
{
   int pos = answ.find(":");
   if (pos>0)
       return atof(answ.substr(pos+1).c_str());
    else
      return 0;
}

void RelayCtrl::IncTh1()
{
   std::string answ;

   answ = SendCommandWaitAnswer( _th1up, _th1answer, 3000, 60, ".C");
   var_overtemp_th1_cur.Set( parseOvertemp(answ));
}

void RelayCtrl::DecTh1()
{
   std::string answ;

   answ = SendCommandWaitAnswer( _th1down, _th1answer, 3000, 60, ".C");
   var_overtemp_th1_cur.Set( parseOvertemp(answ));
}

void RelayCtrl::IncTh2()
{
   std::string answ;

   answ = SendCommandWaitAnswer( _th2up, _th2answer, 3000, 60, ".C");
   var_overtemp_th2_cur.Set( parseOvertemp(answ));
}

void RelayCtrl::DecTh2()
{
   std::string answ;

   answ = SendCommandWaitAnswer( _th2down, _th2answer, 3000, 60, ".C");
   var_overtemp_th2_cur.Set( parseOvertemp(answ));
}

void RelayCtrl::EnableOvertemp()
{
   std::string answ;

   answ = SendCommandWaitAnswer( _overtempEnable, _overtempEnableAnswer, 3000, 60, "on");
   if (answ.find( _overtempEnableAnswer))
      var_overtemp_active_cur.Set(1);
}

void RelayCtrl::DisableOvertemp()
{
   std::string answ;

   answ = SendCommandWaitAnswer( _overtempDisable, _overtempDisableAnswer, 3000, 60, "off");
   if (answ.find( _overtempDisableAnswer))
      var_overtemp_active_cur.Set(1);
}

void RelayCtrl::StateChange( int /* oldstate */, int state)
{
    printf("State change: %d\n", state);
    if (state == STATE_OPERATING) { 
#ifdef USE_OVERTEMP

        // Set default overtemperature values
        double th1 = ConfigDictionary()["threshold1.default"];
        double th2 = ConfigDictionary()["threshold2.default"];
        string on = (std::string) ConfigDictionary()["threshold.enable"];


        // Bring the threshold temperatures to their default
        double th1Cur, th2Cur;
        for (int i=0; i< 200; i++) {
            bool modified=false;
            var_overtemp_th1_cur.Get(&th1Cur);
            var_overtemp_th2_cur.Get(&th2Cur);

            if (th1 > th1Cur) {
                IncTh1();
                modified=true;
            }
            if (th1 < th1Cur) {
                DecTh1();
               modified=true;
            }
            if (th2 > th2Cur) {
                IncTh2();
               modified=true;
            }
            if (th2 < th2Cur) {
                DecTh2();
               modified=true;
            }
            if (!modified)
               break;
        }
        var_overtemp_th1_req.Set(th1, 0, FORCE_SEND);
        var_overtemp_th2_req.Set(th2, 0, FORCE_SEND);
        var_overtemp_alarm.Set(0);
        
        if (on.compare("on") == 0)  {
           EnableOvertemp();
           _overtempEnabled=true;
        }
        else {
           DisableOvertemp();
           _overtempEnabled=false;
        }

#endif
    }
}
                                                

int debug;

int main( int argc, char **argv) {

   debug = 0;

   SetVersion(VersMajor,VersMinor);

   try {
      RelayCtrl *c;

      c = new RelayCtrl( argc, argv);

      c->Exec();

      delete c;

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}

