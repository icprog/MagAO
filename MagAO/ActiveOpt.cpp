#include "ActiveOpt.h"

using namespace Arcetri;

//#define _debug

ActiveOpt::ActiveOpt( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
}


void ActiveOpt::Create() throw (AOException)
{   
   pthread_mutex_init( &mutex, NULL);

   network_ok = 0;

   LoadConfig();

   _M1OffloadOn = 0;
   
}

int ActiveOpt::LoadConfig()
{
   Config_File *cfg = &ConfigDictionary();
   try 
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "Reading vars.");

      M1addr  = (std::string) (*cfg)["M1addr"];

      M1port = (int)(*cfg)["M1port"];

   } 
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "Missing M1 Server configuration data: %s", e.what().c_str());
      throw(e);
   }

   try
   {
      _M1PollInterval = (int)(*cfg)["M1PollInterval"];
   }
   catch (Config_File_Exception &e)
   {
      _M1PollInterval = 1;
   }
   Logger::get()->log( Logger::LOG_LEV_INFO, "M1 polling interval (M1PollInterval) set to %i secs", _M1PollInterval);
   

   return 0;
}


void ActiveOpt::SetupVars()
{
   //Setup the M1 RTDB variables
   try
   {
      var_M1_offload_REQ = RTDBvar("M1", "OFFLOAD",  REQ_VAR, INT_VARIABLE, 1);
      Notify(var_M1_offload_REQ, M1_offload_on_changed);

      var_M1_offload_CUR = RTDBvar("M1", "OFFLOAD",  CUR_VAR, INT_VARIABLE, 1);
      var_M1_offload_CUR.Set(_M1OffloadOn, 0, FORCE_SEND);
      _logger->log( Logger::LOG_LEV_INFO, "M1.OFFLOAD.CUR initialize to %i.", _M1OffloadOn);

      var_M1_coeffs = RTDBvar("AOS.M1", "COEF",  REQ_VAR, REAL_VARIABLE, 13);
      Notify(var_M1_coeffs, M1_coeffs_changed);

   }
   catch (AOVarException &e)
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating AOI AOS RTDB variables");
   }

}

int ActiveOpt::changeOffload(int newoff)
{
   if(newoff <= 0)  _M1OffloadOn = 0;
   else _M1OffloadOn = 1;

   var_M1_offload_CUR.Set(_M1OffloadOn);

   if(_M1OffloadOn)
   {
      _logger->log( Logger::LOG_LEV_INFO, "M1.OFFLOAD.CUR set to %i.  Offloads to M1 enabled.", _M1OffloadOn);
   }
   else
   {
      _logger->log( Logger::LOG_LEV_INFO, "M1.OFFLOAD.CUR set to %i.  Offloads to M1 disabled.", _M1OffloadOn);
   }

   return 0;
}

int ActiveOpt::M1_offload_on_changed(void *pt, Variable *msgb)
{
   ActiveOpt * aopt = (ActiveOpt *) pt;

   return aopt->changeOffload(msgb->Value.Lv[0]);
   
}

int ActiveOpt::M1_coeffs_changed(void *pt, Variable *msgb)
{
   ActiveOpt * aopt = (ActiveOpt *) pt;

   std::vector<double> offlv;

   offlv.resize(msgb->H.NItems);
   for(int i=0;i<msgb->H.NItems; i++)
   {
      offlv[i] = msgb->Value.Dv[i];
   }
   return aopt->process_offload(offlv);
}

