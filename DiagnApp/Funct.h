#ifndef FUNCT_H_INCLUDED
#define FUNCT_H_INCLUDED

#include <string>
#include <iostream> //cout
#include <iomanip>  //setw
#include <boost/serialization/shared_ptr.hpp>
#include <unistd.h>
#include <string.h>
#include <errno.h>


//#ifndef DEBUG
//#define DEBUG false
//#endif

#include "AdamLib.h"
#include "arblib/base/ArbitratorInterface.h"

using namespace Arcetri;
using namespace Arcetri::Arbitrator;

class DiagnApp;
class Funct;
class FunctAction;

typedef boost::shared_ptr<Funct> FunctPtr;
typedef boost::shared_ptr<FunctAction> FunctActionPtr;


//@
// Class:Funct
//
// Used for define the conversion procedure called by a variable during the update process
//@
class Funct {
  public:
    Funct() {}
    Funct(const Funct&) {}
    virtual ~Funct() {}

    virtual void copy()                             { } 
    virtual void BeginLoop() {}

    virtual void doit(double *)                     { }
    virtual void doit(const std::string &, double)  { } 
};

//@
// Class:FunctAction
//
// Used for define the procedure called by a variable in case of warnings or alarms
//@
class FunctAction
{
  public:
    FunctAction();
    virtual ~FunctAction() {}

    void BeginLoop();
    void react(const std::string &y, double f);

  protected:
    virtual void doit(const std::string &, double)  { } 
    bool _called_in_this_loop;
};
//BOOST_CLASS_EXPORT(FunctAction)



//@
// Class:FunctWarning
//
//@
class FunctWarning : public FunctAction
{
  public:
    virtual ~FunctWarning() {}

  protected:
    virtual void doit(const std::string &y, double f);
};
//BOOST_CLASS_EXPORT(FunctWarning)


///////////////////////////////////////////////////////////////////////////////////////////////////


//@
// Class:FunctAlarm
//
//@
class FunctAlarm: public FunctAction
{
  public:
    virtual ~FunctAlarm() {}

  protected:
    virtual void doit(const std::string &y, double f);
};
//BOOST_CLASS_EXPORT(FunctAlarm)



//@
// Class:FunctEmergencyStop
//
//@
class FunctEmergencyStop: public FunctAction
{
  public:

    FunctEmergencyStop(std::string /* siren_file */, bool shutdown=false);
    virtual ~FunctEmergencyStop();

  protected:
    virtual void doit(double *f);
    virtual void doit(const std::string &y, double f);

  private:

    bool isSirenOn; //TODO QUESTO FA CACARE; CI VUOLE LIBRERIA CON _isSirenOn globale.
    bool mustShutdown;
    std::string _siren_complete_filename;
    AdamModbus* adam;
    AlertNotifier* adSecArbNotifier;
    Logger* logger;

    void spawn_siren();
    static void* PlaySiren(void* argp);

};



///////////////////////////////////////////////////////////////////////////////////////////////////

// Same Alarm and Emergency stop but sets dumping flag

//@
// Class:FunctAlarmAndDump
//
//@
class FunctAlarmAndDump: public FunctAlarm
{
  public:
    FunctAlarmAndDump(DiagnApp * diagapp) : FunctAlarm(), _diagapp(diagapp) { }
    FunctAlarmAndDump(const FunctAlarmAndDump& orig): FunctAlarm(), _diagapp(orig._diagapp) { }
    virtual ~FunctAlarmAndDump() {}

  protected:
    virtual void doit(const std::string &y, double f);
    DiagnApp * _diagapp;
};

//@
// Class:FunctEmergencyStopAndDump
//
//@
class FunctEmergencyStopAndDump: public FunctEmergencyStop
{
  public:
    FunctEmergencyStopAndDump(DiagnApp * diagapp, std::string siren, bool shutdown=false) : FunctEmergencyStop(siren, shutdown),
                                                                                            _diagapp(diagapp) { }
    virtual ~FunctEmergencyStopAndDump() {}

  protected:
    virtual void doit(const std::string &y, double f);
    DiagnApp * _diagapp;
};


#endif //FUNCT_H_INCLUDED
