/*
 * @file DiagnValue.cpp
 * 
 * Implementation of the DiagnValue class
 *
 */

#include "Logger.h"
#include "DiagnValue.h"

using namespace Arcetri;
using namespace Arcetri::Diagnostic;

DiagnValue::DiagnValue(
        double mean, 
        double stddev,
        double last,
        double time,
        int    status
        ):
    _mean(mean),
    _stddev(stddev),
    _last(last),
    _time(time),
    _status(status)
{};



