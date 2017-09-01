//@File: producer_fast.c
// produce a "dummy" shared memory for use with fastdiagn
//
//@

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

extern "C"{
#include "base/thrdlib.h"
//#include "buflib.h"
#include "iolib.h"
}

//#include "fastdiagn_shmem.h"
#include "Utils.h"
#include "AdsecDiagnLib.h"     // sdram_diagn_struct
#include "AdSecConstants.h"

using namespace Arcetri;
using namespace Arcetri::AdSecConstants;


char MySelf[200];
int VersMajor = 1;
int VersMinor = 0;
const char *Date = "April 2006";


//
//@Procedure help:
//
//  Prints an usage message
//@

static void help()
{
    printf("\nproducer_fast  - Vers. %d.%d.   L.Busoni, %s\n\n", VersMajor,VersMinor,Date);
    printf("Usage: producer_fast [-v] [-n dsp] [-d secs] [bufnam] \n");
    printf("       producer_fast -h\n\n");
    printf("       -d sec Set delay time (Seconds. Fractions allowed) Default 1 s.\n");
    printf("       -n     Set number of DSP. Default 28.\n");
    printf("       -a     Triggers random alarms.\n");
    printf("       -i     Change identity for shared buffer name.\n");
    printf("       -h     print an usage message.\n");
    printf("       -v     verbose mode.\n");
    printf("       bufnam name of the buffer to define. Default ADSECBUF\n");
    printf("The program loops until stopped with <CTRL>-C\n\n");
}


int counter=0;
int num_dsp=168;
bool trigger_alarms = false;

double maxtime,mintime,avgtime;
MsgBuf msgb;
int verbose;
char *target;
struct timeval start,stop;

//
//@Entry: terminate
//
//  Compute statistics and terminate
//@

static void terminateProducer()
{
    long nsec,nusec;
    double dsec,rate=0;

    gettimeofday(&stop,NULL);         // Get stop time 
    nsec=stop.tv_sec-start.tv_sec;    // Compute delay
    nusec=stop.tv_usec-start.tv_usec;
    dsec = (double)nsec +(double)nusec*0.000001;

    if(counter>0) {
        rate=(double)counter/dsec;
        avgtime = avgtime /(double)counter;      // Compute average delay
    }
    // Print statistics
    printf("\n%s %d messages (%.2f/sec). round-trip min/avg/max %.3f/%.3f/%.3f (ms)\n",
	   MySelf,counter,rate,mintime,avgtime,maxtime);

    // program terminates
    exit(0);
}


//@Entry: sighand
//
//  Signal handler for terminating the program
//@

static void sighand(int thesig  )
{
    if (thesig==SIGINT) terminateProducer();
}


//@Entry: randu
//Return an uniform variate in the range [min,max]
//@
static double randu(double min, double max){
    return ((double)rand()/RAND_MAX) * (max-min) + min;
}

//
// calcola _matr ## _vect e mette il risultato in _r
//
void matrix_vect(float* _matr, float* _vect, float* _r, long _n_row, long _n_column)
{
    memset(_r,0,_n_row*sizeof(*_r));
    //for (int i=0; i<_n_row; i++)
    //    for (int j=0; j<_n_column; j++)
    //        _r[i] += _matr[i*_n_column+j] * _vect[j];

    float *a=_matr;
    float *b=_vect;
    float *b_end = b + _n_column;
    float *c=_r;
    float *c_end = c + _n_row;
    for ( ; c < c_end; c++){
        float *b = _vect;
        for ( ; b < b_end; )
            *c += *a++ * *b++;
    }
}

//@Procedure: bufcont_loop

// Write loop in BUFCONT mode

//@

static void bufcont_loop(BufInfo *info,
			 AdsecDiagnRaw    *data, 
			 struct timespec *delay)
{
    struct timeval start_loop,end_loop, start_cycle, end_cycle;
    double dsec, distave,freq;
    long nsec,nusec;
    int i,j,stat,cntr,timestamp=12, dumpcycle;
    long nact=BcuMirror::N_CH;
    float *modi,*pos,*curr, *Mplus, *K;
    unsigned int  num_acc_samples = 1800;
    double        acc_pos_fixed = 1.0e-11;
    double        acc_curr_fixed = 1.0e-8;

    
    cntr=0;

