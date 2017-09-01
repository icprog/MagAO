// StagesCtrl.h header file
#ifndef STAGESCTRL_H
#define STAGESCTRL_H

#include "AOApp.h"


//@Class: StageCtrl
//
// Extends an AOApp to control a Bayside stage attached to an iDrive controller

class StageCtrl: public AOApp {

public:
      StageCtrl( int argc, char **argv) throw (AOException);

protected:
   void Create(void) throw (AOException);

 
   // Stage operation      
   int SwitchOn(void);
   int SwitchOff(void);
   int StartPoleFinding(void);
   int StartHoming(void);
   int MoveTo(double);

   // local FSM
   int DoFSM(void);

   // Refresh RTDB variables
   int SetPositionVars(void);

   // RTDB handlers
   static int PosReqChanged( void *pt, Variable *msgb);
   static int LimitReqChanged( void *pt, Variable *msgb);
   static int EnableReqChanged( void *pt, Variable *msgb);
   static int BrakeReqChanged( void *pt, Variable *msgb);
   static int SpecialReqChanged( void *pt, Variable *msgb);

   protected:

   // VIRTUAL - Setups variables in RTDB
   void SetupVars(void);

   // VIRTUAL - Perform post-initialization settings
   void PostInit();

   // VIRTUAL - Run
   // switches the stage from one state to the other
   void Run();

   // Connects to iDrive controller
   int SetupNetwork(void);

   // Loads a configuration file onto iDrive
   int LoadParameterFile( std::string filename);


   // Unit conversion
   double StepsToUnits(double);
   double UnitsToSteps(double);

   protected:

   // MV number (set from cfg file)
   int _driveNum;

   // MV name
   std::string _driveName;

   // iDrive hardware address (usually 1, set from cfg file)
   int _driveHwAddr;

   // iDrive network address
   std::string _driveNetAddr;
   int         _driveNetPort;

   // Steps/unit ratio
   float _stepsRatio;

   // Movement limits
   float _mvLowEnd;
   float _mvHighEnd;
   float _goodWindow;

   // RTDB variables

   RTDBvar var_name, var_errmsg, var_enable_cur, var_enable_req;
   RTDBvar var_loend, var_hiend, var_pos_cur, var_pos_req;
   RTDBvar var_current, var_brake_cur, var_brake_req;
   RTDBvar var_switches_req, var_switches_cur;
   RTDBvar var_errcode, var_special;


};

#endif // STAGESCTRL_H
