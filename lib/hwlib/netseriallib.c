
//
// Implementation of the NetSerial library


#include <stdio.h>

#ifdef _WIN32

#include "winsock2.h"
typedef unsigned int socklen_t;
#undef NO_ERROR

#else

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>	// strchr()
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#endif //_WIN32

#include "hwlib/netseriallib.h"
#include "base/errlib.h"

#define Log(a) a

extern int debug;


#ifndef _WIN32
//+Function: nusleep
//
// Implementation of usleep using nanosleep
//
//-

static void nusleep(useconds_t usec)
{
struct timespec delay;

delay.tv_sec=usec/1000000;
delay.tv_nsec=(usec-(delay.tv_sec*1000000))*1000;
nanosleep(&delay,NULL);
}
#endif

// +Entry
//
// SerialInit              starts the serial/network interface
//
// Initializes the serial/network interface
//
// The serial interface code is Windows-specific

int sockfd=-1;		// socket fd
HANDLE serialport;
int use_network =0;

static int verbose = 0;


int SerialInit(const char *address, int port)
{
	int err= NO_ERROR;
	struct sockaddr_in servaddr;

	// Choose between network and serial: a dot in the string means a network address
	//if (strchr( address, '.'))
	//	use_network =1;
	//else
//		use_network =0;

   // Always use network!
   use_network = 1;

   SerialClose();

	if (use_network)
		{
#ifdef _WIN32
		WSADATA wsaData;
		int err;

		err = WSAStartup( 0x0202, &wsaData );
		if ( err != 0 )
			{
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
			return SYSTEM_ERROR(NETWORK_ERROR);
			}
#endif


		sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (sockfd == -1)
         return SYSTEM_ERROR(NETWORK_ERROR);

      struct hostent *h = gethostbyname(address);

		servaddr.sin_family=AF_INET;
		servaddr.sin_port=htons(port);
      memcpy( &servaddr.sin_addr, h->h_addr_list[0], h->h_length);

		err = connect( sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
		}
	else
		{
#ifdef _WIN32
		serialport = SerialSetup( address, 9600 , 8, EVENPARITY, ONESTOPBIT);
		if (serialport == INVALID_HANDLE_VALUE)
			err = SYSTEM_ERROR(COMMUNICATION_ERROR);
		printf("Connected via serial port\n");
#endif
		}

	return err;
}

// +Entry
//
// SerialClose				stops the serial/network interface

int SerialClose()
{
	if (use_network)
		{
#ifdef _WIN32
		return shutdown( sockfd, SD_BOTH);
#else
		shutdown( sockfd, 2);
      close(sockfd);
      return NO_ERROR;

#endif
		}
	else
		return SerialShutdown( serialport);
}

// +Entry
//
// SerialOut			sends a buffer to the serial/network interface
//
// Returns: error code

int SerialOut(const char *buf, int len)
{
	int i=0;
   int stat;
	if (use_network)
		while (i<len)
			{
			stat = send( sockfd, buf+i, len-i, 0);
         if (stat >0)
            i+= stat;
         if ((stat<0) && (stat != EAGAIN))
            return COMMUNICATION_ERROR;
//			if (debug>1)
//				printf("Sent %d bytes\n", i);
			}
	else
		SerialTransmit( serialport, buf, len);

//	if (debug>1)
//		{
//		printf ("Sent %d characters\n", i);
//		for (i=0; i<len; i++)
//			printf("0x%02X ", (unsigned char) buf[i]);
//		printf("\n");
//		}

	return NO_ERROR;
}	
	

// +Entry
//
// SerialIn					receives a series of characters from network/serial line
//
// Return value: the number of bytes actually read
// The timeout value is expressed in milliseconds, and works only on a network interface for now.

int SerialIn( char *buffer, int len, int timeout)
{
	int res=0;

	memset( buffer, 0, len);

	if (use_network)
		while (res < len)
			{
			fd_set rdfs;
			struct timeval tv;
			int retval;

			memset( &tv, 0, sizeof( struct timeval));
			FD_ZERO( &rdfs);
			FD_SET( sockfd, &rdfs);
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout-(timeout/1000)*1000)*1000;

			retval = select( sockfd+1, &rdfs, NULL, NULL, &tv);
			if (!retval)
				return res;

			res += recv( sockfd, buffer+res, len-res, 0);

			SerialSleep(3);
//			if (debug>1)
//				printf("Received %d bytes\n", res);
			}
	else
		res = SerialRead( serialport, buffer, len);

//	if (debug>1)
//		{
//		int i;
//		printf("SerialIn(): received %d characters:\n", res);
//		for ( i=0; i<res; i++)
//			printf("0x%02X ", (unsigned char) buffer[i]);
//		printf("\n");
//		}

	return res;
}





