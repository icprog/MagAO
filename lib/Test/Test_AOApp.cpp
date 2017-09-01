#include "AOApp.h"

int main(int argc, char **argv)
{

    char           *configfile = "conf/left/Test_AOApp.conf";

    try{
        AOApp* app;
        if (argc > 1) 
            app = new AOApp(argc, argv);        
        else
            app = new AOApp(configfile);        
        // go
        app->Exec();
        delete app;
    } catch (AOException &e) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
    } catch (...) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "unknown exception");
    }

    return 0;
}
