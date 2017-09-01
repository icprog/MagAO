#ifndef DIAGNRANGE_H_INCLUDED
#define DIAGNRANGE_H_INCLUDED

#include <iostream> //cout
#include <boost/archive/binary_oarchive.hpp> //serialize

#ifndef DEBUG
#define DEBUG false
#endif

#include "convert.h"
#include "AOExcept.h"
#include "Logger.h"
using namespace Arcetri;

namespace Arcetri {

template<typename T>
class DiagnRange;

//@
// Generic exception for the DiagnRange
// 
//@
template<typename T>
class DiagnRangeException: public AOException {
	
	public:
		DiagnRangeException(
                std::string message,
                DiagnRange<T>* rng,
                //std::string range,
                int errcode = NO_ERROR,
                std::string file="",
                int line = 0
                ): AOException(message, errcode, file, line) 
        { 
            exception_id = "DiagnRangeException"; 
            _rng = *rng;
        }
		
        virtual ~DiagnRangeException() throw() {}
        
        std::string str()
        {
            std::ostringstream s;
            s << _rng << std::endl;
            return s.str();
        }

    private:
        DiagnRange<T> _rng; 
};


//@
// class: DiagnRange
//
// Describes alarm/warning ranges for a DiagnVar
//
//
// Limits are interpreted in a NON conservative way. For instance, a variable's value inside [warn_min, warn_max] is considered OK.
// This allows a range [-inf, inf] to be always OK regardless of the variable's value.
// A NaN variable's value is considered ALARM. 
// 
// Can be filled by a string through operator << and accept the format of strtod ( 1, 1.0, 1e10, inf, nan)
// A NaN value is not a valid range limit; Inf is a valide range limit.
// 
//
// TODO/BUGS
// Why is it templated ????????
// 
// REQUIRES
//
//@
template<typename T>
class DiagnRange
{
    //////////// construction ///////////
    public:
        DiagnRange() {
            _warn_min=0;
            _warn_max=0;
            _alar_min=0;
            _alar_max=0;
            _mask=0;
            /*if(DEBUG) std::cout << "--- DiagnRange constructor()" << std::endl;*/
        }
       /* DiagnRange(T* val) :
            _warn_min(val[1]),
            _warn_max(val[2]),
            _alar_min(val[0]),
            _alar_max(val[3]){
                _mask=0; //if(DEBUG) std::cout << "--- DiagnRange constructor(T*)" << std::endl;
        }*/
        DiagnRange(T amin, T wmin, T wmax, T amax)
        {
            SetAlarmMin(amin);
            SetAlarmMax(amax);
            SetWarningMin(wmin);
            SetWarningMax(wmax);
            CheckOrder();
            _mask=0;
            //if(DEBUG) std::cout << "--- DiagnRange constructor(T,T,T,T)" << std::endl;
        }
        DiagnRange(const DiagnRange& p) :
            _warn_min(p._warn_min),
            _warn_max(p._warn_max),
            _alar_min(p._alar_min),
            _alar_max(p._alar_max),
            _mask(p._mask)
        {
            //CheckOrder();
            //std::cout << "--- DiagnRange copy constructor()" << std::endl;
        }
        ~DiagnRange() {/*if(DEBUG) std::cout << "--- DiagnRange destructor()" << std::endl;*/}

    //////////////  methods ////////////
    public:
        bool isAlarm(const T& x)   const {
            if (isfinite(_alar_min) && isfinite(_alar_max)) { 
                if(isnan(x)) return true; 
                return x <  _alar_min || x >  _alar_max; 
            } 
            else 
                return false;
        } 
        bool isWarning(const T& x) const{if(isnan(x)) return true ; return x <  _warn_min || x >  _warn_max;}
        bool isOk(const T& x)      const{if(isnan(x)) return false; return x >= _warn_min && x <= _warn_max;}

