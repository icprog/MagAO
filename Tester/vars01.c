//@File: vars01.c
//
// The {\tt vars01} program tests RTDB functions related to lists
// of variables (selection via regular expressions and the like).
//
// It first creates a list of variables with random names, then allows
// to get information related to selected variables from the RTDB.
//@

#define VERSMAJ  1
#define VERSMIN  2

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <math.h>

#include <time.h>
#include <sys/time.h>

#include "base/msglib.h"

char *MyName = "VARS01";


//
//@Procedure: help
//
//  Prints an usage message
//@

static void help()
{
    printf("\nVARS01  - %s.   L.Fini\n\n",GetVersionID());
    printf("Tests functions related to multiple variables\n\n");
    printf("Usage: vars01 [-v] [<server>]\n\n");

    printf("       vars01 -h   print an help message\n\n");

    printf("       -v        verbose mode.\n");

    printf("       <server>  Server numeric address (default: \"127.0.0.1\")\n\n");

}

MsgBuf msgb;
int count;
int verbose=0;

#define MAX_REG 100

static char prefix[MAX_REG];

static int DisplayVariables()
{
    Variable *TheVar;
    int notend;

    notend=1;
    printf("\n");
    printf("Name                T Value\n");
    printf("-------------------+-+-----------------------------------------\n");
    while(notend) {
        if(IS_ERROR(WaitMsgFromServer(&msgb))) {
            printf("Error from server\n\n");
            return 0;
        }
        if(hdr_CODE(msgb)==VARREPLY) {
            TheVar=(Variable *)msg_BODY(msgb);

            printf("%-20s", TheVar->H.Name);
            if(TheVar->H.Type==INT_VARIABLE) 
	        printf("I %ld",TheVar->Value.Lv[0]);
            else if(TheVar->H.Type==REAL_VARIABLE) 
	        printf("R %.2f",TheVar->Value.Dv[0]);
            else if(TheVar->H.Type==CHAR_VARIABLE) 
	        printf("S %s",TheVar->Value.Sv);
	    else {
	        printf("Bad variable buffer format");
	    }
	    printf("\n");
        }  else if(hdr_CODE(msgb)==VAREND) {
	    printf("\n");
	    notend=0;
        }  else {
	        printf("Unexpected message from server:\n");
	        printf("    From: %s\n",hdr_FROM(msgb));
	        printf("      to: %s\n",hdr_TO(msgb));
	        printf("  SeqNum: %d\n",hdr_SEQN(msgb));
	        printf("    Code: %x\n",hdr_CODE(msgb));
	        printf("   pLoad: %d\n",hdr_PLOAD(msgb));
	        printf("  MsgLen: %d\n\n",hdr_LEN(msgb));
        }
    }
    return 0;
}
	 

//@Main: 

// Test Msgd-RTDB variable management
//@

static char *VarNames[]= { "X",        "Y",     "Z",       "A0",    "REGEXP", 
                           "ALPHA",    "DELTA", "BETA",    "KAPPA", "TEMP0", 
			   "TEMP1",    "YUP",   "AVERAGE", "SDEV",  "CCD-SIZE",
                           "CCD-DPTH", "UGO",   "PIPPO",   "WFS-T", "SBC_A",
};

static int Nvars = 20;

int main(int argc, char **argv) 
{
    int i,stat;
    char *server;

    SetVersion(VERSMAJ,VERSMIN);

    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-v")==0)
	    verbose=1;
	
	if(strcmp(argv[i],"-h")==0) {
	    help();
	    exit(0);
	}
    }

    if(i>=argc)
	server="127.0.0.1";
    else 
	server=argv[i++];


    if(verbose)
	printf("%s connecting to server at: %s ...",MyName,server);

                                  // Connect to MsgDRTDB server
    if(IS_ERROR(stat=ConnectToServer(server,MyName,1))) {
	printf("Error connecting to server at: %s\n",server);
	printf("Error code: %d - %s\n\n",stat,strerror(errno));
	return 1;
    }

    if(verbose)
	printf(" done!\n");

    InitMsgBuf(&msgb,0);         // Initialize the message structure

    for(i=0;i<Nvars;i++) {
	if(i%2) {
	    long VLong;
	    VLong=i*1000;
	    if(verbose)
		printf("Creating INT  variable: %s\n",VarNames[i]);
	    stat=CreaVar(MyName,VarNames[i],INT_VARIABLE,
		         "P",1,&msgb);
	    if(IS_ERROR(stat))
	        stat=SetVar(MyName,VarNames[i],INT_VARIABLE,
		         1,&VLong,&msgb);

	} else {
	    double VDouble;
	    VDouble=i+0.01;
	    if(verbose)
		printf("Creating REAL variable: %s\n",VarNames[i]);
	    stat=CreaVar(MyName,VarNames[i],REAL_VARIABLE,
		         "P",1,&msgb);
	    if(IS_ERROR(stat))
	        stat=SetVar(MyName,VarNames[i],REAL_VARIABLE,
		         1,&VDouble,&msgb);
	}
	if(IS_ERROR(stat))
	    printf("Error creating variable %s: %d\n",VarNames[i],stat);
    }

    while(1) {
	printf("Get variables matching (empty line exits): ");
	fgets(prefix,MAX_REG,stdin);
	if(*prefix=='\n') break;
	prefix[strlen(prefix)-1]='\0';
	stat=VarList(MyName,prefix,count,&msgb);
	if(IS_ERROR(stat)) 
	    printf("Error calling VarList: %d\n",stat);
	else {
	    if(verbose) printf("VarList request successfully sent\n");
	    DisplayVariables();
	}
    }
	
    return 0;
}
