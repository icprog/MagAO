//@File: Mercury.cpp
//
// Implementation of the Mercury motor class
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

using namespace std;

#include "Mercury.h"

Mercury::Mercury( AOApp *app, Config_File &cfg) : SimpleMotor( app, cfg)
{
   Logger::get()->log( Logger::LOG_LEV_TRACE, "Constructor: mercury");

   // Set default values
   this->LIMIT_POS = (int)3e8;
   this->MAX_POS   = this->LIMIT_POS /2;
   this->SPEED_THRESHOLD = 0;
   this->CRUISE_SPEED = 300;
   this->ACCELERATION = 100;
   this->HOMING_SPEED = cfg["HomingSpeed"];

   this->device = cfg["Device"];
}

int Mercury::TurnOn()
{
   int stat;


   // Test successful connection
   stat = TestNetwork();
   CHECK_SUCCESS(stat)

   GetValue("VE");
     
	return NO_ERROR;
}

int Mercury::TurnOff()
{
	return NO_ERROR;
}

int Mercury::Home()
{
   int stat;

   stat = SendCommand("SV%d", this->HOMING_SPEED);
   CHECK_SUCCESS(stat);

   stat = SendCommand("LN");
   CHECK_SUCCESS(stat);
   
   stat = SendCommand("FE2");
   CHECK_SUCCESS(stat);

   msleep(1000);
	return NO_ERROR;
}

int Mercury::MoveTo( double pos)
{
   char cmd[16];
   int stat;
   int steps = (int) PosToSteps(pos);

   // Motor on
   stat = SendCommand("MN");
   CHECK_SUCCESS(stat);

   Logger::get()->log( Logger::LOG_LEV_INFO, "Mercury: Moving to position %f", pos);

   // Move absolute
   sprintf(cmd, "MA%d", steps);
   stat = SendCommand(cmd);
   CHECK_SUCCESS(stat);

   return NO_ERROR;
}


double Mercury::GetPosition( int force)
{
   double pos = StepsToPos( GetValue("TP"));
   if (force)
      var_pos_cur.Set( pos, 0, FORCE_SEND);
   else
      var_pos_cur.Set(pos);

   return pos;
}	

int Mercury::IsMoving(void)
{
   // Uses the actual speed value (TV) to see if it is moving or not
   //int speed = GetValue("TV");
   //return (abs(speed) <= SPEED_THRESHOLD) ? 0 : 1;

   int pos1 = GetValue("TP");
   msleep(1000);
   int pos2 = GetValue("TP");
   Logger::get()->log( Logger::LOG_LEV_DEBUG, "IsMoving: %d - %d - moving %d", pos1, pos2);
   return (abs(pos1-pos2) > 1) ? 1 : 0;
}

int Mercury::Abort(void)
{
   // Disable & re-enable drive - leaves it at the current position
   int stat = SendCommand("MF");
   CHECK_SUCCESS(stat);

   return SendCommand("MN");
}

int Mercury::TestNetwork(void)
{
    int stat;
    char answer[64];

    Logger::get()->log( Logger::LOG_LEV_TRACE, "Mercury: testing network");

   // Address the controller over the serial bus
   char buffer[2];
   buffer[0] = 0x01;
   buffer[1] = this->device + 48;

   stat = SerialOut(buffer, 2);
   CHECK_SUCCESS(stat)

    // SendCommand() will lock the mutex by itself
    pthread_mutex_lock(&mutex);

    stat = SendCommand_nolock("TP");
    CHECK_SUCCESS(stat);

    stat = SerialInString( answer, 63, 1000, 10);
    pthread_mutex_unlock(&mutex);

    answer[63]=0;
    Logger::get()->log( Logger::LOG_LEV_TRACE, "TestNetwork answer: %s", answer);

    

    CHECK_SUCCESS(stat);

    // Check if there actually was an answer
    if (stat == 0)
       return COMMUNICATION_ERROR;

    // We want a number as answer
    if ((atoi(answer+3) == 0) && (answer[3] != '0'))
       return COMMUNICATION_ERROR;

    return NO_ERROR;
}


int Mercury::SendCommand(const char *fmt, ...)
{
	va_list argp;
	char command[64];
	char ending[2];
   int stat;

	ending[0] = 13;
	ending[1] = 0;

	va_start( argp, fmt);
	vsprintf( command, fmt, argp);
	va_end(argp);

	strcat( command, ending);

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Sending %s", command);

   pthread_mutex_lock(&mutex);
	stat = SerialOut( command, strlen(command));
   pthread_mutex_unlock(&mutex);

   return stat;
}

int Mercury::SendCommand_nolock(const char *fmt, ...)
{
	va_list argp;
	char command[64];
	char ending[2];
   int stat;

	ending[0] = 13;
	ending[1] = 0;

	va_start( argp, fmt);
	vsprintf( command, fmt, argp);
	va_end(argp);

	strcat( command, ending);
   Logger::get()->log( Logger::LOG_LEV_TRACE, "Sending %s", command);

	stat = SerialOut( command, strlen(command));

   return stat;
}

int Mercury::GetValue(const char *command)
{
	int stat;
	char answer[256];

   pthread_mutex_lock(&mutex);

	stat = SendCommand_nolock(command);

   if (stat >= 0)
      stat = SerialInString( answer, 256, 1000, 10);

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Received: %s", answer);

   pthread_mutex_unlock(&mutex);

   if (stat < 0)
      return stat;

   return atoi(answer+2);
}

int Mercury::GetTemperature(void)
{
   return 0;
}

int Mercury::EndHoming()
{
   return SendCommand("DH");
}
