//@File: DiagnVar.h
//
//Definition of the class DiagnVar.
//
//author: Lorenzo Busoni.
//
//version 1.0
//
//Created on: 25 mar 2006
//
//Modification history:
//@
#ifndef DIAGNVAR_H_INCLUDED
#define DIAGNVAR_H_INCLUDED

#include <string>
#include <iostream>
#include <iomanip>  //setw

#include "DiagnRange.h"
#include "DiagnParam.h"
#include "DiagnValue.h"
#include "DiagnBuffer.h"
#include "RunningMean.h"
#include "Funct.h"
#include <boost/archive/binary_oarchive.hpp> //serialize
#include <boost/serialization/shared_ptr.hpp>
#ifndef BOOST_SERIALIZATION_SHARED_PTR_HPP
    #include "boost/smart_ptr.hpp"
#endif

using namespace Arcetri;

// Forwarded definition to use DiagnVar in the next typedef,
// because "Funct.h" needs DiagnVarPtr!
class DiagnVar;
typedef boost::shared_ptr<DiagnVar> DiagnVarPtr;

//@Class: DiagnVar
// DiagnVar implements a variable whose value has to be monitored
// by a DiagnApp.
//
// A DiagnApp keep a list of the DiagnVars it wants to monitor and periodically calls their method Update().
// In the following we describe what happens in a DiagnVar when an update event is triggered.
// \begin{itemize}
// \item If the DiagnVar is disabled (see DiagnParam @see{DiagnParam}) nothing happens,
// \item Otherwise the actual value of the variable is determined invoking a conversion function. The conversion function
// can have as input whatever data available inside the application but can return only a scalar double. So it is a $f(x0, xn) -> R^0$.
// This means that a single DiagnVar cannot monitor vectorial quantities.
//
// \item The actual value of the variable determined in the previous step is pushed in a FIFO buffer to compute a running mean.
// A timestamp associated to the actual value of the variable is also pushed in another FIFO buffer. The two running mean buffers can be seen
// as a FIFO buffer of pairs time/value. The length of the buffers is user-specified and can be modified at run time. Since the user
// specifies the length of the buffer in seconds, the number of samples in the buffers depends on the rate of the update events that is tipically
// varying during at run-time. To account for this, after pushing the actual timestamp/value pair in the FIFOs, the algorithm starts popping from the end
// of the FIFOs the values older than (actual time - running mean period).
// The compution of the statistical values of the RunningMean is done always after every pop or push. See RunningMean @see{RunningMean} for a complete
// list of the available information from a RunningMean object.
//
// \item The mean value of the running mean buffers is compared with 4 user-defined thresholds (alarm-min, warning-min, warning-max, alarm-max).
// If warning-min < mean-value < warning-max, then the status of the DiagnVar is set to OK
// If alarm-min < mean-value < warning-min or warning-max < mean-value < alarm-max , then the status of the DiagnVar is set to WARNING and a
// user-defined warning handler is called.
// If mean-value < alarm-min or mean-value > alarm-max, then the status of the DiagnVar is set to ALARM and a user-defined alarm handler is called.
// TODO manca il CAF.
//
// \item TODO Periodically, the mean-value is inserted in another FIFO buffer (aka History) (indeed 2 buffers, one for values and one for times) to
// create a long term decimated history of the variable. Sampling period and length of this buffer are user-defined and can be modified at run time.
//
// \end{itemize}
//
// TODO manca slow/fast.
// TODO manca la statistica su i tempi di update
//
//
// NOTES
// The conversion function, warning handler and alarm handler are defined when the DiagnVar is constructed and cannot be changed.
// If a DiagnVar doesn't need to be averaged (e.g. a counter), set a very short value for the mean period in order to be sure that the RunningMean buffers
// will always contains only one sample.
//
// QUESTA VA ALTROVE. DiagnApp policy (at the moment) is to give the user full freedom. This means that the user can require a 1-year@1kHz history buffer
// eventually eating all the available memory.
//
//
// A DiagnVar object keeps inside all the informations needed to update its value:
// \begin{itemize}
// \item a Funct object (\verb+_fConv+) used to convert and elaborate the "raw" data.
// \item a Funct object (\verb+_warning_hndl+) used when the warning status is detected.
// \item a Funct object (\verb+_alarm_hndl+) used when the alarm status is detected.
// \item a DiagnParam object that keeps the parameters that specify the behaviour of the DiagnVar.
// \item a RunningMean buffer used to store the timestamps
// \item a RunningMean buffer used to store the values
// \end{itemize}
//
// The only information needed from the external caller (the DiagnApp) is the timestamp to associate with the update event.
//
// The DiagnParam object can be imported during the execution thus changing the behaviour of the DiagnVar. Every parameter in
// DiagnParam can be modified at run time via the ImportParams. See @see{DiagnParam}.
//
// Internal data can be retrieved by mean of \verb+ExportParams()+ @see{ExportParams}, \verb+ExportValues()+  @see{ExportValues},
// \verb+ExportBuffers()+  @see{ExportBuffers};
//
//
// Note that, at the level of the \verb+DiagnVar+, nothing prevents an user to retrieve the data (using \verb+ExportValues()+  @see{ExportValues} or
// \verb+ExportBuffers()+  @see{ExportBuffers}) during an \verb+Update()+; this would lead to unconsistency in the retrieved data.
// The solution of synchronizing the access to the RunningMean @see{RunningMean} with mutexes at the level of the DiagnVar object is the less error prone but it
// has been discarded because of the excessive CPU's cost: a diagnostic application can easyly have more than 10000 \verb+DiagnVar+s and a synchronization
// implemented at the level of the single \verb+DiagnVar+ would require locking and unlocking 10000 mutexes per \verb+Update()+ cycle.
// At the contrary, the synchronization can be done in the \verb+DiagnApp+ object on sets of \verb+DiagnVar+s, thus resulting in only a few mutex's locks per cycle.
// This solution has the drawback of increasing the complexity of the \verb+DiagnApp+ code.
//
//
//
//@
class DiagnVar
{
    //////////// construction ///////////
    public:
        DiagnVar(
                const std::string &   /*family_name*/,
                unsigned int          /*index*/,
                FunctPtr              /*fConv*/,
                FunctActionPtr        /*warning_hndl*/,
                FunctActionPtr        /*alarm_hndl*/
                );
        ~DiagnVar();

