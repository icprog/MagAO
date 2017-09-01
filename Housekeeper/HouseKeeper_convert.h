#ifndef HOUSEKEEPER_CONVERT_H_INCLUDED
#define HOUSEKEEPER_CONVERT_H_INCLUDED

#include <iostream>


#include "AOApp.h"
#include "Funct.h"
#include "Logger.h"
#include "HouseKeeperGain.h"


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

class FunctBCUStratixTemp : public Funct {
    public:
        FunctBCUStratixTemp(int16 *x) : x_(x)
        {
           _logger = Logger::get("FUNCTBCUSTRATIXTEMP",Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])   );
           _logger->log(Logger::LOG_LEV_TRACE, "FunctBCUStratixTemp constructor(int16*)");
        }
        virtual void doit(double *v)
        {
            *v = (double)(*x_)*0.0078125;
            _logger->log(Logger::LOG_LEV_TRACE, "FunctBcuStratixTemp  *v=%g (double*)=%p [%s:%d]", *v, x_, __FILE__, __LINE__);
        }
    private:
        int16  *x_;
        Logger* _logger;
};

class FunctIPAddress : public Funct {
    public:
        FunctIPAddress(uint8 x[][4]) : x_(x)
        {
           _logger = Logger::get("FUNCTIPADDRESS",Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])   );
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctIPAddress constructor(uint16*)");
        }
        virtual void doit(double *v)
        {
            *v = (double) ( (*x_)[0]*1e9 + (*x_)[1]*1e6  + (*x_)[2]*1e3  + (*x_)[3] ) ;
            _logger->log(Logger::LOG_LEV_TRACE, "FunctIPAddress  *v=%g (double*)=%p [%s:%d]", *v, x_, __FILE__, __LINE__);
        }
    private:
        uint8  (*x_)[4];
        Logger* _logger;
};

class FunctConvertGain : public Funct {
    public:
        FunctConvertGain(uint16 *x,  const float gaigno[][3], uint16 *pbserial) : _x(x), _gaigno(gaigno), _pbserial(pbserial)
        {
           _logger = Logger::get("FUNCTCONVERTGAIN", Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])  );
           _logger->log(Logger::LOG_LEV_TRACE, "FunctConvertGain constructor(uint16*, float (*)[3], uint16*)");
        }
        virtual void doit(double *v)
        {
            float vo = (float)(*_x);
            int pb = *_pbserial;
            if ( pb < 0 || pb >= AdSec::N_PB_CAL) {
                pb=1;
               //Logger::get()->log(Logger::LOG_LEV_ERROR,
               //      "FunctConvertGain: Power backplane (pb_serial %d) not calibrated.   [%s:%d]",
               //      pb, __FILE__, __LINE__);
               //throw AOException("[FunctConvertGain] Missing calibration for power backplane",GENERIC_SYS_ERROR,__FILE__,__LINE__);
            }
            const float *coeff = _gaigno[pb];
            *v = (double)(vo*vo*coeff[0] + vo*coeff[1] + coeff[2]);
            _logger->log(Logger::LOG_LEV_TRACE, "FunctConvertGain input %g, coeff %g %g %g, output %g [%s:%d]",
                    vo, coeff[0], coeff[1], coeff[2], *v, __FILE__, __LINE__);
        }
    private:
        uint16  *_x;
        const float   (*_gaigno)[3];
        uint16  *_pbserial;
        Logger* _logger;
};

class FunctDSPCoilCurrent : public Funct {
    public:
        FunctDSPCoilCurrent(uint16 *x, float spi_gain, float spi_offset)
            : x_(x), spi_gain_(spi_gain), spi_offset_(spi_offset)
        {
           _logger = Logger::get("FUNCTDSPCOILCURRENT", Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])   );
           _logger->log(Logger::LOG_LEV_TRACE, "FunctDSPCoilCurrent constructor(uint16*, float, float)");
        }
        virtual void doit(double *v)
        {
            *v = (double)( (*x_) & 0x0FFF ) * spi_gain_ + spi_offset_;
            _logger->log(Logger::LOG_LEV_TRACE, "FunctDSPCoilCurrent x=%hu, spi_gain=%g, spi_offset=%g  v=%g [%s:%d]",
                    *x_, spi_gain_, spi_offset_, *v, __FILE__, __LINE__);
        }
    private:
        uint16  *x_;
        float    spi_gain_;
        float    spi_offset_;
        Logger* _logger;
};

/*
class FunctUint32Bit : public Funct {
    public:
        FunctUint32Bit(uint32 *x, unsigned int bit, bool neg=false) : _x(x), _bit(bit), _neg(neg)
        {
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctUint32Bit constructor(uint32*, uint, bool=0)");
        }
        virtual void doit(double *v)
        {
            if (_neg==false) *v = (double)((*_x >> _bit) &1 );
            else *v = (double)(  (~(*_x) >> _bit) &1) ;
        }
    private:
        uint32  *_x;
        unsigned int  _bit;
        bool  _neg;
};*/

