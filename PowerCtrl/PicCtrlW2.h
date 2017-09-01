// @File: PicCtrlW2.h
//
//@

#ifndef RELAY_CTRL_H_INCLUDED
#define RELAY_CTRL_H_INCLUDED

#include "PowerBoardItem.h"
#include "AbstractRelayCtrl.h"

//@Class: PicCtrlW2
//
// Extends an AOApp to control the relay board
//@

class PicCtrlW2: public AbstractRelayCtrl {

public:
   PicCtrlW2( const std::string &conffile) throw (AOException);

   PicCtrlW2( int argc, char **argv) throw (AOException);

protected:
   void Create(void) throw (AOException);

   // RTDB handlers
   static int LampReqChanged( void *pt, Variable *var);

   virtual void updateLoop(void);

   void updateTemps(void);
   int TestLink(void);
   double dewpoint( double t, double h);


   float AskValue( std::string category, int num, std::string terminator);

   int DetectLamp();

protected:

  // VIRTUAL - Setup RTDB variables
  void SetupVars();
  void PostInit() throw (AOException);

  void StateChange( int oldstate, int state);


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

   // Additional telemetry
   Logger *_temperatureLogger;
   Logger *_humidityLogger;
   Logger *_dewPointLogger;

   vector<double> _temperatureVector;
   vector<double> _humidityVector;
   vector<double> _dewPointVector;

};

#endif // RELAY_CTRL_H_INCLUDED


