//@File: killer.c
//
//  Send a terminate command to {\msgd\}

//  The {\tt killer} program has the only purpose to send to \msgd\  the special
//  {\tt TERMINATE} command which orderly terminates the process.

//@

#define VERSMAJ  1
#define VERSMIN  1

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "base/msglib.h"

char *MyName = "KILLER";


//
//+Entry help
//
//  help prints an usage message
//

static void help()
{
    printf("\nKILLER  - %s.   L.Fini\n\n",GetVersionID());
    printf("Usage: killer <MsgDRTDB IP Address>\n");
    printf("       killer -h\n\n");
}


//+Main: 
//
// Killer  MsgD-RTDB testing program
//
//  This program sends the terminating command to MsgD-RTDB
//

int main(int argc, char **argv) 
{
    int stat,i;
    MsgBuf msgb;
    char *server;

    SetVersion(VERSMAJ,VERSMIN);

    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-h")==0) {
	    help();
	    exit(0);
	}
    }

    if(i>=argc)                 // If no argument, Server IP defaults to
	server ="127.0.0.1";    // 127.0.0.1
    else
	server=argv[i++];       // First argument is Server IP address

                                // Connect to server
    if((stat=ConnectToServer(server,MyName,1))!=NO_ERROR) {
	printf("Error connecting to server at: %s\n",server);
	printf("Error code: %d - %s\n\n",stat,strerror(errno));
	return 1;
    }

    InitMsgBuf(&msgb,0);        // Initialize message buffer.
                                // The TERMINATE message has no body, thus
                                // the message buffer length is 0

    FillHeader(0,0,MyName,"",           // Fill message header for the
	           TERMINATE,&msgb);    // terminate command using symbols
                                        // defined in "msglib.h"

     stat=SendToServer(&msgb);              // Send message to server
    if(stat!=NO_ERROR)  
        printf("Error from SendToServer(): %d\n",stat);
   
    return 1;
}
