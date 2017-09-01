// idrivelib.h
//
// Header file for the idrive library

#ifndef _IDRIVE_H_INCLUDED
#define _IDRIVE_H_INCLUDED


#define IDRIVE_BRAKE_PARAMETER	0x2039		
#define IDRIVE_BRAKE_ON			0x0010
#define IDRIVE_BRAKE_OFF		0x0000

#define IDRIVE_CONTROL_WORD			0x6040
#define IDRIVE_STATUS_WORD			0x6041
#define IDRIVE_CURRENT_POSITION		0x6064
#define IDRIVE_TARGET_POSITION		0x607A
#define IDRIVE_ERROR_CODE		0x603F


typedef unsigned char BYTE;

// +Struct
//
// DriveCmd: struct for a i-Drive serial message

struct iDrive_Command
{
	BYTE	stx;					// Fixed to 0x02
	BYTE	lge;					// Number of following bytes (min 2, max 254)
	BYTE	adr;					// Device address

	BYTE	blk_id;					
	BYTE	object[3];

//	BYTE	service_data[?];
	BYTE	process_data[8+1];		// Includes bcc (Block Check Character)

	int		length;					// Actual command length (the process_data field is a variable-length one)
};


// +Struct
//
// Param: struct for a single parameter set

struct iDrive_Param
{
	int object;
	int subindex;
	int value;
};


typedef struct iDrive_Command DriveCmd;
typedef struct iDrive_Param DriveParam;


// High-level commands

int initLibrary(void);				// call this function to initialize this library

int TestiDriveComm( int drive);		// Tests i-Drive communication

int SetupiDrive( int drive, char *config_file);						// Sets i-Drive internal parameters read from a config file

int MoveiDrive( int drive, int offset, int absolute, int force);	// Starts a movement
int EndMoving( int drive, int position_check);						// Checks for end of movement
int StopiDrive( int drive);											// Stop immediately
int GetCurrentPosition( int drive, int *pos);						// Returns the current position
int GetCurrentError( int drive);       								// Returns the current error code

int BrakeEnable( int drive);				// Enable brake
int BrakeDisable( int drive);				// Disable brake
int IsBraked( int drive);					// Returns brake status (1=braked, 0=no)


// Low-level commands

int LoadDriveConfig( DriveParam **setup_params, char *config_file); // Read a drive configuration file

int SetValue( int drive, int object, int subindex, int data);		// Read a parameter value
int GetValue( int drive, int object, int subindex, int *result);	// Write a paramter value
void SleepBetweenPackets(void);										// Waits the right amount of time between packets


// Utility functions for iDrive packets

int ReceiveCommand( int drive, DriveCmd *cmd, int timeout);
int BuildCommand( DriveCmd *, int drive, int object, int subindex, int writeflag, int data);
void DisplayCommand( DriveCmd *cmd);
int GetDataLen( int object, int subindex);
int GetDataMask( int object, int subindex);
int GetIntFromMask( unsigned char *bytes, int mask);




// Macros to READ (not write) the DriveCmd fields

#define IDRIVE_CONTROLLER_ADDR(cmd)	((cmd).adr & 0x07)
#define IDRIVE_TOTAL_MSG_LEN(cmd)	((cmd).length)
#define IDRIVE_DATA_LEN(cmd)		((cmd).lge-6)
#define IDRIVE_OBJECT(cmd)			((int)((cmd).object[0]) | (int)((cmd).object[1] << 8))
#define IDRIVE_SUBINDEX(cmd)		((cmd).object[2])
#define IDRIVE_DATA(cmd)			(GetIntFromMask((cmd).process_data, GetDataMask( IDRIVE_OBJECT(cmd), IDRIVE_SUBINDEX(cmd))))




#endif /* _IDRIVE_H_INCLUDED */

