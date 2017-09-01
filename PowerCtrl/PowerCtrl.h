//@File: PowerCtrl.h
//
//@

#include "AOApp.h"
#include "Bcu39Item.h"

//@Class: PowerCtrl
//
// Extensd an AOApp to control power relays
//@

class PowerCtrl: public AOApp {

public:

   PowerCtrl( int argc, char **argv) throw (AOException);

   // VIRTUAL create all power items
   void PostInit();
   void Run();

   // VIRTUAL RTDB handler
   int VariableHandler( Variable *var);


protected:
   vector<Bcu39Item *> poweritems;

   int TestComm();
};




