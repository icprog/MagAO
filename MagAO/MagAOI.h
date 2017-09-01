/**
 * \todo need to validate all TCS responses (e.g. 0 or 1 if it is a bool, string length, etc.)
 */

#ifndef __MagAOI_h__
#define __MagAOI_h__


//DEBUG and Testing macros
//#define _debug
#define OFFLOAD_TESTING

//Define this to turn on debugging messages for AOI commands
#define AOI_DEBUG
//Define this to prevent actually issuing arbitrator commands
//#define AOI_NOARB  


#define TELTOL  1e-6

#include "AOApp.h"
#include "AOStates.h"

#include <errno.h>
#include <math.h>
#include <fstream>

extern "C" 
{
#include "hwlib/netseriallib.h"
#include "aoslib/aoscodes.h"
#include "aoslib/aoslib.h"
#include "base/globals.h"
}

#include "arblib/base/ArbitratorInterface.h"

#include "arblib/aoArb/AOArbConst.h"

#include "AOI_DD.h"

#include "offloadCircBuff.h"



namespace arb = Arcetri::Arbitrator;

//Magellan TCS commands
//Those marked as provisional need to be confirmed at a later date.

#define TELRA  "telra"
#define TELDC  "teldc"
#define EPOCH  "epoch"
#define TELAM  "telam"
#define TELAZ  "telaz"
#define TELEL  "telel"
#define TELPA  "telpa"
#define TELDM  "teldm"
#define DMSTAT "dmstat"
#define  DATETIME "datetime"
#define TELPOS "telpos"
#define TELDATA "teldata"


#define VEDATA "vedata"
#define TELROI  "telroi"
#define ROTANGLE "rotangle"
#define ROTATORE "rotatore"

#define TELGUIDE  "telguide"
#define ROTMODE   "rotmode"

#define CATOBJ   "catobj"
#define CATRA    "catra"
#define CATDEC   "catdc"
#define CATDATA  "catdata"

#define TELENV   "telenv"

//Set the number of modes to consider for offloading.
#ifdef OFL_MODES
#undef OFL_MODES
#endif

#define OFL_MODES  7


class MagAOI : public AOApp 
{
   public:
      MagAOI( int argc, char **argv) throw(AOException);
      ~MagAOI();


   /** @name AOAPP Business
     */
   //@{


   protected:
      void Create(void) throw (AOException);
      void updateState(bool force = false);

   public:
      ///Installs the thrdlib handers for Arbitrator alerts and offloads (etc)
      virtual void InstallHandlers();

   protected:
      
      pthread_mutex_t aoiMutex;
      
      ///Load the configuration details from the file
      int LoadConfig();

      ///Setup variables in RTDB (overridden virtual)
      void SetupVars();

      void  PostInit();

	   // VIRTUAL - Run
      void Run();
      
      int DoFSM();

   //@}
     
      /** @name Arbitrator Interface State*************
        */
      //@{

      /** \todo add tmout configuration system*/
      std::string focalStation;
      std::string instrumentName;
      string _magWfsMsgd;  ///< Name of MAG WFS Message Daemon
      int  wfs1_enabled;  ///< Status of the WFS.
      double OffsetSpeedFactorCL;
      double OffsetSpeedFactorOL;

      int AOOffsetXYTmo;
      int AOOffsetZTmo;
      int AONodRaDecTmo;
      
      std::string _aoArb;           ///< MsgD name of AO arbitrator
      arb::ArbitratorInterface *arbIntf; ///< Pointer to Arbitrator Interface

      ///upload the timeouts to the RTDB for DD use.
      void init_tmouts();

      ///clear the command stat and errMsg variables.
      void clear_commands(bool force = false); 

      RTDBvar AOARB_loopon;
      int loopon;

      /// Updates the loopon variable in the AOI_DD
      void update_loopon(); 

      RTDBvar MagAOI_cmds_updateargs; ///< Signals the AOI GUI to update the commands
      RTDBvar MagAOI_cmds_clear; ///< Signals the MAGAOI to clear all commands

      //@}

      /** @name Aribtrator Interface RTDB Variables
        * These are how commands, arguments, and return values are exchanged with AOI clients.
        */
      //@{

      int PresetFlat_tmout;
      RTDBvar MagAOI_cmds_PresetFlat_command;
      RTDBvar MagAOI_cmds_PresetFlat_command_tmout;
      RTDBvar MagAOI_cmds_PresetFlat_arg_flatSpec; ///<Contains flatSpec string
      RTDBvar MagAOI_cmds_PresetFlat_stat;
      RTDBvar MagAOI_cmds_PresetFlat_errMsg;

      int AcqGuider_tmout;
      double guider_acq_wide_full_az ;
      double guider_acq_wide_full_el;
      double guider_acq_wide_strip_az ;
      double guider_acq_wide_strip_el;
      double guider_acq_wide_stamp_az ;
      double guider_acq_wide_stamp_el;
      double guider_acq_wide_substamp_az ;
      double guider_acq_wide_substamp_el;
      double guider_acq_narrow_full_az ;
      double guider_acq_narrow_full_el;
      double guider_acq_narrow_strip_az ;
      double guider_acq_narrow_strip_el;
      double guider_acq_narrow_stamp_az ;
      double guider_acq_narrow_stamp_el;
      double guider_acq_narrow_substamp_az ;
      double guider_acq_narrow_substamp_el;
      
      RTDBvar MagAOI_cmds_AcquireFromGuider_command;
      RTDBvar MagAOI_cmds_AcquireFromGuider_tmout;
      RTDBvar MagAOI_cmds_AcquireFromGuider_stat;
      RTDBvar MagAOI_cmds_AcquireFromGuider_errMsg;