    //////////////  methods /////////////
    public:
        void                    Update(double timestamp);
        void                    CopyOnLocal()       {_fConv->copy();}   // Save raw data for later conversion
        void                    ResetFunctAction()  {_warning_hndl->BeginLoop(); _alarm_hndl->BeginLoop(); _fConv->BeginLoop();}

//@C
//verb+Value()+ return the value of the DiagnVar average over the running mean
//@
        double                  Value() {return _run_mean.Mean();}
//@C
//verb+Time()+ return the average value of verb+_time_run_mean+ i.e. the time Value() can be assigned to
//@
        double                  Time() {return  _time_run_mean.Mean();} // @return Time of
//@C
//verb+StdDev()+ return stddev of values in the running mean
//@
        double                  StdDev(){return _run_mean.StdDev();}
//@C
//verb+Last()+ return last unaveraged value inserted in the running mean buffer
//@
        double                  Last()  {return _run_mean.Last();}

        std::string                 Name();
        const std::string &         Family() {return _family;}
        unsigned int                Index() {return _index;}
        int                         Status()  {return _status;}

        size_type                   MeanSize()     {return _run_mean.NElem();}
        double                      MeanPeriod()   {return _param.MeanPeriod();}

        const DiagnRange<float> &   Range() {return _param.Range();}
        unsigned int                ConsAllowFaults() {return _param.ConsAllowFaults();}

        bool                    isAlarm(){return _param.isAlarm(Value());}
        bool                    isWarning(){return _param.isWarning(Value());}
        bool                    isOk(){return _param.isOk(Value());}
        bool                    isEnabled() {return _param.isEnabled();}
        bool                    isSlow(){return _param.isSlow();}

        double                  GetUpdatePeriod()    {return _run_mean.UpdateAve();}
        double                  GetUpdatePeriodMin() {return _run_mean.UpdateMin();}
        double                  GetUpdatePeriodMax() {return _run_mean.UpdateMax();}
        long                    GetCounter()         {return _run_mean.Ctr();}

