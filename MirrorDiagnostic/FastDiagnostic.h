
#ifndef FASTDIAGNOSTIC_H_INCLUDED
#define FASTDIAGNOSTIC_H_INCLUDED

#include <map>
#include <string>

#include <RTDBvar.h>

//#include <boost/archive/binary_oarchive.hpp> //serialize

#include "DiagnApp.h"
#include "AdSecConstants.h"
#include "Funct.h"
#include "FastDiagnostic_convert.h"
#include "DiagnApp_convert.h"
#include "DiagnRange.h"
#include "DiagnVar.h"
#include "DiagnSet.h"



extern "C" {
#include "base/buflib.h"        // BufInfo
#include "errno.h"         // BufInfo
}

//#include "FastDiagnosticRawStruct.h"     // sdram_diagn_struct
#include "AdsecDiagnLib.h"

using namespace Arcetri::AdSecConstants;


typedef std::vector<DiagnVar>  DiagnVarList;

class FastDiagnostic: public DiagnApp 
{
    //////////// construction ///////////
    public:
    FastDiagnostic(int argc, char **argv) throw (AOException);
    ~FastDiagnostic();

    //////////////  methods /////////////
  public:
    virtual void     ReadConfig();
    virtual void     SetupVars();
    virtual void     PostInit();
    virtual void     InstallHandlers();

    //void DumpRaw();
    void CreateDiagnVars();
    void InSlowLoop();
    void Periodic();
        
    //static Adam*     Adam() {return _adam;}
        
    int ovpHandler(MsgBuf* msgBuf, void* myself);


    void InFastLoop();
        
  protected:
    static int TimeoutReqChanged(void *, Variable *);
    static int AdsecOvspChanged(void *, Variable *);	// ADSEC oversampling period variable.
    static int RefPosChanged(void *, Variable *);	// ADSEC flat mirror positinos.
    static int RefForChanged(void *, Variable *);	// ADSEC flat mirror forces.
    static int DoOffloadModesReqChanged(void *, Variable *); // Offload modes to AOS
    static int HighOrderOffloadMatrixChanged(void*, Variable*); // Get rid of high order modes
    void CreateFD();

  private: 
    virtual void			StoreOfflineFrame(BYTE* frame, uint32 frameSize);
    virtual void            GetRawData();
    virtual double          GetTimeStamp();
    virtual unsigned int    GetFrameCounter();
        
    void emergencyReact();

    ////////////// variables ////////////
  public:
    //RTDBvar     _VarEnableAllCur; 
    //RTDBvar     _VarEnableAllReq; 
    RTDBvar             _VarTimeoutCur;
    RTDBvar             _VarTimeoutReq;
    RTDBvar		    _VarAdsecOvsp;
    RTDBvar             _offloadvars;
    DiagnSetPtr         _varsOffLoadModes;
    //RTDBvar             _offloadprio;
    char                *RawSharedBase; 
    char                *RawSharedName; 
    BufInfo		    *_info; 
    AdsecDiagnRaw       _raw; 
    string             _bufname;

        
  private:
    Config_File       _cfg;
    RTDBvar		  _VarRefPos;
    RTDBvar		  _VarRefFor;
    RTDBvar           _VarDoOffloadModesCur;
    RTDBvar           _VarDoOffloadModesReq;
    RTDBvar           _VarHighOrderOffloadMatrixName;
    RTDBvar           _VarOffloadTime;
    vector<double>*   _ref_pos; 
    vector<double>*   _ref_for; 
    double            _ref_for_mean;
    double*           _high_order_offload_matrix;  
    double*		     _currAveNullMean60;   
    double*           _offloadCommandVector;
    string            _highOrderOffloadMatrixName; 
    //Adam*             _adam;
    FunctActionPtr    _hEmergencyStop;
        
    //SharedVarsMap* _rtdbSharedVarsMap;

    MastDiagnInterface* _mastdiagninterface;

    int               _highOrderOffloadLoopCounter;

    string            _basedir;
};



#endif // FASTDIAGNOSTIC_H_INCLUDED

