//@File: consumer.c
//
// Exercises the use of shared memory buffers for bulk data access.

// This program has the same funcionality as consumer, but is
// implemented by means of the new thrdlib
//@

#define VERSMAJ  1
#define VERSMIN  1

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <math.h>

#include <time.h>
#include <sys/time.h>

#include "base/thrdlib.h"

#define BUF_LEN 1575

unsigned char buffer[BUF_LEN];

char MySelf[10];


//@Procedure: help
//
//  Prints an usage message
//@

static void help()
{
    printf("\nTHCONSUM  - %s.   L.Fini\n\n", GetVersionID());
    printf("Usage: thconsum [-v] [-m] [-c cnt] [-s st] [-n nb] [-t msec] num bufnam [server]\n");
    printf("       thconsum -h\n\n");
    printf("       -h      print an usage message.\n\n");
    printf("       -c cnt  exit after cnt reads.\n");
    printf("       -s st   Start block (Snapshot mode only).\n");
    printf("       -r nb   Number of blocks (Snapshot mode only).\n");
    printf("       -t msec Reply timeout (ms). Default: infinite.\n");
    printf("       -d sec  Delay time (sec. fractions allowed). Default 5 s.\n");
    printf("               Only in snapshot mode delays between Snapshot requests.\n");
    printf("       -m      Set debug mode in thrdlib.\n");
    printf("       -v      verbose mode.\n");
    printf("       num     Use different <num> for concurrent consumers: this\n");
    printf("               argument is REQUIRED!\n");
    printf("       bufnam  name of the buffer to show (E.g: PRDCR-01:pippo)\n");
    printf("       server  Server numeric address (default: \"127.0.0.1\")\n");
    printf("               Note: currently network access is not supported\n\n");
}


double maxtime,mintime,avgtime;
int verbose;
char *target;
int count=0;
int no_sync_cnt=0;
BufInfo info;

//@Procedure: terminate
//
//  Compute statistics and terminate
//@

static void terminate()
{
                                               // Print statistics
    printf("\n%s - Buffer reads: %d  overruns: %d\n",MySelf,count,no_sync_cnt);
    printf("Detaching from buffer %s:%s - at slot: %d\n",
                                                 info.com.producer,
                                                 info.com.name,
                                                 info.com.slot);
    thClose();
    exit(0);
}

//@Entry: sighand
//
//  Signal handler for terminating the program
//@

static void sighand(int thesig)
{
   terminate();
}


static int gen_handler(MsgBuf *msgb, void *argp,int nq)
{
    printf("Message received:\n");
    printf("    From: %10s\n",HDR_FROM(msgb));
    printf("      to: %10s\n",HDR_TO(msgb));
    printf("  SeqNum: %d\n",HDR_SEQN(msgb));
    printf("    Code: %x\n",HDR_CODE(msgb));
    printf("  MsgLen: %d\n\n",HDR_LEN(msgb));
    thRelease(msgb);

return NO_ERROR;
}


static void bufcont_loop(BufInfo *info, int tmout, int maxC)
{
int i,stat;
int cntr_expect= -1;
unsigned char *data;

no_sync_cnt=0;
data=malloc(info->com.lng);   // Allocate memory for buffer
if(data==NULL) {
  printf("Error allocating memory for data\n");
  printf("  SYSERROR:%s\n\n",strerror(errno));
  return;
}

for(i=0;;count++) {	// Loop forever reading data from buffer
  int j;                // and showing the first 20 bytes

  if(maxC && (count>=maxC)) terminate();

  if((stat=bufRead(info,data,tmout))<0) {
    if(stat==TIMEOUT_ERROR) {
        printf("Timeout!\n");
        continue;
    } else {
        printf("Error reading from shared buffer\n");
        printf("Error code: %d (errno:%s)\n\n",CLEAN_ERROR(stat),strerror(errno));
        terminate();
    }
  }
  if(cntr_expect>=0 && cntr_expect!=stat)  {
     no_sync_cnt++;
     if(verbose)
       printf("Received buffer #: %d  expected: %d\n",stat,cntr_expect);
  }
  cntr_expect=stat+1;
  if(verbose) {
     printf("%s:%s #%d:",info->com.producer,info->com.name,stat);
     for(j=1;j<20;j++) printf(" %2.2X",data[j]); 
     printf("\n");
  }
}
}


