#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#include "copleylib.h"
#include "netseriallib.h"
#include "base/errordb.h"

#define BUFLEN (256)

static int debug=0;
static int errflag=0;
static int init_ok=0;

// Error checking macro
//
#define CHECK(a)   { int mystat; if ((mystat = (a)) != NO_ERROR) { return mystat; }}

pthread_mutex_t serialMutex = PTHREAD_MUTEX_INITIALIZER;

void handle_sigpipe(int);

void handle_sigpipe(int a)
{
   a=0;  // Avoid warning
   init_ok=0;
   errflag=1;
}

int copleyInit( char *addr, int port)
{
   int stat;

   SerialClose();
   pthread_mutex_init( &serialMutex, NULL);

  signal( SIGPIPE, handle_sigpipe);

   stat = SerialInit( addr, port);
   if (stat == NO_ERROR)
      init_ok = 1;
   if (debug) printf("copleyInit(): init_ok set to %d\n", init_ok);
   return stat;
}

int copleyShutdown()
{
   if (debug) printf("copleyShutdown: closing communication\n");
   SerialClose();
   init_ok = 0;
   return NO_ERROR;
}

int copleyBinaryCmd( char *buf, int buflen, char *answbuf, int answbuflen, int timeout)
{
   int i;

   if (!init_ok)
      return NOT_INIT_ERROR;

   if (debug) {
      printf("Sending:");
      for (i=0; i<buflen; i++)
         printf(" %02X", buf[i]);
      printf("\n");
   }

  pthread_mutex_lock( &serialMutex);

   SerialOut(buf, buflen);
   SerialIn(answbuf, answbuflen, timeout);

  pthread_mutex_unlock( &serialMutex);

   if (debug) {
      printf("Received:");
      for (i=0; i<answbuflen; i++)
         printf(" %02X", answbuf[i]);
      printf("\n");
   }

   return NO_ERROR;
}

// Set value to NULL if you do not expect an answer
int copleyCmd( int *value, char *fmt, ...)
{
	char inbuf[BUFLEN];
   char outbuf[BUFLEN];
   	va_list vl;
	int len;

   if (debug) printf("copleyCmd: init_ok is %d\n", init_ok);

   if (!init_ok)
      return NOT_INIT_ERROR;


	memset(outbuf, 0, BUFLEN);	
	va_start( vl, fmt);
	vsnprintf(outbuf, BUFLEN, fmt, vl);
	va_end(vl);
	outbuf[BUFLEN-1] = 0;
	
	if (value)
		*value = 0;

	if (debug>=2) printf("Command: %s\n", outbuf);

  pthread_mutex_lock( &serialMutex);

   errflag = 0;   
  
	SerialOut(outbuf, strlen(outbuf));

   // Catch SIGPIPE
   if (errflag)
      {
      init_ok=0;
      return COMMUNICATION_ERROR;
      }

	len = SerialInString(inbuf, BUFLEN, 1000, '\r');
	inbuf[len] =0;

   pthread_mutex_unlock( &serialMutex);
	
	if (debug>=2) printf("Answer:  %s\n", inbuf);

	if (strncmp(inbuf,"ok",2) == 0)
		return NO_ERROR;
	
	if (((inbuf[0] == 'v') || (inbuf[0] == 'r')) && (value))
		{
		*value = atoi(inbuf+2);
		return NO_ERROR;
		}
	
	if (inbuf[0] == 'e')
		{
      if (debug) {
         printf("----------- Error answer from copley\n");
         printf("\n");
         printf("Command: %s\n", outbuf);
         printf("Answer: %s\n", inbuf);
         printf("\n");
      }
		int error = atoi(inbuf+2);
		return COPLEY_ERROR - error;
		}

   init_ok = 0;   
	return COMMUNICATION_ERROR;
}
		
	

