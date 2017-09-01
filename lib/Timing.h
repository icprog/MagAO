//@File: Timing.h
// Timing class declaration
//
// This file contains the declaration of Timing class
// 
//@

#ifndef TIMING_H_INCLUDED
#define TIMING_H_INCLUDED

#include <time.h>
#include <sys/time.h>
//@Class: Timing
//
// Timing class for debugging purpose
//
// Timing implements a timer that can be used to
// clock snippet of user's code.
//
// \subsection*{Usage}
// Here follows an example of how to use the Timing class:
// \begin{verbatim}
// Timing t;
// t.start()
// ... do things ...
// t.stop()
// std::cout << "elapsed time [s] " << t.sec() << std::endl;
// \end{verbatim}
//@
class Timing
{
    public:
        Timing()
        {
            nsec=0;
            nusec=0;
            //TODO inizializzare before, after 
        }
        //@Method{API}: start
        //
        //Reset and start the timer
        //@
        void start()
        {
            gettimeofday(&before,NULL);
        }
        //@Method{API}: start
        //
        //Stop the timer
        //@
        void stop()
        {
            gettimeofday(&after,NULL);
        }
        //@Method{API}: sec
        //
        //Return the timer value in seconds.
        //
        //Call this method after stop()
        //@
        double sec()
        {
            nsec=after.tv_sec-before.tv_sec;
            nusec=after.tv_usec-before.tv_usec;
            return (double)nsec +(double)nusec*0.000001;
        }
        //@Method{API}: msec
        //
        //Return the timer value in milli-seconds.
        //
        //Call this method after stop()
        //@
        double msec()
        {
            nsec=after.tv_sec-before.tv_sec;
            nusec=after.tv_usec-before.tv_usec;
            return (double)nsec*1000.0 +(double)nusec*0.001;
        }
    private:
        struct timeval before,after; 
        long         nsec,nusec;
};


#endif // TIMING_H_INCLUDED
