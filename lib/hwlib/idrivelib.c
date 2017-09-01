// idrivelib.c
//
// Implementation of the idrive library
//
// This version has all brake control disabled, that is, it's assumed
// that some external mechanism is used for brake control.

#include <stdio.h>

#ifdef _WIN32
#include "windows.h"
#else
#include <time.h>			// nanosleep()
#include <string.h>			// memset(), memcpy()
#endif

#include <stdlib.h>
#include <pthread.h>

#include "hwlib/idrivelib.h"
#include "hwlib/netseriallib.h"
#include "base/errlib.h"

pthread_mutex_t iDrive_Mutex;	// Mutex to lock communication

static int debug = 0;

int idrive_homing =0;

int IDRIVE_TIMEOUT  = 1000;		// Default timeout for serial communication


#ifndef _WIN32
//@Function: nusleep
//
// Implementation of usleep using nanosleep
//
//@

static void nusleep(useconds_t usec)
{
struct timespec delay;

delay.tv_sec=usec/1000000;
delay.tv_nsec=(usec-(delay.tv_sec*1000000))*1000;
nanosleep(&delay,NULL);
}
#endif




//@Function  initLibrary	initializes this library
int initLibrary()
{
	pthread_mutex_init( &iDrive_Mutex, NULL);
	return NO_ERROR;
}

// +Entry
//
// DisplayCommand()	displays the contents of an i-Drive command buffer
void DisplayCommand( DriveCmd *cmd)
{
	int i;
	for (i=0; i<cmd->length; i++)
		printf("0x%02X ", ((unsigned char *)cmd)[i]);

	printf("\n");
}

// +Entry
//
// SetValue:      Sends a command to the i-Drive, waits for confirmation
//
// Return value:  error code (NO_ERROR if everything OK).
int SetValue( int drive, int object, int subindex, int data)
{
	DriveCmd cmd;
	char *pointer = (char *)&cmd;
	int error = NO_ERROR;

   if (debug)
	   printf("Setting 0x%04X,0x%02X to %d\n", object, subindex, data);

	// Build the request packet
	error = BuildCommand( &cmd, drive, object, subindex, 1, data);
	if (error<0)
		return error;

	// Lock iDrive
	pthread_mutex_lock( &iDrive_Mutex);

	// Wait the correct amount of time
	SleepBetweenPackets();

	// Send it out the serial/network line
	error = SerialOut( pointer, cmd.length);
	if (error == NO_ERROR)
	{	
		// Waits for the answer.
		error = ReceiveCommand( drive, &cmd, IDRIVE_TIMEOUT);
	}

	// Unlock iDrive
	pthread_mutex_unlock(&iDrive_Mutex);

	// We should check for a valid answer, and for timeouts too....
	return error;
}

// +Entry
//
// GetValue:      Reads a parameter from the i-Drive
//
// parameters:
//             drive:  iDrive number on RS485 bus
//  object, subindex:  parameter numbers
//            result:  pointer to integer where the parameter value will be stored
//
// Return value:  zero or a negative error code
int GetValue( int drive, int object, int subindex, int *result)
{
	DriveCmd cmd;
	char *pointer = (char *)&cmd;
	int error = NO_ERROR;

   if (debug)
      printf("GetValue(): drive=%d, object=%d, subindex=%d\n", drive, object, subindex);

	// Build the request packet
	error = BuildCommand( &cmd, drive, object, subindex, 0, 0);
	if (error<0)
		return error;

	// Lock iDrive
	pthread_mutex_lock( &iDrive_Mutex);

	// Wait the correct amount of time
	SleepBetweenPackets();

	// Sent it out the serial/network line
	error = SerialOut( pointer, cmd.length);
	if (error == NO_ERROR)
		{
		// Waits for the answer
		error = ReceiveCommand( drive, &cmd, IDRIVE_TIMEOUT);
		}

	// Unlock iDrive
	pthread_mutex_unlock(&iDrive_Mutex);

	*result = IDRIVE_DATA(cmd);
	return error;
}

