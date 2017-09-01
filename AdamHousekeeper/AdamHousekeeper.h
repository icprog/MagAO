
#ifndef ADAMHOUSEKEEPER_H_INCLUDED
#define ADAMHOUSEKEEPER_H_INCLUDED

#include <map>
#include <string>

//#include <boost/archive/binary_oarchive.hpp> //serialize

#include "DiagnApp.h"
#include "AdSecConstants.h"
#include "Funct.h"
#include "AdamHousekeeper_convert.h"
#include "DiagnApp_convert.h"
#include "DiagnRange.h"
#include "DiagnVar.h"
#include "DiagnSet.h"

#include "BcuLib/BcuCommon.h"

extern "C" {
#include "base/buflib.h"        // BufInfo
#include "errno.h"         // BufInfo
}


#define ADAM_TSS_POWER_FAULTN0_IDX  0
#define ADAM_TSS_POWER_FAULTN1_IDX  1
#define ADAM_2_IDX                  2  
#define ADAM_TSS_POWER_FAULTN_IDX   3
#define ADAM_TCS_SYS_FAULT_IDX      4
#define ADAM_TCS_POWER_FAULT_N0     5
#define ADAM_TCS_POWER_FAULT_N1     6
#define ADAM_TCS_POWER_FAULT_N2     7
#define ADAM_SYS_RESETN_IDX         8
#define ADAM_BOOTSELECTN_IDX        9
#define ADAM_FPGA_CLEARN_IDX       10
#define ADAM_DRIVER_ENABLE_IDX     11
#define ADAM_TSS_DISABLE_IDX       12
#define ADAM_13_IDX                13  
#define ADAM_WATCHDOG_EXP_IDX      14  
#define ADAM_MAINPOWER_IDX         15

#define ADAM_TSS_DISABLED           0
#define ADAM_TSS_READY              1
#define ADAM_TSS_ACTIVATED          2
#define ADAM_TSS_MALFUNCTION        3

using namespace Arcetri::AdSecConstants;
using namespace Arcetri::Diagnostic;
using namespace Arcetri::Bcu;


class AdamHousekeeper: public DiagnApp 
{
    //////////// construction ///////////
    public:
        AdamHousekeeper(int argc, char **argv) throw(AOException);
        ~AdamHousekeeper();


    //////////////  methods /////////////
    public:
        virtual void     ReadConfig();
        virtual void     SetupVars();
        virtual void     PostInit();
        virtual void     InstallHandlers();

        //void DumpRaw();
        void CreateDiagnVars();
        void InFastLoop();
        void InSlowLoop();
        void Periodic();

    protected:
        void CreateFD();

    private: 
        virtual void            GetRawData();
        virtual unsigned int    GetFrameCounter();
        void                    ReadAdam();
        void                    adam_dummy();

    ////////////// variables ////////////
    private:
        // CommandSender to communicate with BCUs
    	// Timeout used for bcu requests, got from config file.
        //@C
        // \verb+_n_tot_bcu+ is the total number of BCUs in the AdSec system (e.g. 1 switch + 6 crates)
        //@
        //
//@C
// When \verb+_dummy+ is true don't access crates and simulate data instead.  
//@
        bool            _dummy;
        AdamModbus*     _adam;
        int          _adamvec[16];
        int          _timestamp_tmp;
        RTDBvar      _tssStatus;
        RTDBvar      _tssEnabled;
        RTDBvar      _coilStatus;
        RTDBvar      _mainpowerStatus;

        
};



#endif // HOUSEKEEPER_H_INCLUDED

