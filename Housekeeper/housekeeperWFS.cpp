//+File: housekeeperWFS
//
// HouseKeeper diagnostic monitor of WFS.
//

#include "HouseKeeperWFS.h"
#include <exception>

// Boost initializations for Arbitrator alerts
#include "arblib/base/AlertsExport.h"

////////////////////////////////////////
// Versioning information
// 
int         VersMajor     = 0;
int         VersMinor     = 1;
const char        *Date         = "Oct 2008";
const char        *Author       = "L. Busoni";

/*
static void help()
{
    printf("\nhousekeeperWFS  - Vers. %d.%d.  %s, %s\n\n", VersMajor,VersMinor,Author,Date);
    printf("Usage: housekeeperWFS [-v] [-f <config_file>]\n\n");
    printf("   -f    specify configuration file (defaults to housekeeperWFS.conf)\n");
    printf("   -v    verbose mode (add more -v to increase verbosity)\n");
    printf("\n");
}
*/

// +Main: 
//
// Main function
//-
int DiagnSet::_ctr=0;

int main(int argc, char **argv) 
{
	Logger::setParentName("HOUSEKEEPERWFS");

   SetVersion(VersMajor,VersMinor);
	
    try{
        HouseKeeperWFS* app;
        app = new HouseKeeperWFS(argc, argv);        
        app->Exec();
        delete app;
    } catch (AOException &e) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
    } catch (...) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "unknown exception");
    }
}    
