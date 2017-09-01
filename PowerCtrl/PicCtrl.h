// @File: PicCtrl.h
//
//@

#ifndef RELAY_CTRL_H_INCLUDED
#define RELAY_CTRL_H_INCLUDED

#include "PowerBoardItem.h"
#include "AbstractRelayCtrl.h"

//@Class: PicCtrl
//
// Extends an AOApp to control the relay board
//@

class PicCtrl: public AbstractRelayCtrl {

public:
   PicCtrl( int argc, char **argv) throw (AOException);

protected:
   void Create(void) throw (AOException);

   // RTDB handlers
   static int LampReqChanged( void *pt, Variable *var);

   virtual void updateLoop(void);

   void updateTemps(void);

   float AskValue( std::string category, int num);

   int TestLink(void);

protected:

  // VIRTUAL - Setup RTDB variables
  void SetupVars();
  void PostInit() throw (AOException);

protected:

   // Powerboard network address
   int t_num;
   int d_num;
   int h_num;

   std::string _lampUpCmd, _lampDownCmd;
   std::string _lampUpAnswer, _lampDownAnswer;

   int _lampIntReq, _lampIntCur;
   std::string _readSensirion;

   // RTDB variables
   RTDBvar var_temps, var_humidity, var_dewpoints;

   RTDBvar var_lamp_cur, var_lamp_req;
};

#endif // RELAY_CTRL_H_INCLUDED