      int NudgeXY_tmout;
      RTDBvar MagAOI_cmds_NudgeXY_command;
      RTDBvar MagAOI_cmds_NudgeXY_tmout;
      RTDBvar MagAOI_cmds_NudgeXY_arg_x;
      RTDBvar MagAOI_cmds_NudgeXY_arg_y;
      RTDBvar MagAOI_cmds_NudgeXY_stat;
      RTDBvar MagAOI_cmds_NudgeXY_errMsg;

      int NudgeAE_tmout;
      RTDBvar MagAOI_cmds_NudgeAE_command;
      RTDBvar MagAOI_cmds_NudgeAE_tmout;
      RTDBvar MagAOI_cmds_NudgeAE_arg_a;
      RTDBvar MagAOI_cmds_NudgeAE_arg_e;
      RTDBvar MagAOI_cmds_NudgeAE_stat;
      RTDBvar MagAOI_cmds_NudgeAE_errMsg;


      int PresetVisAO_tmout;
      RTDBvar MagAOI_cmds_PresetVisAO_command;
      RTDBvar MagAOI_cmds_PresetVisAO_command_tmout;
      RTDBvar MagAOI_cmds_PresetVisAO_stat;
      RTDBvar MagAOI_cmds_PresetVisAO_errMsg;

      int PresetAO_tmout;
      RTDBvar MagAOI_cmds_PresetAO_command;
      RTDBvar MagAOI_cmds_PresetAO_command_tmout;
      RTDBvar MagAOI_cmds_PresetAO_arg_AOmode; ///<Contains the string AOmode arg for the command
      RTDBvar MagAOI_cmds_PresetAO_arg_wfsSpec; ///<Contains the string wfsSpec arg for the command
      RTDBvar MagAOI_cmds_PresetAO_arg_rocoordx;
      RTDBvar MagAOI_cmds_PresetAO_arg_rocoordy;
      RTDBvar MagAOI_cmds_PresetAO_arg_mag;
      RTDBvar MagAOI_cmds_PresetAO_arg_cindex;
      RTDBvar MagAOI_cmds_PresetAO_arg_adcTracking;

      RTDBvar MagAOI_cmds_PresetAO_stat;
      RTDBvar MagAOI_cmds_PresetAO_errMsg;

      int AcquireRefAO_tmout;
      RTDBvar MagAOI_cmds_AcquireRefAO_command;
      RTDBvar MagAOI_cmds_AcquireRefAO_command_tmout;
      RTDBvar MagAOI_cmds_AcquireRefAO_stat;
      RTDBvar MagAOI_cmds_AcquireRefAO_errMsg;

      RTDBvar MagAOI_cmds_AcquireRefAO_result_deltaX;
      RTDBvar MagAOI_cmds_AcquireRefAO_result_deltaY;
      RTDBvar MagAOI_cmds_AcquireRefAO_result_slNull;
      RTDBvar MagAOI_cmds_AcquireRefAO_result_f1spec;
      RTDBvar MagAOI_cmds_AcquireRefAO_result_f2spec;
      RTDBvar MagAOI_cmds_AcquireRefAO_result_freq;
      RTDBvar MagAOI_cmds_AcquireRefAO_result_gain;
      RTDBvar MagAOI_cmds_AcquireRefAO_result_starMag;
      RTDBvar MagAOI_cmds_AcquireRefAO_result_nBins;
      RTDBvar MagAOI_cmds_AcquireRefAO_result_nModes;
      RTDBvar MagAOI_cmds_AcquireRefAO_result_r0;
      RTDBvar MagAOI_cmds_AcquireRefAO_result_snmode;
      RTDBvar MagAOI_cmds_AcquireRefAO_result_strehl;
      RTDBvar MagAOI_cmds_AcquireRefAO_result_ttMod;


      int CheckRefAO_tmout;
      RTDBvar MagAOI_cmds_CheckRefAO_command;
      RTDBvar MagAOI_cmds_CheckRefAO_command_tmout;
      RTDBvar MagAOI_cmds_CheckRefAO_result; ///<Results (dx, dy, mag) are placed here
      RTDBvar MagAOI_cmds_CheckRefAO_stat;
      RTDBvar MagAOI_cmds_CheckRefAO_errMsg;

      int RefineAO_tmout;
      RTDBvar MagAOI_cmds_RefineAO_command;
      RTDBvar MagAOI_cmds_RefineAO_command_tmout;
      RTDBvar MagAOI_cmds_RefineAO_arg_method;
      RTDBvar MagAOI_cmds_RefineAO_stat;
      RTDBvar MagAOI_cmds_RefineAO_errMsg;

      int ModifyAO_tmout;
      RTDBvar MagAOI_cmds_ModifyAO_command;
      RTDBvar MagAOI_cmds_ModifyAO_command_tmout;
      RTDBvar MagAOI_cmds_ModifyAO_arg_nModes;
      RTDBvar MagAOI_cmds_ModifyAO_arg_freq;
      RTDBvar MagAOI_cmds_ModifyAO_arg_nBins;
      RTDBvar MagAOI_cmds_ModifyAO_arg_ttMod;
      RTDBvar MagAOI_cmds_ModifyAO_arg_f1;
      RTDBvar MagAOI_cmds_ModifyAO_arg_f2;
      RTDBvar MagAOI_cmds_ModifyAO_arg_f3;
      RTDBvar MagAOI_cmds_ModifyAO_stat;
      RTDBvar MagAOI_cmds_ModifyAO_errMsg;