// +Entry
//
// BuildCommand:	 Builds a command for the i-Drive controller
//
// This function fills the DriveCmd structure, given the destination controller,
// the requested object and subindex, and the actual data to be written.
//
// Return value: length of the command in bytes (or a negative value for error)
int BuildCommand( DriveCmd *cmd, int drive, int object, int subindex, int writeflag, int data)
{
	int i, datalen=0;
	char xor;

	memset( cmd, 0, sizeof(DriveCmd));

	// Find the data length for the given object
	if (writeflag)
		datalen = GetDataLen( object, subindex);
		if (datalen<0)
			return PLAIN_ERROR(OBJECT_NOT_FOUND_ERROR);		// Error condition

	// Fill header
	cmd->stx = 0x02;
	if (writeflag)
		cmd->lge = 2+3+datalen+1;
	else
		cmd->lge = 2+3+1;

	cmd->adr = 0x80 | drive;

	if (writeflag)
		cmd->blk_id = 0x02;			// DA DEFINIRE
	else
		cmd->blk_id = 0x01;

	// Object index in lsb/msb/subindex format
	cmd->object[0] = object & 0xFF;
	cmd->object[1] = (object >>8) & 0xFF;
	cmd->object[2] = subindex;

//	cmd->service_data = ?;

	// Data must be formatted lsb...msb
	if (writeflag)
		for ( i=0; i<datalen; i++)
			{
			cmd->process_data[i] = data & 0xFF;
			data >>= 8;
			}

	// Compute the control character
	xor=0;
	for ( i=0; i< (int)sizeof(DriveCmd)-1; i++)
		xor ^= ((char *)cmd)[i];

	// Last data bit is the bcc character
	cmd->process_data[datalen] = xor;

	cmd->length = (int)cmd->lge +2;
	return cmd->length;
}

// +Entry
//
// GetDataLen: returns the I-Drive data length for a given object/subindex
//
// Returns the data length (in bytes) for the requested object and subindex.
// Returns -1 for error (e.g. unknown object)
int GetDataLen( int object, int subindex)
{
   if (subindex ==0)
      if ((object >= 0x2000) && (object <= 0x231D))
         return 2;

   switch(object)
      {
      case 0x225F:
      if (subindex==1)
         return 2;

		case 0x2407:
		if (subindex==12)
			return 2;

		case 0x603F:
		case 0x6040:
		case 0x6041:
		if (subindex ==0)
			return 2;
		break;

		case 0x6060:
		case 0x6061:
		if (subindex ==0)
			return 1;
		break;

		case 0x6064:
		case 0x607A:
		if (subindex == 0)
			return 4;
		break;
	
		case 0x6081:
		case 0x6083:
		case 0x6084:
		if (subindex ==0)
			return 4;
		break;

		case 0x6098:
		if (subindex ==0)
			return 1;
		break;

		case 0x6099:
		if (subindex ==0)
			return 1;
		return 4;
		break;	

		case 0x609A:
		if (subindex ==0)
			return 4;
		break;

		case 0x60FB:
		if (subindex == 0)
			return 1;
		if (subindex == 3)
			return 4;
		else
			return 2;

		break;
		}

	printf("Object 0x%02X,%d is unknown\n", object, subindex);
	return -1;		// Unknown object
}

// +Entry
//
// GetDataMask: returns a mask to get the message data
//
// For a given object and subindex, builds a bit mask to extract the message data
// from the <process_data> field
int GetDataMask( int object, int subindex)
{
	int len;
	int i,mask=0;

	len = GetDataLen( object, subindex);
	for ( i=0; i< len*8; i+=8)
		mask |= 0xFF << i;

	return mask;
}

// +Entry
//
// GetIntFromMask
int GetIntFromMask( unsigned char *bytes, int mask)
{
	unsigned char data[4];
	int i,value =0;

	memcpy( data, bytes, 4);

	for ( i=0; i< 4; i++)
		value += (int) (((unsigned int)data[i]) << i*8);

	value &= mask;
	return value;
}

