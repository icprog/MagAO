#include "MagAOI.h"

using namespace Arcetri;

//#define _debug

#define LOG_TCS_STATUS



#include "arblib/aoArb/AOCommandsExport.h"

MagAOI::MagAOI( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
}

MagAOI::~MagAOI()
{
   if(arbIntf) delete arbIntf;
   arbIntf=0;

   delete offload_thresholds;
}

void MagAOI::Create() throw (AOException)
{   
   pthread_mutex_init( &aoiMutex, NULL);

   arbIntf = 0;

   LoadConfig();
   
   network_ok = 0;

}


void MagAOI::InstallHandlers() // Install handlers
{
   int stat;

   // Handler for Arbitrator alerts Notification
   if((stat=thHandler(ARB_ALERT, "*", 0, arbalert_hndl, (char *)"arbalert", this))<0)
      throw AOException("thHandler error in installing arbalert handler", stat,__FILE__, __LINE__);

   // Handler for hexapod commands
   if((stat=thHandler(AOS_HXP_CMD, (char *)"*", 0, hexapod_hndl, (char *)"AOS_HXP_CMD", this))<0)
      throw AOException("thHandler error in installing AOS_HXP_CMD handler", stat,__FILE__, __LINE__);

   // Handler for AoSup commands
   if((stat=thHandler(AOS_HOUSKEEP, (char *)"*", 0, housekeep_hndl, (char *)"AOS_HOUSKEEP", this))<0)
      throw AOException("thHandler error in installing AOS_HOUSKEEP handler", stat,__FILE__, __LINE__);

   // Offload modes handler
   if((stat=thHandler(AOS_OFFLOAD, (char *)"*", 0, offload_hndl, (char *)"AOS_OFFLOAD", this))<0)
      throw AOException("thHandler error in installing AOS_OFFLOAD handler", stat,__FILE__, __LINE__);

   // Default handler
   //if((stat=thHandler(ANY_MSG, (char *)"*", 0, default_hndl, (char *)"default", this))<0)
   //   throw AOException("thHandler error in installing default handler", stat,__FILE__, __LINE__);
}

#define DTOR 0.0174533

int MagAOI::LoadConfig()
{
   Config_File *cfg = &ConfigDictionary();
   try 
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "Reading vars.");
      TCSaddr  = (std::string) (*cfg)["TCSaddr"];
      TCSport_status = (int)(*cfg)["TCSport_status"];
      TCSport_com = (int)(*cfg)["TCSport_com"];
      TCSpoll_interval = (double)(*cfg)["TCSpoll_interval"];

      DIMMquery_interval = (int)(*cfg)["DIMMquery_interval"];

      bayside_scale = (double)(*cfg)["bayside_scale"];

      
      /*** offload config ***/
      
      int offload_buffer_length = 3600;
      oflCB.setLength(offload_buffer_length, OFL_MODES);
      
      double TTscale = (double)(*cfg)["TTscale"];
      double TTrot = (double)(*cfg)["TTrot"];
      int TTparity = (int)(*cfg)["TTparity"];

      Tmat[0][0] = TTparity*TTscale * cos(TTrot*DTOR);
      Tmat[0][1] = -TTscale * sin(TTrot*DTOR);
      Tmat[1][0] = TTparity*TTscale * sin(TTrot*DTOR);
      Tmat[1][1] = TTscale * cos(TTrot*DTOR);

      TTgain = (double)(*cfg)["TTgain"];

      tt_move_delay = (int)(*cfg)["tt_move_delay"];
      tt_avg_len = (int)(*cfg)["tt_avg_len"];
      
      if(tt_move_delay < tt_avg_len) tt_move_delay = tt_avg_len;
      
      //tt_zerodelay_threshold = (int)(*cfg)["tt_zerodelay_threshold"];

      sec_move_delay_normal = (int)(*cfg)["sec_move_delay_normal"];
      sec_move_delay_collim = (int)(*cfg)["sec_move_delay_collim"];
      sec_move_delay = sec_move_delay_normal;

      m1_move_delay = (int)(*cfg)["m1_move_delay"];

      
      focus_avg_len_normal = (int)(*cfg)["focus_avg_len_normal"];
      focus_avg_len_collim = (int)(*cfg)["focus_avg_len_collim"];
      
      if(focus_avg_len_normal > sec_move_delay_normal) sec_move_delay_normal = focus_avg_len_normal;
      if(focus_avg_len_collim > sec_move_delay_collim) sec_move_delay_collim = focus_avg_len_collim;
      focus_avg_len = focus_avg_len_normal;
      
      Fscale = (double)(*cfg)["Fscale"];
      secscalez = (double)(*cfg)["secscalez"];
      Fgain = (double)(*cfg)["Fgain"];

      focus_off_cum = 0.;
      focus_off_last = 0.;
      focus_off_time = 0;


      //coma_offloading_enabled = 0;
      

      coma_avg_len_normal = (int)(*cfg)["coma_avg_len_normal"];
      coma_avg_len_collim = (int)(*cfg)["coma_avg_len_collim"];
      
      if(coma_avg_len_normal > sec_move_delay_normal) sec_move_delay_normal = coma_avg_len_normal;
      if(coma_avg_len_collim > sec_move_delay_collim) sec_move_delay_collim = coma_avg_len_collim;
      coma_avg_len = coma_avg_len_normal;
      
      double Cscale = (double)(*cfg)["Cscale"];
      double Crot = (double)(*cfg)["Crot"];
      int Cparity = (int)(*cfg)["Cparity"];

      Cmat[0][0] = Cparity*Cscale * cos(Crot*DTOR);
      Cmat[0][1] = -Cscale * sin(Crot*DTOR);
      Cmat[1][0] = Cparity*Cscale * sin(Crot*DTOR);
      Cmat[1][1] = Cscale * cos(Crot*DTOR);

      Cgain =  (double)(*cfg)["Cgain"];
      secscalex = (double)(*cfg)["secscalex"];
      secscaley = (double)(*cfg)["secscaley"];
      secscalev = (double)(*cfg)["secscalev"];
      secscaleh = (double)(*cfg)["secscaleh"];

      coma_off_cum_x = 0.;
      coma_off_cum_y = 0.;
      coma_off_cum_v = 0.;
      coma_off_cum_h = 0.;

      double Ascale = (double)(*cfg)["Ascale"];
      double Arot = (double)(*cfg)["Arot"];
      int Aparity = (int)(*cfg)["Aparity"];
      double Again = (double)(*cfg)["Again"];
      Amax = (double)(*cfg)["Amax"];

      //Calculate the Astigmatism rotation matrix
      Amat[0][0] = Aparity*Ascale*Again * cos(Arot*DTOR);
      Amat[0][1] = -Ascale*Again * sin(Arot*DTOR);
      Amat[1][0] = Ascale*Again * sin(Arot*DTOR);
      Amat[1][1] = Aparity*Again * Ascale * cos(Arot*DTOR);

      nodxdir = (int)(*cfg)["nodxdir"];
      nodydir = (int)(*cfg)["nodydir"];

      _pwv0 = 1.206;
      _Hbar = 0.288;

   } 
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "Missing TCS configuration data: %s", e.what().c_str());
      throw(e);
   }

   try 
   {
      hexapod_source = (int)(*cfg)["hexapod_source"];
      Logger::get()->log( Logger::LOG_LEV_INFO, "Hexapod source set to %i (0=set, 1=enc)", hexapod_source);
   }
   catch (Config_File_Exception &e)
   {
      hexapod_source = 0;
      Logger::get()->log( Logger::LOG_LEV_INFO, "Hexapod source set to 0 (set, default)");
   }

   //Load the offload thresholds.
   offload_thresholds = new double[OFL_ZERN_NUMB];
   char offt_name[64];
   for(int i=0;i<OFL_ZERN_NUMB; i++)
   {
      snprintf(offt_name, 64, "OffThresh_%i", i);
      try
      {
         offload_thresholds[i] = (double)(*cfg)[offt_name];
      }
      catch(...)
      {
         offload_thresholds[i] = 0.;
      }
   }

   try
   {
      threshold_scale_focus = 1.0;
      threshold_scale_focus_collim = (double)(*cfg)["threshold_scale_focus_collim"];

      threshold_scale_coma = 1.0;
      threshold_scale_coma_collim = (double)(*cfg)["threshold_scale_coma_collim"];
   }
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "Missing TCS configuration data: %s", e.what().c_str());
      throw(e);
   }

   try
   {
      focalStation = (std::string)(*cfg)["Focal_Station"];
      instrumentName = (std::string)(*cfg)["Instrument_Name"];
   }
   catch (Config_File_Exception &e)
   {
      focalStation = "MAGAO_NASE";
      instrumentName = "MagAO_bs5050";
   }

   try
   {  
      PresetFlat_tmout = (int)(*cfg)["cmd_presetflat_tmout"];

      //AcquireFromGuider_tmout = (int)(*cfg)["cmd_presetflat_tmout"];
//Do AFG tmout here

      PresetVisAO_tmout = (int)(*cfg)["cmd_presetvisao_tmout"];
      PresetAO_tmout = (int)(*cfg)["cmd_presetao_tmout"];
      AcqGuider_tmout = (int)(*cfg)["cmd_acqguider_tmout"];
      NudgeXY_tmout = (int)(*cfg)["cmd_nudgexy_tmout"];
      NudgeAE_tmout = (int)(*cfg)["cmd_nudgeae_tmout"];

      CheckRefAO_tmout = (int)(*cfg)["cmd_checkref_tmout"];
      AcquireRefAO_tmout = (int)(*cfg)["cmd_acquireref_tmout"];
      RefineAO_tmout = (int)(*cfg)["cmd_refine_tmout"];
      ModifyAO_tmout = (int)(*cfg)["cmd_modify_tmout"];
      StartAO_tmout = (int)(*cfg)["cmd_startao_tmout"];
      OffsetSpeedFactorCL = (double)(*cfg)["OffsetSpeedFactorCL"];
      OffsetSpeedFactorOL = (double)(*cfg)["OffsetSpeedFactorOL"];
      AOOffsetXYTmo = (int)(*cfg)["AOOffsetXYTmo"];
      mountw_tmout = (int)(*cfg)["mountw_tmout"];
      AONodRaDecTmo = (int)(*cfg)["AONodRaDecTmo"];
      AOOffsetZTmo = (int)(*cfg)["AOOffsetZTmo"];
      //Focus_tmout = (int)(*cfg)["Focus_tmout"];
      Stop_tmout = (int)(*cfg)["cmd_stop_tmout"];
      Pause_tmout = (int)(*cfg)["cmd_pause_tmout"];
      Resume_tmout = (int)(*cfg)["cmd_resume_tmout"];
      UserPanic_tmout = (int)(*cfg)["cmd_userpanic_tmout"];
      SetNewInstrument_tmout = (int)(*cfg)["cmd_setnewinstrument_tmout"];
      WfsOn_tmout = (int)(*cfg)["cmd_wfson_tmout"];
      WfsOff_tmout = (int)(*cfg)["cmd_wfsoff_tmout"];
      AdsecOn_tmout = (int)(*cfg)["cmd_adsecon_tmout"];
      AdsecOff_tmout = (int)(*cfg)["cmd_adsecoff_tmout"];
      AdsecSet_tmout = (int)(*cfg)["cmd_adsecset_tmout"];
      AdsecRest_tmout = (int)(*cfg)["cmd_adsecrest_tmout"];
   }
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "Missing AOI configuration data: %s", e.what().c_str());
      throw(e);
   }

   try
   {
      presetvisao_fw3_pos = (double)(*cfg)["presetvisao_fw3_pos"];
   }
   catch (Config_File_Exception &e)
   {
      presetvisao_fw3_pos = 3.;
   }


   //load guider acquisition presets
   try
   {
      guider_acq_wide_full_az = (double)(*cfg)["guider_acq_wide_full_az"];
      guider_acq_wide_full_el = (double)(*cfg)["guider_acq_wide_full_el"];
      guider_acq_wide_strip_az = (double)(*cfg)["guider_acq_wide_strip_az"];
      guider_acq_wide_strip_el = (double)(*cfg)["guider_acq_wide_strip_el"];
      guider_acq_wide_stamp_az = (double)(*cfg)["guider_acq_wide_stamp_az"];
      guider_acq_wide_stamp_el = (double)(*cfg)["guider_acq_wide_stamp_el"];
      guider_acq_wide_substamp_az = (double)(*cfg)["guider_acq_wide_substamp_az"];
      guider_acq_wide_substamp_el = (double)(*cfg)["guider_acq_wide_substamp_el"];
      guider_acq_narrow_full_az = (double)(*cfg)["guider_acq_narrow_full_az"];
      guider_acq_narrow_full_el = (double)(*cfg)["guider_acq_narrow_full_el"];
      guider_acq_narrow_strip_az = (double)(*cfg)["guider_acq_narrow_strip_az"];
      guider_acq_narrow_strip_el = (double)(*cfg)["guider_acq_narrow_strip_el"];
      guider_acq_narrow_stamp_az = (double)(*cfg)["guider_acq_narrow_stamp_az"];
      guider_acq_narrow_stamp_el = (double)(*cfg)["guider_acq_narrow_stamp_el"];
      guider_acq_narrow_substamp_az = (double)(*cfg)["guider_acq_narrow_substamp_az"];
      guider_acq_narrow_substamp_el = (double)(*cfg)["guider_acq_narrow_substamp_el"];
   }
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "Missing AOI configuration data: %s", e.what().c_str());
      throw(e);
   }
   return NO_ERROR;
}

