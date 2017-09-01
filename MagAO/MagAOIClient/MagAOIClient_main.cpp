//The main program for the Magellan AO Interface

#include "MagAOIClient.h"

int debug;

int main( int argc, char **argv) 
{
   debug = 0;
   MagAOIClient *c = NULL;
   
   try 
   {
      c = new MagAOIClient( argc, argv);

      c->Exec();

   } catch (AOException &e) 
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
   
   // Correctly terminate the AOApp, disconnecting form MsgD
   delete c;
}

