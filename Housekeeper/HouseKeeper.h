
#ifndef HOUSEKEEPER_H_INCLUDED
#define HOUSEKEEPER_H_INCLUDED

#include <map>
#include <string>

//#include <boost/archive/binary_oarchive.hpp> //serialize

#include "DiagnApp.h"
#include "AdSecConstants.h"
#include "Funct.h"
#include "DiagnApp_convert.h"
#include "HouseKeeper_actions.h"
#include "DiagnRange.h"
#include "DiagnVar.h"
#include "DiagnSet.h"
#include "HouseKeeper_convert.h"

#include "BcuLib/BcuCommon.h"

extern "C" {
#include "base/buflib.h"        // BufInfo
#include "errno.h"         // BufInfo
}

#include "HouseKeeperRawStruct.h"     // sdram_diagn_struct

#define HOUSEKEEPER_MIN_TIME_BTWDUMP 600                   // min time between dumps in secs
#define HOUSEKEEPER_MASTDIAG_STR     "masterdiagnostic"    // master diagnostics process

using namespace Arcetri::AdSecConstants;
using namespace Arcetri::Diagnostic;
using namespace Arcetri::Bcu;
using namespace Arcetri::HouseKeeperRaw;


typedef struct {
    //na_dsp_nios_fixed_area_struct           dsp_nios[BcuMirror::N_DSB]; //
    na_bcu_nios_fixed_area_struct           switc[BcuSwitch::N_CRATES]; // without h because switch is a keyword
    na_bcu_nios_fixed_area_struct           crate[BcuMirror::N_CRATES];
    //na_bcu_nios_fixed_area_struct           crate[BcuSwitch::N_CRATES + BcuMirror::N_CRATES];
} raw_mem;


class HouseKeeper: public DiagnApp 
{
    //////////// construction ///////////
    public:
        HouseKeeper(int argc, char **argv) throw(AOException);
        ~HouseKeeper();


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
        void                    ReadNios();
//        void                    ReadAdam(int *array);
        void                    GetRawUpdateStatus();

        double                  randi(double a, double b);
        void                    bcu_dummy(na_bcu_nios_fixed_area_struct  *raw, int crate_n);
        void                    dsp_nios_dummy(na_dsp_nios_fixed_area_struct  *raw, int dsp_n);
        uint16                  invConvertGain(double, const float*);

        // dump related
/*
        void    setDumpAlarmFlag(long id = 1);
        long    getDumpAlarmFlagAndClear();
*/

    ////////////// variables ////////////
    public:
        raw_mem                 raw; 

    private:
        // CommandSender to communicate with BCUs
        CommandSender*          _comSender;
 
    	// Timeout used for bcu requests, got from config file.
    	int                     _bcuRequestTimeout_ms;
        
        //@C
        // \verb+_n_tot_bcu+ is the total number of BCUs in the AdSec system (e.g. 1 switch + 6 crates)
        //@
        static const int        _n_tot_bcu = BcuSwitch::N_CRATES + BcuMirror::N_CRATES;

        // used to unpack data from sendMultiBcuCommand
        //na_bcu_nios_fixed_area_struct           bcu_temp[BcuMirror::N_CRATES];
        //na_bcu_nios_fixed_area_struct           swi_temp[BcuSwitch::N_CRATES];
        na_dsp_nios_fixed_area_struct           dsp_nios[BcuMirror::N_DSB];
        na_dsp_nios_fixed_area_struct           sgn_nios[BcuMirror::N_SGN];
        update_status_struct                    updatestatus;

        // used as frame counter 
        unsigned int _sample_counter;
        //
//@C
// When \verb+_dummy+ is true don't access crates and simulate data instead.  
//@
        bool            _dummy;
        AdamModbus*           _adam;
  //      int          _adamvec[16];

    MastDiagnInterface* _mastdiagninterface;

        
};



#endif // HOUSEKEEPER_H_INCLUDED

