//@File: consumer.c
//
// Exercises the use of shared memory buffers for bulk data access.
//@

#define VERSMAJ   1
#define VERSMIN   1

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

#include "base/msglib.h"

#define BUF_LEN 1575

unsigned char buffer[BUF_LEN];

char MySelf[10];


//
//@Procedure help:
//
//  Prints an usage message
//@

static void help()
{
    printf("\nCONSUMER  - %s. L.Fini\n\n", GetVersionID());
    printf("Usage: consumer [-v] [-c cnt] [-s st] [-n nb] [-t msec] num bufnam [server]\n");
    printf("       consumer -h\n\n");
    printf("       -h      print an usage message.\n\n");
    printf("       -c cnt  exit after cnt reads.\n");
    printf("       -s st   Start block (Snapshot mode only).\n");
    printf("       -r nb   Number of blocks (Snapshot mode only).\n");
    printf("       -t msec Reply timeout (ms). Default: infinite.\n");
    printf("       -d sec  Delay time (sec. fractions allowed). Default 5 s.\n");
    printf("               Only in snapshot mode delays between Snapshot requests.\n");

    printf("       -v      verbose mode.\n");
    printf("       -w file save into file.\n");
    printf("       num     Use different <num> for concurrent consumers: this\n");
    printf("               argument is REQUIRED!\n");
    printf("       bufnam  name of the buffer to show (E.g: PRDCR-01:pippo)\n");
    printf("       server  Server numeric address (default: \"127.0.0.1\")\n");
    printf("               Note: currently network access is not supported\n\n");
}


double maxtime,mintime,avgtime;
MsgBuf msgb;
int verbose;
char *target;
int count=0;
int no_sync_cnt=0;
int all = 0;
int quiet =0;
BufInfo info;
char *filename=NULL;
FILE *fp=NULL;

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

    FreeMsgBuf(&msgb);     // This is not needed because the 
                             // program terminates
    if (fp)
       fclose(fp);

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

static void bufcont_loop(BufInfo *info, int tmout, int maxC)
{
int i,stat;
int cntr_expect= -1;
unsigned char *data;
int prev_time = -1;
int count_time = 0;
int freq =0;
int prev_counter=0;
int freq_hw=0;

no_sync_cnt=0;
data=malloc(info->com.lng);   // Allocate memory for buffer
if(data==NULL) {
  printf("Error allocating memory for data\n");
  printf("  SYSERROR:%s\n\n",strerror(errno));
  return;
}

if (filename) fp = fopen(filename, "w");

for(i=0;;count++) {	// Loop forever reading data from buffer
  int j;                // and showing the first 20 bytes

  if(maxC && (count>=maxC)) terminate();

  if((stat=bufRead(info,data,tmout))<0) {
    printf("Error reading from shared buffer\n");
    printf("Error code: %d (errno:%s)\n\n",CLEAN_ERROR(stat),strerror(errno));
    terminate();
  }
  if(cntr_expect>=0 && cntr_expect!=stat)  {
     no_sync_cnt++;
     if(verbose)
       printf("Received buffer #: %d  expected: %d\n",stat,cntr_expect);
  }
  cntr_expect=stat+1;
  if(verbose) {
        char header[MASTER_LOG_HEADER_LEN];
        int pippo = logString("", 1, NULL, header);


     printf("%s %s:%s #%d (%d Hz, hw rate: %d Hz):\n", header,info->com.producer,info->com.name,stat, freq, freq_hw);
     printf("First: ");
     for(j=0;j<10;j++) printf(" %08X", ((unsigned int *)data)[j]); 
     printf("\nLast:  ");
     for(j=10;j>0;j--) printf(" %08X", ((unsigned int *)data)[info->com.lng/4 -j]); 
     printf("\nSlopes:  ");
     for(j=0;j<10;j++) printf(" %08X", ((float *)data)[j+3200+8]); 
     printf("\n");

     if (all)
      {
       printf("All:\n");
        for (j=0; j< info->com.lng/4; j++)
            {
            if (j%8 == 0)
               printf("\n %09d: ", j); 
           
            printf(" %08X", ((unsigned int *)data)[j]);

            }
        printf("\n");
        }
  }

  if (quiet)
     printf("Read frame %d (%d Hz,  hw rate: %d Hz)\n", stat, freq, freq_hw);

  if (fp)
     fwrite(data, info->com.lng, 1, fp);

  // Compute frequency
  count_time++;
  if (time(NULL) > prev_time)
  {
     freq = count_time;
     freq_hw = ((int*)data)[0] - prev_counter;
     count_time =0;
     prev_time = time(NULL);
     prev_counter= ((int*)data)[0];  
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
       for(j=0;j<20;j++) printf(" %2.2X",data[j]); 
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
    int stat,i;
    int maxC,tmout;
    int start,numb;
    char *server;
    char *bufnam;
    double secdelay=5.0;
    struct timespec cpdelay;
    struct timespec *delay;

    SetVersion(VERSMAJ,VERSMIN);

    server=NULL;
    bufnam=NULL;

    verbose=0;
    maxC=0;
    tmout=0;
    start=(-1);
    numb=0;

    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-c")==0) { maxC=atoi(argv[++i]); continue; }
	if(strcmp(argv[i],"-t")==0) { tmout=atoi(argv[++i]); continue; }
	if(strcmp(argv[i],"-s")==0) { start=atoi(argv[++i]); continue; }
	if(strcmp(argv[i],"-d")==0) { secdelay=atof(argv[++i]); continue; }
	if(strcmp(argv[i],"-n")==0) { numb=atoi(argv[++i]); continue; }
	if(strcmp(argv[i],"-v")==0) { verbose=1; continue; }
	if(strcmp(argv[i],"-a")==0) { all=1; continue; }
	if(strcmp(argv[i],"-h")==0) { help(); exit(0); }
	if(strcmp(argv[i],"-q")==0) { quiet=1; continue; }
   if(strcmp(argv[i],"-w")==0) { filename=argv[++i]; continue; }
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

    sprintf(MySelf,"CNSMR-%2.2d",stat);    // Set up client name

    if(verbose)  
	printf("%s connecting to MsgD-RTDB\n", MySelf);

    if((stat=ConnectToServer(server,MySelf,1))!=NO_ERROR) {
	printf("Error connecting to server at: %s\n",server);
	printf("Error code: %d - %s\n\n",CLEAN_ERROR(stat),strerror(errno));
	return 1;
    }

    if((stat=bufRequest(MySelf,bufnam,BUFCONT,&info))!=NO_ERROR) {
	printf("Error attaching to shared buffer: %s\n",bufnam);
	printf("Error code: %d (errno:%s)\n\n",CLEAN_ERROR(stat),strerror(errno));
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

    signal(SIGINT,sighand);      // Install signal handler for INT

    if((info.com.accMode&ACCMODE_MODE_MASK)==BUFCONT)
       bufcont_loop(&info,tmout,maxC);
    else {
       if (start<0) start=0;
       if (numb==0) numb=info.com.maxSnap;

       bufsnap_loop(&info,start,numb,tmout,maxC,delay);
    }

return 0;
}
