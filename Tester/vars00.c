//@File: vars00.c
//
// The {\tt vars00} program first allocates a number of variables in the 
// Real-Time database and then randomly reads values of variables
// while collecting data to evaluate response times.
//@


#define VERSMAJ  1
#define VERSMIN  1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <signal.h>

#include <time.h>
#include <sys/time.h>

#include "base/msglib.h"

char MyName[10];


//
//+Entry help
//
//  help prints an usage message
//

static void help()
{
    printf("\nVARS00  - %s.   L.Fini\n\n",GetVersionID());
    printf("Elementary tests on variable creation and accessing\n\n");
    printf("Usage: vars00 [-c <cnt>] [-v] [-d delay] <Num> [<server>]\n\n");

    printf("       vars00 -h   print an help message\n\n");

    printf("       -v        verbose mode.\n");
    printf("       -c <cnt>  Stop after cnt read requests.\n");
    printf("       -d delay  Specify delay time (seconds. Fractions allowed).\n\n");

    printf("       <num>     Use different <num> for concurrent vars: this\n");
    printf("                 argument is REQUIRED!\n");
    printf("       <server>  Server numeric address (default: \"127.0.0.1\")\n\n");
    printf("The program must be stopped with Ctrl-C or sending the INT signal.\n\n");

}

//+Entry doquit
//
//  Signal handler for ctrl-C stopping
//

MsgBuf msgb;
double dsec,maxtime,mintime,avgtime;
long seconds,useconds,nsec,nusec;
struct timeval before,after;
int count;
int verbose=0,silent=0;


static void doquit(int thesig)
{
    if(count>0)
        avgtime = avgtime /(double)count;
    printf("\n %d messages. round-trip min/avg/max %f/%f/%f\n",
	    count,mintime,avgtime,maxtime);
    exit(0);
}



static int DisplayVariables()
{
    Variable *TheVar;

    if(WaitMsgFromServer(&msgb) !=NO_ERROR) {
        printf("Error from server\n\n");
        return 0;
    }
    gettimeofday(&after,NULL);

    if(hdr_CODE(msgb)==VARREPLY) {
        TheVar=(Variable *)msg_BODY(msgb);

	nsec=after.tv_sec-before.tv_sec;    // Compute delay
        nusec=after.tv_usec-before.tv_usec;
        dsec = (double)nsec*1000.0 +(double)nusec*0.001;
	if(dsec>maxtime) maxtime=dsec;      // Cumulate statistics
        if(dsec<mintime)mintime=dsec;
        avgtime += dsec;

        if(verbose) {
    	    int i;

            printf("%s cnt:%d time:%f (ms) Value: ",
		    TheVar->H.Name,count,dsec);
            if(TheVar->H.Type==INT_VARIABLE) 
                for(i=0;i<TheVar->H.NItems;i++)
		     printf(" %ld",TheVar->Value.Lv[i]);
            else if(TheVar->H.Type==REAL_VARIABLE) 
                for(i=0;i<TheVar->H.NItems;i++)
		     printf(" %.2f",TheVar->Value.Dv[i]);

            else if(TheVar->H.Type==CHAR_VARIABLE) 
                for(i=0;i<TheVar->H.NItems;i++)
		        printf(" %s",TheVar->Value.Sv);
	    else {
		printf("Bad variable buffer format\n");
	    }
	    printf("\n");
	}
    }  else {
	    printf("Unexpected message from server:\n");
	    printf("    From: %s\n",hdr_FROM(msgb));
	    printf("      to: %s\n",hdr_TO(msgb));
	    printf("  SeqNum: %d\n",hdr_SEQN(msgb));
	    printf("    Code: %x\n",hdr_CODE(msgb));
	    printf("   pLoad: %d\n",hdr_PLOAD(msgb));
	    printf("  MsgLen: %d\n\n",hdr_LEN(msgb));
    }
    return 1;
}
	 

