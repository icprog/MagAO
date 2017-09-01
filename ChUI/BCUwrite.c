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

#include "base/errlib.h"
#include "base/msglib.h"
#include "base/rtdblib.h"
#include "commlib.h"
#include "shmlib.h"
#include "BcuLib/BCUmsg.h"

char *MyName = "BCUREAD";
char *Author = "A. Puglisi";
int VersMajor = 1;
int VersMinor = 0;
char *Date = "Sep 2003";

//
//+Entry help
//
//  help prints an usage message
//

static void help()
{
    printf("\nBCUread  - Vers. %d.%d.   %s, %s\n\n",
	    VersMajor,VersMinor,Author,Date);
    printf("Usage: BCUread [-v] [-s <Server Address>] <bcu memory address> <word count> <type> <value1> <value2>\n\n");
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
    MsgBuf msgb;
    unsigned int address, len, value1, value2;
    int shm_id, shm_key;
    int stat,i,seqnum, opcode, err;
    int verbose=0;
    BCUmessage BCUmsg;
    Header *hdr;
    char *server = NULL;
    unsigned char *addr = NULL;


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
    
	if (strcmp( argv[i], "-s") ==0)
		{
		if (++i >= argc)
			break;
		server = argv[i];
		}
	}

    if ( argc -i != 5)
    	{
	help();
	exit(0);
	}

    // Read commad line parameters
    address = strtol(argv[i], NULL, 16);
    len = strtol(argv[i+1], NULL, 16);
    value1 = strtol(argv[i+3], NULL, 16);
    value2 = strtol(argv[i+4], NULL, 16);

    opcode = -1;

    if (strcmp(  argv[i+2], "DSP") == 0)
	opcode = MGP_OP_WRSEQ_DSP;
    else if (strcmp( argv[i+2], "SRAM") == 0)
        opcode = MGP_OP_WRSEQ_SRAM;
    else if (strcmp( argv[i+2], "SDRAM") == 0)
	opcode = MGP_OP_WRSEQ_SDRAM;
    else if (strcmp( argv[i+2], "WAVEGEN") == 0)
	opcode = MGP_OP_WRITE_WAVEGEN_RAM;
    else if (strcmp( argv[i+2], "RELAY") == 0)
	opcode = MGP_OP_WRRD_RELAIS_BOARD;

	if (opcode == -1)
		{
		printf("Invalid type specified\n");
		help();
		exit(0);
		}

	printf("opcode %d\n", opcode);
	printf("address %d len %d\n", address, len);

    if (!server)
	server ="127.0.0.1";    // If not specified, server IP defaults to 127.0.0.1

    if(verbose)
	printf("%s connecting to %s\n",MyName,server);

                                // Connect to server (waiting for the 
                                // server to start)
    if((stat=ConnectToServer(server,MyName,0))!=NO_ERROR) {
	printf("Error connecting to server at: %s\n",server);
	printf("Error code: %d - %s",stat, lao_strerror(stat));
	if(SYSTEM_ERROR(stat))
	    printf(" - %s",strerror(errno));
	printf("\n\n");
	return 1;
    }


    if(verbose)
	printf("Sending message to server\n");

    	shm_key = 0x23349;
    	shm_id = shmget( shm_key, len*4,  IPC_CREAT | IPC_EXCL | 0777);
	addr = (unsigned char *)shmat( shm_id, 0, 0);

	*((unsigned int *)addr) = value1; //0x01; // value;

	*(((unsigned int *)addr)+1) =  value2; // 0xFFFFFFFF;

//	for (i=0; i<len*4; i++)
//		addr[i] = (i%256);

	BCUmsg.BCUnumber = 0;
	strcpy( BCUmsg.Client, MyName);
	BCUmsg.firstDSP = 0x0FF;
	BCUmsg.lastDSP = 0x0FF;
	BCUmsg.command = opcode;
	BCUmsg.address = address;
	BCUmsg.flags = 0;
	BCUmsg.len = len*4;
	BCUmsg.buffer_id = shm_id;

	seqnum = getpid();

	InitMsgBuf(&msgb,0);

	FillMessage( sizeof(BCUmsg), seqnum, MyName, "MIRCTRL39", BCUMSG, &BCUmsg, &msgb);

	SendToServer(&msgb);

	FreeMsgBuf(&msgb);

	

        if(verbose)
	    printf("Waiting for an aswer\n");

	InitMsgBuf( &msgb, 0);

	while(1)
		{
		if ((err = WaitMsgTimeout(&msgb, 2000)) != NO_ERROR)
			{
			fprintf( stderr, "Error: %s\n", lao_strerror(err));
			if (verbose)
				printf("Exiting with error %d: %s\n", err, lao_strerror(err));
			FreeMsgBuf(&msgb);
			shmdt(addr);
			shmctl(shm_id, IPC_RMID, NULL);
			exit(0);
			}
		if (verbose)
		    printf("Got packet, analysing...\n");

		hdr = &(msgb.Msg->HD);

		if ( hdr->Code == BCUMSG)
			{
			printf("Got answer\n");

			for (i=0; i< len; i++)
				{
				printf("0x%08X ", ((int *)addr)[i]);

				printf("%20.10f\n", ((float *)addr)[i]);
				if (i%8 == 7)
					printf("\n");
				}

//			for (i=0; i<len*4; i++)
//				printf("%d\n", addr[i]);
			printf("\n");

			FreeMsgBuf(&msgb);

			shmdt(addr);
			shmctl(shm_id, IPC_RMID, NULL);
			exit(0);
			}
		} 


    exit(0);
}



 