    srandom(time(NULL));

    if (delay)
        dumpcycle= (int)(1.0 / (delay->tv_sec + delay->tv_nsec/1e9));
    else
        dumpcycle=1000;

    //M    = (float*)calloc(nact*nact,sizeof(float));
    //K    = (float*)calloc(nact*nact,sizeof(float));
    //for(i=0;i<nact;i++){ // diagonal
    //    M[i+i*nact]=2;
    //   K[i+i*nact]=3;
    //}
    char Mfile[256];
    sprintf (Mfile, "conf/prodMplus%ld.fits",nact);
    Mplus = (float*) ReadBinaryFile(Mfile, &stat);
    printf("ReadBinaryFile %s  Mplus[0,0]=%7.3g len=%d\n", Mfile, *Mplus, stat);

    char Kfile[256];
    sprintf (Kfile, "conf/prodK%ld.fits",nact);
    K = (float*) ReadBinaryFile(Kfile, &stat);
    printf("ReadBinaryFile %s  K[0,0]=%7.3g len=%d\n", Kfile, *K, stat);

    modi = (float*)malloc(nact*sizeof(float));
    pos  = (float*)malloc(nact*sizeof(float));
    curr = (float*)malloc(nact*sizeof(float));

    gettimeofday(&start_loop,NULL);          // Get time begin loop
    gettimeofday(&start_cycle,NULL);         // Get time begin cycle ( = dumpcycle loops)

