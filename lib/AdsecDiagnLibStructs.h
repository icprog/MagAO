// @File: AdsecDiagnLibStructs.h
//
// Definitions and prototypes for the adsec diagnostic
//
// These functions are intended for consumers of Fast Diagnostic information.
//
// HISTORY
// 2009 Created by Alfio Puglisi (AP) <puglisi@arcetri.astro.it>  deriving from FastDiagnostic code
// 
//@

#ifndef ADSECDIAGNLIBSTRUCTS_H_INCLUDE

#define ADSECDIAGNLIBSTRUCTS_H_INCLUDE

#include "AdSecConstants.h"
#include "aotypes.h"

using namespace Arcetri::AdSecConstants;

// ------------------ Raw structures ---------------------------
// These structures are used to retrieve data from the electronics
// and hold "raw" data in "unreadable" form 

// KEEP THIS STRUCTURE ALIGNED WITH DSP SOFTWARE.

typedef struct
{
    uint32  WFSFrameCounter;          // counter of frames
    uint32  ParamBlockSelector;       // optical loop settings
    uint32  MirrorFrameCounter;       //
    uint32  CommandHistoryPtr;        //
} crate_bcu_header;

////////////ParamBlockSelector///////////////////////////////////////////////////////////////
//
// Bit num.      Description
// 0             parameters block selection 0 = block #0, 1 = block #1 (relevant for SC & RTR)
// 1             the slopes are normalized through the sum of the 4 pixel of each sub-aperture (relevant for SC) bit 1,2,3  should be mutually exclusive
// 2             the slopes are normalized through the sum of all pixel of previous step (relevant for SC) bit 1,2,3  should be mutually exclusive
// 3             the slopes normalized through the SlopeConstant variable (relevant for SC) bit 1,2,3  should be mutually exclusive
// 4             enable (1) or disable (0) the slope linearization procedure (relevant for SC)
// 5             enable (1) or disable (0) the delta command calculation using the actual position of the mirror respect to the old command (relevant for RTR)
// 6             enable (1) or disable (0) the diagnostic data storage to the SDRAM (relevant for SC & RTR)
// 7             enable (1) or disable (0) the fast-link commands used to send the slope vector to the switchBCU (relevant for SC)
// 8             reserved (for internal enable use only)
// 9             reserved (for internal enable use only)
// 10            reserved (for internal enable use only)
// 11            enable (1) or disable (0) the DM accelerometers acquisition (relevant for RTR)
// 12(ro)        indicates the state of disturb on mirror commands (0=disabled, 1=enabled) 
// 13            command and current of off-loading mode block selection 0 = block #0, 1 = block #1
//
// from Modifiche_LBT_SW_dopo_il_28-09-06_v1.7.doc (email Mario 070330)
///////////////////////////////////////////////////////////////////////////////////////////////



typedef struct 
{
    float32 DistAverage[4];         // Linearized position average
    float32 CurrAverage[4];         // Current average
    uint32  DistAccumulator[8];     // Accumulated linearized position
    uint32  CurrAccumulator[8];     // Accumulated current
    uint32  DistAccumulator2[12];   // Accumulated squared linearized position [2 not used]
    uint32  CurrAccumulator2[12];   // Accumulated squared current [2 not used]
    float32 Modes[4];               // Modes calculated
    float32 NewDeltaCommand[4];     // Delta command to apply
    float32 FFCommand[4];           // Delta Feed-Forward Command
    float32 IntControlCurrent[4];   // Integrated Control Current
    float32 FFPureCurrent[4];       // Integrated Feed-Forward' Current
} single_dsp_record;


typedef struct
{
    crate_bcu_header    header;
    single_dsp_record   dsp[BcuMirror::N_DSP_CRATE];
    crate_bcu_header    footer;
} crate_bcu;


typedef struct
{
    uint32  WFSFrameCounter;          // counter of frames
    uint32  ParamBlockSelector;       // optical loop settings
    uint32  MirrorFrameCounter;       //
    uint32  Dummy;                    //
} switch_bcu_header;

#define LB_N_SLOPES 1600
//#define LB_N_SLOPES 1440

#define ADSEC_TIMESTAMP_PERIOD 16.87e-6

