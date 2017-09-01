//+File: Pinger.cpp
//
// {\tt Pinger} : MsgD-RTDB multithreaded client for IP-based host monitoring
//

#include <vector>
using namespace std;

extern "C" {
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <unistd.h>     // fork()
#include <stdlib.h>		// atof()
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>      // popen()
#include <signal.h>

#include <pthread.h>

#include "base/thrdlib.h"
#include "hwlib/netseriallib.h"
}

#include "Pinger.h"

using namespace Arcetri;

// **************************************
// Client name and versioning information
// **************************************

int VersMajor = 1;
int VersMinor = 0;
const char *Date = "Sep 2007";
const char *Author = "A. Puglisi";

//+Entry help
//
//  help prints an usage message
//
// Rimosso perche' non usato (L.F.)
// 
//static void help()
//{
//    printf("\nPinger  - Vers. %d.%d.  %s, %s\n\n", VersMajor,VersMinor,Author,Date);
//    printf("Usage: Pinger -f <config file> [-v]\n\n");
//    printf("   -f    configuration file\n");
//    printf("   -v    increase verbosity\n");
//    printf("\n");
//}

// +Main: 
//

Pinger::Pinger( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
}

void Pinger::Create() throw (AOException)
{
      try {
         _pollingPeriod   = ConfigDictionary()["PollingPeriod"];
         _timeoutPeriod   = ConfigDictionary()["TimeoutPeriod"];

         // Create the device list and corresponding variable array
         map<string, Config_Value>::iterator iter;
         for (iter = ConfigDictionary().begin(); iter != ConfigDictionary().end(); iter++)
            {
            if (iter->first.substr(0,7) == "device_")
               {
               std::string device_name = iter->first.substr(7);
               std::string host = (std::string)iter->second;
               RTDBvar var( MyFullName(), device_name, NO_DIR, REAL_VARIABLE, 1, false);

               struct device d;
               d.host = host;
               d.var = var;
               d.shmid = shmget( IPC_PRIVATE, sizeof(double), IPC_CREAT | 0777);
               d.pnt   = (double *)shmat( d.shmid, NULL, 0);

               devices.push_back(d);              
               }

            }
        } catch (Config_File_Exception &e) {
             _logger->log( Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
            throw AOException("Fatal: Missing configuration data");
         }

}



void Pinger::PostInit()
{
   vector<struct device>::iterator iter;
   for (iter = devices.begin(); iter != devices.end(); iter++)
      {
      iter->var.Create();
      iter->var.Set(-1.0);
      }

}

void Pinger::Run()
{
      while(!TimeToDie())
            {
            vector<struct device>::iterator iter;

            // Fork all pings
            for (iter = devices.begin(); iter != devices.end(); iter++)
               {
               iter->pid = fork();
               if (iter->pid == 0)
                     {
                     double *pnt = (double *)shmat(iter->shmid, NULL, 0);
                     *pnt = -1.0;

                     std::string cmd = "ping -c1 -w1 -q ";
                     cmd += iter->host;
                     FILE *fp = popen(cmd.c_str(), "r");
                     char line[256];
                     char *found;
                     double time = -1.0;
                     while (fgets(line, 256, fp) != NULL)
                        if ((found = strstr(line, "mdev = ")))
                           {
                           time = atof(found+7);
                           *pnt = time;
                           break;
                           }
                     exit(0);
                     }
            }

            nusleep( (unsigned int)(_timeoutPeriod * 1e6));

            // Kill all lingering processes
            for (iter = devices.begin(); iter != devices.end(); iter++)
               kill(iter->pid, SIGKILL);

            // Remove zombie childs
            for (iter = devices.begin(); iter != devices.end(); iter++)
               waitpid( iter->pid, NULL, 0);

            // Set RTDB variables
            for (iter = devices.begin(); iter != devices.end(); iter++) {
               _logger->log( Logger::LOG_LEV_INFO, "Host %s: %f [ms]", iter->host.c_str(), *iter->pnt);
               try {
                  iter->var.Set( *iter->pnt, 0, FORCE_SEND);
               } catch (AOException &e) {
                  Logger::get()->log( Logger::LOG_LEV_ERROR, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
               }
            }

            nusleep( (unsigned int) ((_pollingPeriod - _timeoutPeriod)*1e6));

            }
}

int main( int argc, char **argv) {

   SetVersion(VersMajor,VersMinor);

   try {
      Pinger *c;
      c = new Pinger( argc, argv);
      c->Exec();
      
      delete c;
   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}

