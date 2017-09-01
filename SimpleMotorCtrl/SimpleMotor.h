//@File: SimpleMotor.h
//
// Class declaration for a simple generic linear or rotary motor,
// to be derived into specific classes
//
// CURRENT HIERARCHY
//				
//								SimpleMotor
//									 |
//				   |-----------------|----------------|
//				   |				 |				  |
//			   MCBL2805			  Mercury		  Rerotator
//				   |
//			  FilterWheel
//@

#ifndef SIMPLEMOTOR_H_INCLUDED
#define SIMPLEMOTOR_H_INCLUDED

#include <pthread.h>

#include "AOApp.h"
#include "base/rtdblib.h"
#include "stdconfig.h"
#include "RTDBvar.h"


class SimpleMotor
{

public:

public:
	SimpleMotor( AOApp *app, Config_File &cfg);				// Constructor 
	virtual ~SimpleMotor();				// Deconstructor

public:


public:
	int LoadConfig( Config_File &cfg);
	virtual int SetupVars();

	int SetupNetwork( int force=0);
	int ShutdownNetwork(void);

	
public:
	virtual int 	TurnOn(void)=0;			// Handle everything needed to turn on the motor
	virtual int 	TurnOff(void)=0;		// Handle everything needed to turn off the motor

	virtual int 	Home(void)=0;			// Start homing sequence, if one exists
	virtual int 	MoveTo( double pos)=0;	// Move the motor to the selected position
	virtual double 	GetPosition( int force=false)=0;	// Get the current position
	int 			GetStatus(void);		// Get the current status

	virtual int IsMoving(void)=0;		// Check if the motor is moving, returns 1 if yes
        virtual int Disable();                  // Disable drive after movements (optional)
	virtual int Abort(void)=0;			// Abort any current movement, returns 1 if possible and done

   virtual int TestNetwork(void)=0; 	// Test network connection

   virtual int EndHoming(void);

   virtual int GetSwitchStatus(void);   // Returns current limit switch status (if applicable)
   
public:
	const char *GetTargetVarname(void);			// Returns the complete name of the TGTPOS variable
   	RTDBvar    *GetTargetVar(void) { return &var_pos_req; }
   void SetPosName(string name) { var_pos_name.Set(name); }

   // Get/Set homing offset
   double getHomingOffset();
   void setHomingOffset( double offset);

	double PosToSteps( double position);		// Convert from positioning command to motor steps
	double StepsToPos( double command);			// Convert from motor steps to positioning command

protected:
	virtual int SendCommand(const char *fmt, ...)=0;	// Format a serial command in the right way and send it


	int 	status;
	float32 	steps_per_pos;

	float32 MVmin;
	float32 MVmax;
	int    MVnumber;
	char   *MVaddr;
	int    MVport;	
	char   *MVname;

	int  network_ok;
	const char *errmsg; 
	int  ERRMSG_LEN;

   float32 _homingOffset;           // Offset from the homing position to the logical zero position
                                    // (in motor step units)
public:
   //made var_pos_req public for VisAO modifications
	RTDBvar var_pos_req, var_pos_cur;
   

protected:  
  RTDBvar    var_loend, var_hiend, var_sw_status;
   RTDBvar var_pos_name;
   	AOApp *app;

   	pthread_mutex_t mutex;     // Mutex to lock communication
};


#endif // SIMPLEMOTOR_H_INCLUDED