typedef struct
{
    switch_bcu_header    header;
    float32              slopes[LB_N_SLOPES];           //TODO questo e' un vero dito nel culo: cambia quando alfio rebinna.
	uint32               swb_SafeSkipFrameCnt;          //  il numero di frames saltati perché è fallito il check di sicurezza sui modi, comandi o correnti
	uint32               swb_PendingSkipFrameCnt;       //  skipped frames because of "command pending"
	uint32               swb_WFSGlobalTimeout;          // If switch BCU doesn't receive any WFS frame for a while, 
                                                        // swb_WFSGlobalTimeout is increased ( ++ for each timeout) 
    uint32               swb_NumFLTimeout;              // number of FastLink timeouts.
    uint32               swb_NumFLCrcErr;               // number of FastLinl Crc errors.
    uint32               not_used[3];
	float32              accelerometer_coeffs[3];       // last is not used
	uint32               TimeStamp;                     // 59.29 kHz (16.87us)
    switch_bcu_header    footer;
} switch_bcu;


typedef struct {
    uint32                       mask;
	switch_bcu                   switc[BcuSwitch::N_CRATES];
    crate_bcu                    crate[BcuMirror::N_CRATES];
} AdsecDiagnRaw;

typedef struct {
   switch_bcu_header header;
   uint32 safeSkipFrameCounter;
   uint32 pendingSkipFrameCounter;
   uint32 wfsGlobalTimeout;
   uint32 flTimeout;
   uint32 crcErrors;
   uint32 timestamp;
   float32 Modes[BcuMirror::N_DSP_CRATE * BcuMirror::N_CRATES *4];         // Modal command after integration & filtering
   float32 FFCommand[ BcuMirror::N_DSP_CRATE * BcuMirror::N_CRATES *4];    // Actuator command after m2c and feed forward (relative to bias)
   float32 DistAverage[ BcuMirror::N_DSP_CRATE * BcuMirror::N_CRATES *4];  // Real actuator position (absolute)
   switch_bcu_header footer;
} MirrorOutputDiagn;


/*
typedef struct
{
    uint32  FramesCounter;          // counter of frames
    uint32  ParamBlockSelector;     // optical loop settings
    uint32  Dummy[2];
    float32 DistAverage[4];         // Linearized position average
    float32 CurrAverage[4];         // Current average
    uint32  DistAccumulator[8];     // Accumulated linearized position
    uint32  CurrAccumulator[8];     // Accumulated current
    uint32  DistAccumulator2[12];   // Accumulated squared linearized position
    uint32  CurrAccumulator2[12];   // Accumulated squared current
    float32 Modes[4];               // Modes calculated
    float32 NewDeltaCommand[4];     // Delta command to apply
    float32 FFCommand[4];           // Delta Feed-Forward Command
    float32 IntControlCurrent[4];   // Integrated Control Current
    float32 FFPureCurrent[4];       // Integrated Feed-Forward' Current
    uint32  FramesCounterCheck;     // FramesCounter check
    uint32  ParamBlockSelectorCheck;// ParamBlockSelector check
    uint32  DummyCheck[2];
} sdram_diagn_struct;
#define GetDiagnosticBuffersEnabled(a)  ( (a)->ParamBlockSelector >> 6 & 0x1 )
*/


//--------------------------------------------------------------------

// This structure contains the complete "fast" diagnostic status
// at a certain time t=FramesCounter. 
// The arrays are n_ch long, with n_ch numbers of channels in the system 
// (672 for lbt)
#define fast_diagn_n_fields 13  // below there are 13 double arrays 
typedef struct 
{
    uint32        FramesCounter;
    // TODO add field of ParamBlockSelector here
    char          DiagnosticBuffersEnabled;
    unsigned int  n_ch;
    double        *DistAverage;
    double        *CurrAverage;
    double        *DistAccumulator;
    double        *CurrAccumulator;
    double        *DistAccumulator2;
    double        *CurrAccumulator2;
    double        *Modes;
    double        *NewDeltaCommand;
    double        *FFCommand;
    double        *IntControlCurrent;
    double        *FFPureCurrent;
    double        *DistRMS;
    double        *CurrRMS;
} fast_diagn_struct;


#endif //ADSECDIAGNLIBSTRUCTS_H_INCLUDE
