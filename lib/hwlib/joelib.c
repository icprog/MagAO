// Serve un task che si colleghi al network/serial converter e lo configuri per bene...

// Joelib.c
//
// Library for LittleJoe CCD camera


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <pthread.h>

#include "hwlib/joelib.h"
#include "hwlib/netseriallib.h"
#include "base/errlib.h"

pthread_mutex_t Joe_Mutex;           // Mutex to lock communication

extern int debug;

//@Function  initLibrary                initializes this library

int initLibrary()
{
        pthread_mutex_init( &Joe_Mutex, NULL);
        return NO_ERROR;
}

//+Entry
//
// SendJoeCommand          sends a command to LittleJoe
//
// Sends a command to the LittleJoe CCD camera and waits for the answer.
//
// <command> is the LittleJoe command to be sent. The routine checks for
// the "@" prefix and <CR> ending, and adds them if necessary. If <command> is NULL
// nothing will be sent and the answer portion will be processed as usual.
//
// <answer> must be an already allocated buffer, or NULL. In the latter case,
// the answer from LittleJoe will not be reported.
//
// <max_answer_length>: lenght of the answer buffer in bytes
//
// <want_answer>: if nonzero, waits for an answer from LittleJoe. Otherwise, do not
// wait nor read any answers and return immediately. In the latter case, the
// parameters <answer> and <max_answer_length> are not used.
//
// Return zero or a negative error value. Errors directly from LittleJoe are
// forwarded as errors in the (-8000, -8007) range.

int SendJoeCommand(const char *command, char *answer, int max_answer_length, int want_answer)
{
   return SendJoeCommand_lockflag( command, answer, max_answer_length, want_answer, 1);
}

int SendJoeCommand_lockflag(const char *command, char *answer, int max_answer_length, int want_answer, int lockflag)
{
	char buffer[128], *pointer;
	int offset=0;
	int error_code;
	int command_length;
	
	// If not external buffer is provided, we can re-use our one later
	if (answer == NULL)
		{
		answer = buffer;
		max_answer_length = 128;
		}

	if (command != NULL)
		{	
		command_length = strlen(command);
		if (command_length > 126)
			command_length = 126;
	
		if (command[0] != '@')
			{
			buffer[0] = '@';
			offset=1;
			}
		
		strncpy( buffer+offset, command, command_length);
		buffer[ offset + command_length ] = 13;  // <CR>

		// Lock communication and send the packet		
      if (lockflag)
   		pthread_mutex_lock( &Joe_Mutex);

      if (debug)
            printf("Sending command\n");
		error_code = SerialOut( buffer, offset + command_length +1);
      if (debug)
            printf("Done sending command\n");

		if (debug)
			{
			buffer[ offset + command_length] = 0;
			printf("-->%s<--\n", buffer);
			}

		// Stop here if no answer is needed or some error occured
	 
		if ((error_code != NO_ERROR) || (!want_answer))
			{
			int retval = error_code;
			// Get the ACK
         if (debug)
			   printf("Waiting for ack\n");
			SerialInString( answer, 1, 1000, 0x06);
         if (debug)
			   printf("Got ack 0x%02X\n", answer[0]);
			if (answer[0] == '@')
				{
            if (debug)
				   printf("Getting further answer\n");
				SerialInString( answer+1, max_answer_length-2, 1000, 13);
				// Build a LittleJoe error code	
				if (strncmp( answer, "@ERR^", 5) == 0)
					{
					error_code = atoi( answer+5);
					if (error_code == 0)
						retval = NO_ERROR;
					else
						retval = JOE_BASE_ERROR - error_code;
					}
				}
	
         if (lockflag)
   			pthread_mutex_unlock( &Joe_Mutex);
			return retval;
			}
		}

	// Lock communication if not done before
	if (command == NULL)
		pthread_mutex_lock( &Joe_Mutex);

	// Zero the answer buffer
	// Do not this before, otherwise you could wipe out the command
	
	if (debug) printf("Waiting for answer...\n");
	memset( answer, 0, max_answer_length);
	SerialInString( answer, max_answer_length-1, 10000, 13);
	if (debug)
		printf("Answer:-->%s<-- \n", answer);

	// Unlock communication
   if (lockflag)
   	pthread_mutex_unlock( &Joe_Mutex);

	// Sometimes we have spurious character at the string beginning.
	// Search for the "@" marker and start from there

	if (strlen(answer) == 0)
		return PLAIN_ERROR(COMMUNICATION_ERROR);

	pointer = strchr( answer, '@');
	if (!pointer)
		return PLAIN_ERROR(JOE_UNKNOWN_ANSWER_ERROR);

	memmove( answer, pointer, strlen(pointer+1));


	// Remove the linefeeds/carriage returns at the end
	while ( (pointer = rindex( answer, 13)))
		*pointer =0;
	while ( (pointer = rindex( answer, 10)))
		*pointer =0;

	// Build a LittleJoe error code	
	if (strncmp( answer, "@ERR^", 5) == 0)
		{
		error_code = atoi( answer+5);
		if (error_code == 0)
			return NO_ERROR;
		else
			return JOE_BASE_ERROR - error_code;
		}
	
	return NO_ERROR;
}