//+Main: Test Msgd-RTDB variable management
//
//  This program connects to the MsgD-RTDB process to test the
//  variable management.
//
//  First it creates a number of variables, then accesses at random for 
//  reading
//

#define MAX_VARS 100

char VarName[MAX_VARS][VAR_NAME_LEN+1];

int main(int argc, char **argv) 
{
    int i,Nvars,stat;
    double rfact;
    double secdelay=1.0;
    char *server;
    int stopcnt=0, cnt=1;
    struct timespec delay;

    SetVersion(VERSMAJ,VERSMIN);

    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-v")==0)
	    verbose=1;
	
	if(strcmp(argv[i],"-c")==0) {
	    cnt=atoi(argv[++i]);
	    stopcnt=1;
	}
	if(strcmp(argv[i],"-s")==0)
	    verbose=0;
	
	if(strcmp(argv[i],"-h")==0) {
	    help();
	    exit(0);
	}
	if(strcmp(argv[i],"-d")==0)
	    secdelay=atof(argv[++i]);
	
    }

    if(i>=argc) {
	help();
	return 1;
    }
    stat=atoi(argv[i++]);

    if(i>=argc)
	server="127.0.0.1";
    else 
	server=argv[i++];


    Nvars=20;

    delay.tv_sec=floor(secdelay);           // Set delay time 
    secdelay -= delay.tv_sec;
    delay.tv_nsec=(secdelay*1.0e+9);

    signal(SIGINT,doquit);        // Install signal handler for quitting


    sprintf(MyName,"VARS00-%2.2d",stat);

    if(verbose)
	printf("%s connecting to server at: %s ...",MyName,server);

                                  // Connect to MsgDRTDB server
    if((stat=ConnectToServer(server,MyName,1))!=NO_ERROR) {
	printf("Error connecting to server at: %s\n",server);
	printf("Error code: %d - %s\n\n",stat,strerror(errno));
	return 1;
    }

    if(verbose)
	printf("done!\n");

    InitMsgBuf(&msgb,0);         // Initialize the message structure

    for(i=0;i<Nvars;i++) {
	int j;
	snprintf(VarName[i],VAR_NAME_LEN+1,"%s.%4.4d",MyName,i);
	if(i%2) {
	    long VLong[7];
	    for(j=0;j<7;j++) VLong[j]=i*1000+j;
	    if(verbose)
		printf("Creating INT variable: %s\n",VarName[i]);
	    stat=CreaVar(MyName,VarName[i],INT_VARIABLE,
		         "P",7,&msgb);
	    if(stat==NO_ERROR)
	        stat=SetVar(MyName,VarName[i],INT_VARIABLE,
		         7,VLong,&msgb);

	} else {
	    double VDouble[5];
	    for(j=0;j<5;j++) VDouble[j]=i+0.01*j;
	    if(verbose)
		printf("Creating REAL variable: %s\n",VarName[i]);
	    stat=CreaVar(MyName,VarName[i],REAL_VARIABLE,
		         "P",5,&msgb);
	    if(stat==NO_ERROR)
	        stat=SetVar(MyName,VarName[i],REAL_VARIABLE,
		         5,VDouble,&msgb);
	}
	if(IS_ERROR(stat))
	    printf("Error creating variable %s: %d\n",VarName[i],stat);
    }
	
    mintime=10000000;
    maxtime=0.0;
    avgtime=0.0;

    count=0;
    rfact=(double)Nvars/((double)(RAND_MAX)+1.0);

    if(verbose) printf("\n");
    while(cnt) {
	if(stopcnt) cnt--;
	nanosleep(&delay,NULL);
	i= random() * rfact;
	sprintf(VarName[i],"%s.%4.4d",MyName,i);

	gettimeofday(&before,NULL);
        stat=GetVar(MyName,VarName[i],count,&msgb);
	if(DisplayVariables()==0) break;
	count++;
    }
    doquit(0);
    return 0;
}
