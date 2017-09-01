// NetSerial.h
//
// Header file for the NetSerial library

#ifndef _NETSERIAL_H_INCLUDED
#define _NETSERIAL_H_INCLUDED

#ifdef _WIN32
#include "windows.h"
#else
#define HANDLE	int
#endif

// Function prototypes for serial/network communication

int SerialInit(const char *address, int port);
int SerialClose(void);

int SerialOut(const char *buf, int len);
int SerialIn( char *buf, int len, int timeout);

int SerialInString( char *buf, int len, int timeout, char terminator);
int SerialInString2( char *buf, int len, int timeout, char *terminator);


// Function prototypes for specific serial communications

HANDLE SerialSetup(const char *name, int baudrate, int databits, int parity, int stopbits);

int SerialRead( HANDLE port, char *buffer, int num);			// Serial
int SerialTransmit( HANDLE port,const char *buffer, int length);	// Serial
int SerialReceive( HANDLE port, char *buffer, int length);		// Serial
int SerialShutdown( HANDLE port);

void SerialSleep( int ms);		// OS-independent sleep
int GetSocketFD(void);



#endif /* _NETSERIAL_H_INCLUDED */
