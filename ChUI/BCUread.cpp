//+File: BCUread.c
//
// The BCUread program reads a buffer of data from a BCU

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

extern "C" {
#include "shmlib.h"
#include "commlib.h"
#include "base/thrdlib.h"
}

#include "BcuLib/bcucommand.h"
#include "Logger.h"

const char *MyName = "BCUREAD";
const char *Author = "A. Puglisi";
int VersMajor = 1;
int VersMinor = 0;
const char *Date = "Sep 2003";

using namespace Arcetri;

//
//+Entry help
//
//  help prints an usage message
//

static void help()
{
    printf("\nBCUread  - Vers. %d.%d.   %s, %s\n\n",
	    VersMajor,VersMinor,Author,Date);
    printf("Usage: BCUread [-v] <bcu memory address> <word count> <type>\n\n");
    printf("    <type> can be DSP, SRAM or SDRAM\n");
    printf("    -v     verbose mode\n");
    printf("           ALL NUMBERS ARE HEXADECIMAL\n");
    printf("\n");
}


//+Main: 
//
// Getvar   Utility to ask the MsgD-RTDB the value of a variable 
//
// This program asks the MsgD-RTDB the value of a variable, and prints
// the answer to stdout. Array values are separated by spaces.
//

int main(int argc, char **argv) 
{
    unsigned int address;
    int len, i, opcode;
    int verbose=0;
    unsigned char *data;

   Logger *logger = Logger::get("MAIN", 3);
   logger->log( Logger::LOG_LEV_DEBUG, "Ciao");

   SetVersion(VersMajor,VersMinor);

    if (argc <4)
    	{
	help();
	exit(0);
	}

    // Process command options
    i=1;
    for(i=1;i<argc;i++) {     
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-v")==0)
	    verbose=1;
    
	}

    if ( argc -i != 3)
    	{
	help();
	exit(0);
	}

    // Read commad line parameters
    address = strtol(argv[i], NULL, 16);
    len = strtol(argv[i+1], NULL, 16);

    opcode = -1;

 	if (strcmp(  argv[i+2], "DSP") == 0)
		opcode = MGP_OP_RDSEQ_DSP;
	else if (strcmp( argv[i+2], "SRAM") == 0)
		opcode = MGP_OP_RDSEQ_SRAM;
	else if (strcmp( argv[i+2], "SDRAM") == 0)
		opcode = MGP_OP_RDSEQ_SDRAM;

	if (opcode == -1)
		{
		printf("Invalid type specified\n");
		help();
		exit(0);
		}

	printf("address %d len %d\n", address, len);

    int stat = thInit(MyName);
    printf("Stat1: %d\n", stat);

    stat = thStart("127.0.0.1", 2);
    printf("Stat2: %d\n", stat);


    if(verbose)
	printf("Sending message to server\n");

    data = (unsigned char *)malloc(len*4);

	printf("opcode %d\n", opcode);
    stat = thSendBCUcommand( MyName, 0, 255, 255, opcode, address, data, len*4, 1000, 0, 0);
    printf("Stat: %d\n", stat);

			for (i=0; i< len; i++)
				{
				if (i%8 == 0)
					printf("\n0x%08X:\n", address + i);

				printf("0x%08X ", ((int *)data)[i]);

                printf("%08d", ((int *)data)[i]);

				printf("%20.10f\n", ((float *)data)[i]);
				}

//			for (i=0; i<len*4; i++)
//				printf("%d\n", addr[i]);
			printf("\n");


    exit(0);
}



 
