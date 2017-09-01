/************************************************************
*    CROController_main.cpp
*
* Author: Jared R. Males (jrmales@email.arizona.edu)
*
* The main program for the CRO stage controller
*
* Developed as part of the Magellan Adaptive Optics system.
************************************************************/

/** \file CROController_main.cpp
  * \author Jared R. Males
  * \brief The main program for the CRO stage controller
  * 
  *
*/

#include "CROController.h"
#include "signal.h"

MagAO::CROController * global_cro;

void sigterm_handler(int signum, siginfo_t *siginf, void *ucont)
{
   std::string signame;
   
   switch(signum)
   {
      case SIGTERM:
         signame = "SIGTERM";
         break;
      case SIGINT:
         signame = "SIGINT";
         break;
      case SIGQUIT:
         signame = "SIGQUIT";
         break;
      default:
         signame = "OTHER";
   }
   
   std::cerr << "Caught signal " << signame << ".  Aborting movement." << std::endl;

   if(global_cro)
   {
      global_cro->abort_move();
   }

   exit(0);
   
   return;
}

int debug;

int main( int argc, char **argv) 
{
   debug = 0;
   extern MagAO::CROController * global_cro;
   global_cro = 0;
   MagAO::CROController * cro;

   try 
   {

      cro = new MagAO::CROController(argc, argv);// 3105);

      global_cro = cro;

      cro->Exec();

      // Correctly terminate the AOApp, disconnecting from MsgD
      delete cro;

   } catch (AOException &e) 
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
   

}

/*
int main(int argc, char **argv)
{
   extern MagAO::CROController * global_cro;
   global_cro = 0;
   MagAO::CROController * cro;


   struct sigaction act;
   sigset_t set;
   
   act.sa_sigaction = &sigterm_handler;
   act.sa_flags = SA_SIGINFO;
   sigemptyset(&set);
   act.sa_mask = set;
   
   sigaction(SIGTERM, &act, 0);
   sigaction(SIGQUIT, &act, 0);
   sigaction(SIGINT, &act, 0);

   
   try
   {
      cro = new MagAO::CROController(6701, 5, "192.168.0.101",3105);// 3105);

      global_cro = cro;

      cro->set_motor_mode("A1", 2);
      cro->set_motor_mode("B1", 2);
      cro->set_motor_mode("C1", 2);
      cro->set_motor_mode("A2", 2);
      cro->set_motor_mode("B2", 2);

      cro->move_Z(-2100);
      

      
      delete cro;
		
      return 0;

   }
   catch(...)
   {
      std::cout << "Caught exception in CROController_main.\n";
      return -1;
   }
}*/

