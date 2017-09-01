#ifndef __ActiveOpt_h__
#define __ActiveOpt_h__

#include "AOApp.h"
#include "AOStates.h"

extern "C" 
{
#include "hwlib/netseriallib.h"
}

#include <cmath>

class ActiveOpt : public AOApp
{
   public:
      ActiveOpt(int argc, char **argv) throw (AOException);
      ActiveOpt(std::string name, const std::string &conffile) throw(AOException);

   protected:
      void Create(void) throw(AOException);

      int LoadConfig();

      virtual void SetupVars();

      virtual void Run();

      virtual void DoFSM();

      int _M1PollInterval; ///<Interval at which to poll M1 for network status.

      /** @name M1 state
        */
      //@{

      int _M1OffloadOn; ///<Flag to control whether offload requests are actually sent to M1

      RTDBvar var_M1_offload_REQ; ///<RTDB variable for turning on/off M1 offloading
      RTDBvar var_M1_offload_CUR; ///<RTDB variable for turning on/off M1 offloading

      ///Process a change in _M1OffloadOn
      int changeOffload(int newoff);

      ///RTDB handler for M1 offload requests
      static int M1_offload_on_changed(void *pt, Variable *msgb);

      RTDBvar var_M1_coeffs; ///<RTDB variable for M1 offload coefficients.

      ///RTDB handler for M1 offload requests
      static int M1_coeffs_changed(void *pt, Variable *msgb);

      int process_offload(std::vector<double> offlvec);

      //@}
      
      /** @name M1 Server Interface
        * See M1server_commands.html for documentation of the Magellan M1 server.
        */
      //@{

      ///IP Address of the M1 server (from config)
      std::string   M1addr;
      
      ///Port of the M1 server (from config)
      int  M1port;

      pthread_mutex_t mutex; ///< Mutex to lock communication

      int  network_ok; ///< network connection status

      ///Connect to the TCS
      int SetupNetwork();

      ///Disconnect from the TCS
      int ShutdownNetwork();

      ///Test the connection with TCS.
      virtual int TestNetwork();

      ///Send a status query to the M1 Server and return the response
      std::string getM1Status(const std::string &statreq);

      //@}
      
   public:
 
};



#endif





