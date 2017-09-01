//@File: Rerotator.h
//
// Rerotator class, derived from SimpleMotor
//
//@

#include "SimpleMotor.h"

class Rerotator: public SimpleMotor
{
public:
   Rerotator( AOApp *app, Config_File &cfg);

	int TurnOn(void);
	int TurnOff(void);

	int Home(void);
	int MoveTo(double pos);
	double GetPosition( int force=false);

	int IsMoving(void);
	int Abort(void);

   int TestNetwork(void);

   int EndHoming(void);

protected:
	int SendCommand(const char *fmt, ...);
	int SendCommand( int lock,const char *fmt, ...);

   float speed;
   float homingSpeed;
   float accel;
   int nlimits;

   std::string homingType;
};
	
