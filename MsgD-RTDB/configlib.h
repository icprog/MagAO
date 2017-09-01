//@File: configlib.h
 
//  Header file for the configuration library
 
//@

#ifndef CONFIGLIB_H_INCLUDED
#define CONFIGLIB_H_INCLUDED

#include <stdio.h>

#define ONLY_VARCODES
#include "base/rtdblib.h"    // Find here definitions of variable types

#define TYPE_INT	(0x00)
#define TYPE_FLOAT32	(0x01)

#include "aotypes.h"

//+Struct config_param
//
// Configuration item

struct config_param
{
	char *name;
	int type;
	void *pointer;
};

typedef struct config_param config_param;

int loadConfigFile( char *filename, config_param *config_params, int n_params);
void memRelease(config_param *config_params, int n_params);

// AdOpt standard configuration routines

int GetNextValidLine( FILE *fp, char *buf, int buflen);
int ReadNum( FILE *fp, char *par_name, int num_type, void *num);
int ReadString( FILE *fp, char *par_name, char *buf, int buflen);

#endif // CONFIGLIB_H_INCLUDED
