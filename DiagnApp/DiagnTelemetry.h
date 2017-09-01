
//
//
//@

#ifndef DIAGNTELEMETRY_H_INCLUDED
#define DIAGNTELEMETRY_H_INCLUDED

#include "DiagnSet.h"
#include "Logger.h"
#include <map>

using namespace Arcetri;

//@Class: DiagnTelemetry
//
// Class for logging telemetry in DiagnApps
// 
// All the DiagnVar's belonging to a given family are logged periodically
// 
// A configuration file containing pair FamilyName and log interval (in second) space-separated 
// is needed to construct the object
//
// 
//
//@

class DiagnTelemetry
{
    public:
        DiagnTelemetry(const string  conffile,
                       const string  telemetryfile,
                       DiagnSetPtr   vars2log,
                       const string  loggername, 
                       int logLevel = Logger::LOG_LEV_INFO);
        DiagnTelemetry();
    
        // Log vars 
    void LogTelemetry(float timestamp, bool force = false);

    private:
        // _who_when is a dictionary of DiagnVar's family names and telemetry log intervals.
        map<string, float>     _who_when;
        map<string, float>     _delta_map;
        
        DiagnSetPtr            _vars;

        Logger*                _logger;

        bool                   _initialized;

        float                  _first_timestamp;
 
        string                 _telemetry_file;

};

#endif 