class FunctCrateID : public Funct {
    public:
        FunctCrateID(uint32 *x, unsigned int bit0) : _x(x), _bit0(bit0)
        {
           _logger = Logger::get("FUNCTCRATEID",Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])   );
           _logger->log(Logger::LOG_LEV_TRACE, "FunctCrateID constructor(uint32*, uint)");
        }
        virtual void doit(double *v)
        {
            *v = (double)( (*_x >> _bit0) & 0xF );
            _logger->log(Logger::LOG_LEV_TRACE, "FunctCrateID  *v=%g, bit0=%d, (double*)=%p [%s:%d]", *v, _bit0, _x, __FILE__, __LINE__);
        }
    private:
        uint32  *_x;
        unsigned int  _bit0;
        Logger* _logger;
};

#include "DiagnSet.h"
#include "math.h"
class FunctDewPoint : public Funct {
    public:
        FunctDewPoint(DiagnVarPtr t, DiagnVarPtr hr) : _t(t), _hr(hr)
        {
           _logger = Logger::get("FUNCTDEWPOINT",  Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])   );
           _logger->log(Logger::LOG_LEV_TRACE, "FunctDewPoint constructor");
        }
        virtual void doit(double *v)
        {
            double t  = _t->Value();
            double hr = _hr->Value();
            *v = DewPoint(SVP(t+errt) * (hr+errhr)/100.0);
			_logger->log(Logger::LOG_LEV_TRACE, "FunctDewPoint: dewpoint %g ext_temp %g relative_humidity %g",*v, t, hr);
        }
    private:
        DiagnVarPtr _t;
        DiagnVarPtr _hr;
        double SVP(double t);
        double DewPoint(double vp);
        const static double errt  = 0.1; //let's suppose measured external temperature is 1C less than the real one
        const static double errhr = 5.0; //let's suppose measured relative humidity is 5% less than the real one

        Logger* _logger;
};


//@Class: FunctCheckDewPoint
//
// Used to compare dew point temperature with other temperatures in the system.
// The dew point DiagnVar is given as the first argument of the constructor.
// The temperatures to compare are specified by a DiagSet object as the second argument of the constructor
//
// This conversion class returns the difference between the lowest of the valid temperatured specified
// in the DiagnSet and the dew point. A positive value means that all the temperatures are above the dew point.
// A negative value means that at least one temperature is lower than the dew point.
// Use thresholds like these:
//  warning_max and alarm_max = inf
//  warning_min = 5  calls the warning handler in case the difference between the lowest temperature
//                   and the dew point is less than 5 degrees
//  alarm_min = 0    calls the alarm handler when the lowest temperature coincides with the dew point
//
// If no valid temperatures are found (i.e. all temperature vars are disabled) this function return 0
//
//@
class FunctCheckDewPoint : public Funct {
    public:
        FunctCheckDewPoint(DiagnVarPtr dp, DiagnSetPtr vars) : _dp(dp), _vars(vars)
        {
           _logger = Logger::get("FUNCTCHECKDEWPOINT",  Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])  );
           _logger->log(Logger::LOG_LEV_TRACE, "FunctDewPoint constructor");
        }
        virtual void doit(double *v)
        {
            double Tdp  = _dp->Value();
			double diff = 0;
            double val, lowval = -274;
            *v = 1000.;
            int cnt = 0;
            std::string who;
			for (DiagnSet::iterator it = _vars->begin(); it != _vars->end();  it++)
        	{
                if ( (*it)->isEnabled() ) {
                    val = (*it)->Value();
            	    diff = val - Tdp;
                    if (diff < *v)
                    {
                        *v = diff;
                        lowval = val;
                        who = (*it)->Name();
                    }
                    cnt++;
                }
        	}
		    _logger->log(Logger::LOG_LEV_TRACE, "FunctCheckDewPoint: dewpoint %g temp %g var %s. Return %g", Tdp, lowval, who.c_str(), *v);
        }
    private:
        DiagnVarPtr _dp;
        DiagnSetPtr _vars;
        Logger* _logger;
};

class FunctDSPControlCurrent : public Funct {
    public:
        FunctDSPControlCurrent(DiagnVarPtr dac, DiagnVarPtr ff, DiagnVarPtr bias) : _dac(dac), _ff(ff), _bias(bias)
        {
           _logger = Logger::get("FUNCTDSPCONTROLCURRENT", Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])  );
           _logger->log(Logger::LOG_LEV_TRACE, "FunctDSPControlCurrent constructor");
        }
        virtual void doit(double *v)
        {
            double v_dac  = _dac->Value();
            double v_ff   = _ff->Value();
            double v_bias = _bias->Value();
            *v = v_dac-v_ff-v_bias;
			_logger->log(Logger::LOG_LEV_TRACE, "FunctDSPControlCurrent: ControlCurrent: %g,  dac: %g, ff: %g, bias: %g [%s:%d]",
                    *v, v_dac, v_ff, v_bias, __FILE__, __LINE__);
        }
    private:
        DiagnVarPtr _dac;
        DiagnVarPtr _ff;
        DiagnVarPtr _bias;

        Logger* _logger;
};


#endif //HOUSEKEEPER_CONVERT_H_INCLUDED
