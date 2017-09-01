#include "DiagnVar.h"
#include <stdio.h>
#include <iostream>
#include <time.h>

#include <fstream>
#include <boost/archive/binary_oarchive.hpp> //serialize
void save_diagnvar(const DiagnVar &s, const char * filename){
    // make an archive
    std::ofstream ofs(filename);
    boost::archive::binary_oarchive oa(ofs);
    oa << s;
}

//Return an uniform variate in the range [min,max]
static double randu(double min, double max){
    return ((double)rand()/RAND_MAX) * (max-min) + min;
}

//#include "fastdiagn_shmem.h"     // sdram_diagn_struct

class FunctPippo : public Funct {
    public:
        FunctPippo(double *x) : x_(x)
        {
            if(DEBUG) std::cout << "--- FunctPippo constructor(float32*)" << std::endl; 
        }
        FunctPippo(const FunctPippo&):Funct()
        {
            if(DEBUG) std::cout << "--- FunctPippo copy constructor" << std::endl; 
        }
        virtual void doit(double *v) 
        {
           // std::cout << "FunctPippo address: " <<  x_ << std::endl;
            *v = (double)(*x_)*2;
        }
    private:
    private:
        double  *x_;
};

#include <unistd.h>
class FunctPluto : public Funct {
    public:
        FunctPluto(double *x) : x_(x)
        {
            if(DEBUG) std::cout << "--- FunctPluto c'tor(double*)" << std::endl; 
        }
        FunctPluto(const FunctPluto&):Funct()
        {
            if(DEBUG) std::cout << "--- FunctPluto copy c'tor" << std::endl; 
        }
        virtual void doit(double *v) 
        {
            //std::cout << "---------------------------- FunctPluto DoIt: " <<  x_ << std::endl;
            *v = (double)(*x_)*2;
        }
    private:
    private:
        double  *x_;
};


int main()
{
    double              timestamp=1.23;
    double              value;
    DiagnParam          param;

    std::string         namePippo="pippo";
    FunctPtr            fConvPippo(new FunctPippo(&value));
    FunctPtr            fConvPluto(new FunctPluto(&value));
    FunctPtr            hAlarmPippo(new FunctAlarm());
    FunctPtr            hWarningPippo(new FunctWarning());
   
    // now var "pluto" with the same parameters than pippo
    std::string         namePluto="pluto";
    FunctPtr            hAlarmPluto(new FunctAlarm());
    FunctPtr            hWarningPluto(new FunctWarning());

    DiagnVar   var     (namePippo, 0, fConvPippo, hWarningPippo, hAlarmPippo);
    DiagnVar   varPluto(namePluto, 0, fConvPluto, hWarningPluto, hAlarmPluto);

	DiagnParam diagnparam = DiagnParam();
    diagnparam.SetEnabled(true);
    diagnparam.SetWarningMax(1e10);
    diagnparam.SetAlarmMax(1e10);
	diagnparam.SetMeanPeriod(1);

    var.ImportParams(diagnparam);
    varPluto.ImportParams(diagnparam);

    
    srandom(time(NULL));
    for(int i=0; i<1000; i++){
        value = randu(0.0, 100.0);
        var.Update(timestamp);
        std::cout << var << std::endl;
        value = randu(0.0, 100.0);
        varPluto.Update(timestamp);
        std::cout << varPluto << std::endl;
    }
    // SetMeanPeriod to 200
    
// ********** OBSOLETE use ImportParams instead  
    // var.SetMeanPeriod(1000);
    //DiagnRange<double>  rng1(-100.0, 90 , 110, 200.0);
    // var.SetRange(rng1);
    //varPluto.SetRange(rng1);
// **********
    for(int i=0; i<100000; i++){
        value = randu(40.0, 60.0);
        var.Update(timestamp);
        value = randu(40.0, 60.0);
        varPluto.Update(timestamp);
        if ( i%1000 ==0) {
            std::cout << var << std::endl;
            std::cout << varPluto << std::endl;
        }
    }
    std::cout << var << std::endl;
    std::cout << varPluto << std::endl;

    
    save_diagnvar(var, "Test_DiagnVar.txt");
    /*
    std::cout<<"size of DiagnVar       : "<<sizeof(DiagnVar)<<std::endl;
    std::cout<<"size of var            : "<<sizeof(var)<<std::endl;
    std::cout<<"size of varPluto       : "<<sizeof(varPluto)<<std::endl;
    std::cout<<"size of Funct          : "<<sizeof(Funct)<<std::endl;
    std::cout<<"size of FunctAlarm     : "<<sizeof(FunctAlarm)<<std::endl;
    std::cout<<"size of hAlarmPippo    : "<<sizeof(hAlarmPippo)<<std::endl;
    std::cout<<"size of hWarningPippo  : "<<sizeof(hWarningPippo)<<std::endl;
    std::cout<<"size of FunctPippo     : "<<sizeof(FunctPippo)<<std::endl;
    std::cout<<"size of DiagnRange     : "<<sizeof(DiagnRange<double>)<<std::endl;
    std::cout<<"size of rng            : "<<sizeof(rng)<<std::endl;
    std::cout<<"size of RunningMean    : "<<sizeof(RunningMean<double>)<<std::endl;
    */
    return 0;
}