        T    AlarmMin()   {return _alar_min;}
        T    AlarmMax()   {return _alar_max;}
        T    WarningMin() {return _warn_min;}
        T    WarningMax() {return _warn_max;}

/*        void SetRange  (const DiagnRange& p) { 
            _warn_min = p._warn_min;
            _warn_max = p._warn_max;
            _alar_min = p._alar_min;
            _alar_max = p._alar_max;
            SetMask(MaskAlarmMin);
            SetMask(MaskAlarmMax);
            SetMask(MaskWarningMin);
            SetMask(MaskWarningMax);
            Logger::get()->log(Logger::LOG_LEV_TRACE, "SetRange Rng: %s Mask: %d",str().c_str(), _mask);
        }
        */
        void SetAlarmMin  (T amin) {
            Logger::get("DIAGNRANGE")->log(Logger::LOG_LEV_TRACE, "SetAlarmMin %g Rng: %s Mask: %d",amin, str().c_str(), _mask);
            if (!isValidLimit(amin)) throw AONaNException("SetAlarmMin",  __FILE__,__LINE__);
            _alar_min=amin; 
            SetMask(MaskAlarmMin);
        }
        void SetAlarmMax  (T amax) {
            Logger::get("DIAGNRANGE")->log(Logger::LOG_LEV_TRACE, "SetAlarmMax %g Rng: %s Mask: %d",amax, str().c_str(), _mask);
            if (!isValidLimit(amax)) throw AONaNException("SetAlarmMax",  __FILE__,__LINE__);
            _alar_max=amax;
            SetMask(MaskAlarmMax);
        }
        void SetWarningMin(T wmin) {
            Logger::get("DIAGNRANGE")->log(Logger::LOG_LEV_TRACE, "SetWarningMin %g Rng: %s Mask: %d",wmin, str().c_str(), _mask);
            if (!isValidLimit(wmin)) throw AONaNException("SetWarningMin",  __FILE__,__LINE__);
            _warn_min=wmin;
            SetMask(MaskWarningMin);
        }
        void SetWarningMax(T wmax) {
            Logger::get("DIAGNRANGE")->log(Logger::LOG_LEV_TRACE, "SetWarningMax %g Rng: %s Mask: %d",wmax, str().c_str(), _mask);
            if (!isValidLimit(wmax)) throw AONaNException("SetWarningMax",  __FILE__,__LINE__);
            _warn_max=wmax;
            SetMask(MaskWarningMax);
        }
       
        void CheckOrder() {
            if ( _alar_min > _warn_min) {
                Logger::get("DIAGNRANGE")->log(Logger::LOG_LEV_WARNING, "[CheckOrder] _alar_min > _warn_min [%g > %g] [%s:%d]",_alar_min, _warn_min, __FILE__, __LINE__);
                throw DiagnRangeException<T>("alarm_min > warn_min ", this, NO_ERROR, __FILE__,__LINE__);
            }
            if ( _alar_max < _warn_max){
                Logger::get("DIAGNRANGE")->log(Logger::LOG_LEV_WARNING, "[CheckOrder] _alar_max < _warn_max [%g < %g] [%s:%d]",_alar_max, _warn_max, __FILE__, __LINE__);
                throw DiagnRangeException<T>("alarm_max < warn_max ", this, NO_ERROR, __FILE__,__LINE__);
            }
            if ( _warn_min > _warn_max ){
                Logger::get("DIAGNRANGE")->log(Logger::LOG_LEV_WARNING, "[CheckOrder] _warn_min > _warn_max [%g > %g] [%s:%d]",_warn_min, _warn_max, __FILE__, __LINE__);
                throw DiagnRangeException<T>("warn_min > warn_max ", this, NO_ERROR, __FILE__,__LINE__);
            }
        }  
        