int copleySet( char bank, int id, int value)
{
   // If writing to Flash, check existing value to avoid unnecessary writes
   if (bank == 'f')
      {
      int oldvalue;
      CHECK( copleyGet( bank, id, &oldvalue));
      if (value == oldvalue)
         return NO_ERROR;
      }

   // bank 'F' is write to Flash skipping the previous check
   if (bank == 'F')
      bank = 'f';

	return copleyCmd( NULL, "s %c%d %d\r", bank, id, value);
}

int copleyGet( char bank, int id, int *value)
{
	return copleyCmd( value, "g %c%d\r", bank, id);
}

int copleyCopy( char bank, int id)
{
	return copleyCmd( NULL, "c %c%d\r", bank, id);
}

int copleyReset()
{
	// Will return communication error - intended behaviour 'cos the communication is reset
	return copleyCmd( NULL, "r\r");
}

int copleyTrajectory( int subcommand)
{
	if (debug) printf("Trajectory command: %d\n", subcommand);
	return copleyCmd( NULL, "t %d\r", subcommand);
}

int copleyReadRegister( int regnum, int *value)
{
	return copleyCmd( value, "i %d\r", regnum);
}

int copleyWriteRegister( int regnum, int value)
{
	return copleyCmd( NULL, "i %d %d\r", regnum, value);
}

int copleyRegisterBit( int regnum, int bit) 
{
	int status;
	CHECK( copleyGet( 'r', regnum, &status))

	return isBitSet( status, bit);	
}

int isBitSet( int value, int bit)
{
	return (value & (1<< bit)) ? 1 : 0;
}

int isBitNotSet( int value, int bit)
{
	return (value & (1<< bit)) ? 0 : 1;
}

int copleyEventStatusBit( int bit)
{
	return copleyRegisterBit( 0xA0, bit);
}

/*
void dump( int value)
{
   int i;
   for ( i=0; i<32; i++)
      printf("Bit %d: %d\n", i, isBitSet(value, i));
}
*/

int copleyTrajectoryStatusBit( int bit)
{
	int value;
	CHECK( copleyGet('r', 0xC9, &value));
	if (debug) printf("Trajectory status bit: %d\n", (unsigned int)value);

	return copleyRegisterBit( 0xC9, bit);
}

int copleyFaultStatusBit( int bit)
{
	return copleyRegisterBit( 0xA4, bit);
}





// ---------- Sligthly higher-level...

int copleyMove()
{
	if (debug) printf("Move command\n");
	return copleyTrajectory(1);
}

int copleyAbort()
{
	if (debug) printf("Abort command\n");
	return copleyTrajectory(0);
}

int copleyHome()
{
	if (debug) printf("Home command\n");
	return copleyTrajectory(2);
}

int copleyMoveTo( int pos)
{
   copleySetMoveTarget(pos);
   return copleyMove();
}

int copleySetAbsolutePositioning()
{
	if (debug) printf("Set absolute positioning\n");
	return copleySet('r', 0xC8, 0);
}
int copleySetRelativePositioning()
{
	if (debug) printf("Set relative positioning\n");
	return copleySet('r', 0xC8, 256);
}

int copleySetMoveTarget( int pos)
{
	if (debug) printf("Set move target: %d\n", pos);
	return copleySet('r', 0xCA, pos);
}

int copleySetMoveSpeed( int speed)
{
	if (debug) printf("Set move speed: %d\n", speed);
	return copleySet('r', 0xCB, speed*10);
}

int copleySetAcceleration( int acc)
{
	if (debug) printf("Set acceleration: %d\n", acc);
	return copleySet('r', 0xCC, acc/10);
}

int copleySetDeceleration( int dec)
{
	if (debug) printf("Set deceleration: %d\n", dec);
	return copleySet('r', 0xCD, dec/10);
}

int copleySetAbortDeceleration( int dec)
{
	if (debug) printf("Set abort deceleration: %d\n", dec);
	return copleySet('r', 0xCF, dec/10);
}