      int StartAO_tmout;
      RTDBvar MagAOI_cmds_StartAO_command;
      RTDBvar MagAOI_cmds_StartAO_command_tmout;
      RTDBvar MagAOI_cmds_StartAO_stat;
      RTDBvar MagAOI_cmds_StartAO_errMsg;

      int OffsetXY_tmout;
      RTDBvar MagAOI_cmds_OffsetXY_command;
      RTDBvar MagAOI_cmds_OffsetXY_command_tmout;
      RTDBvar MagAOI_cmds_OffsetXY_arg_x;
      RTDBvar MagAOI_cmds_OffsetXY_arg_y;
      RTDBvar MagAOI_cmds_OffsetXY_stat;
      RTDBvar MagAOI_cmds_OffsetXY_errMsg;

      int NodRaDec_tmout;
      int mountw_tmout; ///<Timeout to wait for the mount to complete an offset command
      RTDBvar MagAOI_cmds_NodRaDec_command;
      RTDBvar MagAOI_cmds_NodRaDec_command_tmout;
      RTDBvar MagAOI_cmds_NodRaDec_arg_ra;
      RTDBvar MagAOI_cmds_NodRaDec_arg_dec;
      RTDBvar MagAOI_cmds_NodRaDec_stat;
      RTDBvar MagAOI_cmds_NodRaDec_errMsg;
      RTDBvar MagAOI_cmds_NodRaDec_InProgress;

      int OffsetZ_tmout;
      RTDBvar MagAOI_cmds_OffsetZ_command;
      RTDBvar MagAOI_cmds_OffsetZ_command_tmout;
      RTDBvar MagAOI_cmds_OffsetZ_arg_z;
      RTDBvar MagAOI_cmds_OffsetZ_stat;
      RTDBvar MagAOI_cmds_OffsetZ_errMsg;

      int Focus_tmout;
      RTDBvar MagAOI_cmds_Focus_command;
      RTDBvar MagAOI_cmds_Focus_command_tmout;
      RTDBvar MagAOI_cmds_Focus_arg_z;
      RTDBvar MagAOI_cmds_Focus_stat;
      RTDBvar MagAOI_cmds_Focus_errMsg;

      int Stop_tmout;
      RTDBvar MagAOI_cmds_Stop_command;
      RTDBvar MagAOI_cmds_Stop_command_tmout;
      RTDBvar MagAOI_cmds_Stop_arg_msg;
      RTDBvar MagAOI_cmds_Stop_stat;
      RTDBvar MagAOI_cmds_Stop_errMsg;

      int Pause_tmout;
      RTDBvar MagAOI_cmds_Pause_command;
      RTDBvar MagAOI_cmds_Pause_command_tmout;
      RTDBvar MagAOI_cmds_Pause_stat;
      RTDBvar MagAOI_cmds_Pause_errMsg;

      int Resume_tmout;
      RTDBvar MagAOI_cmds_Resume_command;
      RTDBvar MagAOI_cmds_Resume_command_tmout;
      RTDBvar MagAOI_cmds_Resume_stat;
      RTDBvar MagAOI_cmds_Resume_errMsg;

      int UserPanic_tmout;
      RTDBvar MagAOI_cmds_UserPanic_command;
      RTDBvar MagAOI_cmds_UserPanic_command_tmout;
      RTDBvar MagAOI_cmds_UserPanic_arg_reason;
      RTDBvar MagAOI_cmds_UserPanic_stat;
      RTDBvar MagAOI_cmds_UserPanic_errMsg;


      int SetNewInstrument_tmout;
      RTDBvar MagAOI_cmds_SetNewInstrument_command;
      RTDBvar MagAOI_cmds_SetNewInstrument_command_tmout;
      RTDBvar MagAOI_cmds_SetNewInstrument_arg_instr;
      RTDBvar MagAOI_cmds_SetNewInstrument_arg_focalSt;
      RTDBvar MagAOI_cmds_SetNewInstrument_stat;
      RTDBvar MagAOI_cmds_SetNewInstrument_errMsg;

      
      #ifdef AOI_NOARB
      RTDBvar MagArb_LED; //Used in WfsOn
      #endif
      
      int WfsOn_tmout;
      RTDBvar MagAOI_cmds_WfsOn_command;
      RTDBvar MagAOI_cmds_WfsOn_command_tmout;
      RTDBvar MagAOI_cmds_WfsOn_arg_WfsID;
      RTDBvar MagAOI_cmds_WfsOn_stat;
      RTDBvar MagAOI_cmds_WfsOn_errMsg;

      int WfsOff_tmout;
      RTDBvar MagAOI_cmds_WfsOff_command;
      RTDBvar MagAOI_cmds_WfsOff_command_tmout;
      RTDBvar MagAOI_cmds_WfsOff_arg_WfsID;
      RTDBvar MagAOI_cmds_WfsOff_stat;
      RTDBvar MagAOI_cmds_WfsOff_errMsg;

      int AdsecOn_tmout;
      RTDBvar MagAOI_cmds_AdsecOn_command;
      RTDBvar MagAOI_cmds_AdsecOn_command_tmout;
      RTDBvar MagAOI_cmds_AdsecOn_stat;
      RTDBvar MagAOI_cmds_AdsecOn_errMsg;

      int AdsecOff_tmout;
      RTDBvar MagAOI_cmds_AdsecOff_command;
      RTDBvar MagAOI_cmds_AdsecOff_command_tmout;
      RTDBvar MagAOI_cmds_AdsecOff_stat;
      RTDBvar MagAOI_cmds_AdsecOff_errMsg;

