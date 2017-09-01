//@File: logger.c
//
// The {\tt logger} program send a sequence of {\tt LOGMSG} commands to the
// \msgd\.
//@

#define VERSMAJ  2
#define VERSMIN  1

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


//@Entry: help
//
//  Prints an usage message
//
//@

static void help()
{
    printf("\nLOGGER  - Vers. %s.   L.Fini\n\n", GetVersionID());
    printf("Generates log messages and sends them to the MsgdRTDB.\n\n");
    printf("Usage: logger [-v] [-c cnt] [-d delay] <num> [<server>]\n\n");
    printf("       logger [-h]  print an help message.\n\n");
    printf("       -v        verbose mode.\n");
    printf("       -c        stop after cnt logs.\n");
    printf("       -d  delay set delay time (Seconds. Fractions allowed) [Def: 1 s].\n\n");

    printf("       <num>     Use different <num> for concurrent loggers: this\n");
    printf("                 argument is REQUIRED!\n");
    printf("       <server>  Server numeric address [Def: \"127.0.0.1\"]\n\n");
    printf("When in continuous mode (no -c option) the program must be stopped\n");
    printf("with Ctrl-C or sending the INT signal.\n\n");
}

struct LogItem {

  int level;
  char *msg;

} ;

struct LogItem logs [] = {
  { MASTER_LOG_LEV_INFO, "Log message N. 1"                              },
  { MASTER_LOG_LEV_FATAL, "Specific message for thread creation error" },
  { MASTER_LOG_LEV_INFO, "Log message N. 2. "       },
  { MASTER_LOG_LEV_WARNING, "This is a warning message" },
  { MASTER_LOG_LEV_INFO, "Auxliliary log"  },
  { MASTER_LOG_LEV_INFO, "Log message N. 3" },
  { MASTER_LOG_LEV_WARNING, "This is another warning message" },
  { MASTER_LOG_LEV_INFO, "Log message N. 4" },
  { MASTER_LOG_LEV_ERROR, "Error message text N. 1" },
  { MASTER_LOG_LEV_INFO, "Log message N. 5" },
  { MASTER_LOG_LEV_FATAL, "Fatal error message text N. 1" },
  { MASTER_LOG_LEV_INFO, "Log message N. 6" },
};

int Nlogs = sizeof(logs)/sizeof(struct LogItem);

int count=0;
int verbose;

//@Entry: terminate
//
//  Signal handler for terminating the program
//
//@

static void terminate()
{
                                               // Print statistics
    printf("\n%d log records sent by %s.\n",count,MySelf);

    exit(0);
}


static void sighand(int thesig)
{
   terminate();
}

//@Main: Logger
//
//  Logging function testing program
//
//  This program connects to the MsgD-RTDB process and sends LOGMSG
//  commands until stopped
//

int main(int argc, char **argv) 
{
    int stat,i;
    char *server;
    double secdelay=1.0;
    int docount;
    int nlogmsg=1;
    struct timespec delay;

    double fact=(double)Nlogs/(double)RAND_MAX * 0.999999;

    docount=RAND_MAX;

    SetVersion(VERSMAJ,VERSMIN);
    server=NULL;

    verbose=0;
    docount=0;

    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-v")==0) {
	    verbose=1;
	}
	if(strcmp(argv[i],"-c")==0) {
	    nlogmsg=atoi(argv[++i]);
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
	server="127.0.0.1";
    else 
	server=argv[i];

    delay.tv_sec=floor(secdelay);           // Set delay time 
    secdelay -= delay.tv_sec;
    delay.tv_nsec=(secdelay*1.0e+9);

    sprintf(MySelf,"LOGGER-%2.2d",stat);    // Set up client name

    signal(SIGINT,sighand);      // Install signal handler for INT

    if(verbose)  
	printf("%s starting\n", MySelf);

    if((stat=ConnectToServer(server,MySelf,1))!=NO_ERROR) {
	printf("Error connecting to server at: %s\n",server);
	printf("Error code: %d - %s\n\n",stat,strerror(errno));
	return 1;
    }

    while(nlogmsg) {                        // loop can be Interrupted by Ctrl-C
        char *msg; int nseq,level,nextlog;
        char loghdr[MASTER_LOG_HEADER_LEN];

	if(docount) nlogmsg--;

	nanosleep(&delay,NULL);             // Delay 
        nextlog=(int)(double)rand()*fact;
        level=logs[nextlog].level;
        msg=logs[nextlog].msg;
        nseq=logString(MySelf,level,NULL,loghdr);
        stat=LogMsg(MySelf,nseq,level,msg);
        
	if(verbose) {                   // Printout data 
            printf("%s Log N. %4d - Msg: %s\n",loghdr,count,msg);
	}

        if(stat!=NO_ERROR)  {
            printf("Error: %d\n",stat);
	    perror("");
	    terminate();
	}
        count ++;
    }
    terminate();
    return 1;
}
