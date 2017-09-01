//@File: SimpleMotorCtrl.h
//
// Functions declarations for the SimpleMotorCtrl program
//
//@

#ifndef SIMPLEMOTORCTRL_H_INCLUDED
#define SIMPLEMOTORCTRL_H_INCLUDED

#include "AOApp.h"
#include "SimpleMotor.h"
#include "AOStates.h"

extern "C" {
#include "base/timelib.h"
}

#include <map>
using namespace std;

class SimpleMotor;


class SimpleMotorCtrl : public AOApp {

  	 public:

        SimpleMotorCtrl( std::string name, const std::string& conffile) throw (AOException);
      	SimpleMotorCtrl( int argc, char **argv) throw (AOException);

      	// RTDB handlers
      	static int PosReqChanged( void *pt, Variable *msgb);

         void setSavedState( double pos);


	protected:

		friend class SimpleMotor;

   	void Create(void) throw (AOException);

	   virtual SimpleMotor *CreateMotor();

	   // Local FSM
	   virtual int DoFSM();

      // Update position variables in RTDB
      virtual void updatePos( bool force=false);


   	protected:
	   // VIRTUAL - Setup variables in RTDB
	   virtual void SetupVars();
      
	   // VIRTUAL - Install thrdlib handlers
	   void InstallHandlers();

	   // VIRTUAL - Run
	   // switches the stage from one state to the other
	   void Run();

   	protected:

      // HANDLERS
      static int          savestate_handler(MsgBuf *, void *, int hndlrQueueSize);

      // RTDB variables	
      RTDBvar _fsmStatus;

      //Publish these for VisAO use
      RTDBvar var_startingpos;
      RTDBvar var_homepos;
      RTDBvar var_abortpos;

      // Internal state

      float _homingPos;
      float _abortPos;
      float _goodWindow;
      float _startingPos;
      int   _autoHoming;
      int   _autoHomingOffset;
      float _targetPos;

      double _savedPos;
      bool   _savedState;
      int    _posAverage;

      int    _homingDither; //Config variable to tell us to use the homing dither
      double _homingDitherOffset; //Config variable, how big is the offset
      
      bool NoDisable; //MagAO doesn't disable ADC motors

     
      // Disable movement after a long timeout
      time_t _starttime;
      time_t _disabletime;

public:
      int _homingDitherApplied; //flag for whether the homing dither has been applied or not

protected:
	      // The object we are controlling
	      SimpleMotor *motor;

      // Custom positions list
      map<string, float> _customPos;

               
};

#endif


