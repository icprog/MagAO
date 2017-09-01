//@File: ADCWheel.cpp
//
// Implementation of the ADCWheel class
//
//@

extern "C" {

#include "base/errlib.h"
#include "base/timelib.h"
#include "hwlib/netseriallib.h"
}

#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <cstdlib>

//#include <unistd.h>   // usleep()

using namespace std;

#include "ADCWheel.h"
#include "Logger.h"

ADCWheel::ADCWheel( AOApp *app, Config_File &cfg) : FilterWheel( app, cfg)
{
   Logger::get()->log( Logger::LOG_LEV_DEBUG, "ADCWheel constructor");
}

// Conversion from degrees to command

float ADCWheel::DegToCmd( float deg)
{
   float cmd = (deg / 360.0) * (this->CIRCLE_STEPS / this->steps_per_pos);
   Logger::get()->log( Logger::LOG_LEV_DEBUG, "Circle steps: %f - steps_per_pos: %f", this->CIRCLE_STEPS, this->steps_per_pos);
   Logger::get()->log( Logger::LOG_LEV_DEBUG, "ADCWheel::DegToCmd(): %f -> %f", deg, cmd);
   return cmd;
}

float ADCWheel::CmdToDeg( float cmd)
{
   float deg = (cmd /  (this->CIRCLE_STEPS / this->steps_per_pos)) * 360.0;
   Logger::get()->log( Logger::LOG_LEV_DEBUG, "Circle steps: %f - steps_per_pos: %f", this->CIRCLE_STEPS, this->steps_per_pos);
   Logger::get()->log( Logger::LOG_LEV_DEBUG, "ADCWheel::CmdToDeg(): %f -> %f", cmd, deg);
   return deg;
}

// ADC homing is complicated by the limit switch behaviour:
//      ___      ___
// ____/ A \_B__/ C \_____ ----> positive encoder direction
//
// Two high levels with a low level in the middle. Approximate dimensions
// for A,B and C in degrees are 40, 20 and 55 respectively. For the rest of the circle,
// the limit switch is always low (about 245 degrees).
//
// The procedure is to turn the wheel freely, sampling the current position and limit switch status,
// until segment B is found. Then the wheel is turned forward 180 and a homing is performed, which
// will find the last edge of C.



int ADCWheel::Home()
{
   enum seq_stats {
        SEQ_START = 0,
        SEQ_FALLING_EDGE_FOUND=1,
        SEQ_RISING_EDGE_FOUND=2
   };

   float pos = GetPosition();
   float falling_pos = 0;
   float rising_pos  = 0;
   int prev_sw = GetSwitchStatus();
   int seq_status = SEQ_START;

   // Give a command for a circle+50%
   Logger::get()->log( Logger::LOG_LEV_DEBUG, "ADCWheel::Home(): starting search movement...");
   MoveTo(pos + DegToCmd(360) * 2);

   int found=0;
   while(!found)
        {
        int cur_sw = GetSwitchStatus();
        pos = GetPosition();

           //Debug code to sample edge behaviour
           /* 
           string edge="";
           if ((prev_sw ==1) && (cur_sw ==0))
             edge="falling";
           if ((prev_sw ==0) && (cur_sw ==1))
             edge="raising";
           if (edge != "")
                Logger::get()->log( Logger::LOG_LEV_DEBUG, "pos %5.2f, edge %s", pos, edge.c_str());
           prev_sw = cur_sw;
           continue;
           */
          


        switch(seq_status)
         {
            // Search first for a falling edge
            case SEQ_START:
            if ((prev_sw == 1) && (cur_sw == 0))
                {
                Logger::get()->log( Logger::LOG_LEV_DEBUG, "ADCWheel::Home(): Found falling edge at position %5.2f", pos);
                seq_status = SEQ_FALLING_EDGE_FOUND;
                falling_pos = pos;
                }
            break;

            // Then or a rising edge
            case SEQ_FALLING_EDGE_FOUND:
            if ((prev_sw == 0) && (cur_sw == 1))
               {
               Logger::get()->log( Logger::LOG_LEV_DEBUG, "ADCWheel::Home(): Found rising edge at position %5.2f", pos);
               seq_status = SEQ_RISING_EDGE_FOUND;
               rising_pos = pos;

               // If we are here, we've got either B or the rest of the circle

               float dim = rising_pos - falling_pos;

               Logger::get()->log( Logger::LOG_LEV_DEBUG, "ADCWheel::Home(): Low zone is %5.2f degrees wide", CmdToDeg(dim));

               // If the space between fallind and rising was too big, it's not B. Start over.
               if (CmdToDeg(dim) > 40)
                  {
                  seq_status = SEQ_START;
                  continue;
                  }

                // Otherwise, we found B.
                Logger::get()->log( Logger::LOG_LEV_DEBUG, "ADCWheel::Home(): Low zone accepted!");
                found=1;
                break;
                }
            }

        prev_sw = cur_sw;
        }


    // Go forward 180 to be *sure* of being after C

    Logger::get()->log( Logger::LOG_LEV_DEBUG, "ADCWheel::Home(): Going forward 180 degrees");
    this->Abort();
    msleep(1000);
    this->MoveTo( GetPosition() +DegToCmd(180));
    msleep(2000);

    while( this->IsMoving())
       {
       GetPosition();     // Update position display
       GetSwitchStatus();

       msleep(200);
       }


   Logger::get()->log( Logger::LOG_LEV_DEBUG, "ADCWheel::Home(): Starting normal homing");

   //homing = 0;
   //return NO_ERROR;

   // Homing usando l'input no. 3 (corrispondente al bit 3, valore=4):
 
   char cmd[16];
   sprintf(cmd, "HOSP%d", this->HOMING_SPEED);

   CHECK_SUCCESS( SendCommand("ha4"));
   CHECK_SUCCESS( SendCommand("hl4"));
   CHECK_SUCCESS( SendCommand("cahoseq"));
   CHECK_SUCCESS( SendCommand("hp0"));
   CHECK_SUCCESS( SendCommand(cmd));
   CHECK_SUCCESS( SendCommand("gohoseq"));

   homing = 1;
   msleep(1000);
	return NO_ERROR;
}


