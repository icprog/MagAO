

#ifndef __MagAOIClient_h__
#define __MagAOIClient_h__


//DEBUG and Testing macros


#include "AOApp.h"
#include "AOStates.h"


#include <errno.h>

extern "C" 
{
#include "aoslib/aoscodes.h"
#include "base/globals.h"
}

#include <stdint.h>

#include "../AOI_DD.h"

struct DD_RTDBVar
{
   RTDBvar RTVar;
   void * DDVar;
   bool log;
   bool update;
   bool created;
};

class MagAOIClient : public AOApp 
{
   public:
      MagAOIClient( int argc, char **argv) throw(AOException);
      MagAOIClient( std::string name, const std::string & conffile) throw (AOException);
   protected:
      void Create(void) throw (AOException);
      void updateState(bool force = false);

      ///Load the configuration details from the file
      int LoadConfig();

      ///Setup variables in RTDB (overridden virtual)
      void SetupVars();

      virtual void init_DD();
      
      // VIRTUAL - Run
      virtual void Run();
      
      virtual int DoFSM();

      std::string _rtdbSide;

   public:
      //************* Data Dictionary *************/
      ///The Magellan AOI Data Dictionary
      /** Here we replicate the LBT DD structure, but not much functionality, to make
        * adaptation of the AOS GUI as easy as possible.  This should work for other
        * AOI clients too.
        */
      AOI_DD aoi;
      //AOS_DD aos;
   protected:
      std::map<std::string, DD_RTDBVar> RTVars;
      typedef pair<std::string, DD_RTDBVar> DD_RT_mapT;
  
      
      RTDBvar AOARB_AO_READY;
      RTDBvar AOARB_CORRECTED_MODES;

      void add_DD_RTVar(std::string base_name, std::string var_name, void * ddptr, int type, int len, bool log, bool update);
      void add_DD_RTVar(std::string base_name, std::string var_name, int * ddptr, int len = 1, bool log=false, bool update=true);
      void add_DD_RTVar(std::string base_name, std::string var_name, double * ddptr, int len=1, bool log=false, bool update=true);
      void add_DD_RTVar(std::string base_name, std::string var_name, std::string * ddptr, int len =79, bool log=false, bool update=true);
      
      /// RTDB handler for a DD variable change.
      static int AOI_DD_var_changed(void *pt, Variable *msgb);

      int update_DD_var(Variable *msgb);
      int update_DD_var(std::string name, int Type, bool post = true);

      virtual void post_update_DD_var(DD_RTDBVar &var);
      
      //******* Arbitrator Interface *************/

      //Commands
      // AOI Command RTDB variables
      RTDBvar MagAOI_cmds_PresetFlat_command; 
      RTDBvar MagAOI_cmds_PresetFlat_arg_flatSpec; ///<Contains flatSpec string

      RTDBvar MagAOI_cmds_PresetVisAO_command;


      RTDBvar MagAOI_cmds_PresetAO_command;
      RTDBvar MagAOI_cmds_PresetAO_arg_AOmode; ///<Contains the string AOmode arg for the command
      RTDBvar MagAOI_cmds_PresetAO_arg_wfsSpec; ///<Contains the string wfsSpec arg for the command
      RTDBvar MagAOI_cmds_PresetAO_arg_rocoordx;
      RTDBvar MagAOI_cmds_PresetAO_arg_rocoordy;
      RTDBvar MagAOI_cmds_PresetAO_arg_mag;
      RTDBvar MagAOI_cmds_PresetAO_arg_cindex;
      RTDBvar MagAOI_cmds_PresetAO_arg_adcTracking;

      RTDBvar MagAOI_cmds_AcquireFromGuider_command;

      RTDBvar MagAOI_cmds_NudgeXY_command;
      RTDBvar MagAOI_cmds_NudgeXY_arg_x;
      RTDBvar MagAOI_cmds_NudgeXY_arg_y;

      RTDBvar MagAOI_cmds_NudgeAE_command;
      RTDBvar MagAOI_cmds_NudgeAE_arg_a;
      RTDBvar MagAOI_cmds_NudgeAE_arg_e;


      RTDBvar MagAOI_cmds_CheckRefAO_command;
      RTDBvar MagAOI_cmds_CheckRefAO_result; ///<Results (dx, dy, mag) are placed here

      RTDBvar MagAOI_cmds_AcquireRefAO_command;

      RTDBvar MagAOI_cmds_RefineAO_command;
      RTDBvar MagAOI_cmds_RefineAO_arg_method;


      RTDBvar MagAOI_cmds_ModifyAO_command;
      RTDBvar MagAOI_cmds_ModifyAO_arg_nModes;
      RTDBvar MagAOI_cmds_ModifyAO_arg_freq;
      RTDBvar MagAOI_cmds_ModifyAO_arg_nBins;
      RTDBvar MagAOI_cmds_ModifyAO_arg_ttMod;
      RTDBvar MagAOI_cmds_ModifyAO_arg_f1;
      RTDBvar MagAOI_cmds_ModifyAO_arg_f2;
      RTDBvar MagAOI_cmds_ModifyAO_arg_f3;
      
      
      RTDBvar MagAOI_cmds_StartAO_command;

