#ifndef ANEMOMETERMON_H
#define ANEMOMETERMON_H

#define ANEMOMETERMON_BASE_PERIOD 20 // [milli sec] (should be less than min sample period)
#define ANEMOMETERMON_RETRY_TIME  5  // [sec]
#define ANEMOMETERMON_RTDBNAME "ANEM."

#include <AOApp.h>
#include "env/Anemometer.h"

#include <list>
#include <algorithm>
#include <numeric>

using namespace Arcetri;
using std::accumulate;

//
//
class MonAverage {
  public:
    MonAverage(unsigned int size = 0);

    double average();
    void add_value(double val);
    void clear();

  private:
    list <double> _buffer;
    unsigned int _size;
};

   
//
//
class AnemometerMon : public AOApp {
  public:
    AnemometerMon( int argc, char **argv) throw (AOException);
    ~AnemometerMon();
    
  protected:
    // AOApp
    void PostInit();
    void Run();
    
    // Internal
    void Periodic();
    void Connect();

  private:
    int _mstimeout;
    string _host;
    int _port;
    bool _dosetup;
    bool _isconnected;

    AnemometerRate _rate;
    Anemometer * _an;
    chardev *_dev;

    RTDBvar _varSpeed;
    RTDBvar _varCheck;

    timeval _lastUpdate;

    MonAverage _averages[3][4];
};











#endif // ANEMOMETER_H