void handle_sigpipe(int);

void handle_sigpipe(int a)
{
   a=0;  // Avoid warning
   printf("Handle sigpipe\n");
}


		
// +Entry
//
// InitJoeLink           initialize network/serial link with the LittleJoe CCD Camera
//

int InitJoeLink(const char *joe_addr, int joe_port)
{
        signal( SIGPIPE, handle_sigpipe);
	return SerialInit( joe_addr, joe_port);
}

// +Entry
//
// TestJoeLink           test network connection & Joe working

// Test the network connection and the fact that LittleJoe is up and running,
// using the standard "ERR?" AIA command.
//
// Returns 0 or a negative error code.

int TestJoeLink()
{
	return SendJoeCommand("ERR?", NULL, 0, 1);
}

//+Entry
//
// GetJoeVersion         gets LittleJoe firmware version
//
// Asks the LittleJoe camera for its firmware version and place the
// identifier string into the <version> buffer, which must be already allocated

int GetJoeVersion( char *version, int max_version_length)
{
	int err;
	
	err = SendJoeCommand("JOE?", version, max_version_length-1, 1);
	if (err != NO_ERROR)
		return err;
	
	fflush(stdout);
	
	if (strncmp( version, "@JOE! ", 5) == 0)
		{
		memmove( version, version + 6, strlen(version+6)+1);
		return NO_ERROR;
		}

	return PLAIN_ERROR(JOE_UNKNOWN_ANSWER_ERROR);
}

//+Entry
//
// CheckJoe         test if the LittleJoe camera is ready to work
//
// This function tests the network connection with LittleJoe and
// the camera functionality.
//
// (combines TestJoeLink() and GetJoeVersion() in one)
//
// Returns zero or a negative error code.

int CheckJoe()
{
	int error;
	char version[32];
	
	if ((error = TestJoeLink()) != NO_ERROR)
		return error;
	
	if (debug)
		printf("Joe link OK\n");
	
	if ((error = GetJoeVersion( version, 32)) != NO_ERROR)
		return error;
		
	if ( strcmp( version, "2.2.4") &&
         strcmp( version, "3.0.0") &&
         strcmp( version, "3.1.1") &&
	strcmp( version, "4.0.3")&&
	strcmp( version, "4.1.3"))
		return PLAIN_ERROR(JOE_UNSUPPORTED_VERSION_ERROR);

	if (debug)
		printf("Joe version %s OK\n", version);
	
	return NO_ERROR;
}

//+Entry 
//
// GetJoeStatus          returns the Joe camera status
//
// Returns the Joe CCD camera status, as follows:
//  JOE_NOT_RUNNING: not running
//  JOE_RUNNING	   : running
//  <0             : error code

