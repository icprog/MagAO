/*
 * @File: MirrorCtrl_Main.cpp
 * Entry point for the MirrorCtrl application.
 * @
 */

#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "MirrorCtrl.h"
#include "Logger.h"

using namespace Arcetri;

int main( int argc, char **argv) {

	MirrorCtrl*  mirrorCtrl = NULL;

   SetVersion(VERS_MAJOR,VERS_MINOR);


    try {
	   mirrorCtrl = new MirrorCtrl( argc, argv);
      mirrorCtrl->Exec();
    }
    catch(LoggerFatalException &e) {
		// In this case the logger can't log!!!
      	printf("%s\n", e.what().c_str());
   	} 
    catch(MirrorCtrlException& e) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
    }
    catch(AOException& e) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
    }
    
    delete mirrorCtrl;
    return 0;
}
