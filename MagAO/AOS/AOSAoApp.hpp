
#ifndef AOSAOAPP_H
#define AOSAOAPP_H

#include <string>

#include <ctime>

using namespace std;

#include <tcs/core/Thread.hpp>
#include <tcs/setvalue/Gtype.hpp>
#include <tcs/core/Side.hpp>

//AO libraries
extern "C" {
#include "aos/aosupervisor/base/globals.h"
#include "aos/aosupervisor/base/thrdlib.h"
#include "aos/aosupervisor/aoslib/aoslib.h"
}

#include "aos/aosupervisor/arblib/base/ArbitratorInterface.h"

#include "AOSconst.hpp"
#include "AOApp.h"
#include "VarUtils.hpp"
#include "ConnStatus.hpp"

#define MSGD_REQUIRE 1002      // Required MsgD Version

namespace arb = Arcetri::Arbitrator;

namespace lbto {

class SecondaryMirrorClient;

class AOSAoApp : public AOApp
{
    // construction
    public:
        AOSAoApp(const string msgdIP, const string msgdN, int dbglevel, const string logdir );
        virtual ~AOSAoApp();

    // methods
    protected:

    public:
        void      execute(void *arg);
        void      InstallHandlers();
        void      defineTCSVars();
        void      defineRTVars();
        int       sendAoCommand(int spec);
        void      tvUpdateAcquisition(char *, int);
        int       makeFlatList(string list);

        int       whyQuit(void);     // Get reason for quitting
        int       reqService(void);
        int       setDebug(int);
        bool      msgd_up(void);
        bool      synchronize(void);
        int       status(void);
        string    chstatus(void);
        int       wakeup(void);
        void      Run();
        void      Quit(int);
        string    fmtMsg(string src, string prefix, MsgBuf *msg);


    private:
        void        PostInit();
        static int  arbalert_hndl(MsgBuf* msg, void* app, int);    // Handler for Arbitrator alerts
        static int  varnotify_hndl(MsgBuf* msg, void* app, int);   // Handler for RTDB variable change notification
        static int  default_hndl(MsgBuf* msg, void* app, int);     // Default handler

        static int  hexapod_hndl(MsgBuf* msg, void* app, int);     // Handler for engineering mode commands
        static int  offload_hndl(MsgBuf* msg, void* app, int);     // Handler for offload modes
        static int  housekeep_hndl(MsgBuf* msg, void* app, int);   // Handler for housekeeping commands


        static void _tvUpdateCurrent(void *, int, int);       // Preprocessing routine for WFS TV images
        static void tvUpdateCurrent1(void *, int);            // Preprocessing routine for variable WFSARB.x.TV_IMAGE@M_FLAOWFS
        static void tvUpdateCurrent2(void *, int);            // Preprocessing routine for variable WFSARB.x.TV_IMAGE@M_LBTIWFS

        static void anem_speed(void *, int);                  // Preprocesssing routine for Anemometer speed
        static void tvRotate(TVImage *in, TVImage *out, int rot);

        static string  _cleanSt(string s);                   // Returns a clean version of string
        static void  _sendEV(string msg);                    // Preprocessing routines for messages
        static void  sendEV_AOArb(void *data, int datalen);  // from Supervisor
        static void  sendEV_AdsecArb(void *data, int datalen);
        static void  sendEV_Wfs1Arb(void *data, int datalen);
        static void  sendEV_Wfs2Arb(void *data, int datalen);

        static void  w1_counts(void *data, int datalen);    // Preprocessing routines for CCD counts
        static void  w2_counts(void *data, int datalen);


        void        checkWFS();                              // Routine to check for WFS MsgD
        void        addTCSVar(const string name, int num, int ddref, int freq);
        void        addTCSVar(const string name, int num,
                              int ddref, int freq, int expir, int tstamp);

        void        addTCSVar(const string name, int(* ddref)(), int freq);
        void        addTCSVar(const string name, double(* ddref)(), int freq);

        void        addRTVar(const string name, int rtype, int num, int ddref, void(* pproc)(void *, int)=NULL, string vclass="",bool logit=false);

        int         updateTCSVariables();                   // Refresh values of DD variables mirrored into RTDB
        
        void        setStatus(int stat, bool force=false);  // Updates connection status

        int         log_item(char*);                 // Message logging
        int         tbd(string msg, MsgBuf *);       // Stub for commands to be implemented
        int         SendAOCommand(int code, int subc, int lng, void * body);
        void        offload(float *, bool ignore);   // Do Mode offload

        string      tstamp();                        // Get timestamp in seconds since start

    // variables

    private:
    //  Logger*        _logger;         // Log handle
        TCSVarTable    outVars;         // Exported variables table
        RTVarTable     inVars;          // Imported variables table
        int            DDPollingPeriod; // Polling period for DD variables
        int            AoSupTimeout;    // Timeout for AoSup messages
        bool           _msgd_isup;      // Status variable: MsgD is on and connected
        bool           _aoarb_running;  // Status variable: AO arbitrator is up and running
        bool           _noVariables;    // Set to true when variables in RTDB have been created
        int           _status;          // Status variable
        string        _rtdbSide;        // "L" or "R"
        string        _servstatName;    // Name of service status variable in RTDB
        string        _aoArb;           // MsgD name of AO arbitrator
        string        _wfsSpec;         // Current WFS Specification
        string        _tvImgVarName;    // Name of RTDB variable holding WFS TV img data
        string        _flaoWfsMsgd;     // Name of FLAO WFS Message Daemon
        string        _lbtiWfsMsgd;     // Name of LBTI WFS Message Daemon
        int           _reasonToDie;     // Terminate flag
        int           _nUpdate;         // Number of variable updates
        int           _ofld_ignored;    // Counter of ignored offload requests
        int           logSeqN;          // Sequence number for logs to MsgD
        int           _pendOffload;     // Number of pending offloads
        int           _MsgDretStat;     // Return status from MsgD;
        PMutex        _syncMutex;       // Msgd Syncronization mutex

        float         lastTip;          // Remember last PSF Shell Position
        float         lastTilt;         // Remember last PSF Shell Position
        float         lastFoc;          // Remember last PSF Shell Position

        arb::Serializator* _serializator;

        bool          doCollect;       // Telemetry collection flag
    public:
        arb::ArbitratorInterface *arbIntf; // Pointer to Arbitrator Interface
        SecondaryMirrorClient *g_smClient; // Pointer to secondary mirror object
        lbto::Side   tSide;                // Telescope side (a la Paul)

};

}  // namespace lbto
#endif // AOSAOAPP_H
