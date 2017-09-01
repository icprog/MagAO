//@File: producer.c
//
// Exercises the use of shared memory buffers for bulk data access.
//@

#define VERSMAJ  1
#define VERSMIN  4

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

char MySelf[10];


//
//@Procedure help:
//
//  Prints an usage message
//@

static void help()
{
    printf("\nPRODUCER  - %s.   L.Fini\n\n", GetVersionID());
    printf("Usage: producer [-cont] [-v] [-c cnt] [-d secs] num bufnam [len]\n");
    printf("       producer -snap [-v] [-c cnt] [-n nb] num bufnam [len]\n");
    printf("       producer -h\n\n");
    printf("       -cont  Continuous mode (default).\n");
    printf("       -snap  Snapshot mode. \n\n");
    printf("       -c num Stop after num cycles. Default=0: goon until stopped by CTRL-C\n");
    printf("       -d sec Set delay time (Seconds. Fractions allowed) Default 1 s.\n");
    printf("              Continous mode only; in snapshot mode no delay is added.\n");
    printf("       -m     manual mode; in continuous mode wait for <CR> to generate\n");
    printf("              a new buffer of data. Delay is ignored.\n");
    printf("       -n nb  Number of buffers (snapshot mode only, default:10).\n");
    printf("       -r file Read data from file instead of generating random data\n");
    printf("       -h     print an usage message.\n");
    printf("       -v     verbose mode.\n");
    printf("       num    Use different <num> for concurrent producers: this\n");
    printf("              argument is REQUIRED!\n");
    printf("       bufnam name of the buffer to define (REQUIRED)\n");
    printf("       len    buffer length (default:1500)\n\n");
}


int count=0;
double maxtime,mintime,avgtime;
MsgBuf msgb;
int verbose;
char *target;
struct timeval start,stop;
char *filename = NULL;
FILE *fp=NULL;

//
//@Entry: terminate
//
//  Compute statistics and terminate
//@