    for(;;) {		// Loop forever writing variable buffer data

        if(cntr!=0) {
            gettimeofday(&end_loop,NULL);          // Get time endloop
            nsec=end_loop.tv_sec-start_loop.tv_sec;    // Compute delay
            nusec=end_loop.tv_usec-start_loop.tv_usec;
            dsec = (double)nsec +(double)nusec*0.000001;
            timestamp +=  (int) (dsec / ADSEC_TIMESTAMP_PERIOD);
            if(verbose>2) printf("timestamp %d\n", timestamp);
        }
        gettimeofday(&start_loop,NULL);         // Get time begin loop

        // positions are created
        distave = ( (cntr/10)%2) ? 60e-6 : 70e-6 ;
        for(i=0;i<nact;i++){
            pos[i] = distave + randu(-6e-8, 6e-8);
        }
        // modes and currents are reconstructed
        if (cntr%10) matrix_vect(Mplus, pos, modi , nact, nact);
        if (cntr%10) matrix_vect(K, pos , curr, nact, nact);

        if(verbose>2) for(i=0;i<nact;i++)   printf("%04d %11.3g %11.3g %11.3g\n", i, modi[i], pos[i], curr[i]);

		
	for(int sw=0; sw<BcuSwitch::N_CRATES; sw++) {
            data->switc[sw].header.WFSFrameCounter      = 11;
            data->switc[sw].header.ParamBlockSelector   = (1<<6)+(1<<7)+(1<<2);
            data->switc[sw].header.MirrorFrameCounter   = cntr;
	    data->switc[sw].header.Dummy                = 0;   

            data->switc[sw].footer.WFSFrameCounter      = data->switc[sw].header.WFSFrameCounter;
            data->switc[sw].footer.ParamBlockSelector   = data->switc[sw].header.ParamBlockSelector;
            data->switc[sw].footer.MirrorFrameCounter   = data->switc[sw].header.MirrorFrameCounter;
	    data->switc[sw].footer.Dummy                = data->switc[sw].header.Dummy;   
            
            
            for(int sl=0; sl<LB_N_SLOPES; sl++) {
                data->switc[sw].slopes[sl] = sl;
            }
            data->switc[sw].swb_SafeSkipFrameCnt        = cntr/1000; 
	    data->switc[sw].swb_PendingSkipFrameCnt     = cntr/2000; 
	    data->switc[sw].swb_WFSGlobalTimeout        = cntr/3000; 
	    data->switc[sw].accelerometer_coeffs[0]     = 0.1;      
	    data->switc[sw].accelerometer_coeffs[1]     = 0.2;      
	    data->switc[sw].accelerometer_coeffs[2]     = 0.3;  
	    if ((trigger_alarms) && (rand() < RAND_MAX/200+1)) {
		printf("Triggering Alarm accelerometer_coeffs...\n");
		data->switc[sw].accelerometer_coeffs[2] = INFINITY;
	    }
	    if ((trigger_alarms) && (rand() < RAND_MAX/200+1)) {
		printf("Triggering Alarm SafeSkipFrameCnt...\n");
		data->switc[sw].swb_SafeSkipFrameCnt = 30000;
	    }
            data->switc[sw].TimeStamp                   = timestamp;   // 59.29 kHz (16.87us)
        }
	for(int cr=0; cr<BcuMirror::N_CRATES; cr++) {
            data->crate[cr].header.WFSFrameCounter      = data->switc[0].header.WFSFrameCounter;
            data->crate[cr].header.ParamBlockSelector   = data->switc[0].header.ParamBlockSelector;
            data->crate[cr].header.MirrorFrameCounter   = data->switc[0].header.MirrorFrameCounter;
	    data->crate[cr].header.CommandHistoryPtr    = data->switc[0].header.WFSFrameCounter;   

            data->crate[cr].footer.WFSFrameCounter      = data->crate[cr].header.WFSFrameCounter;
            data->crate[cr].footer.ParamBlockSelector   = data->crate[cr].header.ParamBlockSelector;
            data->crate[cr].footer.MirrorFrameCounter   = data->crate[cr].header.MirrorFrameCounter;
	    data->crate[cr].footer.CommandHistoryPtr    = data->crate[cr].header.CommandHistoryPtr;   

	    for(i=0;i<BcuMirror::N_DSP_CRATE ;i++){
		for (j=0; j< BcuMirror::N_CH_DSP_ ; j++){
		    data->crate[cr].dsp[i].DistAverage[j]  = pos  [cr*BcuMirror::N_CH_CRATE + i*BcuMirror::N_CH_DSP_+j]; 
		    data->crate[cr].dsp[i].CurrAverage[j]  = curr [cr*BcuMirror::N_CH_CRATE + i*BcuMirror::N_CH_DSP_+j]; 
		    data->crate[cr].dsp[i].Modes[j]        = modi [cr*BcuMirror::N_CH_CRATE + i*BcuMirror::N_CH_DSP_+j] +1e-9;
                    
                    // acc = <x> / acc_pos_fixed * num_acc_samples
                    double acc = pos [cr*BcuMirror::N_CH_CRATE + i*BcuMirror::N_CH_DSP_+j] / acc_pos_fixed * num_acc_samples;  
		    data->crate[cr].dsp[i].DistAccumulator [2*j]   = (uint32) (acc - (uint32)(acc / 2e32)); //TODO facacare
		    data->crate[cr].dsp[i].DistAccumulator [2*j+1] = (uint32) (acc / 2e32);
                    double acc2 = acc*acc*1.1;  
		    data->crate[cr].dsp[i].DistAccumulator2[2*j]   =  (uint32) (acc2 - (uint32)(acc2 / 2e32));
		    data->crate[cr].dsp[i].DistAccumulator2[2*j+1] =  (uint32) (acc2 / 2e32); 
		    data->crate[cr].dsp[i].DistAccumulator2[8] 	= 0; //TODO qui e' sbagliato ma chissefrega.
		    data->crate[cr].dsp[i].DistAccumulator2[9] 	= 0;

                    acc = curr [cr*BcuMirror::N_CH_CRATE + i*BcuMirror::N_CH_DSP_+j] / acc_curr_fixed * num_acc_samples;  
		    data->crate[cr].dsp[i].CurrAccumulator [2*j]   = (uint32) (acc - (uint32)(acc / 2e32)); //TODO facacare
		    data->crate[cr].dsp[i].CurrAccumulator [2*j+1] = (uint32) (acc / 2e32);
                    acc2 = acc*acc*1.1;  
		    data->crate[cr].dsp[i].CurrAccumulator2[2*j]   =  (uint32) (acc2 - (uint32)(acc2 / 2e32));
		    data->crate[cr].dsp[i].CurrAccumulator2[2*j+1] =  (uint32) (acc2 / 2e32); 
		    data->crate[cr].dsp[i].CurrAccumulator2[8] 	= 0; //TODO qui e' sbagliato ma chissefrega.
		    data->crate[cr].dsp[i].CurrAccumulator2[9] 	= 0;

		    if  ((trigger_alarms) && ((cntr % 1000) > 30) && ((cntr % 1000) < 100) && (i == 0) && (j == 0)) {
			printf("Triggering Alarm CurrAverage...\n");
			data->crate[cr].dsp[i].CurrAverage[j] = 1.0;
		    }
		    else if ((trigger_alarms) && (rand() < RAND_MAX/25000+1)) {
			printf("Triggering Alarm CurrAverage...\n");
			data->crate[cr].dsp[i].CurrAverage[j] = 1.0;
		    }
		    else if ((trigger_alarms) && (rand() < RAND_MAX/25000+1)) {
			printf("Triggering Alarm DistAverage...\n");
			data->crate[cr].dsp[i].DistAverage[j] = 0.666;
		    }
		}
	    }
	}
        if(delay) nanosleep(delay,NULL);             // Delay 

        if(verbose>1) {                   // Printout data 
            printf("MirrorFrameCounter %d, DistAverage[0]  %g, CurrAverage[0] %g, Modes[0] %g\n",
		   data->crate[0].header.MirrorFrameCounter, data->crate[0].dsp[0].DistAverage[0],
		   data->crate[0].dsp[0].CurrAverage[0], data->crate[0].dsp[0].Modes[0] );
        }
        //gettimeofday(&before,NULL);         // Get time before writing
        if((stat=bufWrite(info,data,cntr,0))!=NO_ERROR) {
            printf("Error writing buffer data to: %s:%s\n",info->com.producer,info->com.name);
            printf("Error code: %d (errno:%s)\n\n",CLEAN_ERROR(stat),strerror(errno));
            break;
        }
        //gettimeofday(&after,NULL);          // Get time after
        //nsec=after.tv_sec-before.tv_sec;    // Compute delay
        //nusec=after.tv_usec-before.tv_usec;
        //dsec = (double)nsec*1000.0 +(double)nusec*0.001; // [ms]

        //if(verbose) {                   // Printout data 
        //    printf("Written %d bytes to %s:%s - seq:%d  time:%7.3f ms\n",
        //            buflen,MySelf,info->com.name,cntr,dsec);
        //}
        //if(dsec>maxtime) maxtime=dsec;      // Cumulate statistics
        //if(dsec<mintime) mintime=dsec;
        //avgtime += dsec;
        //j = (j+1)%256;
        cntr++;
        counter++;

        if (!(counter % dumpcycle)){
            gettimeofday(&end_cycle,NULL);          // Get time endloop
            nsec=end_cycle.tv_sec-start_cycle.tv_sec;    // Compute delay
            nusec=end_cycle.tv_usec-start_cycle.tv_usec;
            dsec = (double)nsec +(double)nusec*0.000001;
            freq = dumpcycle/dsec;
            printf("Processed %d cycles in %8.3f s (%8.3f Hz)\n", dumpcycle,dsec, freq);
            gettimeofday(&start_cycle,NULL);         // Get time begin loop
        }
    }
    free(modi);
    free(pos);
    free(curr);
    free(Mplus);
    free(K);
}


