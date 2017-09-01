
#ifndef OPTLOOPDIAGNOSTIC_H_INCLUDED
#define OPTLOOPDIAGNOSTIC_H_INCLUDED

#include <string>
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


class OptLoopDiagnostic: public AOApp
{
    //////////// construction ///////////
    public:
        OptLoopDiagnostic(const std::string& conffile //Configuration file relative path (see lib/stdconfig.h)
                      ) throw (AOException);

        OptLoopDiagnostic(int argc, char **argv) throw (AOException);
        ~OptLoopDiagnostic();


    //////////////  methods /////////////
    public:
        virtual void     ReadConfig();
        virtual void     SetupVars();
        virtual void     PostInit();
        virtual void     InstallHandlers();

        virtual void Run();

        void InSlowLoop();
        void Periodic();

    public:
        // Handler for variable notifications
        static int ccd39Changed(void* thisPtr, Variable* var);
        void reloadDark();

        float computeCounts();

        void setGainZero();
        void setInitialGain();

    protected:

        int sendCommandWithReply( ArbitratorInterface *intf, Command* cmd, Command **reply = NULL);


        OptLoopDiagnClass *_diagf;
        ArbitratorInterface* _adsecIntf;


        void Create() throw (AOException);

    private: 
        virtual void            GetRawData();
       
         void Dump(); 

    ////////////// variables ////////////
    public:
        RTDBvar            _ccd39DarkCur;
        RTDBvar            _ccd39BinCur;

        int16       *_darkBuf;
        int16       *_subtractedBuf;
        int16       *_driftFixedBuf;
        int                _darkLen;
        string             _darkFilename;
        float                _totCounts;
        string       _initialGainFile;

        BufInfo				*_info; 
        OptLoopDiagFrame   _raw; 
        string             _bufname;
        string             _client;
        optsave_reply      _reply;

        int        _pixelLen;
        time_t     _prevTime;
        int        _prevFrame;

        int    _lastBin;

    private:
        Config_File     _cfg;
        
};



#endif // OPTLOOPDIAGNOSTIC_H_INCLUDED

