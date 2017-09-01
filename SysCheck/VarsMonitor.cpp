//@File: VarsMonitor.cpp
//
// RTDB variables logger 
//
// Takes a configuration file with variable names and logging period
// Example: varsmonitor.conf
//
// Server       string     127.0.0.1
// LogLevel     string     INF
//
// Monitor.Elevation       array   [ AOS.TEL.EL,               5 ]
// Monitor.Azimuth         array   [ AOS.TEL.AZ,               5 ]
// Monitor.ExtWindspeed    array   [ AOS.EXTERN.WINDSPEED,     5 ]
// Monitor.ExtWinddir      array   [ AOS.EXTERN.WINDDIRECTION, 5 ]
// Monitor.Windspeed       array   [ AOS.AMB.WINDSPEED,        5 ]
// Monitor.Dimmseeing      array   [ AOS.DIMM.SEEING,          10 ]
//
// Monitor.AOArbFsmState   array   [ AOARB.L.ADSEC.FSM_STATE,  1 ]
// Monitor.AdSecRefPos     array   [ ADSEC.L.REF_POS,          10 ]
// Monitor.AOArbCoilStat   array   [ AOARB.L.ADSEC.COIL_STATUS, 10 ]
//
// NOTES:
// The application will ask for variables change notification, thus will not do a polling.
// On each loop cycle and on any variable change notification will check if a log has to be done,
// the criteria is the time since the last log.
// The logs are only available for INT, REAL and CHAR rtdb variables, including arrays.
//
//@


#define VERS_MAJOR   1
#define VERS_MINOR   0


#include <sys/time.h>
#include <algorithm>

#include "VarsMonitor.h"
#include "Logger.h"

using namespace Arcetri;


//
//
VarsMonitor::VarsMonitor( int argc, char **argv) throw (AOException) : AOApp( argc, argv) {


}

//
//
VarsMonitor::~VarsMonitor() {
    
    map<string, MonitorStruct>::iterator it;

    // free variable buffers
    for (it = m_mon.begin(); it != m_mon.end(); it++) {

        MonitorStruct * mon = &it->second;
        if (mon->lastvalpt != NULL) {
            free(mon->lastvalpt);
            mon->lastvalpt = NULL;
        }
    }
}


//
//
void VarsMonitor::PostInit() {

    ReadVariablesConfig();
}




//
//
void VarsMonitor::Run() {

    while(!TimeToDie()) {
        Periodic();
        msleep(VARSMONITOR_BASE_PERIOD);
    }
    _logger->log(Logger::LOG_LEV_INFO, "Periodic loop stopped");
}



// Check if the variable has to be logged
//
void VarsMonitor::CheckAndLog(MonitorStruct &mon) {

    string str;
    timeval time;
    gettimeofday(&time, NULL);

    if ((mon.lastvalpt != NULL) &&
        /*(mon.lastupdate > mon.lastlog) && */
        (time.tv_sec - mon.lastlog > mon.period)) {
        
        switch (mon.lastvalpt->H.Type) 
            {
            case INT_VARIABLE:
                mon.log->log_telemetry(Logger::LOG_LEV_INFO, mon.lastvalpt->Value.Lv, mon.lastvalpt->H.NItems);
                break;
            case REAL_VARIABLE:
                mon.log->log_telemetry(Logger::LOG_LEV_INFO, mon.lastvalpt->Value.Dv, mon.lastvalpt->H.NItems);
                break;
            case CHAR_VARIABLE:
                str.assign(mon.lastvalpt->Value.Sv, mon.lastvalpt->H.NItems);
                mon.log->log_telemetry(Logger::LOG_LEV_INFO, &str, 1);
                break;
            default:
                // not supported
                break;
            }
        mon.lastlog = time.tv_sec;
    }
}



// Checks for updated variables and logs if the period sionce last logs has passed
//
void VarsMonitor::Periodic() {

    map<string, MonitorStruct>::iterator it;

    for (it = m_mon.begin(); it != m_mon.end(); it++) {
        CheckAndLog(it->second);
    }
}

// Variable change notification handler
//
int VarsMonitor::OnNotify( void *_this, Variable *var) {

    int size;
    timeval time;
    gettimeofday(&time, NULL);
    VarsMonitor * me = (VarsMonitor *)_this;

    // update variable
    me->m_mon[var->H.Name].lastupdate = time.tv_sec;

    if (me->m_mon[var->H.Name].lastvalpt != NULL)
        free(me->m_mon[var->H.Name].lastvalpt);

    me->m_mon[var->H.Name].lastvalpt = (Variable *)malloc(size = sizeof(VHeader) + VarSize(var->H.Type, var->H.NItems));

    if (me->m_mon[var->H.Name].lastvalpt != NULL) {
        _logger->log(Logger::LOG_LEV_DEBUG, "OnNotify(): Variable %s updated", me->m_mon[var->H.Name].lastvalpt->H.Name);
        // make a copy of the variable
        memcpy(me->m_mon[var->H.Name].lastvalpt, var, size);
        // check if the log for the variable can be done now
        me->CheckAndLog(me->m_mon[var->H.Name]) ;
    }

    return 0;
}

//
// Look for configration keys Monitor.xxx
// The log name is taken from 'xxx'
// Monitored variables list is built and notification requested
//
void VarsMonitor::ReadVariablesConfig() {

    Config_File::iterator it;

    m_mon.clear();
    for (it = ConfigDictionary().begin(); it != ConfigDictionary().end(); it++) {
        string name = it->first;
        std::transform(name.begin(), name.end(), name.begin(), (int(*)(int))std::toupper);

        if (name.find(VARSMONITOR_MONITOR_STR) == 0) {
            vector<string> params = it->second;
            
            m_mon[params[0]].period = atoi(params[1].c_str());
            m_mon[params[0]].lastlog = 0;
            m_mon[params[0]].lastvalpt = NULL;
            m_mon[params[0]].log = Logger::get(name.substr(strlen(VARSMONITOR_MONITOR_STR)));

            eNotify(params[0], OnNotify, true);

            _logger->log(Logger::LOG_LEV_INFO, "Monitoring %s (every %d sec)", params[0].c_str(), m_mon[params[0]].period);
        }
    }


}

// Main
//
int main(int argc, char *argv[]) 
{
    SetVersion(VERS_MAJOR,VERS_MINOR);

    try {
        VarsMonitor * app = new VarsMonitor( argc, argv);

        app->Exec();
    }
    catch(AOException& e) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
    }
    catch (...) {
        printf("Application initialization failed\n");
    }

    return 0;
}
