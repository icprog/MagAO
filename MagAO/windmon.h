#ifndef __windmon_h__
#define __windmon_h__

#include "AOApp.h"
#include "AOStates.h"

extern "C" 
{
#include "hwlib/netseriallib.h"
}

#include <cmath>

class windmon : public AOApp
{
   public:
      windmon(int argc, char **argv) throw (AOException);
      windmon(std::string name, const std::string &conffile) throw(AOException);

   protected:
      void Create(void) throw(AOException);

      int LoadConfig();

      virtual void SetupVars();

      virtual void Run();

      virtual void DoFSM();

      int pollInterval; ///<Interval at which to read the wind monitor output.

      int N_avg; ///<The number of measurements to include in the circular buffer
      int N_rej;  ///<The number of high and low measurements to reject
      double lastspeed; ///< the last value read from the probe, in m/s.
      double airspeed_mean; ///<Local value of windspeed from probe, in m/s, mean of N_hist measurements.
      double airspeed_median; ///<Local value of windspeed from probe, in m/s, median of N_hist measurements.
      double airspeed_hlrej; ///<Local value of windspeed from probe, in m/s, mean of N_hist measurements with high/low rejection of N_rej samples.


      double wxwind; ///<Local value of TCS wind, converted to m/s.

      RTDBvar var_airspeed;  ///<RTDB airspeed variable 
      RTDBvar var_wxwind;

      RTDBvar var_TSSoverride; ///<If set to 1, then override_TSS is set to 1.
      bool override_TSS; ///<If set to 1, the windspeed is set to 500 m/s to engage TSS

      /// RTDB handler for a PresetVisAO command via MsgD.
      static int TSSoverride_changed(void *pt, Variable *msgb);
      int set_override_TSS();
      
      // Wind logger

       Logger *_windsLogger;

      /** @name TCP/IP Interface
        * 
        */
      //@{

      ///IP Address of the port server (from config)
      std::string   ipAddr;
      
      ///Port 
      int  port;

      pthread_mutex_t mutex; ///< Mutex to lock communication

      int  network_ok; ///< network connection status

      ///Connect to the TCS
      int SetupNetwork();

      ///Disconnect from the TCS
      int ShutdownNetwork();

      ///Test the connection with TCS.
      virtual int TestNetwork();


      //@}

      //The circular buffer
      int avg_idx;
      std::vector<double> measures;
      std::vector<double> smeasures;

   public:
 
};



#endif