// +Entry
//
// SerialInString		 	receives a series of characters from network/serial line
//					with a string terminator
//
// Return value: the number of bytes actually read
// The timeout value is expressed in milliseconds, and works only on a network interface for now.
//
// Serial line reading stops at the first occurence of:
// 1) timeout
// 2) max. len reached
// 3) terminator reached

int SerialInString( char *buffer, int len, int timeout, char terminator)
{
   int signaled;
	int res=0;
   struct timeval tv0, tv1;
   double t0, t1;
	memset( buffer, 0, len);

   if(debug)
      printf("initial timeout = %i\n", timeout);

   gettimeofday(&tv0, 0);
   t0 = ((double)tv0.tv_sec + (double)tv0.tv_usec/1e6);

   gettimeofday(&tv1, 0);
   t1 = ((double)tv1.tv_sec + (double)tv1.tv_usec/1e6);

	if (use_network)
		while (res < len && ((t1-t0)*1000. < timeout))
		{
			fd_set rdfs;
			struct timeval tv;
			int retval;

			memset( &tv, 0, sizeof( struct timeval));
			FD_ZERO( &rdfs);
			FD_SET( sockfd, &rdfs);
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout-(timeout/1000)*1000)*1000;

         if(debug)
            printf("Selecting . . .\n");

         /* Making this a signal-safe call to select*/
         /*JRM: otherwise, signals will cause select to return
           causing this loop to never timeout*/
         
         signaled = 1;

         while(signaled && ((t1-t0)*1000. < timeout))
         {
            errno = 0;
            signaled = 0;
            retval = select(sockfd+1, &rdfs, NULL, NULL, &tv);
            if(retval < 0)
            {
               //This means select was interrupted by a signal, so keep going.
               if(errno == EINTR)
               {

                  if(debug)
                     printf("EINTR\n");
                  signaled = 1;
                  gettimeofday(&tv1, 0);
                  t1 = ((double)tv1.tv_sec + (double)tv1.tv_usec/1e6) ;
                  //Reset for smaller timeout
                  timeout = timeout - (t1-t0)*1000.;

                  if(debug)
                  {
                     printf("t1-t0 = %f\n", (t1-t0)*1000);
                     printf("timeout = %i\n", timeout);
                  }
                  tv.tv_sec = timeout / 1000;
                  tv.tv_usec = (timeout-(timeout/1000)*1000)*1000;
                  if(tv.tv_sec < 0) tv.tv_sec = 0;
                  if(tv.tv_usec < 0) tv.tv_usec = 0;
                  
               }
            }
         }

         if(debug)
            printf("select returned %i . . .\n", retval);

			if (retval <= 0) //this means we timed out or had an error
				return res;

         if(debug)
            printf("Starting read . . .\n");

			res += recv( sockfd, buffer+res, len-res, 0);
         if(debug)
            printf("Read %i bytes. . .\n", res);

// {
//       int i;
//       printf("SerialIn(): received %d characters:\n", res);
//       for ( i=0; i<res; i++)
//          printf("0x%02X ", (unsigned char) buffer[i]);
//       printf("\n");
//       }

			buffer[res] =0;
			if (verbose)
      		printf("SerialInString received %d bytes: %s\n", res, buffer);
			if (strchr( buffer, terminator))
				return res;

         gettimeofday(&tv1, 0);
         t1 = ((double)tv1.tv_sec + (double)tv1.tv_usec/1e6);

         if(debug)
         {
            printf("t1-t0 = %f\n", (t1-t0)*1000.);
            printf("timeout = %i\n", timeout);
         }

			SerialSleep(3);
	   }
	else
		res = SerialRead( serialport, buffer, len);

	if (1)//debug>1)
		{
		int i;
		printf("SerialIn(): received %d characters:\n", res);
		for ( i=0; i<res; i++)
			printf("0x%02X ", (unsigned char) buffer[i]);
		printf("\n");
		}

	return res;
}

// The same, with string terminator