      int AdsecSet_tmout;
      RTDBvar MagAOI_cmds_AdsecSet_command;
      RTDBvar MagAOI_cmds_AdsecSet_command_tmout;
      RTDBvar MagAOI_cmds_AdsecSet_stat;
      RTDBvar MagAOI_cmds_AdsecSet_errMsg;

      int AdsecRest_tmout;
      RTDBvar MagAOI_cmds_AdsecRest_command;
      RTDBvar MagAOI_cmds_AdsecRest_command_tmout;
      RTDBvar MagAOI_cmds_AdsecRest_stat;
      RTDBvar MagAOI_cmds_AdsecRest_errMsg;

      
      
      //@}
      /// RTDB handler for a PresetFlat command via MsgD.
      static int AOI_clear_command_changed(void *pt, Variable *msgb);


      /** @name Arbitrator Interface AOI Commands
        * The Magellan implementation of Arbitrator commands, and handlers for msgD notifications.
        * These RTDB handlers receive the variable notification from the msgD and call the appropriate command method.
        */
      //@{
         
      /// RTDB handler for a PresetFlat command via MsgD.
      static int AOI_PresetFlat_command_changed(void *pt, Variable *msgb);
      
      ///The PresetFlat command implementation for seeing limited operation
      int PresetFlat(string flatSpec);


      /// RTDB handler for a PresetVisAO command via MsgD.
      static int AOI_PresetVisAO_command_changed(void *pt, Variable *msgb);
      
      ///The AOI Gui PresetVisAO command
      int PresetVisAO(); 

      double presetvisao_fw3_pos;
      
      //std::string PresetAO(std::string AOmode, Position refStar);                    // Called by IIF

      /// RTDB handler for a PresetAO command via MsgD.
      static int AOI_PresetAO_command_changed(void *pt, Variable *msgb);
      
      ///The AOI Gui PresetAO command
      /** \todo develop way to set socoord for off-axis work
        * \todo develop plan for focStation and instr, make config vars too
        */
      int PresetAO(string AOmode, string wfsSpec, double rocoordx, double rocoordy, double mag, double cindex, int adcTracking); // Called by AOSGUI


      /// RTDB handler for a AcquireFromGuider command via MsgD.
      static int AOI_AcquireFromGuider_command_changed(void *pt, Variable *msgb);
      
      ///The AOI Gui AcquireFromGuider command
      int AcquireFromGuider(std::string ClioFOV);

      /// RTDB handler for a NudgeXY command via MsgD.
      static int AOI_NudgeXY_command_changed(void *pt, Variable *msgb);
      
      ///The NudgeXY command
      int NudgeXY(double x, double y);

      /// RTDB handler for a NudgeAE command via MsgD.
      static int AOI_NudgeAE_command_changed(void *pt, Variable *msgb);
      
      ///The AOI Gui NudgeAE command
      int NudgeAE(double a, double e);


      ///RTDB handler for a CheckRefAO command via MsgD.
      static int AOI_CheckRefAO_command_changed(void *pt, Variable *msgb);
      
      ///The CheckRefAO command
      int CheckRefAO(double *dx, double *dy, double *smag);


      /// RTDB handler for a AcquireRefAO command via MsgD.
      static int AOI_AcquireRefAO_command_changed(void *pt, Variable *msgb);
      
      ///The AOI Gui AcquireRefAO command
      int AcquireRefAO(bool repoint); // Called by AOSGUI



      ///RTDB handler for a RefineAO command via MsgD.
      static int AOI_RefineAO_command_changed(void *pt, Variable *msgb);

      ///The RefineAO command
      int RefineAO(string method);

      ///RTDB handler for the ModifyAO command via MsgD.
      static int AOI_ModifyAO_command_changed(void *pt, Variable *msgb);

      ///The ModifyAO command
      int ModifyAO(int nModes, double freq, int nBins, double ttMod, string f1, string f2, string f3);

      /// RTDB handler for a StartAO command via MsgD.
      static int AOI_StartAO_command_changed(void *pt, Variable *msgb);

      ///The StartAO command.
      int StartAO();

      /// RTDB handler for a OffsetXY command via MsgD.
      static int AOI_OffsetXY_command_changed(void *pt, Variable *msgb);
      
      /// The OffsetXY command-
      /** \ToDo Need to check loop status and adjust timeouts.   
        */
      int OffsetXY(double x, double y);

      /// RTDB handler for a NodRaDec command via MsgD.
      static int AOI_NodRaDec_command_changed(void *pt, Variable *msgb);
      
      /// The NodRaDec command
      /** \ToDo Need to check loop status and adjust timeouts (as in OffsetXY above).   
        */
      int NodRaDec(double x, double y);

      int nodxdir;
      int nodydir;

      /// The NodRaDec Open Loop command
      /** \ToDo Need to check loop status and adjust timeouts (as in OffsetXY above).   
        */
      int NodRaDecOpenLoop(double x, double y);

      /// RTDB handler for a OffsetXY command via MsgD.
      static int AOI_OffsetZ_command_changed(void *pt, Variable *msgb);
      
      ///The OffsetZ command
      int OffsetZ(double z);

      /// RTDB handler for a Focus command via MsgD.
      static int AOI_Focus_command_changed(void *pt, Variable *msgb);
      
      ///The Focus command
      /** Processes a closed loop focus offset from the IR science camera.
        * \param z the desired z stage offset in mm
        */
      int Focus(double z);


      int CorrectModes(double modes[N_CORRECTMODES]);
      int SetZernikes(int n_zern, double modes[N_ZERN]);

      /// RTDB handler for a Stop command via MsgD.
      static int AOI_Stop_command_changed(void *pt, Variable *msgb);

      /// The Stop command
      int Stop(std::string msg);

