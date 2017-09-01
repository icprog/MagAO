#include "windmon.h"

using namespace Arcetri;

//#define _debug

windmon::windmon( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
}


void windmon::Create() throw (AOException)
{   
   pthread_mutex_init( &mutex, NULL);

   network_ok = 0;

   _windsLogger = Logger::get("WINDSONIC", Logger::LOG_LEV_INFO, "TELEMETRY");

   LoadConfig();
   
}

int windmon::LoadConfig()
{
   Config_File *cfg = &ConfigDictionary();
   try 
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "Reading vars.");

      ipAddr  = (std::string) (*cfg)["ipAddr"];

      port = (int)(*cfg)["port"];

   } 
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "Missing port server configuration data: %s", e.what().c_str());
      throw(e);
   }

   try
   {
      pollInterval = (int)(*cfg)["pollInterval"];
   }
   catch (Config_File_Exception &e)
   {
      pollInterval = 1;
   }
   Logger::get()->log( Logger::LOG_LEV_INFO, "Polling interval (pollInterval) set to %i secs", pollInterval);


   try
   {
      N_avg = (int)(*cfg)["N_avg"];
   }
   catch (Config_File_Exception &e)
   {
      N_avg=10;
   }
   Logger::get()->log( Logger::LOG_LEV_INFO, "Averaging window (N_avg) set to %i points", N_avg);

   try
   {
      N_rej = (int)(*cfg)["N_rej"];
   }
   catch (Config_File_Exception &e)
   {
      N_rej=2;
   }
   Logger::get()->log( Logger::LOG_LEV_INFO, "High/low rejection (N_rej) set to %i points", N_rej);


   lastspeed = 0;
   measures.resize(N_avg);
   smeasures.resize(N_avg);
   avg_idx = 0;

   return 0;
}


void windmon::SetupVars()
{
   //Setup the RTDB variables
    try
    {
      var_airspeed = RTDBvar("AOS", "SHELL.WINDSPEED",  NO_DIR, REAL_VARIABLE, 1);
      var_wxwind = RTDBvar("AOS", "TEL.WXWIND",  NO_DIR, REAL_VARIABLE, 1);
      var_TSSoverride = RTDBvar("AOS", "SHELL.TSS_OVERRIDE",  NO_DIR, INT_VARIABLE, 1);

      Notify( var_TSSoverride, TSSoverride_changed);
      var_TSSoverride.Set(0,0, FORCE_SEND);
      override_TSS = false;


    }
    catch (AOVarException &e)
    {
       Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%i: %s", __FILE__, __LINE__, e.what().c_str());
       throw AOException("Error creating AOI AOS RTDB variables");
    }

}

int windmon::TSSoverride_changed(void *pt, Variable *msgb)
{
   windmon * wm = (windmon *) pt;

   return wm->set_override_TSS();
}

int windmon::set_override_TSS()
{
   var_TSSoverride.Update();

   var_TSSoverride.Get(&override_TSS);

   std::cout << "override_TSS set to " << override_TSS << "\n";

   if(override_TSS)
   {
      var_airspeed.Set(500.0, 0, FORCE_SEND);
   }
   return 0;
}

void windmon::Run()
{
   _logger->log( Logger::LOG_LEV_INFO, "Running...");
   //TestNetwork();

   while(!TimeToDie()) 
   {
      try 
      {
        
         //TestNetwork();
         DoFSM();
      } 
      catch (AOException &e) 
      {
         _logger->log( Logger::LOG_LEV_ERROR, "Caught exception (at %s:%d): %s", __FILE__, __LINE__, e.what().c_str());
         
         // When the exception is thrown, the mutex was held!
         pthread_mutex_unlock(&mutex);
         usleep( (unsigned int)( 1e6));
      }
   }
}

void windmon::DoFSM()
{
   int status;

   status = getCurState();
   
   // Always check the network
   if (status != STATE_NOCONNECTION)
   {
      if(TestNetwork() != NO_ERROR)
      {
         setCurState(STATE_NOCONNECTION);
         status = STATE_NOCONNECTION;
      }
      else
      {
         setCurState(STATE_OPERATING);
      }
   }
   
   switch(status)
   {
      case STATE_NOCONNECTION:
         if (SetupNetwork() == NO_ERROR)
         {
            if (TestNetwork() == NO_ERROR)
            {
               setCurState(STATE_CONNECTED);
            }
            else
            {
              ShutdownNetwork();
              setCurState(STATE_NOCONNECTION);
              status = STATE_NOCONNECTION;
            }
         }
         break;

      case STATE_OPERATING:
         


         
         break;
   }
   
   var_wxwind.Update();
   var_wxwind.Get(&wxwind);
   wxwind *= (5280./3600.)*.3048;


   //First calculate mean
   airspeed_mean = 0;

   for(int q = 0; q<N_avg;q++)
   {
      std::cout << measures[q] << " ";
      airspeed_mean += measures[q];
   }

   airspeed_mean /= N_avg;
   std::cout << "mean= " << airspeed_mean << " ";



   //Now calculate median and high/low rejection

   for(int q=0;q<N_avg;++q) smeasures[q] = measures[q];

   std::sort(smeasures.begin(), smeasures.end());

   airspeed_hlrej = 0;
   for(int q = N_rej; q < (N_avg-N_rej); q++)
   {
      airspeed_hlrej += smeasures[q];
   }
   airspeed_hlrej /= (N_avg - 2*N_rej);
   std::cout << "hlmn= " << airspeed_hlrej << " ";

   if(N_avg % 2 == 0)
   {
      airspeed_median = 0.5* (smeasures[(int) (0.5*N_avg-1)] + smeasures[(int) (0.5*N_avg)]);
   }
   else
   {
      airspeed_median = smeasures[(int)0.5*N_avg];
   }
   std::cout << "med= " << airspeed_median << " ";

   std::cout << std::endl;

   if(override_TSS)
   {
      var_airspeed.Set(500.0, 0, FORCE_SEND);
   }
   else
   {
      var_airspeed.Set(airspeed_mean, 0, FORCE_SEND);
   }

    _windsLogger->log( Logger::LOG_LEV_INFO, "%0.4f %0.2f %0.2f %0.2f %0.2f", lastspeed, airspeed_mean, airspeed_hlrej, airspeed_median, wxwind);

   // Always set current status (for external watchdog)
   setCurState( getCurState());
   
   usleep( (unsigned int)( pollInterval * 1e6));
   
}




