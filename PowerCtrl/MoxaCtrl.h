//@File: MoxaCtrl.h
//
//@

#include "AOApp.h"
#include "MoxaItem.h"

//@Class: MoxaCtrl
//
// Extensd an AOApp to control power relays
//@

class MoxaCtrl: public AOApp {

public:

   MoxaCtrl( int argc, char **argv) throw (AOException);

   // VIRTUAL create all power items
   void PostInit();
   void Run();

   // VIRTUAL RTDB handler
   int VariableHandler( Variable *var);

   int handle() { return _handle; }

   double ttAmp();

   static int TTAmpChanged(void *pt, Variable *var);

   RTDBvar var_ttamp_cur;

protected:
   vector<MoxaItem *> poweritems;
   int TestComm();
   int _handle;


   string _ip;
   int _port;
};




