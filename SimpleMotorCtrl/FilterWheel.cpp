//@File: FilterWheel.cpp
//
// Implementation of the FilterWheel class
//
//@

extern "C" {

#include "base/errlib.h"
#include "hwlib/netseriallib.h"
}

#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <cstdlib>

using namespace std;

#include "FilterWheel.h"
#include "Logger.h"

FilterWheel::FilterWheel( AOApp *app, Config_File &cfg) : MCBL2805( app, cfg)
{
   Logger::get()->log( Logger::LOG_LEV_DEBUG, "Filterwheel constructor");
   this->CIRCLE_STEPS = cfg["CircleSteps"];
   this->CRUISE_SPEED = cfg["CruiseSpeed"];
   this->ACCELERATION = cfg["Acceleration"];
   this->HOMING_SPEED = cfg["HomingSpeed"];
   this->VPROP = cfg["VelocityProportional"];
   this->VINT = cfg["VelocityIntegral"];
   this->PPROP = cfg["PositionProportional"];
   this->PDERV = cfg["PositionDerivative"];
   
   this->unidirectional = cfg["Unidirectional"];

   homing=0;
}

int FilterWheel::TurnOn()
{
   int stat, value;
   int motor_type = 2;

   char cmd[16];

   // Perform general MCBL setup
   MCBL2805::TurnOn();

   // Change moving threshold
   this->SPEED_THRESHOLD = 10;

   // Set motor type
   sprintf(cmd, "MOTTYP%d", motor_type);

   stat = SendCommand(cmd);
   CHECK_SUCCESS(stat);

   value = GetValue("GMOTTYP");
   if (value != motor_type)
      return OBJECT_NOT_ENABLED_ERROR;

   // Set controller gains
   sprintf(cmd, "POR%d", this->VPROP);
   stat = SendCommand(cmd);
   CHECK_SUCCESS(stat);
   sprintf(cmd, "PD%d", this->PDERV);
   stat = SendCommand(cmd);
   CHECK_SUCCESS(stat);
   sprintf(cmd, "PP%d", this->PPROP);
   stat = SendCommand(cmd);
   CHECK_SUCCESS(stat);
   sprintf(cmd, "I%d", this->VINT);
   stat = SendCommand(cmd);
   CHECK_SUCCESS(stat);

   // Set acceleration
   sprintf(cmd, "AC%d", this->ACCELERATION);
   stat = SendCommand(cmd);
   CHECK_SUCCESS(stat);
   value = GetValue("GAC");
   if (value != this->ACCELERATION)
      return OBJECT_NOT_ENABLED_ERROR;

   // Set speed
   sprintf(cmd, "SP%d", this->CRUISE_SPEED);
   stat = SendCommand(cmd);
   CHECK_SUCCESS(stat);
   value = GetValue("GSP");
   if (value != this->CRUISE_SPEED)
      return OBJECT_NOT_ENABLED_ERROR;

   // Move to position zero at start
   CHECK_SUCCESS( SendCommand("EN"));
   CHECK_SUCCESS( SendCommand("LA0"));
   CHECK_SUCCESS( SendCommand("M"));

   return NO_ERROR;
}

int FilterWheel::Home()
{

   // Homing usando l'input no. 3 (corrispondente al bit 3, valore=4):
 
   char cmd[16];
   sprintf(cmd, "HOSP%d", this->HOMING_SPEED);

   CHECK_SUCCESS( SendCommand("EN"));
   CHECK_SUCCESS( SendCommand("ha4"));
   CHECK_SUCCESS( SendCommand("hl4"));
   CHECK_SUCCESS( SendCommand("cahoseq"));
   CHECK_SUCCESS( SendCommand("hp0"));
   CHECK_SUCCESS( SendCommand(cmd));
   CHECK_SUCCESS( SendCommand("gohoseq"));

   homing = 1;
	return NO_ERROR;
}

// Move to the specified position. Takes care of calculating the right
// movement direction depending if the filterwheel is unidirectional or bidirectional.

int FilterWheel::MoveTo(double pos)
{
   int stat, newpos;
   char cmd[20];
  
   // Disable digital inputs if needed
   if (homing)
      {
      CHECK_SUCCESS( SendCommand("ha0"));
      CHECK_SUCCESS( SendCommand("hl0"));
      homing=0;
      }

   // Get current raw position
   int curpos = GetValue("POS");

   if ((curpos > LIMIT_POS) || (curpos < -LIMIT_POS))
      {
      Home();
      curpos = 0;
      }

   Logger::get()->log( Logger::LOG_LEV_TRACE, "FilterWheel::MoveTo(): curpos is %d", curpos);

   if (unidirectional)
      {  
      // Get a relative movement from the current position
      curpos %= (int) CIRCLE_STEPS;
      newpos = (int)PosToSteps(pos) - curpos;

      Logger::get()->log( Logger::LOG_LEV_TRACE, "FilterWheel::MoveTo(): newpos (relative) is %d", newpos);

      // Remap to the next positive movement
      while (newpos<0)
         newpos += CIRCLE_STEPS;

      Logger::get()->log( Logger::LOG_LEV_TRACE, "FilterWheel::MoveTo(): newpos (unidirectional relative) is %d", newpos);
      }
   else
      {
      // If bidirectional, just compute the relative movement
      newpos = (int)PosToSteps(pos) - curpos;
      }


   // Enable drive (it may have been disabled by a failed abort command)
   stat = SendCommand("EN");
   CHECK_SUCCESS(stat);

   // Go
   Logger::get()->log( Logger::LOG_LEV_INFO, "FilterWheel::MoveTo(): moving to position %f", pos);
   sprintf(cmd, "LR%d", newpos);
   stat = SendCommand(cmd);
   CHECK_SUCCESS(stat);

   return SendCommand("M");
}


