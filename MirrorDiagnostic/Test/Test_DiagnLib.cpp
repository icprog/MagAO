#include "AOApp.h"
#include <iostream>
#include "diagnlib.h"

class TestDiagnLib : public AOApp
{
    public:
        TestDiagnLib(const std::string& cfgfile) : AOApp(cfgfile) {};

        void Run()
        {
/*
            DiagnWhich which("DistAverage",0,5);
            DiagnParam param;
            param.SetSlow(false);
            param.SetRange(DiagnRange<double>(-31,-21,21,52));
            param.SetConsAllowFaults(7);

            int count;
            for (int i =0; i < 200; i++){
                try{
                    count = SetDiagnVarParam("FASTDGN",which,  param, 1000 );
                } catch (AOException &e) { std::cout << e.what() << std::endl; } 
                std::cout << "[SetParam] " << which << " ---> " << param << " [" << count << " vars modified]" << std::endl;
                usleep(100000);
            }
            
            
            for(int i=0;i< 10 ;i++){
                std::cout << " GETDIAGNVARPARAM send to FASTDGN (begin) " << std::endl;
                DiagnWhich whicha("*", 0 ,1000);
                ParamDict dict;
                try{
                    dict = GetDiagnVarParam("FASTDGN", whicha, 2000);
                } catch (AOException &e) { std::cout << e.what() << std::endl; } 
            
                for (ParamDict::iterator it = dict.begin(); it != dict.end();  it++){
                    //std::cout << (*it).first << " :: " << (*it).second << std::endl;
                }
                std::cout << " Send Msg to FASTDGN (end) " << std::endl;
                usleep(100000);
            }          
*/          
            std::string mode;
            DiagnWhich which;
            int count;
            while(TimeToDie()==false){
                std::cout << "Enter Mode [set/get] & DiagnWhich [Family, From, To]: ";
                std::cin >> mode >> which;
                if (!mode.compare("set"))
                {
                    DiagnParam param;
                    std::cout << "Enter DiagnParam [DiagnRange, RunningMeanPeriod, ConsAllowFaults, Ena/Dis, Slow/Fast]: ";
                    std::cin >> param;
                    try{
                        count = SetDiagnVarParam("FASTDGN", which, param, 1000 );
                    } catch (AOException &e) { std::cout << e.what() << std::endl; } 
                    std::cout << "[SetParam] " << which << " ---> " << param << " [" << count << " vars modified]" << std::endl;
                } else if (!mode.compare("get"))
                {
                    ParamDict dict;
                    try{
                        dict = GetDiagnVarParam("FASTDGN", which, 2000);
                    } catch (AOException &e) { std::cout << e.what() << std::endl; }
                    for (ParamDict::iterator it = dict.begin(); it != dict.end();  it++){
                        std::cout << (*it).first << " :: " << (*it).second << std::endl;
                    }
                }
            }
        
        }
};

int main()
{
    TestDiagnLib app("conf/left/Test_AOApp.conf");
    app.SetVerbosity(2);
    app.Exec();
    return 0;
}
