//@File: testaoapp.cpp
//
//  Very simple test of a generic AOApp
//
//@


#include <AOApp.h>

class MyApp: public AOApp {
    public:
        MyApp(string name, string side, string ip, string logdir);
        virtual ~MyApp();
        virtual void InstallHandlers();
    private:
        static int def_hndl(MsgBuf *msgb, void *app, int nqueue);
    
};



void help(void)
{
printf("TestAOApp.  L.Fini, June 2009\n\n");
printf("Usage: testaoapp [-h] [-l logdir] [-n] [-s side] [IP]\n\n");
printf("Where:\n\n");
printf("        -h  Print this usage mesage and exit\n");
printf("        -n  Disable logger\n");
printf("        -l  Specify log dir\n");
printf("        -s  Specify side\n");
printf("        IP  MsgD server IP number (default: 127.0.0.1)\n\n");

}

MyApp::MyApp(string name, string side, string ip, string logdir): 
       AOApp(name,side,ip, Logger::LOG_LEV_DEBUG, false, logdir) { };

MyApp::~MyApp(){};

int MyApp::def_hndl(MsgBuf *msgb, void *argp, int /* nqueue */)
{
   MyApp *app=(MyApp *)argp;

   int code=HDR_CODE(msgb);

   if(code==TERMINATE) {
       app->_logger->log(Logger::LOG_LEV_INFO, "TERMINATE message received");
       printf("\n TERMINATE message received. Stopping\n");
       app->SetTimeToDie(true);
   } else {
       printf("Unexpected Msg - From:%-10s To:%-10s Code:%-6x pLoad:%-6d Flg:%2.2x SeqN:%d Len:%d\n",
       HDR_FROM(msgb),HDR_TO(msgb),HDR_CODE(msgb),HDR_PLOAD(msgb),
       HDR_FLAGS(msgb),HDR_SEQN(msgb), HDR_LEN(msgb));
   }

   thRelease(msgb);
   return NO_ERROR;
}

void MyApp::InstallHandlers()
{
int stat;

if((stat=thHandler(ANY_MSG, "*", 0, def_hndl, (char *)"default", this))<0) 
      throw AOException("thHandler error in installing default handler", stat,__FILE__, __LINE__);
}



int main(int argc, char *argv[])
{
   string appName("TestAOApp");
   string side("");
   string ip,logdir;
   MyApp *myapp=NULL;

   for(int i=1;i<argc;i++) {
      if(strcmp(argv[i],"-n")==0) {
         logdir="NONE";
         continue;
      }
      if(strcmp(argv[i],"-h")==0) {
         help();
         exit(0);
      }
      if(strcmp(argv[i],"-s")==0) {
         side=argv[++i];
         continue;
      }
      if(strcmp(argv[i],"-l")==0) {
         logdir=argv[++i];
         continue;
      }
      ip=argv[i];
      break;
   }

   if(ip.empty()) ip="127.0.0.1";

   printf("\n\nCreating AOApp. Server @ %s\n\n", ip.c_str());

   try {
       myapp=new MyApp(appName,side,ip,logdir);
       printf("\nExecuting AOApp. MsgD name: %s\n\n",myapp->MyName().c_str());
       printf("The program will stay alive (doing nothing) until terminated by\n");
       printf("a TERM message.\n\n");
       printf("Will return after 10 seconds if MsgD not available\n\n");
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

