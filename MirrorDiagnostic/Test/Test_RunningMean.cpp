#include "RunningMean.h"
#include <stdio.h>
#include <iostream>

#include <fstream>
//#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
template<class T>
void save_runningmean(const RunningMean<T> &s, const char * filename){
    // make an archive
    std::ofstream ofs(filename);
    //boost::archive::text_oarchive oa(ofs);
    boost::archive::binary_oarchive oa(ofs);
    oa << s;
}



int main()
{
    size_type size=2;
    RunningMean<double>   rm(size);
    //RunningMean<unsigned int>   rm;

    std::deque<float> d;
    d.push_back(1.0);
    d.push_back(1.0);
    std::cout << "deque size " << d.size() << " front "<< d.front() <<std::endl;
    d.pop_front();
    d.pop_front();
    d.pop_front();
    std::cout << "deque size " << d.size() << " front "<< d.front() <<std::endl;

    rm.PushOne(1.0);
    rm.PushOne(1.0);
    rm.PushOne(1.0);
    std::cout << "nelem " << rm.NElem() << " values "<< rm <<std::endl;
    rm.PopOne();
    rm.PopOne();
    rm.PopOne();
    std::cout << "nelem " << rm.NElem() << " values "<< rm <<std::endl;
    std::cout << "front " << rm.front() << std::endl;
    rm.PopOne();
    std::cout << "nelem " << rm.NElem() << " values "<< rm <<std::endl;

    double off = 1e7;
    rm.Update(4+off);
    rm.Dump();
    std::cout << "Add: "<<rm.Last()<<"   NElem:"<<rm.NElem()<<"     mean:"<< rm.Mean() << "     var:"<<rm.Variance() << std::endl;
    rm.Update(7+off);
    rm.Dump();
    std::cout << "Add: "<<rm.Last()<<"   NElem:"<<rm.NElem()<<"     mean:"<< rm.Mean() << "     var:"<<rm.Variance() << std::endl;
    rm.Update(13+off);
    rm.Dump();
    std::cout << "Add: "<<rm.Last()<<"   NElem:"<<rm.NElem()<<"     mean:"<< rm.Mean() << "     var:"<<rm.Variance() << std::endl;
    rm.Update(16+off);
    rm.Dump();
    std::cout << "Add: "<<rm.Last()<<"   NElem:"<<rm.NElem()<<"     mean:"<< rm.Mean() << "     var:"<<rm.Variance() << std::endl;
    rm.Resize(3);
    rm.Dump();
    //std::cout << "Res: "<<rm.front()<<"   NElem:"<<rm.NElem()<<"     mean:"<< rm.Mean() << "     var:"<<rm.Variance() << std::endl;
    //rm.Resize(2);
    //rm.Dump();
    //std::cout << "Res: "<<rm.front()<<"   NElem:"<<rm.NElem()<<"     mean:"<< rm.Mean() << "     var:"<<rm.Variance() << std::endl;
    //rm.Resize(1);
    //rm.Dump();
    //std::cout << "Res: "<<rm.front()<<"   NElem:"<<rm.NElem()<<"     mean:"<< rm.Mean() << "     var:"<<rm.Variance() << std::endl;

    int i;
    for (i=0; i<100; i++)
    {
        rm.Update(i);
        rm.Dump();
        std::cout << "Add: "<<rm.Last()<<"   NElem:"<<rm.NElem()<<"     mean:"<< rm.Mean()
            <<"     stddev:"<<rm.StdDev() << " ctr: "<< rm.Ctr() << " [" << rm.UpdateMin() << "/" << rm.UpdateAve() << "/" << rm.UpdateMax() << "]" << std::endl;
    }
    rm.Resize(300);
    for (i=0; i<1000; i++)
    {
        rm.Update(1e3);
        //rm.Dump();
        std::cout << "Add: "<<i<<"   NElem:"<<rm.NElem()<<"     mean:"<< rm.Mean()
            <<"     stddev:"<<rm.StdDev() << " ctr: "<< rm.Ctr() << " [" << rm.UpdateMin() << "/" << rm.UpdateAve() << "/" << rm.UpdateMax() << "]" << std::endl;
    }
    rm.Resize(100);
    for (i=0; i<10000; i++)
    {
        rm.Update(i*1e-5);
        //rm.Dump();
        std::cout << "Add: "<<i<<"   NElem:"<<rm.NElem()<<"     mean:"<< rm.Mean() << " last:" << rm.Last()
            <<"     stddev:"<<rm.StdDev() << " ctr: "<< rm.Ctr() << " [" << rm.UpdateMin() << "/" << rm.UpdateAve() << "/" << rm.UpdateMax() << "]" << std::endl;
    }

    save_runningmean(rm, "Test_RunningMean.txt");
    return 0;
}
