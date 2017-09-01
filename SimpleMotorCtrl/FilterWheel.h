//@File: FilterWheel.h
//
// Filter wheel class, derived from SimpleMotor
//
//@

#ifndef FILTERWHEEL_H_INCLUDED
#define FILTERWHEEL_H_INCLUDED

#include "MCBL2805.h"

class FilterWheel: public MCBL2805
{
public:
	FilterWheel( AOApp *app, Config_File &cfg);

	int TurnOn(void);          // Wheel-specific setup
   int MoveTo( double pos);   // Overloaded because of unidirectional motion
	virtual int Home(void);            // Wheel-specific homing

protected:
   float32 CIRCLE_STEPS;          // Number of encoder steps for a complete circle

   int unidirectional;        // Unidirectional flag: if set, it only moves forward

   int homing;                // Homing flag (used to disable the digital inputs after homing)
};

#endif  //FILTERWHEEL_H_INCLUDED
	