      /// RTDB handler for a Pause command via MsgD
      static int AOI_Pause_command_changed(void *pt, Variable *msgb);

      /// The Pause command
      int Pause();

      /// RTDB handler for a Resume command via MsgD
      static int AOI_Resume_command_changed(void *pt, Variable *msgb);
      
      /// The Resume command
      int Resume();

      /// RTDB handler for a User Panic command via MsgD
      static int AOI_UserPanic_command_changed(void *pt, Variable *msgb);
      
      /// The User Panic command
      int UserPanic(string reason);

      /// RTDB handler for a SetNewInstrument command via MsgD
      static int AOI_SetNewInstrument_command_changed(void *pt, Variable *msgb);

      ///Set a new instrument/focal station combo
      /** \todo SetNewInstrument currently does nothing.  Should set flat based on instrument though.
        */
      int SetNewInstrument(string instr, string focalSt);

      /// RTDB handler for a WfsOn command via MsgD.
      static int AOI_WfsOn_command_changed(void *pt, Variable *msgb);

      ///The Wfs On command
      int WfsOn(string WfsId);

      /// RTDB handler for a WfsOff command via MsgD.
      static int AOI_WfsOff_command_changed(void *pt, Variable *msgb);

      ///The WfsOff command
      int WfsOff(string WfsId);

      /// RTDB handler for a AdsecOn command via MsgD.
      static int AOI_AdsecOn_command_changed(void *pt, Variable *msgb);

      ///The AdsecOn command
      int AdsecOn();

      /// RTDB handler for a AdsecOff command via MsgD.
      static int AOI_AdsecOff_command_changed(void *pt, Variable *msgb);

      ///The AdsecOff command
      int AdsecOff();

      /// RTDB handler for a AdsecSet command via MsgD.
      static int AOI_AdsecSet_command_changed(void *pt, Variable *msgb);

      ///The AdsecSet command
      int AdsecSet();

      /// RTDB handler for a AdsecRest command via MsgD.
      static int AOI_AdsecRest_command_changed(void *pt, Variable *msgb);

      ///The AdsecRest command
      int AdsecRest();




      //@}
      
      /** @name TCS Communication Configuration and State
        */
      //@{

       ///IP Address of the TCS (from config)
      std::string   TCSaddr;
      
      ///Port of the TCS for status only requests (from config)
      int  TCSport_status;
      
      ///Port of the TCS for status and commands (from config)
      int TCSport_com;
      
      /// Poll interval (from config)
      double TCSpoll_interval;
      
      int  network_ok;

      /// Interval in seconds between SQL queries for DIMM state (from config).
      int DIMMquery_interval;

      //@} //TCS Communication Configuration and State

      /** @name TCS State
        * See TCS_communications.html for documentation of the Magellan TCS.
        * Values listed here in basically same order as in that doc
        * but grouped by dump commands.
        */
      //@{

      //Telescope time
      std::string _dateobs; ///<the UT date
      int _telut; ///<seconds since midnight
      int _telst; ///<seconds since midnight

      //Telescope position
      double _telRA; ///<Right ascension, arcsecs
      double _telDec; ///<Declination, arcsecs
      double _telEpoch; ///<Coordinate epoch (really should be equinox), years
      double _telHA; ///<Hour angle, seconds
      double _telAM; ///<Current airmass
      double _telRotOff; ///<Rotator angle, degrees

      //Telescope data
      int  _telROI; ///<rotator of interest, 0-5
      bool _telTracking; ///<whether or not the telescope is tracking, bool
      bool _telGuiding; ///<whether or not the telescope is guiding, bool
      bool _telSlewing; ///<whether or not the telescope is slewing, bool
      bool _telGuiderMoving; ///<whether or not the guider probe is moving, bool
      double _telAz; ///< telescope azimuth, arcsecs
      double _telEl; ///< telescope elevation, arcsecs
      double _telZd; ///< telescope zenith distance, arcsecs
      double _telPA; ///< Parallactic Angle, degrees
      double _telDomeAz; ///<Azimuth angle of the dome, degrees
      int _telDomeStat; ///<Dome status, 0 = closed, 1 = open, -1 = unknown

      //Target catalog
      double _catRA; ///<The catalog RA of the current target, arcsecs
      double _catDec; ///<The catalog DEC of the current target, arcsecs
      double _catEp; ///<The catalog equinox (epoch in TCS), years
      double _catRo; ///<The catalog rotator offset, degrees
      std::string _catRm; ///<The catalog rotator mode
      std::string _catObj; ///<Catalog object name

      //Secondary vane end
      //Set values
      double _telSecZ; ///<focus set value, millimeters
      double _telSecX; ///<secondary x set value, millimeters
      double _telSecY; ///<secondary y set value, millimeters
      double _telSecH; ///<secondary tip set value, arcsecs
      double _telSecV; ///<secondary tilt set value, arcsecs
      //Encoder values
      double _telEncZ;  ///<focus econder value, millimeters
      double _telEncX;  ///<secondary x encoder value, millimeters
      double _telEncY; ///<secondary y encoder value, millimeters
      double _telEncH; ///<secondary h (t/t) encoder value, millimeters
      double _telEncV; ///<secondary v (t/t) encoder value, millimeters
      int hexapod_source; ///<flag to determine which positions are put in the hexapod RTDB var.  0 (default) = set, 1 = encoder.

      //Rotator values not covered by telpos and teldata requests
      double _telRotEncAng; ///<Rotator encoder angle, degrees
      bool _rotFollowing; ///<whether or not the rotator is following

