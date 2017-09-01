#include "MagFrameGrabberCtrl.h"


int main( int argc, char **argv) 
{
   try 
   {
      VisAO::FrameGrabberCtrl *c;
            

         c = new VisAO::FrameGrabberCtrl( argc, argv);
      

      c->Exec();

      delete c;
   } 
   catch (AOException &e) 
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}

