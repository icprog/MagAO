// @File: RelayCtrlW2.h
//
//@

#ifndef RELAY_CTRL_H_INCLUDED
#define RELAY_CTRL_H_INCLUDED

#include "PowerBoardItem.h"
#include "AbstractRelayCtrl.h"

//@Class: RelayCtrlW2
//
// Extends an AOApp to control the relay board
//@

class RelayCtrlW2: public AbstractRelayCtrl {

public:
   RelayCtrlW2( int argc, char **argv) throw (AOException);

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

   Logger *_boardTempsLogger;
   Logger *_boxesTempsLogger;

   vector<double> _boardTemps;
   vector<double> _boxesTemps;

protected:

  // VIRTUAL - Setup RTDB variables
  void SetupVars();

  void StateChange( int oldstate, int state);

  void Run();

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
   void DetectTh1();
   void DetectTh2();

   std::string _th1up, _th1down, _th1answer;
   std::string _th2up, _th2down, _th2answer;
   std::string _overtempEnable, _overtempEnableAnswer;
   std::string _overtempDisable, _overtempDisableAnswer;


   // RTDB variables
   RTDBvar var_temps;

   RTDBvar var_overtemp_active_cur;
   RTDBvar var_overtemp_active_req;
   RTDBvar var_overtemp_th1_cur;
   RTDBvar var_overtemp_th1_req;
   RTDBvar var_overtemp_th2_cur;
   RTDBvar var_overtemp_th2_req;

};

#endif // RELAY_CTRL_H_INCLUDED