// +Entry
//
// ReceiveCommand: receive a DriveCmd string from the i-Drive controller
//
// This function waits for a correct DriveCmd string from a controller,
// and fills the DriveCmd structure. A timeout (in seconds) can be
// specified (0 for no timeout). 
//
// Return value: error code
int ReceiveCommand( int drive, DriveCmd *cmd, int timeout)
{
	char buffer[128];		// this is more than the biggest possible packet
	int len, num, num2;

	drive=0; // Not used for now

	// Get the forts two bytes (DriveCmd stx and lge) from serial port
	num = SerialIn( buffer, 2, timeout);
	if (num != 2)
		{
      if (debug)
		   printf("Error in SerialIn(): read %d bytes\n", num);
		return PLAIN_ERROR(NETWORK_ERROR);
		}
	
	// Finds the packet lenght
	len = (unsigned char)buffer[1];

	// Get the other "lenght" bytes from packet
	num2 = SerialIn( buffer+num, len, timeout);
	if ( num2 != len)
		{
      if (debug)
		   printf("Error in SerialIn(): read %d bytes instead of %d\n", num2, 2+len);
		return PLAIN_ERROR(NETWORK_ERROR);
		}

	memcpy( cmd, buffer, sizeof(DriveCmd));
	cmd->length = len+2;

	return NO_ERROR;
}


//@Function  TestiDriveComm          Tests communication with i-Drive
//
// This function perform a quick communication test and returns an error code
// indicating the test result. A NO_ERROR code means that communications is OK
//
// The communication test does not change the iDrive state in any way and can
// be performed at any time
//@
int TestiDriveComm( int drive)
{
	int temp;
	return GetValue( drive, IDRIVE_CONTROL_WORD, 0, &temp);
}
	
// +Entry
//
// SetupiDrive				Sets i-Drive parameters from a configuration file
//
// This function reads a configuration file and sets the specified i-Drive parameters,
// usually to put it in a known state before operation.
int SetupiDrive( int drive, char *config_file)
{
	int error = NO_ERROR;
	int num_params,i,value;
	DriveParam *setup_params = NULL;

	// Test communication
	if ((error = TestiDriveComm(drive)) != NO_ERROR)
		return error;

   if (debug)
	   printf("i-Drive %d communication OK, starting configuration\n", drive);

	// Read the i-Drive configuration file
	num_params = LoadDriveConfig( &setup_params, config_file);
//	printf("I have %d parameters\n", num_params);

	if (num_params < 0)
		return num_params;
	
	for (i=0; i<num_params; i++)
	{
      //if (debug)
		   printf("Setting 0x%02X,0x%02X (P%d) to %d (0x%02X)  ", setup_params[i].object, setup_params[i].subindex, setup_params[i].object-0x2000, setup_params[i].value, setup_params[i].value);
      	fflush(stdout);

		// Before setting each parameter, read it to see if it is already
		// at the desired value. Repeated writes will damage the Flash memory
		// in the long term

		error = GetValue( drive, setup_params[i].object, setup_params[i].subindex, &value);
		if (error)
        {
         	printf("error: (%d) %s\n", error, lao_strerror(error));
			return error;
        }

		if ( value != setup_params[i].value)
		{
			error = SetValue( drive, setup_params[i].object, setup_params[i].subindex, setup_params[i].value);
			if (error)
            {
            	printf("error: (%d) %s\n", error, lao_strerror(error));
				return error;
            }
         	if (debug) printf("done.\n");
		}
		else
         {
			//if (debug)
            printf("already set, skipping\n");	
         }
	}

	free( setup_params);

	return NO_ERROR;
}



// +Entry
//
// MoveiDrive         start an i-Drive movemnt
//
// This function tells the i-Drive to start a movement.
// <offset> is the requested movement (relative), in encoder counts.
// <absolute> if different from 0, will interpret <offset> as an absolute position
// <force> does not wait the end of the previous movement before starting
//
// The motor brake will be automatically released if necessary.
//
// WARNING: There is no check about the movement validity (i.e. out of range, etc.):
//
// After this function has been called, the EndMoving() function MUST be called one or more times until
// the movement is acknowledged (which is, EndMoving() returns 1).
// For example:
//
// MoveiDrive( STEP_SIZE )
// while (!EndMoving())
//		{ sleep(10ms); }
//
// Calls to MoveiDrive() before EndMoving() returns 1 will not be executed, but rather saved into the iDrive 
// (the number of movements saved is not known, and it's likely just one), unless the <force> flag is specified.
// In this case, the new movement will start immediately.
int MoveiDrive( int drive, int offset, int absolute, int force)
{
	int error = NO_ERROR;
	int pos = 0;
	int value = 0x001F; // 31

	// Release brake if necessary
//	if ( IsBraked(drive) >0)
//		error = BrakeDisable(drive);

	if (error)
		return error;

	// Set step size
	error = SetValue( drive, IDRIVE_TARGET_POSITION, 0, offset);
	if (error)
		return error;

	error = GetCurrentPosition( drive, &pos);
	if (error)
		return error;

	if (debug) printf("Starting from %d\n", pos);

	// Start moving
	if (!absolute)
		value |= 0x0040;	// 64
	if (force)
		value |= 0x0020;	// 32

	error = SetValue( drive, IDRIVE_CONTROL_WORD, 0, value);
	if (debug)
		printf("MoveiDrive(): setting control word to 0x%02X\n", value);

	return error;
}