      //Environment
      double _wxtemp; ///< Outside temperature, Celsius
      double _wxpres; ///< Outside pressue, millibars
      double _wxhumid; ///< Outside humidity, percent
      double _wxwind; ///< outside wind intensity, mph
      double _wxwdir; ///< outside wind direction, degrees
      double _ttruss; ///< Telescope truss temperature, Celsius
      double _tcell; ///< Primary mirror cell temperature, Celsius
      double _tseccell; ///< Secondary mirror cell temperature, Celsius
      double _tambient; ///< Dome air temperature, Celsius

      double _wxdewpoint; ///<Dew point from weather station

      double _pwv0; ///<PWV calibration offset
      double _Hbar; ///<PWV calibration site average scale height
      double _wxpwvest; ///<PWV estimated using Ya-Lin's method

      double _wx_dimm_fwhm; ///<DIMM Seeing
      int _wx_dimm_time; ///<Time of DIMM measurement

      double _wx_mag1_fwhm; ///<Baade Seeing
      int _wx_mag1_time; ///<Time of Baade measurement

      double _wx_mag2_fwhm; ///<Clay Seeing
      int _wx_mag2_time; ///<Time of Clay measurement

      double _shellwind; ///<The shell windspeed.
     
      //TCS rtdb variables
      RTDBvar var_dateobs_cur; ///<RTDB variable for dateobs
      RTDBvar var_telut_cur; ///<RTDB variable for UT
      RTDBvar var_telst_cur; ///<RTDB variable for ST
      
      RTDBvar var_telRA_cur; ///<RTDB variable for Right Ascension
      RTDBvar var_telDec_cur; ///<RTDB variable for Declination
      RTDBvar var_telEpoch_cur; ///<RTDB variable for Epoch
      RTDBvar var_telHA_cur; ///<RTDB variable for Hour Angle
      RTDBvar var_telAM_cur; ///<RTDB variable for Airmass
      RTDBvar var_telRotOff_cur; ///<RTDB variable for Rotator Angle

      RTDBvar var_telROI_cur; ///<RTDB variable for Rotator of Interest
      RTDBvar var_telTracking; ///<RTDB variable for tracking state
      RTDBvar var_telGuiding; ///<RTDB variable for guider state
      RTDBvar var_telSlewing; ///<RTDB variable for slewing state
      RTDBvar var_telGuiderMoving; ///<RTDB variable for guider moving state
      RTDBvar var_telAz_cur;  ///<RTDB variable for Azimuth
      RTDBvar var_telEl_cur;  ///<RTDB variable for Elevation
      RTDBvar var_telZd_cur;  ///<RTDB variable for Zenith distance
      RTDBvar var_telPA_cur;  ///<RTDB variable for Parallactic angle
      RTDBvar var_telDomeAz_cur;  ///<RTDB variable for Dome Azimuth
      RTDBvar var_telDomeStat_cur;  ///<RTDB variable for Dome shutter state

      RTDBvar  var_catra;  ///<RTDB variable for the catalog right ascension
      RTDBvar  var_catdec;///<RTDB variable for the catalog declination
      RTDBvar  var_catep;///<RTDB variable for the catalog epoch (means equinox)
      RTDBvar  var_catro;///<RTDB variable for the catalog rotator offset
      RTDBvar  var_catrm;///<RTDB variable for the catalog rotator mode
      RTDBvar  var_catobj; ///<RTDB variable for the catalog object name

      RTDBvar var_obsinst; ///<RTDB variable for the observer's institution
      RTDBvar var_obsname; ///<RTDB variable for the observer's name

      RTDBvar var_telSec_SetPos; ///<RTDB variable for the vane end set position
      RTDBvar var_telSec_EncPos; ///<RTDB variable for the vane end encoder position
      RTDBvar var_telHex_AbsPos; ///<RTDB variable for the vane end positions, as hexapod for the supervisor.  can be either set or encoder postions based on value of hexapod_source flag.

      RTDBvar var_telRotAng_cur; ///<RTDB variable for the rotator encoder angle
      RTDBvar  var_rotFollowing; ///<RTDB variable for the state of rotator following.

      RTDBvar var_wxtemp; ///< RTDB variable for outside temperature, Celsius
      RTDBvar var_wxpres; ///< RTDB variable for outside pressue, millibars
      RTDBvar var_wxhumid; ///< RTDB variable for outside humidity, percent
      RTDBvar var_wxwind; ///< RTDB variable for outside wind intensity, mph
      RTDBvar var_wxwdir; ///< RTDB variable for outside wind direction, degrees
      RTDBvar var_ttruss; ///< RTDB variable for telescope truss temperature, Celsius
      RTDBvar var_tcell; ///< RTDB variable for primary mirror cell temperature, Celsius
      RTDBvar var_tseccell; ///< RTDB variable for secondary mirror cell temperature, Celsius
      RTDBvar var_tambient; ///< RTDB variable for dome air temperature, Celsius

      RTDBvar var_wxdewpoint;  ///< RTDB variable for dewpoint
      RTDBvar var_wxpwvest; ///< RTDB variable for estimated PWV

      RTDBvar var_shellwind; ///<Shell windspeed, from wind probe.
      RTDBvar var_ambwind; ///< RTDB variable for ambient wind --> either wxwind or from wind probe.

      RTDBvar var_dimmfwhm; ///<RTDB variable for DIMM seeing
      RTDBvar var_dimmtime; ///<RTDB variable for DIMM time

      RTDBvar var_mag1fwhm; ///<RTDB variable for Baade seeing
      RTDBvar var_mag1time; ///<RTDB variable for Baade time

      RTDBvar var_mag2fwhm; ///<RTDB variable for Clay seeing
      RTDBvar var_mag2time; ///<RTDB variable for Clay time

