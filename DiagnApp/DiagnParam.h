//@File: DiagnParam.h
//
//Definition of the class DiagnParam.
//
//author: Lorenzo Busoni.
//
//version 1.0
//
//Created on: 30 Jul 2006
//
//Modification history:
//@ 
#ifndef DIAGNPARAM_H_INCLUDED
#define DIAGNPARAM_H_INCLUDED

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>  //setw

//#include "RunningMean.h" //size_type
#include "DiagnRange.h"


#include <boost/archive/binary_oarchive.hpp> //serialize

//@Class: DiagnParam
//
//Parameters for a DiagnVar object.
//
//This class contains all the parameters that can be used to specify 
//the behaviour of a DiagnVar object.
//@
class DiagnParam
{
    ////////////// methods   ////////////
    public:
        DiagnParam()  {
            _cons_allow_faults=0;        // consecutive allowed faults
            _enabled=false;              // toggle enable/disable
            _slow=false;                 // update variable in a separate thread
            _run_mean_period=1.0;        // requested period for the running mean [s]
            _mask=0;
        }

        float                           MeanPeriod() {return _run_mean_period;}
        DiagnRange<float > &            Range() {return _rng;}
        float                           AlarmMin() {return _rng.AlarmMin();}
        float                           AlarmMax() {return _rng.AlarmMax();}
        float                           WarningMin() {return _rng.WarningMin();}
        float                           WarningMax() {return _rng.WarningMax();}
        unsigned int                    ConsAllowFaults() {return _cons_allow_faults;}
        bool                            isEnabled() {return _enabled;}
        bool                            isSlow(){return _slow;}


        void            SetMeanPeriod(float  period) {
                            if( !isfinite(period) || period < 0 ) {
                            	std::ostringstream s;
       							s << "SetMeanPeriod " << period;
								throw AOInvalidNumberException(s.str(), INVALID_NUMBER_ERROR,  __FILE__,__LINE__); 
							}
							_run_mean_period=period; 
                            SetMask(MaskMeanPeriod);
                        }
        void            SetConsAllowFaults(unsigned int caf){
                            _cons_allow_faults=caf; 
                            SetMask(MaskConsAllowFaults);
                        }
        void            SetEnabled(bool ena) {_enabled = ena; SetMask(MaskEnabled); }
        void            SetSlow(bool slow) {_slow = slow; SetMask(MaskSlow); }

        void            SetRange(DiagnRange<float > rng) {_rng=rng;} 
        void            SetAlarmMin(float  amin) {_rng.SetAlarmMin(amin);} 
        void            SetAlarmMax(float  amax) {_rng.SetAlarmMax(amax);} 
        void            SetWarningMin(float  wmin) {_rng.SetWarningMin(wmin);}
        void            SetWarningMax(float  wmax) {_rng.SetWarningMax(wmax);}


        bool            isAlarm(double value){return _rng.isAlarm(value);}
        bool            isWarning(double value){return _rng.isWarning(value);}
        bool            isOk(double value){return _rng.isOk(value);}


        bool            isSetMeanPeriod() { return isSetMask(MaskMeanPeriod);}
        bool            isSetConsAllowFaults() { return isSetMask(MaskConsAllowFaults);}
        bool            isSetEnabled() { return isSetMask(MaskEnabled);}
        bool            isSetSlow() {  return isSetMask(MaskSlow);}

        void            Reset() { _mask = 0;}

        void            Import(DiagnParam& param){
                            if (param.isSetMeanPeriod())        SetMeanPeriod(param.MeanPeriod());
                            if (param.isSetEnabled())           SetEnabled(param.isEnabled());
                            if (param.isSetSlow())              SetSlow(param.isSlow());
                            if (param.isSetConsAllowFaults())   SetConsAllowFaults(param.ConsAllowFaults());
                            _rng.Import(param.Range());
                            //_rng.CheckOrder();
                        }

        std::string     str(){std::ostringstream oss; oss << *this; return oss.str();}

    private:
        bool            isSetMask(unsigned int parameter) { return (_mask >> parameter) & 1; }
        void            SetMask(unsigned int parameter)   { _mask = (( 1<<parameter) | _mask); }

        const static  int  MaskMeanPeriod=1;
        const static  int  MaskConsAllowFaults=3;
        const static  int  MaskEnabled=4;
        const static  int  MaskSlow=5;

    
        friend class boost::serialization::access;
        template<class Archive>
        void  serialize(Archive & ar, const unsigned int /* file_version */){
            ar & _cons_allow_faults & _rng & _enabled & _slow & _run_mean_period & _mask;
        }

    friend std::ostream &operator<<(std::ostream & output, const DiagnParam & p)
    {
        //output.precision(3);
        //if (p._enabled) output  << " ena ";
        //else            output  << " dis ";

        //if (p._slow)    output  << " slow ";
        //else            output  << " fast ";

        //output <<  " CAF: "  << std::setw(3) << p._cons_allow_faults;
        //output <<  " Rng: "  << p._rng;
        //output <<  " RMP[s]: " << std::setw(6) << p._run_mean_period;
		
		output << p._rng;
		output << setw(8) << p._run_mean_period;
		output << setw(8) << p._cons_allow_faults;
        
		if (p._enabled) output  << setw(6) << "ena";
        else            output  << setw(6) << "dis";

        if (p._slow)    output  << setw(6) << "slow";
        else            output  << setw(6) << "fast";
        output << " mask " << p._mask;
        return output;
    }
    
    friend std::istream &operator>>(std::istream & input, DiagnParam & p)
    {
        std::string        s;
        unsigned int       ui;
        DiagnRange<float > r;
        float              sz;
        
        input >> r;   p.SetRange(r);
        
        ExtractFromStream<float>(input, sz); //input >> sz;
        p.SetMeanPeriod(sz); 

        ExtractFromStream<unsigned int>(input, ui);  //input >> ui;
        p.SetConsAllowFaults(ui);

        input >> s; 
        if ( !s.compare(0,3,"dis") || !s.compare("false") ) p.SetEnabled(false);
        else p.SetEnabled(true);
        input  >>  s; 
        if ( !s.compare("slow") || !s.compare("true") ) p.SetSlow(true);
        else p.SetSlow(false);
        return input;
    }

    public:
    bool operator ==(DiagnParam& y)
    {
        if (_run_mean_period != y.MeanPeriod()) return false;
        if (_rng != y.Range()) return false;
        if (_cons_allow_faults != y.ConsAllowFaults()) return false;
        if (_enabled != y.isEnabled()) return false;
        if (_slow != y.isSlow()) return false;
        return true;
    }

    ////////////// variables ////////////
    private:
        unsigned int             _cons_allow_faults;  // consecutive allowed faults
        DiagnRange<float >       _rng;

        bool                     _enabled;            // toggle enable/disable
        bool                     _slow;    // update variable in a separate thread
        //CircBuffer      history;         //  
        float                    _run_mean_period;        // requested period for the running mean
        
        unsigned int             _mask;
};

#include <boost/serialization/tracking.hpp>
BOOST_CLASS_TRACKING(DiagnParam, boost::serialization::track_never);

#endif

