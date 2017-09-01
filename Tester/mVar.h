
#include "AOApp.h"
extern "C"{
   #include "base/thrdlib.h"
}

using namespace Arcetri;

class mVar : public AOApp {

public:
   mVar( int argc, char *argv[]);

   virtual ~mVar() {};

   string _target;
   vector<string> _toExport;

protected:
      void InstallHandlers();
      void PostInit();
      void Run();

      void init();

      static int mVar_incoming( MsgBuf *, void *, int);
      static int mVar_notif( void *thisPtr, Variable *var);
};




