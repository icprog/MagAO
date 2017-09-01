//+File: RelayCtrlW2.cpp
//
// Control program for the relay and temperature board
//-

#include "RelayCtrlW2.h"
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

// Program identification

int VersMajor = 1;
int VersMinor = 0;
const char *Date = "Oct 2008";

//+Entry  help
//
// help prints an usage message
//

void help()
{
        printf("\nRelayCtrlW2 - Vers. %d.%d     A. Puglisi, %s\n\n", VersMajor, VersMinor, Date);
        printf("Usage: RelayCtrlW2 [-v] [-h] [-i] [-f config file] [<server>]\n\n");
        printf("       -f       configuration file (defaults to \"config\")\n");
        printf("       -v       verbose\n");
        printf("       -i       interactive mode\n");
        printf("       <server> Server numeric address (default: \"127.0.0.1\")\n");
        printf("       -h       prints this message\n\n");
}

RelayCtrlW2::RelayCtrlW2( int argc, char **argv) throw (AOException): AbstractRelayCtrl( argc, argv)
{
   Create();
}

void RelayCtrlW2::Create() throw (AOException)
{
    try {
        t_num         = ConfigDictionary()["t_num"];

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

    } catch (Config_File_Exception &e) {
       _logger->log( Logger::LOG_LEV_FATAL, e.what());
       throw AOException("Fatal: missing configuration data");
    }

    _boardTempsLogger = Logger::get("BoardTemperatures", Logger::LOG_LEV_INFO, "TELEMETRY");
    _boxesTempsLogger = Logger::get("BoxesTemperatures", Logger::LOG_LEV_INFO, "TELEMETRY");

    _boardTemps.resize(4);
    _boxesTemps.resize(6);
}


// Called at regular intervals from DoFSM()

void RelayCtrlW2::updateLoop(void)
{
   double th1Req, th2Req, th1Cur, th2Cur;

   // Give up to 100 updates at a time to have a fast reaction
   for (int i=0; i< 100; i++)
      {
      var_overtemp_th1_req.Get(&th1Req);
      var_overtemp_th1_cur.Get(&th1Cur);
      var_overtemp_th2_req.Get(&th2Req);
      var_overtemp_th2_cur.Get(&th2Cur);

      if (th1Req > th1Cur)
        IncTh1();
      if (th1Req < th1Cur)
        DecTh1();
      if (th2Req > th2Cur)
        IncTh2();
      if (th2Req < th2Cur)
        DecTh2();
      }

   _lastCmdOk = false;
   updateTemps();
   if (_readTemps == t_num)
       _lastCmdOk = true;
}

// TestLink using temperature reading

int RelayCtrlW2::TestLink(void)
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

void RelayCtrlW2::updateTemps(void)
{
    int counter;

    pthread_mutex_lock( &serial_mutex);

    SendCommand("7", 1);
    _readTemps = 0;
    counter = 0;   // Counter to avoid infinite blocking
    while ((_readTemps < t_num) && (counter<5))
       {
       EmptySerial(1, 1000, false);
       msleep(1000);
       counter++;
       }

    _boardTempsLogger->log_telemetry( Logger::LOG_LEV_INFO, _boardTemps);
    _boxesTempsLogger->log_telemetry( Logger::LOG_LEV_INFO, _boxesTemps);

    pthread_mutex_unlock( &serial_mutex);
}


// +Entry
//
// SetupVars      create the MsgD-RTDB variables for this CCD
//
// Creates all the variables needed for CCD control.
// Register for notification for all "global" variables

void RelayCtrlW2::SetupVars()
{
   try {

   var_overtemp_active_cur = RTDBvar( MyFullName(), "Overtemp.active", CUR_VAR);
   var_overtemp_active_req = RTDBvar( MyFullName(), "Overtemp.active", REQ_VAR);

   var_overtemp_th1_cur = RTDBvar( MyFullName(), "Overtemp.th1", CUR_VAR, REAL_VARIABLE);
   var_overtemp_th1_req = RTDBvar( MyFullName(), "Overtemp.th1", REQ_VAR, REAL_VARIABLE);

   var_overtemp_th2_cur = RTDBvar( MyFullName(), "Overtemp.th2", CUR_VAR, REAL_VARIABLE);
   var_overtemp_th2_req = RTDBvar( MyFullName(), "Overtemp.th2", REQ_VAR, REAL_VARIABLE);

   Notify( var_overtemp_th1_req, OvertempTh1Changed);
   Notify( var_overtemp_th2_req, OvertempTh2Changed);
   Notify( var_overtemp_active_req, OvertempActiveChanged);


	var_temps = RTDBvar(MyFullName(), "TEMPS", NO_DIR, REAL_VARIABLE, t_num);
   } catch (AOVarException &e) {
     _logger->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
     throw AOException("Error creating RTDB variables");
   }

}

int RelayCtrlW2::OvertempTh1Changed( void *pt, Variable *var)
{
   RelayCtrlW2 *ctrl = (RelayCtrlW2 *)pt;
   ctrl->var_overtemp_th1_req.Set(var);
   return NO_ERROR;
}

int RelayCtrlW2::OvertempTh2Changed( void *pt, Variable *var)
{
   RelayCtrlW2 *ctrl = (RelayCtrlW2 *)pt;
   ctrl->var_overtemp_th2_req.Set(var);
   return NO_ERROR;
}

int RelayCtrlW2::OvertempActiveChanged( void *pt, Variable *var)
{
   RelayCtrlW2 *ctrl = (RelayCtrlW2 *)pt;

   int req = var->Value.Lv[0];

   if (req)
    ctrl->EnableOvertemp();
   else
    ctrl->DisableOvertemp();

   return NO_ERROR;
}


