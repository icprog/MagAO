//@File: telemetry.c
//
// Telemetry library
//@

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "telemetry.h"


// Use IDL telemetry script
#define USE_IDL

// Telemetry_list

char **telemetry_list() {

#ifdef USE_IDL

    char **ret = (char**)malloc(sizeof(char*));
    ret[0]=NULL;
    int num=1;

    
    char cmd[] = "idl -e 'print,\"StartList:\",telemetry(/list)'";
    size_t len=1024;
    char *line = malloc(len);
    FILE *fp = popen(cmd, "r");
    int found=0;
    char *ptr;

    while (!feof(fp)) {
        getline(&line, &len, fp);
        if (!found) {
            if ((ptr =strstr(line, "StartList")) != NULL) {
                found=1;
                ptr = strtok(line, " ");
	         }
        } else {
            ptr = strtok(line, " ");
            if (ptr) {
                char *word = (char*)malloc(strlen(ptr)+1);
                strcpy(word, ptr);  
                ret = (char **)realloc(ret, sizeof(char*)*(++num));
                ret[num-1] = word;
	         }
        }

        return ret;
    }



#else

  return {"a", "b", "c", NULL};

#endif

}

//@Function: get
//
// Returns an array of telemetry values.
// Array must be freed by user program.
//@

double *telemetry_get( char *varname, int start_epoch, int end_epoch) {

#ifdef USE_IDL

 
   return NULL;

#else

   return NULL;

#endif


}



