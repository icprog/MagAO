/*
 * @file DiagnVar.cpp
 *
 * Implementation of the DiagnVar class
 *
 */

#include "Logger.h"
#include "diagnlib.h"

#include "DiagnVar.h"
#include <iostream>
#include <iomanip>
#include <sstream>  //ostringstream

using namespace Arcetri;
using namespace Arcetri::Diagnostic;

DiagnVar::DiagnVar(
        const std::string  &family_name,
        unsigned int        index,
        FunctPtr            fConv,
        FunctActionPtr      warning_hndl,
        FunctActionPtr      alarm_hndl
        ):
    _fConv(fConv),
    _warning_hndl(warning_hndl),
    _alarm_hndl(alarm_hndl),
    _run_mean((size_type)(10)),
    _family(family_name),
    _index(index)
{
    std::transform(_family.begin(), _family.end(), _family.begin(), (int(*)(int)) std::toupper);
    _logger = Logger::get("DIAGNVAR");
    _logger->log(Logger::LOG_LEV_TRACE, "DiagnVar() c'tor %s:%d [%s:%d]", _family.c_str(), _index,  __FILE__, __LINE__);
    _status=Status::OK;
    _caf=0;
    _oldtimestamp=-1;
}

DiagnVar::~DiagnVar()
{
    _logger->log(Logger::LOG_LEV_TRACE, "DiagnVar() d'tor %s:%d [%s:%d]", _family.c_str(), _index,  __FILE__, __LINE__);
}


//@Member: Name
//
//@
std::string DiagnVar::Name()
{
    std::ostringstream os;
    os << _family << "-"<< std::setfill('0')<< std::setw(4) << _index; // << std::ends;
    return os.str();
}


/**
 * @Member: Update
 *
 * Function that update the variable with a new current value
 *
 * Calls ConvRaw to convert raw data
 * Updates running mean buffer
 * Check range
 *
 * @see ConvRaw()
 * @see RunningMean
 * @see Check()
 **/
void DiagnVar::Update(double timestamp)
{
    double value;
    float  meanperiod=_param.MeanPeriod();
    if (_param.isEnabled()){
    	try {
    		ConvRaw(&value);               // Convert raw value
    	}
    	catch(AOException& e) {
    		_logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
    		throw AOException("DiagnVar::Update()] Conversion error: " + Name(),GENERIC_SYS_ERROR,__FILE__,__LINE__);
    	}

        _run_mean.PushOne(value);           // Push the new value into running mean
        _time_run_mean.PushOne(timestamp);  // Push the new timestamp into queue of timestamp

        //@C
        // if oldtimestamp > timestamp  then it means the hardware has been reset and
        // I have to flush all the running means
        //@
        if( _oldtimestamp > timestamp ){
            _run_mean.Resize(1); // Resize() means discard from front (i.e. oldest first)
            _time_run_mean.Resize(1); // Leave only the value just inserted.
        }
        _oldtimestamp = timestamp;


        while(_time_run_mean.front() < timestamp - meanperiod ){
            //_logger->log(Logger::LOG_LEV_TRACE, "[DiagnVar::Update()] front_pop %g [time %g, period %g] size is %d [%s:%d]",
            //        _time_run_mean.front(), timestamp, MeanPeriod(), _time_run_mean.NElem(), __FILE__, __LINE__);
            _time_run_mean.PopOne();
        }
        //_time_run_mean.Resize(_time_run_mean.NElem());
        _run_mean.Resize(_time_run_mean.NElem());

        Check();                       // Check ranges
        // TODO update history if needed.
        /*if (_history_sampling != 0 && _time_history_last < timestamp - _history_sampling) {
            _history.PushOne(_run_mean.Mean());
            _time_history.PushOne(_time_run_mean.Mean());
            _time_history_last = timestamp;
            while(_time_history.front() < timestamp - _history_period ){
                _time_history.PopOne();
            }
            _history.Resize(_time_history.NElem());
        }*/
    }
}



//@Member: Check
//
// Check if the variable averaged value is outside the allowed range
//
// If the variable value is in the alarm range, calls the Alarm handler function.
// Else, if it is in the warning range, calls the Warning handler function.
// In any case set the status variable to one of "OK", "Warning", "Alarm"
//
// IMPORTANT NOTE: Warning and Alarm handler functions should return quickly.
//@
void DiagnVar::Check()
{
    if (isOk()) {
        _status = Status::OK;
        _caf = 0;
        return;
    } else _caf++;

    if (_caf >= ConsAllowFaults() ) {
        if (isAlarm()){
            Alarm(Value());
            _status = Status::ALARM;
        } else {
            Warn(Value());
            _status = Status::WARNING;
        }
        _caf = 0;
    }
}

//@Member: ImportParams
//
//Import configuration from a DiagnParam
//@
void DiagnVar::ImportParams(DiagnParam & param)
{
    _param.Import(param);
}


//@Member: ExportParams
//
//Export configuration to a DiagnParam
//@
DiagnParam&  DiagnVar::ExportParams()
{
    return _param;
}

//@Member: ExportValues
//
//Export values to a DiagnValue
//@
DiagnValue  DiagnVar::ExportValues()
{
    return DiagnValue(Value(), StdDev(), Last(), Time(), Status());
}

//@Member: ExportBuffers
//
// Return a DiagnBuffer object containing
// \verb+_time_run_mean+  and \verb+_run_mean+
//@
DiagnBufferPtr  DiagnVar::ExportBuffers()
{
    DiagnBufferPtr  buf ( new DiagnBuffer(_time_run_mean, _run_mean) );
    _logger->log(Logger::LOG_LEV_TRACE, "[DiagnVar::ExportBuffers()] object created [%s:%d]",  __FILE__, __LINE__);
    return buf;
}

//@Member: ExportHistory
//
// Return a DiagnBuffer object containing
// \verb+_time_history+  and \verb+_history+
//@
/*DiagnBufferPtr  DiagnVar::ExportHistory()
{
    DiagnBufferPtr  hist ( new DiagnBuffer(_time_history, _history) );
    _logger->log(Logger::LOG_LEV_TRACE, "[DiagnVar::ExportHistory()] object created [%s:%d]",  __FILE__, __LINE__);
    return hist;
}*/


