//+File: PicCtrlW2.cpp
//
// Control program for the relay and temperature board
//-

#include "PicCtrlW2.h"
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
#include <math.h>       // log()

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
        printf("\nPicCtrlW2 - Vers. %d.%d     A. Puglisi, %s\n\n", VersMajor, VersMinor, Date);
        printf("Usage: PicCtrlW2 [-v] [-h] [-i] [-f config file] [<server>]\n\n");
        printf("       -f       configuration file (defaults to \"config\")\n");
        printf("       -v       verbose\n");
        printf("       -i       interactive mode\n");
        printf("       <server> Server numeric address (default: \"127.0.0.1\")\n");
        printf("       -h       prints this message\n\n");
}

PicCtrlW2::PicCtrlW2( int argc, char **argv) throw (AOException): AbstractRelayCtrl( argc, argv)
{
   Create();
}


void PicCtrlW2::PostInit() throw (AOException)
{
   AbstractRelayCtrl::PostInit();
   var_lamp_cur.Set(_lampIntCur);
}

void PicCtrlW2::Create() throw (AOException)
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
    _lampIntReq=0;

    _temperatureLogger = Logger::get("SensirionTemperature", Logger::LOG_LEV_INFO, "TELEMETRY");
    _humidityLogger = Logger::get("SensirionHumidity", Logger::LOG_LEV_INFO, "TELEMETRY");
    _dewPointLogger = Logger::get("SensirionDewpoint", Logger::LOG_LEV_INFO, "TELEMETRY");

    _temperatureVector.resize(2);
    _humidityVector.resize(2);
    _dewPointVector.resize(2);

}

// Called at regular intervals from DoFSM()

void PicCtrlW2::updateLoop(void)
{

   int prev_cur = _lampIntCur;

   if (_lampIntCur != _lampIntReq)
      _logger->log( Logger::LOG_LEV_INFO, "Current lamp: %d - request: %d", _lampIntCur, _lampIntReq);

   // Give up to 100 updates at a time to have a fast lamp reaction
   for (int i=0; i< 30; i++)
      {
      if (_lampIntReq > _lampIntCur)
         {
         SendCommandWaitAnswer( _lampUpCmd, _lampUpAnswer, 3000, 60, "V");
         _lampIntCur++;
         }
      if (_lampIntReq < _lampIntCur)
         {
         SendCommandWaitAnswer( _lampDownCmd, _lampDownAnswer, 3000, 60, "V");
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

int PicCtrlW2::DetectLamp() {

   string answ1 = SendCommandWaitAnswer( _lampUpCmd, _lampUpAnswer, 3000, 60, "V");
   string answ2 = SendCommandWaitAnswer( _lampDownCmd, _lampDownAnswer, 3000, 60, "V");

   // Parse current voltage setting

   string str = "DacDato=";
   int volts = atoi( answ2.substr( answ2.find(str) + str.size()).c_str());
   _logger->log(Logger::LOG_LEV_INFO, "Detected lamp setting: %d", volts);
   
   return volts;

}

// VIRTUAL state change function - detect the current lamp status
// and then call the base class function

void PicCtrlW2::StateChange( int oldstate, int state) {

   if (state == STATE_OPERATING) {
      _lampIntCur = DetectLamp();
      _lampIntReq = _lampIntCur;
      var_lamp_cur.Set(_lampIntCur);
   }

   AbstractRelayCtrl::StateChange( oldstate, state);
}

void PicCtrlW2::updateTemps(void)
{
   if (_readSensirion != "on")
      return;

   static int counter =0;
   double value;

    EmptySerial();

   if (counter <t_num)
      {
      value = AskValue( "temps", counter+1, "Degree");
      _logger->log( Logger::LOG_LEV_DEBUG, "Temp %d - %f", counter, value);
      var_temps.Set( value, counter, FORCE_SEND);
      _temperatureVector[counter] = value;
      }
   else if (counter < (t_num+h_num))
      {
      int n = counter - t_num;
      value = AskValue( "humidity", n+1, "% ");
      _logger->log( Logger::LOG_LEV_DEBUG, "Hum %d - %f", n, value);
      var_humidity.Set( value, n, FORCE_SEND);
      _humidityVector[n] = value;


      // Calculate dew point
      double t,h;
      var_temps.Get( n, &t);
      var_humidity.Get(n, &h);
      double dewp = dewpoint( t,h);
      var_dewpoints.Set( dewp, n, FORCE_SEND);
      _dewPointVector[n]= dewp;
      }

   counter++;

   if (counter == t_num)
      _temperatureLogger->log_telemetry( Logger::LOG_LEV_INFO, _temperatureVector);
   if (counter == t_num+h_num) {
      _humidityLogger->log_telemetry( Logger::LOG_LEV_INFO, _humidityVector);
      _dewPointLogger->log_telemetry( Logger::LOG_LEV_INFO, _dewPointVector);
      }

   counter %= (t_num + h_num);

}


float PicCtrlW2::AskValue( std::string category, int num, std::string terminator)
{
   char buf[64];

   _logger->log( Logger::LOG_LEV_TRACE, "AskValue: %s %d", category.c_str(), num);

   sprintf( buf, "%s.readCmd%d", category.c_str(), num);
   std::string cmd = (std::string)ConfigDictionary()[buf];

   sprintf( buf, "%s.prefix%d", category.c_str(), num);
   std::string answ = (std::string)ConfigDictionary()[buf];

   std::string a = SendCommandWaitAnswer( cmd, answ, 3000, 60, terminator);
   return atof( a.substr( a.find(answ)).substr(answ.size()).c_str());
}



// +Entry
//
// SetupVars      create the MsgD-RTDB variables for this CCD
//
// Creates all the variables needed for CCD control.
// Register for notification for all "global" variables

void PicCtrlW2::SetupVars()
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

int PicCtrlW2::LampReqChanged( void *pt, Variable *var)
{
   PicCtrlW2 *ctrl = (PicCtrlW2 *)pt;

   int req = var->Value.Lv[0];

   // It's a single int, no need for locking...
   ctrl->_lampIntReq = req;

   _logger->log( Logger::LOG_LEV_INFO, "Lamp request: %d\n", req);

   return NO_ERROR;
}

double PicCtrlW2::dewpoint( double t, double h)
{
    double a = 17.27;
    double b = 237.7;

    double alpha = ((a*t)/(b+t)) + log(h/100.0);

    return (b*alpha)/(a - alpha);
}



// ------------------------------- Main ---------------------
int debug;
int main( int argc, char **argv) {

   debug = 0;


   SetVersion(VersMajor,VersMinor);


   try {
      PicCtrlW2 *c;

      c = new PicCtrlW2( argc, argv);

      c->Exec();

      delete c;

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}

int PicCtrlW2::TestLink(void)
{
   try {
      SendCommandWaitAnswer("z", "Retry", 5000, 21, "!!!!"); // A command not recognized that triggers an error reply
      _logger->log( Logger::LOG_LEV_TRACE, "TestLink() successful");
   } catch (AOException &e) {
      _logger->log( Logger::LOG_LEV_TRACE, "TestLink() failed");
    }

   if (_lastCmdOk)
      return NO_ERROR;
   else
      return NETWORK_ERROR;
}

