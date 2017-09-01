//@File: pinger.h
//
//@

#include "AOApp.h"

//@Class: Pinger
//
// Pinger program
//@

#include <string>
#include <vector>

struct device
{  
  std::string host;
  RTDBvar var;
  int     pid;
  int     shmid;
  double   *pnt;
};



class Pinger: public AOApp {

public:

   Pinger( int argc, char **argv) throw (AOException);

   // VIRTUAL
   void Run();
   void PostInit();

protected:
   void Create() throw (AOException);

   float _pollingPeriod;
   float _timeoutPeriod;

   vector<struct device> devices;

};




