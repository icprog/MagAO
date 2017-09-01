// @File: RelayCtrl.h
//
//@

#ifndef RELAY_CTRL_H_INCLUDED
#define RELAY_CTRL_H_INCLUDED

#include "PowerBoardItem.h"
#include "AbstractRelayCtrl.h"

//@Class: RelayCtrl
//
// Extends an AOApp to control the relay board
//@

class RelayCtrl: public AbstractRelayCtrl {

public:
   RelayCtrl( int argc, char **argv) throw (AOException);

protected:
   void Create(void) throw (AOException);

   // RTDB handlers
   static int OvertempTh1Changed( void *pt, Variable *msgb);
   static int OvertempTh2Changed( void *pt, Variable *msgb);
   static int OvertempActiveChanged( void *pt, Variable *msgb);

   virtual void updateLoop(void);
   void updateTemps(void);
   int parseTemps( std::string s);

   int TestLink(void);

protected:

  // VIRTUAL - Setup RTDB variables
  void SetupVars();
  void StateChange( int oldstate, int state);


protected:

   // Powerboard network address
   int t_num;

   // Overtemp management
   double parseOvertemp( std::string answ);
   void IncTh1();
   void DecTh1();
   void IncTh2();
   void DecTh2();
   void EnableOvertemp();
   void DisableOvertemp();

   bool _overtempEnabled;
   bool _overtempAlarm;

   std::string _th1up, _th1down, _th1answer;
   std::string _th2up, _th2down, _th2answer;
   std::string _overtempEnable, _overtempEnableAnswer;
   std::string _overtempDisable, _overtempDisableAnswer;

   RTDBvar var_temps;
   RTDBvar var_overtemp_active_cur;
   RTDBvar var_overtemp_active_req;
   RTDBvar var_overtemp_th1_cur;
   RTDBvar var_overtemp_th1_req;
   RTDBvar var_overtemp_th2_cur;
   RTDBvar var_overtemp_th2_req;
   RTDBvar var_overtemp_alarm;

};

#endif // RELAY_CTRL_H_INCLUDED