//@Main: 
//
//  fastdiagn testing program
//
//  This program connects to the MsgD-RTDB to define a shared memory buffer
//  Then begins to write data into the buffer mimicking crates' fast diagnostic 
//@

int main(int argc, char **argv) 
{
    int stat,i;
    int aMode=BUFCONT, maxCS;
    const char *server="127.0.0.1";
    const char *bufnam="ADSECBUF";
    int buflen;
    double secdelay=-1.0;
    struct timespec cpdelay;
    struct timespec *delay;

    BufInfo info;

    AdsecDiagnRaw  *data;

    SetVersion(VersMajor,VersMinor);

    //buflen=1500;
    buflen=sizeof(data);
    verbose=0;

    //sprintf(MySelf,"PRDFAST%d",stat);    // Set up client name
    //sprintf(MySelf,"MIRRORCTRL");    // Set up client name
    //snprintf(MySelf,200,"MASTDIAGN00");    // Set up client name
    snprintf(MySelf,200,"masterdiagnostic.%s", Utils::getAdoptSide().c_str());


    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

        if(strcmp(argv[i],"-v")==0) { verbose++; continue; }
        if(strcmp(argv[i],"-d")==0) { secdelay=atof(argv[++i]); continue; }
        if(strcmp(argv[i],"-n")==0) { num_dsp=atoi(argv[++i]); continue; }
        if(strcmp(argv[i],"-b")==0) { bufnam=argv[i++]; continue;}
        if(strcmp(argv[i],"-a")==0) { trigger_alarms = true; continue;}
        if(strcmp(argv[i],"-i")==0) { snprintf(MySelf,200,argv[++i]); }
        if(strcmp(argv[i],"-h")==0) { help(); return 0; }
    }

    if(i<argc) buflen=atoi(argv[i++]);

    if(secdelay<0.0) {
        secdelay=1.0;
    }

    maxCS=5; //max number of consumer

    if(secdelay>0.0) {
        cpdelay.tv_sec=(__time_t) floor(secdelay);           // Set delay time 
        secdelay -= cpdelay.tv_sec;
        cpdelay.tv_nsec=(long int)(secdelay*1.0e+9);
        delay=&cpdelay;
    } else
        delay=NULL;


    buflen = sizeof(AdsecDiagnRaw) ; //BcuMirror::N_CRATES * sizeof(crate_bcu);
    data=(AdsecDiagnRaw*)malloc((size_t)buflen);

    if (data==NULL) {
        printf("Error allocating %d bytes for data\n",buflen);
        printf("   SYSERROR: %s\n\n",strerror(errno));
    }

    signal(SIGINT,sighand);      // Install signal handler for INT

    if(verbose)  
        printf("%s connecting to MsgD-RTDB\n", MySelf);

    if(IS_ERROR(stat=thInit(MySelf))) {
        printf("Error from thInit(): [%d] - %s\n\n",stat,lao_strerror(stat));
        return 1;
    }
    if(IS_ERROR(stat=thStart(server,1))) {
        printf("Error connecting to server at: %s\n",server);
        printf("Error code: %d - %s\n\n",stat,lao_strerror(stat));
        return 1;
    }

    if(IS_ERROR(stat=bufCreate(MySelf,bufnam,aMode,buflen,maxCS,&info))) {
        printf("Error creating shared buffer: %s\n",bufnam);
        printf("Error code: %d  (errno:%s)\n\n",CLEAN_ERROR(stat),strerror(errno));
        return 1;
    }

    if(verbose) {
        printf("Created BUFCONT buffer: %s:%s. Id:%d Size:%d  MaxClients:%d MagicNum:%d\n",
	       info.com.producer,info.com.name,
	       info.com.bufID,info.com.lng,info.com.maxC,info.com.uniq);
    }

    mintime=10000000;                  // Initialize statistics
    maxtime=0.0;
    avgtime=0.0;
    gettimeofday(&start,NULL);         // Get start time 

    bufcont_loop(&info,data,delay);

    free(data);
    return 1;
}