/********************* TCS Interface ********************************/

int windmon::SetupNetwork()
{
   int stat;
   static int logged = 0;
   //Always close previous connection
   ShutdownNetwork();

   if(!logged) Logger::get()->log( Logger::LOG_LEV_INFO, "Connecting to %s:%d", ipAddr.c_str(), port);

   // Setup serial/network interface
   pthread_mutex_lock(&mutex);

   //?? Is this the right thing to use for non-serial comms?
   stat = SerialInit( ipAddr.c_str(), port ); // This locks if network is down or host unreachable

   pthread_mutex_unlock(&mutex);
   
   if (stat != NO_ERROR)
   {
      network_ok = 0;
      if(!logged) Logger::get()->log( Logger::LOG_LEV_ERROR, "Connect result: %d - Errno: %s", stat, strerror(errno));
      logged = 1;
      return stat;
   }
   logged = 0;
   //CHECK_SUCCESS(stat);
   
   //********** Why are we not doing any error checking here? ************************//

    // Set OK flag
   network_ok =1;

   Logger::get()->log( Logger::LOG_LEV_INFO, "Network connect OK");

   return NO_ERROR;
}//int windmon::SetupNetwork()

int windmon::ShutdownNetwork()
{
   int stat = NO_ERROR;

   if ( network_ok)
   {
      pthread_mutex_lock(&mutex);
      Logger::get()->log( Logger::LOG_LEV_DEBUG, "Closing communication");
      stat = SerialClose();
      pthread_mutex_unlock(&mutex);
      CHECK_SUCCESS(stat);

      network_ok = 0;

   }
   
   return stat;
}//int windmon::ShutdownNetwork()


int windmon::TestNetwork(void)
{
   int stat;
   
   std::string resp;

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Testing network, network_ok=%d", network_ok);
   if (!network_ok)
   {
      stat = SetupNetwork();
      if(stat != NO_ERROR) return stat;
   }
   
  
   char answer[512];
   stat = SerialInString(answer, 512, 100, '\n');

   std::string meas = answer;

   std::cout << answer << "\n";

   std::string tmpstr;
   int p1,p2;

   //Wind Direction
   p1 = meas.find(',',0);
   p2 = meas.find(',', p1+1);

   if(p1 > 0 && p2 > p1)
   {
      tmpstr = meas.substr(p1+1, p2-p1-1);
   }      
   else
   {
      std::cerr << "parse error (direction)\n";
   }

   //Wind Speed
   p1 = p2;
   p2 = meas.find(',', p1+1);
   p1 = p2;
   p2 = meas.find(',', p1+1);

   if(p1 > 0 && p2 > p1)
   {
      tmpstr = meas.substr(p1+1, p2-p1-1);
      std::cout << tmpstr << "\n";

      measures[avg_idx] = strtod(tmpstr.c_str(),0); 
   }      
   else
   {
      std::cerr << "parse error (wind speed)\n";
   }

   //Wind speed units
   p1 = p2;
   p2 = meas.find(',', p1+1);

   if(p1 > 0 && p2 > p1)
   {
      tmpstr = meas.substr(p1+1, p2-p1-1);
   }
   else
   {
      std::cerr << "parse error (units)\n";
   }

   //WindSonic Status Code
   p1 = p2;
   p2 = meas.find('\n', p1+1);

   if(p1 > 0 && p2 > p1)
   {
      tmpstr = meas.substr(p1+1, p2-p1-1);
      std::cout << tmpstr  << "\n";
   }
   else
   {
      std::cerr << "parse error (status)\n";
   }

   lastspeed = measures[avg_idx];
   avg_idx++;
   if(avg_idx >= N_avg) avg_idx = 0;
   
   //else
   //airspeed_mean = -1;
   
   if (stat<0)
   {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error sending test command");
      lastspeed = -1;
      
      return stat;
   }

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Network test OK - answer %s", resp.c_str());
   return NO_ERROR;
}//int windmon::TestNetwork(void)
                
