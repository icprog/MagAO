//@File: Mercury.h
//
// Class for the Mercury controller, derived from the SimpleMotor class
//
//@

#ifndef MERCURY_H_INCLUDED
#define MERCURY_H_INCLUDED

#include "SimpleMotor.h"

class Mercury: public SimpleMotor
{
public:
   Mercury( AOApp *app, Config_File &cfg);

	int TurnOn(void);     // This is virtual to allow setting the motor type
	int TurnOff(void);

	virtual int Home(void);
	virtual int MoveTo(double pos);
	double GetPosition( int force = false);

	int IsMoving(void);
	int Abort(void);
   int TestNetwork(void);     // Test network connection

   int EndHoming(void);

   int GetTemperature(void);

protected:
	int SendCommand(const char *fmt, ...);
	int SendCommand_nolock(const char *fmt, ...);

   int GetValue(const char *command);

   int device;        // Mercury device address (0-15)

   int MAX_POS;       // Maximum commanded position (positive or negative);
   int LIMIT_POS;     // Positioning limit after which the program must take corrective action
   int SPEED_THRESHOLD; // Max. speed to be considered "not moving" (defaults to zero)
   int CRUISE_SPEED;  // Default movement speed
   int ACCELERATION;  // Default acceleration
   int HOMING_SPEED;  // Default homing speed
};

#endif