      RTDBvar var_lbtswa; ///< RTDB variable for swing arm status, which is always 1 at Magellan!

      RTDBvar var_M1_coeffs; ///<RTDB variable for M1 offload coefficients.

      RTDBvar var_LoopStat;      
      //@} //TCS State

      /** @name TCS Interface
        * See TCS_communications.html for documentation of the Magellan TCS.
        * These methods communicate with the TCS, retrieving various state values, and/or send commands
        * to the TCS.
        * The get methods generally use the group dump queries rather than the individual queries to save
        * and fill in the internal state variables, but do not update RTDBvars.
        * Only updateMagTelStatus() updates the RTDB.
        */
      //@{

      pthread_mutex_t mutex; ///< Mutex to lock communication

      ///Connect to the TCS
      int SetupNetwork();

      ///Disconnect from the TCS
      int ShutdownNetwork();

      ///Test the connection with TCS.
      virtual int TestNetwork(); 

      ///Send a status request and return the response string
      /** The basic method for querying the TCS for a status and getting the result.
        * \param statreq is the Request command.  Note that the '\n' will be added.
        * \retval string which is the TCS response.  Is empty "" on communication error,
        * otherwise needs to be checked for errors.
        */
      std::string getMagTelStatus(const std::string &statreq);

      ///Parse a space delimited string of Magellan telescope data
      /** Break the Xdata Magellan TCS "dump" commands into individual
        * strings.
        * \param tdat is the telescope response string
        * \retval vres is a vector of strings
        */
      std::vector<std::string> parse_teldata(std::string &tdat);

      ///Parse strings of the form "XX:MM:SS.SSS"
      /** \param xmsstr is the input string
        * \param x is a pointer for the first value
        * \param m is a pointer to the middle value
        * \param s is a pointer to the last value
        * \retval 0 on success
        * \retval -1 on error
        */
      int parse_xms(const std::string & xmsstr, double * x, double *m, double *s);

      ///Send a command and return the response string
      /** The basic method for sending a TCS command.
        * \retval -1000 on a communication problem, otherwise it is the
        * response from the TCS
        */
      int sendMagTelCommand(const std::string &command, int timeout);

      ///Update all Telescope status variables in the RTDB.
      /** Calls all the getX methods, and then updates the RTDBvars.
        * \retval 0 on success
        * \retval -1 on error
        */
      int updateMagTelStatus();
      
      ///Get date and time values from TCS using the datetime dump command
      /** updates _dateobs, _telut, and _telst
        * \retval 0 on success
        * \retval -1 on error
        */
      int getDateTime();

      ///Get telescope position values from TCS using the telpos dump command
      /** updates _telRA, _telDec, _telEpoch, _telHA, _telAM, _telRotAng
        * \retval 0 on success
        * \retval -1 on error
        */
      int getTelPos();

      ///Get telescope data values from TCS using the teldata dump command
      /** updates _telROI, _telTracking, _telGuiding, _telSlewing,
        * _telGuiderMoving, _telAz, _telEl, _telZd, _telPA, _telDomeAz, _telDomeStat
        * \retval 0 on success
        * \retval -1 on error
        */
      int getTelData();

      ///Get catalog values from TCS using the catdata dump command
      /** updates _catRA, _catDec, _catEp, _catRo, _catRm, _catObj.
        * \retval 0 on success
        * \retval -1 on error
        */
      int getCatData();

      ///Get vane end values from TCS using the vedata dump command
      /** updates _telSecZ, _telSecX, _telSecY, _telSecH, _telSecv,
        *  _telEncZ,_telEncX, _telEncY, _telEncH, _telEncV.
        * \retval 0 on success
        * \retval -1 on error
        */
      int getVaneData();

      ///Get the rotator encoder angle from TCS.
      /** updates _telRotEncAng
        * \retval 0 on success
        * \retval -1 on error
        */
      int getTelRotEncAng();

      ///Get the status of rotator following from TCS.
      /** updates _rotFollowing
        * \retval 0 on success
        * \retval -1 on error
        */
      int getRotFollowing();

      ///Get the telescope environment from TCS.
      /** updates _wxtemp, _wxpres, _wxhumid, _wxwind, _wxwdir, _wxdewpoint, _ttruss, _tcell, _tseccell, _tambient
        * \retval 0 on success
        * \retval -1 on error
        */
      int getTelEnv();

      ///Calulcate PWV estimate using the estimate that Ya-Lin found
      int getPWV();

      ///Query DIMM seeing by running the seedump.sh script
      int getDIMM();

      //@} //TCS Interface

      double bayside_scale;///<Bayside stage scale, in arcsec/mm

      /** @name Offloading
        * State and methods for offloading low order modes to the Telescope.  The msgD handlers are also included here
        * since offloading is what we mainly use them for.
        */
      //@{

      
      offloadCircBuff oflCB;
      
      int tt_move_delay; ///<Time in seconds between az/el offsets
      int tt_avg_len; ///<Length of moving average in seconds
      
      double TTgain; ///<The tip-tilt gain
      
      double Tmat[2][2]; ///<The tip-tilt interaction matrix

      //Secondary Offloads
      int sec_move_delay_normal; ///<Time in seconds between offloads to the vane ends during normal ops
      int sec_move_delay_collim; ///<Time in seconds between offloads to the vane ends during collimation
      int sec_move_delay; ///<Time in seconds between offloads to the vane ends.  Focus and Coma are treated separately.

      //Focus
      int focus_avg_len_normal;
      int focus_avg_len_collim;
      int focus_avg_len;
      
      double Fscale; ///<The focus scaling factor
      double secscalez; ///<The focus zernike to secondary motion conversion
      
