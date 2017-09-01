
#include "AOApp.h"
extern "C"{
   #include "base/thrdlib.h"
}

#include <pthread.h>

using namespace Arcetri;

class logtest : public AOApp {

public:
   logtest( int argc, char *argv[]);

   virtual ~logtest() {};


     Logger *log1, *log2, *log3, *log4, *log5, *log6, *log7;
protected:
      void Run();

      void init();

     static void* th1( void *arg);
     static void* th2( void *arg);
     static void* th3( void *arg);
     static void* th4( void *arg);
     static void* th5( void *arg);
     static void* th6( void *arg);
     static void* th7( void *arg);
     static void* th8( void *arg);


     pthread_t thread1,thread2, thread3, thread4, thread5, thread6, thread7, thread8;
     pthread_attr_t attr1, attr2, attr3, attr4, attr5, attr6, attr7, attr8;

};




