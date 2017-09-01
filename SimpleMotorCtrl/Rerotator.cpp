//@File: Rerotator.cpp
//
// Implementation of the Rerotator class
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

//#include <unistd.h>  // usleep()



#include "AOStates.h"

using namespace std;

#include "Rerotator.h"

static int verbose=0;

Rerotator::Rerotator( AOApp *app, Config_File &cfg) : SimpleMotor( app, cfg)
{
   this->speed = cfg["Speed"];
   this->homingSpeed = cfg["HomingSpeed"];
   this->accel = cfg["Accel"];
   this->nlimits = cfg["nlimits"];
   this->homingType = (std::string)cfg["HomingType"];
}

int Rerotator::TurnOn()
{

        char answer[128];
        memset( answer, 0, 128);
                                                                                             
        // Clean serial line: when cold-starting the Pollux,
        // the seria line gets a hundred bytes or so of garbage!!!
        //CHECK_SUCCESS( SendCommand(1, "1 nreset"));
        //CHECK_SUCCESS( SendCommand(1, "-1.0 1 setnpos"));
        //while (GetPosition() != 1.0)
         // {}

        Logger::get()->log( Logger::LOG_LEV_DEBUG, "Waiting for controller startup (5 secs)");
        msleep(5000);

        if (this->nlimits == 1)
            { 
            CHECK_SUCCESS( SendCommand(1, "1 0 1 setsw"));
            CHECK_SUCCESS( SendCommand(1, "0 1 1 setsw"));
            CHECK_SUCCESS( SendCommand(1, "1 nsave"));
            msleep(1000);
            }
        else if (this->nlimits == 2)
            {
            CHECK_SUCCESS( SendCommand(1, "1 0 1 setsw"));
            CHECK_SUCCESS( SendCommand(1, "1 1 1 setsw"));
            CHECK_SUCCESS( SendCommand(1, "1 nsave"));
            msleep(1000);
            }

        CHECK_SUCCESS( SendCommand(1, "1 nreset"));
        msleep(1000);

        CHECK_SUCCESS( SendCommand(1, "2.000 1 setpitch"));        // Set motor pitch
        CHECK_SUCCESS( SendCommand(1, "%5.2f 1 snv", this->speed));         // Set speed to 24 mm/s
        CHECK_SUCCESS( SendCommand(1, "%5.2f 1 sna", this->accel));           // Set acceleration to 200 mm/s^2
        CHECK_SUCCESS( SendCommand(1, "%5.2f 1 1 setncalvel", this->homingSpeed)); // Set homing speed #1
        CHECK_SUCCESS( SendCommand(1, "0.2 2 1 setncalvel")); 
        CHECK_SUCCESS( SendCommand(1, "%5.2f 1 1 setnrmvel", this->homingSpeed));
        CHECK_SUCCESS( SendCommand(1, "0.2 2 1 setnrmvel"));
        CHECK_SUCCESS( SendCommand(1, "3500 1 setumotmin"));
        CHECK_SUCCESS( SendCommand(1, "140 1 setumotgrad"));
        CHECK_SUCCESS( SendCommand(1, "-1000.000000 1000.000000 1 setnlimit")); // Set movement limits

        CHECK_SUCCESS( SendCommand(1, "0.000000 1 setnpos"));

        msleep(1000);
        GetPosition();
        GetPosition();
        CHECK_SUCCESS( SendCommand(1, "0.000000 1 setnpos"));
        GetPosition();
        GetPosition();
        //IsMoving();
        GetPosition();
        GetPosition();

     

	return NO_ERROR;
}

int Rerotator::TurnOff()
{
	return NO_ERROR;
}

int Rerotator::Home()
{
	int stat = SendCommand(1, "1 %s", this->homingType.c_str());
   msleep(1000);      // To avoid controller not responding anymore
   return stat;
}

