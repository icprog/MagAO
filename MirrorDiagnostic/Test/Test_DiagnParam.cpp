#include "DiagnParam.h"
#include "DiagnWhich.h"
/*
//@Function: save_diagnparam
//
//@
#include <fstream>
#include <boost/archive/binary_oarchive.hpp> //serialize
void save_diagnparam(const DiagnParam &s, const char * filename){
    // make an archive
    std::ofstream ofs(filename);
    boost::archive::binary_oarchive oa(ofs);
    oa << s;
}*/

//int DiagnParam::_ctr=0;

//#define USESTRSTREAM

#ifdef USESTRSTREAM
#include <strstream>
#else
#include <sstream>
#endif

#include <boost/archive/binary_oarchive.hpp> //serialize
#include <boost/archive/binary_iarchive.hpp> //serialize

int main(int argc, char** argv)
{
	Logger::get(Logger::LOG_LEV_INFO, Logger::LOG_METHOD_STDOUT);
	Logger::get()->log(Logger::LOG_LEV_WARNING, "Start [%g > %g] [%s:%d]", __FILE__, __LINE__);

    DiagnParam param;

    param.SetEnabled(false);
    param.SetSlow(false);
    param.SetConsAllowFaults(111);
    param.SetMeanPeriod(13);
    param.SetRange(DiagnRange<float>(1./0,-21,21,52));

    std::cout << param << std::endl;


    //save_diagnparam(param, "Test_DiagnParam.txt");
#ifdef  USESTRSTREAM
    // serializzo in buffer
    std::ostrstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << param;
    const char* buffer = oss.str();
    int  lungh = oss.pcount();
    std::cout << "Pcount " << oss.pcount() << std::endl;
    std::cout << "serialc len " << strlen(buffer) << std::endl;
    std::cout <<  buffer << std::endl;
   
    // trasmissione 
    
    // riprendo da buffer
    DiagnParam param2;
    std::istrstream iss(buffer, lungh);
    boost::archive::binary_iarchive ia(iss);
    ia >> param2;
#else
    std::ostringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << param;
    std::string sbuf = oss.str();
    const char* buffer = sbuf.c_str();
    int  lungh = sbuf.size();
    std::cout << "Lungh " << lungh << std::endl;
    std::cout << "serialc len " << strlen(buffer) << std::endl;
    std::cout <<  buffer << std::endl;
    
    // trasmissione 
    
    // riprendo da buffer
    DiagnParam p2;
    std::istringstream iss(std::string(buffer,lungh));
    boost::archive::binary_iarchive ia(iss);
    ia >> p2;

#endif
    std::cout << p2 << std::endl;


    return 0;
}
