#ifndef ACQUIRE_H_INCLUDED
#define ACQUIRE_H_INCLUDED

#include "arblib/base/ArbitratorInterface.h"

#include "AOApp.h"
using namespace Arcetri;

class acquireBatch: public AOApp {

public:
   acquireBatch( int argc, char *argv[]);

   virtual ~acquireBatch();

protected:
      void PostInit();
      void Run();

      void init();

      string _jobfile;

private:

    ArbitratorInterface* _wfsIntf;
    ArbitratorInterface* _adsecIntf;

};

#endif // ACQUIRE_H_INCLUDED