int Rerotator::MoveTo( double pos)
{
   if (pos > MVmax)
      pos = MVmax;
   if (pos < MVmin)
      pos = MVmin;

   Logger::get()->log( Logger::LOG_LEV_INFO, "Moving to position %f", pos);

	double position = PosToSteps(pos);

   return SendCommand( 1, "%9.3f 1 nm", position);
}

double Rerotator::GetPosition( int force)
{
	char answer[64];
   double pos;
                      
   pthread_mutex_lock(&mutex);

   memset( answer, 0, 64);
   SendCommand(0, "1 np");
   SerialInString( answer, 63, 300, 10);
   Logger::get()->log( Logger::LOG_LEV_TRACE, "Answer -->%s", answer);

   pthread_mutex_unlock(&mutex);

   if (verbose)
         printf("Got answer: %s\n", answer);
                                                                                               
   pos = StepsToPos(atof(answer));
   if (force)
      var_pos_cur.Set( pos, 0, FORCE_SEND);
   else
      var_pos_cur.Set(pos);
   if (verbose)
      printf("var_curpos set to %5.2f\n", pos);
   
   return pos;
}


int Rerotator::IsMoving(void)
{
    char answer[64];
    int byte, result;

        memset( answer, 0, 64);
        if (status == STATE_ERROR)
                return 0;
                      
        pthread_mutex_lock(&mutex);

        SendCommand( 0, "1 nstatus");
        SerialInString( answer, 63, 5000, 10);

        pthread_mutex_unlock(&mutex);

        if (verbose)
                printf("Got answer: %s\n", answer);
        byte = atoi(answer);

        if ( byte & 0x01)
                {
                if (verbose)
                        printf("Moving\n");
                result= 1;
                }
        else
                result =0;
                                                                                               
	return result;
}

int Rerotator::Abort(void)
{
        return SendCommand( 1, "1 nabort");
}

int Rerotator::SendCommand(const char *fmt, ...)
{
	va_list argp;
	char command[64];
	char ending[4];

	ending[0] = 32;
	ending[1] = 13;
	ending[2] = 0;
	ending[3] = 0;

	va_start( argp, fmt);
	vsprintf( command, fmt, argp);
	va_end(argp);

	strcat( command, ending);

   pthread_mutex_lock(&mutex);


   char *c = command;
   while (c[0] == ' ')
      c++;
   Logger::get()->log( Logger::LOG_LEV_TRACE, "Sending -->%s", c);
   int stat= SerialOut( c, strlen(c));
   //int stat= SerialOut( command, strlen(command));

   msleep(200);

   pthread_mutex_unlock(&mutex);


   return stat;
}

int Rerotator::SendCommand( int lock,const char *fmt, ...)
{
	va_list argp;
	char command[64];
	char ending[4];

	ending[0] = 32;
	ending[1] = 13;
	ending[2] = 0;
	ending[3] = 0;

	va_start( argp, fmt);
	vsprintf( command, fmt, argp);
	va_end(argp);

	strcat( command, ending);

   if (lock)
      pthread_mutex_lock(&mutex);

   char *c = command;
   while (c[0] == ' ')
      c++;
   Logger::get()->log( Logger::LOG_LEV_TRACE, "Sending -->%s", c);
   int stat= SerialOut( c, strlen(c));

   //Logger::get()->log( Logger::LOG_LEV_TRACE, "Sending %s", command);
   //int stat = SerialOut( command, strlen(command));

   msleep(100);
   if (lock)
      pthread_mutex_unlock(&mutex);

   return stat;
}

int Rerotator::TestNetwork(void)
{
    char answer[64];
    //double pos;
    int stat, num;

    pthread_mutex_lock(&mutex);

    memset( answer, 0, 64);
    stat = SendCommand(0, "1 np");
    if (stat == NO_ERROR)
        num = SerialInString( answer, 63, 1000, 10);

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Answer -->%s", answer);
    pthread_mutex_unlock(&mutex);

    if (num == 0) return COMMUNICATION_ERROR;

    return stat;
}

int Rerotator::EndHoming()
{
   SendCommand(1, "0.000000 1 setnpos");
   GetPosition();
   return NO_ERROR;
}
