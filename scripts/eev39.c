//+File: eev39.c
//
// Utility functions for the EEV39 CCD
//-

#include <stdio.h>
#include <malloc.h>

#include "errlib.h"
#include "eev39.h"

// Global data

int **EEV39_tables=NULL;
int EEV39_numtables=0;

//+Function: LoadTables
//
// Loads the EEV39/raster reordering tables from disk.
//
// This function MUST be called before using any conversion function.
// Tables are allocated as follows:
//
// EEV39_tables: array of tables
//
// EEV39_tables[0]: first table
// EEV39_tables[1]: second table
//
// EEV39_tables[0][0]: CCD dimension for first table
// EEV39_tables[0][1]: first pixel of first table
// ...
//-

int LoadTables()
{
	int tables[] = {80,40,20,12};
	int i,n,num=4;
	char filename[ FILENAME_MAX ];
	FILE *fp;
	int *table;

	FreeTables();

	EEV39_numtables = num;
	EEV39_tables = (int **)malloc( sizeof(int *) * EEV39_numtables);

	if (! EEV39_tables)
		return MEMORY_ALLOC_ERROR;

	for (i=0; i< EEV39_numtables; i++)
		{

		int dx = tables[i];

		sprintf( filename, "EEV39table%d.txt", tables[i]);
		if ((fp = fopen(filename, "r")) == NULL)
			return FILE_ERROR;

		EEV39_tables[i] = (int *)malloc( sizeof(int) * ( dx*dx +1));
		if (!EEV39_tables[i])
			return MEMORY_ALLOC_ERROR;

		EEV39_tables[i][0] = dx;

		table = EEV39_tables[i] +1;

		for (n=0; n<dx*dx; n++)
			fscanf( fp, "%d", table+n);

		fclose(fp);
		}

	return NO_ERROR;
}

//+Function: FreeTables
//
// Frees the memory used by CCD reordering tables
//-

int FreeTables()
{
	int i;

	if (EEV39_tables)
		{
		for (i=0; i<EEV39_numtables; i++)
			if (EEV39_tables[i])
				free (EEV39_tables[i]);
		free (EEV39_tables);
		}

	EEV39_tables = NULL;

	return NO_ERROR;
}

//+Function: EEV39toFrame
//
// Converts an EEV39-ordered frame to a raster format
//-

int EEV39toFrame( int *from, int *to, int ccd_dim)
{
	int i,n,table;

	// Check if we are properly configured
	if (!EEV39_tables)
		return ROUTINE_NOT_INITIALIZED_ERROR;

	// Search for the right conversion table
	for (table=0; table<EEV39_numtables; i++)
		if (EEV39_tables[table][0] == ccd_dim)
			break;

	// Sometimes we can fail...
	if (table == EEV39_numtables)
		return VALUE_OUT_OF_RANGE_ERROR;

	n = ccd_dim * ccd_dim;

	for (i=0; i<n; i++)
		to[ EEV39_tables[table][i+1] ] = from[i];

	return NO_ERROR;
}