int GetJoeStatus()
{
	int error;
	char buffer[32];
	
	if ((error = SendJoeCommand("SEQ?", buffer, 32, 1)) != NO_ERROR)
		return error;
		
	return atoi( buffer+6);
}

int GetBlackLevel(int n)
{
   int error;
   char req[32], buffer[32];

   if ((n<0) || (n>3))
      return VALUE_OUT_OF_RANGE_ERROR;

   sprintf(req, "OIC? #%d", n);
   if ((error = SendJoeCommand(req, buffer, 32, 1)) != NO_ERROR)
      return error;

   return strtol( buffer+10, NULL, 16);
}

//+Entry
//
// StartJoe           start image acquisition
//
// Return value: zero if OK, or a negative error code

int StartJoe()
{
	int error;

	if ( (error = GetJoeStatus()) == JOE_RUNNING)
		return NO_ERROR;

	if (error == JOE_NOT_RUNNING)
		 return SendJoeCommand("SEQ 1", NULL, 0, 0);
	else
		return error;

}

//+Entry
//
// StopJoe           stop image acquisition
//
// Return value: zero if OK, or a negative error code

int StopJoe()
{
	int error;

	if ( (error = GetJoeStatus()) == JOE_NOT_RUNNING)
		return NO_ERROR;

	if (error == JOE_RUNNING)
		return SendJoeCommand("SEQ 0", NULL, 0, 0);
	else
		return error;
}

//+Entry
//
// GetJoeTemperature       Find Joe's temperatures in Celsius degrees
//
// This function ask LittleJoe for its case and CCD temperatures,
// and returns the Celsius equivalents for them into the three
// floats pointed by t1, t2 and t3.
//
// t1 is the case temperature.
// t2 and t3 are the CCD temperatures.
//
// Returns zero or a negative error code.

int GetJoeTemperature( float *t1, float *t2, float *t3)
{
	int error;
	char answer[64], *token;
	int temp1, temp2, temp3;
	float a,b,c,d;

	temp1 = temp2 = temp3=0;

	// Joe temperature costants
	a = 3.354E-3;
	b = 2.888E-4;
	c = 3725.6;
	d = 11.403;
	
	if ((error = SendJoeCommand("TMP?", answer, 64, 1)) != NO_ERROR)
		return error;
	
	token = strtok( answer, ":");
	token = strtok( NULL, ":");
	if (token) temp1 = atoi( token);
	token = strtok( NULL, ":");
	if (token) temp2 = atoi( token);
	token = strtok( NULL, ":");
	if (token) temp3 = atoi( token);

   if (debug)
      printf("temp1: %d - temp2: %d - temp3: %d\n", temp1, temp2, temp3);   
	*t1 = ( 1.0 / ( a + b*(log( temp1 / 207.0)))) - 273.15;
	*t2 = ( c / ( log( temp2 / 2.55) + d)) - 273.15;
	*t3 = ( c / ( log( temp3 / 2.55) + d)) - 273.15;

   if (debug)   
      printf("t1: %5.2f - t2: %5.2f - t3: %5.2f\n", *t1, *t2, *t3);   
	return NO_ERROR;
}

//+Entry
//
// SetJoeProgram           Set a Joe program (pixel readout & related settings) 
//
// <program> : program number 0-7 inclusive
//
// Return value: zero if OK, or a negative error code

int SetJoeProgram( int program)
{
	char buffer[32];
	int cur_program, error;

	//error = SendJoeCommand("@PRG?", buffer, 31, 1);
   error = SendJoeCommand("@RCL?", buffer, 31, 1);
	if (error != NO_ERROR)
		return error;

	cur_program = atoi(buffer+6);

        // If the selected program is already active, stop here
	if (cur_program == program)
		return NO_ERROR;

   if (debug)
	   printf("Asked for program %d\n", program);

/*	sprintf( buffer, "@PRG %1d", program);
	if ((error = SendJoeCommand( buffer, NULL, 0, 0)) != NO_ERROR)
		return error;*/
	
	sprintf( buffer, "@RCL %1d", program);
	if ((error = SendJoeCommand( buffer, NULL, 32, 1)) != NO_ERROR)
		return error;
	
	return error;
}