int SerialInString2( char *buffer, int len, int timeout, char *terminator)
{
	int res=0;

	memset( buffer, 0, len);

	if (use_network)
		while (res < len)
			{
			fd_set rdfs;
			struct timeval tv;
			int retval;

			memset( &tv, 0, sizeof( struct timeval));
			FD_ZERO( &rdfs);
			FD_SET( sockfd, &rdfs);
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout-(timeout/1000)*1000)*1000;

			retval = select( sockfd+1, &rdfs, NULL, NULL, &tv);
			if (!retval)
				return res;

			res += recv( sockfd, buffer+res, len-res, 0);

			buffer[res] =0;
			if (verbose)
      		printf("SerialInString received %d bytes: %s\n", res, buffer);
			if (strstr( buffer, terminator))
				return res;

			SerialSleep(3);
			}
	else
		res = SerialRead( serialport, buffer, len);

//	if (debug>1)
//		{
//		int i;
//		printf("SerialIn(): received %d characters:\n", res);
//		for ( i=0; i<res; i++)
//			printf("0x%02X ", (unsigned char) buffer[i]);
//		printf("\n");
//		}

	return res;
}





// +Entry
//
// SerialRead                read a string of bytes from the serial line
//
// Return value: number of bytes actually written, or an error code.

// +Entry
//
// SerialRead                read a string of bytes from the serial line
//
// Return value: number of bytes actually written, or an error code.
//
// This function tries to read from the serial line <num> bytes. If more bytes
// are arriving, they are left in the input queue.
// This routine is Windows-specific. A Linux version must be written


int SerialRead( HANDLE port, char *buffer, int num)
{

#ifdef _WIN32

	DWORD dwCommEvent;
	int n;
	int received = 0;

	while (received<num)
		{
		if (!WaitCommEvent( port, &dwCommEvent, NULL))
			{
			Log("Error in WaitCommEvent()");
			return SYSTEM_ERROR(COMMUNICATION_ERROR);
			}

		else 
			{
			DWORD errors;
			COMSTAT stat;

			ClearCommError( port, &errors, &stat);

			if (dwCommEvent == EV_ERR)
				{
				char *err;
				switch (errors)
					{
					case CE_FRAME:
					err = "CE_FRAME";
					break;

					case CE_RXPARITY:
					err = "CE_RXPARITY";
					break;

					case CE_OVERRUN:
					err = "CE_OVERRUN";
					break;

					default:
					err = "Unknown error";
					break;
					}

				Log("Error in serial communication (see next line)");
				Log(err);
				return 0;
				}

			else
				{
				n=  stat.cbInQue;
//				printf("Buffer: %d characters\n", stat.cbInQue);

				if ( received + n > num)
					n = num - received;
				received += SerialReceive( port, buffer+received, n);
				}
			}

		if (!SetCommMask( port, EV_RXCHAR |EV_ERR |EV_TXEMPTY))
			{
			Log("Error in SetCommMask()");
			return SYSTEM_ERROR(COMMUNICATION_ERROR);
			}

		}
_debug
	return received;

#else
	port = num =0;		// use them in some way to avoid warnings
	buffer=NULL;
	return 0;
#endif
}



// parity: EVENPARITY, ODDPARITY, NOPARITY
// stopbits: ONESTOPBIT, TWOSTOPBITS

// +Entry
//
// SerialSetup                  setup the serial port
//
// This function sets up the serial port for the use with our i-Drives

