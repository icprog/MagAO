#ifndef ADAMHOUSEKEEPER_CONVERT_H_INCLUDED
#define ADAMHOUSEKEEPER_CONVERT_H_INCLUDED

#include <iostream>


#include "Funct.h"
#include "Logger.h"

extern "C" {
#include "aotypes.h"        // float32
}

using namespace Arcetri;

/*class FunctFloat32 : public Funct {
    public:
        FunctFloat32(float32 *x) : x_(x)
        {
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctFloat32 constructor(float32*)");
        }
        virtual void doit(double *v)
        {
            //std::cout << "FunctFloat32 address: " <<  x_ << std::endl;
            *v = (double)(*x_);
        }
    private:
        float32  *x_;
};

class FunctUint16 : public Funct {
    public:
        FunctUint16(uint16 *x) : x_(x)
        {
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctUint16 constructor(uint16*)");
        }
        virtual void doit(double *v)
        {
            *v = (double)(*x_);
        }
    private:
        uint16  *x_;
};

class FunctUint32 : public Funct {
    public:
        FunctUint32(uint32 *x) : x_(x)
        {
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctUint32 constructor(uint32*)");
        }
        virtual void doit(double *v)
        {
            *v = (double)(*x_);
        }
    private:
        uint32  *x_;
};*/

#endif
