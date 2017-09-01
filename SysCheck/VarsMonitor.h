#ifndef VARSMONITOR_H
#define VARSMONITOR_H

#define VARSMONITOR_BASE_PERIOD 500
#define VARSMONITOR_MONITOR_STR "MONITOR."


#include <AOApp.h>

using namespace Arcetri;

class MonitorStruct {
  public:
    int period;
    time_t lastlog;
    time_t lastupdate;    
    Variable * lastvalpt;
    Logger * log;
};

class VarsMonitor : public AOApp {
  public:
    VarsMonitor( int argc, char **argv) throw (AOException);
    ~VarsMonitor();
    
  protected:
    // AOApp
    void PostInit();
    void Run();
    
    // Internal
    void Periodic();
    void ReadVariablesConfig();
    void CheckAndLog(MonitorStruct &mon);

    // Callbacks
    static int OnNotify( void *, Variable *);

  private:
    map<string, MonitorStruct> m_mon;

};











#endif // VARSMONITOR_H
