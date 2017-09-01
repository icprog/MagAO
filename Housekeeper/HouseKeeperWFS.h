
#ifndef HOUSEKEEPERWFS_H_INCLUDED
#define HOUSEKEEPERWFS_H_INCLUDED

#include <map>
#include <string>

//#include <boost/archive/binary_oarchive.hpp> //serialize

#include "DiagnApp.h"
#include "WfsConstants.h"
#include "Funct.h"
#include "HouseKeeper_convert.h"
#include "DiagnApp_convert.h"
#include "HouseKeeper_actions.h"
#include "DiagnRange.h"
#include "DiagnVar.h"
#include "DiagnSet.h"



extern "C" {
#include "base/buflib.h"        // BufInfo
#include "errno.h"         // BufInfo
}

#include "HouseKeeperRawStruct.h"     // sdram_diagn_struct

using namespace Arcetri::WfsConstants;
using namespace Arcetri::Diagnostic;
using namespace Arcetri::HouseKeeperRaw;

typedef struct {
    na_bcu_nios_fixed_area_struct  bcu39[Bcu39::N_CRATES]; // without h because switch is a keyword
    na_bcu_nios_fixed_area_struct  bcu47[Bcu47::N_CRATES];
} raw_mem;


class HouseKeeperWFS: public DiagnApp 
{
    //////////// construction ///////////
    public:
        HouseKeeperWFS(int argc, char **argv) throw(AOException);
        ~HouseKeeperWFS();


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

        double                  randi(double a, double b);
        void                    bcu_dummy(na_bcu_nios_fixed_area_struct  *raw, int crate_n);
        void                    dsp_nios_dummy(na_dsp_nios_fixed_area_struct  *raw, int dsp_n);
        uint16                  invConvertGain(double, const float*);


    ////////////// variables ////////////
    public:
        raw_mem                 raw; 

    private:
    
    	// Timeout used for bcu requests, got from config file.
    	int _bcuRequestTimeout_ms;
        
        //@C
        // \verb+_n_tot_bcu+ is the total number of BCUs in the AdSec system (e.g. 1 switch + 6 crates)
        //@
        static const int        _n_tot_bcu = Bcu39::N_CRATES + Bcu47::N_CRATES;

        // used to unpack data from sendMultiBcuCommand
        na_bcu_nios_fixed_area_struct           bcu39[Bcu39::N_CRATES];
        na_bcu_nios_fixed_area_struct           bcu47[Bcu47::N_CRATES];
        na_dsp_nios_fixed_area_struct           wgn_nios[Bcu47::N_WGN];

        //@C
        // Vars to replicate data in the RTDB
        //@

        RTDBvar varBcu39stratix_name;
        RTDBvar varBcu39stratix_value;
        RTDBvar varBcu47stratix_name;
        RTDBvar varBcu47stratix_value;
        RTDBvar varBcu39power_name;
        RTDBvar varBcu39power_value;
        RTDBvar varBcu47power_name;
        RTDBvar varBcu47power_value;
        RTDBvar varHVtemperature_name;
        RTDBvar varHVtemperature_value;

        //
//@C
// When \verb+_dummy+ is true don't access crates and simulate data instead.  
//@
        bool            _dummy;
        
};



#endif // HOUSEKEEPERWFS_H_INCLUDED

