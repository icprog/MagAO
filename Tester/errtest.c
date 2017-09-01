//@File: errtest.c
//
// Small test for error codes

//@

#define VERSMAJ  1
#define VERSMIN  0


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "base/msglib.h"

#include "base/msgtxt.h"

#define PATH_LEN 2048

static void help()
{
    printf("\nerrtest  -  L.Fini\n\n");
    printf("Test error codes and messages\n");
}

int main(int argc, char **argv) 
{
int loop=1;
int errc;
DbTable* pt;

help();

while(loop) {
    printf("\nType numerical error code (decimal, positive) (0 to end): ");
    scanf("%d",&errc);
    if(errc==0) break;
    errc=(-errc);
    pt=lao_errinfo(errc);
    if(pt) 
       printf("\nErrC %d (Sys:%d  Fatal:%d)  %s - %s\n",errc,SYSTEM_ERROR(errc),FATAL_ERROR(errc),pt->name,pt->descr);
    else
       printf("\nErrC %d: Undefined\n",errc);

}
return 0;
}