int ActiveOpt::process_offload(std::vector<double> offlvec)
{
   std::string offlcom;
   char coeff[32];
   std::string resp;
   double oval;

   if(_M1OffloadOn)
   {
      //std::cout << "Got offload vector of size " << offlvec.size() << "\n";

      offlcom = "UPDR ao limit";

      for(int i=0;i<4;i++) offlcom += " 0.000";

      




      //We're only doing astigmatism and putting it in the eng-only Zernike modes
      oval = offlvec[0];
      if(fabs(oval) < 1e-5) oval = 0.0; //we're only sending three zeros anyway.
      snprintf(coeff, 32, " %0.3f", oval);
      offlcom += coeff;

      oval = offlvec[1];
      if(fabs(oval) < 1e-5) oval = 0.0; //we're only sending three zeros anyway.
      snprintf(coeff, 32, " %0.3f", oval);
      offlcom += coeff;

      for(int i=6;i<9;i++) offlcom += " 0.000";

      for(unsigned int i=2; i < offlvec.size(); i++)
      {
         oval = offlvec[i];
         if(fabs(oval) < 1e-5) oval = 0.0; //we're only sending three zeros anyway.
         snprintf(coeff, 32, " %0.3f", oval);
         offlcom += coeff;
      }

      for(unsigned int i=0; i < (37-offlvec.size() - 7); i++) offlcom += " 0.000";

      //offlcom += '\n';

      _logger->log( Logger::LOG_LEV_INFO, "[OFFL] Received offload vector.  Sending: SEND ao");
      resp = getM1Status("SEND ao");

      if(resp[0] == '1')
      {
         _logger->log( Logger::LOG_LEV_INFO, "[OFFL] M1 Response: %s", resp.c_str());
      }
      else
      {
         _logger->log( Logger::LOG_LEV_ERROR, "[OFFL] M1 returned error for SEND ao: %s", resp.c_str());
         return -1;
      }

      //std::cout << offlcom << "\n";
      _logger->log( Logger::LOG_LEV_INFO, "[OFFL] Sending vector: %s", offlcom.c_str());
      resp = getM1Status(offlcom);
      _logger->log( Logger::LOG_LEV_INFO, "[OFFL] M1 Response: %s", resp.c_str());
         //std::cout << resp << "\n";

   }
//    else
//    {
//       std::cout << "Offloads off\n";
//    }

   return 0;
}
   
void ActiveOpt::Run()
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

void ActiveOpt::DoFSM()
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
   
   // Always set current status (for external watchdog)
   setCurState( getCurState());
   
   usleep( (unsigned int)( _M1PollInterval * 1e6));
   
}




/********************* TCS Interface ********************************/

int ActiveOpt::SetupNetwork()
{
   int stat;
   static int logged = 0;
   //Always close previous connection
   ShutdownNetwork();

   if(!logged) Logger::get()->log( Logger::LOG_LEV_INFO, "Connecting to %s:%d", M1addr.c_str(), M1port);

   // Setup serial/network interface
   pthread_mutex_lock(&mutex);

   //?? Is this the right thing to use for non-serial comms?
   stat = SerialInit( M1addr.c_str(), M1port ); // This locks if network is down or host unreachable

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
}//int ActiveOpt::SetupNetwork()

int ActiveOpt::ShutdownNetwork()
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
}//int ActiveOpt::ShutdownNetwork()


int ActiveOpt::TestNetwork(void)
{
   int stat;
   
   std::string resp;

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Testing network, network_ok=%d", network_ok);
   if (!network_ok)
   {
      stat = SetupNetwork();
      if(stat != NO_ERROR) return stat;
   }
   
   
   resp = getM1Status("SEND");
   
   //Process response here.
   //turn into a stat
   if(resp == "") stat = -1;
   else stat = 0;
   
   if (stat<0)
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "Error sending test command");
      //pthread_mutex_unlock(&aoiMutex);
      return stat;
   }

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Network test OK - answer %s", resp.c_str());
   return NO_ERROR;
}//int ActiveOpt::TestNetwork(void)
                
std::string ActiveOpt::getM1Status(const std::string &statreq)
{
   int stat;
   char answer[512];
   std::string statreq_nl;
   
   #ifdef _debug
      std::cout << "Sending " << statreq << "\n";
   #endif
   
   pthread_mutex_lock(&mutex);
   
   Logger::get()->log( Logger::LOG_LEV_TRACE, "Sending status request: %s", statreq.c_str());
   
   statreq_nl = statreq;
   statreq_nl += '\n';
   stat = SerialOut(statreq_nl.c_str(), statreq_nl.length());
   
   if(stat != NO_ERROR)
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "Error sending status request: %s", statreq.c_str());
      pthread_mutex_unlock(&mutex);
      return "";
   }
   
   #ifdef _debug
      std::cout << "waiting for response\n";
   #endif

   stat = SerialInString(answer, 512, 10000, '\n'); //ToDO - need to wait for \r\n!!
   pthread_mutex_unlock(&mutex);
   #ifdef _debug
      std::cout << "Response " << answer << "\n";
   #endif

   if(stat <= 0)
   {
      Logger::get()->log(Logger::LOG_LEV_TRACE, "No response received to status request: %s", statreq.c_str());   
      return "";
   }
   
   Logger::get()->log(Logger::LOG_LEV_TRACE, "Received response: %s", answer);
      
   return answer;
   
}//std::string ActiveOpt::getM1Status(const std::string &statreq)



