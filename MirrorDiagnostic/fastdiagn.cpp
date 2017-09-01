//+File: fastdiagn
//
// Real-time diagnostic monitor of AdSec mirror.
//

#include "FastDiagnostic.h"
#include <exception>

// Boost initializations for Arbitrator alerts
#include "arblib/base/AlertsExport.h"

////////////////////////////////////////
// Versioning information
// 
int         VersMajor     = 0;
int         VersMinor     = 1;
const char  *Date         = "Mar 2006";
const char  *Author       = "L. Busoni";

/*
static void help()
{
    printf("\nfastdiagn  - Vers. %d.%d.  %s, %s\n\n", VersMajor,VersMinor,Author,Date);
    printf("Usage: fastdiagn [-v] [-f <config_file>]\n\n");
    printf("   -f    specify configuration file (defaults to fastdiagn.conf)\n");
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
    SetVersion(VersMajor,VersMinor);
    FastDiagnostic *app=NULL;

    try{
        app = new FastDiagnostic(argc, argv);        
        app->Exec();
        delete app;
    } catch (AOException &e) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
        if(app) delete app;
    } catch (exception &e) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what());
        cerr << "exception caught: " << e.what() << endl;
        if(app) delete app;
    }
    //Logger::get()->log(Logger::LOG_LEV_FATAL, "PIPPO");
}    
