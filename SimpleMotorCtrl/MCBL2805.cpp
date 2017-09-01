//@File: MCBL2805.cpp
//
// Implementation of the MCBL2805 motor class
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
#include <cmath>

using namespace std;

#include "MCBL2805.h"
#include "Logger.h"

MCBL2805::MCBL2805( AOApp *app, Config_File &cfg) : SimpleMotor( app, cfg)
{
   Logger::get()->log( Logger::LOG_LEV_DEBUG, "MCBL2805 constructor");

	// Set default values
	this->LIMIT_POS = (int)3e8;
	this->MAX_POS   = this->LIMIT_POS /2;
	this->SPEED_THRESHOLD = 0;
	this->CRUISE_SPEED = 1000;
	this->ACCELERATION = 100;
	this->HOMING_SPEED = 1000;
}

int MCBL2805::TurnOn()
{
	int stat;

	// RS232 control
	stat = GetValue("SOR0");		// Will answer "OK"

	// Deactivate asynchronous stuff
	stat = SendCommand("ANSW0");
	CHECK_SUCCESS(stat);

	// Ensure a wide range of movement
	int range = (int)this->MAX_POS;
	char cmd[20];

	sprintf(cmd, "LL%d",range);
	stat = SendCommand(cmd);
	CHECK_SUCCESS(stat);

	sprintf(cmd, "LL-%d", range);
	stat = SendCommand(cmd);
	CHECK_SUCCESS(stat);

	return NO_ERROR;
}

int MCBL2805::TurnOff()
{
	return NO_ERROR;
}

int MCBL2805::Home()
{
	return NO_ERROR;
}

int MCBL2805::MoveTo( double pos)
{
	char cmd[16];
	int stat;
	int steps = (int) PosToSteps(pos);

   Logger::get()->log( Logger::LOG_LEV_INFO, "MCBL2805: Moving to position %f", pos);

	// Move absolute
	sprintf(cmd, "LA%d", steps);
	stat = SendCommand(cmd);
	CHECK_SUCCESS(stat);

	// Enable motor (it may have been disabled by a failed abort command)
	stat = SendCommand("EN");
	CHECK_SUCCESS(stat);

	return SendCommand("M");
}


double MCBL2805::GetPosition( int force)
{
	double pos = StepsToPos( GetValue("POS"));

   //If force < 0 we don't update RTDB
   if (force > 0)
	   var_pos_cur.Set(floor(pos*1000.+.5)/1000., 0, FORCE_SEND);
   else if (force == 0)
      var_pos_cur.Set(floor(pos*1000.+.5)/1000.);

	return pos;
}	

int MCBL2805::IsMoving(void)
{
	// Uses the actual speed value (GN) to see if it is moving or not
	int speed = GetValue("GN");
	return (abs(speed) <= SPEED_THRESHOLD) ? 0 : 1;
}

int MCBL2805::Abort(void)
{
	return Disable();
}

int MCBL2805::Disable(void)
{
	int stat = SendCommand("DI");
        if (!IS_ERROR(stat)) 
           Logger::get()->log( Logger::LOG_LEV_INFO, "MCBL2805: Drive disabled");
        else
           Logger::get()->log( Logger::LOG_LEV_INFO, "MCBL2805: Error disabling drive: (%d) %s", stat, lao_strerror(stat));

        return stat;
}

int MCBL2805::TestNetwork(void)
{
	int stat;
    char answer[64];

    Logger::get()->log( Logger::LOG_LEV_TRACE, "MCBL2805: Testing network, network_ok=%d", network_ok);
    if (!network_ok)
       SetupNetwork();

    pthread_mutex_lock(&mutex);
    // SendCommand() will lock the mutex by itself
    stat = SendCommand_nolock("POS");
    if (stat<0)
         {
     	 Logger::get()->log( Logger::LOG_LEV_TRACE, "MCBL2805: Error sending test command");
         pthread_mutex_unlock(&mutex);
         return stat;
         }

    stat = SerialInString( answer, 63, 1000, 10);
    pthread_mutex_unlock(&mutex);
    CHECK_SUCCESS(stat);

    // Check if there actually was an answer
    if (stat == 0)
	{
       Logger::get()->log( Logger::LOG_LEV_TRACE, "MCBL2805: No answer to test command");
       return COMMUNICATION_ERROR;
	}

    // We want a number as answer
    if ((atoi(answer) == 0) && (answer[0] != '0'))
	{
	Logger::get()->log( Logger::LOG_LEV_TRACE, "MCBL2805: Wrong answer to test command (%s)", answer);
	EmptySerial(0, 200);
	return COMMUNICATION_ERROR;
	}

    Logger::get()->log( Logger::LOG_LEV_TRACE, "Network test OK - answer %s", answer);
    return NO_ERROR;
}


int MCBL2805::SendCommand(const char *fmt, ...)
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
        //printf("%s\n", command);

  	pthread_mutex_lock(&mutex);
	stat = SerialOut( command, strlen(command));
  	pthread_mutex_unlock(&mutex);

  	return stat;
}

int MCBL2805::SendCommand_nolock(const char *fmt, ...)
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

int MCBL2805::GetValue(const char *command)
{
	int stat;
	char answer[64];
	answer[0] =0;

   pthread_mutex_lock(&mutex);

	stat = SendCommand_nolock(command);

   if (stat >= 0)
      stat = SerialInString( answer, 64, 1000, 10);

   pthread_mutex_unlock(&mutex);

   if (stat < 0)
      return stat;

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Received: %s", answer);

   return atoi(answer);
}

int MCBL2805::GetTemperature(void)
{
   	return GetValue("TEM");
}

int MCBL2805::GetSwitchStatus(void)
{
    int status = (GetValue("GAST")==110) ? 1 : 0;
    var_sw_status.Set(status);

    return status;
}

int MCBL2805::EmptySerial( int avoidLock, int timeout)
{
   char buffer[1024];

   if (!avoidLock)
      pthread_mutex_lock( &mutex);

   int n;
   while( (n= SerialInString( buffer, 1024-1, timeout, 0x0A)) > 0)
   	Logger::get()->log( Logger::LOG_LEV_TRACE, "EmptySerial(): received string: %s", buffer);

   Logger::get()->log( Logger::LOG_LEV_TRACE, "EmptySerial(): exiting");

   if (!avoidLock)
      pthread_mutex_unlock( &mutex);

     return NO_ERROR;
}