int copleyEnableInPositioningMode()
{
	if (debug) printf("Enable in positioning mode\n");
	return copleySet('r', 0x24, 21);
}


int copleyDisable()
{
	if (debug) printf("Disable\n");
	//return copleySet('r', 0x24, 0);

   char buf[8];
   buf[0] = 0x00;
   buf[1] = 0x71;
   buf[2] = 0x02;
   buf[3] = 0x0D;
   buf[4] = 0x00;
   buf[5] = 0x24;
   buf[6] = 0x00;
   buf[7] = 0x00;

   char answbuf[4];

   return copleyBinaryCmd( buf, 8, answbuf, 4, 1000);
}

int copleyIsMoving()
{
	int ismoving = copleyEventStatusBit(27);
	if (debug) printf("isMoving: %d\n", ismoving);
	return ismoving;
}

int copleyIsAborted()
{
	int isaborted = copleyTrajectoryStatusBit(14);
	if (debug) printf("isAborted: %d\n", isaborted);
	return isaborted;
}

int copleyIsHomed()
{
	int ishomed = copleyTrajectoryStatusBit(12);
	if (debug) printf("isHomed: %d\n", ishomed);

	int ishoming = copleyTrajectoryStatusBit(13);
	if (debug) printf("Running homing: %d\n", ishoming);

	return ishomed;
}


int copleySetHomingMethod( int method)
{
	return copleySet('r', 0xC2, method);
}

int copleySetFastHomingSpeed( int speed)
{
	return copleySet('r', 0xC3, speed*10);
}

int copleySetSlowHomingSpeed( int speed)
{
	return copleySet('r', 0xC4, speed*10);
}

int copleySetHomingAcceleration( int acc)
{
	return copleySet('r', 0xC5, acc/10);
}

int copleySetHomingOffset( int offset)
{
	return copleySet('r', 0xC6, offset);
}

int copleySetPositiveSoftwareLimit( int pos)
{
	return copleySet('r', 0xB8, pos);
}

int copleySetNegativeSoftwareLimit( int pos)
{
	return copleySet('r', 0xB9, pos);
}

int copleyValue( char bank, int id, double scale, double *value)
{
	int v;
	CHECK( copleyGet( bank, id, &v));

   *value = v*scale; 
	return NO_ERROR;
}
	
int copleyCommandedCurrent( double *value)
{
	int stat = copleyValue( 'r', 0x15, 0.01, value);
	if (debug) printf("Commanded current: %5.2f\n", *value);
	return stat;
}
int copleyActualCurrent( double *value)
{
	int stat = copleyValue( 'r', 0x0C, 0.01, value);
	if (debug) printf("Actual current: %5.2f\n", *value);
	return stat;
}
int copleyLimitedCurrent( double *value)
{
	return copleyValue( 'r', 0x25, 0.01, value);
}
int copleyCommandedVelocity( double *value)
{
	return copleyValue( 'r', 0x2C, 0.1, value);
}
int copleyLimitedVelocity( double *value)
{
	return copleyValue( 'r', 0x29, 0.1, value);
}
int copleyActualMotorVelocity( double *value)
{
	return copleyValue( 'r', 0x18, 0.1, value);
}
int copleyActualLoadVelocity( double *value)
{
	return copleyValue( 'r', 0x5E, 0.1, value);
}
int copleyVelocityLoopError( double *value)
{
	return copleyValue( 'r', 0x2A, 0.1, value);
}
int copleyMotorPosition( double *value)
{
	int stat = copleyValue( 'r', 0x32, 1, value);
	if (debug) printf("Motor position: %5.2f\n", *value);
	return stat;
	
}
int copleyLoadPosition( double *value)
{
	int stat = copleyValue( 'r', 0x17, 1, value);
	if (debug) printf("Load position: %5.2f\n", *value);
	return stat;
}
int copleyFollowingError( double *value)
{
	return copleyValue( 'r', 0x35, 1, value);
}
int copleyBusVoltage( double *value)
{
	return copleyValue( 'r', 0x1E, 0.1, value);
}
int copleyTemperature( double *value)
{
	return copleyValue( 'r', 0x20, 1, value);
}




