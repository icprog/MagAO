/* AOS.hpp
 *
 * AOS subsystem class header file for the AOS subsystem.
 *
 */

#ifndef AOS_H
#define AOS_H


#include <tcs/core/Signal.hpp>
#include <tcs/core/PMutex.hpp>

#include "AOSconst.hpp"
#include "aos/interface/AOSInterface.hpp"
#include "AOSAoApp.hpp"

#include "aos/aosupervisor/arblib/aoArb/AOArbConst.h"

#define CHK_OK          0     // Reason code for command which can proceed
#define CHK_SIMULATION  1     // Reason code for simulation mode
#define CHK_BUSY        2     // Reason code for AOS busy executing prev. command
#define CHK_DISABLED    3     // Reason code for commands disabled


#define TO_METERS      1.e-9   // Converting nanometers to meters
#define TO_NANOMETERS  1.e+9   // Converting meters to nanometers 

namespace lbto {

void  signalHandler(int);

class AOSSubsystem : public Thread, public Signal, public AOSInterface<string>
{
    // construction/destruction
    public:
        AOSSubsystem(string msgdIP, string msgdName, int dbgLevel, const string logdir, bool simulation);
        ~AOSSubsystem();
	
    //methods
    public:
        void      execute(void *arg);
        void      signalHandler(int signum);
        string    shutDown(string why);


    // Commands
        string   PresetFlat(string desc);
        string   PresetAO(string AOmode, Position refStar);                    // Called by IIF
        string   PresetAOg(string AOmode, string wfsSpec, 
                 double rocoordx, double rocoordy, double mag, double cindex); // Called by AOSGUI
        string   CheckRefAO();
        string   AcquireRefAO(bool rePoint);
        string   RefineAO(string method);
        string   ModifyAO(int nModes, double freq, int nBins, double ttMod, string f1, string f2);
        string   StartAO();
        string   OffsetXY();
        string   OffsetXYg(double x, double y);
        string   OffsetZ(double z);
        string   CorrectModes(double modes[N_CORRECTMODES]);
        string   SetZernikes(int n_zern, double modes[N_ZERN]);
        string   Stop(string);
        string   Pause();
        string   Resume();
        string   UserPanic(string reason);
        string   SetNewInstrument(string instr, string focalSt);
        string   AdsecOn();
        string   AdsecOff();
        string   AdsecSet();
        string   AdsecRest();
        string   WfsOn(string);
        string   WfsOff(string);

    private:
        bool cmdCheck(string cmdName,                        // Preliminary checks prior of any command
                     int msgSeverity=MASTER_LOG_LEV_WARNING, // Severity of message if msg cannot be sent
                     bool sendAlways=false);                 // Send message in any case
        void cmdUnlock();               // Free lock for the command
        string cmdStatNotify();         // Notify command status return
        string formatAOparams();        // Return a formatted string with AO param vaules

        bool ConfigureOffload();        // Load gains and thresholds for offload modes
        bool ConfigureZernikes();       // Load gains for SetZernikes command
        bool _setWFS(string);           // Set WFS related status variables
        void _setInstr(string,string);  // Set internal status related to instrument

    // properties
    public:
        AOSAoApp       *aoapp;

    //variables
    private: 
//      Logger*   _logger;
        string    _quitreason;   // Reason to quit
        int       AOSSleepTime;
        bool      _simulation;   // When true, no connection to MsgD is attempted
        bool      timeToQuit;    // Termination flag 
        string    msgdIP;        // IP number of MSGD
        string    msgdN;         // Client name into MSGD
        int       _dbgLevel;
        string    _logdir;       // Directory for own logging
        string    aosPref;       // Name prefix for events

	Position  _refstar;      // Remember reference star
	double    _refstar_x;    // Remember reference star X position
	double    _refstar_y;    // Remember reference star Y position
        string    _instrument;   // Saved instrument request from SetNewInstrument()
        string    _focStation;   // Saved focal station request from SetNewInstrument()
        PMutex    _mutex;        // Command serialization mutex
        string    _reason;       // Error reason message
        int       _reason_code;  // Error reason code
        int       _cmd_stat;     // Command status for cmdCheck()
        bool      _cmdbusy;
        string    _cmdname;
        int       _cmdnum;

        string    _authorizedInstr;   // Authorized instrument
        string    _authorizedFocStn;  // Authorized focal station
        string    _wfsSpec;           // Resulting WFS specification
        string    _flatSpec;          // Resulting Flat specification

};

} // namespace lbto

#endif // AOS_SUBSYSTEM_H