        // if rng is not sorted don't modify this and throw DiagnRangeException.
        void Import(DiagnRange& rng){
            Logger* logger = Logger::get("DIAGNRANGE");
            DiagnRange tmp(*this);
            logger->log(Logger::LOG_LEV_TRACE, "[Import] Range Imported: %s Mask: %d [%s:%d]",rng.str().c_str(), rng._mask, __FILE__, __LINE__);
            logger->log(Logger::LOG_LEV_TRACE, "[Import] Range before  : %s Mask: %d [%s:%d]",tmp.str().c_str(), tmp._mask, __FILE__, __LINE__);
            if (rng.isSetMask(MaskAlarmMin))    tmp.SetAlarmMin(rng.AlarmMin());
            if (rng.isSetMask(MaskAlarmMax))    tmp.SetAlarmMax(rng.AlarmMax());
            if (rng.isSetMask(MaskWarningMin))  tmp.SetWarningMin(rng.WarningMin());
            if (rng.isSetMask(MaskWarningMax))  tmp.SetWarningMax(rng.WarningMax());
            tmp.CheckOrder();
            _alar_min = tmp.AlarmMin();
            _alar_max = tmp.AlarmMax();
            _warn_min = tmp.WarningMin();
            _warn_max = tmp.WarningMax();
            logger->log(Logger::LOG_LEV_TRACE, "[Import] Range Tmp     : %s Mask: %d [%s:%d]",tmp.str().c_str(), tmp._mask, __FILE__, __LINE__);
            logger->log(Logger::LOG_LEV_TRACE, "[import] Range Now is  : %s Mask: %d [%s:%d]",str().c_str(), _mask, __FILE__, __LINE__);
        }

        std::string     str(){std::ostringstream oss; oss << *this; return oss.str();}

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void  serialize(Archive & ar, const unsigned int /* file_version */){
            ar & _warn_min & _warn_max & _alar_min & _alar_max & _mask;
        }


        bool            isSetMask(unsigned int parameter) { return (_mask >> parameter) & 1; }
        void            SetMask(unsigned int parameter)   { _mask = (( 1<<parameter) | _mask); }
        bool            isValidLimit(T v)                 { if(isnan(v)) return false; return true; }

        const static  int  MaskAlarmMin=1;
        const static  int  MaskAlarmMax=2;
        const static  int  MaskWarningMin=3;
        const static  int  MaskWarningMax=4;

    friend std::ostream &operator<<(std::ostream & output, const DiagnRange&  p)
    {
        //output << '['<< p._alar_min << ',' << p._warn_min << ',' << p._warn_max << ',' << p._alar_max << ']';
        output << setw(12) << p._alar_min << setw(12) << p._warn_min << setw(12) << p._warn_max << setw(12) << p._alar_max << " (mask " << p._mask << ")";
        return output;   
    }

    friend std::istream &operator>>(std::istream & input, DiagnRange<T>&  p)
    {
        try { ExtractFromStream<T>(input, p._alar_min);} catch (AOOutOfRangeException) {}
        try { ExtractFromStream<T>(input, p._warn_min);} catch (AOOutOfRangeException) {}
        try { ExtractFromStream<T>(input, p._warn_max);} catch (AOOutOfRangeException) {}
        try { ExtractFromStream<T>(input, p._alar_max);} catch (AOOutOfRangeException) {}
        p.SetMask(MaskAlarmMin);
        p.SetMask(MaskAlarmMax);
        p.SetMask(MaskWarningMin);
        p.SetMask(MaskWarningMax);
        return input;   
    }

    public:
    bool operator ==(DiagnRange& y)
    {
        if (_alar_min != y.AlarmMin()) return false;
        if (_alar_max != y.AlarmMax()) return false;
        if (_warn_min != y.WarningMin()) return false;
        if (_warn_max != y.WarningMax()) return false;
        return true;
    }
    bool operator !=(DiagnRange& y)
    {
        if (_alar_min != y.AlarmMin()) return true;
        if (_alar_max != y.AlarmMax()) return true;
        if (_warn_min != y.WarningMin()) return true;
        if (_warn_max != y.WarningMax()) return true;
        return false;
    }

    ////////////// variables ////////////
    private:
        T               _warn_min;
        T               _warn_max;
        T               _alar_min;
        T               _alar_max;

        // Need a _mask in Import() to know which field has to be modified. 
        unsigned int    _mask;
};

} // namespace Arcetri
#endif // DIAGNRANGE_H_INCLUDED