//+Entry:  SetJoeBlack  
//
// Sets a black level

int SetJoeBlack( int blacknum, int value)
{
	char buffer[32];
	int stat;

	sprintf( buffer, "@OIC #%d:%d", blacknum, value);
	if ((stat = SendJoeCommand( buffer, NULL, 0, 0)) != NO_ERROR)
		return stat;
	return TestJoeLink();
}

//+Entry:  SetJoeRepetitions
//
// Sets a new repetition value for LittleJoe
//-

int SetJoeRepetitions( int rep)
{
	char buffer[32];

	sprintf( buffer, "@REP %d", rep);
	return SendJoeCommand( buffer, NULL, 0, 0);
}

//+Entry:  RecallSettings
//
// Recall factory settings and program from Flash memory without rebooting
//-

int RecallSettings()
{
        char buffer[32];
	int stat;

        sprintf( buffer, "@FTC");
        stat = SendJoeCommand( buffer, NULL, 0, 0);
        if (stat == NO_ERROR) {
		sprintf(buffer, "@FTP");
		return SendJoeCommand( buffer, NULL, 0, 0);
	} else return stat;
}


//@Function
//
// SendXmodemFile()     Sends a file using the Xmodem protocol
//
// Sends a file to the LittleJoe controller using the Xmodem protocol
// Used to send new pattern and control sequences to the controller.
//@

int SendXmodemFile(const char *upload_cmd,const char *filename)
{
	int pid;
	int fd;
//	char answer[128];

	// Get the socket to use and check its validity
	fd = GetSocketFD();
	if (fd == -1)
		return NETWORK_ERROR;

	// Check filename validity
	if (!filename)
		return FILE_ERROR;

	// Lock communication for the entire length of the update
   pthread_mutex_lock( &Joe_Mutex);

   // Send the upload command WITHOUT taking the lock again
   SendJoeCommand_lockflag( upload_cmd, NULL, 0, 0, 0); 

	// Fork to have a child doing the sx stuff
	pid = fork();

	// CHILD
	if (pid==0)
		{
		dup2( fd, 0);   // Close stdin, duplicate socket to stdin
		dup2( fd, 1);   // Close stdout, duplicate socket to stdout

		if (execl("/usr/bin/sx", "sx", "-vv", filename, NULL) == -1)
			{
			printf("Error executing sx\n");
			exit(0);
			}
		}

	// PARENT
	else
		{
		// We need something to synchronize outselves with the child
		wait(NULL);
		}

	// Get end-of-transfer string from LittleJoe

//	SendJoeCommand(NULL, answer, 128, 1);

	// Unlock communication
        pthread_mutex_unlock(&Joe_Mutex);

	return NO_ERROR;
}

int GetJoeClampLevel()
{
	int error;
	char buffer[32];
	
	if ((error = SendJoeCommand("DCA?", buffer, 32, 1)) != NO_ERROR)
		return error;

        printf("%s\n", buffer);
		
	return atoi( buffer+9);
}

int GetJoeSampleLevel()
{
	int error;
	char buffer[32];
	
	if ((error = SendJoeCommand("DSA?", buffer, 32, 1)) != NO_ERROR)
		return error;
		
	return atoi( buffer+9);
}


int SetJoeClampLevel( int level)
{
	char buffer[32];

        if (level<0)
           level=0;
        if (level>255)
           level=0;

	sprintf( buffer, "@DCA %d", level);
	return SendJoeCommand( buffer, NULL, 0, 0);
}

int SetJoeSampleLevel( int level)
{
	char buffer[32];

        if (level<0)
           level=0;
        if (level>255)
           level=0;

	sprintf( buffer, "@DSA %d", level);
	return SendJoeCommand( buffer, NULL, 0, 0);
}

