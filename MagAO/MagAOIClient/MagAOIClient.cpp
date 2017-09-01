#include "MagAOIClient.h"

using namespace Arcetri;

//#define _debug

MagAOIClient::MagAOIClient( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
}

MagAOIClient::MagAOIClient( std::string name, const std::string & conffile) throw (AOException) : AOApp( name, conffile)
{
   Create();
}

void MagAOIClient::Create() throw (AOException)
{
   _rtdbSide = "L";
   init_AOI_DD(&aoi);
   
   LoadConfig();
}      

int MagAOIClient::LoadConfig()
{
   Config_File *cfg = &ConfigDictionary();

   try 
   {
      aoi.side.adsec.fl_filename = (std::string) (*cfg)["FlatFile"];
   }
   catch (Config_File_Exception &e)
   {
      //Do nothing.
   }

   return NO_ERROR;
}

void MagAOIClient::add_DD_RTVar(std::string base_name, std::string var_name, void * ddptr, int type, int len, bool log, bool update )
{
   std::string name = base_name;
   pair<map<std::string, DD_RTDBVar>::iterator,bool> ret;
   DD_RTDBVar ddel;

   name += ".";
   name += var_name;
     
   try
   {
      ddel.RTVar = RTDBvar(base_name, var_name,  NO_DIR, type, len, false); //Do not create
   }
   catch (AOVarException &e)
   {
      //std::cerr << strlen(e.what().c_str()) << "\n";
      //Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating AOI DD RTDB variable");
   }  
   ddel.DDVar = ddptr;
   ddel.log = log;
   ddel.update = update;
   
   ret = RTVars.insert(DD_RT_mapT(name, ddel));
   
   if(ret.second == false)
   {
      std::ostringstream oss;
      oss << "Could not insert: " << name << ". Must already be inserted.  Not notifying";
      Logger::get()->log(Logger::LOG_LEV_ERROR, oss.str());
      return;
   }
      
   Notify(ddel.RTVar, AOI_DD_var_changed);

}

void MagAOIClient::add_DD_RTVar(std::string base_name, std::string var_name, int * ddptr, int len, bool log, bool update)
{
   return add_DD_RTVar(base_name, var_name, (void *) ddptr, INT_VARIABLE, len, log, update);
}

void MagAOIClient::add_DD_RTVar(std::string base_name, std::string var_name, double * ddptr, int len, bool log, bool update)
{
   return add_DD_RTVar(base_name, var_name, (void *) ddptr, REAL_VARIABLE, len, log, update);
}

void MagAOIClient::add_DD_RTVar(std::string base_name, std::string var_name, std::string * ddptr, int len, bool log, bool update)
{
   return add_DD_RTVar(base_name, var_name, (void *) ddptr, CHAR_VARIABLE, len, log, update);
}

