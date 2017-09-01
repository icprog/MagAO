// @File: fastdiagn.h
//
// Definitions and prototypes for the module fastdiagn
//
// HISTORY
// 21 Jul 2005 Created by Lorenzo Busoni (LB) <lbusoni@arcetri.astro.it>
// 
//@

#ifndef FASTDIAGN_INCLUDE

#define FASTDIAGN_INCLUDE


#include "commlib.h"
#include "bcucommand.h" 


//TODO leggere il numero identificativo della BCU_board (=255)  e SIGGEN_board (=252) dal file ???
#define  BCU_board_n       255
#define  SIGGEN_board_n    252    


// ------------------ Raw structures ---------------------------
// These structures are used to retrieve data from the electronics
// and hold "raw" data in "unreadable" form 

// Here the number of channel per dsp chip is supposed to be 4:
// use _n_ch_per_dsp instead than n_act_per_dsp.
#define _n_ch_per_dsp 4
// KEEP THIS STRUCTURE ALIGNED WITH DSP SOFTWARE.

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


#endif //FASTDIAGN_INCLUDE
