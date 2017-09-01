#include <fstream>
#include "DiagnTelemetry.h"

//@
//
// Parse the config file of telemetry and return a dictionary <Family, TelemetryPeriod>,
// where Family is a string and TelemetryPeriod is the logger period in seconds.
//
// Telemetry config file name must be declared in DiagnApp config file
// under the tag TelemetryCfgFile. In case this tag or the file are absent, no telemetry logging
// will be performed. Path is relative to ADOPT_ROOT.
//
// Each line in the config file is composed of a Family name (string) and a 
// telemetry log period in seconds (float)
//
// Telemetry will be logged in telemetryfile (absolute path)
//
//@

DiagnTelemetry::DiagnTelemetry(string conffile, string telemetryfile, DiagnSetPtr vars, string loggername, int loglevel)
{
    _logger = Logger::get(loggername, loglevel);
    _vars = vars;
    _telemetry_file = telemetryfile;
    _initialized = false;
    _first_timestamp = 0.0;
    
    char *adopt_root = getenv("ADOPT_ROOT");
    if (!adopt_root) throw Config_File_Exception("ADOPT_ROOT environment variable does not exist");
    //string conffileabs = std::string(adopt_root) + "/" + conffile;
    string conffileabs = Utils::getAdoptRoot()+"/conf/"+Utils::lowercase(Utils::getAdoptSubsystem())+"/current/processConf/" + conffile;
    
    
    // parse telemetry configuration file and populate _who_when
    std::ifstream in(conffileabs.c_str());
    if (!in) throw Config_File_Exception("Unable to find configuration file: " + conffileabs, FILE_ERROR, __FILE__, __LINE__);
    
    string                        family;
    float                         period;
    std::string                   line;
    std::string::size_type        first;
    while (getline(in,line))
    {
        //Strip blank characters
        first = line.find_first_not_of(" \t");
        if (first != std::string::npos)
            line = line.substr(first, line.find_last_not_of(" \t")-first+1);
        //Checks if comment line
        first = line.find('#');
        //Remove comment ad the end of the line
        if (first != std::string::npos) line = line.substr(0, first);
        // Found only comment or empty line
        if (line.size() <1) continue;
        // Parse text
        istringstream ssline(line);
        //WhichParam wp;
        ssline >> family;
        ExtractFromStream<float>(ssline, period); 
        _who_when.insert ( pair<string,float>(family, period)  );
        _logger->log(Logger::LOG_LEV_INFO, "Add to telemetry  %s %f  [%s:%d]", family.c_str(), period,  __FILE__, __LINE__);
    }
}



void DiagnTelemetry::LogTelemetry(float timestamp, bool force)
{

    if (_who_when.empty() ) return;

    if (_initialized==false) {
        _first_timestamp = timestamp;
        _delta_map = _who_when;
        _initialized=true;
    }
   
    float ora  = timestamp - _first_timestamp;
    _logger->log(Logger::LOG_LEV_TRACE, "ORA %f [%s:%d]", ora, __FILE__, __LINE__);
    
    // for debug
    //for (map<string, float>::iterator  it= _who_when.begin(); it !=_who_when.end();   it++ )
    //     _logger->log(Logger::LOG_LEV_TRACE, "PRIMA %s %f [%s:%d]",(*it).first.c_str(), (*it).second, __FILE__, __LINE__);


    for (map<string, float>::iterator it = _who_when.begin() ; it != _who_when.end(); it++ ){
        string family = (*it).first;
        float  time_to_log = (*it).second;
        if ((time_to_log > ora) && (!force))
            continue; 
        _logger->log(Logger::LOG_LEV_TRACE, "GUARDO %s %f [%s:%d]",family.c_str(), time_to_log,__FILE__, __LINE__);
        try {
            DiagnSetPtr vars2log  = _vars->Filter(DiagnWhich(family, DiagnWhich::All, DiagnWhich::All, DiagnWhich::Fast, DiagnWhich::Enabled));
            if ( vars2log->empty() == false ) {
                _logger->log(Logger::LOG_LEV_TRACE, "GETTO %s  [%s:%d]",family.c_str(),__FILE__, __LINE__);
                vector<double> vvv = vars2log->GetValues() ;
                _logger->log(Logger::LOG_LEV_INFO, "Now is %f : logging %s %f [%s:%d]",ora, family.c_str(), time_to_log, __FILE__, __LINE__);
                Logger::get(family, Logger::LOG_LEV_INFO, _telemetry_file)->log_telemetry(Logger::LOG_LEV_INFO,  vvv ) ;
            }
        } catch (DiagnSetException &e) {
            _logger->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
        }
        //float delta = _delta_map[family];
        _who_when[family] += _delta_map[family];
    }

    // for debug
    //for (map<string, float>::iterator  it= _who_when.begin(); it !=_who_when.end();   it++ )
    //     _logger->log(Logger::LOG_LEV_TRACE, "DOPO %s %f [%s:%d]",(*it).first.c_str(), (*it).second,__FILE__, __LINE__);
    
}
