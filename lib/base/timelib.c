//@File: timelib.c
//
// Utility functions to sleep based on nanosleep()

#include <time.h>
#include "timelib.h"

//@Function: msleep
//
// Implementation of  a sleep function (in ms) using nanosleep
//
// This should not interfere with SIGALRM.
//
// If interrupted by signal will still try to sleep for given time
//@

int msleep(unsigned int msec)  // @R: always returns 0
{
struct timespec delay;
struct timespec rem;

delay.tv_sec=msec/1000;
delay.tv_nsec=(msec-(delay.tv_sec*1000))*1000000;
while(nanosleep(&delay,&rem)<0) {
   delay.tv_sec=rem.tv_sec;
   delay.tv_nsec=rem.tv_nsec;
}
return 0;
}


//@Function: nusleep
//
// Implementation of  the usleep function (in microseconds) using nanosleep
//
// usleep standard function is considered deprecable
//
// If interrupted by signal will still try to sleep for given time
//@

int nusleep(unsigned int usec)
{
struct timespec delay;

delay.tv_sec=usec/1000000;
delay.tv_nsec=(usec-(delay.tv_sec*1000000))*1000;
return nanosleep(&delay,NULL);
}


