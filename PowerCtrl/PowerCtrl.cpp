//+File: PowerCtrl.cpp
//
// {\tt PowerCtrl} : MsgD-RTDB multithreaded client for power monitoring and control of the WFS board
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

#include "BcuLib/BcuCommon.h"
#include "BcuLib/bcucommand.h"
#include "PowerCtrl.h"
#include "AOStates.h"

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
//    printf("\nPowerCtrl  - Vers. %d.%d.  %s, %s\n\n", VersMajor,VersMinor,Author,Date);
//    printf("Usage: PowerCtrl [-v] [-s <Server Address>]\n\n");
//    printf("   -s    specify server address (defaults to localhost)\n");
//    printf("   -v    verbose mode\n");
//    printf("\n");
//}

// +Main: 
//

PowerCtrl::PowerCtrl( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
}


void PowerCtrl::PostInit()
{
    int num_items = ConfigDictionary()["num_items"];
    int i;

    for (i=0; i<num_items; i++)
        {
        char item_name[32];

        sprintf( item_name, "item%d", i);
        Config_File *cfg = ConfigDictionary().extract( item_name); 
        Bcu39Item *item = new Bcu39Item( *this, *cfg);
        poweritems.push_back(item);
        }
}

// Main thread: continously poll the BCU. If it goes offline, mark everything as OFF since
// the BCU switches all relays open when losing power.

void PowerCtrl::Run()
{
   static int commStatus=0;
   vector<Bcu39Item *>::iterator iter;

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
                  (*iter)->Retry();
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

int PowerCtrl::TestComm(void)
{
   static int seqnum = 0;
   uint32 buffer;
   try {
      return thSendBCUcommand( (char *) MyFullName().c_str(), 0, 0xFF, 0xFF, Bcu::OpCodes::MGP_OP_RDSEQ_SDRAM,
                           0, (unsigned char *)&buffer, sizeof(uint32), 1000, seqnum++, 0);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error in TestComm(): %s.", e.what().c_str());
      return COMMUNICATION_ERROR;
   }
}



int PowerCtrl::VariableHandler( Variable *var)
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

int debug;
int main( int argc, char **argv) {

   debug = 0;

   SetVersion(VersMajor,VersMinor);

   try {
      PowerCtrl *c;

      c = new PowerCtrl( argc, argv);

      c->Exec();
      
      delete c;
   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}

