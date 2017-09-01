//@File: configlib.c
//
// ********************************
// First set: Configuration library.
//
// This library contains functions to read configuration files and set
// an arbitrary number of parameters from them.
//
// The list of parameters is passed as an argument to loadConfigFile(). It's
// a pointer to an array of structs (of type config_param), defining name, 
// type and length of each argument. Parameter values are passed back as
// pointers inside the same structure.
//
// Structures in the array must be in alphabetical order, using the parameter name (!)
//
// Warning: char parameters are allocated by the library and should be freed
// by the main program when they are not needed anymore. A call to memRelease 
// will do the job on all the config array elements
//
// See configlib.h for details on this struct.
//
// NOTE: CURRENT VERSION ONLY MANAGES SINGLE VARIABLES OF TYPE INTEGER, DOUBLE,
//       STRING.
//
// ************************************************************
// Second set: routines for standard AdOpt configuration format
//
// Routines capable of reading the standard AdOpt config
//
//@
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/errlib.h"

#include "configlib.h"




//@Function loadConfigFile
//
// Loads parameters from a configuration file
//@

int loadConfigFile(char *filename,              // @P{filename}: configuration file to read
                   config_param *config_params, // @P{conffig_params}: parameters to search
                   int n_params)                // @P{n_params}: number of parameters (array length)
                                                // @R: completion code.
{
	FILE *fp;
	char buffer[1024];
	char seps[] = " \t\n";
	char *name, *value;
	int found,pos;

	// Open file
	if ( (fp = fopen(filename, "r")) == NULL)
		return SYSTEM_ERROR(FILE_ERROR);

	// Read one line at a time
	while ( fgets( buffer, 1023, fp))
		{
		name = strtok( buffer, seps);
		if (!name)
			continue;

		// Skip comment lines
		if (name[0] == '#')
			continue;

		value = strtok( NULL, seps);
		if (!value)
			continue;

		// search for parameter name (inefficient, but only at configuration time)
		found=0;
		pos=-1;
		while ((!found) && (pos < n_params))
			if (strcmp( config_params[++pos].name, name) == 0)
				found=1;

		// Unknown parameter
		if (!found)
			continue;

		switch( config_params[pos].type)
			{
			case INT_VARIABLE:
			config_params[pos].pointer = malloc(sizeof(int));
			*(int *)(config_params[pos].pointer) = atoi(value);
			break;

			case REAL_VARIABLE:
			config_params[pos].pointer = malloc(sizeof(double));
			*(double *)(config_params[pos].pointer) = atof(value);
			break;

			case CHAR_VARIABLE:
			config_params[pos].pointer = (void *)strdup(value);
			break;

			default:
			break;
			}
		}


	return NO_ERROR;
}

//@Function memRelease
//
// Releases memory allocated by loadConfigFile
//@

void memRelease(config_param *config_params,  // parameters to search
	                       int n_params)  // number of parameters (array length)
{
int i;

for(i=0;i<n_params;i++) {
    if(config_params[i].pointer) free(config_params[i].pointer);
}

}

//@Function ReadString
//
// Reads a string parameter from a configuration file
//@

int ReadString( FILE *fp, char *par_name, char *buf, int buflen)
{
        char linebuf[1024];
        int stat;
        char seps[] = " \t\n";
        char *name, *type, *value;

        while( !feof(fp))
                {
                if (( stat = GetNextValidLine( fp, linebuf, 1024)) != NO_ERROR)
                        return stat;

                // Get parameter name
                if ((name = strtok( linebuf, seps)) == NULL)
                        continue;

                // Check for parameter name
                if (strcmp(name, par_name))
                        continue;

                // Get parameter type
                if ((type = strtok( NULL, seps)) == NULL)
                        continue;

                // Check for the right type: string or structure
                if (strcmp(type, "string") && strcmp(type, "structure"))
                        continue;

                // Get the string between quotes
                if ((value = strtok( NULL, "\"")) == NULL)
                        continue;
                if ((value = strtok( NULL, "\"")) == NULL)
                        continue;

                // Copy string into buffer and zero-terminate it
                strncpy( buf, value, buflen-1);
                buf[buflen-1] =0;
                return NO_ERROR;
                }

        return FILE_ERROR;
}

//@Function: Readnum
//
// Reads a numerical parameter from a configuration file
//

int ReadNum( FILE *fp, char *par_name, int num_type, void *num)
{
        char linebuf[1024];
        int stat;
        char seps[] = " \t\n";
        char *name, *type, *value;

        while (!feof(fp))
                {
                if (( stat = GetNextValidLine( fp, linebuf, 1024)) != NO_ERROR)
                        return stat;

                // Get parameter name
                if ((name = strtok( linebuf, seps)) == NULL)
                        continue;

                // Check parameter name
                if (strcmp(name, par_name))
                        continue;

                // Get parameter type
                if ((type = strtok( NULL, seps)) == NULL)
                        continue;

                // Get parameter value
                if ((value = strtok( NULL, seps)) == NULL)
                        continue;

                // Assign value depending on type
                switch( num_type)
                        {
                        case TYPE_INT:
                        if (strcmp(type, "int"))
                                continue;
                        *((int *)num) = atoi(value);
                        return NO_ERROR;
                        break;

                        case TYPE_FLOAT32:
                        if (strcmp(type, "float32"))
                                continue;
                        *((float32 *)num) = atof(value);
                        return NO_ERROR;
                        break;

                        default:
                        continue;
                        }
                }

        return FILE_ERROR;
}

//@Function: GetNextValidLine
//
// Gets a valid line, skipping empty and comment lines, and removing trailing comments from valid lines
//@

int GetNextValidLine( FILE *fp, char *buf, int buflen)
{
	int n;
	char c;
	char *pos;

	while(!feof(fp))
		{	
		fgets( buf, buflen-1, fp);

		// Skip comments
		if (buf[0] == ';')
			continue;
		if (buf[0] == '#')
			continue;

		// Skip empty lines
		n = sscanf( buf, "%c", &c);
		if (n==0)
			continue;

		// Remove trailing comments
		if ((pos = strchr(buf, ';')) != NULL)
			*pos = 0;
		if ((pos = strchr(buf, '#')) != NULL)
			*pos = 0;

		return NO_ERROR;
		}

	buf[0] = 0;
	return FILE_ERROR;
}


