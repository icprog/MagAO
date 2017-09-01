//@File: iptest.c
//
// Small test for ip normalization algorithm

//@

#define VERSMAJ  1
#define VERSMIN  0


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "base/msglib.h"

static void help()
{
    printf("\niptest  -  L.Fini\n\n");
    printf("Test IP normalization routine\n");
}

int main(int argc, char **argv) 
{
help();
char buffer[2048];
int stat;

for(;;) {
    printf("\nInput IP addres (in the form: a.b.c.d:p) (0: end): ");
    scanf("%s",buffer);

    if(buffer[0]=='0') break;

    stat=NormalizeIP(buffer,50,NULL);

    printf("\nNormalized IP: %s", buffer);

    if(IS_ERROR(stat))
        printf("   - Error: %s\n\n",lao_strerror(stat));
    else
        printf("   - OK\n\n");
}
return 0;
}