void MagAOI::SetupVars()
{

   //Setup the AOI Command RTDB variables
   try
   {
      /*** Loop Status ***/
      AOARB_loopon = RTDBvar("AOARB.L", "LOOPON", NO_DIR, INT_VARIABLE, 1);

      MagAOI_cmds_updateargs = RTDBvar("MagAOI.cmds", "updateargs", NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_clear = RTDBvar("MagAOI.cmds", "clear", NO_DIR, INT_VARIABLE, 1);

       //Notification for Clear
      Notify(MagAOI_cmds_clear, AOI_clear_command_changed);


      /*** PresetFlat ***/
      MagAOI_cmds_PresetFlat_command = RTDBvar("MagAOI.cmds", "PresetFlat",  NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_PresetFlat_command_tmout = RTDBvar("MagAOI.cmds", "PresetFlat.tmout",  NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_PresetFlat_arg_flatSpec = RTDBvar("MagAOI.cmds", "PresetFlat.arg.flatSpec", NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_PresetFlat_stat  = RTDBvar("MagAOI.cmds", "PresetFlat.stat", NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_PresetFlat_errMsg  = RTDBvar("MagAOI.cmds", "PresetFlat.errMsg", NO_DIR, CHAR_VARIABLE, 256);

      //Notification for PresetFlat
      Notify( MagAOI_cmds_PresetFlat_command, AOI_PresetFlat_command_changed);

      /*** PresetVisAO ***/
      MagAOI_cmds_PresetVisAO_command = RTDBvar("MagAOI.cmds", "PresetVisAO",              NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_PresetVisAO_command_tmout = RTDBvar("MagAOI.cmds", "PresetVisAO.tmout",      NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_PresetVisAO_stat = RTDBvar("MagAOI.cmds", "PresetVisAO.stat",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_PresetVisAO_errMsg = RTDBvar("MagAOI.cmds", "PresetVisAO.errMsg",       NO_DIR, CHAR_VARIABLE, 256);
      //Notification for PresetVisAO
      Notify(MagAOI_cmds_PresetVisAO_command, AOI_PresetVisAO_command_changed);

      /*** AcquireFromGuider ***/
      MagAOI_cmds_AcquireFromGuider_command = RTDBvar("MagAOI.cmds", "AcquireFromGuider",              NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AcquireFromGuider_tmout = RTDBvar("MagAOI.cmds", "AcquireFromGuider.tmout",      NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AcquireFromGuider_stat = RTDBvar("MagAOI.cmds", "AcquireFromGuider.stat",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AcquireFromGuider_errMsg = RTDBvar("MagAOI.cmds", "AcquireFromGuider.errMsg",       NO_DIR, CHAR_VARIABLE, 256);
      //Notification for AcquireFromGuider
      Notify(MagAOI_cmds_AcquireFromGuider_command, AOI_AcquireFromGuider_command_changed);

      /*** NudgeXY ***/
      MagAOI_cmds_NudgeXY_command = RTDBvar("MagAOI.cmds", "NudgeXY",              NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_NudgeXY_tmout = RTDBvar("MagAOI.cmds", "NudgeXY.tmout",      NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_NudgeXY_arg_x = RTDBvar("MagAOI.cmds", "NudgeXY.arg.x",         NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_NudgeXY_arg_y = RTDBvar("MagAOI.cmds", "NudgeXY.arg.y",         NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_NudgeXY_stat = RTDBvar("MagAOI.cmds", "NudgeXY.stat",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_NudgeXY_errMsg = RTDBvar("MagAOI.cmds", "NudgeXY.errMsg",       NO_DIR, CHAR_VARIABLE, 256);
      //Notification for NudgeXY
      Notify(MagAOI_cmds_NudgeXY_command, AOI_NudgeXY_command_changed);

      /*** NudgeAE ***/
      MagAOI_cmds_NudgeAE_command = RTDBvar("MagAOI.cmds", "NudgeAE",              NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_NudgeAE_tmout = RTDBvar("MagAOI.cmds", "NudgeAE.tmout",      NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_NudgeAE_arg_a = RTDBvar("MagAOI.cmds", "NudgeAE.arg.a",         NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_NudgeAE_arg_e = RTDBvar("MagAOI.cmds", "NudgeAE.arg.e",         NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_NudgeAE_stat = RTDBvar("MagAOI.cmds", "NudgeAE.stat",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_NudgeAE_errMsg = RTDBvar("MagAOI.cmds", "NudgeAE.errMsg",       NO_DIR, CHAR_VARIABLE, 256);
      //Notification for NudgeAE
      Notify(MagAOI_cmds_NudgeAE_command, AOI_NudgeAE_command_changed);



      /*** PresetAO ***/
      MagAOI_cmds_PresetAO_command      = RTDBvar("MagAOI.cmds", "PresetAO",              NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_PresetAO_command_tmout  = RTDBvar("MagAOI.cmds", "PresetAO.tmout",      NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_PresetAO_arg_AOmode   = RTDBvar("MagAOI.cmds", "PresetAO.arg.AOmode",   NO_DIR, CHAR_VARIABLE, 50);
      MagAOI_cmds_PresetAO_arg_wfsSpec  = RTDBvar("MagAOI.cmds", "PresetAO.arg.wfsSpec",  NO_DIR, CHAR_VARIABLE, 50);
      MagAOI_cmds_PresetAO_arg_wfsSpec.Set("MAGWFS");

      MagAOI_cmds_PresetAO_arg_rocoordx = RTDBvar("MagAOI.cmds", "PresetAO.arg.rocoordx", NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_PresetAO_arg_rocoordy = RTDBvar("MagAOI.cmds", "PresetAO.arg.rocoordy", NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_PresetAO_arg_mag      = RTDBvar("MagAOI.cmds", "PresetAO.arg.mag",      NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_PresetAO_arg_cindex   = RTDBvar("MagAOI.cmds", "PresetAO.arg.cindex",   NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_PresetAO_arg_adcTracking   = RTDBvar("MagAOI.cmds", "PresetAO.arg.adcTracking",   NO_DIR, INT_VARIABLE, 1);

      MagAOI_cmds_PresetAO_stat         = RTDBvar("MagAOI.cmds", "PresetAO.stat",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_PresetAO_errMsg       = RTDBvar("MagAOI.cmds", "PresetAO.errMsg",       NO_DIR, CHAR_VARIABLE, 256);

      //Notification for PresetAO
      Notify(MagAOI_cmds_PresetAO_command, AOI_PresetAO_command_changed);

      /*** AcquireRefAO ***/
      MagAOI_cmds_AcquireRefAO_command = RTDBvar("MagAOI.cmds", "AcquireRefAO",              NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AcquireRefAO_command_tmout = RTDBvar("MagAOI.cmds", "AcquireRefAO.tmout", NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AcquireRefAO_stat = RTDBvar("MagAOI.cmds", "AcquireRefAO.stat",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AcquireRefAO_errMsg = RTDBvar("MagAOI.cmds", "AcquireRefAO.errMsg",       NO_DIR, CHAR_VARIABLE, 256);

      MagAOI_cmds_AcquireRefAO_result_deltaX = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.deltaX", NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_AcquireRefAO_result_deltaY = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.deltaY", NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_AcquireRefAO_result_slNull = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.slNull", NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_AcquireRefAO_result_f1spec = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.f1spec", NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_AcquireRefAO_result_f2spec = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.f2spec", NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_AcquireRefAO_result_freq  = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.freq", NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_AcquireRefAO_result_gain  = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.gain", NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_AcquireRefAO_result_starMag = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.starMag", NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_AcquireRefAO_result_nBins = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.nBins", NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AcquireRefAO_result_nModes = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.nModes", NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AcquireRefAO_result_r0 = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.r0", NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_AcquireRefAO_result_snmode = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.snmode", NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_AcquireRefAO_result_strehl = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.strehl", NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_AcquireRefAO_result_ttMod = RTDBvar("MagAOI.cmds", "AcquireRefAO.result.ttMod", NO_DIR, REAL_VARIABLE, 1);


      //Notification for AcquireRefAO
      Notify(MagAOI_cmds_AcquireRefAO_command, AOI_AcquireRefAO_command_changed);

      
      /*** CheckRefAO ***/
      MagAOI_cmds_CheckRefAO_command = RTDBvar("MagAOI.cmds", "CheckRefAO",        NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_CheckRefAO_command_tmout = RTDBvar("MagAOI.cmds", "CheckRefAO.tmout", NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_CheckRefAO_result  = RTDBvar("MagAOI.cmds", "CheckRefAO.result", NO_DIR, REAL_VARIABLE, 3);
      MagAOI_cmds_CheckRefAO_stat  = RTDBvar("MagAOI.cmds", "CheckRefAO.stat",     NO_DIR, INT_VARIABLE, 1); 
      MagAOI_cmds_CheckRefAO_errMsg  = RTDBvar("MagAOI.cmds", "CheckRefAO.errMsg", NO_DIR, CHAR_VARIABLE, 256);

      //Notification for CheckRefAO
      Notify(MagAOI_cmds_CheckRefAO_command, AOI_CheckRefAO_command_changed);

      /*** RefineAO ***/
      MagAOI_cmds_RefineAO_command =    RTDBvar("MagAOI.cmds", "RefineAO",            NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_RefineAO_command_tmout =    RTDBvar("MagAOI.cmds", "RefineAO.tmout", NO_DIR,INT_VARIABLE, 1);
      MagAOI_cmds_RefineAO_arg_method = RTDBvar("MagAOI.cmds", "RefineAO.arg.method", NO_DIR, CHAR_VARIABLE, 50);
      MagAOI_cmds_RefineAO_stat =       RTDBvar("MagAOI.cmds", "RefineAO.stat",       NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_RefineAO_errMsg =     RTDBvar("MagAOI.cmds", "RefineAO.errMsg",     NO_DIR, CHAR_VARIABLE, 256);

      //Notification for RefineAO
      Notify(MagAOI_cmds_RefineAO_command, AOI_RefineAO_command_changed);

      MagAOI_cmds_ModifyAO_command       = RTDBvar("MagAOI.cmds", "ModifyAO",            NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_ModifyAO_command_tmout = RTDBvar("MagAOI.cmds", "ModifyAO.tmout",            NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_ModifyAO_arg_nModes = RTDBvar("MagAOI.cmds", "ModifyAO.arg.nModes", NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_ModifyAO_arg_freq   = RTDBvar("MagAOI.cmds", "ModifyAO.arg.freq",   NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_ModifyAO_arg_nBins  = RTDBvar("MagAOI.cmds", "ModifyAO.arg.nBins",  NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_ModifyAO_arg_ttMod  = RTDBvar("MagAOI.cmds", "ModifyAO.arg.ttMod",  NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_ModifyAO_arg_f1     = RTDBvar("MagAOI.cmds", "ModifyAO.arg.f1",     NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_ModifyAO_arg_f2     = RTDBvar("MagAOI.cmds", "ModifyAO.arg.f2",     NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_ModifyAO_arg_f3     = RTDBvar("MagAOI.cmds", "ModifyAO.arg.f3",     NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_ModifyAO_stat       = RTDBvar("MagAOI.cmds", "ModifyAO.stat",       NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_ModifyAO_errMsg     = RTDBvar("MagAOI.cmds", "ModifyAO.errMsg",     NO_DIR, CHAR_VARIABLE, 256);

      //Notification for ModifyAO
      Notify(MagAOI_cmds_ModifyAO_command, AOI_ModifyAO_command_changed);
      
      /*** StartAO ***/
      MagAOI_cmds_StartAO_command = RTDBvar("MagAOI.cmds", "StartAO",        NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_StartAO_command_tmout = RTDBvar("MagAOI.cmds", "StartAO.tmout",        NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_StartAO_stat    = RTDBvar("MagAOI.cmds", "StartAO.stat",   NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_StartAO_errMsg  = RTDBvar("MagAOI.cmds", "StartAO.errMsg", NO_DIR, CHAR_VARIABLE, 256);

      //Notification for StartAO
      Notify(MagAOI_cmds_StartAO_command, AOI_StartAO_command_changed);

      MagAOI_cmds_OffsetXY_command = RTDBvar("MagAOI.cmds", "OffsetXY",        NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_OffsetXY_command_tmout = RTDBvar("MagAOI.cmds", "OffsetXY.tmout",        NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_OffsetXY_arg_x   = RTDBvar("MagAOI.cmds", "OffsetXY.arg.x",  NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_OffsetXY_arg_y   = RTDBvar("MagAOI.cmds", "OffsetXY.arg.y",  NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_OffsetXY_stat    = RTDBvar("MagAOI.cmds", "OffsetXY.stat",   NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_OffsetXY_errMsg  = RTDBvar("MagAOI.cmds", "OffsetXY.errMsg", NO_DIR, CHAR_VARIABLE, 256);

      //Notification for OffsetXY
      Notify(MagAOI_cmds_OffsetXY_command, AOI_OffsetXY_command_changed);

      //---- NodRaDec ----/
      MagAOI_cmds_NodRaDec_command = RTDBvar("MagAOI.cmds", "NodRaDec", NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_NodRaDec_command_tmout = RTDBvar("MagAOI.cmds", "NodRaDec.tmout",  NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_NodRaDec_arg_ra   = RTDBvar("MagAOI.cmds", "NodRaDec.arg.ra",  NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_NodRaDec_arg_dec   = RTDBvar("MagAOI.cmds", "NodRaDec.arg.dec",  NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_NodRaDec_stat    = RTDBvar("MagAOI.cmds", "NodRaDec.stat",   NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_NodRaDec_errMsg  = RTDBvar("MagAOI.cmds", "NodRaDec.errMsg", NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_NodRaDec_InProgress = RTDBvar("MagAOI.cmds", "NodRaDec.InProgress", NO_DIR, INT_VARIABLE, 1);

      //Notification for NodRaDec
      Notify(MagAOI_cmds_NodRaDec_command, AOI_NodRaDec_command_changed);


      //---- OffsetZ ----//
      MagAOI_cmds_OffsetZ_command = RTDBvar("MagAOI.cmds", "OffsetZ",        NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_OffsetZ_command_tmout = RTDBvar("MagAOI.cmds", "OffsetZ.tmout",        NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_OffsetZ_arg_z   = RTDBvar("MagAOI.cmds", "OffsetZ.arg.z",  NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_OffsetZ_stat    = RTDBvar("MagAOI.cmds", "OffsetZ.stat",   NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_OffsetZ_errMsg  = RTDBvar("MagAOI.cmds", "OffsetZ.errMsg", NO_DIR, CHAR_VARIABLE, 256);

      //Notification for OffsetZ
      Notify(MagAOI_cmds_OffsetZ_command, AOI_OffsetZ_command_changed);

      //---- Focus ----//
      MagAOI_cmds_Focus_command = RTDBvar("MagAOI.cmds", "Focus",        NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_Focus_command_tmout = RTDBvar("MagAOI.cmds", "Focus.tmout",        NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_Focus_arg_z = RTDBvar("MagAOI.cmds", "Focus.arg.z",  NO_DIR, REAL_VARIABLE, 1);
      MagAOI_cmds_Focus_stat = RTDBvar("MagAOI.cmds", "Focus.stat",   NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_Focus_errMsg = RTDBvar("MagAOI.cmds", "Focus.errMsg", NO_DIR, CHAR_VARIABLE, 256);

      //Notification for Focus
      Notify(MagAOI_cmds_Focus_command, AOI_Focus_command_changed);


      //---- Stop ----//
      MagAOI_cmds_Stop_command = RTDBvar("MagAOI.cmds", "Stop",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_Stop_command_tmout = RTDBvar("MagAOI.cmds", "Stop.tmout",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_Stop_arg_msg = RTDBvar("MagAOI.cmds", "Stop.arg.msg", NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_Stop_stat    = RTDBvar("MagAOI.cmds", "Stop.stat",    NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_Stop_errMsg  = RTDBvar("MagAOI.cmds", "Stop.errMsg",  NO_DIR, CHAR_VARIABLE, 256);

      //Notification for Stop
      Notify(MagAOI_cmds_Stop_command, AOI_Stop_command_changed);

      //---- Pause ----//
      MagAOI_cmds_Pause_command = RTDBvar("MagAOI.cmds", "Pause",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_Pause_command_tmout = RTDBvar("MagAOI.cmds", "Pause.tmout",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_Pause_stat    = RTDBvar("MagAOI.cmds", "Pause.stat",    NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_Pause_errMsg  = RTDBvar("MagAOI.cmds", "Pause.errMsg",  NO_DIR, CHAR_VARIABLE, 256);
      
      //Notification for Pause
      Notify(MagAOI_cmds_Pause_command, AOI_Pause_command_changed);

      //---- Resume ----//
      MagAOI_cmds_Resume_command = RTDBvar("MagAOI.cmds", "Resume",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_Resume_command_tmout = RTDBvar("MagAOI.cmds", "Resume.tmout",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_Resume_stat    = RTDBvar("MagAOI.cmds", "Resume.stat",    NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_Resume_errMsg  = RTDBvar("MagAOI.cmds", "Resume.errMsg",  NO_DIR, CHAR_VARIABLE, 256);
      
      //Notification for Resume
      Notify(MagAOI_cmds_Resume_command, AOI_Resume_command_changed);

      //--- UserPanic ---//
      MagAOI_cmds_UserPanic_command = RTDBvar("MagAOI.cmds", "UserPanic",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_UserPanic_command_tmout = RTDBvar("MagAOI.cmds", "UserPanic.tmout",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_UserPanic_arg_reason = RTDBvar("MagAOI.cmds", "UserPanic.arg.reason", NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_UserPanic_stat = RTDBvar("MagAOI.cmds", "UserPanic.stat",    NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_UserPanic_errMsg = RTDBvar("MagAOI.cmds", "UserPanic.errMsg",  NO_DIR, CHAR_VARIABLE, 256);
      
      //Notification for UserPanic
      Notify(MagAOI_cmds_UserPanic_command, AOI_UserPanic_command_changed);


      //--- SetNewInstrument ---//
      MagAOI_cmds_SetNewInstrument_command = RTDBvar("MagAOI.cmds", "SetNewInstrument",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_SetNewInstrument_command_tmout = RTDBvar("MagAOI.cmds", "SetNewInstrument.tmout",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_SetNewInstrument_arg_instr = RTDBvar("MagAOI.cmds", "SetNewInstrument.arg.instr", NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_SetNewInstrument_arg_focalSt = RTDBvar("MagAOI.cmds", "SetNewInstrument.arg.focalSt", NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_SetNewInstrument_stat = RTDBvar("MagAOI.cmds", "SetNewInstrument.stat",    NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_SetNewInstrument_errMsg = RTDBvar("MagAOI.cmds", "SetNewInstrument.errMsg",  NO_DIR, CHAR_VARIABLE, 256);
      
      //Notification for SetNewInstrument
      Notify(MagAOI_cmds_SetNewInstrument_command, AOI_SetNewInstrument_command_changed);
      
      
      #ifdef AOI_NOARB
      MagArb_LED = RTDBvar("wfsarb.L", "LED", NO_DIR, INT_VARIABLE, 1);
      #endif
      
      MagAOI_cmds_WfsOn_command = RTDBvar("MagAOI.cmds", "WfsOn",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_WfsOn_command_tmout = RTDBvar("MagAOI.cmds", "WfsOn.tmout",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_WfsOn_arg_WfsID = RTDBvar("MagAOI.cmds", "WfsOn.arg.WfsID", NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_WfsOn_stat = RTDBvar("MagAOI.cmds", "WfsOn.stat",    NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_WfsOn_errMsg = RTDBvar("MagAOI.cmds", "WfsOn.errMsg",  NO_DIR, CHAR_VARIABLE, 256);

      //Notification for WfsOn
      Notify(MagAOI_cmds_WfsOn_command, AOI_WfsOn_command_changed);

      MagAOI_cmds_WfsOff_command = RTDBvar("MagAOI.cmds", "WfsOff",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_WfsOff_command_tmout = RTDBvar("MagAOI.cmds", "WfsOff.tmout",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_WfsOff_arg_WfsID = RTDBvar("MagAOI.cmds", "WfsOff.arg.WfsID", NO_DIR, CHAR_VARIABLE, 256);
      MagAOI_cmds_WfsOff_stat = RTDBvar("MagAOI.cmds", "WfsOff.stat",    NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_WfsOff_errMsg = RTDBvar("MagAOI.cmds", "WfsOff.errMsg",  NO_DIR, CHAR_VARIABLE, 256);
      
      //Notification for WfsOff
      Notify(MagAOI_cmds_WfsOff_command, AOI_WfsOff_command_changed);


      //---- AdsecOn ----//
      MagAOI_cmds_AdsecOn_command = RTDBvar("MagAOI.cmds", "AdsecOn",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AdsecOn_command_tmout = RTDBvar("MagAOI.cmds", "AdsecOn.tmout",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AdsecOn_stat    = RTDBvar("MagAOI.cmds", "AdsecOn.stat",    NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AdsecOn_errMsg  = RTDBvar("MagAOI.cmds", "AdsecOn.errMsg",  NO_DIR, CHAR_VARIABLE, 256);
      
      //Notification for AdsecOn
      Notify(MagAOI_cmds_AdsecOn_command, AOI_AdsecOn_command_changed);

      //---- AdsecOff ----//
      MagAOI_cmds_AdsecOff_command = RTDBvar("MagAOI.cmds", "AdsecOff",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AdsecOff_command_tmout = RTDBvar("MagAOI.cmds", "AdsecOff.tmout",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AdsecOff_stat    = RTDBvar("MagAOI.cmds", "AdsecOff.stat",    NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AdsecOff_errMsg  = RTDBvar("MagAOI.cmds", "AdsecOff.errMsg",  NO_DIR, CHAR_VARIABLE, 256);
      
      //Notification for AdsecOff
      Notify(MagAOI_cmds_AdsecOff_command, AOI_AdsecOff_command_changed);

      //---- AdsecSet ----//
      MagAOI_cmds_AdsecSet_command = RTDBvar("MagAOI.cmds", "AdsecSet",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AdsecSet_command_tmout = RTDBvar("MagAOI.cmds", "AdsecSet.tmout",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AdsecSet_stat    = RTDBvar("MagAOI.cmds", "AdsecSet.stat",    NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AdsecSet_errMsg  = RTDBvar("MagAOI.cmds", "AdsecSet.errMsg",  NO_DIR, CHAR_VARIABLE, 256);
      
      //Notification for AdsecSet
      Notify(MagAOI_cmds_AdsecSet_command, AOI_AdsecSet_command_changed);

      //---- AdsecRest ----//
      MagAOI_cmds_AdsecRest_command = RTDBvar("MagAOI.cmds", "AdsecRest",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AdsecRest_command_tmout = RTDBvar("MagAOI.cmds", "AdsecRest.tmout",         NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AdsecRest_stat    = RTDBvar("MagAOI.cmds", "AdsecRest.stat",    NO_DIR, INT_VARIABLE, 1);
      MagAOI_cmds_AdsecRest_errMsg  = RTDBvar("MagAOI.cmds", "AdsecRest.errMsg",  NO_DIR, CHAR_VARIABLE, 256);
      
      //Notification for AdsecRest
      Notify(MagAOI_cmds_AdsecRest_command, AOI_AdsecRest_command_changed);

       //Notification for AdsecRest
      Notify(MagAOI_cmds_AdsecRest_command, AOI_AdsecRest_command_changed);



   }
   catch (AOVarException &e)
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%i: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating AOI Command RTDB variables");
   }
   
   try 
   {

      var_LoopStat = RTDBvar("wfsarb.L", "FSM_STATE@M_MAGWFS", NO_DIR, CHAR_VARIABLE, 50);

      var_dateobs_cur = RTDBvar("AOS.TEL", "DATEOBS",  NO_DIR, CHAR_VARIABLE, 10);
      var_telut_cur = RTDBvar("AOS.TEL",  "UT", NO_DIR, INT_VARIABLE, 1);
      var_telst_cur = RTDBvar("AOS.TEL",  "ST", NO_DIR, INT_VARIABLE, 1);

      var_telRA_cur  = RTDBvar("AOS.TEL",  "RA", NO_DIR, REAL_VARIABLE, 1);
      var_telDec_cur = RTDBvar("AOS.TEL", "DEC", NO_DIR, REAL_VARIABLE, 1);
      var_telEpoch_cur = RTDBvar("AOS.TEL", "EPOCH",  NO_DIR, REAL_VARIABLE, 1);
      var_telHA_cur = RTDBvar("AOS.TEL", "HA",  NO_DIR, REAL_VARIABLE, 1);
      var_telAM_cur = RTDBvar("AOS.TEL", "AM",  NO_DIR, REAL_VARIABLE, 1);
      var_telRotOff_cur  = RTDBvar("AOS.L.ROTATOR", "OFFSET",  NO_DIR, REAL_VARIABLE, 1);

      var_telROI_cur = RTDBvar("AOS.L.ROTATOR", "ROI",  NO_DIR, INT_VARIABLE, 1);
      var_telTracking = RTDBvar("AOS.TEL", "ISTRACKING",  NO_DIR, INT_VARIABLE, 1);
      var_telGuiding = RTDBvar("AOS.TEL", "ISGUIDING",  NO_DIR, INT_VARIABLE, 1);
      var_telSlewing = RTDBvar("AOS.TEL", "ISSLEWING",  NO_DIR, INT_VARIABLE, 1);
      var_telGuiderMoving = RTDBvar("AOS.TEL", "GUIDER.ISMOVING",  NO_DIR, INT_VARIABLE, 1);
      var_telAz_cur  = RTDBvar("AOS.TEL", "AZ",  NO_DIR, REAL_VARIABLE, 1);      
      var_telEl_cur  = RTDBvar("AOS.TEL",  "EL", NO_DIR, REAL_VARIABLE, 1);
      var_telZd_cur  = RTDBvar("AOS.TEL",  "ZD", NO_DIR, REAL_VARIABLE, 1);
      var_telPA_cur = RTDBvar("AOS.TEL", "PA",  NO_DIR, REAL_VARIABLE, 1);
      var_telDomeAz_cur = RTDBvar("AOS.TEL", "DOME.AZ",  NO_DIR, REAL_VARIABLE, 1);
      var_telDomeStat_cur = RTDBvar("AOS.TEL", "DOME.STAT",  NO_DIR, INT_VARIABLE, 1);

      var_catra = RTDBvar("AOS.TEL", "CATRA",  NO_DIR, REAL_VARIABLE, 1);
      var_catdec = RTDBvar("AOS.TEL", "CATDEC",  NO_DIR, REAL_VARIABLE, 1);
      var_catep = RTDBvar("AOS.TEL", "CATEP",  NO_DIR, REAL_VARIABLE, 1);
      var_catro = RTDBvar("AOS.TEL", "CATRO",  NO_DIR, REAL_VARIABLE, 1);
      var_catrm = RTDBvar("AOS.TEL", "CATRM",  NO_DIR, CHAR_VARIABLE, 5);
      var_catobj = RTDBvar("AOS.TEL", "CATOBJ",  NO_DIR, CHAR_VARIABLE, 50);

      var_obsinst = RTDBvar("AOS.TEL", "OBSINST", NO_DIR, CHAR_VARIABLE, 50);
      var_obsname = RTDBvar("AOS.TEL", "OBSNAME", NO_DIR, CHAR_VARIABLE, 50);

      var_telHex_AbsPos = RTDBvar("AOS.L.HEXAPOD", "ABS_POS",  NO_DIR, REAL_VARIABLE, 6);
      var_telSec_SetPos = RTDBvar("AOS.TEL", "VE.SET",  NO_DIR, REAL_VARIABLE, 6);
      var_telSec_EncPos = RTDBvar("AOS.TEL", "VE.ENC",  NO_DIR, REAL_VARIABLE, 6);

      var_telRotAng_cur = RTDBvar("AOS.L.ROTATOR", "ANGLE",  NO_DIR, REAL_VARIABLE, 1);
      var_rotFollowing = RTDBvar("AOS.L.ROTATOR", "FOLLOWING",  NO_DIR, INT_VARIABLE, 1);

      var_wxtemp = RTDBvar("AOS.TEL", "WXTEMP",  NO_DIR, REAL_VARIABLE, 1);
      var_wxpres = RTDBvar("AOS.TEL", "WXPRES",  NO_DIR, REAL_VARIABLE, 1);
      var_wxhumid = RTDBvar("AOS.TEL", "WXHUMID",  NO_DIR, REAL_VARIABLE, 1);
      var_wxwind = RTDBvar("AOS.TEL", "WXWIND",  NO_DIR, REAL_VARIABLE, 1);
      var_wxwdir = RTDBvar("AOS.TEL", "WXWDIR",  NO_DIR, REAL_VARIABLE, 1);
      var_ttruss = RTDBvar("AOS.TEL", "TTRUSS",  NO_DIR, REAL_VARIABLE, 1);
      var_tcell = RTDBvar("AOS.TEL", "TCELL",  NO_DIR, REAL_VARIABLE, 1);
      var_tseccell = RTDBvar("AOS.TEL", "TSECCELL",  NO_DIR, REAL_VARIABLE, 1);
      var_tambient = RTDBvar("AOS.TEL", "TAMBIENT",  NO_DIR, REAL_VARIABLE, 1);

      var_wxdewpoint = RTDBvar("AOS.TEL", "WXDEWPOINT",  NO_DIR, REAL_VARIABLE, 1);
      var_wxpwvest = RTDBvar("AOS.TEL", "WXPWVEST",  NO_DIR, REAL_VARIABLE, 1);


      var_shellwind = RTDBvar("AOS", "SHELL.WINDSPEED", NO_DIR, REAL_VARIABLE, 1);
      var_ambwind = RTDBvar("AOS", "AMB.WINDSPEED", NO_DIR, REAL_VARIABLE, 1);

      var_dimmfwhm = RTDBvar("AOS", "DIMM.FWHM", NO_DIR, REAL_VARIABLE, 1);
      var_dimmtime = RTDBvar("AOS", "DIMM.TIME", NO_DIR, INT_VARIABLE, 1);

      var_mag1fwhm = RTDBvar("AOS", "MAG1.FWHM", NO_DIR, REAL_VARIABLE, 1);
      var_mag1time = RTDBvar("AOS", "MAG1.TIME", NO_DIR, INT_VARIABLE, 1);

      var_mag2fwhm = RTDBvar("AOS", "MAG2.FWHM", NO_DIR, REAL_VARIABLE, 1);
      var_mag2time = RTDBvar("AOS", "MAG2.TIME", NO_DIR, INT_VARIABLE, 1);


      var_lbtswa = RTDBvar("AOS.L", "SWA.DEPLOYED",  NO_DIR, INT_VARIABLE, 1);
   

      var_M1_coeffs = RTDBvar("AOS.M1", "COEF",  REQ_VAR, REAL_VARIABLE, 13);


      //Offloading
      var_focus_off_cum = RTDBvar("AOS.OFFLOAD" ,"FOCUS_CUM", NO_DIR, REAL_VARIABLE, 1);
      var_focus_off_cum_reset = RTDBvar("AOS.OFFLOAD" ,"FOCUS_CUM_RESET", REQ_VAR, INT_VARIABLE, 1);

   
      var_focus_off_last = RTDBvar("AOS.OFFLOAD" ,"FOCUS_LAST", NO_DIR, REAL_VARIABLE, 1);
      var_focus_off_time = RTDBvar("AOS.OFFLOAD" ,"FOCUS_TIME", NO_DIR, INT_VARIABLE, 1);

      var_coma_off_enable_cur = RTDBvar("AOS.OFFLOAD" ,"COMA_ENABLED", CUR_VAR, REAL_VARIABLE, 1);
      var_coma_off_enable_req = RTDBvar("AOS.OFFLOAD" ,"COMA_ENABLED", REQ_VAR, REAL_VARIABLE, 1);

      var_coma_off_cum_x = RTDBvar("AOS.OFFLOAD" ,"COMA_CUM_x", NO_DIR, REAL_VARIABLE, 1);
      var_coma_off_cum_y = RTDBvar("AOS.OFFLOAD" ,"COMA_CUM_y", NO_DIR, REAL_VARIABLE, 1);
      var_coma_off_cum_v = RTDBvar("AOS.OFFLOAD" ,"COMA_CUM_v", NO_DIR, REAL_VARIABLE, 1);
      var_coma_off_cum_h = RTDBvar("AOS.OFFLOAD" ,"COMA_CUM_h", NO_DIR, REAL_VARIABLE, 1);
      var_coma_off_cum_reset = RTDBvar("AOS.OFFLOAD" ,"COMA_CUM_RESET", REQ_VAR, INT_VARIABLE, 1);


      Notify(var_focus_off_cum_reset, focus_cum_reset_req);
      Notify(var_coma_off_cum_reset, coma_cum_reset_req);

      var_offcollim = RTDBvar("AOS.OFFLOAD", "COLLIMATE", NO_DIR, INT_VARIABLE, 1);

      Notify(var_offcollim, offcollim_changed);

      #ifdef OFFLOAD_TESTING
      var_testoffload_tt = RTDBvar("AOS.OFFLOAD" ,"TEST_TT", NO_DIR, INT_VARIABLE, 1);
      var_testoffload_foc = RTDBvar("AOS.OFFLOAD" ,"TEST_FOC", NO_DIR, INT_VARIABLE, 1);
      var_testoffload_com = RTDBvar("AOS.OFFLOAD" ,"TEST_COMA", NO_DIR, INT_VARIABLE, 1);
      var_testoffload_LO = RTDBvar("AOS.OFFLOAD" ,"TEST_LO", NO_DIR, INT_VARIABLE, 1);
      var_testoffload_astig = RTDBvar("AOS.OFFLOAD" ,"TEST_ASTIG", NO_DIR, INT_VARIABLE, 1);
      var_testoffload_ALL = RTDBvar("AOS.OFFLOAD" ,"TEST_ALL", NO_DIR, INT_VARIABLE, 1);
             
      var_testoff_ttX = RTDBvar("AOS.OFFLOAD" ,"TT_X", NO_DIR, REAL_VARIABLE, 1);
      var_testoff_ttY = RTDBvar("AOS.OFFLOAD" ,"TT_Y", NO_DIR, REAL_VARIABLE, 1);
      var_testoff_foc = RTDBvar("AOS.OFFLOAD" ,"FOC", NO_DIR, REAL_VARIABLE, 1);
      var_testoff_comaX = RTDBvar("AOS.OFFLOAD" ,"COMA_X", NO_DIR, REAL_VARIABLE, 1);
      var_testoff_comaY = RTDBvar("AOS.OFFLOAD" ,"COMA_Y", NO_DIR, REAL_VARIABLE, 1);
      var_testoff_astigX = RTDBvar("AOS.OFFLOAD" ,"ASTIG_X", NO_DIR, REAL_VARIABLE, 1);
      var_testoff_astigY = RTDBvar("AOS.OFFLOAD" ,"ASTIG_Y", NO_DIR, REAL_VARIABLE, 1);


      Notify(var_testoffload_tt, testoffload_tt_changed);
      Notify(var_testoffload_foc, testoffload_foc_changed);
      Notify(var_testoffload_com, testoffload_com_changed);
      Notify(var_testoffload_LO, testoffload_LO_changed);
      Notify(var_testoffload_astig, testoffload_astig_changed);
      Notify(var_testoffload_ALL, testoffload_ALL_changed);
      
      #endif
   } 
   catch (AOVarException &e)  
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%i: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating AOI AOS RTDB variables");
   }

}

void MagAOI::PostInit()
{
   _aoArb = "AOARB.L";
    arbIntf=new arb::ArbitratorInterface(_aoArb);
}

void MagAOI::Run()
{
   _logger->log( Logger::LOG_LEV_INFO, "Running...");

   init_tmouts();
   clear_commands(true);
   initialize_offload_cum();
   
   while(!TimeToDie()) 
   {
      try 
      {
         DoFSM();
      } 
      catch (AOException &e) 
      {
         _logger->log( Logger::LOG_LEV_ERROR, "Caught exception (at %s:%d): %s", __FILE__, __LINE__, e.what().c_str());
         
         // When the exception is thrown, the mutex was held!
         pthread_mutex_unlock(&aoiMutex);
         usleep( (unsigned int)(TCSpoll_interval * 1e6));
      }
   }
}//void MagAOI::Run()
      
    
int MagAOI::DoFSM()
{
   int status;

   status = getCurState();
   
   // Always check the network
   if (status != STATE_NOCONNECTION)
   {
      if(TestNetwork() != NO_ERROR)
      {
         setCurState(STATE_NOCONNECTION);
         status = STATE_NOCONNECTION;
      }
      else
      {
         setCurState(STATE_OPERATING);
      }
   }
   
   switch(status)
   {
      case STATE_NOCONNECTION:
         if (SetupNetwork() == NO_ERROR)
         {
            if (TestNetwork() == NO_ERROR)
            {
               setCurState(STATE_CONNECTED);
            }
            else
            {
              ShutdownNetwork();
              setCurState(STATE_NOCONNECTION);
              status = STATE_NOCONNECTION;
            }
         }
         break;

      case STATE_OPERATING:
         
         updateMagTelStatus();
         
         break;
   }

   // Always set current status (for external watchdog)
   setCurState( getCurState());

   usleep( (unsigned int)(TCSpoll_interval * 1e6));

   return NO_ERROR;

}// int MagAOI::DoFSM()

/****************** Arbitrator Interface ****************************/

void MagAOI::init_tmouts()
{
   MagAOI_cmds_PresetFlat_command_tmout.Set(PresetFlat_tmout, 0, FORCE_SEND);
   MagAOI_cmds_PresetAO_command_tmout.Set(PresetAO_tmout, 0, FORCE_SEND);
   MagAOI_cmds_AcquireFromGuider_tmout.Set(AcqGuider_tmout, 0, FORCE_SEND);
   MagAOI_cmds_PresetVisAO_command_tmout.Set(PresetVisAO_tmout, 0, FORCE_SEND);
   MagAOI_cmds_AcquireRefAO_command_tmout.Set(AcquireRefAO_tmout, 0, FORCE_SEND);
   MagAOI_cmds_CheckRefAO_command_tmout.Set(CheckRefAO_tmout, 0, FORCE_SEND);
   MagAOI_cmds_RefineAO_command_tmout.Set(RefineAO_tmout, 0, FORCE_SEND);
   MagAOI_cmds_ModifyAO_command_tmout.Set(ModifyAO_tmout, 0, FORCE_SEND);
   MagAOI_cmds_StartAO_command_tmout.Set(StartAO_tmout, 0, FORCE_SEND);
   MagAOI_cmds_Stop_command_tmout.Set(Stop_tmout, 0, FORCE_SEND);
   MagAOI_cmds_Pause_command_tmout.Set(Pause_tmout, 0, FORCE_SEND);
   MagAOI_cmds_Resume_command_tmout.Set(Resume_tmout, 0, FORCE_SEND);
   MagAOI_cmds_Resume_command_tmout.Set(UserPanic_tmout, 0, FORCE_SEND);
   MagAOI_cmds_Resume_command_tmout.Set(SetNewInstrument_tmout, 0, FORCE_SEND);
   MagAOI_cmds_WfsOn_command_tmout.Set(WfsOn_tmout, 0, FORCE_SEND);
   MagAOI_cmds_WfsOff_command_tmout.Set(WfsOff_tmout, 0, FORCE_SEND);
   MagAOI_cmds_AdsecOn_command_tmout.Set(AdsecOn_tmout, 0, FORCE_SEND);
   MagAOI_cmds_AdsecOff_command_tmout.Set(AdsecOff_tmout, 0, FORCE_SEND);
   MagAOI_cmds_AdsecSet_command_tmout.Set(AdsecSet_tmout, 0, FORCE_SEND);
   MagAOI_cmds_AdsecRest_command_tmout.Set(AdsecRest_tmout, 0, FORCE_SEND);
}//void MagAOI::init_tmouts()


void MagAOI::clear_commands(bool force)
{
   std::string errMsg;

   send_actions send;
   
   if(force) send = FORCE_SEND;
   else send = CHECK_SEND;


   MagAOI_cmds_PresetFlat_errMsg.Set("", send);
   MagAOI_cmds_PresetFlat_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_PresetVisAO_errMsg.Set("", send);
   MagAOI_cmds_PresetVisAO_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_PresetAO_errMsg.Set("", send);
   MagAOI_cmds_PresetAO_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_AcquireFromGuider_errMsg.Set("", send);
   MagAOI_cmds_AcquireFromGuider_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_NudgeXY_errMsg.Set("", send);
   MagAOI_cmds_NudgeXY_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_NudgeAE_errMsg.Set("", send);
   MagAOI_cmds_NudgeAE_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_AcquireRefAO_errMsg.Set("", send);
   MagAOI_cmds_AcquireRefAO_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_CheckRefAO_errMsg.Set("", send);
   MagAOI_cmds_CheckRefAO_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_RefineAO_errMsg.Set("", send);
   MagAOI_cmds_RefineAO_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_ModifyAO_errMsg.Set("", send);
   MagAOI_cmds_ModifyAO_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_StartAO_errMsg.Set("", send);
   MagAOI_cmds_StartAO_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_OffsetXY_errMsg.Set("", send);
   MagAOI_cmds_OffsetXY_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_OffsetZ_errMsg.Set("", send);
   MagAOI_cmds_OffsetZ_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_NodRaDec_errMsg.Set("", send);
   MagAOI_cmds_NodRaDec_stat.Set(CMD_DEFAULT, 0, send);


   MagAOI_cmds_Stop_errMsg.Set("", send);
   MagAOI_cmds_Stop_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_Pause_errMsg.Set("", send);
   MagAOI_cmds_Pause_stat.Set(CMD_DEFAULT, 0, send);
   
   MagAOI_cmds_Resume_errMsg.Set("", send);
   MagAOI_cmds_Resume_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_UserPanic_errMsg.Set("", send);
   MagAOI_cmds_UserPanic_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_SetNewInstrument_errMsg.Set("", send);
   MagAOI_cmds_SetNewInstrument_stat.Set(CMD_DEFAULT, 0, send);
   
   MagAOI_cmds_WfsOn_errMsg.Set("", send);
   MagAOI_cmds_WfsOn_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_WfsOff_errMsg.Set("", send);
   MagAOI_cmds_WfsOff_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_AdsecOn_errMsg.Set("", send);
   MagAOI_cmds_AdsecOn_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_AdsecOff_errMsg.Set("", send);
   MagAOI_cmds_AdsecOff_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_AdsecSet_errMsg.Set("", send);
   MagAOI_cmds_AdsecSet_stat.Set(CMD_DEFAULT, 0, send);

   MagAOI_cmds_AdsecRest_errMsg.Set("", send);
   MagAOI_cmds_AdsecRest_stat.Set(CMD_DEFAULT, 0, send);

}//void MagAOI::clear_commands(bool force)

void MagAOI::update_loopon()
{
   var_LoopStat.Update();

   if(var_LoopStat.Get() == "LoopClosed")
   {
      loopon = 1;
   }
   else loopon = 0;

   //std::cout << loopon << std::endl;

   //AOARB_loopon.Update();
   //AOARB_loopon.Get(&loopon);
}

int MagAOI::AOI_clear_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;

   magaoi->clear_commands();

   #ifdef AOI_DEBUG
   std::cout << "Received clear commands:\n";
   #endif

   return 0;

}

int MagAOI::AOI_PresetFlat_command_changed(void *pt, Variable *msgb  __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   magaoi->MagAOI_cmds_PresetFlat_arg_flatSpec.Update();
   std::string flatSpec = magaoi->MagAOI_cmds_PresetFlat_arg_flatSpec.Get();
   
   #ifdef AOI_DEBUG
   std::cout << "Received PresetFlat:\n";
   std::cout << "   flatSpec:   " << flatSpec << "\n";
   #endif

   rv =  magaoi->PresetFlat(flatSpec);
   
   return rv;

}//int MagAOI::AOI_PresetFlat_command_changed(void *pt, Variable *msgb  __attribute__((unused)))
   
int MagAOI::PresetFlat(string flatSpec)
{
   int stat, isSuccess;
   std::string errMsg;

   clear_commands();
   MagAOI_cmds_PresetFlat_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::Command* scmd, * scmdR __attribute__((unused));
   
   std::ostringstream o;
   o << "Received PresetFlat flatSpec: " << flatSpec;
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());
   
   try 
   {
      arb::presetFlatParams params;
      params.flatSpec=flatSpec;
      scmd = new arb::PresetFlat(PresetFlat_tmout ,params);
      
      #ifndef AOI_NOARB
      scmdR = arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      sleep(10);
      isSuccess = 1;
      #endif
      
      if(isSuccess)
      {
         stat = CMD_SUCCESS;
         errMsg = "PresetFlat completed successfully";
         Logger::get()->log(Logger::LOG_LEV_INFO, "PresetFlat completed successfully.");
      }
      else
      {
         stat = CMD_FAILURE;
         #ifndef AOI_NOARB
         errMsg = scmdR->getErrorString();
         #else
         errMsg = "unkown error.";
         #endif
      }
   } 
   catch(ArbitratorInterfaceException& e) 
   {
      stat = CMD_FAILURE;
      errMsg = e.what(Terse);
   } 
    
   delete scmd;

   MagAOI_cmds_PresetFlat_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_PresetFlat_stat.Set(stat,0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing PresetFlat: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }

   return stat;
}//int MagAOI::PresetFlat(string flatSpec)


int MagAOI::AOI_AcquireFromGuider_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   std::string AOmode;
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;

   magaoi->MagAOI_cmds_PresetAO_arg_AOmode.Update();
   
   AOmode = magaoi->MagAOI_cmds_PresetAO_arg_AOmode.Get();

   #ifdef AOI_DEBUG
   std::cout << "Received AcquireFromGuider.\n";   
   std::cout << "   AOMode:   " << AOmode << "\n";
   #endif

   rv = magaoi->AcquireFromGuider(AOmode);
   
   return rv;
}
      
int MagAOI::AcquireFromGuider(std::string ClioFOV)
{
   int stat;
   std::string errMsg;
   double az, el, az0, el0;
   double q;

   char tcscomstr[256];

   clear_commands();
   MagAOI_cmds_AcquireFromGuider_stat.Set(CMD_RUNNING, 0, FORCE_SEND);

   //First check that loop is open
   update_loopon(); //Update loop status DD

   //std::cout << loopon << " " << LOOP_ON << "\n";
   if(loopon == LOOP_ON)      // Check the loop status
   {
      MagAOI_cmds_AcquireFromGuider_stat.Set(CMD_FAILURE, 0, FORCE_SEND);
      MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND);
      return -1; //Can't do this if loop is not open
   }


   //default is wide_full
   az0 = guider_acq_wide_full_az;
   el0 = guider_acq_wide_full_el;

   if(ClioFOV ==  "wide_strip")
   {
      az0 = guider_acq_wide_strip_az;
      el0 = guider_acq_wide_strip_el;
   }
   else if(ClioFOV ==  "wide_stamp")
   {
      az0 = guider_acq_wide_stamp_az;
      el0 = guider_acq_wide_stamp_el;
   }
   else if(ClioFOV ==  "wide_substamp")
   {
      az0 = guider_acq_wide_substamp_az;
      el0 = guider_acq_wide_substamp_el;
   }
   else if(ClioFOV ==  "narrow_full")
   {
      az0 = guider_acq_narrow_full_az;
      el0 = guider_acq_narrow_full_el;
   }
   else if(ClioFOV ==  "narrow_strip")
   {
      az0 = guider_acq_narrow_strip_az;
      el0 = guider_acq_narrow_strip_el;
   }
   else if(ClioFOV ==  "narrow_stamp")
   {
      az0 = guider_acq_narrow_stamp_az;
      el0 = guider_acq_narrow_stamp_el;
   }
   else if(ClioFOV ==  "narrow_substamp")
   {
      az0 = guider_acq_narrow_substamp_az;
      el0 = guider_acq_narrow_substamp_el;
   }


   //Convert current telescope rotator angle to radians
   q = (_telRotEncAng)*3.14159/180.;

   //The rotation matrix
   az = az0*cos(q) - el0*sin(q);
   el = az0*sin(q) + el0*cos(q);


   std::ostringstream t2;
   t2 << "[ACQ] Sending AZ/EL acquisition offset " << az << "," << el;
   Logger::get()->log(Logger::LOG_LEV_INFO, t2.str());

   snprintf(tcscomstr, 256, "aeg %0.1f %0.1f", az, el);      

   int tcsrv = sendMagTelCommand(tcscomstr, 1000);
   
   std::cout << "----\nacq: " << tcscomstr << "\n";

   if(tcsrv != 0)
   {
      std::ostringstream err;
      err << "[ACQ] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      errMsg = "AcquireFromGuider: failed -- error form TCS.";
      stat = CMD_FAILURE;

   }
   else
   {
      Logger::get()->log(Logger::LOG_LEV_INFO, "[ACQ] TCS returned 0.");
      errMsg = "AcquireFromGuider: success";
      stat = CMD_SUCCESS;
   }


   MagAOI_cmds_AcquireFromGuider_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_AcquireFromGuider_errMsg.Set(errMsg, FORCE_SEND);

   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND);

   return stat;
}


// RTDB handler for a AcquireFromGuider command via MsgD.
int MagAOI::AOI_NudgeXY_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   double x, y;
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;

   magaoi->MagAOI_cmds_NudgeXY_arg_x.Update();
   magaoi->MagAOI_cmds_NudgeXY_arg_y.Update();

   magaoi->MagAOI_cmds_NudgeXY_arg_x.Get(&x);
   magaoi->MagAOI_cmds_NudgeXY_arg_y.Get(&y);

   #ifdef AOI_DEBUG
   std::cout << "Received NudgeXY.\n";
   std::cout << "   x:   " << x << "\n";
   std::cout << "   y:   " << y << "\n";
   #endif

   rv = magaoi->NudgeXY(x, y);
   
   return rv;

}
      
//The AOI Gui AcquireFromGuider command
int MagAOI::NudgeXY(double x, double y)
{
   int stat;
   std::string errMsg;
   double az, el;
   double q;

   char tcscomstr[256];

   clear_commands();
   MagAOI_cmds_NudgeXY_stat.Set(CMD_RUNNING, 0, FORCE_SEND);

   //defaults
   errMsg = "NudgeXY: success";
   stat = CMD_SUCCESS;

   //First check that loop is open
   update_loopon(); //Update loop status DD

   //std::cout << loopon << " " << LOOP_ON << "\n";
   if(loopon == LOOP_ON)      // Check the loop status
   {
      MagAOI_cmds_NudgeXY_stat.Set(CMD_FAILURE, 0, FORCE_SEND);
      MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND);

      std::ostringstream err;
      err << "[NUD] can not nudge unless loop open";
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      errMsg = "NudgeXY: can not nudge unless loop open";
      stat = -1;

      MagAOI_cmds_NudgeXY_stat.Set(stat, 0, FORCE_SEND);
      MagAOI_cmds_NudgeXY_errMsg.Set(errMsg, FORCE_SEND);

      MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND);

      return -1; //Can't do this if loop is not open
   }

   //Convert current telescope rotator angle to radians
   q = (_telRotOff)*3.14159/180.;

   //The rotation matrix
   x*=-1;
   el = x*cos(q) - y*sin(q);
   az = x*sin(q) + y*cos(q);

   snprintf(tcscomstr, 256 , "ofra %f", az);


   int tcsrv = sendMagTelCommand(tcscomstr, 10000);
   if(tcsrv != 0)
   {
      std::ostringstream err;
      err << "[NUD] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      errMsg = "NudgeXY: ofra failed -- error form TCS.";
      stat = -1;
   }

   snprintf(tcscomstr, 256 , "ofdc %f", el);
   tcsrv = sendMagTelCommand(tcscomstr, 10000);
   if(tcsrv != 0)
   {
      std::ostringstream err;
      err << "[NUD]  TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      errMsg = "NudgeXY: ofdc failed -- error form TCS.";
      stat = -1;
   }


   snprintf(tcscomstr, 256 , "offp ");
   tcsrv = sendMagTelCommand(tcscomstr, 10000);
   if(tcsrv != 0)
   {
      std::ostringstream err;
      err << "[NUD] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      errMsg = "NudgeXY: ofdc failed -- error form TCS.";
      stat = -1;

   }


//    std::ostringstream t2;
//    t2 << "[NUD] Sending AZ/EL nudge offset " << az << "," << el;
//    Logger::get()->log(Logger::LOG_LEV_INFO, t2.str());
// 
//    snprintf(tcscomstr, 256, "aeg %0.1f %0.1f", az, el);      
// 
//    int tcsrv = sendMagTelCommand(tcscomstr, 1000);
//    
//    if(tcsrv != 0)
//    {
//       std::ostringstream err;
//       err << "[NUD] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
//       Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
//       errMsg = "NudgeXY: failed -- error form TCS.";
//       stat = CMD_FAILURE;
// 
//    }
//    else
//    {
//       Logger::get()->log(Logger::LOG_LEV_INFO, "[NUD] TCS returned 0.");
//       errMsg = "NudgeXY: success";
//       stat = CMD_SUCCESS;
//    }
// 

   MagAOI_cmds_NudgeXY_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_NudgeXY_errMsg.Set(errMsg, FORCE_SEND);

   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND);

   return stat;

}


// RTDB handler for a AcquireFromGuider command via MsgD.
int MagAOI::AOI_NudgeAE_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   double a, e;
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;

   magaoi->MagAOI_cmds_NudgeAE_arg_a.Update();
   magaoi->MagAOI_cmds_NudgeAE_arg_e.Update();

   magaoi->MagAOI_cmds_NudgeAE_arg_a.Get(&a);
   magaoi->MagAOI_cmds_NudgeAE_arg_e.Get(&e);

   #ifdef AOI_DEBUG
   std::cout << "Received NudgeAE.\n";
   std::cout << "   a:   " << a << "\n";
   std::cout << "   e:   " << e << "\n";
   #endif

   rv = magaoi->NudgeAE(a, e);
   
   return rv;

}
      
//The AOI Gui AcquireFromGuider command
int MagAOI::NudgeAE(double a, double e)
{
   int stat;
   std::string errMsg;

   char tcscomstr[256];

   clear_commands();
   MagAOI_cmds_NudgeAE_stat.Set(CMD_RUNNING, 0, FORCE_SEND);

   //defaults
   errMsg = "NudgeAE: success";
   stat = CMD_SUCCESS;

   //First check that loop is open
   update_loopon(); //Update loop status DD

   //std::cout << loopon << " " << LOOP_ON << "\n";
   if(loopon == LOOP_ON)      // Check the loop status
   {
      MagAOI_cmds_NudgeAE_stat.Set(CMD_FAILURE, 0, FORCE_SEND);
      MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND);

      std::ostringstream err;
      err << "[NUD] can not nudge unless loop open";
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      errMsg = "NudgeAE: can not nudge unless loop open";
      stat = -1;

      MagAOI_cmds_NudgeAE_stat.Set(stat, 0, FORCE_SEND);
      MagAOI_cmds_NudgeAE_errMsg.Set(errMsg, FORCE_SEND);

      MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND);

      return -1; //Can't do this if loop is not open
   }

   snprintf(tcscomstr, 256 , "aeg %f %f", a, e);

   int tcsrv = sendMagTelCommand(tcscomstr, 10000);
   if(tcsrv != 0)
   {
      std::ostringstream err;
      err << "[NUD] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      errMsg = "NudgeAE: aeg failed -- error form TCS.";
      stat = -1;
   }

   MagAOI_cmds_NudgeAE_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_NudgeAE_errMsg.Set(errMsg, FORCE_SEND);

   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND);

   return stat;

}




int MagAOI::AOI_PresetVisAO_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{

   MagAOI * magaoi = (MagAOI *) pt;
   int rv;

   #ifdef AOI_DEBUG
   std::cout << "Received PresetVisAO.\n";
  #endif

   
   rv = magaoi->PresetVisAO();
   
   return rv;
   
}//int MagAOI::AOI_PresetVisAO_command_changed(void *pt, Variable *msgb __attribute__((unused)))

int MagAOI::PresetVisAO()
{
   int stat;
   std::string errMsg;
   clear_commands();

   MagAOI_cmds_PresetVisAO_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND);

   std::ostringstream o;
   o << "Received PresetVisAO.";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   RTDBvar master_remote, gimbal_center, fw3_open, fw3_open_cur, ccd47_preset, ccd47_state;
   double fw3_curpos;
   int ccd47_status;

   try
   {
      master_remote = RTDBvar("visaoiclient.L", "Master.Remote.REQ@M_MAGWFS", NO_DIR, INT_VARIABLE, 1,0);
      gimbal_center = RTDBvar("gimbal.L", "center.REQ@M_MAGWFS", NO_DIR, REAL_VARIABLE, 1,0);
      //fw2_open = RTDBvar("filterwheel2.L", "POS.REQ@M_MAGWFS", NO_DIR, REAL_VARIABLE, 1,0);
      fw3_open = RTDBvar("filterwheel3.L", "POS.REQ@M_MAGWFS", NO_DIR, REAL_VARIABLE, 1,0);
      fw3_open_cur = RTDBvar("filterwheel3.L", "POS.CUR@M_MAGWFS", NO_DIR, REAL_VARIABLE, 1,0);
      ccd47_preset = RTDBvar("ccd47.L", "Preset.REQ@M_MAGWFS", NO_DIR, INT_VARIABLE, 1,0);
      ccd47_state = RTDBvar("ccd47.L", "STATUS@M_MAGWFS", NO_DIR, INT_VARIABLE, 1,0);
   }
   catch(...)
   {
      std::cerr << "Exception Caught creating RTDBvars in PresetVisAO.\n";
   }
   master_remote.Set(10, 0, FORCE_SEND);
   sleep(1);
   ccd47_preset.Set(1, 0, FORCE_SEND);
   fw3_open.Set(presetvisao_fw3_pos , 0, FORCE_SEND);
   sleep(1);

   fw3_open_cur.Update();
   fw3_open_cur.Get(&fw3_curpos);
   
   while(fabs(fw3_curpos - presetvisao_fw3_pos) > .1)
   {
      sleep(1);
      fw3_open_cur.Update();
      fw3_open_cur.Get(&fw3_curpos);
   }
   //gimbal_center.Set(1.0, 0, FORCE_SEND);

   ccd47_state.Update();
   ccd47_state.Get(&ccd47_status);

   while(!(ccd47_status == STATE_OPERATING || ccd47_status == STATE_READY))
   {
      sleep(1);
      ccd47_state.Update();
      ccd47_state.Get(&ccd47_status);
   }

   stat = CMD_SUCCESS;

   MagAOI_cmds_PresetVisAO_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_PresetVisAO_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing PresetVisAO: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   return stat;

}//int MagAOI::PresetVisAO()

int MagAOI::AOI_PresetAO_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   std::string AOmode;
   std::string wfsSpec;
   double rocoordx;
   double rocoordy;
   double mag;
   double cindex; //NOTE: We're using cindex to pass bad seeing enabled for now.
   int adcTracking;
/** \todo handle bad seeing with r0 instead of cindex
*/

   magaoi->MagAOI_cmds_PresetAO_arg_AOmode.Update();
   magaoi->MagAOI_cmds_PresetAO_arg_wfsSpec.Update();
   magaoi->MagAOI_cmds_PresetAO_arg_rocoordx.Update();
   magaoi->MagAOI_cmds_PresetAO_arg_rocoordy.Update();
   magaoi->MagAOI_cmds_PresetAO_arg_mag.Update();
   magaoi->MagAOI_cmds_PresetAO_arg_cindex.Update();
   magaoi->MagAOI_cmds_PresetAO_arg_adcTracking.Update();

   AOmode = magaoi->MagAOI_cmds_PresetAO_arg_AOmode.Get();
   wfsSpec = magaoi->MagAOI_cmds_PresetAO_arg_wfsSpec.Get();
   magaoi->MagAOI_cmds_PresetAO_arg_rocoordx.Get(&rocoordx);
   magaoi->MagAOI_cmds_PresetAO_arg_rocoordy.Get(&rocoordy);
   magaoi->MagAOI_cmds_PresetAO_arg_mag.Get(&mag);
   magaoi->MagAOI_cmds_PresetAO_arg_cindex.Get(&cindex);
   magaoi->MagAOI_cmds_PresetAO_arg_adcTracking.Get(&adcTracking);

   #ifdef AOI_DEBUG
   std::cout << "Received PresetAO:\n";
   std::cout << "   AOMode:      " << AOmode << "\n";
   std::cout << "   wfsSpec:     " << wfsSpec << "\n";
   std::cout << "   rocoordx:    " << rocoordx << "\n";
   std::cout << "   rocoordy:    " << rocoordy << "\n";
   std::cout << "   mag:         " << mag << "\n";
   std::cout << "   cindex:      " << cindex << "\n";
   std::cout << "   adcTracking: " << adcTracking << "\n";
   #endif

   
   rv = magaoi->PresetAO(AOmode, wfsSpec, rocoordx, rocoordy, mag, cindex, adcTracking);
   
   return rv;
   
}//int MagAOI::AOI_PresetAO_command_changed(void *pt, Variable *msgb __attribute__((unused)))

int MagAOI::PresetAO(string AOmode, string wfsSpec, double rocoordx, double rocoordy, double mag, double cindex, int adcTracking)
{
   int stat, isSuccess;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_PresetAO_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::PresetAO* scmd, * scmdR __attribute__((unused));
   
   double NaN=Arcetri::NaNutils::dNaN();
   double elev=Arcetri::NaNutils::dNaN();
   double rotang=Arcetri::NaNutils::dNaN();
   double gravang=Arcetri::NaNutils::dNaN();
   double windspeed=Arcetri::NaNutils::dNaN();
   double winddir=Arcetri::NaNutils::dNaN();

   double socoord1,socoord2;

   socoord1=0;
   socoord2=0;

   presetAOParams params;
   params.aoMode=AOmode;
   params.focStation=focalStation;
   params.instr=instrumentName;
   params.wfsSpec=wfsSpec;
   params.soCoord[0]=socoord1;
   params.soCoord[1]=socoord2;
   params.roCoord[0]=rocoordx;
   params.roCoord[1]=rocoordy;
   params.elevation=elev;
   params.rotAngle=rotang;
   params.gravAngle=gravang;
   params.mag=mag;
   params.color=NaN;
   params.r0=cindex;
   params.skyBrgt=NaN;
   params.windSpeed=windspeed;
   params.windDir=winddir;

   params.adcTracking = adcTracking;

   std::ostringstream o;
   o << "Received PresetAO: instr: " << instrumentName << " AOmode: " << AOmode << " wfsSpec: " << wfsSpec << " rocoordx: " << rocoordx;
   o << " rocoordy: " << rocoordy << " mag: " << mag << " bad seeing: " << cindex << " adcTracking: " << adcTracking;
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   
   
   try 
   {
      scmd = new arb::PresetAO(PresetAO_tmout, params);

      #ifndef AOI_NOARB
      scmdR = (arb::PresetAO *)arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      #endif
      
      if(isSuccess)
      {
         stat = CMD_SUCCESS;
         errMsg = "PresetAO completed successfully.";
      }
      else
      {
         stat = CMD_FAILURE;
         #ifndef AOI_NOARB
         errMsg = scmdR->getErrorString();
         #else
         errMsg = "error in PresetAO.";
         #endif
      }
   } 
   catch(ArbitratorInterfaceException& e) 
   {
      stat = CMD_FAILURE;
      errMsg = e.what(Terse);
   }
   
   delete scmd;

   MagAOI_cmds_PresetAO_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_PresetAO_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing PresetAO: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   return stat;
}//int MagAOI::PresetAO(string AOmode, string wfsSpec, double rocoordx, double rocoordy, double mag, double cindex)

int MagAOI::AOI_CheckRefAO_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   double dx;
   double dy;
   double mag;
   std::vector<double> resvec(3);
 
   #ifdef AOI_DEBUG
   std::cout << "Received CheckRefAO:\n";
   #endif
   rv = magaoi->CheckRefAO(&dx, &dy, &mag);
   
   resvec[0] = dx;
   resvec[1] = dy;
   resvec[2] = mag;

   magaoi->MagAOI_cmds_CheckRefAO_result.Set(resvec, FORCE_SEND);
   
   #ifdef AOI_DEBUG
   std::cout << "   dx:  " << dx << "\n";
   std::cout << "   dy:  " << dy << "\n";
   std::cout << "   mag: " << mag << "\n";
   #endif
   
   return rv;
}//int MagAOI::AOI_CheckRefAO_command_changed(void *pt, Variable *msgb __attribute__((unused)))
   
int MagAOI::CheckRefAO(double *dx, double *dy, double *mag)
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_CheckRefAO_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::CheckRefAO* scmd, *scmdR __attribute__((unused));

   std::ostringstream o;
   o << "Received CheckRefAO AOmode.";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());
   
   try 
   {
      scmd = new arb::CheckRefAO(CheckRefAO_tmout);
      
      #ifndef AOI_NOARB
      scmdR = (arb::CheckRefAO *)arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      #endif
      
      if(isSuccess)
      {
         stat = CMD_SUCCESS;
         errMsg = "CheckRefAO completed successfully";

         #ifndef AOI_NOARB
         arb::checkRefAOResult result=scmdR->getResult();
         *dx=result.deltaXY[0];
         *dy=result.deltaXY[1];
         *mag=result.starMag;
         #else
         *dx = 0;
         *dy = 0;
         *mag = 0;
         #endif
      } 
      else 
      {
         stat = CMD_FAILURE;
         #ifndef AOI_NOARB
         errMsg = scmdR->getErrorString();
         #else
         errMsg = "unkown error.";
         #endif
      }
   } 
   catch(ArbitratorInterfaceException& e) 
   {
      stat = CMD_FAILURE;
      errMsg = e.what(Terse);
   }
   
   delete scmd;

   MagAOI_cmds_CheckRefAO_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_CheckRefAO_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing CheckRefAO: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   return stat;
}//int MagAOI::CheckRefAO(double *dx, double *dy, double *mag)

int MagAOI::AOI_AcquireRefAO_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   #ifdef AOI_DEBUG
   std::cout << "Received AcquireRefAO:\n";
   #endif
   rv = magaoi->AcquireRefAO(false);
      
   return rv;
}//int MagAOI::AOI_AcquireRefAO_command_changed(void *pt, Variable *msgb __attribute__((unused)))

int MagAOI::AcquireRefAO(bool rePoint)
{
   int stat, isSuccess;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_AcquireRefAO_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated

   arb::AcquireRefAO *aqscmd, *aqscmdR;

   std::ostringstream o;
   o << "Received AcquireRefAO AOmode.";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   int checkrefOK = true;

   if(rePoint)
   {              // We want the stages not to move

      //not implemented at Magellan yet.
   }
   if(checkrefOK)
   {
      try
      {
         aqscmd = new arb::AcquireRefAO(AcquireRefAO_tmout);

         #ifndef AOI_NOARB
         aqscmdR = (arb::AcquireRefAO *)arbIntf->requestCommand(aqscmd);
         isSuccess = aqscmdR->isSuccess();
         #else
         isSuccess = 1;
         sleep(10);
         #endif
      
         if(isSuccess)
         {
            stat = CMD_SUCCESS;
            errMsg = "AcquireRefAO completed successfully";

            #ifndef AOI_NOARB

            // Get return values
            arb::acquireRefAOResult result=aqscmdR->getResult();

            MagAOI_cmds_AcquireRefAO_result_deltaX.Set(result.deltaXY[0], 0, FORCE_SEND);
            MagAOI_cmds_AcquireRefAO_result_deltaY.Set(result.deltaXY[1], 0, FORCE_SEND);
            MagAOI_cmds_AcquireRefAO_result_slNull.Set(result.slNull, FORCE_SEND);
            MagAOI_cmds_AcquireRefAO_result_f1spec.Set(result.f1spec, FORCE_SEND);
            MagAOI_cmds_AcquireRefAO_result_f2spec.Set(result.f2spec, FORCE_SEND);
            MagAOI_cmds_AcquireRefAO_result_freq.Set(result.freq,0, FORCE_SEND);
            MagAOI_cmds_AcquireRefAO_result_gain.Set(result.gain,0, FORCE_SEND);
            MagAOI_cmds_AcquireRefAO_result_starMag.Set(result.starMag,0, FORCE_SEND);
            MagAOI_cmds_AcquireRefAO_result_nBins.Set(result.nBins,0, FORCE_SEND);
            MagAOI_cmds_AcquireRefAO_result_nModes.Set(result.nModes,0, FORCE_SEND);
            MagAOI_cmds_AcquireRefAO_result_r0.Set(result.r0,0, FORCE_SEND);
            MagAOI_cmds_AcquireRefAO_result_snmode.Set(0.0, 0, FORCE_SEND);
            MagAOI_cmds_AcquireRefAO_result_strehl.Set(result.strehl,0, FORCE_SEND);
            MagAOI_cmds_AcquireRefAO_result_ttMod.Set(result.ttMod,0, FORCE_SEND);


            /*aos.side[side].ao.param.dx=result.deltaXY[0];
            aos.side[side].ao.param.dy=result.deltaXY[1];
            aos.side[side].ao.param.slnull=result.slNull;
            aos.side[side].ao.param.filter1=result.f1spec;
            aos.side[side].ao.param.filter2=result.f2spec;
            aos.side[side].ao.param.freq=result.freq;
            aos.side[side].ao.param.gain=result.gain;
            aos.side[side].ao.param.mag=result.starMag;
            aos.side[side].ao.param.nbins=result.nBins;
            aos.side[side].ao.param.nmodes=result.nModes;
            aos.side[side].ao.param.r0=result.r0;
            aos.side[side].ao.param.snmode=0.0;                // T.B.D.
            aos.side[side].ao.param.strehl=result.strehl;
            aos.side[side].ao.param.ttmodul=result.ttMod;*/
            ostringstream o;
            o << "Stages offset applied - dx: " << result.deltaXY[0] << ";  dy: " << result.deltaXY[1];
            AOIinfoLog( o.str());

            #endif
         }
         else 
         {
            stat = CMD_FAILURE;
            #ifndef AOI_NOARB
            errMsg = aqscmdR->getErrorString();
            #else
            errMsg = "unkown error.";
            #endif
         }  
      } 
      catch(ArbitratorInterfaceException& e) 
      {
         stat = CMD_FAILURE;
         errMsg = e.what(Terse);
      }
   
      delete aqscmd;
   }

   MagAOI_cmds_AcquireRefAO_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_AcquireRefAO_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing AcquireRefAO: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   return stat;
}//int MagAOI::AcquireRefAO(bool rePoint)

int MagAOI::AOI_RefineAO_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   magaoi->MagAOI_cmds_RefineAO_arg_method.Update();
   
   std::string method = magaoi->MagAOI_cmds_RefineAO_arg_method.Get();
   
   #ifdef AOI_DEBUG
   std::cout << "Received RefineAO:\n";
   std::cout << "   method:   " << method << "\n";
   #endif
   
   rv =  magaoi->RefineAO(method);
      
   return rv;
   
}//int MagAOI::AOI_RefineAO_command_changed(void *pt, Variable *msgb __attribute__((unused)))

int MagAOI::RefineAO(string method)
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_RefineAO_stat.Set(CMD_RUNNING,0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::RefineAO *scmd,  *scmdR __attribute__((unused));

   std::ostringstream o;
   o << "Received RefineAO method: " << method;
   
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());
   
   try
   {
      refineAOParams params;
      params.method=method;
      scmd = new arb::RefineAO(RefineAO_tmout,params);
      
      #ifndef AOI_NOARB
      scmdR = (arb::RefineAO *) arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      #endif
      
      if(isSuccess)
      {
         stat=CMD_SUCCESS;
         errMsg = "RefineAO completed successfully";
         #ifndef AOI_NOARB
         arb::acquireRefAOResult result=scmdR->getResult();
         /*aoi_info.ao.param.dx=result.deltaXY[0];
         aoi_info.ao.param.dy=result.deltaXY[1];
         aoi_info.ao.param.slnull=result.slNull;
         aoi_info.ao.param.filter1=result.f1spec;
         aoi_info.ao.param.filter2=result.f2spec;
         aoi_info.ao.param.freq=result.freq;
         aoi_info.ao.param.gain=result.gain;
         aoi_info.ao.param.mag=result.starMag;
         aoi_info.ao.param.nbins=result.nBins;
         aoi_info.ao.param.nmodes=result.nModes;
         aoi_info.ao.param.r0=result.r0;
         aoi_info.ao.param.snmode=0.0;                // T.B.D.
         aoi_info.ao.param.strehl=result.strehl;
         aoi_info.ao.param.ttmodul=result.ttMod;
         
         //aoapp->tvUpdateAcquisition(result.TVframe,256*256+2*sizeof(int));
*/
         #else
         /*aoi_info.ao.param.dx=0;
         aoi_info.ao.param.dy=0;
         aoi_info.ao.param.slnull="slope null";
         aoi_info.ao.param.filter1="filter 1";
         aoi_info.ao.param.filter2="filter 2";
         aoi_info.ao.param.freq=1000.;
         aoi_info.ao.param.gain=0.4;
         aoi_info.ao.param.mag=5.;
         aoi_info.ao.param.nbins=1.;
         aoi_info.ao.param.nmodes=561;
         aoi_info.ao.param.r0=22;
         aoi_info.ao.param.snmode=0.0;                // T.B.D.
         aoi_info.ao.param.strehl=.45;
         aoi_info.ao.param.ttmodul=1;*/
         #endif

      }
      else
      {
         stat = CMD_FAILURE;
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unknown error";
         #endif
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat = CMD_FAILURE;
   }
   delete scmd;
   
   MagAOI_cmds_RefineAO_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_RefineAO_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing RefineAO: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   return stat;
}//int MagAOI::RefineAO(string method)

int MagAOI::AOI_ModifyAO_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;

   int nModes;
   double freq;
   int nBins;
   double ttMod;
   std::string f1;
   std::string f2;
   std::string f3;
   
   magaoi->MagAOI_cmds_ModifyAO_arg_nModes.Update();
   magaoi->MagAOI_cmds_ModifyAO_arg_freq.Update();
   magaoi->MagAOI_cmds_ModifyAO_arg_nBins.Update();
   magaoi->MagAOI_cmds_ModifyAO_arg_ttMod.Update();
   magaoi->MagAOI_cmds_ModifyAO_arg_f1.Update();
   magaoi->MagAOI_cmds_ModifyAO_arg_f2.Update();

   magaoi->MagAOI_cmds_ModifyAO_arg_nModes.Get(&nModes);
   magaoi->MagAOI_cmds_ModifyAO_arg_freq.Get(&freq);
   magaoi->MagAOI_cmds_ModifyAO_arg_nBins.Get(&nBins);
   magaoi->MagAOI_cmds_ModifyAO_arg_ttMod.Get(&ttMod);
   f1 = magaoi->MagAOI_cmds_ModifyAO_arg_f1.Get();
   f2 = magaoi->MagAOI_cmds_ModifyAO_arg_f2.Get();
   f3 = magaoi->MagAOI_cmds_ModifyAO_arg_f3.Get();
   

   #ifdef AOI_DEBUG
   std::cout << "Received ModifyAO:\n";
   std::cout << "   nModes: " << nModes << "\n";
   std::cout << "   freq:   " << freq << "\n";
   std::cout << "   nBins:  " << nBins << "\n";
   std::cout << "   ttMod:  " << ttMod << "\n";
   std::cout << "   f1:     " << f1 << "\n";
   std::cout << "   f2:     " << f2 << "\n";
   std::cout << "   f3:     " << f3 << "\n";
   #endif
   
   
   rv = magaoi->ModifyAO(nModes, freq, nBins, ttMod, f1, f2, f3);
      
   return rv;
   
}//int MagAOI::AOI_ModifyAO_command_changed(void *pt, Variable *msgb __attribute__((unused)))

int MagAOI::ModifyAO(int nModes, double freq, int nBins, double ttMod, string f1, string f2, string f3)
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_ModifyAO_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::ModifyAO *scmd, *scmdR __attribute__((unused));

   std::ostringstream o;
   o << "Received ModifyAO nModes: " << nModes << " freq: " << freq << " nBins: " << nBins;
   o << " ttMod: " << ttMod << " f1: " << f1 << " f2: " << f2;
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   try
   {
      modifyAOParams params;
      params.nModes= nModes;
      params.freq= freq;
      params.binning= nBins;
      params.TTmod= ttMod;
      params.f1spec=f1;
      params.f2spec=f2;
      f3.length(); //temporary warning suppression
      
      scmd = new arb::ModifyAO(ModifyAO_tmout,params);

      #ifndef AOI_NOARB
      scmdR = (arb::ModifyAO *) arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;;
      sleep(10);
      #endif
      
      if(isSuccess)
      {
         stat=CMD_SUCCESS;
         errMsg = "ModifyAO Completed Successfully.";
         
         // Get return values
         #ifndef AOI_NOARB
         arb::acquireRefAOResult result=scmdR->getResult();
         /*aoi_info.ao.param.dx=result.deltaXY[0];
         aoi_info.ao.param.dy=result.deltaXY[1];
         aoi_info.ao.param.slnull=result.slNull;
         aoi_info.ao.param.filter1=result.f1spec;
         aoi_info.ao.param.filter2=result.f2spec;
         aoi_info.ao.param.freq=result.freq;
         aoi_info.ao.param.gain=result.gain;
         aoi_info.ao.param.mag=result.starMag;
         aoi_info.ao.param.nbins=result.nBins;
         aoi_info.ao.param.nmodes=result.nModes;
         aoi_info.ao.param.r0=result.r0;
         aoi_info.ao.param.snmode=0.0;                // T.B.D.
         aoi_info.ao.param.strehl=result.strehl;
         aoi_info.ao.param.ttmodul=result.ttMod;*/
         //aoapp->tvUpdateAcquisition(result.TVframe,256*256+2*sizeof(int));
         #else
         /*aoi_info.ao.param.dx=0;
         aoi_info.ao.param.dy=0;
         aoi_info.ao.param.slnull="slope null";
         aoi_info.ao.param.filter1="filter 1";
         aoi_info.ao.param.filter2="filter 2";
         aoi_info.ao.param.freq=1000.;
         aoi_info.ao.param.gain=0.4;
         aoi_info.ao.param.mag=5.;
         aoi_info.ao.param.nbins=1.;
         aoi_info.ao.param.nmodes=561;
         aoi_info.ao.param.r0=22;
         aoi_info.ao.param.snmode=0.0;                // T.B.D.
         aoi_info.ao.param.strehl=.45;
         aoi_info.ao.param.ttmodul=1;*/
         #endif
         
      }
      else
      {
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unknown error";
         #endif
         stat=CMD_FAILURE;
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat=CMD_FAILURE;
   }
   delete scmd;

   MagAOI_cmds_ModifyAO_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_ModifyAO_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing ModifyAO: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   return stat;
}//int MagAOI::ModifyAO(int nModes, double freq, int nBins, double ttMod, string f1, string f2, string f3)

int MagAOI::AOI_StartAO_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   int rv;
   MagAOI * magaoi = (MagAOI *) pt;

   #ifdef AOI_DEBUG
   std::cout << "Received StartAO\n";
   #endif

   rv = magaoi->StartAO();

   return rv;
}//int MagAOI::AOI_StartAO_command_changed(void *pt, Variable *msgb)

int MagAOI::StartAO()
{
   //extern time_t  _startTime;

   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_StartAO_stat.Set(CMD_RUNNING,0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::Command* scmd, * scmdR __attribute__((unused));

   //_startTime=time(NULL);        // Preset session timer

   std::ostringstream o;
   o << "Received StartAO.";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   
   try 
   {
      scmd = new arb::StartAO(StartAO_tmout);

      #ifndef AOI_NOARB
      scmdR = arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      #endif
     
      if(isSuccess)
      {
         stat = CMD_SUCCESS;
         errMsg = "StartAO completed successfully.";
         //aos.side[side].ao.loopon=LOOP_ON;   // Force status (to avoid timing
                                                // problems in variable update)
      } 
      else 
      {
         stat = CMD_FAILURE;
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unknown error";
         #endif
         
      }
   } 
   catch(ArbitratorInterfaceException& e) 
   {
      stat = CMD_FAILURE;
      errMsg = e.what(Terse);
   }

   delete scmd;

   MagAOI_cmds_StartAO_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_StartAO_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing StartAO: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   return stat;

}//int MagAOI::StartAO()

int MagAOI::AOI_OffsetXY_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   double x;
   double y;
   
   magaoi->MagAOI_cmds_OffsetXY_arg_x.Update();
   magaoi->MagAOI_cmds_OffsetXY_arg_y.Update();
   
   magaoi->MagAOI_cmds_OffsetXY_arg_x.Get(&x);
   magaoi->MagAOI_cmds_OffsetXY_arg_y.Get(&y);
   
   #ifdef AOI_DEBUG
   std::cout << "Received OffsetXY:\n";
   std::cout << "   x: " << x << "\n";
   std::cout << "   y: " << y << "\n";
   #endif
   
   rv = magaoi->OffsetXY(x, y);
   
   return rv;
   
}//int MagAOI::AOI_OffsetXY_command_changed(void *pt, Variable *msgb __attribute__((unused)))

int MagAOI::OffsetXY(double ofsX, double ofsY)
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   //Don't sent CMD_RUNNING yet, have to set time out first
   
   arb::Command* scmd, * scmdR __attribute__((unused));
   offsetXYParams params;
   
   params.deltaXY[0]=ofsX;
   params.deltaXY[1]=ofsY;

   update_loopon(); //Update loop status DD

   if(loopon==LOOP_ON)       // Check the loop status
   {
      OffsetXY_tmout=(int)(max(fabs(ofsX),fabs(ofsY))*OffsetSpeedFactorCL)+AOOffsetXYTmo;
   }
   else
   {
      OffsetXY_tmout=(int)(max(fabs(ofsX),fabs(ofsY))*OffsetSpeedFactorOL)+AOOffsetXYTmo;
   }
   
   MagAOI_cmds_OffsetXY_command_tmout.Set(OffsetXY_tmout,0, FORCE_SEND);
   MagAOI_cmds_OffsetXY_stat.Set(CMD_RUNNING,0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated


   std::ostringstream o;
   o << "[OFFXY] Received OffsetXY x: " << ofsX << " y: " << ofsY << " OffsetXY_tmout: " << OffsetXY_tmout;
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   try
   {
      scmd = new arb::OffsetXY(OffsetXY_tmout,params);

      #ifndef AOI_NOARB
      scmdR = arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      #endif
      
      if(isSuccess)
      {
         errMsg = "OffsetXY completed successfully.";
         stat=CMD_SUCCESS;
      }
      else
      {
         #ifndef AOI_NOARB       
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unkown error";
         #endif
         stat=CMD_FAILURE;
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat=CMD_FAILURE;
   }
   delete scmd;

   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "[OFFXY] Error executing OffsetXY: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }

   MagAOI_cmds_OffsetXY_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_OffsetXY_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   

   return stat;
}//int MagAOI::OffsetXY(double ofsX, double ofsY)

int MagAOI::AOI_NodRaDec_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   double x;
   double y;
   
   magaoi->MagAOI_cmds_NodRaDec_arg_ra.Update();
   magaoi->MagAOI_cmds_NodRaDec_arg_dec.Update();
   
   magaoi->MagAOI_cmds_NodRaDec_arg_ra.Get(&x);
   magaoi->MagAOI_cmds_NodRaDec_arg_dec.Get(&y);
   
   #ifdef AOI_DEBUG
   std::cout << "Received NodRaDec:\n";
   std::cout << "   ra: " << x << "\n";
   std::cout << "   dec: " << y << "\n";
   #endif
   
   rv = magaoi->NodRaDec(x, y);
   
   return rv;
   
}//int MagAOI::AOI_OffsetXY_command_changed(void *pt, Variable *msgb __attribute__((unused)))

int MagAOI::NodRaDec(double ofs_RA, double ofs_DEC)
{
   double ofsX, ofsY;

   int stat;
   std::string errMsg;
   clear_commands();

   //defaults
   errMsg = "NodRaDec: completed successfully.";

   update_loopon(); //Update loop status DD

   if(loopon!=LOOP_ON)       // Check the loop status
   {
      std::cout << "Open Loop Nod\n";
      return NodRaDecOpenLoop(ofs_RA, ofs_DEC);
   }
   std::cout << "Closed Loop Nod\n";

   MagAOI_cmds_NodRaDec_InProgress.Set(1, 0, FORCE_SEND);

   //Here must turn ofs_RA and ofs_DEC into x and y mm.
   /*
    * do math here . . .
    */
   double ang = 3.14159*(_telRotOff + 90.)/180.;
   //double bayside_scale = 1./1.97;// (mm/arcsec)

   ofsY = nodydir*(1./bayside_scale) * (-ofs_DEC*cos(ang) - ofs_RA*sin(ang));
   ofsX = nodxdir*(1./bayside_scale) * ( ofs_DEC*sin(ang) - ofs_RA*cos(ang));


   //--------------------- Dead X Workaround
   // set offsX to 0, and then change the RA and DEC to nod only in the Y direction
#if 0
   ofsX = 0.0;

   ofs_DEC = -1*ofsY*(bayside_scale/nodydir)*cos(ang);
   ofs_RA = -1*ofsY*(bayside_scale/nodydir)*sin(ang);
#endif


   //-------------------------------------------------------









   NodRaDec_tmout=(int)(max(fabs(ofsX),fabs(ofsY))*OffsetSpeedFactorOL)+AONodRaDecTmo;
   
   MagAOI_cmds_NodRaDec_command_tmout.Set(NodRaDec_tmout,0, FORCE_SEND);

   MagAOI_cmds_NodRaDec_stat.Set(CMD_RUNNING,0, FORCE_SEND);

   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated

   std::ostringstream o;
   o << "[NOD] Received NodRaDec RA: " << ofs_RA << " = " << ofsX << " DEC: " << ofs_DEC << " = " <<  ofsY;
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   stat = Pause(); // Will block until complete

   if(stat != CMD_SUCCESS)
   {
      std::ostringstream err;
      err << "[NOD] Error pausing loop.  Not moving.";
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());    
      MagAOI_cmds_NodRaDec_stat.Set(CMD_FAILURE,0, FORCE_SEND);
      MagAOI_cmds_NodRaDec_InProgress.Set(0, 0, FORCE_SEND);
      return -1;
   }

   char tcscomstr[256];
   int tcsrv;

   snprintf(tcscomstr, 256 , "ofra %f", ofs_RA);
     
   tcsrv = sendMagTelCommand(tcscomstr, 10000);
   if(tcsrv != 0)
   {
      std::ostringstream err;
      err << "[NOD] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      MagAOI_cmds_NodRaDec_stat.Set(CMD_FAILURE,0, FORCE_SEND);
      MagAOI_cmds_NodRaDec_InProgress.Set(0, 0, FORCE_SEND);
      return -1;
   }

   snprintf(tcscomstr, 256 , "ofdc %f", ofs_DEC);
   tcsrv = sendMagTelCommand(tcscomstr, 10000);
   if(tcsrv != 0)
   {
      std::ostringstream err;
      err << "[NOD]  TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      MagAOI_cmds_NodRaDec_stat.Set(CMD_FAILURE,0, FORCE_SEND);
      MagAOI_cmds_NodRaDec_InProgress.Set(0, 0, FORCE_SEND);
      return -1;
   }

   snprintf(tcscomstr, 256 , "offp ");
   tcsrv = sendMagTelCommand(tcscomstr, 10000);
   if(tcsrv != 0)
   {
      std::ostringstream err;
      err << "[NOD] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      errMsg = "Nod: TCS returned error.";
      MagAOI_cmds_NodRaDec_stat.Set(stat, 0, FORCE_SEND);
      MagAOI_cmds_NodRaDec_errMsg.Set(errMsg, FORCE_SEND);
      MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
      MagAOI_cmds_NodRaDec_InProgress.Set(0, 0, FORCE_SEND);


      return -1;

   }

   stat = OffsetXY(ofsX, ofsY); // Move AO stages

   if(stat != CMD_SUCCESS)
   {
      std::ostringstream err;
      err << "[NOD] Error in OffsetXY.";
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      errMsg = "Nod: error in OffsetXY.";
      MagAOI_cmds_NodRaDec_stat.Set(stat, 0, FORCE_SEND);
      MagAOI_cmds_NodRaDec_errMsg.Set(errMsg, FORCE_SEND);
      MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
      MagAOI_cmds_NodRaDec_InProgress.Set(0, 0, FORCE_SEND);

      return -1;
   }


   double amp = sqrt(ofs_RA*ofs_RA + ofs_DEC*ofs_DEC);

   std::cout << "nod amp: " << amp << "\n";
   if(amp < 1)
   {
      //no sleep;
   }
   else if(amp < 5)
   {
      sleep(2);
   }
   else if(amp < 10)
   {
      sleep(4);
   }
   else
   {
      sleep(6);
   }
//   sleep(5);

//    //mountw
//    snprintf(tcscomstr, 256 , "mountw %i", mountw_tmout);
// 
//    //sleep(3);
//    tcsrv = sendMagTelCommand(tcscomstr, 10000);//mountw_tmout*1000);
//    if(tcsrv != 0)
//    {
//       std::ostringstream err;
//       err << "[NOD] TCS mountw returned error " << tcsrv << " i.r.t. " << tcscomstr;
//       Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
//       //sleep(3);
//       //MagAOI_cmds_NodRaDec_InProgress.Set(0, 0, FORCE_SEND);
//       //return -1;
//    }

//    snprintf(tcscomstr, 256 , "mountw %i", 1);
// 
//    bool complete = false;
//    int max = 3;
//    int count = 0;
// 
//    while(complete == false && count < max)
//    {
//       sleep(3);
//       tcsrv = sendMagTelCommand(tcscomstr, 2);//mountw_tmout*1000);
//       if(tcsrv == 0) complete = true;
//       ++count;
//    }

   //if(complete == false || count >= max || tcsrv < 0)
   //For now, there's nothing to do, we've already waited > 15 secs. 

   stat = Resume(); // Will block until complete
   if(stat != CMD_SUCCESS)
   {
      std::ostringstream err;
      err << "[NOD] Error in resuming loop.";
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
/*      MagAOI_cmds_NodRaDec_stat.Set(CMD_FAILURE,0, FORCE_SEND);

      MagAOI_cmds_NodRaDec_InProgress.Set(0, 0, FORCE_SEND);*/
      errMsg = "Nod: Error in resuming loop.";
      MagAOI_cmds_NodRaDec_stat.Set(stat, 0, FORCE_SEND);
      MagAOI_cmds_NodRaDec_errMsg.Set(errMsg, FORCE_SEND);
      MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
      MagAOI_cmds_NodRaDec_InProgress.Set(0, 0, FORCE_SEND);

   }

   MagAOI_cmds_NodRaDec_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_NodRaDec_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   MagAOI_cmds_NodRaDec_InProgress.Set(0, 0, FORCE_SEND);

   return stat;
}//int MagAOI::NodRaDec(double ofsX, double ofsY)

int MagAOI::NodRaDecOpenLoop(double ofs_RA, double ofs_DEC)
{
   std::ostringstream o;
   o << "[NOD] Received NodRaDec Open Loop RA: " << ofs_RA << " DEC: " << ofs_DEC;
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   char tcscomstr[256];
   int tcsrv;

   MagAOI_cmds_NodRaDec_stat.Set(CMD_RUNNING,0, FORCE_SEND);
   MagAOI_cmds_NodRaDec_InProgress.Set(1, 0, FORCE_SEND);

   snprintf(tcscomstr, 256 , "ofra %f", ofs_RA);
   tcsrv = sendMagTelCommand(tcscomstr, 10000);
   if(tcsrv != 0)
   {
      std::ostringstream err;
      err << "[NOD] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());

      MagAOI_cmds_NodRaDec_stat.Set(CMD_FAILURE,0, FORCE_SEND);
      MagAOI_cmds_NodRaDec_InProgress.Set(0, 0, FORCE_SEND);
      return -1;
   }

   snprintf(tcscomstr, 256 , "ofdc %f", ofs_DEC);      
   tcsrv = sendMagTelCommand(tcscomstr, 10000);
   if(tcsrv != 0)
   {
      std::ostringstream err;
      err << "[NOD] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      MagAOI_cmds_NodRaDec_stat.Set(CMD_FAILURE,0, FORCE_SEND);
      MagAOI_cmds_NodRaDec_InProgress.Set(0, 0, FORCE_SEND);
      return -1;
   }

   snprintf(tcscomstr, 256 , "offp ");
   tcsrv = sendMagTelCommand(tcscomstr, 10000);
   if(tcsrv != 0)
   {
      std::ostringstream err;
      err << "[NOD] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      MagAOI_cmds_NodRaDec_stat.Set(CMD_FAILURE,0, FORCE_SEND);
      MagAOI_cmds_NodRaDec_InProgress.Set(0, 0, FORCE_SEND);
      return -1;

   }

//   sleep(5);
   //mountw
//   snprintf(tcscomstr, 256 , "mountw %i", mountw_tmout) ;
//    tcsrv = sendMagTelCommand(tcscomstr, mountw_tmout*1000*3); //Just wait for a really long time, since we're open loop
//    if(tcsrv != 0)
//    {
//       std::ostringstream err;
//       err << "[NOD] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
//       Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
//       MagAOI_cmds_NodRaDec_stat.Set(CMD_FAILURE,0, FORCE_SEND);
// 
//    }
//   else MagAOI_cmds_NodRaDec_stat.Set(CMD_SUCCESS,0, FORCE_SEND);

   snprintf(tcscomstr, 256 , "mountw %i", 1) ;

   bool complete = false;
   int max = 3;
   int count = 0;
   while(complete == false && count < max)
   {
      sleep(3);
      tcsrv = sendMagTelCommand(tcscomstr, 2);//mountw_tmout*1000);
      if(tcsrv == 0) complete = true;
      if(tcsrv < 0) break;
      ++count;
   }

   if(tcsrv < 0)
   {
      std::ostringstream err;
      err << "[NOD] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
      Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
      MagAOI_cmds_NodRaDec_stat.Set(CMD_FAILURE,0, FORCE_SEND);
   }
   else
   {
      MagAOI_cmds_NodRaDec_stat.Set(CMD_SUCCESS,0, FORCE_SEND);
   }

   MagAOI_cmds_NodRaDec_InProgress.Set(0, 0, FORCE_SEND);

   return 0;

}


int MagAOI::AOI_OffsetZ_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   double z;
   
   magaoi->MagAOI_cmds_OffsetZ_arg_z.Update();
   
   magaoi->MagAOI_cmds_OffsetZ_arg_z.Get(&z);
   
   #ifdef AOI_DEBUG
   std::cout << "Received OffsetZ:\n";
   std::cout << "   z: " << z << "\n";
   #endif
   
   rv = magaoi->OffsetZ(z);
   
   return rv;
   
}//int MagAOI::AOI_OffsetZ_command_changed(void *pt, Variable *msgb)

int MagAOI::OffsetZ(double dz)
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   //Don't send CMD_RUNNING yet, have to set time out first
   
   arb::OffsetZ* scmd, * scmdR __attribute__((unused));
              
   offsetZParams params;
   params.deltaZ=dz;

   update_loopon(); //Update loop status DD
   
   if(loopon==LOOP_ON)       // Check the loop status
   {
      OffsetZ_tmout=(int)(fabs(dz)*OffsetSpeedFactorCL)+10000;
   }
   else
   {
      OffsetZ_tmout=(int)(fabs(dz)*OffsetSpeedFactorOL)+10000;
   }

   MagAOI_cmds_OffsetZ_command_tmout.Set(OffsetZ_tmout, 0, FORCE_SEND);
   MagAOI_cmds_OffsetZ_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated


   std::ostringstream o;
   o << "[OFFZ] Received OffsetZ z: " << dz;
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   try
   {
      scmd = new arb::OffsetZ(OffsetZ_tmout,params);
      #ifndef AOI_NOARB
      scmdR = (arb::OffsetZ *) arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      #endif
      
      if(isSuccess)
      {
         errMsg = "OffsetZ completed successfully.";
         stat=CMD_SUCCESS;
      }
      else
      {
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unkown error";
         #endif
         stat=CMD_FAILURE;
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat=CMD_FAILURE;
   }
   delete scmd;
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "[OFFZ] Error executing OffsetZ: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   MagAOI_cmds_OffsetZ_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_OffsetZ_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   

   return stat;
}//int MagAOI::OffsetZ(double dz)


int MagAOI::AOI_Focus_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   double z;
   
   magaoi->MagAOI_cmds_Focus_arg_z.Update();
   
   magaoi->MagAOI_cmds_Focus_arg_z.Get(&z);
   
   #ifdef AOI_DEBUG
   std::cout << "Received Focus:\n";
   std::cout << "   z: " << z << "\n";
   #endif
   
   rv = magaoi->OffsetZ(z);//Focus(z);
   
   return rv;
   
}//int MagAOI::AOI_Focus_command_changed(void *pt, Variable *msgb)


int MagAOI::Focus(double dz)
{
   int stat;
   std::string errMsg;
   clear_commands();
   //Don't sent CMD_RUNNING yet, have to set time out first
   
   update_loopon(); //Update loop status DD

   if(loopon==LOOP_ON)       // Check the loop status
   {
      Focus_tmout=(int)(fabs(dz)*OffsetSpeedFactorCL)+10000;
   }
   else
   {
      Focus_tmout=(int)(fabs(dz)*OffsetSpeedFactorOL)+10000;
   }

   MagAOI_cmds_Focus_command_tmout.Set(Focus_tmout, 0, FORCE_SEND);
   MagAOI_cmds_Focus_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated


   std::ostringstream o;
   o << "Received Focus z: " << dz;
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   stat = Pause(); // Will block until complete

   if(stat == 0)  stat = OffsetZ(dz); // Will block until complete

   if(stat == 0)  stat = Resume(); // Will block until complete

   MagAOI_cmds_Focus_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_Focus_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   

   return stat;
}//int MagAOI::Focus(double dz)



// RTDB handler for a Stop command via MsgD.
int MagAOI::AOI_Stop_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   std::string msg;
   
   magaoi->MagAOI_cmds_Stop_arg_msg.Update();
   
   msg = magaoi->MagAOI_cmds_Stop_arg_msg.Get();
   
   #ifdef AOI_DEBUG
   std::cout << "Received Stop:\n";
   std::cout << "   msg: " << msg << "\n";
   #endif
   
   rv = magaoi->Stop(msg);
   
   return rv;
   
}//int MagAOI::AOI_Stop_command_changed(void *pt, Variable *msgb)


int MagAOI::Stop(string msg)
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_Stop_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::Command* scmd, * scmdR __attribute__((unused));

   std::ostringstream o;
   o << "Received Stop msg: " << msg;
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   
   stopParams params;
   params.msg=msg;
   try
   {
      scmd = new arb::Stop(Stop_tmout, params);
      
      #ifndef AOI_NOARB
      scmdR = arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      #endif
      
      if(isSuccess)
      {
         errMsg = "Stop completed successfully.";
         stat=CMD_SUCCESS;
      }
      else
      {
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unkown error";
         #endif
         stat=CMD_FAILURE;
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat=CMD_FAILURE;
   }
   delete scmd;
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing Stop: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   MagAOI_cmds_Stop_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_Stop_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   
   return stat;
}//int MagAOI::Stop(string msg)


int MagAOI::AOI_Pause_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   #ifdef AOI_DEBUG
   std::cout << "Received Pause.\n";
   #endif
   
   rv = magaoi->Pause();
   
   return rv;
   
}//int AOI_Pause_command_changed(void *pt, Variable *msgb)

int MagAOI::Pause()
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_Pause_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::Command* scmd, * scmdR __attribute__((unused));

   std::ostringstream o;
   o << "Received Pause";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   try
   {
      scmd = new arb::Pause(Pause_tmout);

      #ifndef AOI_NOARB
      scmdR = arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      #endif
      
      if(isSuccess)
      {
         errMsg = "Pause completed successfully.";
         stat=CMD_SUCCESS;
      }
      else
      {
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unkown error";
         #endif
         stat=CMD_FAILURE;
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat=CMD_FAILURE;
   }
   delete scmd;
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing Pause: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   MagAOI_cmds_Pause_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_Pause_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   return stat;
}//int MagAOI::Pause()

int MagAOI::AOI_Resume_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   #ifdef AOI_DEBUG
   std::cout << "Received Resume.\n";
   #endif
   
   rv = magaoi->Resume();
   
   return rv;
   
}//int AOI_Resume_command_changed(void *pt, Variable *msgb)

int MagAOI::Resume()
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_Resume_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::Command* scmd, * scmdR __attribute__((unused));
   
   std::ostringstream o;
   o << "Received Resume";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());
   
   try
   {
      scmd = new arb::Resume(Resume_tmout);
      
      #ifndef AOI_NOARB
      scmdR = arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      #endif
      
      if(isSuccess)
      {
         errMsg = "Resume completed successfully.";
         stat=CMD_SUCCESS;
      }
      else
      {
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unkown error";
         #endif
         stat=CMD_FAILURE;
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat=CMD_FAILURE;
   }
   delete scmd;
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing Resume: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   MagAOI_cmds_Resume_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_Resume_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   return stat;
}//int MagAOI::Resume()

int MagAOI::AOI_UserPanic_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   std::string reason;
   
   magaoi->MagAOI_cmds_UserPanic_arg_reason.Update();
   
   reason = magaoi->MagAOI_cmds_UserPanic_arg_reason.Get();
   
   #ifdef AOI_DEBUG
   std::cout << "Received UserPanic:\n";
   std::cout << "   Reason: " << reason << "\n";
   #endif
   
   rv = magaoi->UserPanic(reason);
   
   return rv;
   
}//int MagAOI::AOI_UserPanic_command_changed(void *pt, Variable *msgb)

int MagAOI::UserPanic(std::string reason)
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_UserPanic_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated

   arb::Command* scmd, * scmdR __attribute__((unused));

   std::ostringstream o;
   o << "Received UserPanic Reason: " << reason;
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());
   
   userPanicParams param;
   param.msg=reason;
   try
   {
      scmd = new arb::UserPanic(WfsOn_tmout,param);
      #ifndef AOI_NOARB
      scmdR = arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      sleep(10);
      isSuccess = 1;
      #endif
      
      if(isSuccess)
      {
         errMsg = "UserPanic completed successfully.";
         stat=CMD_SUCCESS;
      }
      else
      {
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unkown error";
         #endif
         stat=CMD_FAILURE;
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat=CMD_FAILURE;
   }
   delete scmd;
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing UserPanic: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   MagAOI_cmds_UserPanic_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_UserPanic_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   
   return stat;
   
}//int MagAOI::UserPanic(std::string reason)

int MagAOI::AOI_SetNewInstrument_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   std::string instr, focalSt;
   
   magaoi->MagAOI_cmds_SetNewInstrument_arg_instr.Update();
   magaoi->MagAOI_cmds_SetNewInstrument_arg_focalSt.Update();
   
   instr = magaoi->MagAOI_cmds_SetNewInstrument_arg_instr.Get();
   focalSt = magaoi->MagAOI_cmds_SetNewInstrument_arg_focalSt.Get();
   
   #ifdef AOI_DEBUG
   std::cout << "Received SetNewInstrument:\n";
   std::cout << "   instr: " << instr << "\n";
   std::cout << "   focalSt: " << focalSt << "\n";
   #endif
   
   rv = magaoi->SetNewInstrument(instr, focalSt);
   
   return rv;
   
}//static int AOI_SetNewInstrument_command_changed(void *pt, Variable *msgb)

int MagAOI::SetNewInstrument(std::string instr, std::string focalSt)
{
   int stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_SetNewInstrument_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated

   

   std::ostringstream o;
   o << "Received SetNewInstrument instr: " << instr << " focalSt: " << focalSt;
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());
   
   instrumentName = instr;


   MagAOI_cmds_SetNewInstrument_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_SetNewInstrument_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   
   return stat;  
}//int SetNewInstrument(std::string instr, std::string focalSt);
      
int MagAOI::AOI_WfsOn_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   std::string WfsID;
   
   magaoi->MagAOI_cmds_WfsOn_arg_WfsID.Update();
   
   WfsID = magaoi->MagAOI_cmds_WfsOn_arg_WfsID.Get();
   
   #ifdef AOI_DEBUG
   std::cout << "Received Wfs On:\n";
   std::cout << "   WfsID: " << WfsID << "\n";
   #endif
   
   rv = magaoi->WfsOn(WfsID);
   
   return rv;
   
}//int MagAOI::AOI_WfsOn_command_changed(void *pt, Variable *msgb)

int MagAOI::WfsOn(string WfsId)
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_WfsOn_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::Command* scmd, * scmdR __attribute__((unused));
   
   std::ostringstream o;
   o << "Received WfsOn WfsId: " << WfsId;
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());
   
   wfsOnOffParams param;
   param.wfsid=WfsId;
   try
   {
      scmd = new arb::PowerOnWfs(WfsOn_tmout,param);
      #ifndef AOI_NOARB
      scmdR = arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      sleep(10);
      MagArb_LED.Set(1, 0, FORCE_SEND);
      isSuccess = 1;
      #endif
      
      if(isSuccess)
      {
         errMsg = "WfsOn completed successfully.";
         stat=CMD_SUCCESS;
      }
      else
      {
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unkown error";
         #endif
         stat=CMD_FAILURE;
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat=CMD_FAILURE;
   }
   delete scmd;
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing WfsOn: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   MagAOI_cmds_WfsOn_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_WfsOn_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   
   return stat;
   
}

// RTDB handler for a WfsOff command via MsgD.
int MagAOI::AOI_WfsOff_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   std::string WfsID;
   
   magaoi->MagAOI_cmds_WfsOff_arg_WfsID.Update();
   
   WfsID = magaoi->MagAOI_cmds_WfsOff_arg_WfsID.Get();
   
   #ifdef AOI_DEBUG
   std::cout << "Received Wfs Off:\n";
   std::cout << "   WfsID: " << WfsID << "\n";
   #endif
   
   rv = magaoi->WfsOff(WfsID);
   
   return rv;
   
}//int MagAOI::AOI_WfsOff_command_changed(void *pt, Variable *msgb)

int MagAOI::WfsOff(string WfsId)
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_WfsOff_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::Command* scmd, * scmdR __attribute__((unused));
   
   std::ostringstream o;
   o << "Received WfsOff WfsId: " << WfsId;
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());
   
   wfsOnOffParams param;
   param.wfsid=WfsId;
   try
   {
      scmd = new arb::PowerOffWfs(WfsOff_tmout,param);
      #ifndef AOI_NOARB
      scmdR = arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      MagArb_LED.Set(0, 0, FORCE_SEND);
      isSuccess = 1;
      #endif
      
      if(isSuccess)
      {
         errMsg = "WfsOff completed successfully.";
         stat=CMD_SUCCESS;
      }
      else
      {
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unkown error";
         #endif
         stat=CMD_FAILURE;
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat=CMD_FAILURE;
   }
   delete scmd;
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing WfsOff: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   MagAOI_cmds_WfsOff_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_WfsOff_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   
   return stat;
   
}//int MagAOI::WfsOff(string WfsId)


int MagAOI::AOI_AdsecOn_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   #ifdef AOI_DEBUG
   std::cout << "Received AdsecOn.\n";
   #endif
   
   rv = magaoi->AdsecOn();
   
   return rv;
   
}//int AOI_AdsecOn_command_changed(void *pt, Variable *msgb)

int MagAOI::AdsecOn()
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_AdsecOn_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::Command* scmd, * scmdR __attribute__((unused));

   std::ostringstream o;
   o << "Received AdsecOn";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   try
   {
      scmd = new arb::PowerOnAdSec(AdsecOn_tmout);

      #ifndef AOI_NOARB
      scmdR = arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      #endif
      
      if(isSuccess)
      {
         errMsg = "AdsecOn completed successfully.";
         stat=CMD_SUCCESS;
      }
      else
      {
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unkown error";
         #endif
         stat=CMD_FAILURE;
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat=CMD_FAILURE;
   }
   delete scmd;
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing AdsecOn: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   MagAOI_cmds_AdsecOn_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_AdsecOn_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   return stat;
}//int MagAOI::AdsecOn()

int MagAOI::AOI_AdsecOff_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   #ifdef AOI_DEBUG
   std::cout << "Received AdsecOff.\n";
   #endif
   
   rv = magaoi->AdsecOff();
   
   return rv;
   
}//int AOI_AdsecOff_command_changed(void *pt, Variable *msgb)

int MagAOI::AdsecOff()
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_AdsecOff_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::Command* scmd, * scmdR __attribute__((unused));

   std::ostringstream o;
   o << "Received AdsecOff";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   try
   {
      scmd = new arb::PowerOffAdSec(AdsecOff_tmout);

      #ifndef AOI_NOARB
      scmdR = arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      #endif
      
      if(isSuccess)
      {
         errMsg = "AdsecOff completed successfully.";
         stat=CMD_SUCCESS;
      }
      else
      {
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unkown error";
         #endif
         stat=CMD_FAILURE;
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat=CMD_FAILURE;
   }
   delete scmd;
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing AdsecOff: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   MagAOI_cmds_AdsecOff_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_AdsecOff_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   return stat;
}//int MagAOI::AdsecOff()


int MagAOI::AOI_AdsecSet_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   #ifdef AOI_DEBUG
   std::cout << "Received AdsecSet.\n";
   #endif
   
   rv = magaoi->AdsecSet();
   
   return rv;
   
}//int AOI_AdsecSet_command_changed(void *pt, Variable *msgb)

int MagAOI::AdsecSet()
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_AdsecSet_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::Command* scmd, * scmdR __attribute__((unused));

   std::ostringstream o;
   o << "Received AdsecSet";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   try
   {
      scmd = new arb::MirrorSet(AdsecSet_tmout);

      #ifndef AOI_NOARB
      scmdR = arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      #endif
      
      if(isSuccess)
      {
         errMsg = "AdsecSet completed successfully.";
         stat=CMD_SUCCESS;
      }
      else
      {
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unkown error";
         #endif
         stat=CMD_FAILURE;
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat=CMD_FAILURE;
   }
   delete scmd;
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing AdsecSet: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   MagAOI_cmds_AdsecSet_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_AdsecSet_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   return stat;
}//int MagAOI::AdsecSet()


int MagAOI::AOI_AdsecRest_command_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;
   int rv;
   
   #ifdef AOI_DEBUG
   std::cout << "Received AdsecRest.\n";
   #endif
   
   rv = magaoi->AdsecRest();
   
   return rv;
   
}//int AOI_AdsecRest_command_changed(void *pt, Variable *msgb)

int MagAOI::AdsecRest()
{
   int isSuccess, stat;
   std::string errMsg;
   clear_commands();
   MagAOI_cmds_AdsecRest_stat.Set(CMD_RUNNING, 0, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   arb::Command* scmd, * scmdR __attribute__((unused));

   std::ostringstream o;
   o << "Received AdsecRest";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   try
   {
      scmd = new arb::MirrorRest(AdsecRest_tmout);

      #ifndef AOI_NOARB
      scmdR = arbIntf->requestCommand(scmd);
      isSuccess = scmdR->isSuccess();
      #else
      isSuccess = 1;
      sleep(10);
      #endif
      
      if(isSuccess)
      {
         errMsg = "AdsecRest completed successfully.";
         stat=CMD_SUCCESS;
      }
      else
      {
         #ifndef AOI_NOARB
         errMsg=scmdR->getErrorString();
         #else
         errMsg = "unkown error";
         #endif
         stat=CMD_FAILURE;
      }
   }
   catch(ArbitratorInterfaceException& e)
   {
      errMsg=e.what(Terse);
      stat=CMD_FAILURE;
   }
   delete scmd;
   
   if(stat != CMD_SUCCESS)
   {
      o.str("");
      o << "Error executing AdsecRest: " << errMsg;
      Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());
   }
   
   MagAOI_cmds_AdsecRest_stat.Set(stat, 0, FORCE_SEND);
   MagAOI_cmds_AdsecRest_errMsg.Set(errMsg, FORCE_SEND);
   MagAOI_cmds_updateargs.Set(1, 0, FORCE_SEND); // Values on command display must be updated
   
   return stat;
}//int MagAOI::AdsecRest()





/********************* TCS Interface ********************************/

int MagAOI::SetupNetwork()
{
   int stat;

   //Always close previous connection
   ShutdownNetwork();

   Logger::get()->log( Logger::LOG_LEV_INFO, "Connecting to %s:%d", TCSaddr.c_str(), TCSport_com);

   // Setup serial/network interface
   pthread_mutex_lock(&aoiMutex);

   //?? Is this the right thing to use for non-serial comms?
   stat = SerialInit( TCSaddr.c_str(), TCSport_com ); // This locks if network is down or host unreachable

   pthread_mutex_unlock(&aoiMutex);
   
   if (stat != NO_ERROR)
   {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Connect result: %d - Errno: %s", stat, strerror(errno));
      return stat;
   }
   
   //CHECK_SUCCESS(stat);
   
   //********** Why are we not doing any error checking here? ************************//

    // Set OK flag
   network_ok =1;
   

   Logger::get()->log( Logger::LOG_LEV_INFO, "Network connect OK");

   return NO_ERROR;
}//int MagAOI::SetupNetwork()

int MagAOI::ShutdownNetwork()
{
   int stat = NO_ERROR;

   if ( network_ok)
   {
      pthread_mutex_lock(&aoiMutex);
      Logger::get()->log( Logger::LOG_LEV_DEBUG, "Closing communication");
      stat = SerialClose();
      pthread_mutex_unlock(&aoiMutex);
      CHECK_SUCCESS(stat);

      network_ok = 0;

   }
   
   return stat;
}//int MagAOI::ShutdownNetwork()


int MagAOI::TestNetwork(void)
{
   int stat;
   
   std::string resp;


   Logger::get()->log( Logger::LOG_LEV_TRACE, "Testing network, network_ok=%d", network_ok);
   if (!network_ok)
   {
      stat = SetupNetwork();
      if(stat != NO_ERROR) return stat;
   }

   //To test.
   //stat = getDateTime();

   resp = getMagTelStatus("dateobs");
   
   //Process response here.
   //turn into a stat
   if(resp == "") stat = -1;
   else stat = 0;
   
   if (stat<0)
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "Error sending test command");
      return stat;
   }

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Network test OK - answer %s", resp.c_str());
   return NO_ERROR;
}//int MagAOI::TestNetwork(void)
                
std::string MagAOI::getMagTelStatus(const std::string &statreq)
{
   int stat;
   char answer[512];
   std::string statreq_nl;

   #ifdef _debug
   std::cout << "locking mutex for: " << statreq << "\n";
   #endif   

   pthread_mutex_lock(&aoiMutex);

   #ifdef _debug
   std::cout << "mutex locked for:" << statreq << "\n";
   #endif


   #ifdef LOG_TCS_STATUS
   Logger::get()->log( Logger::LOG_LEV_INFO, "[TCS] Sending status request: %s", statreq.c_str());
   #endif

   statreq_nl = statreq;
   statreq_nl += '\n';
   stat = SerialOut(statreq_nl.c_str(), statreq_nl.length());
   
   if(stat != NO_ERROR)
   {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "[TCS] Error sending status request: %s", statreq.c_str());
      pthread_mutex_unlock(&aoiMutex);
      return "";
   }
   
   #ifdef _debug
   std::cout << "waiting for response\n";
   #endif

   stat = SerialInString(answer, 512, 1000, '\n');
   
   #ifdef _debug
   std::cout << "Response " << answer << "\n";
   #endif

   if(stat <= 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] No response received to status request: %s", statreq.c_str());
      pthread_mutex_unlock(&aoiMutex);
      return "";
   }

   char * nl = strchr(answer, '\n');
   if(nl) answer[nl-answer] = '\0';

   #ifdef LOG_TCS_STATUS
   Logger::get()->log(Logger::LOG_LEV_INFO, "[TCS] Received response: %s", answer);
   #endif

   pthread_mutex_unlock(&aoiMutex);
   
   return answer;
   
}//std::string MagAOI::getMagTelStatus(const std::string &statreq)

int MagAOI::sendMagTelCommand(const std::string &command, int timeout)
{
   int stat;
   char answer[512];

   std::string command_nl;
   
   #ifdef _debug
   std::cout << "Sending " << command << "\n";
   #endif
   
   pthread_mutex_lock(&aoiMutex);

   #ifdef _debug
   std::cout << "mutex locked for:" << command << "\n";
   #endif

   Logger::get()->log( Logger::LOG_LEV_INFO, "[TCSCOM] Sending command: %s", command.c_str());

   command_nl = command;
   command_nl += '\n';
   stat = SerialOut(command_nl.c_str(), command_nl.length());

   if(stat != NO_ERROR)
   {
      Logger::get()->log( Logger::LOG_LEV_INFO, "[TCSCOM] Error sending command: %s", command.c_str());
      pthread_mutex_unlock(&aoiMutex);
      return -1000;
   }

   stat = SerialInString(answer, 512, timeout, '\n');

   #ifdef _debug
   std::cout << "Response " << answer << "\n";
   #endif

   if(stat <= 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCSCOM] No response received to command: %s", command.c_str());
      pthread_mutex_unlock(&aoiMutex);
      return -1000;
   }
   
   Logger::get()->log(Logger::LOG_LEV_INFO, "[TCSCOM] Received response: %s", answer);
   
   pthread_mutex_unlock(&aoiMutex);
   
   return atoi(answer);
   
}//int MagAOI::sendMagTelCommand(const std::string &command)


std::vector<std::string> MagAOI::parse_teldata(std::string &tdat)
{
   std::vector<std::string> vres;

   std::string tok;

   int pos1, pos2;

  
   //skip all leading spaces
   pos1 = tdat.find_first_not_of(" " , 0);
    
   if(pos1 == -1) pos1 = 0;

   pos2 = tdat.find_first_of(" ", pos1);

   while(pos2 > 0)
   {
      tok = tdat.substr(pos1, pos2-pos1);

      vres.push_back(tok);

      //now move past end of current spaces - might be more than one.
      pos1 = pos2;

      pos2 = tdat.find_first_not_of(" ", pos1);

      //and then find the end of this value.
      pos1 = pos2;

      pos2 = tdat.find_first_of(" ", pos1);
   }

   //If there is another value, we pick it up here.
   if(pos1 >= 0)
   {
      pos2 = tdat.length();

      tok = tdat.substr(pos1, pos2-pos1);

      pos2 = tok.find_first_of(" \n\r", 0);

      if(pos2 >= 0) tok.erase(pos2, tok.length()-pos2);

      vres.push_back(tok);
   }

   return vres;
}//std::vector<std::string> MagAOI::parse_teldata(std::string &tdat)

int MagAOI::parse_xms(const std::string & xmsstr, double * x, double *m, double *s)
{
   int st, en;

   int sgn = 1;


   st = 0;
   en = xmsstr.find(':', st);
   
   *x = strtod(xmsstr.substr(st, en-st).c_str(), 0);

   //Check for negative
   //if(*x <= -0) sgn = -1;
   if(signbit(*x)) sgn = -1;

   st = en + 1;
   
   en = xmsstr.find(':', st);
   
   *m = sgn*strtod(xmsstr.substr(st, en-st).c_str(), 0);
   
   st = en+1;
   
   *s = sgn*strtod(xmsstr.substr(st, xmsstr.length()-st).c_str(), 0);
   
   return 0;
}//int MagAOI::parse_xms(const std::string & xmsstr, double * x, double *m, double *s)



int MagAOI::updateMagTelStatus()
{
   int rv;
   send_actions send_rule = CHECK_SEND;

   //return 0;

   //The get functions return < 0 on a communications error.  We exit here with an error if that happens.
   //A positve return value indicates successful communications with TCS, but TCS response was an error or incomplete.
   //we continue in that case, but do not update the relevant values
   //rv == 0 is success.

   rv = getDateTime();
   if(rv < 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error updating telescope date and time");
      return -1;
   }
   if(rv == 0)
   {
      var_dateobs_cur.Set(_dateobs, send_rule);
      var_telut_cur.Set(_telut, 0, send_rule);
      var_telst_cur.Set(_telst, 0, send_rule);
   }

   rv = getTelPos();
   if(rv < 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error updating telescope positions");
      return -1;
   }
   if(rv == 0)
   {
      var_telRA_cur.Set(_telRA, 0, send_rule);
      var_telDec_cur.Set(_telDec, 0, send_rule);
      var_telEpoch_cur.Set(_telEpoch, 0, send_rule);
      var_telHA_cur.Set(_telHA, 0, send_rule);
      var_telAM_cur.Set(_telAM, 0, send_rule);
      var_telRotOff_cur.Set(_telRotOff, 0, send_rule);
   }

   rv = getTelData();
   if(rv < 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error updating telescope data");
      return -1;
   }
   if(rv == 0)
   {
      var_telROI_cur.Set(_telROI, 0, send_rule);
      var_telTracking.Set(_telTracking, 0, send_rule);
      var_telGuiding.Set(_telGuiding, 0, send_rule);
      var_telSlewing.Set(_telSlewing, 0, send_rule);
      var_telGuiderMoving.Set(_telSlewing, 0, send_rule);
      var_telAz_cur.Set(_telAz, 0, send_rule);
      var_telEl_cur.Set(_telEl, 0, FORCE_SEND);
      var_telZd_cur.Set(_telZd, 0, send_rule);
      var_telPA_cur.Set(_telPA, 0, send_rule);
      var_telDomeAz_cur.Set(_telDomeAz, 0, send_rule);
      var_telDomeStat_cur.Set(_telDomeStat, 0, send_rule);
   }

//    std::string rotstr;
//    
//    rotstr = getMagTelStatus("telguide");
// 
//    std::cout << rotstr << "\n";
   rv = getVaneData();
   if(rv < 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error updating telescope secondary set positions");
      return -1;
   }

   if(rv == 0)
   {
      //load the secondary positions in to vectors for transfer to RTDB
      std::vector<double> veset(6);
      std::vector<double> veenc(6);

      veset[0] = _telSecX;
      veset[1] = _telSecY;
      veset[2] = _telSecZ;
      veset[3] = _telSecH;
      veset[4] = _telSecV;
      veset[5] = 0.0; //_telSecTz - always 0.0

      veenc[0] = _telEncX;
      veenc[1] = _telEncY;
      veenc[2] = _telEncZ;
      veenc[3] = _telEncH;
      veenc[4] = _telEncV;
      veenc[5] = 0.0; //_telEncTz - always 0.0

      var_telSec_SetPos.Set(veset, send_rule);
      var_telSec_EncPos.Set(veenc, send_rule);


      //Also fill in the hexapod var, with either set or encoder values.
      if(hexapod_source == 1)
      {
         var_telHex_AbsPos.Set(veenc, send_rule);
      }
      else
      {
         var_telHex_AbsPos.Set(veset, send_rule);
      }
   }

   rv = getTelRotEncAng();
   if(rv < 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error updating Rotator Encoder Angle");
      //return -1;
   }
   if(rv == 0) var_telRotAng_cur.Set(_telRotEncAng, 0, send_rule);


   rv = getCatData();
   if(rv < 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error updating catalog data");
      return -1;
   }
   if(rv == 0)
   {
      var_catra.Set(_catRA, 0, send_rule);
      var_catdec.Set(_catDec, 0, send_rule);
      var_catep.Set(_catEp, 0, send_rule);
      var_catro.Set(_catRo, 0, send_rule);
      var_catrm.Set(_catRm, send_rule);
      var_catobj.Set(_catObj, send_rule);
   }

   rv = getTelEnv();
   if(rv < 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error updating environment data");
      return -1;
   }
   if(rv == 0)
   {
      var_wxtemp.Set(_wxtemp, 0, send_rule);
      var_wxpres.Set(_wxpres, 0, send_rule);
      var_wxhumid.Set(_wxhumid, 0, send_rule);
      var_wxwind.Set(_wxwind, 0, FORCE_SEND);

      var_wxdewpoint.Set(_wxdewpoint, 0, send_rule);
      var_wxpwvest.Set(_wxpwvest, 0, send_rule);

      var_shellwind.Update();
      var_shellwind.Get(&_shellwind);

      //var_ambwind.Set(_wxwind*(5280./3600.*0.3048), 0, FORCE_SEND);
      var_ambwind.Set(_shellwind, 0, FORCE_SEND);

      var_wxwdir.Set(_wxwdir, 0, send_rule);
      var_ttruss.Set(_ttruss, 0, send_rule);
      var_tcell.Set(_tcell, 0, send_rule);
      var_tseccell.Set(_tseccell, 0, send_rule);
      var_tambient.Set(_tambient, 0, send_rule);

      var_dimmfwhm.Set(_wx_dimm_fwhm, 0, send_rule);
      var_dimmtime.Set(_wx_dimm_time, 0, send_rule);

      var_mag1fwhm.Set(_wx_mag1_fwhm, 0, send_rule);
      var_mag1time.Set(_wx_mag1_time, 0, send_rule);

      var_mag2fwhm.Set(_wx_mag2_fwhm, 0, send_rule);
      var_mag2time.Set(_wx_mag2_time, 0, send_rule);

   }


   

   var_lbtswa.Set(1, 0, FORCE_SEND);

   rv = getDateTime();

   return 0;
}//int MagAOI::updateMagTelStatus()

int MagAOI::getDateTime()
{
   double  h,m,s;

   std::vector<std::string> ddat;
   std::string dtstr;
   
   dtstr = getMagTelStatus(DATETIME);

   if(dtstr == "")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting telescope date and time");
      return -1;
   }

   ddat = parse_teldata(dtstr);

   if(ddat[0] == "-1")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting date and time (datetime): TCS returned -1");
      return 1;
   }

   if(ddat.size() != 3)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting date and time (datetime): TCS response wrong size");
      return 2;
   }

   _dateobs = ddat[0];

   if(parse_xms(ddat[1].c_str(), &h, &m, &s) != 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error parsing telescope UT");
      return -1;
   }

   _telut = (int)(h*3600. + m*60. + s);

   if(parse_xms(ddat[2].c_str(), &h, &m, &s) != 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error parsing telescope ST");
      return -1;
   }

   _telst = (int)(h*3600. + m*60. + s);

   return 0;
}//int MagAOI::getDateTime()

int MagAOI::getTelPos()
{
   double  h,m,s;

   std::vector<std::string> pdat;
   std::string posstr;
   
   posstr = getMagTelStatus(TELPOS);

   if(posstr == "")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting telescope position(telpos)");
      return -1;
   }

   pdat = parse_teldata(posstr);

   if(pdat[0] == "-1")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting telescope position (telpos): TCS returned -1");
      return 1;
   }

   if(pdat.size() != 6)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting telescope position (telpos): TCS response wrong size");
      return 2;
   }

   if(parse_xms(pdat[0].c_str(), &h, &m, &s) != 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error parsing telescope RA");
      return -1;
   }

   _telRA = (h + m/60. + s/3600.)*15.;

   //if(h < 0) _telRA *= -1;
   
   if(parse_xms(pdat[1], &h, &m, &s) != 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error parsing telescope Dec");
      return -1;
   }
   
   _telDec = h + m/60. + s/3600.;

   //if(h < 0) _telDec *= -1;
   
   _telEl = strtod(pdat[1].c_str(),0);// * 3600.;

   _telEpoch = strtod(pdat[2].c_str(),0);

   if(parse_xms(pdat[3], &h, &m, &s) != 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error parsing telescope HA");
      return -1;
   }

   /************ BUG: this won't handle -0!
   */
   _telHA = h + m/60. + s/3600.;
   //if(h < 0) _telHA *= -1;


   _telAM = strtod(pdat[4].c_str(),0);

   _telRotOff = strtod(pdat[5].c_str(),0);

   return 0;
}//int MagAOI::getTelPos()

int MagAOI::getTelData()
{
   std::string xstr;
   std::vector<std::string> tdat;

   xstr = getMagTelStatus(TELDATA);

   if(xstr == "")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting telescope data (teldata)");
      return -1;
   }

   tdat = parse_teldata(xstr);

   if(tdat[0] == "-1")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting telescope data (teldata): TCS returned -1");
      return 1;
   }

   if(tdat.size() != 10)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting telescope data (teldata): TCS response wrong size");
      return 2;
   }


   _telROI = atoi(tdat[0].c_str());

   //Parse the telguide string
   char bit[2];
   bit[1] = 0;
   bit[0] = tdat[1].c_str()[0];
   _telTracking= atoi(bit);
   bit[0] = tdat[1].c_str()[1];
   _telGuiding= atoi(bit);

   //parse the gdrmountmv string
   bit[0] = tdat[2].c_str()[0];
   _telSlewing= atoi(bit);
   bit[0] = tdat[2].c_str()[1];
   _telGuiderMoving = atoi(bit);

   //number 3 is mountmv

   _telAz = strtod(tdat[4].c_str(),0);

   _telEl = strtod(tdat[5].c_str(),0);

   _telZd = strtod(tdat[6].c_str(),0);// * 3600.;

   _telPA = strtod(tdat[7].c_str(),0);

   _telDomeAz = strtod(tdat[8].c_str(),0);

   _telDomeStat = atoi(tdat[9].c_str());

   return 0;
}//int MagAOI::getTelData()

int MagAOI::getCatData()
{
   double h, m,s;

   std::vector<std::string> cdat;
   std::string cstr;
   
   cstr = getMagTelStatus(CATDATA);
   
   if(cstr == "")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting catalog data (catdata)");
      return -1;
   }

   cdat = parse_teldata(cstr);

   if(cdat[0] == "-1")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting catalog data (catdata): TCS returned -1");
      return 1;
   }

   if(cdat.size() != 6)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting catalog data (catdata): TCS response wrong size");
      return 2;
   }

   if(parse_xms(cdat[0].c_str(), &h, &m, &s) != 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error parsing catalg RA");
      return -1;
   }

   _catRA = (h + m/60. + s/3600.)*15.;
   
   if(parse_xms(cdat[1].c_str(), &h, &m, &s) != 0)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error parsing catalg Dec");
      return -1;
   }
   
   _catDec = h + m/60. + s/3600.;

   _catEp = strtod(cdat[2].c_str(),0);

   _catRo = strtod(cdat[3].c_str(),0);

   _catRm = cdat[4];

   _catObj = cdat[5];
   
   return 0;
}//int MagAOI::getCatData()

int MagAOI::getVaneData()
{
   std::string xstr;
   std::vector<std::string> vedat;

   xstr = getMagTelStatus(VEDATA);
   
   if(xstr == "")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting telescope secondary positions (vedata)");
      return -1;
   }

   vedat = parse_teldata(xstr);

   if(vedat[0] == "-1")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting telescope secondary positions (vedata): TCS returned -1");
      return 1;
   }

   if(vedat.size() != 10)
   {
      //std::cout << "vedata\n";
      //std::cout << vedat.size() << "\n";
      //std::cout << vedat[0] << "\n";
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting telescope secondary positions (vedata): TCS response wrong size");
      //Logger::get()->log(Logger::LOG_LEV_INFO, "vedata TCS response: %s\nParsed size: %i", xstr.c_str(), vedat.size());
      return 2;
   }


   _telSecZ = strtod(vedat[0].c_str(),0);
   _telEncZ = strtod(vedat[1].c_str(),0);
   _telSecX = strtod(vedat[2].c_str(),0);
   _telEncX = strtod(vedat[3].c_str(),0);
   _telSecY = strtod(vedat[4].c_str(),0);
   _telEncY = strtod(vedat[5].c_str(),0);
   _telSecH = strtod(vedat[6].c_str(),0);
   _telEncH = strtod(vedat[7].c_str(),0);
   _telSecV = strtod(vedat[8].c_str(),0);
   _telEncV = strtod(vedat[9].c_str(),0);
   return 0;
}//int MagAOI::getVaneData()

int MagAOI::getTelRotEncAng()
{
   std::string rotstr;
   
   rotstr = getMagTelStatus(ROTATORE);
   
   if(rotstr == "")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting telescope Rotator Encoder Angle");
      _telRotEncAng = 999.;
      return -1;
   }


   _telRotEncAng = strtod(rotstr.c_str(),0);

   return 0;
}//int MagAOI::getTelRotEncAng()

/*int MagAOI::getRotFollowing()
{
   std::string followstr;
   
   followstr = getMagTelStatus(ROTMODE);
   
   if(followstr == "")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "Error getting rotator following status");
      _rotFollowing = 0;
      return -1;
   }
   
   
   _rotFollowing= -1;//atoi(followstr.c_str());

   //Should validate this as 0 or 1
   return 0;
}//int MagAOI::getRotFollowing()*/

int MagAOI::getTelEnv()
{
   std::vector<std::string> edat;
   std::string estr;

 //return 0;

   estr = getMagTelStatus(TELENV);

   if(estr == "")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting environment data (telenv)");
      return -1;
   }

   edat = parse_teldata(estr);

   if(edat[0] == "-1")
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting environment data (telenv): TCS returned -1");
      return 1;
   }

   if(edat.size() != 10)
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting environment data (telenv): TCS response wrong size");
      return 2;
   }

   _wxtemp = strtod(edat[0].c_str(), 0);
   _wxpres = strtod(edat[1].c_str(), 0);
   _wxhumid = strtod(edat[2].c_str(), 0);
   _wxwind = strtod(edat[3].c_str(), 0);
   _wxwdir = strtod(edat[4].c_str(), 0);
   _ttruss = strtod(edat[5].c_str(), 0);
   _tcell = strtod(edat[6].c_str(), 0);
   _tseccell = strtod(edat[7].c_str(), 0);
   _tambient = strtod(edat[8].c_str(), 0);
   _wxdewpoint = strtod(edat[9].c_str(),0);
   //std::cout << strtod(edat[9].c_str(),0) << "\n";;
   getPWV();

   getDIMM();

   return 0;
}//int MagAOI::getTelEnv()

int MagAOI::getPWV()
{

   //First get Dewpoint.  This will eventually be replaced by a TCS call
   //For now it's from a shell script.
   /*std::ifstream fin;
   std::string label, tdpstr, rhstr;

   fin.open("/tmp/wxdump");

   if(fin.fail())
   {
      Logger::get()->log(Logger::LOG_LEV_ERROR, "[TCS] Error getting environment data (wget wxdump)");
      _wxdewpoint = 999.;
      _wxpwvest = 999.;
      return -1;
   }


   fin >> label;
   fin >> tdpstr;

   fin.close();

   for(unsigned i=0;i<tdpstr.length();++i) if(tdpstr[i] == '"' || tdpstr[i] == ',') tdpstr[i] = ' ';

   _wxdewpoint = strtod(tdpstr.c_str(), 0);

   std::cout << _wxdewpoint << "\n";
   *///Now calculate PWV
   //The reference for this is Butler, "Precipitable Water at the VLA -- 1990-1998", VLA Scientific Memo. No. 237, 1998

   double amu = 1.66053829e-27; //AMU in kg
   double kb = 1.3806488e-23; //Boltzmann's constant
   double mw = 18.0*amu; //mass of water in kg
   double pl = 1./1000. * (100.*100.*100.); //density of water in kg/m^3

   double a, b, c,P0;//Partial pressure of water vapor
   //P0 = 100.*exp(1.81 + 17.27*_wxdewpoint/(_wxdewpoint+273.3)); //from the VLA memo

   /* constants used in the VLA memo
    * a = 6.11045
      b = 17.27
      c = 273.3
    */

   //Constants from Buck (1981)
   if(_wxdewpoint >= 0)
   {
      a = 6.1121;
      b = 17.368;
      c = 238.88;
   }
   else
   {
      a = 6.1121;
      b = 17.966;
      c = 247.15;
   }
   //Magnus formula for vapor pressure.
   P0 = 100. * (_wxhumid/100.0) * a*exp(b*_wxtemp/(_wxtemp + c));

   //double H = 1.75*1000.; //A compromise between 1 and 2 km from the memo.
    
   _wxpwvest = _pwv0 + (_Hbar*1000.0)*(mw*P0 / (pl*kb*(_wxtemp+273.15)))*1000.;
   
   return 0;
}//int MagAOI::getPWV()

int MagAOI::getDIMM()
{
   static int last_query = 0;

   time_t sec_midnight;
   time_t dt;

   if(time(0) - last_query > DIMMquery_interval)
   {
      system("/home/aosup/bin/seedump.sh");
      last_query = time(0);
      sec_midnight = last_query % 86400;

      std::ifstream fin;
      std::string label, datestr, timestr, fwhmstr;
      double dimmel, mag1el, h, m,s;

      /* process DIMM */
      fin.open("/tmp/dimmdump");

      if(fin.fail())
      {
         Logger::get()->log(Logger::LOG_LEV_ERROR, "[DIMM] Error getting DIMM seeing (seedump.sh)");
         _wx_dimm_fwhm = 0.;
        return -1;
      }

      fin >> label;
      fin >> label;
      fin >> label;
      fin >> datestr;
      fin >> timestr;

      fin >> _wx_dimm_fwhm;
      fin >> dimmel;
      fin.close();

      


      parse_xms(timestr, &h, &m, &s);

      _wx_dimm_time = (int) (h*3600 + m*60 + s + 0.5);

      dt = sec_midnight - _wx_dimm_time;
      if(dt < 0) dt = 86400-dt;

      if(dt > 300 || _wx_dimm_fwhm <= 0)
      {
         _wx_dimm_fwhm = -1;
      }
      else
      {
         _wx_dimm_fwhm *= pow(cos( (90.-dimmel)*DTOR), 3./5.);
      }

      std::ostringstream see1;
      see1 << "[SEEING] Retrieved DIMM seeing: " << _wx_dimm_fwhm << " " << _wx_dimm_time << "\n";
      Logger::get()->log(Logger::LOG_LEV_INFO,see1.str());


      /* process Baade seeing */
      fin.open("/tmp/mag1dump");

      if(fin.fail())
      {
         Logger::get()->log(Logger::LOG_LEV_ERROR, "[SEEING] Error getting Baade seeing (seedump.sh)");
         _wx_mag1_fwhm = 0.;
        return -1;
      }

      fin >> label;
      fin >> label;
      fin >> label;
      fin >> datestr;
      fin >> timestr;

      fin >> _wx_mag1_fwhm;
      fin >> mag1el;
      fin.close();

      parse_xms(timestr, &h, &m, &s);

      _wx_mag1_time = (int) (h*3600 + m*60 + s + 0.5);

      dt = sec_midnight - _wx_mag1_time;
      if(dt < 0) dt = 86400-dt;

      if(dt > 300 || _wx_mag1_fwhm <= 0)
      {
         _wx_mag1_fwhm = -1;
      }
      else
      {
         _wx_mag1_fwhm *= pow(cos( (90.-mag1el)*DTOR), 3./5.);
      }

      std::ostringstream see2;
      see2 << "[SEEING] Retrieved BAADE seeing: " << _wx_mag1_fwhm << " " << _wx_mag1_time << "\n";
      Logger::get()->log(Logger::LOG_LEV_INFO,see2.str());


      /* process Clay seeing */
      fin.open("/tmp/mag2dump");

      if(fin.fail())
      {
         Logger::get()->log(Logger::LOG_LEV_ERROR, "[SEEING] Error getting Clay seeing (seedump.sh)");
         _wx_mag2_fwhm = -1;
        return -1;
      }

      fin >> label;
      fin >> label;
      fin >> label;
      fin >> datestr;
      fin >> timestr;

      fin >> _wx_mag2_fwhm;
      fin >> mag1el;
      fin.close();

      parse_xms(timestr, &h, &m, &s);

      _wx_mag2_time = (int) (h*3600 + m*60 + s + 0.5);

      dt = sec_midnight - _wx_mag2_time;
      if(dt < 0) dt = 86400-dt;

      if(dt > 300 || _wx_mag2_fwhm <= 0)
      {
         _wx_mag2_fwhm = -1;
      }
      else
      {
         _wx_mag2_fwhm *= pow(cos( (90.-mag1el)*DTOR), 3./5.);
      }

      std::ostringstream see3;
      see3 << "[SEEING] Retrieved Clay seeing: " << _wx_mag2_fwhm << " " << _wx_mag2_time << "\n";
      Logger::get()->log(Logger::LOG_LEV_INFO,see3.str());

      
   }

   return 0;
}//int MagAOI::getDIMM()

/**********************************************************************/
/*                                                                    */
/*                                                                    */
/*                                                                    */
/*                    *** Offloading ***                              */
/*                                                                    */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

int  MagAOI::arbalert_hndl(MsgBuf* msg __attribute__((unused)), void* app __attribute__((unused)), int i __attribute__((unused)))    
{
   std::ostringstream o;

   o << "Received arbitrator alert";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   return 0;
}

int  MagAOI::default_hndl(MsgBuf* msg __attribute__((unused)), void* app __attribute__((unused)), int i __attribute__((unused)))     
{
   std::ostringstream o;

   o << "Default Handler";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   return 0;
}

int  MagAOI::hexapod_hndl(MsgBuf* msg __attribute__((unused)), void* app __attribute__((unused)), int i __attribute__((unused)))   
{
   std::ostringstream o;

   o << "Hexapod Handler";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   return 0;
}


int MagAOI::offload_hndl(MsgBuf* msg, void* argp, int nItms)    
{
   int i;
   double *pt;
   float zernike[OFL_ZERN_NUMB];
   bool ignore=false;
      
   MagAOI *app = (MagAOI*)argp;
      
   if(nItms>0) ignore=true;     // If nItms>0, there are more than one
                                // offload modes commands pending.
                               // Ignore all but the last
   pt= (double *) MSG_BODY(msg);
   for(i=0;i<OFL_ZERN_NUMB;i++)  zernike[i]=*(pt+i);
   thRelease(msg);              // Free the message buffer

   app->offload(zernike, ignore);
   

   return NO_ERROR;
}

#define OFFLOAD_TTX    1
#define OFFLOAD_TTY    2

#define OFFLOAD_FOCUS  3

#define OFFLOAD_COMAX  6
#define OFFLOAD_COMAY  7


void MagAOI::offload(float *z, bool ignore)
{
   int i;
   static int _ofld_ignored;
   char tcscomstr[256];
   
   bool TipTilt_threshold = false;
   bool Focus_threshold = false;
   bool Coma_threshold = false;
   bool higherord= false;

   double offaz, offel, offfoc, offx, offy, offv, offh, offax, offay;

   static time_t last_tt_move = time(0)-tt_move_delay;
   static time_t last_secz_move = time(0)-sec_move_delay;
   static time_t last_secxy_move = time(0)-sec_move_delay;
   static time_t last_m1_move = time(0)-m1_move_delay;

   std::ostringstream o;

   o << "[OFFL] Received offload: ";
   for(int i=0;i<OFL_ZERN_NUMB;i++) o << z[i] << " ";
   Logger::get()->log(Logger::LOG_LEV_INFO, o.str());

   
   /****** Here accumulate offloads in circular buffer*/
   
   oflCB.addEntry(time(0), z);
   
   /*Now access each offload by calling a circular buffer average function.
   *******/
   


   //Verify thresholds
   //Difference from LBT: low order includes Coma.
   
   TipTilt_threshold = false;
   
   if(fabs( oflCB.average(OFFLOAD_TTX, tt_avg_len) ) > offload_thresholds[OFFLOAD_TTX] ) TipTilt_threshold = true;
   if(fabs( oflCB.average(OFFLOAD_TTY, tt_avg_len) ) > offload_thresholds[OFFLOAD_TTY] ) TipTilt_threshold = true;
   


   Focus_threshold = false;
   if(fabs( oflCB.average(OFFLOAD_FOCUS, focus_avg_len) ) > offload_thresholds[OFFLOAD_FOCUS]*threshold_scale_focus ) Focus_threshold = true;


   Coma_threshold = false;
   if(fabs( oflCB.average(OFFLOAD_COMAX, coma_avg_len) ) > offload_thresholds[OFFLOAD_COMAX]*threshold_scale_focus ) Coma_threshold = true;
   
   if(fabs( oflCB.average(OFFLOAD_COMAY, coma_avg_len) ) > offload_thresholds[OFFLOAD_COMAY]*threshold_scale_focus ) Coma_threshold = true;
      

   
   
   //Difference from LBT: higher order starts with Astigmatism.
   for(i=4;i<OFL_MODES;i++)
   {
      if(i == 6 || i == 7) continue;
      if(fabs(z[i])>offload_thresholds[i]) higherord=true;
   }


   if((!TipTilt_threshold) && (!Focus_threshold) && (!Coma_threshold) && (!higherord) )
   {
      Logger::get()->log(Logger::LOG_LEV_INFO, "[OFFL] All offloads below threshold");
      return;
   }

   if(ignore)
   {            // Count ignored requests
      Logger::get()->log(Logger::LOG_LEV_INFO, "[OFFL] Offload ignored.");
      _ofld_ignored++;
      return;
   }

   if(_ofld_ignored>0)
   {
      std::ostringstream o;
      o << "[OFFL] " <<  _ofld_ignored << " previous offload requests were ignored";
      Logger::get()->log(Logger::LOG_LEV_INFO, o.str());
      _ofld_ignored=0;
   }

   if(TipTilt_threshold)
   {
      double ttx = oflCB.average(OFFLOAD_TTX, tt_avg_len); //z[1]
      double tty = oflCB.average(OFFLOAD_TTY, tt_avg_len); //z[2]
      offaz = TTgain*(Tmat[0][0] * ttx + Tmat[0][1] * tty);
      offel = TTgain*(Tmat[1][0] * ttx + Tmat[1][1] * tty);
      if(fabs(offaz) < TELTOL) offaz = 0;
      if(fabs(offel) < TELTOL) offel = 0;

      std::ostringstream t1;
      t1 << "[OFFL] LO Offload request. Tip:" << z[1] << " Tilt:" << z[2];
      Logger::get()->log(Logger::LOG_LEV_INFO,t1.str());

      //If we are above the threshold and it has been longer than tt_move_delay since last T/T offload
      //then apply the offset
      if(time(0) - last_tt_move >= tt_move_delay)
      {
         std::ostringstream t2;
         t2 << "[OFFL] Sending AZ/EL Offsets " << offaz << "," << offel;
         Logger::get()->log(Logger::LOG_LEV_INFO, t2.str());

         snprintf(tcscomstr, 256 , "aeg %f %f", offaz, offel);
      
         int tcsrv = sendMagTelCommand(tcscomstr, 1000);
         last_tt_move = time(0); 
         if(tcsrv != 0)
         {
            std::ostringstream err;
            err << "[OFFL] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
            Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
         }
         else Logger::get()->log(Logger::LOG_LEV_INFO, "[OFFL] TCS returned 0.");
      }
      else
      {
         Logger::get()->log(Logger::LOG_LEV_INFO, "[OFFL] TT offload request within tt_move_delay.");
      }
   }

   if(Focus_threshold)
   {

      double focus = oflCB.average(OFFLOAD_FOCUS, focus_avg_len);// z[3]
      offfoc = Fscale * secscalez * Fgain * focus; //z[3];
 

      if(time(0) - last_secz_move >= sec_move_delay)
      {
         focus_off_cum += offfoc;
         focus_off_last = offfoc;

         std::ostringstream t1;
         t1 << "[OFFL] LO Offload request. Foc:" << z[3];
         Logger::get()->log(Logger::LOG_LEV_INFO,t1.str());
      
         std::ostringstream t2;
         t2 << "[OFFL] Sending Focus secondary offset " << offfoc;
         Logger::get()->log(Logger::LOG_LEV_INFO, t2.str());

         t2.str("");
         t2 << "[OFFL] Cumulative Focus secondary offset: " << focus_off_cum;
         Logger::get()->log(Logger::LOG_LEV_INFO, t2.str());

         //Changed to zimr from zstr to update the IMA values, rather than SET values. 2015/05/26 -- JRM
         snprintf(tcscomstr, 256, "zimr %f", offfoc);
         //snprintf(tcscomstr, 256, "zstr %f", offfoc);

         int tcsrv = sendMagTelCommand(tcscomstr, 10000);
         focus_off_time = time(0);

         if(tcsrv != 0)
         {
            std::ostringstream err;
            err << "[OFFL] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
            Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
         }
         else Logger::get()->log(Logger::LOG_LEV_INFO, "[OFFL] TCS returned 0.");


         last_secz_move = (time_t) focus_off_time;


         var_focus_off_cum.Set(focus_off_cum,0, FORCE_SEND);
         var_focus_off_last.Set(focus_off_last, 0, FORCE_SEND);
         var_focus_off_time.Set(focus_off_time, 0, FORCE_SEND);

      }
      else
      {
         std::ostringstream t2;
         t2 << "[OFFL] Focus offload request within sec_move_delay " << offfoc;
         Logger::get()->log(Logger::LOG_LEV_INFO, t2.str());
      } 
   }

   if(Coma_threshold)
   {
      double coma_x = oflCB.average(OFFLOAD_COMAX, coma_avg_len);
      double coma_y = oflCB.average(OFFLOAD_COMAY, coma_avg_len);
      
      offx = Cgain*(Cmat[0][0] * coma_x + Cmat[0][1] * coma_y);
      offy = Cgain*(Cmat[1][0] * coma_x + Cmat[1][1] * coma_y);

      offv = offx*secscalev;
      offx *= secscalex;

      offh = offy*secscaleh;
      offy *= secscaley;

      if(fabs(offx) < TELTOL) offx = 0;
      if(fabs(offv) < TELTOL) offv = 0;
      if(fabs(offy) < TELTOL) offy = 0;
      if(fabs(offh) < TELTOL) offh = 0;

      std::ostringstream t1;
      t1 << "[OFFL] LO Offload request. Coma-X:" << z[6] << " Coma-Y:" << z[7];
      Logger::get()->log(Logger::LOG_LEV_INFO,t1.str());

      if(time(0) - last_secxy_move >= sec_move_delay)
      {

         coma_off_cum_x += offx;
         coma_off_cum_y += offy;
         coma_off_cum_v += offv;
         coma_off_cum_h += offh;


         std::ostringstream t2;
         t2 << "[OFFL] Sending Coma secondary offsets X=" << offx << ", V=" << offv << ", Y=" << offy << ", H=" << offh;
         Logger::get()->log(Logger::LOG_LEV_INFO, t2.str());

         t2.str("");
         t2 << "[OFFL] Cumulative Coma secondary offsets X=" << coma_off_cum_x << ", V=" << coma_off_cum_v;
         t2 << ", Y=" << coma_off_cum_y << ", H=" << coma_off_cum_h;
         Logger::get()->log(Logger::LOG_LEV_INFO, t2.str());

         //-------Send XVIR
         snprintf(tcscomstr, 256, "xvir %f %f", offx, offv);
      
         int tcsrv = 0;//sendMagTelCommand(tcscomstr); 
         if(tcsrv != 0)
         {
            std::ostringstream err;
            err << "[OFFL] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
            Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
         }
         else Logger::get()->log(Logger::LOG_LEV_INFO, "[OFFL] TCS returned 0.");

         //------Send yhir
         snprintf(tcscomstr, 256, "yhir %f %f", offy, offh);
         tcsrv = 0;//sendMagTelCommand(tcscomstr);
         if(tcsrv != 0)
         {
            std::ostringstream err;
            err << "[OFFL] TCS returned error " << tcsrv << " i.r.t. " << tcscomstr;
            Logger::get()->log(Logger::LOG_LEV_ERROR, err.str());
         }
         else Logger::get()->log(Logger::LOG_LEV_INFO, "[OFFL] TCS returned 0.");


         last_secxy_move = time(0);
      }
      else
      {
         std::ostringstream t2;
         t2 << "[OFFL] Coma offload request within sec_move_delay.";
         Logger::get()->log(Logger::LOG_LEV_INFO, t2.str());
      }

      var_coma_off_cum_x.Set(coma_off_cum_x, 0, FORCE_SEND);
      var_coma_off_cum_y.Set(coma_off_cum_y, 0, FORCE_SEND);
      var_coma_off_cum_v.Set(coma_off_cum_v, 0, FORCE_SEND);
      var_coma_off_cum_h.Set(coma_off_cum_h, 0, FORCE_SEND);

   }
   
   if(higherord)
   {
      //This includes gain, scale, parity, and rotation
      offax = Amat[0][0] * z[4] + Amat[0][1] * z[5];
      offay = Amat[1][0] * z[4] + Amat[1][1] * z[5];

      if(fabs(offax) > Amax)
      {
         Logger::get()->log(Logger::LOG_LEV_INFO, "[OFFL] Astig-X request truncated to Amax");
         offax = offax * (Amax/fabs(offax));
      }

      if(fabs(offay) > Amax)
      {
         Logger::get()->log(Logger::LOG_LEV_INFO, "[OFFL] Astig-Y request truncated to Amax");
         offay = offay * (Amax/fabs(offay));
      }

      std::vector<double> bvecs(13,0);

      bvecs[0] = offax;
      bvecs[1] = offay;

      std::ostringstream t1;
      t1 << "[OFFL] HO Offload request. Astig-X: " << z[4] << " Astig-y:" << z[5];
      
      Logger::get()->log(Logger::LOG_LEV_INFO,t1.str());

      //std::cout << time(0) << "\n";
      //std::cout << last_m1_move << "\n";
      //std::cout << m1_move_delay << "\n";

      if(time(0) - last_m1_move > m1_move_delay)
      {

         std::ostringstream t2;
         t2 << "[OFFL] Sending M1 Offsets " << offax << "," << offay;
         Logger::get()->log(Logger::LOG_LEV_INFO, t2.str());

         //Update RTDB.
         var_M1_coeffs.Set(bvecs, FORCE_SEND);

         last_m1_move = time(0);
      }
      else
      {
         Logger::get()->log(Logger::LOG_LEV_INFO, "[OFFL] Not Sending M1 Offsets (m1_move_delay)");
      }
   }

}//void MagAOI::offload(float *z, bool ignore)


int MagAOI::focus_cum_reset_req(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;

   magaoi->focus_cum_reset();

   return 0;
}

void MagAOI::focus_cum_reset()
{
   focus_off_cum = 0.;
   var_focus_off_cum.Set(focus_off_cum, 0, FORCE_SEND);
}

int MagAOI::coma_cum_reset_req(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;

   magaoi->coma_cum_reset();

   return 0;
}

void MagAOI::coma_cum_reset()
{
   coma_off_cum_x = 0;
   var_coma_off_cum_x.Set(coma_off_cum_x, 0, FORCE_SEND);

   coma_off_cum_y = 0;
   var_coma_off_cum_y.Set(coma_off_cum_y, 0, FORCE_SEND);

   coma_off_cum_v = 0;
   var_coma_off_cum_v.Set(coma_off_cum_v, 0, FORCE_SEND);

   coma_off_cum_h = 0;
   var_coma_off_cum_h.Set(coma_off_cum_h, 0, FORCE_SEND);
}

void MagAOI::initialize_offload_cum()
{
   var_focus_off_cum.Set(0., 0, FORCE_SEND);
   var_coma_off_cum_x.Set(0., 0, FORCE_SEND);
   var_coma_off_cum_y.Set(0., 0, FORCE_SEND);
   var_coma_off_cum_v.Set(0., 0, FORCE_SEND);
   var_coma_off_cum_h.Set(0., 0, FORCE_SEND);
}


int MagAOI::offcollim_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * magaoi = (MagAOI *) pt;

   magaoi->set_offload_collimation();

   return 0;
}

void MagAOI::set_offload_collimation()
{
   int offcoll;

   var_offcollim.Update();
   var_offcollim.Get(&offcoll);

   std::ostringstream t2;
   
   if(offcoll != 1)
   {
      std::cout << "Offload collimation off\n";
      sec_move_delay = sec_move_delay_normal;
      focus_avg_len = focus_avg_len_normal;
      coma_avg_len = coma_avg_len_normal;
      
      threshold_scale_focus = 1.0;
      threshold_scale_coma = 1.0;
      
      t2 << "[OFFL] collimation mode off";
   }
   else
   {
      std::cout << "Offload collimation on\n";
      sec_move_delay = sec_move_delay_collim;
      focus_avg_len = focus_avg_len_collim;
      coma_avg_len = coma_avg_len_collim;
      
      threshold_scale_focus = threshold_scale_focus_collim;
      threshold_scale_coma = threshold_scale_coma_collim;
      
      t2 << "[OFFL] collimation mode on";
   }
   
   Logger::get()->log(Logger::LOG_LEV_INFO, t2.str());
}

/*
   

   

   
   if(_ofld_ignored>0) {
      std::ostringstream t1;
      t1 << _ofld_ignored << " previous offload requests were ignored";
      AOSwnglog(t1.str());
      _ofld_ignored=0;
   }
   
   oflm_status=0;
   if(tiptiltfocus) oflm_status += OFLM_TTFOC;
   if(higherord) oflm_status += OFLM_HIGH;
   
   telemetry.storeOflm(oflm_status,zernike);
   
   for(i=0;i<OFL_MODES;i++) aos.side[side].adsec.offload[i]=(double)zernike[i];
   
   PSFClient psf=PSFClient(NetworkConfig::getSide());
   
   if(tiptiltfocus) {
      float pos[6];
      pos[0]=0.0; pos[1]=0.0; pos[2]=zernike[3]; pos[3]=zernike[2]; pos[4]=zernike[1]; pos[5]=0.0;
      
      CSQHandle psfhandle=psf.secondarymirror.setShellOffload(pos,true);
      std::ostringstream t1;
      t1 << "TT Offload request. Tip:" << zernike[1] << " Tilt:" << zernike[2] << " Foc:" << zernike[3] ;
      AOSinfolog(t1.str());
      std::ostringstream t2;
      t2 << "setShellOffload("<<pos[0]<<","<<pos[1]<<","<<pos[2]<<","<<pos[3]<<","<<pos[4]<<","<<pos[5]<< ",rel=true)";
      AOSdbglog(t2.str());
      psfhandle.block();
      CommandReturn result = psfhandle.getResult();
      if(result.isError()) {
         logTCSerror(result,"setShellOffload(). PSF says:");
      }
   }
   
   if(higherord) {
      zernike[0]=0.0;
      zernike[1]=0.0;
      zernike[2]=0.0;
      zernike[3]=0.0;
      std::ostringstream t1;
      CSQHandle psfhandle=psf.setZernikes(DEFAULT, 22, zernike);
      t1 << "HO Offload request. Z:";
      for(int i=0;i<OFL_MODES;i++) t1 << " " << zernike[i];
                     AOSinfolog(t1.str());
      
      psfhandle.block();
      CommandReturn result = psfhandle.getResult();
      if(result.isError()) {
         logTCSerror(result,"setZernikes(). PSF says:");
      }
   }
   AOSdbglog("Offload processing end @:"+tstamp());
}*/

int  MagAOI::housekeep_hndl(MsgBuf* msg __attribute__((unused)), void* app __attribute__((unused)), int i __attribute__((unused))) 
{
   std::ostringstream o;

   o << "housekeeper handler";
   Logger::get()->log(Logger::LOG_LEV_ERROR, o.str());

   return 0;
}

#ifdef OFFLOAD_TESTING


int MagAOI::testoffload_tt_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * m = (MagAOI *) pt;

   return m->send_testoffload(0);
}

int MagAOI::testoffload_foc_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * m = (MagAOI *) pt;

   return m->send_testoffload(1);
}

int MagAOI::testoffload_com_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * m = (MagAOI *) pt;

   return m->send_testoffload(2);
}

int MagAOI::testoffload_LO_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * m = (MagAOI *) pt;

   return m->send_testoffload(3);
}

int MagAOI::testoffload_astig_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * m = (MagAOI *) pt;

   return m->send_testoffload(4);
}

int MagAOI::testoffload_ALL_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   MagAOI * m = (MagAOI *) pt;

   return m->send_testoffload(5);
}

int MagAOI::send_testoffload(int type)
{
   float z[OFL_ZERN_NUMB];
   double ttx, tty, foc, comax, comay, astigx, astigy;

   for(int i=0;i<OFL_ZERN_NUMB; i++) z[i] = 0.;

   if(type == 0 || type == 3 || type ==5)
   {
      var_testoff_ttX.Update();
      var_testoff_ttX.Get(&ttx);
      var_testoff_ttY.Update();
      var_testoff_ttY.Get(&tty);
   }
   else
   {
      ttx = 0.;
      tty = 0.;
   }

   if(type == 1 || type == 3 || type == 5)
   {
      var_testoff_foc.Update();
      var_testoff_foc.Get(&foc);
   }
   else
   {
      foc = 0.;
   }

   if(type == 2 || type == 3 || type ==5)
   {
      var_testoff_comaX.Update();
      var_testoff_comaX.Get(&comax);
      var_testoff_comaY.Update();
      var_testoff_comaY.Get(&comay);
   }
   else
   {
      comax = 0.;
      comay = 0.;
   }

   if(type == 4 || type ==5)
   {
      var_testoff_astigX.Update();
      var_testoff_astigX.Get(&astigx);
      var_testoff_astigY.Update();
      var_testoff_astigY.Get(&astigy);
   }
   else
   {
      astigx = 0.;
      astigy = 0.;
   }

   //std::cout << "Sending test offload vector . . .\n";

   z[1] = ttx*1e-9;
   z[2] = tty*1e-9;
   z[3] = foc*1e-9;
   z[4] =  astigx*1e-9;
   z[5] =  astigy*1e-9;
   z[6] =  comax*1e-9;
   z[7] =  comay*1e-9;

   offload(z, false);

   return 0;
}

#endif //OFFLOAD_TESTING


void MagAOI::AOIinfoLog(std::string logmsg)
{
   Logger::get()->log(Logger::LOG_LEV_INFO, logmsg);
}


