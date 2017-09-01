//@File: MCBL2805.h
//
// Class for the MCBL2805 motor, derived from the SimpleMotor class
//
//@

#ifndef MCBL2805_H_INCLUDED
#define MCBL2805_H_INCLUDED

#include "SimpleMotor.h"

class MCBL2805: public SimpleMotor
{
public:
   	MCBL2805( AOApp *app, Config_File &cfg);

	virtual int TurnOn(void);     // This is virtual to allow setting the motor type
	int TurnOff(void);

	virtual int Home(void);
	virtual int MoveTo(double pos);
	double GetPosition( int force=false);

	int IsMoving(void);
	int Abort(void);
   	int TestNetwork(void);     // Test network connection
        int Disable(void);	   // Disable drive

   	int GetTemperature(void);

   int GetSwitchStatus(void); 

protected:
	int SendCommand(const char *fmt, ...);
	int SendCommand_nolock(const char *fmt, ...);
	int EmptySerial( int avoidLock = 0, int timeout = 200);

   	int GetValue(const char *command);

	int MAX_POS;       		// Maximum commanded position (positive or negative);
	int LIMIT_POS;     		// Positioning limit after which the program must take corrective action
	int SPEED_THRESHOLD; 	// Max. speed to be considered "not moving" (defaults to zero)
	int CRUISE_SPEED;  		// Default movement speed
	int ACCELERATION;  		// Default acceleration
	int HOMING_SPEED;  		// Default homing speed

	int VPROP;			// Velocity proportional gain
	int VINT; 			// Velocity integral gain
	int PPROP;			// Position proportional gain
	int PDERV;			// Position derivative gain
};

#endif
