//@File: simplevar.c
//
// Simple variable access test

// This program allocates a variable named TEST into the RTDB Writes a
// Value and then reads back the value written
//@

#define VERSMAJ  1
#define VERSMIN  2

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "base/msglib.h"

char *MyName="SIMPLEVAR";


//
//+Entry help
//
//  help prints an usage message
//

static void help()
{
    printf("\nSIMPLEVAR  - %s.   L.Fini\n\n",GetVersionID());
    printf("The simplest test on variable creation and accessing\n\n");
    printf("Usage: simplevar [server]\n\n");

    printf("       simplevar -h   print an help message\n\n");

    printf("       server         Server numeric address (default: \"127.0.0.1\")\n\n");

}


	 

//@Main: 

// Test Msgd-RTDB variable management
//
//  This program connects to the MsgD-RTDB process to test the
//  variable management.
//
//  First it creates a an integer variable, then writes a value to it
//  abd reads it back
//

char *VarName="INTEGER_VAR";

int main(int argc, char **argv) 
{
int i,stat;
long VLong[3]={19520709L, 19790126L, 19820414L };
long vbuf[3];
char *server="127.0.0.1";
MsgBuf msgb;

SetVersion(VERSMAJ,VERSMIN);

if(argc>1) {
    if(*argv[1] == '-') {
        help();
        exit(0);
    } else 
	server=argv[1];
}

printf("%s connecting to server at: %s ...",MyName,server);

                                  // Connect to MsgDRTDB server
if((stat=ConnectToServer(server,MyName,1))!=NO_ERROR) {
    printf("Error connecting to server at: %s\n",server);
    printf("Error code: %d - %s\n\n",stat,strerror(errno));
    return 1;
}
printf("done!\n");

InitMsgBuf(&msgb,0);         // Initialize the message structure

printf("Creating INT variable: %s\n",VarName);
stat=CreaVar(MyName,VarName,INT_VARIABLE,"P",3,&msgb);
if(stat!=NO_ERROR) {
    printf("Error creating variable: %d\n",stat);
    return 1;
}

printf("Setting Variable value to: %ld %ld %ld\n",VLong[0],VLong[1],VLong[2]);
stat=SetVar(MyName,VarName,INT_VARIABLE,3,&VLong,&msgb);
if(stat!=NO_ERROR) {
    printf("Error setting variable value: %d\n",stat);
    return 1;
}

printf("Requesting Variable value: ");
stat=ReadVar(MyName,VarName,&vbuf,3,0,NULL,&msgb);

if(stat < 0) {
    printf("Error from server %d\n\n",stat);
    return 0;
}

for(i=0;i<stat;i++) printf("%ld ",vbuf[i]);
printf("\n");

return 0;
}