int copleyCommTest(void)
{
   double dummy;
   return copleyLoadPosition(&dummy);
}

// Sets the specified input pin as positive limit switch.
// The routine checks whether another input pin was
// previously set for the same function, and resets it.
//
// Input ranges from 0 to 11

int poslimit_pin = -1;
int neglimit_pin = -1;
int home_pin     = -1;

int copleySetPosLimitSwitchInput( int input)
{
   if ((input<0) || (input>11))
      return VALUE_OUT_OF_RANGE_ERROR;

   CHECK(copleyDisablePosLimitSwitch());
   poslimit_pin = input;
   return NO_ERROR;
}


int copleySetNegLimitSwitchInput( int input)
{
   if ((input<0) || (input>11))
      return VALUE_OUT_OF_RANGE_ERROR;

   CHECK(copleyDisableNegLimitSwitch());
   neglimit_pin = input;
   return NO_ERROR;
}

int copleyInputAddr( int input)
{
   if ((input >=0) && (input <8))
      return 0x78+input;
   if ((input >=8) && (input < 12))
      return 0xd0+input;

   return VALUE_OUT_OF_RANGE_ERROR;
}

int copleySetHomeSwitchInput( int input)
{
   if ((input<0) || (input>11))
      return VALUE_OUT_OF_RANGE_ERROR;

   CHECK(copleyDisableHomeSwitch());
   home_pin = input;
   return NO_ERROR;
}

int copleyResetInputFunction( int function)
{ 
   int i, addr, value;

   for (i=0; i<11; i++)
      {
      addr = copleyInputAddr(i);
      CHECK( copleyGet( 'r', addr, &value));
      if (value == function)
         CHECK( copleySet( 'r', addr, 0));
      }

   return NO_ERROR;
}

int copleyEnablePosLimitSwitch()
{
   if (poslimit_pin >= 0)
      return copleySet( 'r', copleyInputAddr(poslimit_pin), 4);
   else
      return NOT_INIT_ERROR;
}

int copleyEnableNegLimitSwitch()
{
   if (neglimit_pin >= 0)
      return copleySet( 'r', copleyInputAddr(neglimit_pin), 6);
   else
      return NOT_INIT_ERROR;
}

int copleyEnableHomeSwitch()
{
   if (home_pin >= 0)
      return copleySet( 'r', copleyInputAddr(home_pin), 14);
   else
      return NOT_INIT_ERROR;
}

int copleyDisablePosLimitSwitch()
{
   CHECK( copleyResetInputFunction(4));
   CHECK( copleyResetInputFunction(5));
   return NO_ERROR;
}

int copleyDisableNegLimitSwitch()
{
   CHECK( copleyResetInputFunction(6));
   CHECK( copleyResetInputFunction(7));
   return NO_ERROR;
}

int copleyDisableHomeSwitch()
{
   CHECK( copleyResetInputFunction(14));
   CHECK( copleyResetInputFunction(15));
   return NO_ERROR;
}

int copleyLoadParameters( char *filename)
{
   return NO_ERROR;
}

int copleySetBrakeDelay( double secs)
{
   return copleySet( 'f', 0x53, (int)(secs*1000));
}

int copleySetHandBrake( double secs)
{
   return copleySet( 'f', 0x54, (int)(secs*1000));
}

int copleySetPositionGains( int Pp, int Aff)
{
   int stat;

   stat = copleySet('r', 0x30, Pp);
   if (stat == NO_ERROR) stat =copleySet('r', 0x34, Aff);
   return stat;

}