      double Fgain; ///<Focus offload gain

      double focus_off_cum; ///<cumulative focus offload
      RTDBvar var_focus_off_cum; ///<RTDB var to report cumulative focus offload
      RTDBvar var_focus_off_cum_reset;

      double focus_off_last; ///< Last focus offload
      RTDBvar var_focus_off_last; ///<RTDB var to report last focus offload

      int focus_off_time; ///< Time of last focus offload
      RTDBvar var_focus_off_time; ///< RTDB var to report time of last focus offload
      
      //Coma      
      int comma_offloading_enabled;
      RTDBvar var_coma_off_enable_cur;
      RTDBvar var_coma_off_enable_req;


      int coma_avg_len_normal;
      int coma_avg_len_collim;
      int coma_avg_len;
      
      double Cmat[2][2]; ///<The coma rotation matrix
      double secscalex; ///<Scale factor setting vane end X motion for 1 micron zernike x coma, rms-norm
      double secscaley; ///<Scale factor setting vane end Y motion for 1 micron zernike y coma, rms-norm
      double secscalev; ///<Scale factor setting vane end V motion for 1 micron zernike x coma, rms-norm
      double secscaleh; ///<Scale factor setting vane end H motion for 1 micron zernike y coma, rms-norm

      double Cgain; ///<Gain for Coma
 
      double coma_off_cum_x; ///<Cumulative Coma X offset
      double coma_off_cum_y; ///<Cumulative Coma Y offset
      double coma_off_cum_v; ///<Cumulative Coma V offset
      double coma_off_cum_h; ///<Cumulative Coma H offset


      RTDBvar var_coma_off_cum_x; ///<RTDB var to report cumulative coma x offset
      RTDBvar var_coma_off_cum_y; ///<RTDB var to report cumulative coma y offset
      RTDBvar var_coma_off_cum_v; ///<RTDB var to report cumulative coma v offset
      RTDBvar var_coma_off_cum_h; ///<RTDB var to report cumulative coma h offset
      RTDBvar var_coma_off_cum_reset;

      //Primary Offloads
      int m1_move_delay; ///<Time in seconds between offloads to M1.

      double Amat[2][2]; ///<The astigmatism interaction matrix

      double Amax; ///<The maximum absolute value of astigmatism to send at once
      double Amaxtot; ///<The maximum cumulative absolute value of astigmatism to send
      double AcumX; ///<The cumulative amount of astigmatism sent in x
      double AcumY; ///<The cumulative amount of astigmatism sent in y

      //-------Thresholds
      double * offload_thresholds; ///<The offload threshold vector;
      double threshold_scale_focus; ///<The scale to apply to the focus threshold, default is 1
      double threshold_scale_focus_collim; ///< The scale to apply to focus threshold during collimation, <1
      double threshold_scale_coma; ///<The scale to apply to the coma threshold, default is 1
      double threshold_scale_coma_collim; ///< The scale to apply to coma threshold during collimation, <1
   
      /// Handler for Arbitrator alerts
      static int  arbalert_hndl(MsgBuf* msg, void* app, int);

      /// Default handler    
      static int  default_hndl(MsgBuf* msg, void* app, int);     

      /// Handler for engineering mode commands
      static int  hexapod_hndl(MsgBuf* msg, void* app, int);

      /// Handler for offload modes
      static int  offload_hndl(MsgBuf* msg, void* app, int);

      /// Handler for housekeeping commands
      static int  housekeep_hndl(MsgBuf* msg, void* app, int);   



      /// Do an offload
      void offload(float *z, bool ignore);

      /// Handler for focus_cum reset
      static int focus_cum_reset_req(void *pt, Variable *msgb);
      void focus_cum_reset();

      /// Handler for coma_cum reset
      static int coma_cum_reset_req(void *pt, Variable *msgb);
      void coma_cum_reset();

      void initialize_offload_cum();


      
      //Offload for collimation
      RTDBvar var_offcollim;

      static int offcollim_changed(void *pt, Variable *msgb);

      void set_offload_collimation();


      //@}

      /** @name Offload testing
        * macro protected facility to test offload propagation.
        */
      //@{

      #ifdef OFFLOAD_TESTING

      RTDBvar var_testoffload_tt;  //0 
      RTDBvar var_testoffload_foc; //1
      RTDBvar var_testoffload_com; //2
      RTDBvar var_testoffload_LO; //3
      RTDBvar var_testoffload_astig; //4
      RTDBvar var_testoffload_ALL; //5 ///<When changed (to any value) in RTDB we send the test offload vector

      RTDBvar var_testoff_ttX;
      RTDBvar var_testoff_ttY;
      RTDBvar var_testoff_foc;
      RTDBvar var_testoff_comaX;
      RTDBvar var_testoff_comaY;
      RTDBvar var_testoff_astigX;
      RTDBvar var_testoff_astigY;


      /// RTDB handler for testoffload
      static int testoffload_tt_changed(void *pt, Variable *msgb);
      static int testoffload_foc_changed(void *pt, Variable *msgb);
      static int testoffload_com_changed(void *pt, Variable *msgb);
      static int testoffload_LO_changed(void *pt, Variable *msgb);
      static int testoffload_astig_changed(void *pt, Variable *msgb);
      static int testoffload_ALL_changed(void *pt, Variable *msgb);

      /// Sends a test offload vector by calling offload(z, false) with a hard coded set of values.
      int send_testoffload(int type);

      #endif

      //@}

      /** @name Utilities
        */
      //@{

      void AOIinfoLog(std::string);
       
      //@}


};



#endif //__MagAOI_h__

