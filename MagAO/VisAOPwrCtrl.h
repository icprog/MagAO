
#ifndef __VisAOPwrCtrl_h__
#define __VisAOPwrCtrl_h__

#include "AOApp.h"
#include "AOStates.h"

#include <errno.h>
extern "C" {
#include "hwlib/netseriallib.h"
}
using namespace std;

//#define _debug

class VisAOPwrCtrl : public AOApp 
{
   public:

      VisAOPwrCtrl( int argc, char **argv) throw(AOException);
     
      int setOutletState(int outnum, int outstate);
      
      int parseOutletNumber(const std::string &stStr);
      int parseOutletStatus(const std::string &stStr);
      int parseStatus(const std::string &ansstr);
      int sendOutletState(int on, int os);
      
      
   protected:
      void Create(void) throw (AOException);
      void updateState(bool force = false);
      
   protected:
      int LoadConfig();

      /// VIRTUAL - Setup variables in RTDB
      void SetupVars();
      
      /// VIRTUAL - Run
      void Run();
      int DoFSM();

      
      char   *PSaddr;
      int    PSport;	
      int  network_ok;
      const char *errmsg; 
      int  ERRMSG_LEN;
      
      int SetupNetwork( int force=0);
      int ShutdownNetwork(void);

      virtual int TestNetwork(void); 
      virtual int EmptySerial( int avoidLock, int timeout);
      
      int sendCommand(const char * com);
      
   protected:

     

      // Internal state

      int outletState[8]; //Current state of the 8 outlets
      int _ctype; //Control Mode.


      std::vector<std::string> item_codes;
      std::vector<std::string> item_names;


      RTDBvar var_outlet1_cur, var_outlet1_req, var_outlet1_name;
      RTDBvar var_outlet2_cur, var_outlet2_req, var_outlet2_name;
      RTDBvar var_outlet3_cur, var_outlet3_req, var_outlet3_name;
      RTDBvar var_outlet4_cur, var_outlet4_req, var_outlet4_name;
      RTDBvar var_outlet5_cur, var_outlet5_req, var_outlet5_name;
      RTDBvar var_outlet6_cur, var_outlet6_req, var_outlet6_name;
      RTDBvar var_outlet7_cur, var_outlet7_req, var_outlet7_name;
      RTDBvar var_outlet8_cur, var_outlet8_req, var_outlet8_name;
      
      RTDBvar var_ctype_cur, var_ctype_req;
      
      pthread_mutex_t mutex;     // Mutex to lock communication

   public:
      // HANDLERS
      
      static int StateReqChanged(void *pt, Variable *msgb);
      static int CtypeReqChanged(void *pt, Variable *msgb);

      static int outlet1ReqChanged(void *pt, Variable *msgb);
      static int outlet2ReqChanged(void *pt, Variable *msgb);
      static int outlet3ReqChanged(void *pt, Variable *msgb);
      static int outlet4ReqChanged(void *pt, Variable *msgb);
      static int outlet5ReqChanged(void *pt, Variable *msgb);
      static int outlet6ReqChanged(void *pt, Variable *msgb);
      static int outlet7ReqChanged(void *pt, Variable *msgb);
      static int outlet8ReqChanged(void *pt, Variable *msgb);
      
      
      
};

#endif //__VisAOPwrCtrl_h__