int copleyCurStatus( char *errstr, int len)
{
    int status, i;


    /// Status register
    
    CHECK( copleyGet( 'r', 0xA0, &status));

    if (debug) printf("\n\n--------------- Status register (0xA0) ------------- \n\n");

//    dump(status);

    int status_faults[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 18, 20, 21, 29};
    int num_status_faults = 17;
    char *status_str[] = {
		"Short circuit",
		"Amplifier over temperature",
		"Over voltage",
		"Under voltage",
		"Motor over temperature",
		"Feedback error",
		"Motor phasing error",
		"Current limited",
		"Voltage limited",
		"Positive limit switch",
		"Negative limit switch",
      // "Amp disabled by hardware",
      "", // "Amp disabled by software",
      "", // "Motor brake active",
      "", // "Motor output disabled",
		"Following error",
		"Amplifier has been reset",
		"Encoder position wrapped and/or at limit",
		"Phase not initialized"
		};

    for (i=0; i<num_status_faults; i++)
      if (isBitSet(status, status_faults[i]))
         {
			if (debug) printf("%s \n", status_str[i]);

         if (errstr)
            if (strlen(errstr)>0)
               {
               strncat( errstr, status_str[i], len-strlen(errstr)-1);
               strncat( errstr, "\n", len-strlen(errstr)-1);
               }
         }


    if (debug) printf("\n\n\n");

    
    return NO_ERROR;
/*
    if (isBitSet( status, 22))
	{
    	// Latched fault! Read the fault register

    	if ((ret = copleyGet( 'r', 0xC9, &fault)) != NO_ERROR)
        	return ret;

	if (debug)
		{
		printf("Fault register: ");
		for (i=31; i>=0; i--)
			{
			int bit = (fault & (1<<i)) ? 1 : 0;
			printf("%1d", bit);
			}
		printf("\n");
		}

    	int fault_faults[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	char *faults_str[] = { 
			"Fatal hardware error: flash data currupt (CRC error)",
			"Fatal hardware error: A/D offset out of range",
			"Short circuit",
			"Amplifier over temperature",
			"Motor over temperature",
			"Over voltage",
			"Under voltage",
			"Feedback error",
			"Motor phasing error",
			"Following error",
			"Over current"
			};

    	int num_fault_faults = 11;

    	for (i=0; i<num_fault_faults; i++)
		if (isBitNotSet(status, fault_faults[i]))
			{
			printf("Fault bit %d set\n", fault_faults[i]);
			if (errstr)
				{
				printf("%s \n", faults_str[i]);
				snprintf(errstr, len-1, "%s", faults_str[i]);
				errstr[len-1] = 0;
				}
			return COPLEY_ERROR;
			}
	}

    int status_faults[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 18, 20, 21, 29};
    int num_status_faults = 15;
    char *status_str[] = {
		"Short circuit",
		"Amplifier over temperature",
		"Over voltage",
		"Under voltage",
		"Motor over temperature",
		"Feedback error",
		"Motor phasing error",
		"Current limited",
		"Voltage limited",
		"Positive limit switch",
		"Negative limit switch",
		"Following error",
		"Amplifier has been reset",
		"Encoder position wrapped and/or at limit",
		"Phase not initialized"
		};

    for (i=0; i<num_status_faults; i++)
	if (isBitNotSet(status, status_faults[i]))
		{
		printf("Status fault bit %d set\n", status_faults[i]);
		if (errstr)
			{
			printf("%s \n", status_str[i]);
			snprintf(errstr, len-1, "%s", status_str[i]);
			errstr[len-1] = 0;
			}
		return COPLEY_ERROR;
		}
      */

   return NO_ERROR;
}

int copleyInputs( int *state)
{
	int stat = copleyGet('r', 0xa6, state);	
   //printf("Inputs:\n");
   //dump(*state);
   return stat;
}

int copleyPositiveLimit(void)
{
   return copleyEventStatusBit(9);
}

int copleyNegativeLimit(void)
{
   return copleyEventStatusBit(10);
}
