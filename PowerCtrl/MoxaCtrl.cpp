//+File: MoxaCtrl.cpp
//
// {\tt MoxaCtrl} : MsgD-RTDB multithreaded client for power monitoring and control of the WFS board
//

#include <vector>
using namespace std;

extern "C" {
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>		// atof()
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <pthread.h>

#include "base/thrdlib.h"
#include "hwlib/netseriallib.h"
}

#include "MoxaCtrl.h"
#include "AOStates.h"

#include "MoxaItem.h"
#include "mxio.h"

using namespace Arcetri;

// **************************************
// Client name and versioning information
// **************************************

int VersMajor = 1;
int VersMinor = 0;
const char *Date = "Aug 2005";
const char *Author = "A. Puglisi";

//+Entry help
//
//  help prints an usage message
//
// Rimossa perche' non usata (L.F.)
//static void help()
//{
//    printf("\nMoxaCtrl  - Vers. %d.%d.  %s, %s\n\n", VersMajor,VersMinor,Author,Date);
//    printf("Usage: MoxaCtrl [-v] [-s <Server Address>]\n\n");
//    printf("   -s    specify server address (defaults to localhost)\n");
//    printf("   -v    verbose mode\n");
//    printf("\n");
//}

// +Main: 
//

MoxaCtrl::MoxaCtrl( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
}


void MoxaCtrl::PostInit()
{
    int num_items = ConfigDictionary()["num_items"];
    _ip = (std::string)ConfigDictionary()["ipAddr"];
    _port = ConfigDictionary()["ipPort"];

    int i;

    for (i=0; i<num_items; i++)
        {
        char item_name[32];

        sprintf( item_name, "item%d", i);
        Config_File *cfg = ConfigDictionary().extract( item_name); 
        MoxaItem *item = new MoxaItem( *this, *cfg);
        poweritems.push_back(item);
        }

    var_ttamp_cur = RTDBvar("ttctrl.L", "AMP_RADIUS", CUR_VAR, REAL_VARIABLE, 1, false);
    Notify( var_ttamp_cur, TTAmpChanged);

}

// Main thread: continously poll the BCU. If it goes offline, mark everything as OFF since
// the BCU switches all relays open when losing power.

void MoxaCtrl::Run()
{
   static int commStatus=0;
   vector<MoxaItem *>::iterator iter;

   while (!TimeToDie())
      {
      Logger::get()->log(Logger::LOG_LEV_DEBUG, "TESTING COMMUNICATION...");
      if (TestComm() != NO_ERROR)
          {
          Logger::get()->log(Logger::LOG_LEV_ERROR, " -> COMMUNICATION DOWN :-(");
          commStatus=0;
          try {
            setCurState( STATE_NOCONNECTION);
            for (iter = poweritems.begin(); iter != poweritems.end(); iter++)
                (*iter)->forceStatus( PWRSTATUS_NOTREACHABLE);
          } catch (AOException &e) {
            Logger::get()->log( Logger::LOG_LEV_ERROR, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
          }

          }
      else
  		  {
  		  Logger::get()->log(Logger::LOG_LEV_DEBUG, " -> COMMUNICATION OK !!!");
        setCurState( STATE_READY);
        if (commStatus == 0)
            {
            try
               {
                for (iter = poweritems.begin(); iter != poweritems.end(); iter++)
                   (*iter)->forceStatus( (*iter)->Status());
               }
            catch (AOException &e) {
               Logger::get()->log( Logger::LOG_LEV_ERROR, "Error in Retry(): %s File %s line %d.", e.what().c_str(), __FILE__, __LINE__);
               }
         }
        commStatus=1;
  		  }
      msleep(5000);
      }
}

//@Function: TestComm
//
// Tests the communication with the BCU hardware. Since this
// process is not intended to directly communicate with
// the BCU, a successful test requires also the appropriate
// BCU controller to be running.
//
// Returns NO_ERROR if ok or an error code.
//@

int MoxaCtrl::TestComm(void)
{
    BYTE bytStatus;
    char Password[8] = {'\0'};


    int ret = MXEIO_CheckConnection( _handle, 2000, &bytStatus);
    if (bytStatus == CHECK_CONNECTION_OK)
      return NO_ERROR;
    else {
       // Try to reconnect

      _logger->log( Logger::LOG_LEV_INFO, "Connecting to %s, %d", (char *) _ip.c_str(), _port);

      ret = MXEIO_Init();
      if (ret == MXIO_OK) {


         ret = MXEIO_E1K_Connect( (char *) _ip.c_str(),
                                           _port,
                                            2000,            // timeout
                                             &_handle,
                                            Password);          // password
         if (ret == MXIO_OK) {
          _logger->log( Logger::LOG_LEV_INFO, "Network reconfigured OK");
           return NO_ERROR;
         }
      }
    }

    return COMMUNICATION_ERROR;
}



int MoxaCtrl::VariableHandler( Variable *var)
{
   unsigned int i;

   int status = getCurState();
   if (status != STATE_READY)
       {
       _logger->log( Logger::LOG_LEV_ERROR, "Command refused: no network connection");
       throw AOException("Can't send command: state is not READY", NETWORK_ERROR, __FILE__, __LINE__);
       }

   for (i=0; i < poweritems.size(); i++)
      poweritems[i]->HandleRequest(var);

   return NO_ERROR;
}

int MoxaCtrl::TTAmpChanged(void *pt, Variable *var) {

        MoxaCtrl* moxa = (MoxaCtrl*)pt;
        double ttAmp = var->Value.Dv[0];

        _logger->log(Logger::LOG_LEV_INFO, "TTamp is %g", ttAmp);
        moxa->var_ttamp_cur.MatchAndSet(var);
        return NO_ERROR;
}

double MoxaCtrl::ttAmp() {

    double ttAmp;
    var_ttamp_cur.Get(&ttAmp);
    return ttAmp;
}

int main( int argc, char **argv) {

   SetVersion(VersMajor,VersMinor);

   try {
      MoxaCtrl *c;

      c = new MoxaCtrl( argc, argv);

      c->Exec();
      
      delete c;
   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}

