#include "DiagnApp.h"

int DiagnSet::_ctr=0;

class TestDiagnApp: public DiagnApp
{
     //////////// construction ///////////
    public:
        TestDiagnApp(const std::string& conffile, //Configuration file relative path (see lib/stdconfig.h)
               int verb=Logger::LOG_LEV_ERROR) : DiagnApp(conffile, verb) {_fc=0;}

        TestDiagnApp(int argc, char **argv) : DiagnApp(argc, argv) {_fc=0;}
        ~TestDiagnApp(){;}

        private: 
        unsigned int    GetFrameCounter() {return _fc++ ;}
        unsigned int _fc;
};

int main(int argc, char **argv)
{
    int            verb=6;
    char           *configfile = "conf/left/Test_DiagnApp.conf";

    try{
        TestDiagnApp* app;
        if (argc > 1) 
            app = new TestDiagnApp(argc, argv);        
        else
            app = new TestDiagnApp(configfile, verb);        
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
