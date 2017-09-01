//@File: AnemometerMon.cpp
//
//@


#define VERS_MAJOR   1
#define VERS_MINOR   0


#include <sys/time.h>
#include <algorithm>
#include <math.h>


#include "AnemometerMon.h"
#include "Logger.h"

using namespace Arcetri;

//
//
MonAverage::MonAverage(unsigned int size): _size(size) {
}

//
//
double MonAverage::average() { 
    if (_buffer.size() > 0)
        return accumulate(_buffer.begin(), _buffer.end(), 0.0)/_buffer.size();
    else
        return 0.0;
}

//
//
void MonAverage::add_value(double val) {
    _buffer.push_back(val);
    while ((_buffer.size() > _size) && (_buffer.size() > 0))
        _buffer.pop_front();
}

//
//
void MonAverage::clear() {
    _buffer.clear();
}

//
//
AnemometerMon::AnemometerMon( int argc, char **argv) throw (AOException) : 
    AOApp( argc, argv), _dev(NULL) {
    
}

//
//
AnemometerMon::~AnemometerMon() {
    
}

//
//
void AnemometerMon::PostInit() {
    // read config
    try {
        _host = (string) ConfigDictionary()["Host"];
        _port = ConfigDictionary()["Port"];
        _rate = AnemometerRate((string) ConfigDictionary()["Rate"]);
        _dosetup = (int)ConfigDictionary()["DoSetup"];
        _mstimeout = 10000;
    }
    catch (Config_File_Exception &e) {
        _logger->log( Logger::LOG_LEV_FATAL, "Failed to read configuration: %s", e.what().c_str());
        printf("FATAL: Failed to read configuration: %s\n", e.what(Terse).c_str());
        throw;
    }

    // setup average variables
    // setup 1sec averages
    int i;
    for (i = 0; i < 4; i++) 
        _averages[0][i] = MonAverage(1*1000/_rate.getMilliSeconds());

    // setup 10s averages
    for (i = 0; i < 4; i++) 
        _averages[1][i] = MonAverage(10*1000/_rate.getMilliSeconds());

    // setup 60s averages
     for (i = 0; i < 4; i++) 
        _averages[2][i] = MonAverage(60*1000/_rate.getMilliSeconds());
   
    // setup variables
    try {
        _varSpeed = RTDBvar(ANEMOMETERMON_RTDBNAME+Side()+".SPEED", REAL_VARIABLE, 12);
        _varCheck = RTDBvar(ANEMOMETERMON_RTDBNAME+Side()+".CKSPEED", REAL_VARIABLE, 1);
    }
    catch (AOVarException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, "Failed to setup variables (%s)", e.what().c_str());
        printf("FATAL: Failed to setup variables (%s)\n", e.what(Terse).c_str());
		throw;
	}
    double nan=NaNutils::dNaN();
    _varCheck.Set(nan);

    // try to connect
    Connect();
  
    // Started
    _logger->log( Logger::LOG_LEV_INFO, "Started.");
}




//
//
void AnemometerMon::Run() {

    while(!TimeToDie()) {
        Periodic();
    }
    _logger->log(Logger::LOG_LEV_INFO, "Periodic loop stopped");
}


void AnemometerMon::Connect() {
    
    _isconnected = false;
    _logger->log(Logger::LOG_LEV_INFO, "Connecting to anemometer (%s:%d)", _host.c_str(), _port);
    try {
        _dev = new chardev(_host, _port);
        _an = new Anemometer(_dev,_rate, _dosetup);
    }
    catch (Anemometer::exception &e) {
      if (_dev != NULL) {
	delete _dev;
	_dev = NULL;
      }
        _logger->log(Logger::LOG_LEV_INFO, "Failed to connect to device (%s)", e.what());
        return;
    }
    catch (...) {
        _logger->log(Logger::LOG_LEV_INFO, "Failed to connect to device");
        return;
    }
    _logger->log( Logger::LOG_LEV_INFO, "Connected.");
    _isconnected = true;
}


//
//
void AnemometerMon::Periodic() {

    static int counter = 0;
    static time_t time_lasttry = 0;
    time_t time_now;

    time_now = time(NULL);

    if (_isconnected) {

        // counter reset every second
        counter = (counter + 1) % (1000/_rate.getMilliSeconds());

        // Read data
        AnemometerData ad;
	try {
	  try {
            ad = _an->read_frame();
	  }
	  catch (AOException &e) {
	    // try again
	    _logger->log(Logger::LOG_LEV_INFO, "Retrying reading frame (%s)", e.what(Terse).c_str());
            ad = _an->read_frame();
	  }
	}
	catch (...) {
	  _logger->log(Logger::LOG_LEV_INFO, "Failed to read frame");
	  _isconnected = false;
	  delete _an;
	  delete _dev;
	  
	  _an = NULL;
	  _dev = NULL;
	  for (int i=0; i<3; i++)
	    for (int j=0; j<4; j++) {
	      _averages[i][j].clear();
	    }
	  return;
	}
        
        // update average buffers
        for (int i=0; i<3 ; i++) {
            _averages[i][0].add_value(ad.getU());
            _averages[i][1].add_value(ad.getV());
            _averages[i][2].add_value(ad.getW());
            _averages[i][3].add_value(sqrt(ad.getU()*ad.getU()+ad.getV()*ad.getV()+ad.getW()*ad.getW()));
        }


        // check if more than one sec has passed and write variables
        if (counter == 0) {
            _logger->log(Logger::LOG_LEV_INFO, "Anemometer data %c (status %d): %.2f %.2f %.2f", 
                         ad.getID(), ad.getStatus(), ad.getU(), ad.getV(), ad.getW());

            vector <double> values(12);
            for (int i=0; i<3; i++)
                for (int j=0; j<4; j++) {
                    values[i*4+j] = _averages[i][j].average();
                }
            _varSpeed.Set(values);
            _varCheck.Set(values[7], 0, FORCE_SEND);
        }
        msleep(_rate.getMilliSeconds() - 5);
    }
    else {
        // try to connect
        if (time_now - time_lasttry >= ANEMOMETERMON_RETRY_TIME) {
            time_lasttry = time_now;
            Connect();
        }
        msleep(ANEMOMETERMON_BASE_PERIOD);
    }
}

// Main
//
int main(int argc, char *argv[]) 
{
    SetVersion(VERS_MAJOR,VERS_MINOR);

    try {
        AnemometerMon * app = new AnemometerMon( argc, argv);

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
