//+File: provatables.c
//
// test file for EEV39 table routines
//

#include <stdio.h>
#include <stdlib.h>
#include "eev39.h"

int main()
{
	int ccd_dim=80;
	int numframes = 100;

	char *filename = "prova.out";
	char *outfile = "frames.out";
	FILE *fp;
	int *frames;
	int *frames2;
	int framesize, numpixels,i;

	LoadTables();

	framesize = ccd_dim * ccd_dim * sizeof(int);
	numpixels = ccd_dim * ccd_dim;

	frames = (int *)malloc (framesize * numframes);
	frames2 = (int *)malloc (framesize * numframes);

	fp = fopen(filename, "r");

	fread( frames, framesize, numframes, fp);
        fclose(fp);

	for (i=0; i<numframes; i++)
		EEV39toFrame( frames + (numpixels*i), frames2 + (numpixels*i), ccd_dim);

	fp = fopen(outfile, "w");
	fwrite( frames2, framesize, numframes, fp);
	fclose(fp);


	FreeTables();
	free( frames);
	free(frames2);

	return 0;
}


