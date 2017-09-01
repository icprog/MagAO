
#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "InterferometerCtrl.h"

using namespace Arcetri;

int main(int argc, char* argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);
	
	InterferometerCtrl* ctrl = NULL;
	
    try {
      ctrl = new InterferometerCtrl( argc, argv);
      ctrl->Exec();
    }
    catch(LoggerFatalException &e) {
		// In this case the logger can't log!!!
      	printf("%s\n", e.what().c_str());
   	} 
    catch(AOException& e) {
    	// TODO Check why this logs to INTERFCTR00L.log file after is has been archived!
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
    }
    
    delete ctrl;
    return 0;
}
