/*
 * matlab interface library
 */
#include <stdlib.h>
#include <stdio.h>
#include "engine.h"
#define  BUFSIZE 256

char dir[] = "/home/aosup/hvc_alfio";
char buffer[BUFSIZE];

void *matlab_init()
{
	Engine *ep;
	mxArray *result = NULL;

	/*
	 * Start the MATLAB engine locally by executing the string
	 * "matlab"
	 *
	 * To start the session on a remote host, use the name of
	 * the host as the string rather than \0
	 *
	 * For more complicated cases, use any string with whitespace,
	 * and that string will be executed literally to start MATLAB
	 */
	if (!(ep = engOpen("\0"))) {
		fprintf(stderr, "\nCan't start MATLAB engine\n");
		return EXIT_FAILURE;
	}

	/*
         * Initialize mirror
	 */
        sprintf(buffer, "cd '%s'", dir);
	engEvalString(ep, buffer);
	engEvalString(ep, "init");
	engEvalString(ep, "mirrorSetAll");

        return (void *)ep;
}

void matlab_setTT( void *p, double x, double y) {

   Engine *ep = (Engine *)p;

   double min=-2e-3;
   double max=2e-3;

   if (x<min)
       x=min;
   if (y<min)
       y=min;
   if (x>max)
       x=max;
   if (y>max)
       y=max;

   sprintf( buffer, "aoWrite('hvc_TT1_pos_command',[%5.3f %5.3f 0 0],0:3);", x, y);
   engEvalString(ep, buffer);
}

void matlab_setCL( void *p, double x, double y) {

   Engine *ep = (Engine *)p;

   double min=0;
   double max=120e-6;

   if (x<min)
       x=min;
   if (y<min)
       y=min;
   if (x>max)
       x=max;
   if (y>max)
       y=max;


   sprintf( buffer, "aoWrite('hvc_TT2_pos_command',[%5.3f %5.3f 0 0],0:3);", x, y);
   engEvalString(ep, buffer);
}