static void bufsnap_loop(BufInfo *info, int start, int numb, 
                         int tmout, int maxC,  struct timespec *delay)
{
int i,stat;
unsigned char *data;

no_sync_cnt=0;
data=malloc(info->com.lng);   // Allocate memory for buffer
if(data==NULL) {
  printf("Error allocating memory for data\n");
  printf("  SYSERROR:%s\n\n",strerror(errno));
  return;
}

for(i=0;;count++) {	// Loop forever reading data from buffer
  int j,nb;             // and showing the first 20 bytes

  if(maxC && (count>=maxC)) terminate();

  if((stat=bufSnap(MySelf,info,start,numb))!=NO_ERROR) {
      printf("Error requesting snapshot\n");
      printf("Error code: %d (errno:%s)\n\n",CLEAN_ERROR(stat),strerror(errno));
      terminate();
  }

  for(nb=0;nb<numb;nb++) {
    if((stat=bufRead(info,data,tmout))<0) {
      printf("Error reading from shared buffer\n");
      printf("Error code: %d (errno:%s)\n\n",CLEAN_ERROR(stat),strerror(errno));
      terminate();
    }
    if(stat!=nb) {
     no_sync_cnt++;
     if(verbose)
       printf("Received buffer #: %d  expected: %d\n",stat,nb);
    }
    if(verbose) {
       printf("%s:%s #%d:",info->com.producer,info->com.name,stat);
       for(j=1;j<20;j++) printf(" %2.2X",data[j]); 
       printf("\n");
    }
  }
  if(delay) nanosleep(delay,NULL);             // Delay 
}
}

//@Main: 
//
//  MsgD-RTDB testing program
//
//  This program connects to the MsgD-RTDB to access a (previously
//  defined) shared memory buffer
//@

int main(int argc, char **argv) 
{
    int stat,i,thdeb;
    int maxC,tmout;
    int start,numb;
    char *server;
    char *bufnam;
    double secdelay=5.0;
    struct timespec cpdelay;
    struct timespec *delay;

    server=NULL;
    bufnam=NULL;

    verbose=0;
    thdeb=0;
    maxC=0;
    tmout=0;
    start=(-1);
    numb=0;

    SetVersion(VERSMAJ,VERSMIN);

    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-c")==0) { maxC=atoi(argv[++i]); continue; }
	if(strcmp(argv[i],"-t")==0) { tmout=atoi(argv[++i]); continue; }
	if(strcmp(argv[i],"-s")==0) { start=atoi(argv[++i]); continue; }
	if(strcmp(argv[i],"-d")==0) { secdelay=atof(argv[++i]); continue; }
	if(strcmp(argv[i],"-n")==0) { numb=atoi(argv[++i]); continue; }
	if(strcmp(argv[i],"-v")==0) { verbose=1; continue; }
	if(strcmp(argv[i],"-m")==0) { thdeb=1; continue; }
	if(strcmp(argv[i],"-h")==0) { help(); exit(0); }
    }

    if(i>=argc) { help(); return 1; }
    stat=atoi(argv[i++]);

    if(i>=argc) { help(); return 1; }
    bufnam = argv[i++];

    if(i>=argc)
	server="127.0.0.1";
    else 
	server=argv[i];

    if(secdelay>0.0) {                   // This block meaningful for snapshot
                                         // mode only ....
        cpdelay.tv_sec=floor(secdelay);  // Set delay time 
        secdelay -= cpdelay.tv_sec;
        cpdelay.tv_nsec=(secdelay*1.0e+9);
        delay= &cpdelay;
    } else
        delay=NULL;                      // No delay

    sprintf(MySelf,"THCONS-%2.2d",stat);    // Set up client name

    printf("%s starting",MySelf);

    signal(SIGINT,sighand);      // Install signal handler for INT

    if(thdeb) {
	if(verbose) printf("Setting thrdlib debug mode\n");
        thDebug(1);
    }

    if(verbose)  
	printf("Initializing thrdlib\n");

    if(IS_ERROR(stat=thInit(MySelf))) {
        printf("Error %d (%s) from thInit() .. exiting", stat,lao_strerror(stat));
        exit(1);
    }
    if(verbose)  
        printf("Installing generic handler ...\n");
    if((stat=thHandler(ANY_MSG,"*",0,gen_handler,"gen_handler",NULL))<0) {
        printf("Error %d (%s) from thHandler() .. exiting", stat,lao_strerror(stat));
        exit(1);
    }

    if(verbose)  printf("Starting thrdlib ...\n");
    if(IS_ERROR(stat=thStart(server,5))) {
        printf("Error %d (%s) from thStart() .. exiting", stat,lao_strerror(stat));
        exit(1);
    }
    if(verbose) printf("done!\n\n");

    if((stat=bufRequest(MySelf,bufnam,BUFCONT,&info))!=NO_ERROR) {
	printf("Error attaching to shared buffer: %s\n",bufnam);
	printf("Error code: %d (errno:%s)\n\n",CLEAN_ERROR(stat),strerror(errno));
        thClose();
	return 1;
    }

    if(verbose) {
      char *pp;
      if((info.com.accMode&ACCMODE_MODE_MASK)==BUFCONT)
        pp="continous";
      else
        pp="snapshot";
      printf("Connected to %s mode shared buf: %s:%s Slot:%d Size:%d MagicNum:%d\n",
              pp,info.com.producer,info.com.name,info.com.slot,info.com.lng,info.com.uniq);
    }

    if((info.com.accMode&ACCMODE_MODE_MASK)==BUFCONT)
       bufcont_loop(&info,tmout,maxC);
    else {
       if (start<0) start=0;
       if (numb==0) numb=info.com.maxSnap;

       bufsnap_loop(&info,start,numb,tmout,maxC,delay);
    }

return 0;
}