// +Entry
//
// EndMoving		see if the previous movement is finished
//
// This function must be called after EndMoving() to get information about the movement status.
// There is no minimum or maximum time between calls to MoveiDrive() and EndMoving().
//
// The movement status is reported by the iDrive. However, it is sometimes inaccurate due to overshoots,
// and it may report a "target reached" status even when the stage is still moving. The "position_check"
// flag forces the routine to perform an extra check on the stage position, asking five consecutive
// reads on the position encoder and only accepting the result when all three are the same.
//
// Return value: 1 if the movement has ended and has been acknowledged, 0 if not.
// The function should be called repeatedly until it returns 1. A new movement must not be attempted
// until the previus one has been acknowledged.

int EndMoving( int drive, int position_check)
{
	int status, error;

	// Get the status word
	error = GetValue( drive, IDRIVE_STATUS_WORD, 0, &status);
	if (error)
		return error;

	//if (debug)
        printf("EndMoving(): status word is 0x%02X\n", status);
	
	// See if we are arrived
	if (status & 0x400)//  || ((idrive_homing) && (status == 0x1221)))
	{

		// Perform extra positioning check if required, reading
		// the current position multiple times and only accepting
		// the result when all the positions are the same.
		if (position_check)
		{
			int pos, pos2, i, n_checks = 5;

			if ((error = GetCurrentPosition(drive, &pos)) != NO_ERROR)
				return error; 
			for (i=0; i<n_checks; i++)
			{
				if ((error = GetCurrentPosition(drive, &pos2)) != NO_ERROR)
					return error;
				if (debug) printf("EndMoving() extra check: pos1 = %d, pos2 = %d\n", pos, pos2);
				if (pos2 != pos)
					break;
			}

			// If the loop was not completed, we aren't arrived yet
			if (i != n_checks)
				return 0;	
		}	

		// See if we have to acknowledge an absolute or relative movement
		if (status & 0x0040)
			status = 0x004F;
		else
			status = 0x000F;

		// If yes, acknowledge movement
		SetValue( drive, IDRIVE_CONTROL_WORD, 0, status);
		if (debug)
			printf("EndMoving(): setting control word to 0x%02X\n", status);
		

		if (debug) printf("Target reached\n");
		return 1;
	}
	// Also check for end-of-homing bit
	//
	// There is nothing to acknowledge, just return success
//	else if (status & 0x800)
   	else if (status == 0x1221)
		return 1;
	// Otherwise return 0 to indicate that the movement is still in progress
	else
	{
		return 0;
	}
}



// +Entry
//
// GetCurrentPosition()                gets the current i-Drive position
//
// Finds current i-Drive position, as read directly from the encoder
// The current position is placed in the integer pointed by <pos>, and
// can be both positive and negative

int GetCurrentPosition( int drive, int *pos)
{
	return GetValue( drive, IDRIVE_CURRENT_POSITION, 0, pos);
}


// +Entry
//
// StopiDrive()		   iDrive disabling and/or emergency stop
//
// This function tells the i-Drive to stop immediately.
//
// Return value: NO_ERROR if OK, or an error code

int StopiDrive( int drive)
{
	SetValue( drive, IDRIVE_CONTROL_WORD, 0, 0x0080);
	return SetValue( drive, IDRIVE_CONTROL_WORD, 0, 0x0000);
}

