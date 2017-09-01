#include <AOApp.h>


class testApp: public AOApp {
  public:
    testApp( string name,
             string side,
             string serverIp,
             int logLevel,
             bool inst_sighand,
             string logDir): AOApp(name, side, serverIp, logLevel, inst_sighand, logDir) {};


  private:
    void Run() {
        while(!TimeToDie())  {
            nusleep(1000000);
            printf("Testing Logger::get()->log(Logger::LOG_LEV_ERROR ...)\n");
            Logger::get()->log(Logger::LOG_LEV_ERROR, "logtestNone: Testing Logger::get()->log(Logger::LOG_LEV_ERROR ...)");
        }        
        
    };
};

int main(int argc, char *argv[])
{
   string appName("logtestNone");
   string ip("127.0.0.1");
   string logdir("NONE");
   string side(Utils::getAdoptSide());
   testApp *myapp=NULL;

   printf("\n\nCreating AOApp. Server @ %s\n\n", ip.c_str());

   try {
       myapp=new testApp(appName,side,ip, Logger::LOG_LEV_DEBUG, false, logdir);
       myapp->waitForMsgD(10);
       myapp->Exec();
   } catch(LoggerFatalException &e) {
		// In this case the logger can't log!!!
       printf("%s\n", e.what().c_str());
   } catch(AOException& e) {
       Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
   }
   
   printf("\nTestAOapp terminated.\n\n");
   
   delete myapp;
   
   return 0;
}