static void terminate()
{
    long nsec,nusec;
    double dsec,rate=0;

    gettimeofday(&stop,NULL);         // Get stop time 
    nsec=stop.tv_sec-start.tv_sec;    // Compute delay
    nusec=stop.tv_usec-start.tv_usec;
    dsec = (double)nsec +(double)nusec*0.000001;

    if(count>0) {
        rate=(double)count/dsec;
        avgtime = avgtime /(double)count;      // Compute average delay
    }
                                               // Print statistics
    printf("\n%s %d messages (%.2f/sec). round-trip min/avg/max %.3f/%.3f/%.3f (ms)\n",
            MySelf,count,rate,mintime,avgtime,maxtime);

    CloseServer(MySelf);

    FreeMsgBuf(&msgb);     // This is not actually needed because the 
                           // program terminates
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


//@Procedure: bufcont_loop

// Write loop in BUFCONT mode

//@

static void bufcont_loop(BufInfo *info,
                         unsigned char * data, 
                         int buflen, 
                         struct timespec *delay,
                         int manual,
                         int nloop)
{
struct timeval before,after;
double dsec;
long nsec,nusec;
int i,j,stat,cntr;
int cnt,step;

if(nloop>0) {
   cnt=nloop;
   step=1;
} else {
   cnt=1;
   step=0;
}

if (filename) fp = fopen(filename, "r");
  
j=0;cntr=0;
for(;cnt>0;cnt-=step) {		// Loop forever writing variable buffer data
    if (fp) {
       fread(data, buflen, 1, fp);
       if (feof(fp)) {
          fclose(fp);
          fp = fopen(filename, "r");
       }
    } else {
       for(i=0;i<buflen;i++)
         data[i]= (i+j)%256;
    }

    if(manual) {
        printf("Press <CR>");
        getchar();
    } else
        if(delay) nanosleep(delay,NULL);             // Delay 

    gettimeofday(&before,NULL);         // Get time before writing
    if((stat=bufWrite(info,data,cntr,0))!=NO_ERROR) {
        printf("Error writing buffer data to: %s:%s\n",info->com.producer,info->com.name);
        printf("Error code: %d (errno:%s)\n\n",CLEAN_ERROR(stat),strerror(errno));
        break;
    }
    gettimeofday(&after,NULL);          // Get time after
    nsec=after.tv_sec-before.tv_sec;    // Compute delay
    nusec=after.tv_usec-before.tv_usec;
    dsec = (double)nsec*1000.0 +(double)nusec*0.001;

    if(verbose) {                   // Printout data 
        printf("Written %d bytes to %s:%s - seq:%d  time:%7.3f ms\n",
                     buflen,MySelf,info->com.name,cntr,dsec);
    }
    if(dsec>maxtime) maxtime=dsec;      // Cumulate statistics
    if(dsec<mintime)mintime=dsec;
    avgtime += dsec;
    j = (j+1)%256;
    cntr++;
    count++;
}
}


//@Procedure: bufsnap_loop

// Write loop in BUFSNAP mode

//@

static void bufsnap_loop(BufInfo *info,
                         unsigned char * data, 
                         int buflen)
{
struct timeval before,after;
double dsec;
long nsec,nusec;
int i,stat,cntr=0;
int start,number;

for(;;) {                                          // Loop forever 
    int ret,j;

    ret=bufWaitSnap(info,&start,&number,NULL);  // wait SHMSNAP requests
    if(ret!=NO_ERROR) {
        printf("Error waiting for snapshot request. Error code: %d\n",ret);
        break;
    }

    gettimeofday(&before,NULL);   // Get time before start writing
    for(j=0;j<number;j++) {
        for(i=0;i<buflen;i++)           // Generate dummy data on-the-fly
            data[i]= (i+cntr)%256;

        if((stat=bufWrite(info,data,j,0))!=NO_ERROR) {
            printf("Error writing buffer data to: %s:%s\n",info->com.producer,info->com.name);
            printf("Error code: %d (errno:%s)\n\n",CLEAN_ERROR(stat),strerror(errno));
            break;
        }
        if(verbose) {                   // Printout data 
            printf("Written %d bytes to %s:%s - seq:%d\n",
                         buflen,MySelf,info->com.name,j);
        }
        cntr++;
    }
    gettimeofday(&after,NULL);          // Get time after
    nsec=after.tv_sec-before.tv_sec;    // Compute delay
    nusec=after.tv_usec-before.tv_usec;
    dsec = (double)nsec*1000.0 +(double)nusec*0.001;

    if(verbose)                     // Printout data 
        printf("Written %d buffers to %s:%s - time:%7.3f\n",
                         number,info->com.producer,info->com.name,dsec);
    if(dsec>maxtime) maxtime=dsec;      // Cumulate statistics
    if(dsec<mintime)mintime=dsec;
    avgtime += dsec;
    count++;
}
}

//@Main: 
//
//  MsgD-RTDB testing program
//
//  This program connects to the MsgD-RTDB to define a shared memory buffer
//  Then begins to write data into the buffer
//@

int main(int argc, char **argv) 
{
    int stat,i;
    int maxSnap=10, aMode=BUFCONT, maxCS;
    char *server="127.0.0.1";
    char *bufnam;
    int buflen=1500;
    double secdelay= -1.0;
    struct timespec cpdelay;
    struct timespec *delay;
    int manual=0;
    int nloop=0;

    BufInfo info;

    unsigned char * data;

    bufnam=NULL;
    verbose=0;

    SetVersion(VERSMAJ,VERSMIN);

    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-v")==0) { verbose=1; continue; }
	if(strcmp(argv[i],"-c")==0) { nloop=atoi(argv[++i]); continue; }
	if(strcmp(argv[i],"-d")==0) { secdelay=atof(argv[++i]); continue; }
	if(strcmp(argv[i],"-m")==0) { manual=1; continue; }
	if(strcmp(argv[i],"-n")==0) { maxSnap=atof(argv[++i]); continue; }
	if(strcmp(argv[i],"-r")==0) { filename=argv[++i]; continue; }
	if(strncmp(argv[i],"-s",2)==0) { aMode=BUFSNAP; continue; }
	if(strncmp(argv[i],"-c",2)==0) { aMode=BUFCONT; continue; }
	if(strcmp(argv[i],"-h")==0) { help(); return 0; }
    }

    if(i>=argc) { help(); return 1; }
    stat=atoi(argv[i++]);

    if(i>=argc) { help(); return 1; }
    bufnam = argv[i++];

    if(i<argc) buflen=atoi(argv[i++]);

    if(secdelay<0.0) {
        if(aMode==BUFSNAP) 
            secdelay=5.0;
        else
            secdelay=1.0;
    }

    if(aMode==BUFCONT)
      maxCS=11;
    else
      maxCS=maxSnap;

    if(secdelay>0.0) {
        cpdelay.tv_sec=floor(secdelay);           // Set delay time 
        secdelay -= cpdelay.tv_sec;
        cpdelay.tv_nsec=(secdelay*1.0e+9);
        delay= &cpdelay;
    } else
        delay=NULL;

    sprintf(MySelf,"PRODC-%2.2d",stat);    // Set up client name

    data=malloc((size_t)buflen);

    if (data==NULL) {
	printf("Error allocating %d bytes for data\n",buflen);
	printf("   SYSERROR: %s\n\n",strerror(errno));
    }

    signal(SIGINT,sighand);      // Install signal handler for INT

    if(verbose)  
	printf("%s connecting to MsgD-RTDB\n", MySelf);

    if((stat=ConnectToServer(server,MySelf,1))!=NO_ERROR) {
	printf("Error connecting to server at: %s\n",server);
	printf("Error code: %d - %s\n\n",CLEAN_ERROR(stat),strerror(errno));
	return 1;
    }

   if (IS_ERROR(stat=bufCreate(MySelf,bufnam,aMode,buflen,maxCS,&info))) {
	printf("Error creating shared buffer: %s\n",bufnam);
   printf("%s\n", lao_strerror(stat));
	printf("Error code: %d (errno:%s)\n\n",CLEAN_ERROR(stat),strerror(errno));
	return 1;
    }

    if(verbose) {
        if(aMode==BUFCONT)
            printf("Created BUFCONT buffer: %s:%s. Id:%d Size:%d  MaxClients:%d MagicNum:%d\n",
                        info.com.producer,info.com.name,
                        info.com.bufID,info.com.lng,info.com.maxC,info.com.uniq);
        else
            printf("Created BUFSNAP buffer: %s:%s. Id:%d Size:%d MaxSnaps:%d  MaxClients:%d MagicNumb:%d\n",
                        info.com.producer,info.com.name,
                        info.com.bufID,info.com.lng,
                        info.com.maxSnap,info.com.maxC,info.com.uniq);
    }

    mintime=10000000;                  // Initialize statistics
    maxtime=0.0;
    avgtime=0.0;
    gettimeofday(&start,NULL);         // Get start time 

    if(aMode==BUFCONT)
        bufcont_loop(&info,data,buflen,delay,manual,nloop);
    else
        bufsnap_loop(&info,data,buflen);

    free(data);
    return 1;
}
