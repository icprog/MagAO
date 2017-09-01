//@File: splitdata.c
//
// Splits a data buffer into individual pixel, slopes, etc.
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

#include "bcu_diag.h"
#include "AdsecDiagnLibStructs.h"

#define BUF_LEN 1575

//
//@Procedure help:
//
//  Prints an usage message
//@

static void help()
{
    printf("Usage: splitdata [-s chunksize] inputfile\n");
    printf("       splitdata -h\n\n");
    printf("       -h      print an usage message.\n\n");
    printf("       -s chunksize  split into multiple chuncks of this size.\n");
}


int chunksize =0;
char *filename=NULL;

//@Main: 
//
//@

#define MAX_PATH (1024)

int main(int argc, char **argv) 
{
    int stat,i;
    OptLoopDiagFrame diagf;
    FILE *fp=NULL;
    FILE *slopes_fp=NULL;
    FILE *commands_fp = NULL;
    char slopes_fn[MAX_PATH];
    char commands_fn[MAX_PATH];

    chunksize=-1;

    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	    if(strcmp(argv[i],"-s")==0) { chunksize=atoi(argv[++i]); continue; }
    }

    if(i>=argc) { help(); return 1; }
    filename = argv[i++];

 
    memset(slopes_fn, 0, sizeof(MAX_PATH)); 
    memset(commands_fn, 0, sizeof(MAX_PATH)); 
    snprintf(slopes_fn, MAX_PATH-1, "%s_slopes", filename);
    snprintf(commands_fn, MAX_PATH-1, "%s_commands", filename);

    fp = fopen(filename, "r");
    slopes_fp = fopen(slopes_fn, "w");
    commands_fp = fopen(commands_fn, "w");

    if (!fp) {
        fprintf(stderr, "Cannot read file %s\n", filename);
        exit(-1);
    }

    int count=0;
    while (!feof(fp)) {
         fread( &diagf, 1, sizeof(diagf), fp);
         fwrite( diagf.slopecomp.slopes, 1, 1600*sizeof(float32), slopes_fp);
         fwrite( diagf.adsec.DistAverage, 1, BcuMirror::N_DSP_CRATE * BcuMirror::N_CRATES *4*sizeof(float32), commands_fp);
         printf("Processed %d frames\r", count++);
    }

    printf("\n");

    fclose(fp);
    fclose(slopes_fp);
    fclose(commands_fp);

   return 0;
 
}
