//@File: ADCWheel.h
//
// ADC wheel class, derived from filterwheel
//
// It's a filter wheel with specialized homing
//
//
//@

#ifndef ADCWHEEL_H_INCLUDED
#define ADCWHEEL_H_INCLUDED

#include "FilterWheel.h"

class ADCWheel: public FilterWheel
{
public:
	ADCWheel( AOApp *app, Config_File &cfg);

   float CmdToDeg( float cmd);
   float DegToCmd( float deg);

//   int MoveTo( double pos);   // Overloaded because of unidirectional motion
	int Home(void);            // Wheel-specific homing

};

#endif  // ADCWHEEL_H_INCLUDED
	
