
#ifndef OPTLOOPDIAGNOSTIC_H_INCLUDED
#define OPTLOOPDIAGNOSTIC_H_INCLUDED

#include <string>
#include <vector>
using namespace std;

#include "AOApp.h"
#include "Paths.h"
#include "arblib/base/ArbitratorInterface.h"

#include "AdSecConstants.h"

extern "C" {
#include "base/buflib.h"        // BufInfo
#include "errno.h"         // BufInfo
}

#include "AdsecDiagnLib.h"
#include "bcu_diag.h"

using namespace Arcetri::AdSecConstants;

#include "OptLoopDiagnosticStructs.h"

typedef struct {
   float value;
   float timestamp;
} valueTS;

typedef struct {
   float x;
   float y;
   float timestamp;
} guideValueTS;

typedef struct {
   int    counter;
   string filename;
} darkApplication;


class OptLoopDiagnostic: public AOApp
{
    //////////// construction ///////////
    public:
        OptLoopDiagnostic(int argc, char **argv) throw (AOException);
        ~OptLoopDiagnostic();


    //////////////  methods /////////////
    public:
        virtual void     SetupVars();
        virtual void     PostInit();
        virtual void     InstallHandlers();

        virtual void Run();

        void StartAntiDrift() { _antiDrift=true; }
        void StopAntiDrift() { _antiDrift=false; }
        
        
        static int      optsave_handler(MsgBuf *, void *, int hndlrQueueSize);
        static int      varEnableHdlr( void *ptr, Variable *var);



    public:
        void startSaving( optsave_cmd *cmd);
        void endSaving();
        string selectTrackingNum();
        void cleanData();
        int addDimmValue( float value, struct timeval mtime);
        int addWindSpeedValue( float value, struct timeval mtime);
        int addWindDirValue( float value, struct timeval mtime);
        int addGuideCamValue( float x, float y, struct timeval mtime);

        // Handler for variable notifications
        static int ccd39Changed(void* thisPtr, Variable* var);
        static int nSubapsChanged(void* thisPtr, Variable* var);
        static int dimmVarChanged(void* thisPtr, Variable* var);
        static int windSpeedVarChanged(void* thisPtr, Variable* var);
        static int windDirVarChanged(void* thisPtr, Variable* var);
        static int guideCamVarChanged(void* thisPtr, Variable* var);
        void reloadDark();

    protected:

        int sendCommandWithReply( ArbitratorInterface *intf, Command* cmd, Command **reply = NULL);

        OptLoopDiagnClass *_diagf;
        ArbitratorInterface* _wfsIntf;
        ArbitratorInterface* _adsecIntf;

        void Create() throw (AOException);

        float computeDrift( float *quadrants = NULL);
        float computeSlopeRms();
        void correctDrift( float drift);
        void correctDrift( float *quadrants);

       /*
        * Replace <side> pattern in variable names
        */
       string ReplaceSide( string s);


    private: 
        virtual void            GetRawData();
       
        void Dump(); 

    ////////////// variables ////////////
    public:
        RTDBvar            _ccd39DarkReq;
        RTDBvar            _ccd39DarkCur;
        RTDBvar            _ccd39BinCur;
        RTDBvar            _scPixelLutCur;
        RTDBvar            _nSubapsCur;
	RTDBvar            _fastLinkEnabled;
        RTDBvar            _VarEnableDriftReq;
        RTDBvar            _VarEnableDriftCur;
        RTDBvar            _VarDriftCur;
        RTDBvar            _VarSlopeRmsCur;
        RTDBvar            _VarCountsCur;
        RTDBvar            _dimmVar;
        RTDBvar            _windSpeedVar;
        RTDBvar            _windDirVar;
        RTDBvar            _guideCamXVar;
        RTDBvar            _guideCamYVar;
        RTDBvar            _varSaving;

        // Telemetry
        Logger *_driftLogger;
        Logger *_countsLogger;

        // Anti-drift 

        bool   _antiDrift;            // If true, anti-drift is active
        int    _lastBin;              // Current ccd39 binning
        float  _lastDrift;            // Drift correction
	string _lastDarkFile;         // Last dark set for drift correction
        string _lastLut;              // Current ccd39 pixel LUT
	float  _driftThreshold;	      // Threshold to apply drift (in counts)
	int    _driftMinCorrTime;     // Min correction interval (sec)
        // Dark variables

        int16    *_darkBuf;           // Last valid dark for ccd39
        int16    *_subtractedBuf;     // Last valid frame - dark
        int16    *_darkFixedBuf;      // Last valid dark plus drift correction
        int       _darkLen;           // Dark len in bytes
        string    _darkFilename;      // Filename of the last valid dark for ccd39
        int      _totCounts;         // Total ccd counts (pixels - dark + drift correction)

        // Shared memory access

        BufInfo           *_info; 
        OptLoopDiagFrame   _raw; 
        string             _bufname;

        // Messaging with WFS arb
        
        string             _client;
        optsave_reply      _reply;

        // Saving of optical loop data

        bool       _saving;           // If true, optical loop data is in progress
        int        _nFrames;          // Total no. of frames to save
        int        _framesCounter;    // Frames saved so far
        string     _tracknum;         // Acquisition tracking number
        string     _path;             // Complete path of acquisition
        int        _pixelLen;         // No. of pixels in a ccd frame
        time_t     _prevTime;         // Variable to compute saving speed
        int        _prevFrame;        // Variable to compute saving speed

        // Binary buffers to save optical loop data

        uint16     *_pixelBuf;
        float32    *_slopesBuf;
        float32    *_modesBuf;
        float32    *_ffcommandsBuf;
        uint32     *_framesCounterBuf;
        uint32     *_mirrorCounterBuf;
        float32    *_distaveragesBuf;
        uint32     *_safeSkipFrameCounterBuf;
        uint32     *_pendingSkipFrameCounterBuf;
        uint32     *_wfsGlobalTimeoutBuf;
        uint32     *_flTimeoutBuf;
        uint32     *_crcErrorsBuf;
        uint32     *_timestampBuf;
        uint32     *_loopClosedBuf;
        float32    *_antiDriftBuf;
        vector<valueTS> _dimmVector;
        vector<valueTS> _windSpeedVector;
        vector<valueTS> _windDirVector;
        vector<guideValueTS> _guideCamVector;
        vector<darkApplication> _darkApplications;

};



#endif // OPTLOOPDIAGNOSTIC_H_INCLUDED