// +Entry
//
// BrakeEnable             enable motor brake
//
// This function tells the i-Drive to enable the motor brake
// If a movement is then attempted, the brake will be released automatically
//
// !!! Not used: now the brake is controlled by a relay !!!
int BrakeEnable( int drive)
{
	return SetValue( drive, IDRIVE_BRAKE_PARAMETER, 0, IDRIVE_BRAKE_ON);
}

// +Entry
//
// BrakeDisable            disable motor brake
//
// Tells the i-Drive to release the motor brake.
// This function is called automatically from MoveiDrive() as necessary
//
// !!! Not used: now the brake is controlled by a relay !!!
int BrakeDisable( int drive)
{
	return SetValue( drive, IDRIVE_BRAKE_PARAMETER, 0, IDRIVE_BRAKE_OFF);
}

// +Entry
//
// IsBraked                checks if motor is braked
//
// Asks the brake status. Returns 1 if the brake is enabled, 0 if not,
// or a negative error code.
//
// !!! Not used: now the brake is controlled by a relay !!!
int IsBraked( int drive)
{
	int value, error;

	error = GetValue( drive, IDRIVE_BRAKE_PARAMETER, 0, &value);
	if (error)
		return error;

	if (debug) printf("Brake parameter is %d\n", value);

	if (value == IDRIVE_BRAKE_ON)
		return 1;
	if (value == IDRIVE_BRAKE_OFF)
		return 0;
	else
		return value;
}

// +Entry
//
// LoadDriveConfig			reads an i-Drive configuration file
//
// This function reads an i-Drive configuration file. The setup_params
// array is created and filled with configuration data.
// Freeing of the array is the responsability of the calling routine.
//
// Returns the number of parameters read, or a negative error code. In the latter
// case, no array is allocated.
int LoadDriveConfig( DriveParam **setup_params, char *config_file)
{
	FILE *fp;
	char buffer[1024];
	char seps[] = " ,\t\n";
	char *token;
	int object, subindex, value;
	int counter=0;

	// Try to open the file
	if ( (fp = fopen( config_file, "r")) == NULL)
		return SYSTEM_ERROR(FILE_ERROR);

	// Read one line at a time
	while ( fgets( buffer, 1024, fp))
		{
		token = strtok( buffer, seps );
		if (!token)
			continue;

		// Skip comment lines
		if ((token[0] == '#') || (token[0] == ';'))
			continue;

		// Get a parameter number with 'P' syntax
		if ((token[0] == 'P') || (token[0] == 'p'))
			object = atoi(token+1) + 0x2000;

		// or a hexadecimal number
		else if (strchr( token, 'x'))
			sscanf( token, "%x", &object);

		// or a plain decimal number
		else
			object = atoi(token);

		// Now find the subindex
		token = strtok( NULL, seps );
		if (!token)
			continue;

		// which can be decimal or hexadecimal
		if (strchr( token, 'x'))
			sscanf( token, "%x", &subindex);
		else
			subindex = atoi( token);


		// At last, the value
		token = strtok( NULL, seps );
		if (!token)
			continue;

		// decimal or hexadecimal
		if (strchr( token, 'x'))
			sscanf( token, "%x", &value);
		else
			value = atoi( token);

		// Make sure that we have a big enough array	
		if (!counter)
			*setup_params = (DriveParam *)malloc( sizeof(DriveParam)*8);
		else if ((counter %8) == 0)
			*setup_params = (DriveParam *)realloc( *setup_params, sizeof(DriveParam)*(counter+8));

		// Write the new parameter
		(*setup_params)[counter].object = object;
		(*setup_params)[counter].subindex = subindex;
		(*setup_params)[counter].value = value;

		counter++;
		}

	fclose(fp);

	return counter;
}


// +Entry
//
// SleepBetweenPackets()			waits the right amount of time between packets
//
// Two i-Drive packets must be separated by a minimum amount of time. Call this
// function before each read or write to/from i-Drives to ensure that the
// minimum delay is present.

void SleepBetweenPackets()
{
#ifdef _WIN32
	Sleep(3);
#else
	nusleep(3000);
#endif
}

int GetCurrentError( int drive)
{
   int code;
   GetValue( drive, IDRIVE_ERROR_CODE, 0, &code);
   return code;
}
