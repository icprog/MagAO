
#include <stdio.h>
//#include <memory.h>
//#include <time.h>
#include <string.h>		// strncmp()
#include <stdlib.h>		// atoi()

typedef unsigned char BYTE;


// Windows-specific declarations


#include "netseriallib.h"
#include "errlib.h"
#include "idrivelib.h"

int debug=0;


#define Log(a)	a;





// Serial/Ethernet communication
//
// IDRIVE_ADDR can be an hostname/IP number or a serial port name like "COM1" (in this case, port number is ignored)

//#define IDRIVE_ADDR		"COM1"
#define IDRIVE_ADDR		"193.206.155.206"
#define IDRIVE_PORT		4660
#define IDRIVE_NUM		1


void SerialTest( int drive)
{
	int value;

	printf("Testing serial communication...\n");
//	SetValue( drive, 0x6040, 0, 0x0006);

	value = GetValue( drive, 0x6061, 0);

	printf("Got answer for control word: %d\n", value);
}

void main()
{
	char buffer[1024];
	int num,value;

	SerialInit( IDRIVE_ADDR, IDRIVE_PORT);

//	SerialTest( IDRIVE_NUM);
//	return;

	SetupiDrive( IDRIVE_NUM, "idrive-start.cfg");

	while( gets(buffer))
		{
//		if (buffer[0] == 0)
//			continue;

		if (strncmp( buffer, "go", 2) == 0)
			{
			num = atoi(buffer+2);
			printf("Moving by %d\n", num);
			MoveiDrive( IDRIVE_NUM, num);

			while( !EndMoving( IDRIVE_NUM))
				{
				printf("Moving: position %d\n", GetCurrentPosition( IDRIVE_NUM));
				Sleep(1000);
				}
			}
				
		else if (strncmp( buffer, "pos", 3) == 0)
			{
			printf("Current position: %d\n", GetCurrentPosition( IDRIVE_NUM));
			}
	
		else if (strncmp( buffer, "exit", 4) == 0)
			break;

		else if (strncmp( buffer, "read", 4) == 0)
			{
			if (strncmp( buffer+5, "0x", 2) == 0)
				sscanf( buffer+4, "%x", &num);
			else if (buffer[5] == 'P')
				num = atoi( buffer+6) + 0x2000;
			else
				num = atoi( buffer+4);

			value = GetValue( IDRIVE_NUM, num, 0);
			printf("Parameter %d (0x%02X) = %d (0x%02X)\n", num, num, value, value);
			}

		else if (strncmp( buffer, "write", 5) == 0)
			{
			printf("Not implemented\n");
			}

		else if (strncmp( buffer, "help", 4) == 0)
			{
			printf("Commands are:\n");
			printf("go <n>: move the stage by <n> steps (both positive and negative numbers)\n");
			printf("pos: print current position\n");
			printf("read <n>: read i-Drive paramater number <n> (decimal and hex are allowed)\n");
			printf("exit: quit program\n");
			printf("help: this file\n");
			}
		else
			printf("Unknown command ('help' lists available commands)\n");
		}



	printf("Closing serial communication\n");
	SerialClose();
}






void TestNetwork()
{
	unsigned int i;
	char buffer[1024];

	sprintf( buffer, "GET / HTTP/1.0%c%c%c%c", 0x0D, 0x0A, 0x0D, 0x0A);

	SerialOut( buffer, strlen(buffer));

	memset( buffer, 0, 1023);

	i=0;
	while(( SerialIn( buffer+i,0) >=0) && (i<1023))
		i++;

	printf("Buffer: %s\n", buffer);
}