// Parses the temperature readout
// Returns the # of temperatures parsed

int RelayCtrlW2::parseTemps( std::string s)
{
   std::string::size_type pos;
   char buf[20], buf2[20];
   int counter = 0;

   std::string tags[] = {
    "Temp_board1",
    "Temp_board2",
    "Temp_board3",
    "Temp_CCD39",
    "Temp_water",
    "Temp_lj39",
    "Temp_bcu39",
    "Temp_lj47",
    "Temp_bcu47",
    "Temp_copley" };

   int max_tnum = sizeof(tags) / sizeof(std::string);
   if (t_num > max_tnum)
    t_num = max_tnum;

   for (int temp_num=0; temp_num<t_num; temp_num++)
   {
      sprintf(buf, "%s:", tags[temp_num].c_str());

      if ((pos = s.find(buf)) != std::string::npos)
         {
         double temp;

         temp = atof( s.substr(strlen(buf)+pos).c_str());

         sprintf(buf2, "temp%d_offset", temp_num+1);

         double offset = ConfigDictionary()[buf2];
         temp += offset;

         _logger->log( Logger::LOG_LEV_DEBUG, "Temperature sensor %d: %f (with offset %5.2f)", temp_num, temp, offset);
         printf("Temperature sensor %d: %f (with offset %5.2f)\n", temp_num, temp, offset);
         var_temps.Set( temp, temp_num, NO_SEND);

         if (temp_num <4)
             _boardTemps[temp_num] = temp;
         else
             _boxesTemps[temp_num-4] = temp;

         counter++;
         }
      }

   var_temps.Send();


   return counter;
}

// ----------------- Overtemp protection ---------------

double RelayCtrlW2::parseOvertemp( std::string answ)
{
   int pos = answ.find(":");
   if (pos>0)
       return atof(answ.substr(pos+1).c_str());
    else
        return 0;
}

void RelayCtrlW2::IncTh1()
{
   std::string answ;

   answ = SendCommandWaitAnswer( _th1up, _th1answer, 3000, 60, ".C");
   var_overtemp_th1_cur.Set( parseOvertemp(answ));
}

void RelayCtrlW2::DecTh1()
{
   std::string answ;

   answ = SendCommandWaitAnswer( _th1down, _th1answer, 3000, 60, ".C");
   var_overtemp_th1_cur.Set( parseOvertemp(answ));
}

void RelayCtrlW2::IncTh2()
{
   std::string answ;

   answ = SendCommandWaitAnswer( _th2up, _th2answer, 3000, 60, ".C");
   var_overtemp_th2_cur.Set( parseOvertemp(answ));
}

void RelayCtrlW2::DecTh2()
{
   std::string answ;

   answ = SendCommandWaitAnswer( _th2down, _th2answer, 3000, 60, ".C");
   var_overtemp_th2_cur.Set( parseOvertemp(answ));
}

void RelayCtrlW2::EnableOvertemp()
{
   std::string answ;

   answ = SendCommandWaitAnswer( _overtempEnable, _overtempEnableAnswer, 3000, 60, "on");
   if (answ.find( _overtempEnableAnswer))
      var_overtemp_active_cur.Set(1);
}

void RelayCtrlW2::DisableOvertemp()
{
   std::string answ;

   answ = SendCommandWaitAnswer( _overtempDisable, _overtempDisableAnswer, 3000, 60, "off");
   if (answ.find( _overtempDisableAnswer))
      var_overtemp_active_cur.Set(1);
}

void RelayCtrlW2::DetectTh1()
{
   double value;
   IncTh1();
   DecTh1();

   var_overtemp_th1_cur.Get(&value);
   var_overtemp_th1_req.Set(value);
}

void RelayCtrlW2::DetectTh2()
{
   double value;

   IncTh2();
   DecTh2();

   var_overtemp_th2_cur.Get(&value);
   var_overtemp_th2_req.Set(value);
}


void RelayCtrlW2::StateChange( int /* oldstate */, int state)
{
    printf("State change: %d\n", state);
    if (state == STATE_OPERATING)
        {
        DetectTh1();
        DetectTh2();
        }
}


// Reimplemented to turn off cameralens after a while

int watchdogTime = 600;

void RelayCtrlW2::Run()
{
   static int cameraLensStatus=0;
   static int cameraLensStart=0;

   while (TimeToDie() == false)
   {
      try {
         DoFSM();

         EmptySerial();

        // Cameralens watchdog
        if (0)  // Watchdog disabled!!
        for (unsigned int i=0; i<poweritems.size(); i++)
            {
            if (poweritems[i]->name() == "Cameralens")
                {
                int stat = poweritems[i]->getStatus();
                if ((cameraLensStatus == 0) && (stat == 1))
                    cameraLensStart = time(NULL);

                if (stat==1)
                    {
                    int now = time(NULL);
                    if (now - cameraLensStart > watchdogTime)
                        {
                        _logger->log( Logger::LOG_LEV_WARNING, "Cameralens watchdog: turning OFF after %d seconds", watchdogTime);
                        poweritems[i]->SwitchOff();
                        }
                    }
                cameraLensStatus = stat;
                }
            }

      } catch (AOException &e) {
         _logger->log( Logger::LOG_LEV_DEBUG, "Run(): caught exception %s", e.what().c_str());
         // When the exception was thrown, the mutex may be held!
         pthread_mutex_unlock( &serial_mutex);
      }
    }
}

int debug;
int main( int argc, char **argv) {

   debug = 0;

   SetVersion(VersMajor,VersMinor);

   try {
      RelayCtrlW2 *c;

      c = new RelayCtrlW2( argc, argv);

      c->Exec();

      delete c;

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}