HANDLE SerialSetup(const char *name, int baudrate, int databits, int parity, int stopbits)
{
#ifdef _WIN32

	DCB config;
	HANDLE handlePort;
	BOOL fSuccess;
    COMMTIMEOUTS comTimeOut;                   
	COMSTAT stat;
	DWORD errors;

	// Create the serial port "file"

	handlePort = CreateFile ( name,			// Port device (es. "COM1")

		GENERIC_READ | GENERIC_WRITE,       // Specify mode that open device.
		0,                                  // the devide isn't shared.
		NULL,                               // the object gets a default security.
		OPEN_EXISTING,                      // Specify which action to take on file.
		0,                                  // default.
		NULL);      

	if ( handlePort == INVALID_HANDLE_VALUE) 
		{
	    // handle error 
		Log("Error creating COM1 file");
		return handlePort;
		}

	SetupComm( handlePort, 1024, 512);
	PurgeComm( handlePort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    // Specify time-out between charactor for receiving.
    comTimeOut.ReadIntervalTimeout = -1;
    // Specify value that is multiplied 
    // by the requested number of bytes to be read. 
    comTimeOut.ReadTotalTimeoutMultiplier = 0;
    // Specify value is added to the product of the 
    // ReadTotalTimeoutMultiplier member
    comTimeOut.ReadTotalTimeoutConstant = 0;
    // Specify value that is multiplied 
    // by the requested number of bytes to be sent. 
    comTimeOut.WriteTotalTimeoutMultiplier = 0;
    // Specify value is added to the product of the 
    // WriteTotalTimeoutMultiplier member
    comTimeOut.WriteTotalTimeoutConstant = 5000;
    // set the time-out parameter into device control.
    SetCommTimeouts( handlePort,&comTimeOut);



	// Get the current serial port configuration
	fSuccess = GetCommState( handlePort, &config);
	if (!fSuccess) 
		{
		Log("Error getting comm state\n");
		return handlePort;
		}
	
	// Paste our parameters
	config.BaudRate = baudrate;
	config.ByteSize = databits;
	config.Parity = parity;
	config.StopBits = stopbits;
	config.fDtrControl = DTR_CONTROL_ENABLE;
	config.XonLim = 256;
	config.XoffLim = 256;
	config.fParity = 1;
	config.fDsrSensitivity =0;
	config.fBinary = TRUE;
	config.fRtsControl = RTS_CONTROL_ENABLE;

	// Set the new serial port configuration
	fSuccess = SetCommState( handlePort, &config);

	if (!fSuccess) 
		{
		Log("Error setting comm state\n");
		return handlePort;
		}

	SetCommMask( handlePort, EV_RXCHAR | EV_ERR | EV_TXEMPTY);
	ClearCommError( handlePort, &errors, &stat);
	  
	return handlePort;

#else
	name=NULL;
	baudrate = databits = parity = stopbits =0;
	return 0;
#endif

}



// +Entry
//
// SerialTransmit          trasmit bytes to the serial line
//
// Return value: the number of bytes actually written

int SerialTransmit( HANDLE port,const char *buffer, int length)
{
#ifdef _WIN32

	int result;
	unsigned long written;
	int total=0;
	int original_length = length;

	while (total < original_length)
		{
		result= WriteFile(  port, // handle to file to write to
							buffer,              // pointer to data to write to file
							length,              // number of bytes to write
							&written,
							NULL);	     // pointer to number of bytes written

		if (result == 0)
			{
			Log("Failed to write to serial port");
			return 0;
			}	

		total += written;
		buffer += written;
		length -= written;
		}

	return total;

#else
	port = length = 0;
	buffer=NULL;
	return 0;

#endif
}


// +Entry
//
// SerialReceive               receive bytes from the serial line
//
// Return value: the number of bytes actually read

int SerialReceive( HANDLE port, char *buffer, int length)
{	

#ifdef _WIN32

	int result;
	unsigned long read;
	int total=0;
	int original_length = length;

	

	result = ReadFile(  port,	  // handle of file to read
							buffer,   // pointer to buffer
							length,       // number of bytes to read
							&read,        // pointer to number of bytes read
							NULL);        // pointer to structure for data


		if (result == 0)
			{
			Log("Failed to read from serial port");
			return 0;
			}

//		total += read;
//		buffer += read;
//		length -= read;
//		}

	return read;
//	return total;

#else
	port = length = 0;
	buffer=NULL;
	return 0;
#endif

}

// +Entry
//
// SerialShutdown              closes the serial port
//
// This function closes the serial port and frees it for use by other programs
//
// This function is Window-specific. A Linux version must be written.

int SerialShutdown( HANDLE port)
{

#ifdef _WIN32

	if( CloseHandle( port) == 0)
		{
		Log("Error closing serial port");
		return FALSE;
		} 

	return 0;
#else
	port=0;
	return 0;
#endif
}




// +Entry
//
// SerialSleep					wrapper for Sleep() function on different OSs

void SerialSleep( int ms)
{
#ifdef _WIN32
	Sleep(ms);
#else
	nusleep(ms*1000);
#endif
}

// +Entry
//
// GetSocketFD		returns the socket descriptor used for network communication
//
// Returns the socket descriptor used for network communication, or -1 if a serial port is used

int GetSocketFD()
{
	if (use_network)
		return sockfd;
	else
		return -1;
}