        void                    ImportParams(DiagnParam &);
        DiagnParam&             ExportParams();
        DiagnValue              ExportValues();
        DiagnBufferPtr          ExportBuffers();
        //DiagnBufferPtr          ExportHistory();

        std::string             str() { std::ostringstream out; out << *this; return out.str();}

    private:
        void           Check();
        void           ConvRaw (double *v) {_fConv->doit(v);}  /**< Convert raw data */
        void           Warn(double f)  {_warning_hndl->react(Name(), f);}
        void           Alarm(double f) {_alarm_hndl->react(Name(), f);}

        friend class boost::serialization::access;
        template<class Archive>
        void  serialize(Archive & ar, const unsigned int /* file_version */){
            //ar & _fConv & _warning_hndl & _alarm_hndl & ;
            // TODO in Funct c'e' un double *v che e' difficile da far serializzare
            ar & _run_mean & _time_run_mean & _status & _family & _index & _param;
        }


    friend std::ostream &operator<<(std::ostream & output, DiagnVar & p)
    {
        output.precision(3);
        output <<  std::setw(25) << p.Name()  << ": " << std::setw(9) <<  p.Value();
        output << " (" << std::setw(9) << p.StdDev() << ")";
        output << std::setw(7) << p.Time();
        output << std::setw(11) << p.Last();
        output << std::setw(6) << p.MeanSize();
        output << std::setw(3) << p.Status();
        //output << " ["<< std::setw(7)  << p.GetUpdatePeriodMin() <<"/"<< std::setw(7) <<
        //    p.GetUpdatePeriod() <<"/" << std::setw(7)  << p.GetUpdatePeriodMax() << "]";
        output <<  std::setw(8) << p.GetCounter();
        output << p._param;
        return output;
    }


    ////////////// variables ////////////
    protected:
        FunctPtr                 _fConv;              // to convert from raw data to mks
    private:
        FunctActionPtr           _warning_hndl;
        FunctActionPtr           _alarm_hndl;

        RunningMean<double>      _run_mean;            // holds data used for the running mean
        RunningMean<double>      _time_run_mean;       // holds timestamps of data. TODO could be done float to save memory: precision issue
        //RunningMean<float>      _history;             // holds data used for the history running mean
        //RunningMean<float>      _time_history;        // holds timestamps of data in the history rm
        //double                   _time_history_last;   // holds timestamp of last insertion into _history
        int                      _status;              // Status::OK, Status::WARNING, Status::ALARM

        // variable name (typically _family+_index, used for displaying and log)
        std::string              _family;              // family name
        unsigned int             _index;               // progressive index in the family

        DiagnParam               _param;               // parameters used to configure DiagnVar behaviour
        unsigned int             _caf;                 // counter of consecutive faults

        double                   _oldtimestamp;        // timestamp of the last insertion in the running mean buffers
        Logger*                  _logger;
};



//@Class: MatchDiagnVar
//
//@
class MatchDiagnVar
{
    public:
        MatchDiagnVar(const std::string &s, unsigned int n) : 
            _s(s), 
            _n(n) 
        {
            std::transform(_s.begin(), _s.end(), _s.begin(), (int(*)(int)) std::toupper);
        };
        bool operator() (const DiagnVarPtr & obj)
        {
            //Logger::get()->log(Logger::LOG_LEV_INFO, "MatchDiagnVar %s-%d vs %s-%d", _s.c_str(), _n, obj->Family().c_str(), obj->Index());
            return (obj->Family()==_s && obj->Index()==_n);
        }

    friend std::ostream &operator<<(std::ostream & output, const MatchDiagnVar & p)
    {
        output <<  p._s << ":" << p._n ;
        return output;
    }

    private:
        std::string     _s;
        unsigned int    _n;
};


#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
namespace std
{
    template<> struct less<DiagnVarPtr>
    {
        bool operator()(const DiagnVarPtr p1, const DiagnVarPtr p2)
        {
          if(!p1)
            return true;
          if(!p2)
            return false;
          if(p1->Family() < p2->Family()) return true;
          if(p1->Family() == p2->Family()  && p1->Index() < p2->Index()) return true;
          return false;
        }
    };
};

#endif  // DIAGNVAR_H_INCLUDED
