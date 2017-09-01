
#include "AOApp.h"
extern "C"{
   #include "base/thrdlib.h"
}

#include <pthread.h>

using namespace Arcetri;

class nettest : public AOApp {

public:
   nettest( int argc, char *argv[]);

   virtual ~nettest() {};


protected:
      void Run();

};