      RTDBvar MagAOI_cmds_OffsetXY_command;
      RTDBvar MagAOI_cmds_OffsetXY_arg_x;
      RTDBvar MagAOI_cmds_OffsetXY_arg_y;

      RTDBvar MagAOI_cmds_NodRaDec_command;
      RTDBvar MagAOI_cmds_NodRaDec_stat;
      RTDBvar MagAOI_cmds_NodRaDec_arg_ra;
      RTDBvar MagAOI_cmds_NodRaDec_arg_dec;

      RTDBvar MagAOI_cmds_OffsetZ_command;
      RTDBvar MagAOI_cmds_OffsetZ_arg_z;

      RTDBvar MagAOI_cmds_Focus_command;
      RTDBvar MagAOI_cmds_Focus_arg_z;

      RTDBvar MagAOI_cmds_Stop_command;
      RTDBvar MagAOI_cmds_Stop_arg_msg;

      RTDBvar MagAOI_cmds_Pause_command;

      RTDBvar MagAOI_cmds_Resume_command;

      RTDBvar MagAOI_cmds_UserPanic_command;
      RTDBvar MagAOI_cmds_UserPanic_arg_reason;

      RTDBvar MagAOI_cmds_SetNewInstrument_command;
      RTDBvar MagAOI_cmds_SetNewInstrument_arg_instr;
      RTDBvar MagAOI_cmds_SetNewInstrument_arg_focalSt;
            
      RTDBvar MagAOI_cmds_WfsOn_command;
      RTDBvar MagAOI_cmds_WfsOn_arg_WfsID;

      RTDBvar MagAOI_cmds_WfsOff_command;
      RTDBvar MagAOI_cmds_WfsOff_arg_WfsID;

      RTDBvar MagAOI_cmds_AdsecOn_command;

      RTDBvar MagAOI_cmds_AdsecOff_command;

      RTDBvar MagAOI_cmds_AdsecSet_command;

      RTDBvar MagAOI_cmds_AdsecRest_command;

      RTDBvar AOARB_cmds_Stop_command;

public:
      /** @name AOI Commands
        * The Magellan implementation of LBT TCS-AOS commands.
        * These methods set the appropriate RTDBvar values (above) and issue the command to the AOS via the RTDB.
        */
      //@{
         
      ///The PresetFlat command implementation for seeing limited operation
      int PresetFlat(string flatSpec);

      ///The AOI Gui PresetVisAO command
      int PresetVisAO(); 

      //std::string PresetAO(std::string AOmode, Position refStar);                    // Called by IIF

      ///The AOI Gui PresetAO command
      /** \todo develop way to set socoord for off-axis work
        * \todo develop plan for focStation and instr, make config vars too
        */
      int PresetAO(string AOmode, string wfsSpec, double rocoordx, double rocoordy, double mag, double cindex, bool adcTracking); // Called by AOSGUI

      int AcquireFromGuider();

      int NudgeXY(double x, double y);
      int NudgeAE(double a, double e);

      ///The CheckRefAO command
      int CheckRefAO();

      //Does this move telescope? - yes, it can.
      int AcquireRefAO(bool rePoint);

      ///The RefineAO command
      int RefineAO(string method);

      ///The ModifyAO command
      int ModifyAO(int nModes, double freq, int nBins, double ttMod, string f1, string f2, string f3);

      ///The StartAO command.
      int StartAO();

      /// The OffsetXY command
      /** \ToDo Need to check loop status and adjust timeouts.   
        */
      int OffsetXY(double x, double y);

      int NodRaDec(double ra, double dec);

      /// The OffsetZ command
      int OffsetZ(double z);

      /// The Focus command
      int Focus(double z);



      //std::string CorrectModes(double modes[N_CORRECTMODES]);
      //std::string SetZernikes(int n_zern, double modes[N_ZERN]);

      ///The Stop command
      int Stop(string msg);

      ///Pause the loop
      int Pause();

      ///Resume the loop
      int Resume();

      ///Issue a panic shutdown
      int UserPanic(string reason);

      /// Set a new instrument name.
      int SetNewInstrument(string instr, string focalSt);
      int AdsecOn();
      int AdsecOff();
      int AdsecSet();
      int AdsecRest();
      int WfsOn(string);
      int WfsOff(string);


      int AOArbStop();

      //@}
      




      /** @name Utilities
        */
      //@{
      int updateObservers(std::string obsname, std::string obsinst);

      int parseLoopGains(double &tt, double &ho1, double &ho2);

      //@}       



};



#endif //__MagAOI_h__

