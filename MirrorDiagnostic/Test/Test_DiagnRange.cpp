#include <stdio.h>
#include "DiagnRange.h"

#include <cmath>
#include <cstdlib> //strtod
#include <cerrno>


typedef DiagnRange<int>    iRange;
typedef DiagnRange<double> dRange;
typedef DiagnRange<float>  fRange;

int main(){
 //   rng.SetAlarmMin(25);
 //   rng.SetAlarmMax(100);
 //   rng.SetWarningMin(0);
 //   rng.SetWarningMax(50);

    double v;
    char str[100];
    char *endptr;
    dRange  rng;
    dRange drng;
    fRange frng;


    std::string strin;
    istringstream ssline;
    
    std::cout << " -------------- DiagnRange<double> ------------------------ " << std::endl;
    strin = string(" -2 -1 1 2 ");
    ssline.str(strin);
    std::cout <<  "Input : " << strin;
    ssline >> drng;
    std::cout << " ------> " << drng << std::endl;

    strin = string(" -2e100 -1e100 1e100 2e100 ");
    ssline.str(strin);
    std::cout <<  "Input : " << strin;
    ssline >> drng;
    std::cout << " ------> " << drng << std::endl;

    strin = string(" -inf 0 0 inf ");
    ssline.str(strin);
    std::cout <<  "Input : " << strin;
    ssline >> drng;
    std::cout << " ------> " << drng << std::endl;
    
    strin = string(" -1e1000 -1e-1000 1e-1000 1e1000 ");
    ssline.str(strin);
    std::cout <<  "Input : " << strin;
    ssline >> drng;
    std::cout << " ------> " << drng << std::endl;
    
    
    std::cout << " -------------- DiagnRange<float> ------------------------ " << std::endl;
    strin = string(" -2 -1 1 2 ");
    ssline.str(strin);
    std::cout <<  "Input : " << strin;
    ssline >> frng;
    std::cout << " ------> " << frng << std::endl;

    strin = string(" -2e100 -1e100 1e100 2e100 ");
    ssline.str(strin);
    std::cout <<  "Input : " << strin;
    ssline >> frng;
    std::cout << " ------> " << frng << std::endl;

    strin = string(" -inf 0 0 inf ");
    ssline.str(strin);
    std::cout <<  "Input : " << strin;
    ssline >> frng;
    std::cout << " ------> " << frng << std::endl;
    
    strin = string(" -1e1000 -1e-1000 1e-1000 1e1000 ");
    ssline.str(strin);
    std::cout <<  "Input : " << strin;
    ssline >> frng;
    std::cout << " ------> " << frng << std::endl;
    
    std::cout << " -------------------------------------- " << std::endl;

    while(1){
        std::cout << "range>";
        std::cin.getline(str,100);
        istringstream ssline(str);
        ssline >> rng;
        std::cout << rng << std::endl; 
        rng.CheckOrder();

        std::cout << "value>";
        //std::cin  >> v; //std::cin.ignore(256, '\n'); //discard newline left in the buffer by cin
        std::cin.getline(str,100);
        errno = 0;    // To distinguish success/failure after call 
        v = strtod(str, &endptr);
        if (endptr == str) { // No digits were found
            v = NAN;
        }

        printf("value: %g ---> isOK: %d   isWarning: %d   isAlarm: %d\n",
            //v, rng.isOk(int(v)),   rng.isWarning(int(v)),   rng.isAlarm(int(v)));
            //v, rng.isOk(float(v)), rng.isWarning(float(v)), rng.isAlarm(float(v)));
            v, rng.isOk(v),        rng.isWarning(v),        rng.isAlarm(v));
    }
    return 0;
}
