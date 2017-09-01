//@File: pinger.c
//
// The {\tt pinger} program send a sequence of {\tt ECHO} commands and collects
// data to evaluate response times.
// 
// {\bf Note}: The \msgd\  has been coded to reply to {\tt ECHO} commands with
// an echo, so that {\tt pinger} can evaluate response times of the 
// \msgd\  itself, other than the roud-trip time with another Component by 
// pinging an {\tt echoer} component.
//@

#define VERSMAJ  1
#define VERSMIN  2


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <math.h>

#include <time.h>
#include <sys/time.h>

#include "base/msglib.h"

char MySelf[10];


//@Preocedure: help
//
//  help prints an usage message
//@

static void help()
{
    printf("\nPINGER  - %s.   L.Fini\n\n", GetVersionID());
    printf("Usage: pinger [-v] [-c cnt] [-d delay] [-l len] <num> [<target>] [<server>]\n\n");
    printf("       pinger [-h]  print an help message.\n\n");
    printf("       -v        verbose mode (twice to get debugging log).\n");
    printf("       -c        stop after cnt pings.\n");
    printf("       -l        set message body length [Def: 64].\n");
    printf("       -d  delay set delay time (Seconds. Fractions allowed) [Def: 1 s].\n\n");

    printf("       <num>     Use different <num> for concurrent pingers: this\n");
    printf("                 argument is REQUIRED!\n");
    printf("       <target>  Target name (default: \"MSGDRTDB\")\n");
    printf("       <server>  Server numeric address [Def: \"127.0.0.1\"]\n\n");
    printf("When in continuous mode (no -c option) the program must be stopped\n");
    printf("with Ctrl-C or sending the INT signal.\n\n");
}


int count=0;
double maxtime,mintime,avgtime;
MsgBuf msgb;
int verbose;
char *target;

//@Procedure: sendping
//
//  Signal handler for terminating the program
//@

static void terminate()
{
    if(count>0) {
        avgtime = avgtime /(double)count;      // Compute average delay
    }
                                               // Print statistics
    printf("\n%s %d messages. round-trip min/avg/max %.3f/%.3f/%.3f (ms)\n",
            MySelf,count,mintime,avgtime,maxtime);

    FreeMsgBuf(&msgb);     // This is not needed because the 
                             // program terminates
    exit(0);
}


static void sighand(int thesig)
{
   terminate();
}

//@Main 
//
// Pinger  MsgD-RTDB testing program
//
//  This program connects to the MsgD-RTDB process and sends ECHO
//  commands until stopped
//@

int main(int argc, char **argv) 
{
    int stat,i;
    char *server;
    struct timeval before,after;
    double dsec,secdelay=1.0;
    long nsec,nusec;
    int pinglen,docount;
    int npings=1;
    struct timespec delay;

    server=NULL;
    target=NULL;

    SetVersion(VERSMAJ,VERSMIN);

    pinglen=64;
    verbose=0;
    docount=0;

    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-v")==0) {
            if(verbose) MsgDebug(1);
	    verbose=1;
	}
	if(strcmp(argv[i],"-l")==0) {
	    pinglen=atoi(argv[++i]);
	}
	if(strcmp(argv[i],"-c")==0) {
	    npings=atoi(argv[++i]);
	    docount=1;
	}
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
	target="";
    else 
	target=argv[i++];

    if(i>=argc)
	server="127.0.0.1";
    else 
	server=argv[i];

    delay.tv_sec=floor(secdelay);           // Set delay time 
    secdelay -= delay.tv_sec;
    delay.tv_nsec=(secdelay*1.0e+9);

    sprintf(MySelf,"PINGER-%2.2d",stat);    // Set up client name

    signal(SIGINT,sighand);      // Install signal handler for INT

    if(verbose)  
	printf("%s to %s (body length: %d)\n",
		MySelf, target, pinglen);

    if((stat=ConnectToServer(server,MySelf,1))!=NO_ERROR) {
	printf("Error connecting to server at: %s\n",server);
	printf("Error code: %d - %s\n\n",stat,strerror(errno));
	return 1;
    }

    InitMsgBuf(&msgb,pinglen);         // Initialize message buffer with
                                       // proper length
    FillHeader(0,NewSeqN(),MySelf,"",SETREADY,&msgb);
    stat=SendToServer(&msgb);          // Notify I'm ready

    mintime=10000000;                  // Initialize statistics
    maxtime=0.0;
    avgtime=0.0;

    while(npings) {                      // loop can be Interrupted by Ctrl-C
	if(docount) npings--;

	nanosleep(&delay,NULL);             // Delay 
        FillHeader(pinglen,NewSeqN(),MySelf,    // Prepare message
		   target,ECHO,&msgb);   
        gettimeofday(&before,NULL);         // Get time before
        stat=SendToServer(&msgb);           // Send the message
        stat=WaitMsgFromServer(&msgb);      // Wait for reply
        gettimeofday(&after,NULL);          // Get time after

        nsec=after.tv_sec-before.tv_sec;    // Compute delay
        nusec=after.tv_usec-before.tv_usec;
        dsec = (double)nsec*1000.0 +(double)nusec*0.001;

        if(stat==NO_ERROR)  {
	    if(verbose) {                   // Printout data 
                printf("%d bytes back from %s - code:%-6x pLoad:%-6d  Flg:%2.2x SeqN:%d  time:%f ms\n",
    	            hdr_LEN(msgb),
    	            hdr_FROM(msgb),
    	            hdr_CODE(msgb),
    	            hdr_PLOAD(msgb),
    	            hdr_FLAGS(msgb),
    	            hdr_SEQN(msgb),
                    dsec);
	    }
            if(dsec>maxtime) maxtime=dsec;      // Cumulate statistics
            if(dsec<mintime)mintime=dsec;
            avgtime += dsec;
        } else {
            printf("Error: %d\n",stat);
	    perror("");
	    terminate();
	}
        count ++;
    }
    terminate();
    return 1;
}