void MagAOIClient::SetupVars()
{
   std::string subsys = getenv("ADOPT_SUBSYSTEM");
   std::string wfsadd, adsadd;
   
   if(subsys == "WFS")
   {
      adsadd = "@M_ADSEC";
      Logger::get()->log(Logger::LOG_LEV_INFO, "Attaching to ADOPT_SUBSYSTEM: %s", subsys.c_str());
   }
   else if(subsys == "ADSEC")
   {
      wfsadd = "@M_MAGWFS";
      Logger::get()->log(Logger::LOG_LEV_INFO, "Attaching to ADOPT_SUBSYSTEM: %s", subsys.c_str());
   }
   else
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "Unknown or unset ADOPT_SUBSYSTEM environment variable. %s:%i", __FILE__, __LINE__);
      throw AOException("Unknown or unset ADOPT_SUBSYSTEM environment variable.");
   }

   add_DD_RTVar("AOS.TEL", "DATEOBS"+adsadd,  &aoi.dateobs);//
   add_DD_RTVar("AOS.TEL", "UT"+adsadd,  &aoi.ut);
   add_DD_RTVar("AOS.TEL", "ST"+adsadd,  &aoi.st);//

   add_DD_RTVar("AOS.TEL", "RA"+adsadd,  &aoi.ra);
   add_DD_RTVar("AOS.TEL", "DEC"+adsadd, &aoi.dec);
   add_DD_RTVar("AOS.TEL", "EPOCH"+adsadd,  &aoi.epoch);//
   add_DD_RTVar("AOS.TEL", "HA"+adsadd,  &aoi.ha);//
   add_DD_RTVar("AOS.TEL", "AM"+adsadd,  &aoi.am);

   add_DD_RTVar("AOS.L.ROTATOR", "ROI"+adsadd,  &aoi.rotator.roi);
   add_DD_RTVar("AOS.L.ROTATOR", "ANGLE"+adsadd,  &aoi.rotator.angle);
   add_DD_RTVar("AOS.L.ROTATOR", "OFFSET"+adsadd,  &aoi.rotator.offset);
   add_DD_RTVar("AOS.L.ROTATOR", "FOLLOWING"+adsadd,  &aoi.rotator.following);

   add_DD_RTVar("AOS.TEL", "ISTRACKING"+adsadd,  &aoi.istracking);
   add_DD_RTVar("AOS.TEL", "ISGUIDING"+adsadd,  &aoi.isguiding);
   add_DD_RTVar("AOS.TEL", "ISSLEWING"+adsadd,  &aoi.isslewing);
   add_DD_RTVar("AOS.TEL", "GUIDER.ISMOVING"+adsadd,  &aoi.guider_ismoving);

   add_DD_RTVar("AOS.TEL", "AZ"+adsadd,  &aoi.az.position);
   add_DD_RTVar("AOS.TEL", "EL"+adsadd,  &aoi.el.position);
   add_DD_RTVar("AOS.TEL", "ZD"+adsadd,  &aoi.zd);//
   add_DD_RTVar("AOS.TEL", "PA"+adsadd,  &aoi.pa);
   add_DD_RTVar("AOS.TEL", "DOME.AZ"+adsadd,  &aoi.dome.az);
   add_DD_RTVar("AOS.TEL", "DOME.STAT"+adsadd,  &aoi.dome.stat);

   add_DD_RTVar("AOS.TEL", "CATRA"+adsadd,  &aoi.cat.ra);
   add_DD_RTVar("AOS.TEL", "CATDEC"+adsadd,  &aoi.cat.dec);
   add_DD_RTVar("AOS.TEL", "CATEP"+adsadd,  &aoi.cat.epoch);
   add_DD_RTVar("AOS.TEL", "CATRO"+adsadd,  &aoi.cat.rotOff);
   add_DD_RTVar("AOS.TEL", "CATRM"+adsadd,  &aoi.cat.rotMode);
   add_DD_RTVar("AOS.TEL", "CATOBJ"+adsadd,  &aoi.cat.obj);
   add_DD_RTVar("AOS.TEL", "OBSINST"+adsadd,  &aoi.cat.obsinst);
   add_DD_RTVar("AOS.TEL", "OBSNAME"+adsadd,  &aoi.cat.obsname);

   add_DD_RTVar("AOS.L.HEXAPOD", "ABS_POS"+adsadd, &(aoi.hexapod.abs_pos[0]), 6);
   add_DD_RTVar("AOS.TEL", "VE.SET"+adsadd, &(aoi.hexapod.set_pos[0]), 6);
   add_DD_RTVar("AOS.TEL", "VE.ENC"+adsadd, &(aoi.hexapod.enc_pos[0]), 6);

   add_DD_RTVar("AOS.TEL", "WXTEMP"+adsadd,  &aoi.environ.wxtemp);//
   add_DD_RTVar("AOS.TEL", "WXPRES"+adsadd,  &aoi.environ.wxpres);//
   add_DD_RTVar("AOS.TEL", "WXHUMID"+adsadd,  &aoi.environ.wxhumid);//
   add_DD_RTVar("AOS.TEL", "WXWIND"+adsadd,  &aoi.environ.wxwind);//
   add_DD_RTVar("AOS.TEL", "WXWDIR"+adsadd,  &aoi.environ.wxwdir);//
   add_DD_RTVar("AOS.TEL", "WXDEWPOINT"+adsadd,  &aoi.environ.wxdewpoint);//
   add_DD_RTVar("AOS.TEL", "WXPWVEST"+adsadd,  &aoi.environ.wxpwvest);//

   add_DD_RTVar("AOS.TEL", "TTRUSS"+adsadd,  &aoi.environ.ttruss);
   add_DD_RTVar("AOS.TEL", "TCELL"+adsadd,  &aoi.environ.tcell);//
   add_DD_RTVar("AOS.TEL", "TSECCELL"+adsadd,  &aoi.environ.tseccell);
   add_DD_RTVar("AOS.TEL", "TAMBIENT"+adsadd,  &aoi.environ.tambient);

   add_DD_RTVar("AOS.DIMM", "FWHM"+adsadd,  &aoi.environ.dimmfwhm);
   add_DD_RTVar("AOS.DIMM", "TIME"+adsadd,  &aoi.environ.dimmtime);
   add_DD_RTVar("AOS.MAG1", "FWHM"+adsadd,  &aoi.environ.mag1fwhm);
   add_DD_RTVar("AOS.MAG1", "TIME"+adsadd,  &aoi.environ.mag1time);
   add_DD_RTVar("AOS.MAG2", "FWHM"+adsadd,  &aoi.environ.mag2fwhm);
   add_DD_RTVar("AOS.MAG2", "TIME"+adsadd,  &aoi.environ.mag2time);

   /*----------------------------------------------------------------------------*/
   //A bunch of RTDB variables we will keep local copies of.  From AOSAoApp.cpp
   string AOARB_S = "AOARB."+_rtdbSide;
   string AOARB_S_ADSEC = "AOARB."+_rtdbSide;
   string ADSEC_S = "ADSEC."+_rtdbSide;
   string ANEM_S = "ANEM."+_rtdbSide;
   string adsecarb_S = "adsecarb."+_rtdbSide;
   string adamhousekeeper_S ="adamhousekeeper."+_rtdbSide;
   string optloopdiag_S = "optloopdiag."+_rtdbSide;
   string wfsarb_S = "wfsarb."+_rtdbSide;
   string ccd39_S ="ccd39."+_rtdbSide;
   string ccd47_S ="ccd47."+_rtdbSide;
   string filterwheel1_S ="filterwheel1."+_rtdbSide;
   string filterwheel2_S ="filterwheel2."+_rtdbSide;
   string filterwheel3_S ="filterwheel3."+_rtdbSide;
   string slopecompctrl_S ="slopecompctrl."+_rtdbSide;
   string _tvImgVarName="wfsarb."+_rtdbSide+".TV_IMAGE";
   
   
   // The following variables are parameters related to the AO system
   add_DD_RTVar(AOARB_S, "AO_READY"+adsadd ,       &aoi.side.ao.ao_ready, 1, true);
   add_DD_RTVar(AOARB_S, "CORRECTEDMODES"+adsadd,  &aoi.side.ao.correctedmodes);
   
   add_DD_RTVar(AOARB_S, "FSM_STATE"+adsadd,       &aoi.side.ao.status, 50, true);
   add_DD_RTVar(AOARB_S, "LAB_MODE"+adsadd,        &aoi.side.labmode,1, true);
   add_DD_RTVar(AOARB_S, "LOOPON"+adsadd,          &aoi.side.ao.loopon,1, true);//
   add_DD_RTVar(AOARB_S, "MODE"+adsadd,            &aoi.side.ao.mode);//?
   add_DD_RTVar(AOARB_S, "MSG"+adsadd,             &aoi.side.ao.msg);
   add_DD_RTVar(AOARB_S, "OFL_ENABLED"+adsadd,     &aoi.side.ao.ofl_enabled);
   add_DD_RTVar(AOARB_S, "SL_READY"+adsadd,        &aoi.side.ao.sl_ready,1, true);
   add_DD_RTVar(AOARB_S, "STREHL"+adsadd,          &aoi.side.ao.strehl);//->from visao, wfe!!!!
   add_DD_RTVar(AOARB_S, "WFS_SOURCE"+adsadd,      &aoi.side.ao.wfs_source);
   add_DD_RTVar("ttctrl.L", "LOWAMP.CUR"+wfsadd,   &aoi.side.ao.tt_amp[0],3);
   add_DD_RTVar("ttctrl.L", "LOWFREQ.CUR"+wfsadd,  &aoi.side.ao.tt_freq[0], 3);
   add_DD_RTVar("ttctrl.L", "LOWOFF.CUR"+wfsadd,   &aoi.side.ao.tt_offset[0], 3);
   add_DD_RTVar("ADSEC.L", "G_GAIN_A"+adsadd,   &aoi.side.ao.loop_gains, 256);//need to parse or write file_name
   add_DD_RTVar(_rtdbSide, "IDL_STAT"+adsadd,     &aoi.side.idlstat,1, true);
   add_DD_RTVar("ADSEC.L", "B0_A"+adsadd, &aoi.side.ao.reconstructor, 256);//Reconstructor
   
   
   string gainsetter_s = "gainset." + _rtdbSide;
   add_DD_RTVar(gainsetter_s, "nmodes"+adsadd,   &aoi.side.ao.nmodes);
   add_DD_RTVar(gainsetter_s, "homiddle"+adsadd,   &aoi.side.ao.homiddle);
   add_DD_RTVar(gainsetter_s, "gain_tt.CUR"+adsadd,   &aoi.side.ao.gain_tt);
   add_DD_RTVar(gainsetter_s, "gain_ho1.CUR"+adsadd,   &aoi.side.ao.gain_ho1);
   add_DD_RTVar(gainsetter_s, "gain_ho2.CUR"+adsadd,   &aoi.side.ao.gain_ho2);
   
   
   // The following variables are parameters related to the AdSec
   //add_DD_RTVar(ANEM_S, "SPEED"+adsadd,                         &(aoi.side.adsec.anem_speed[0]), 12);
   //add_DD_RTVar(AOARB_S_ADSEC , "ANEM_UPD"+adsadd,              &aoi.side.adsec.anem_upd);
   //add_DD_RTVar(AOARB_S_ADSEC, "ELEV_UPD"+adsadd,               &aoi.side.adsec.elev_upd);
   add_DD_RTVar(AOARB_S_ADSEC, "ADSEC.HEALTH"+adsadd,                 &aoi.side.adsec.health,1, true);
   //add_DD_RTVar(ADSEC_S, "CONTAMINATION"+adsadd,                &aoi.side.adsec.contamination, 1 ,true);
   //add_DD_RTVar(ADSEC_S, "NWACT"+adsadd,                        &aoi.side.adsec.nwact, 1, true);
   //add_DD_RTVar(ADSEC_S, "POPMSG"+adsadd,                       &aoi.side.adsec.popmsg);
   add_DD_RTVar(ADSEC_S, "SHAPE"+adsadd,                        &aoi.side.adsec.shape);
   add_DD_RTVar(adamhousekeeper_S, "COIL_STATUS"+adsadd,        &aoi.side.adsec.coil_status,1, true);
   add_DD_RTVar(adamhousekeeper_S, "MAIN_POWER_STATUS"+adsadd,  &aoi.side.adsec.pwr_status, 1, true);
   add_DD_RTVar(adamhousekeeper_S, "TSS_STATUS"+adsadd,         &aoi.side.adsec.tss_status, 1, true);
   add_DD_RTVar(adsecarb_S, "FSM_STATE"+adsadd,                 &aoi.side.adsec.status, 50, true);
   add_DD_RTVar(adsecarb_S, "LED"+adsadd,                       &aoi.side.adsec.led,1, true);
   add_DD_RTVar(adsecarb_S, "MSG"+adsadd,                       &aoi.side.adsec.msg);
   add_DD_RTVar(adsecarb_S, "SAFESKIP_PERCENT"+adsadd,          &aoi.side.adsec.safeskip_perc);
   
   // The following variables are parameters related to Mag WFS
   add_DD_RTVar(AOARB_S, "FLAOWFS.HEALTH"+adsadd,      &aoi.side.wfs1.health);
   add_DD_RTVar(ccd39_S, "XBIN.CUR"+wfsadd,            &aoi.side.wfs1.ccdbin);
   add_DD_RTVar(ccd39_S, "FRMRT.CUR"+wfsadd,           &aoi.side.wfs1.ccdfreq);
   add_DD_RTVar(ccd47_S, "XBIN.CUR"+wfsadd,            &aoi.side.wfs1.tv_binning);
   add_DD_RTVar(ccd47_S, "FRMRT.CUR"+wfsadd,           &aoi.side.wfs1.tv_exptime);  
   add_DD_RTVar(filterwheel1_S, "POSNAME.CUR"+wfsadd,  &aoi.side.wfs1.filter1, 50);
   add_DD_RTVar(filterwheel1_S, "POS.CUR"+wfsadd,  &aoi.side.wfs1.filter1_pos);  
   add_DD_RTVar(filterwheel1_S, "POS.REQ"+wfsadd,  &aoi.side.wfs1.filter1_reqpos);

   add_DD_RTVar(filterwheel1_S, "STARTPOS"+wfsadd,  &aoi.side.wfs1.filter1_startpos);
   add_DD_RTVar(filterwheel1_S, "HOMINGPOS"+wfsadd,  &aoi.side.wfs1.filter1_homingpos);
   add_DD_RTVar(filterwheel1_S, "ABORTPOS"+wfsadd,  &aoi.side.wfs1.filter1_abortpos);

   add_DD_RTVar(filterwheel2_S, "POSNAME.CUR"+wfsadd,  &aoi.side.wfs1.tv_filter2, 50);
   add_DD_RTVar(filterwheel3_S, "POSNAME.CUR"+wfsadd,  &aoi.side.wfs1.tv_filter3, 50);
   
   add_DD_RTVar("rerotator.L", "POS.CUR"+wfsadd, &aoi.side.wfs1.rerotator_angle);

   add_DD_RTVar("baysidex.L", "POS.CUR"+wfsadd, &aoi.side.wfs1.baysidex);
   add_DD_RTVar("baysidex.L", "ENABLE.CUR"+wfsadd, &aoi.side.wfs1.baysidex_enabled);
   add_DD_RTVar("baysidey.L", "POS.CUR"+wfsadd, &aoi.side.wfs1.baysidey);
   add_DD_RTVar("baysidey.L", "ENABLE.CUR"+wfsadd, &aoi.side.wfs1.baysidey_enabled);
   add_DD_RTVar("baysidez.L", "POS.CUR"+wfsadd, &aoi.side.wfs1.baysidez);
   add_DD_RTVar("baysidez.L", "ENABLE.CUR"+wfsadd, &aoi.side.wfs1.baysidez_enabled);

   add_DD_RTVar("visao", "avgwfe"+wfsadd, &aoi.avgwfe);
   add_DD_RTVar("visao", "stdwfe"+wfsadd, &aoi.stdwfe);
   add_DD_RTVar("visao", "instwfe"+wfsadd, &aoi.instwfe);

   add_DD_RTVar(optloopdiag_S, "COUNTS"+wfsadd,        &aoi.side.wfs1.counts);
   add_DD_RTVar(slopecompctrl_S, "NSUBAPS.CUR"+wfsadd, &aoi.side.wfs1.no_subaps);
   add_DD_RTVar(wfsarb_S, "FSM_STATE"+wfsadd,          &aoi.side.wfs1.status,1,true);
   add_DD_RTVar(wfsarb_S, "LED"+wfsadd,                &aoi.side.wfs1.led,1,true);
   add_DD_RTVar(wfsarb_S, "MOD_AMPL"+wfsadd,           &aoi.side.wfs1.mod_ampl);
   add_DD_RTVar(wfsarb_S, "MSG"+wfsadd,                &aoi.side.wfs1.msg, 50);
   add_DD_RTVar(wfsarb_S, "PYRAMID_POS"+wfsadd,        &(aoi.side.wfs1.pyramid_pos[0]),2);

   add_DD_RTVar("MagAOI.cmds", "updateargs"+adsadd,     &aoi.side.cmds.updateargs);

   add_DD_RTVar("MagAOI.cmds", "PresetFlat.tmout"+adsadd, &aoi.side.cmds.presetflat.tmout);
   add_DD_RTVar("MagAOI.cmds", "PresetFlat.stat"+adsadd, &aoi.side.cmds.presetflat.stat);
   add_DD_RTVar("MagAOI.cmds", "PresetFlat.errMsg"+adsadd, &aoi.side.cmds.presetflat.errMsg);

   add_DD_RTVar("MagAOI.cmds", "PresetVisAO.tmout"+adsadd, &aoi.side.cmds.presetvisao.tmout);
   add_DD_RTVar("MagAOI.cmds", "PresetVisAO.stat"+adsadd, &aoi.side.cmds.presetvisao.stat);
   add_DD_RTVar("MagAOI.cmds", "PresetVisAO.errMsg"+adsadd, &aoi.side.cmds.presetvisao.errMsg);

   add_DD_RTVar("MagAOI.cmds", "PresetAO.tmout"+adsadd, &aoi.side.cmds.presetao.tmout);
   add_DD_RTVar("MagAOI.cmds", "PresetAO.stat"+adsadd, &aoi.side.cmds.presetao.stat);
   add_DD_RTVar("MagAOI.cmds", "PresetAO.errMsg"+adsadd, &aoi.side.cmds.presetao.errMsg);

   add_DD_RTVar("MagAOI.cmds", "AcquireFromGuider.tmout"+adsadd, &aoi.side.cmds.acqguider.tmout);
   add_DD_RTVar("MagAOI.cmds", "AcquireFromGuider.stat"+adsadd, &aoi.side.cmds.acqguider.stat);
   add_DD_RTVar("MagAOI.cmds", "AcquireFromGuider.errMsg"+adsadd, &aoi.side.cmds.acqguider.errMsg);

   add_DD_RTVar("MagAOI.cmds", "CheckRefAO.tmout"+adsadd, &aoi.side.cmds.checkref.tmout);
   add_DD_RTVar("MagAOI.cmds", "CheckRefAO.stat"+adsadd, &aoi.side.cmds.checkref.stat);
   add_DD_RTVar("MagAOI.cmds", "CheckRefAO.errMsg"+adsadd, &aoi.side.cmds.checkref.errMsg);
   add_DD_RTVar("MagAOI.cmds", "CheckRefAO.result"+adsadd, &(aoi.side.cmds.checkref.result[0]), 3);

   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.tmout"+adsadd, &aoi.side.cmds.acquireref.tmout);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.stat"+adsadd, &aoi.side.cmds.acquireref.stat);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.errMsg"+adsadd, &aoi.side.cmds.acquireref.errMsg);

   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.deltaX"+adsadd, &aoi.side.ao.param.dx);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.deltaY"+adsadd, &aoi.side.ao.param.dy);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.slNull"+adsadd, &aoi.side.ao.param.slnull);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.f1spec"+adsadd, &aoi.side.ao.param.filter1);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.f2spec"+adsadd, &aoi.side.ao.param.filter2);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.freq"+adsadd, &aoi.side.ao.param.freq);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.gain"+adsadd, &aoi.side.ao.param.gain);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.starMag"+adsadd, &aoi.side.ao.param.mag);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.nBins"+adsadd, &aoi.side.ao.param.nbins);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.nModes"+adsadd, &aoi.side.ao.param.nmodes);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.r0"+adsadd, &aoi.side.ao.param.r0);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.snmode"+adsadd, &aoi.side.ao.param.snmode);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.strehl"+adsadd, &aoi.side.ao.param.strehl);
   add_DD_RTVar("MagAOI.cmds", "AcquireRefAO.result.ttMod"+adsadd, &aoi.side.ao.param.ttmodul);

   add_DD_RTVar("MagAOI.cmds", "RefineAO.tmout"+adsadd, &aoi.side.cmds.refine.tmout);
   add_DD_RTVar("MagAOI.cmds", "RefineAO.stat"+adsadd, &aoi.side.cmds.refine.stat);
   add_DD_RTVar("MagAOI.cmds", "RefineAO.errMsg"+adsadd,   &aoi.side.cmds.refine.errMsg);

   add_DD_RTVar("MagAOI.cmds", "ModifyAO.tmout"+adsadd, &aoi.side.cmds.modify.tmout);
   add_DD_RTVar("MagAOI.cmds", "ModifyAO.stat"+adsadd, &aoi.side.cmds.modify.stat);
   add_DD_RTVar("MagAOI.cmds", "ModifyAO.errMsg"+adsadd, &aoi.side.cmds.modify.errMsg);

   add_DD_RTVar("MagAOI.cmds", "StartAO.tmout"+adsadd, &aoi.side.cmds.startao.tmout);
   add_DD_RTVar("MagAOI.cmds", "StartAO.stat"+adsadd, &aoi.side.cmds.startao.stat);
   add_DD_RTVar("MagAOI.cmds", "StartAO.errMsg"+adsadd, &aoi.side.cmds.startao.errMsg);

   add_DD_RTVar("MagAOI.cmds", "OffsetXY.tmout"+adsadd,   &aoi.side.cmds.offsetxy.tmout);
   add_DD_RTVar("MagAOI.cmds", "OffsetXY.stat"+adsadd,   &aoi.side.cmds.offsetxy.stat);
   add_DD_RTVar("MagAOI.cmds", "OffsetXY.errMsg"+adsadd, &aoi.side.cmds.offsetxy.errMsg);

   add_DD_RTVar("MagAOI.cmds", "NodRaDec.InProgress"+adsadd, &aoi.nodInProgress);

   add_DD_RTVar("MagAOI.cmds", "OffsetZ.tmout"+adsadd, &aoi.side.cmds.offsetz.tmout);
   add_DD_RTVar("MagAOI.cmds", "OffsetZ.stat"+adsadd, &aoi.side.cmds.offsetz.stat);
   add_DD_RTVar("MagAOI.cmds", "OffsetZ.errMsg"+adsadd, &aoi.side.cmds.offsetz.errMsg);

   add_DD_RTVar("MagAOI.cmds", "Stop.tmout"+adsadd, &aoi.side.cmds.stop.tmout);
   add_DD_RTVar("MagAOI.cmds", "Stop.stat"+adsadd, &aoi.side.cmds.stop.stat);
   add_DD_RTVar("MagAOI.cmds", "Stop.errMsg"+adsadd, &aoi.side.cmds.stop.errMsg);

   add_DD_RTVar("MagAOI.cmds", "Pause.tmout"+adsadd, &aoi.side.cmds.pause.tmout);
   add_DD_RTVar("MagAOI.cmds", "Pause.stat"+adsadd, &aoi.side.cmds.pause.stat);
   add_DD_RTVar("MagAOI.cmds", "Pause.errMsg"+adsadd, &aoi.side.cmds.pause.errMsg);

   add_DD_RTVar("MagAOI.cmds", "Resume.tmout"+adsadd, &aoi.side.cmds.resume.tmout);
   add_DD_RTVar("MagAOI.cmds", "Resume.stat"+adsadd, &aoi.side.cmds.resume.stat);
   add_DD_RTVar("MagAOI.cmds", "Resume.errMsg"+adsadd, &aoi.side.cmds.resume.errMsg);

   add_DD_RTVar("MagAOI.cmds", "UserPanic.tmout"+adsadd, &aoi.side.cmds.userpanic.tmout);
   add_DD_RTVar("MagAOI.cmds", "UserPanic.stat"+adsadd, &aoi.side.cmds.userpanic.stat);
   add_DD_RTVar("MagAOI.cmds", "UserPanic.errMsg"+adsadd, &aoi.side.cmds.userpanic.errMsg);

   add_DD_RTVar("MagAOI.cmds", "SetNewInstrument.tmout"+adsadd, &aoi.side.cmds.userpanic.tmout);
   add_DD_RTVar("MagAOI.cmds", "SetNewInstrument.stat"+adsadd, &aoi.side.cmds.userpanic.stat);
   add_DD_RTVar("MagAOI.cmds", "SetNewInstrument.errMsg"+adsadd, &aoi.side.cmds.userpanic.errMsg);
   
   add_DD_RTVar("MagAOI.cmds", "WfsOn.tmout"+adsadd, &aoi.side.cmds.wfson.tmout);
   add_DD_RTVar("MagAOI.cmds", "WfsOn.stat"+adsadd, &aoi.side.cmds.wfson.stat);
   add_DD_RTVar("MagAOI.cmds", "WfsOn.errMsg"+adsadd, &aoi.side.cmds.wfson.errMsg);

   add_DD_RTVar("MagAOI.cmds", "WfsOff.tmout"+adsadd, &aoi.side.cmds.wfsoff.tmout);
   add_DD_RTVar("MagAOI.cmds", "WfsOff.stat"+adsadd, &aoi.side.cmds.wfsoff.stat);
   add_DD_RTVar("MagAOI.cmds", "WfsOff.errMsg"+adsadd, &aoi.side.cmds.wfsoff.errMsg);

   add_DD_RTVar("MagAOI.cmds", "AdsecOn.tmout"+adsadd, &aoi.side.cmds.adsecon.tmout);
   add_DD_RTVar("MagAOI.cmds", "AdsecOn.stat"+adsadd, &aoi.side.cmds.adsecon.stat);
   add_DD_RTVar("MagAOI.cmds", "AdsecOn.errMsg"+adsadd, &aoi.side.cmds.adsecon.errMsg);

   add_DD_RTVar("MagAOI.cmds", "AdsecOff.tmout"+adsadd, &aoi.side.cmds.adsecoff.tmout);
   add_DD_RTVar("MagAOI.cmds", "AdsecOff.stat"+adsadd, &aoi.side.cmds.adsecoff.stat);
   add_DD_RTVar("MagAOI.cmds", "AdsecOff.errMsg"+adsadd, &aoi.side.cmds.adsecoff.errMsg);

   add_DD_RTVar("MagAOI.cmds", "AdsecSet.tmout"+adsadd, &aoi.side.cmds.adsecset.tmout);
   add_DD_RTVar("MagAOI.cmds", "AdsecSet.stat"+adsadd, &aoi.side.cmds.adsecset.stat);
   add_DD_RTVar("MagAOI.cmds", "AdsecSet.errMsg"+adsadd, &aoi.side.cmds.adsecset.errMsg);

   add_DD_RTVar("MagAOI.cmds", "AdsecRest.tmout"+adsadd, &aoi.side.cmds.adsecrest.tmout);
   add_DD_RTVar("MagAOI.cmds", "AdsecRest.stat"+adsadd, &aoi.side.cmds.adsecrest.stat);
   add_DD_RTVar("MagAOI.cmds", "AdsecRest.errMsg"+adsadd, &aoi.side.cmds.adsecrest.errMsg);

   
   /*add_DD_RTVar( &aoi.side.cmds..stat);
   add_DD_RTVar( &aoi.side.cmds..errMsg);

   add_DD_RTVar( &aoi.side.cmds..stat);
   add_DD_RTVar( &aoi.side.cmds..errMsg);*/
   
   /** \todo Need way to handle tv images */
   add_DD_RTVar(_tvImgVarName, "", &aoi.side.wfs1.tvImgVar, BIT8_VARIABLE, 65536, false, false);   
   
   //Setup the AOI Command RTDB variables
   try
   {

      /*** PresetFlat ***/
      MagAOI_cmds_PresetFlat_command = RTDBvar("MagAOI.cmds", "PresetFlat"+adsadd,  NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_PresetFlat_arg_flatSpec = RTDBvar("MagAOI.cmds", "PresetFlat.arg.flatSpec"+adsadd, NO_DIR, CHAR_VARIABLE, 256,0);

      /*** PresetVisAO ***/
      MagAOI_cmds_PresetVisAO_command = RTDBvar("MagAOI.cmds", "PresetVisAO"+adsadd,              NO_DIR, INT_VARIABLE, 1,0);

      /*** PresetAO ***/
      MagAOI_cmds_PresetAO_command      = RTDBvar("MagAOI.cmds", "PresetAO"+adsadd,              NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_PresetAO_arg_AOmode   = RTDBvar("MagAOI.cmds", "PresetAO.arg.AOmode"+adsadd,   NO_DIR, CHAR_VARIABLE, 50,0);
      MagAOI_cmds_PresetAO_arg_wfsSpec  = RTDBvar("MagAOI.cmds", "PresetAO.arg.wfsSpec"+adsadd,  NO_DIR, CHAR_VARIABLE, 50,0);
      MagAOI_cmds_PresetAO_arg_rocoordx = RTDBvar("MagAOI.cmds", "PresetAO.arg.rocoordx"+adsadd, NO_DIR, REAL_VARIABLE, 1,0);
      MagAOI_cmds_PresetAO_arg_rocoordy = RTDBvar("MagAOI.cmds", "PresetAO.arg.rocoordy"+adsadd, NO_DIR, REAL_VARIABLE, 1,0);
      MagAOI_cmds_PresetAO_arg_mag      = RTDBvar("MagAOI.cmds", "PresetAO.arg.mag"+adsadd,      NO_DIR, REAL_VARIABLE, 1,0);
      MagAOI_cmds_PresetAO_arg_cindex   = RTDBvar("MagAOI.cmds", "PresetAO.arg.cindex"+adsadd,   NO_DIR, REAL_VARIABLE, 1,0);
      MagAOI_cmds_PresetAO_arg_adcTracking   = RTDBvar("MagAOI.cmds", "PresetAO.arg.adcTracking"+adsadd,   NO_DIR, INT_VARIABLE, 1,0);

      /*** AcquireFromGuider ***/
      MagAOI_cmds_AcquireFromGuider_command = RTDBvar("MagAOI.cmds", "AcquireFromGuider",              NO_DIR, INT_VARIABLE, 1,0);

      /*** NudgeXY ***/
      MagAOI_cmds_NudgeXY_command = RTDBvar("MagAOI.cmds", "NudgeXY",  NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_NudgeXY_arg_x = RTDBvar("MagAOI.cmds", "NudgeXY.arg.x",  NO_DIR, REAL_VARIABLE, 1,0);
      MagAOI_cmds_NudgeXY_arg_y = RTDBvar("MagAOI.cmds", "NudgeXY.arg.y",  NO_DIR, REAL_VARIABLE, 1,0);

      /*** NudgeAE ***/
      MagAOI_cmds_NudgeAE_command = RTDBvar("MagAOI.cmds", "NudgeAE",  NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_NudgeAE_arg_a = RTDBvar("MagAOI.cmds", "NudgeAE.arg.a",  NO_DIR, REAL_VARIABLE, 1,0);
      MagAOI_cmds_NudgeAE_arg_e = RTDBvar("MagAOI.cmds", "NudgeAE.arg.e",  NO_DIR, REAL_VARIABLE, 1,0);


      /*** CheckRefAO ***/
      MagAOI_cmds_CheckRefAO_command = RTDBvar("MagAOI.cmds", "CheckRefAO"+adsadd,        NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_CheckRefAO_result  = RTDBvar("MagAOI.cmds", "CheckRefAO.result"+adsadd, NO_DIR, REAL_VARIABLE, 3,0);

      MagAOI_cmds_AcquireRefAO_command = RTDBvar("MagAOI.cmds", "AcquireRefAO"+adsadd,        NO_DIR, INT_VARIABLE, 1,0);

      /*** RefineAO ***/
      MagAOI_cmds_RefineAO_command =    RTDBvar("MagAOI.cmds", "RefineAO"+adsadd,            NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_RefineAO_arg_method = RTDBvar("MagAOI.cmds", "RefineAO.arg.method"+adsadd, NO_DIR, CHAR_VARIABLE, 50,0);

      MagAOI_cmds_ModifyAO_command    = RTDBvar("MagAOI.cmds", "ModifyAO"+adsadd,            NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_ModifyAO_arg_nModes = RTDBvar("MagAOI.cmds", "ModifyAO.arg.nModes"+adsadd, NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_ModifyAO_arg_freq   = RTDBvar("MagAOI.cmds", "ModifyAO.arg.freq"+adsadd,   NO_DIR, REAL_VARIABLE, 1,0);
      MagAOI_cmds_ModifyAO_arg_nBins  = RTDBvar("MagAOI.cmds", "ModifyAO.arg.nBins"+adsadd,  NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_ModifyAO_arg_ttMod  = RTDBvar("MagAOI.cmds", "ModifyAO.arg.ttMod"+adsadd,  NO_DIR, REAL_VARIABLE, 1,0);
      MagAOI_cmds_ModifyAO_arg_f1     = RTDBvar("MagAOI.cmds", "ModifyAO.arg.f1"+adsadd,     NO_DIR, CHAR_VARIABLE, 256,0);
      MagAOI_cmds_ModifyAO_arg_f2     = RTDBvar("MagAOI.cmds", "ModifyAO.arg.f2"+adsadd,     NO_DIR, CHAR_VARIABLE, 256,0);
      MagAOI_cmds_ModifyAO_arg_f3     = RTDBvar("MagAOI.cmds", "ModifyAO.arg.f3"+adsadd,     NO_DIR, CHAR_VARIABLE, 256,0);
     
      /*** StartAO ***/
      MagAOI_cmds_StartAO_command = RTDBvar("MagAOI.cmds", "StartAO"+adsadd,        NO_DIR, INT_VARIABLE, 1,0);

      /*** OffsetXY ***/
      MagAOI_cmds_OffsetXY_command = RTDBvar("MagAOI.cmds", "OffsetXY"+adsadd,        NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_OffsetXY_arg_x   = RTDBvar("MagAOI.cmds", "OffsetXY.arg.x"+adsadd,  NO_DIR, REAL_VARIABLE, 1,0);
      MagAOI_cmds_OffsetXY_arg_y   = RTDBvar("MagAOI.cmds", "OffsetXY.arg.y"+adsadd,  NO_DIR, REAL_VARIABLE, 1,0);

      /*** OffsetZ ***/
      MagAOI_cmds_OffsetZ_command = RTDBvar("MagAOI.cmds", "OffsetZ"+adsadd,        NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_OffsetZ_arg_z   = RTDBvar("MagAOI.cmds", "OffsetZ.arg.z"+adsadd,  NO_DIR, REAL_VARIABLE, 1,0);

      /*** Focus ***/
      MagAOI_cmds_Focus_command = RTDBvar("MagAOI.cmds", "Focus"+adsadd,        NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_Focus_arg_z   = RTDBvar("MagAOI.cmds", "Focus.arg.z"+adsadd,  NO_DIR, REAL_VARIABLE, 1,0);

      /*** NodRaDec ***/
      MagAOI_cmds_NodRaDec_command = RTDBvar("MagAOI.cmds", "NodRaDec"+adsadd,        NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_NodRaDec_stat = RTDBvar("MagAOI.cmds", "NodRaDec.stat"+adsadd,        NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_NodRaDec_arg_ra   = RTDBvar("MagAOI.cmds", "NodRaDec.arg.ra"+adsadd,  NO_DIR, REAL_VARIABLE, 1,0);
      MagAOI_cmds_NodRaDec_arg_dec   = RTDBvar("MagAOI.cmds", "NodRaDec.arg.dec"+adsadd,  NO_DIR, REAL_VARIABLE, 1,0);

      /*** Stop ***/
      MagAOI_cmds_Stop_command = RTDBvar("MagAOI.cmds", "Stop"+adsadd,         NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_Stop_arg_msg = RTDBvar("MagAOI.cmds", "Stop.arg.msg"+adsadd, NO_DIR, CHAR_VARIABLE, 256,0);

      /*** Pause ***/
      MagAOI_cmds_Pause_command = RTDBvar("MagAOI.cmds", "Pause"+adsadd,         NO_DIR, INT_VARIABLE, 1,0);

      /*** Resume ***/
      MagAOI_cmds_Resume_command = RTDBvar("MagAOI.cmds", "Resume"+adsadd,         NO_DIR, INT_VARIABLE, 1,0);

      /*** UserPanic ***/
      MagAOI_cmds_UserPanic_command = RTDBvar("MagAOI.cmds", "UserPanic"+adsadd,         NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_UserPanic_arg_reason = RTDBvar("MagAOI.cmds", "UserPanic.arg.reason"+adsadd, NO_DIR, CHAR_VARIABLE, 256,0);

      /*** SetNewInstrument ***/
      MagAOI_cmds_SetNewInstrument_command = RTDBvar("MagAOI.cmds", "SetNewInstrument"+adsadd,         NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_SetNewInstrument_arg_instr = RTDBvar("MagAOI.cmds", "SetNewInstrument.arg.instr"+adsadd, NO_DIR, CHAR_VARIABLE, 256,0);
      MagAOI_cmds_SetNewInstrument_arg_focalSt = RTDBvar("MagAOI.cmds", "SetNewInstrument.arg.focalSt"+adsadd, NO_DIR, CHAR_VARIABLE, 256,0);
      
      /*** WfsOn ***/
      MagAOI_cmds_WfsOn_command = RTDBvar("MagAOI.cmds", "WfsOn"+adsadd,         NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_WfsOn_arg_WfsID = RTDBvar("MagAOI.cmds", "WfsOn.arg.WfsID"+adsadd, NO_DIR, CHAR_VARIABLE, 256,0);

      /*** WfsOff ***/
      MagAOI_cmds_WfsOff_command = RTDBvar("MagAOI.cmds", "WfsOff"+adsadd,         NO_DIR, INT_VARIABLE, 1,0);
      MagAOI_cmds_WfsOff_arg_WfsID = RTDBvar("MagAOI.cmds", "WfsOff.arg.WfsID"+adsadd, NO_DIR, CHAR_VARIABLE, 256,0);

      /*** AdsecOn ***/
      MagAOI_cmds_AdsecOn_command = RTDBvar("MagAOI.cmds", "AdsecOn"+adsadd,         NO_DIR, INT_VARIABLE, 1,0);

      /*** AdsecOff ***/
      MagAOI_cmds_AdsecOff_command = RTDBvar("MagAOI.cmds", "AdsecOff"+adsadd,NO_DIR, INT_VARIABLE, 1,0);

      /*** AdsecSet ***/
      MagAOI_cmds_AdsecSet_command = RTDBvar("MagAOI.cmds", "AdsecSet"+adsadd,         NO_DIR, INT_VARIABLE, 1,0);

      /*** AdsecRest ***/
      MagAOI_cmds_AdsecRest_command = RTDBvar("MagAOI.cmds", "AdsecRest"+adsadd,         NO_DIR, INT_VARIABLE, 1,0);

      /*** AOArb Stop Command ***/
      AOARB_cmds_Stop_command = RTDBvar("AOARB.L", "STOPCMD",   NO_DIR, INT_VARIABLE, 1);
   }
   catch (AOVarException &e)
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating AOI Command RTDB variables");
   }
   
}

void MagAOIClient::init_DD()
{
   std::map<std::string, DD_RTDBVar>::iterator it;

   for(it = RTVars.begin(); it!=RTVars.end(); it++)
   {
      update_DD_var(it->second.RTVar.complete_name(), it->second.RTVar.type(), false);
   }
}

void MagAOIClient::Run()
{
   _logger->log( Logger::LOG_LEV_INFO, "Running...");

   init_DD();
   
   while(!TimeToDie()) 
   {
      try 
      {
         DoFSM();
      } 
      catch (AOException &e) 
      {
         _logger->log( Logger::LOG_LEV_ERROR, "Caught exception (at %s:%d): %s", __FILE__, __LINE__, e.what().c_str());
                 
         sleep(1);
      }
   }
}
      
    
int MagAOIClient::DoFSM()
{
   std::ostringstream oss;
   
   setCurState( STATE_OPERATING);

   sleep(3);
      
   return NO_ERROR;
}

int MagAOIClient::AOI_DD_var_changed(void *pt, Variable *msgb)
{
   MagAOIClient * magaoiC = (MagAOIClient *) pt;

   return magaoiC->update_DD_var(msgb);
}

int MagAOIClient::update_DD_var(Variable *msgb)
{
   std::ostringstream oss;
   
   #ifdef _debug
   //std::cout << "(1)Got change notification: " << msgb->H.Name << "\n";
   #endif

   return update_DD_var(msgb->H.Name, msgb->H.Type);
   
}

int MagAOIClient::update_DD_var(std::string name, int Type, bool post)
{
   std::ostringstream oss;
    
   
   DD_RTDBVar changed = RTVars[name];

   #ifdef _debug
   //std::cout << "(2)Got change notification: " << changed.RTVar.complete_name() << "\n";
   #endif

   if(changed.update)
   {
      try
      {
         changed.RTVar.Update();
      }
      catch(AOVarException e)
      {
         oss.str("");
         oss << e.what() << " (" << __FILE__ << " " << __LINE__ << ")\n";
         Logger::get()->log(Logger::LOG_LEV_ERROR, oss.str());
         #ifdef _debug
         std::cout << "caught exception " << e.what() << "\n";
         #endif
         return 0;
      }
      if(Type != changed.RTVar.type())
      {
         oss.str("");
         oss << "Variable types do not match for " << name << " in update_DD_var.  got type: ";
         oss << Type << " expected type: " << changed.RTVar.type();
         Logger::get()->log(Logger::LOG_LEV_ERROR, oss.str());
         return -1;
      }
      
      switch(Type)
      {
         case INT_VARIABLE:
            for(int i=0; i<changed.RTVar.len(); i++)
            {
               changed.RTVar.Get(i, &((int32_t *)changed.DDVar)[i]);
            }
            break;
         case REAL_VARIABLE:
            for(int i=0; i<changed.RTVar.len(); i++)
            {
               changed.RTVar.Get(i, &((double *)changed.DDVar)[i]);
            }
            break;
         case CHAR_VARIABLE:
            //std::cout << changed.RTVar.Get() << "\n";
            *((std::string *) changed.DDVar) = changed.RTVar.Get();
            break;
         case BIT8_VARIABLE:
            for(int i=0; i<changed.RTVar.len(); i++)
            {
               changed.RTVar.Get(i, &((uint8_t *)changed.DDVar)[i]);
            }
            break;
         case BIT16_VARIABLE:
            for(int i=0; i<changed.RTVar.len(); i++)
            {
               changed.RTVar.Get(i, &((uint16_t *)changed.DDVar)[i]);
            }
            break;
         case BIT32_VARIABLE:
            for(int i=0; i<changed.RTVar.len(); i++)
            {
               changed.RTVar.Get(i, &((uint32_t *)changed.DDVar)[i]);
            }
            break;
         case BIT64_VARIABLE:
            for(int i=0; i<changed.RTVar.len(); i++)
            {
               changed.RTVar.Get(i, &((uint64 *)changed.DDVar)[i]);
            }
            break;
         default:
            std::cerr << "Unkown Type\n";
            return -1;
      }
   }

   if(post) post_update_DD_var(changed);
   
   return 0;
}

void MagAOIClient::post_update_DD_var(DD_RTDBVar &var)
{
   var.RTVar.type();
   return;
}

/****************** Arbitrator Interface ****************************/

int MagAOIClient::PresetFlat(string flatSpec)
{
   MagAOI_cmds_PresetFlat_arg_flatSpec.Set(flatSpec, FORCE_SEND);
   MagAOI_cmds_PresetFlat_command.Set(1, 0,FORCE_SEND);
   
   return 0;
};

int MagAOIClient::PresetVisAO()
{
   MagAOI_cmds_PresetVisAO_command.Set(1, 0, FORCE_SEND);
   return 0;
}

int MagAOIClient::PresetAO(string AOmode, string wfsSpec, double rocoordx, double rocoordy, double mag, double cindex, bool adcTracking)
{
   
   MagAOI_cmds_PresetAO_arg_AOmode.Set(AOmode, FORCE_SEND);
   MagAOI_cmds_PresetAO_arg_wfsSpec.Set(wfsSpec, FORCE_SEND);
   MagAOI_cmds_PresetAO_arg_rocoordx.Set(rocoordx,0, FORCE_SEND);
   MagAOI_cmds_PresetAO_arg_rocoordy.Set(rocoordy,0, FORCE_SEND);
   MagAOI_cmds_PresetAO_arg_mag.Set(mag, 0, FORCE_SEND);
   MagAOI_cmds_PresetAO_arg_cindex.Set(cindex,0, FORCE_SEND);
   MagAOI_cmds_PresetAO_arg_adcTracking.Set(adcTracking,0, FORCE_SEND);

   MagAOI_cmds_PresetAO_command.Set(1, 0, FORCE_SEND);

   aoi.side.cmds.presetao.wfsSpec = wfsSpec;
   aoi.side.cmds.presetao.rocoordx = rocoordx;
   aoi.side.cmds.presetao.rocoordy = rocoordy;
   aoi.side.cmds.presetao.mag = mag;
   aoi.side.cmds.presetao.cindex = cindex;
   aoi.side.cmds.presetao.adcTracking = adcTracking;

   return 0;
};

int MagAOIClient::AcquireFromGuider()
{
   MagAOI_cmds_AcquireFromGuider_command.Set(1, 0, FORCE_SEND);

   return 0;
}

int MagAOIClient::NudgeXY(double x, double y)
{
   MagAOI_cmds_NudgeXY_arg_x.Set(x, 0, FORCE_SEND);
   MagAOI_cmds_NudgeXY_arg_y.Set(y, 0, FORCE_SEND);

   MagAOI_cmds_NudgeXY_command.Set(1, 0, FORCE_SEND);

   return 0;
}

int MagAOIClient::NudgeAE(double a, double e)
{
   MagAOI_cmds_NudgeAE_arg_a.Set(a, 0, FORCE_SEND);
   MagAOI_cmds_NudgeAE_arg_e.Set(e, 0, FORCE_SEND);

   MagAOI_cmds_NudgeAE_command.Set(1, 0, FORCE_SEND);

   return 0;
}

int MagAOIClient::CheckRefAO()
{
   MagAOI_cmds_CheckRefAO_command.Set(1,0,FORCE_SEND);
   
   return 0;
};

int MagAOIClient::AcquireRefAO(bool rePoint __attribute__((unused)))
{
   
   MagAOI_cmds_AcquireRefAO_command.Set(1,0,FORCE_SEND);

   return 0;
}
   
int MagAOIClient::RefineAO(string method)
{
   MagAOI_cmds_RefineAO_arg_method.Set(method, FORCE_SEND);
   
   MagAOI_cmds_RefineAO_command.Set(1, 0, FORCE_SEND);

   return 0;
   
};

int MagAOIClient::ModifyAO(int nModes, double freq, int nBins, double ttMod, string f1, string f2, string f3)
{
   
   MagAOI_cmds_ModifyAO_arg_nModes.Set(nModes, 0, FORCE_SEND);
   MagAOI_cmds_ModifyAO_arg_freq.Set(freq, 0, FORCE_SEND);
   MagAOI_cmds_ModifyAO_arg_nBins.Set(nBins, 0, FORCE_SEND);
   MagAOI_cmds_ModifyAO_arg_ttMod.Set(ttMod, 0, FORCE_SEND);
   MagAOI_cmds_ModifyAO_arg_f1.Set(f1, FORCE_SEND);
   MagAOI_cmds_ModifyAO_arg_f2.Set(f2, FORCE_SEND);
   MagAOI_cmds_ModifyAO_arg_f3.Set(f3, FORCE_SEND);
   
   MagAOI_cmds_ModifyAO_command.Set(1,0, FORCE_SEND);
   
   return 0;
}; //int MagAOIClient::ModifyAO(int nModes, double freq, int nBins, double ttMod, string f1, string f2)


int MagAOIClient::StartAO()
{

   MagAOI_cmds_StartAO_command.Set(1, 0, FORCE_SEND);
   
   return 0;
};//int MagAOIClient::AOI_OffsetXY_command_changed(void *pt, Variable *msgb)

int MagAOIClient::OffsetXY(double ofsX, double ofsY)
{
   
   MagAOI_cmds_OffsetXY_arg_x.Set(ofsX, 0, FORCE_SEND);
   MagAOI_cmds_OffsetXY_arg_y.Set(ofsY, 0, FORCE_SEND);
   
   MagAOI_cmds_OffsetXY_command.Set(1, 0, FORCE_SEND);
   return 0;
};//int MagAOIClient::OffsetXY(double ofsX, double ofsY)

int MagAOIClient::NodRaDec(double ra, double dec)
{
   
   MagAOI_cmds_NodRaDec_arg_ra.Set(ra, 0, FORCE_SEND);
   MagAOI_cmds_NodRaDec_arg_dec.Set(dec, 0, FORCE_SEND);
   
   MagAOI_cmds_NodRaDec_command.Set(1, 0, FORCE_SEND);

   int i=0;

   //First wait for the MagAOI to start the nod
   while(aoi.nodInProgress == 0 && i <= 25)
   {
      sleep(1);
      i++;
   }

   //Next wait for the node to end
   i=0;
   while(aoi.nodInProgress == 1 && i <= 25)
   {
      sleep(1);
      i++;
   }

   return 0;
};//int MagAOIClient::NodRaDec(double ofsX, double ofsY)

int MagAOIClient::OffsetZ(double ofsZ)
{
   MagAOI_cmds_OffsetZ_arg_z.Set(ofsZ, 0, FORCE_SEND);
   
   MagAOI_cmds_OffsetZ_command.Set(1, 0, FORCE_SEND);
   
   return 0;
};//int MagAOIClient::OffsetZ(double ofsZ)

int MagAOIClient::Focus(double ofsZ)
{
   MagAOI_cmds_Focus_arg_z.Set(ofsZ, 0, FORCE_SEND);
   
   MagAOI_cmds_Focus_command.Set(1, 0, FORCE_SEND);
   
   return 0;
};//int MagAOIClient::Focus(double ofsZ)


int MagAOIClient::Stop(string msg)
{
   MagAOI_cmds_Stop_arg_msg.Set(msg, FORCE_SEND);

   MagAOI_cmds_Stop_command.Set(1, 0, FORCE_SEND);
   
   return 0;
}//int MagAOIClient::Stop(string msg)

int MagAOIClient::Pause()
{
   MagAOI_cmds_Pause_command.Set(1, 0, FORCE_SEND);
   return 0;
}

int MagAOIClient::Resume()
{
   MagAOI_cmds_Resume_command.Set(1, 0, FORCE_SEND);
   return 0;
}

int MagAOIClient::UserPanic(string reason)
{
   MagAOI_cmds_UserPanic_arg_reason.Set(reason, FORCE_SEND);

   MagAOI_cmds_UserPanic_command.Set(1, 0, FORCE_SEND);
   
   return 0;
}

int MagAOIClient::SetNewInstrument(string instr, string focalSt)
{
   MagAOI_cmds_SetNewInstrument_arg_instr.Set(instr, FORCE_SEND);
   MagAOI_cmds_SetNewInstrument_arg_focalSt.Set(focalSt, FORCE_SEND);
   
   MagAOI_cmds_SetNewInstrument_command.Set(1, 0, FORCE_SEND);

   return 0;
}

int MagAOIClient::AdsecOn()
{
   MagAOI_cmds_AdsecOn_command.Set(1, 0, FORCE_SEND);
   return 0;
}

int MagAOIClient::AdsecOff()
{
   MagAOI_cmds_AdsecOff_command.Set(1, 0, FORCE_SEND);
   return 0;
}

int MagAOIClient::AdsecSet()
{
   MagAOI_cmds_AdsecSet_command.Set(1, 0, FORCE_SEND);
   return 0;
}

int MagAOIClient::AdsecRest()
{
   MagAOI_cmds_AdsecRest_command.Set(1, 0, FORCE_SEND);
   return 0;
}

int MagAOIClient::WfsOn(string wfsID)
{
   MagAOI_cmds_WfsOn_arg_WfsID.Set(wfsID, FORCE_SEND);

   MagAOI_cmds_WfsOn_command.Set(1, 0, FORCE_SEND);
   
   return 0;
}

int MagAOIClient::WfsOff(string wfsID)
{
   MagAOI_cmds_WfsOff_arg_WfsID.Set(wfsID, FORCE_SEND);

   MagAOI_cmds_WfsOff_command.Set(1, 0, FORCE_SEND);
   
   return 0;
}


int MagAOIClient::AOArbStop()
{
   AOARB_cmds_Stop_command.Set(1, 0, FORCE_SEND);

   return 0;
}

int MagAOIClient::updateObservers(std::string obsname, std::string obsinst)
{

   std::string subsys = getenv("ADOPT_SUBSYSTEM");
   std::string adsadd;
   
   if(subsys == "WFS")
   {
      adsadd = "@M_ADSEC";
   }

   std::string obsi_varn = std::string("AOS.TEL.OBSINST")+adsadd;
   std::string obsn_varn = std::string("AOS.TEL.OBSNAME")+adsadd;

   DD_RTDBVar obsn = RTVars[obsn_varn];
   DD_RTDBVar obsi = RTVars[obsi_varn];


   obsn.RTVar.Set(obsname);
   obsi.RTVar.Set(obsinst);

   return 0;
}


int MagAOIClient::parseLoopGains(double &tt, double &ho1, double &ho2)
{

   int p = aoi.side.ao.loop_gains.find("g_tt_", 0);

   if(p < 0 || p > (int)aoi.side.ao.loop_gains.length() - 4)
   {
      tt = 0.;
      ho1 = 0.;
      ho2 = 0.;
   }
   else
   {
            
      tt = strtod(aoi.side.ao.loop_gains.substr(p+5, 4).c_str(), 0);
      
      p = aoi.side.ao.loop_gains.find("_ho1_", 0);

      if(p < 0 || p > (int)aoi.side.ao.loop_gains.length() - 5)
      {
         ho1 = 0.;
      }
      else
      {
         ho1 = strtod(aoi.side.ao.loop_gains.substr(p+5, 4).c_str(), 0);
      }
            
      p = aoi.side.ao.loop_gains.find("_ho2_", p);

      if(p < 0 || p > (int)aoi.side.ao.loop_gains.length() - 5)
      {
         ho2 = 0.;
      }
      else
      {
         ho2 = strtod(aoi.side.ao.loop_gains.substr(p+5, 4).c_str(), 0);
      }
   }

   return 0;
}



/** \todo add more inits here
 */
void init_AOI_command_info(AOI_command_info *com)
{
   com->tmout = 0;
   com->stat = 0;

   com->aomode = AO_MODE_ACE;
   com->wfsSpec = "MAGWFS";
   com->cindex = 0;
   com->rocoordx = 0;
   com->rocoordy = 0;
   com->mag = 0;

   com->result[0] = 0;
   com->result[1] = 0;
   com->result[2] = 0;

   com->repoint = 0;

   com->nmodes = 0;
   com->freq = 0;
   com->nbins = 0;
   com->ttmod = 0;

   com->x = 0;
   com->y = 0;

   com->z = 0;


}

void init_AOI_Commands_list(AOI_Commands_list * com)
{
   init_AOI_command_info(&com->presetflat);
   init_AOI_command_info(&com->presetvisao);
   init_AOI_command_info(&com->presetao);
   init_AOI_command_info(&com->acqguider);
   init_AOI_command_info(&com->checkref);
   init_AOI_command_info(&com->acquireref);
   init_AOI_command_info(&com->refine);
   init_AOI_command_info(&com->modify);
   init_AOI_command_info(&com->startao);
   init_AOI_command_info(&com->offsetxy);
   init_AOI_command_info(&com->offsetz);

   init_AOI_command_info(&com->stop);
   init_AOI_command_info(&com->pause);
   init_AOI_command_info(&com->resume);
   init_AOI_command_info(&com->userpanic);
   init_AOI_command_info(&com->setnewinstrument);
   init_AOI_command_info(&com->wfson);
   init_AOI_command_info(&com->wfsoff);
   init_AOI_command_info(&com->adsecon);
   init_AOI_command_info(&com->adsecoff);
   init_AOI_command_info(&com->adsecset);
   init_AOI_command_info(&com->adsecrest);
   
   com->updateargs = 1; //starts at 1 on init so it forces a GUI update.
   com->busy = 0;
}

void init_AOI_AO_params(AOI_AO_params *par)
{
   par->dx = 0;
   par->dy = 0;
   par->freq = 0;
   par->gain = 0;
   par->mag = 0;
   par->nbins = 0;
   par->nmodes = 0;
   par->r0 = 0;
   par->snmode = 0;
   par->strehl = 0;
   par->ttmodul = 0;
}

void init_AOI_wfs_info(AOI_wfs_info *wfs)
{
   wfs->enabled = 0;
   wfs->active = 0;
   wfs->health = 0;
   wfs->ccdbin = 0;
   wfs->ccdfreq = 0;
   wfs->tv_binning = 0;
   wfs->tv_exptime = 0;
   wfs->counts = 0;
   wfs->no_subaps = 0;
   wfs->led = 0;
   wfs->mod_ampl = 0;
   wfs->pyramid_pos[0] = 0;
   wfs->pyramid_pos[1] = 0;
   wfs->tv_angle=0.;
}

void init_AOI_AO_DD(AOI_AO_DD *ao)
{
   ao->ao_ready = 0;
   ao->correctedmodes = 0;
   ao->loopon = 0;
   ao->ofl_enabled = 0;
   ao->sl_ready = 0;
   ao->strehl = 0;
   init_AOI_AO_params(&ao->param);
}

void init_AOI_ADSEC_DD(AOI_ADSEC_DD *ads)
{
   for(int i =0;i<12;i++) ads->anem_speed[i] = 0.;
   ads->anem_upd = 0;
   ads->elev_upd = 0;
   ads->health = 0;
   ads->contamination = 0;
   ads->nwact = 0;
   ads->coil_status = 0;
   ads->pwr_status = 0;
   ads->tss_status = 0;
   ads->led = 0;
   ads->safeskip_perc = 0;
   for(int i=0;i<3;i++) ads->offload[i] = 0;
}

void init_AOI_DD_rotator(AOI_DD_rotator *rot)
{
   rot->roi = 0;
   rot->angle = 0.;
   rot->offset = 0.;
   rot->following = 0;
};

void init_AOI_DD_catalog(AOI_DD_catalog *cat)
{
   cat->ra = 0;
   cat->dec = 0;
   cat->epoch = 0;
   cat->rotOff = 0;
}   

void init_AOI_DD_environment(AOI_DD_environment *env)
{
   env->wxtemp = 0.;
   env->wxpres = 0.;
   env->wxhumid = 0.;
   env->wxwind = 0.;
   env->wxwdir = 0.;
   env->wxdewpoint = 0.;
   env->wxpwvest = 0.;

   env->ttruss = 0.;
   env->tcell = 0.;
   env->tseccell = 0.;
   env->tambient = 0.;

   env->dimmfwhm = 0.;
   env->dimmtime = 0.;
   env->mag1fwhm = 0.;
   env->mag1time = 0.;

}

void init_AOI_side_DD(AOI_side_DD *side)
{
   init_AOI_Commands_list(&side->cmds);
   init_AOI_AO_DD(&side->ao);
   init_AOI_ADSEC_DD(&side->adsec);
   
   side->labmode = 0;
   side->idlstat = 0;
   
   init_AOI_wfs_info(&side->wfs1);
   
   side->vmajor = 0;
   side->vminor = 0;
   side->tel_enabled = 0;

   side->updateargs = 0;
   side->connected =0;
   side->sstat_color=0;

}


void init_AOI_DD(AOI_DD * aoi)
{
   aoi->ut = 0;
   aoi->st = 0;

   aoi->ra = 0.;
   aoi->dec = 0.;
   aoi->epoch = 0.;
   aoi->ha = 0.;
   aoi->am = 0.;

   init_AOI_DD_rotator(&aoi->rotator);

   aoi->istracking = 0;
   aoi->isguiding = 0;
   aoi->isslewing = 0;
   aoi->guider_ismoving = 0;

   aoi->az.position = 0.;
   aoi->el.position = 0.;
   aoi->el.time = 0.;

   aoi->zd = 0;
   aoi->pa = 0;

   aoi->dome.az = 0.;
   aoi->dome.stat = -1; //unknown

   init_AOI_DD_catalog(&aoi->cat);

   for(int i=0;i<6;i++) aoi->hexapod.abs_pos[i] = 0.;
   for(int i=0;i<6;i++) aoi->hexapod.set_pos[i] = 0.;
   for(int i=0;i<6;i++) aoi->hexapod.enc_pos[i] = 0.;

   init_AOI_DD_environment(&aoi->environ);

   init_AOI_side_DD(&aoi->side);

}

