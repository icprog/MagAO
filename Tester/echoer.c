//@File: echoer.c
//
// MsgD-RTDB Test program

// The {\tt echoer} program echoes every message it receives back to 
// the sender.
//@

#define VERSMAJ   1
#define VERSMIN   5

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "base/msglib.h"

char *MyName = "ECHOER";


//@Entry: help
//
//  help prints an usage message
//@

static void help()
{
    printf("\nECHOER  - %s.   L.Fini\n\n",GetVersionID());
    printf("Echoes back any message received. If the message is an ECHO request,");
    printf("replies with an ECHORPLY code.\n\n");
    printf("Usage: echoer [-v] [-d s] [<Server Address>]\n\n");
    printf("       echoer -h\n\n");
    printf("   -d    delay s seconds before replying (fractions allowed)\n\n");
    printf("   -v    verbose mode\n\n");
    printf("   -h    prints help page and exit\n\n");
    printf("<Server Address> defaults to 127.0.0.1\n\n");
}


//@Main: 
//
// Echoer  MsgD-RTDB testing program
//
//  This program echoes back all messages received.
//  until stopped
//@

int main(int argc, char **argv) 
{
    int stat,i;
    int verbose=0;
    MsgBuf msgb;
    char *server;
    double delay=0.0;
    int dodelay=0;
    struct timespec dspec;

    SetVersion(VERSMAJ,VERSMIN);

    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-h")==0) {
	    help();
	    exit(0);
	}

	if(strcmp(argv[i],"-v")==0)
	    verbose=1;

	if(strcmp(argv[i],"-d")==0) 
	    delay=atof(argv[++i]);
    }

    if(i>=argc)                 // If no argument, Server IP defaults to
	server ="127.0.0.1";    // 127.0.0.1
    else
	server=argv[i++];       // First argument is Server IP address

    if(delay>0.0) {
        dodelay=1;
        dspec.tv_sec=delay;
        dspec.tv_nsec=(delay-dspec.tv_sec)*1000000000.0;
        if(dspec.tv_nsec>999999999) dspec.tv_nsec=999999999;
        if(verbose) {
            int aux=dspec.tv_nsec/1000;
            printf("Delaying replies by %d.%6.6d\n",(int)dspec.tv_sec,aux);
        }
    }
    if(verbose)
	printf("%s connecting to %s\n",MyName,server);

                                // Connect to server (waiting for the 
                                // server to start)
    if((stat=ConnectToServer(server,MyName,0))!=NO_ERROR) {
	printf("Error connecting to server at: %s\n",server);
	printf("Error code: %d",stat);
	if(SYSTEM_ERROR(stat))
	    printf(" - %s",strerror(errno));
	printf("\n\n");
	return 1;
    }

    InitMsgBuf(&msgb,0);      // Initialize message buffer.
                              // Initial message body length is set to 0
                              // It will be extended as needed when reading
                              // messages

    FillHeader(0,NewSeqN(),MyName,"",SETREADY,&msgb);
    stat=SendToServer(&msgb);          // Notify I'm ready

    if(verbose)
	printf("Waiting messages from server\n");
    for(;;) {                              // Begin infinite loop
        stat=WaitMsgFromServer(&msgb);    // Wait a message from server

	if(stat==NO_ERROR)  {
	    if(verbose) {
	        printf("Received From:%s Code:%x pLoad:%d Ttl:%d SeqN:%d Length:%d\n", 
		        hdr_FROM(msgb), hdr_CODE(msgb), hdr_PLOAD(msgb), hdr_TTL(msgb), hdr_SEQN(msgb), hdr_LEN(msgb));
	    }
	} else
	    printf("Error: %d\n",stat);

	ReverseHeader(&msgb);           // Swap FROM and TO fields in header
        if(hdr_CODE(msgb)==ECHO) SetMsgCode(ECHOREPLY,&msgb);
        if(hdr_PLOAD(msgb)!=0)     // setting Pload to n ot zero, prevents setting of REPLY flag
                                   // in reply message: used to test message expiration in receive queue
            CLEAR_REPLY(&msgb);
        else
            SET_REPLY(&msgb);

        if(dodelay) nanosleep(&dspec,NULL);

        if(verbose) {
            printf("Sending To:%s Code:%x pLoad:%d Ttl:%d SeqN:%d Length:%d\n", 
		    hdr_TO(msgb), hdr_CODE(msgb), hdr_PLOAD(msgb), hdr_TTL(msgb), hdr_SEQN(msgb), hdr_LEN(msgb));
        }
	stat=SendToServer(&msgb);       // Send back message
	if(stat!=NO_ERROR)  {
	    printf("Error from SendToServer(): %d\n",stat);
            exit(1);
        }
    }
    return 1;
}
