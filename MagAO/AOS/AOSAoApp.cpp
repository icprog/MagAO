//@File: AOSAoApp.cpp
//
// Code for the AOSAoApp class
//@

#include <fstream>
#include <cmath>

#include <tcs/core/SystemClock.hpp>
#include <GshmVar.hpp>
#include <tcs/core/SysLog.hpp>
#include <tcs/setvalue/SetValueInterface.hpp> 
#include <oss/client/SecondaryMirrorClient.hpp>
#include <psf/client/PSF.hpp>
#include <tcs/commandreturn/CommandReturn.hpp>
#include <tcs/utility/TCSDateTime.hpp>
#include <tcs/networkconfig/NetworkConfig.hpp>

#include <iif/tcs/IIFFS.hpp>

#define TRACKING  "TRACKING"

#include "aos/aosupervisor/arblib/base/ArbitratorInterface.h"
#include "aos/aosupervisor/arblib/base/Commands.h"

#include "AOSconst.hpp"
#include "AOSEvent.hpp"
#include "AOSAoApp.hpp"

#include "AOScmdstat.hpp"

#include "AOSTelemetry.hpp"

namespace lbto {

extern int      side;
extern string   sideName;
extern int      AOSvmajor, AOSvminor;
extern int      logSeqN;
extern int    _MsgDretStat;

extern AOSTelemetry telemetry;

namespace arb = Arcetri::Arbitrator;

//@Function: tel_tracking
//
// Returns 1 if the telescope is currently tracking
//@

static int tel_tracking() //@R: 1 if the telescope is traking on given object, either 0
{
   string az=mcs.azDrive.trackingMode;
   string el=mcs.elDrive.trackingMode;
   int onSrc=mcs.onSource;
   return (onSrc && (az==TRACKING) && (el==TRACKING))?1:0;
}

//@Function: tel_guiding
//
// Returns 1 if the telescope is currently guiding
//
// Note: here follows the rationale.

//@

static int tel_guiding() //@R: 1 if the telescope is guiding on given object, either 0
{
   bool isGuiding = gcs.side[side].guidingFlag && 
                    (!gcs.side[side].guidingPaused) &&
                    (!gcs.side[side].guideStarLost) &&
                    gcs.side[side].GuidestarCloseToHotspot &&
                    gcs.side[side].xmitCorrectionsToPCS  &&
                    pcs.side[side].isGuidingEnabled;

   return isGuiding?1:0;
}

//@Function: vent_on
//
// Returns 1 if the primary mirror ventilation is on
//@

static int vent_on() //@R: 1 if the primary mirror ventilation is on, either 0
{
   bool runState;
   if(side==NetworkConfig::left) {
      runState=(ecs.mirrorVent.PB0401_Status.runState) &&
               (!ecs.alarm.PB0401_FailToRun);
   } else {
      runState=(ecs.mirrorVent.PB0402_Status.runState) &&
               (!ecs.alarm.PB0402_FailToRun);
   }
   bool ecsOn=NetworkConfig::isRunning("ECS");
   return (runState && ecsOn)?1:0;
}

//@Function: hbs_on
//
// Returns 1 if the hydrostatic bearing system is on
//@

static int hbs_on() //@R: 1 if the hydrostatic bearing system is on, either 0
{
   bool hbson=mcs.hydrostaticBearing.on &&
              (mcs.hydrostaticBearing.hydrostaticPressure >= 115.0);
   return hbson?1:0;
}


//@Function: swa_atStop
//
// Returns 1 if the M2 Swing Arm is currently atStop
//@

static int swa_atStop() //@R: 1 if the swingarm is fully deployed or retracted
                        //    and motor is off
{
   return ( (bool)oss.side[side].swa.sa_switch_deployed[0]  ||
            (bool)oss.side[side].swa.sa_switch_retracted[0]    ) && 
           !(bool)oss.side[side].swa.drive_state_power_on ;
}

//@Function: hexapod_status
//
// Returns hexapod status
//@

static int hexapod_status() // @R: A value indicating the current status of the hexapod
                            //     TODO: returned values are currently without any meaning
{
     if((oss.side[side].adsc.state==1)&&
        (oss.side[side].adsc.status==3)  ) return 1;
     return 0;
}

//@Class: AOSAoApp
//
// This class manages communication between the AOS and AO-Sup MsgD-RTDB
//
// Data mirroring from TCD Data Dictionary to the RTDB is performed by polling
// a list of DD variable and writing the corresponding values to RTDB
//
// Data mirroring from the RTDB and the TCS Data Dictionary is based on
// the notification mechanism of the RTDB.
//
// This class is derived from AOApp (AO Supervisor)
// and is actually the bridge between the TCS world and the AO Supervisor world
//@

AOSAoApp::AOSAoApp(const string msgdIP, const string rtdbSide, int dbglev, string logdir)
        : AOApp("AOS",rtdbSide,msgdIP, dbglev, false,"NONE")
{
//  thDebug(1);                  // Enable message queue debugging
    _reasonToDie=0;
    _msgd_isup=false;
    arbIntf=NULL;
    DDPollingPeriod = DDPOLLINGPERIOD*1000;  // Microseconds
    AoSupTimeout = AOSUPTIMEOUT;             // Milliseconds
    _rtdbSide=rtdbSide;
    _noVariables=true;
    _aoArb="AOARB."+_rtdbSide;
    _servstatName=_aoArb+".ServStat";
    _tvImgVarName="wfsarb."+_rtdbSide+".TV_IMAGE";
    _nUpdate=0;
    g_smClient = NULL;
    _pendOffload=0;
    if(rtdbSide[0]== 'L')
      tSide = Side::Left;
    else
      tSide = Side::Right;

    _serializator=new arb::Serializator();

    SetVersion(AOSvmajor,AOSvminor);

    TCSVarTable outVars;      // Variables exported from TCS to AOSupervisor
    RTVarTable  inVars;
    _reasonToDie=AOS_NOTSTARTED;

    _flaoWfsMsgd=string("M_")+AO_WFS_FLAO;
    _lbtiWfsMsgd=string("M_")+AO_WFS_LBTI;
}

AOSAoApp::~AOSAoApp()
{
    if(arbIntf) delete arbIntf;
    arbIntf=NULL;
}

string AOSAoApp::tstamp()
{
    extern time_t  _startTime;
    struct timeval tv;
    ostringstream st;

    gettimeofday(&tv,NULL);

    double tt=(double)(tv.tv_sec-_startTime)+(double)tv.tv_usec*0.000001;

    st.precision(3);
    st<< fixed << tt;
    return st.str();
}


void AOSAoApp::PostInit()
{
    arbIntf=new arb::ArbitratorInterface(_aoArb);
}

int AOSAoApp::setDebug(int level)  // Change logger level. Values are: 
                                   //              MASTER_LOG_LEV_TRACE
                                   //              MASTER_LOG_LEV_DEBUG
                                   //              MASTER_LOG_LEV_INFO
                                   //              MASTER_LOG_LEV_WARNING
                                   //              MASTER_LOG_LEV_ERROR
                                   //              MASTER_LOG_LEV_FATAL

{
// _logger->setLevel(level);
// _logger->log(Logger::LOG_LEV_INFO,"Logging level set to %d",level); 
   return AOS_ACK;
}

//@@Method: AOSAoApp::addTCSVar
//
// Creates a variable into RTDB
//
// Note: this is a Wrapper around TCSVarTable.append() with logging added
//
// Entry 1: Define plain variable
// @

void AOSAoApp::addTCSVar(const string name, int num, int ddref,int freq)
{
    outVars.append(name,num,ddref,freq);
    AOSinfolog("Mirroring RTDB var "+name+" established");
}

void AOSAoApp::addTCSVar(const string name, int num, int ddref,int freq, int expir, int tstamp)
{
    outVars.append(name,num,ddref,freq,expir,tstamp);
    AOSinfolog("Mirroring RTDB var "+name+" established");
}

// Entry 2: Define integer function value
//
void AOSAoApp::addTCSVar(const string name, int(* ddref)(),int freq)
{
    outVars.append(name,ddref,freq);
    AOSinfolog("Mirroring int funct to RTDB var "+name+" established");
}

// Entry 3: Define double function value
//
void AOSAoApp::addTCSVar(const string name, double(* ddref)(),int freq)
{
    outVars.append(name,ddref,freq);
    AOSinfolog("Mirroring double funct to RTDB var "+name+" established");
}


//@@Method: AOSAoApp::defineTCSVars
//
// Creates variables into the RTDB
// @

void AOSAoApp::defineTCSVars(void)     // Set up TCS variables to be mirrored to RTBD
{
    string s_pref=MyFullName();

    AOSdbglog("Defining variables into RTDB");

// Populate DD mirror variable table
    addTCSVar("AOS.AMB.WINDSPEED",     1, &env.anemometer.r_average_10s,0,1,&env.anemometer.timestamp);

    addTCSVar("AOS.TEL.AZ",            1, &mcs.azDrive.position,0,1,&mcs.azDrive.time);
    addTCSVar("AOS.TEL.EL",            1, &mcs.elDrive.position,0,1,&mcs.elDrive.time);
    addTCSVar("AOS.TEL.DEC",           1, &pcs.pointingStatus.achieved.achieved_DEC.Radians,0);
    addTCSVar("AOS.TEL.RA",            1, &pcs.pointingStatus.achieved.achieved_RA.Radians,0);

    addTCSVar("AOS.TEL.ISTRACKING",          &tel_tracking,0);
    addTCSVar("AOS.TEL.ISGUIDING",           &tel_guiding,0);
    addTCSVar("AOS.TEL.HBS_ON",              &hbs_on,0);
    addTCSVar("AOS.EXTERN.WINDSPEED",     1, &env.weather.lbt.windSpeed,1);
    addTCSVar("AOS.EXTERN.WINDDIRECTION", 1, &env.weather.lbt.windDirection,1);
    addTCSVar("AOS.DIMM.SEEING",          1, &iif.DIMM.seeing,5,60,&iif.DIMM.time);

    addTCSVar(s_pref+".TEL.VENT_ON",         &vent_on,0);
    addTCSVar(s_pref+".HEXAPOD.ABS_POS",  6, &(oss.side[side].adsc.abs_pos[0]),0);
    addTCSVar(s_pref+".HEXAPOD.STATUS",      &hexapod_status,0);

    addTCSVar(s_pref+".TERTIARY.ABS_POS", 4, &(oss.side[side].terc.abs_pos[0]),0);

    addTCSVar(s_pref+".SWA.DEPLOYED",        &swa_atStop,0);

    addTCSVar(s_pref+".ROTATOR.ANGLE",    1, &mcs.rotatorSide[side].rotators[IIFFS::FS_BENTGREGORIANFRONT].actualPositionASec,0);


    addTCSVar(s_pref+".GUIDECAM.CENTROID.X", 1, &gcs.side[side].GuideCam.centroid_FWHM_X,1);
    addTCSVar(s_pref+".GUIDECAM.CENTROID.Y", 1, &gcs.side[side].GuideCam.centroid_FWHM_Y,1);
}

string AOSAoApp::_cleanSt(string s)
{
   string ret;
   char *cstr= new char[s.size()+1];
   strcpy(cstr,s.c_str());

   for(size_t i=0; i<s.size(); i++)
      if(!isprint(cstr[i]))cstr[i]=' ';
   
   ret = string(cstr);
   delete[] cstr;
   return ret;
}

void AOSAoApp::_sendEV(string msg) 
{
   string intmsg=_cleanSt(msg);
   int intlev=1;
   size_t colon=msg.find(":");
   if(colon!=string::npos) {
      intmsg=msg.substr(colon+1);
      string ccode=msg.substr(0,colon);
      intlev=atoi(ccode.c_str());
   } 
   switch(intlev) {
     default:
     case 1:
        AOSerrlog(intmsg);
        break;
     case 2:
        AOSwnglog(intmsg);
        break;
     case 3:
        AOSinfolog(intmsg);
        break;
     case 4:
        AOSinfo2log(intmsg);
        break;
     case 5:
        AOSinfo3log(intmsg);
        break;
   }
}

void AOSAoApp::sendEV_AOArb(void *data, int datalen) // Called when messages to log are received
{
   string evString((char *)data,datalen);

   _sendEV(evString);
}
   

void AOSAoApp::sendEV_AdsecArb(void *data, int datalen) // Called when messages to log are received
{
   string evString((char *)data,datalen);

   _sendEV(evString);
}
   

void AOSAoApp::sendEV_Wfs1Arb(void *data, int datalen) // Called when messages to log are received
{
   string evString((char *)data,datalen);

   _sendEV(evString);
}

void AOSAoApp::sendEV_Wfs2Arb(void *data, int datalen) // Called when messages to log are received
{
   string evString((char *)data,datalen);

   _sendEV(evString);
}
   
   
void AOSAoApp::w2_counts(void *data, int datalen)
{
   int value=*(int *)data; 
   telemetry.storeW1cnt(value);
}


void AOSAoApp::w1_counts(void *data, int datalen)
{
   int value=*(int *)data; 
   telemetry.storeW2cnt(value);
}


int AOSAoApp::updateTCSVariables(void)  // Here we update variables into the RTDB from the
                                         // variable table initialized by the constructor.
{
   int n_var=outVars.size();
   int u;
   int stat=0;

   if(_noVariables ) {
      defineTCSVars();
      _noVariables=false;
   }
   for (u=0; u < n_var; u++) {
      TCSVar *v=outVars[u];

      try {
          stat= v->write();
      } catch(AOException& e) {
          stat=(-1);
          AOSwnglog(string("Error for Variable:"+ v->name() +" AOException: " + e.what(Terse) ));
          break;
       } catch(const std::exception& e) {
          stat=(-1);
          AOSwnglog(string("Error for variable:" + v->name() +" std::exception: " + string(e.what()) ));
          break;
       } catch(...) {
          stat=(-1);
          AOSerrlog(string("Error for variable:" + v->name() +" (unexpected exception)" ));
          break;
       }

      if (stat>0) {
          string msg="Variable " + v->name();
          switch(stat) {
          case JUST_EXPIRED:
              msg += " expired";
              break;
          case JUST_VALID:
              msg += " is valid";
              break;
          }
         AOSwnglog(msg);
         stat=0;
      }
   }
   return stat;
}


//@@Method: checkWFS
//
// Check status of WFS processes 
//
// @

void AOSAoApp::checkWFS( )
{
   if(thHaveYou(_flaoWfsMsgd.c_str())) {
      if(aos.side[side].wfs1.enabled!=1) {
         AOSinfolog("FLAO WFS daemon is now on");
         inVars.refreshNotify(_flaoWfsMsgd);
      }
      aos.side[side].wfs1.enabled=1;
   } else {
      if(aos.side[side].wfs1.enabled!=0)
         AOSinfolog("FLAO WFS daemon went off");
      aos.side[side].wfs1.enabled=0;
      aos.side[side].wfs1.health=0;
   }

   if(thHaveYou(_lbtiWfsMsgd.c_str())) {
      if(aos.side[side].wfs2.enabled!=1) {
         AOSinfolog("LBTI WFS daemon is now on");
         inVars.refreshNotify(_lbtiWfsMsgd);
      }
      aos.side[side].wfs2.enabled=1;
   } else {
      if(aos.side[side].wfs2.enabled!=0)
         AOSinfolog("LBTI WFS daemon went off");
      aos.side[side].wfs2.enabled=0;
      aos.side[side].wfs2.health=0;
   }
}

//@@Method: AOSAoApp::addRTVar
//
// Preset for mirroring of a variable from RTDB
//
// Note: this is a Wrapper around RTVarTable.append() with logging added
// @

void AOSAoApp::addRTVar(const string name, int rtype, int num, int ddref, void(*pproc)(void *, int), string vclass, bool logit)
{
    string fullname;

    if(vclass.empty())
       fullname=name;
    else
       fullname=name+"@"+vclass;

    AOSdbglog("Defined mirrored var from RTDB: "+fullname);
    if(ddref<0 && pproc==NULL)
        AOSerrlog("Neither DD counterpart nor processing routine defined for var: "+fullname);
    int ret=inVars.insert(fullname,rtype,num,ddref,pproc,vclass,logit);
    if(IS_ERROR(ret)) {
        string msg="addRTVar error on variable: "+fullname;
        AOSerrlog(msg);
    }
}


void AOSAoApp::defineRTVars(void)     // Set up RTDB variables to be mirrored from RTBD
{
    string AOARB_S = "AOARB."+_rtdbSide+".";
    string AOARB_S_ADSEC = "AOARB."+_rtdbSide+".ADSEC.";
    string ADSEC_S = "ADSEC."+_rtdbSide+".";
    string ANEM_S = "ANEM."+_rtdbSide+".";
    string adsecarb_S = "adsecarb."+_rtdbSide+".";
    string adamhousekeeper_S ="adamhousekeeper."+_rtdbSide+".";
    string optloopdiag_S = "optloopdiag."+_rtdbSide+".";
    string wfsarb_S = "wfsarb."+_rtdbSide+".";
    string ccd39_S ="ccd39."+_rtdbSide+".";
    string ccd47_S ="ccd47."+_rtdbSide+".";
    string filterwheel1_S ="filterwheel1."+_rtdbSide+".";
    string filterwheel2_S ="filterwheel2."+_rtdbSide+".";
    string slopecompctrl_S ="slopecompctrl."+_rtdbSide+".";

// The following variables are parameters related to the AO system
//            Name,                     type,       Num,    DD Variable,             preproc, vClass, LogIt

    addRTVar(AOARB_S+"AO_READY",          INT_VARIABLE,   1, &aos.side[side].ao.ao_ready,NULL,"",true);
    addRTVar(AOARB_S+"CORRECTEDMODES",    INT_VARIABLE,  1,  &aos.side[side].ao.correctedmodes);
    addRTVar(AOARB_S+"FSM_STATE",         CHAR_VARIABLE, 50, &aos.side[side].ao.status,NULL,"",true);
    addRTVar(AOARB_S+"LAB_MODE",          INT_VARIABLE,   1, &aos.side[side].labmode,NULL,"",true);
    addRTVar(AOARB_S+"LOOPON",            INT_VARIABLE,   1, &aos.side[side].ao.loopon,NULL,"",true);
    addRTVar(AOARB_S+"MODE",              CHAR_VARIABLE, 50, &aos.side[side].ao.mode);
    addRTVar(AOARB_S+"MSG",               CHAR_VARIABLE, 50, &aos.side[side].ao.msg,&sendEV_AOArb);
    addRTVar(AOARB_S+"OFL_ENABLED",       INT_VARIABLE,   1, &aos.side[side].ao.ofl_enabled);
    addRTVar(AOARB_S+"SL_READY",          INT_VARIABLE,   1, &aos.side[side].ao.sl_ready,NULL,"",true);
    addRTVar(AOARB_S+"STREHL",            REAL_VARIABLE, 1,  &aos.side[side].ao.strehl);
    addRTVar(AOARB_S+"WFS_SOURCE",        CHAR_VARIABLE, 50, &aos.side[side].ao.wfs_source);

    addRTVar(_rtdbSide+".IDL_STAT",       INT_VARIABLE,   1, &aos.side[side].idlstat,NULL,"",true);

// The following variables are parameters related to the AdSec
//            Name,                     type,       Num,    DD Variable,             preproc, vClass, LogIt
//  addRTVar(AOARB_S_ADSEC+"LOOPGAIN_LIMITS", REAL_VARIABLE, 2,  &(aos.side[side].adsec.loopgain_limits[0]));
//  addRTVar(AOARB_S_ADSEC+"LOOPGAIN",        REAL_VARIABLE, 1,  &aos.side[side].adsec.loopgain);

    
//               Corresponding DD variables: aos.side[side].adsec.anem_speed
//               NOTE: the (-1) must be substiotuted with the proper variable name when defined into DD
    addRTVar(ANEM_S+"SPEED",                  REAL_VARIABLE, 12, &(aos.side[side].adsec.anem_speed[0]), &anem_speed); // Managed by anem_speed

    addRTVar(AOARB_S_ADSEC+"ANEM_UPD",        INT_VARIABLE,  1,  &aos.side[side].adsec.anem_upd);
    addRTVar(AOARB_S_ADSEC+"ELEV_UPD",        INT_VARIABLE,  1,  &aos.side[side].adsec.elev_upd);
    addRTVar(AOARB_S_ADSEC+"HEALTH",          INT_VARIABLE,  1,  &aos.side[side].adsec.health,NULL,"",true);
    addRTVar(ADSEC_S+"CONTAMINATION",         INT_VARIABLE,  1,  &aos.side[side].adsec.contamination,NULL,"",true);
    addRTVar(ADSEC_S+"NWACT",                 INT_VARIABLE,  1,  &aos.side[side].adsec.nwact,NULL,"",true);
    addRTVar(ADSEC_S+"POPMSG",                CHAR_VARIABLE, 50, &aos.side[side].adsec.popmsg);
    addRTVar(ADSEC_S+"SHAPE",                 CHAR_VARIABLE, 50, &aos.side[side].adsec.shape,&VarUtils::trimfile);
    addRTVar(adamhousekeeper_S+"COIL_STATUS", INT_VARIABLE,  1,  &aos.side[side].adsec.coil_status,NULL,"",true);
    addRTVar(adamhousekeeper_S+"MAIN_POWER_STATUS",INT_VARIABLE,  1,  &aos.side[side].adsec.pwr_status,NULL,"",true);
    addRTVar(adamhousekeeper_S+"TSS_STATUS",  INT_VARIABLE,  1,  &aos.side[side].adsec.tss_status,NULL,"",true);
    addRTVar(adsecarb_S+"FSM_STATE",          CHAR_VARIABLE, 50, &aos.side[side].adsec.status,NULL,"",true);
    addRTVar(adsecarb_S+"LED",                INT_VARIABLE,  1,  &aos.side[side].adsec.led,NULL,"",true);
    addRTVar(adsecarb_S+"MSG",                CHAR_VARIABLE, 50, &aos.side[side].adsec.msg,&sendEV_AdsecArb);
    addRTVar(adsecarb_S+"SAFESKIP_PERCENT",   REAL_VARIABLE, 1,  &aos.side[side].adsec.safeskip_perc);

// The following variables are parameters related to WFS 1
//              Name,                     type,       Num,    DD Variable,             preproc, vClass, LogIt
//  addRTVar(wfsarb_S+"CCDFREQ_LIMITS",    REAL_VARIABLE, 2,  &(aos.side[side].wfs1.ccdfreq_limits[0]),NULL,_flaoWfsMsgd);

    addRTVar(AOARB_S+"FLAOWFS.HEALTH",     INT_VARIABLE,  1,  &aos.side[side].wfs1.health);
    addRTVar(ccd39_S+"XBIN.CUR",           INT_VARIABLE,  1,  &aos.side[side].wfs1.ccdbin,NULL,_flaoWfsMsgd);
    addRTVar(ccd39_S+"FRMRT.CUR",          REAL_VARIABLE, 1,  &aos.side[side].wfs1.ccdfreq,NULL,_flaoWfsMsgd);
    addRTVar(ccd47_S+"XBIN.CUR",           INT_VARIABLE,  1,  &aos.side[side].wfs1.tv_binning,NULL,_flaoWfsMsgd);
    addRTVar(ccd47_S+"FRMRT.CUR",          REAL_VARIABLE, 1,  &aos.side[side].wfs1.tv_exptime,NULL,_flaoWfsMsgd);
    addRTVar(filterwheel1_S+"POSNAME.CUR", CHAR_VARIABLE, 50, &aos.side[side].wfs1.filter1,NULL,_flaoWfsMsgd);
    addRTVar(filterwheel2_S+"POSNAME.CUR", CHAR_VARIABLE, 50, &aos.side[side].wfs1.tv_filter2,NULL,_flaoWfsMsgd);
    addRTVar(optloopdiag_S+"COUNTS",       INT_VARIABLE,  1,  &aos.side[side].wfs1.counts,&w1_counts,_flaoWfsMsgd);
    addRTVar(slopecompctrl_S+"NSUBAPS.CUR",INT_VARIABLE,  1,  &aos.side[side].wfs1.no_subaps,NULL,_flaoWfsMsgd);
    addRTVar(wfsarb_S+"FSM_STATE",         CHAR_VARIABLE, 50, &aos.side[side].wfs1.status,NULL,_flaoWfsMsgd,true);
    addRTVar(wfsarb_S+"LED",               INT_VARIABLE,  1,  &aos.side[side].wfs1.led,NULL,_flaoWfsMsgd,true);
    addRTVar(wfsarb_S+"MOD_AMPL",          REAL_VARIABLE, 1,  &aos.side[side].wfs1.mod_ampl,NULL,_flaoWfsMsgd);
    addRTVar(wfsarb_S+"MSG",               CHAR_VARIABLE, 50, &aos.side[side].wfs1.msg,&sendEV_Wfs1Arb,_flaoWfsMsgd);
    addRTVar(wfsarb_S+"PYRAMID_POS",       INT_VARIABLE,  2,  &(aos.side[side].wfs1.pyramid_pos[0]),NULL,_flaoWfsMsgd);

    addRTVar(_tvImgVarName,             BIT8_VARIABLE, 65536, (-1), &tvUpdateCurrent1,_flaoWfsMsgd); // Not written into DD

// The following variables are parameters related to WFS 2
//            Name,                     type,       Num,    DD Variable,             preproc, vClass, LogIt
    addRTVar(AOARB_S+"LBTIWFS.HEALTH",     INT_VARIABLE,  1,  &aos.side[side].wfs2.health);
    addRTVar(ccd39_S+"XBIN.CUR",           INT_VARIABLE,  1,  &aos.side[side].wfs2.ccdbin,NULL,_lbtiWfsMsgd);
    addRTVar(ccd39_S+"FRMRT.CUR",          REAL_VARIABLE, 1,  &aos.side[side].wfs2.ccdfreq,NULL,_lbtiWfsMsgd);
    addRTVar(optloopdiag_S+"COUNTS",       INT_VARIABLE,  1,  &aos.side[side].wfs2.counts,&w2_counts,_lbtiWfsMsgd);
    addRTVar(filterwheel1_S+"POSNAME.CUR", CHAR_VARIABLE, 50, &aos.side[side].wfs2.filter1,NULL,_lbtiWfsMsgd);
    addRTVar(wfsarb_S+"FSM_STATE",         CHAR_VARIABLE, 50, &aos.side[side].wfs2.status,NULL,_lbtiWfsMsgd,true);
    addRTVar(wfsarb_S+"LED",               INT_VARIABLE,  1,  &aos.side[side].wfs2.led,NULL,_lbtiWfsMsgd,true);
    addRTVar(wfsarb_S+"MOD_AMPL",          REAL_VARIABLE, 1,  &aos.side[side].wfs2.mod_ampl,NULL,_lbtiWfsMsgd);
    addRTVar(wfsarb_S+"MSG",               CHAR_VARIABLE, 50, &aos.side[side].wfs2.msg,&sendEV_Wfs2Arb,_lbtiWfsMsgd);
    addRTVar(slopecompctrl_S+"NSUBAPS.CUR",INT_VARIABLE,  1,  &aos.side[side].wfs2.no_subaps,NULL,_lbtiWfsMsgd);
    addRTVar(wfsarb_S+"PYRAMID_POS",       INT_VARIABLE,  2,  &(aos.side[side].wfs2.pyramid_pos[0]),NULL,_lbtiWfsMsgd);
    addRTVar(ccd47_S+"XBIN.CUR",           INT_VARIABLE,  1,  &aos.side[side].wfs2.tv_binning,NULL,_lbtiWfsMsgd);
    addRTVar(ccd47_S+"FRMRT.CUR",          REAL_VARIABLE, 1,  &aos.side[side].wfs2.tv_exptime,NULL,_lbtiWfsMsgd);
    addRTVar(filterwheel2_S+"POSNAME.CUR", CHAR_VARIABLE, 50, &aos.side[side].wfs2.tv_filter2,NULL,_lbtiWfsMsgd);

    addRTVar(_tvImgVarName,            BIT8_VARIABLE, 65536, (-1), &tvUpdateCurrent2,_lbtiWfsMsgd); // Not written into DD

}

void AOSAoApp::tvRotate(TVImage *in, TVImage *out, int rot) // Rotate TV images
{
   int tot,src=0,dst=0;
   out->rows=in->rows;
   out->cols=out->cols;
   switch(rot) {
     int i,j;
     case 90:
     case -270:
       for(j=in->rows-1;j>=0;j--) for(i=0;i<in->cols;i++) { src=(i*in->cols+j); out->pix[dst++]=in->pix[src]; }; break;
     case 180:
       src=out->cols*out->rows-1;
       while (src>=0) { out->pix[dst++]=in->pix[src--]; }; break;
     case -90:
     case 270:
       tot=out->cols*out->rows;
       for(j=0;j<in->cols;j++) for(i=in->rows-1;i>=0;i--) { src=(i*in->cols+j); out->pix[dst++]=in->pix[src]; }; break;
     default:
       while (src<tot) { out->pix[dst++]=in->pix[src++]; }; break;
   }
}

void AOSAoApp::_tvUpdateCurrent(void *data, int datalen, int wfs_id) // Create new TV file with current image
{
   char tvbuf[MAX_TV_PIXELS+2*sizeof(int)];
   TVImage *tvi=(TVImage *)data;
   string dst_filename;
   extern string tmp_filename;
   mode_t filemode=S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;

   switch(wfs_id) {
      case 1:
        dst_filename=aos.side[side].wfs1.tv_filename0;
        break;
      case 2:
        dst_filename=aos.side[side].wfs2.tv_filename0;
        break;
      default:
        AOSerrlog("Illegal WFS selector in tvUpdateCurrent");
        return;
   }

   if(datalen<=0) {
       unlink(dst_filename.c_str());
       return;
   }
   int imglen=tvi->rows*tvi->cols;
   if(imglen<=0) {
       unlink(dst_filename.c_str());
       return;
   }
   if(imglen>MAX_TV_PIXELS) imglen=MAX_TV_PIXELS;
   TVImage *dst=(TVImage *)tvbuf;
   tvRotate(tvi, dst, 90);           // Rotate TV image
   ofstream fl(tmp_filename.c_str(), (ios::out) | (ios::trunc) | (ios::binary) );
   fl.write(tvbuf,datalen);
   fl.close();
   chmod(tmp_filename.c_str(),filemode);
   rename(tmp_filename.c_str(),dst_filename.c_str());
   AOStracelog("Written current TV image file "+dst_filename);
}

void AOSAoApp::anem_speed(void *data, int datalen) // store Anemometer data into telemetry
{
   double *values=(double *)data;
   telemetry.storeAnem(values);
}

void AOSAoApp::tvUpdateCurrent1(void *data, int datalen) // Create new TV file with current image
{
   _tvUpdateCurrent(data, datalen, 1);
}

void AOSAoApp::tvUpdateCurrent2(void *data, int datalen) // Create new TV file with current image
{
   _tvUpdateCurrent(data, datalen, 2);
}

void AOSAoApp::tvUpdateAcquisition(char *data, int datalen) // Create new TV file with acquisition image
{
   extern string tmp_filename;
   mode_t filemode=S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;

   string dst_filename;
   string wfs_source=aos.side[side].ao.wfs_source;

   int wfs_id=0;

   if(wfs_source == AO_WFS_FLAO) wfs_id=1;
   if(wfs_source == AO_WFS_LBTI) wfs_id=2;

   if(wfs_id==0) {
     AOSerrlog("Illegal WFS source specification: "+wfs_source);
     return;
   }

   switch(wfs_id) {
      case 1:
        dst_filename=aos.side[side].wfs1.tv_filename1;
        break;
      case 2:
        dst_filename=aos.side[side].wfs2.tv_filename1;
        break;
      default:
        AOSerrlog("Illegal WFS selector in tvUpdateAcquisition");
        return;
   }

   TVImage *tvi=(TVImage *)data;
   if(dst_filename.empty()) return;

   if(datalen<=0) {
       unlink(dst_filename.c_str());
       return;
   }
   int imglen=tvi->rows*tvi->cols;
   if(imglen<=0) {
       unlink(dst_filename.c_str());
       return;
   }
   if(imglen>MAX_TV_PIXELS) imglen=MAX_TV_PIXELS;
   ofstream fl(tmp_filename.c_str(), (ios::out) | (ios::trunc) | (ios::binary) );
   fl.write(data,datalen);
   fl.close();
   chmod(tmp_filename.c_str(),filemode);
   rename(tmp_filename.c_str(),dst_filename.c_str());
   AOStracelog("Written acquisition TV image file "+dst_filename);
}

int AOSAoApp::reqService(void)       // Request telescope service
{
  int ret;

  if(!synchronize()) ret=AOS_NACK;   // This is just to wait for status update
  if(status() == NormalOperating) {
     AOSinfolog("reqService: ALLOWED");
                                   // TBD: here we must put commandts to TCS
                                   // to allow us to control the telescope
     
     ret=NO_ERROR;
  } else {
     AOSwnglog("reqService: NOT ALLOWED. AO Arb must first set OPERATING status");
     ret=AOS_REQUEST_DENIED;
  }
  return ret;
}


bool AOSAoApp::synchronize(void) // Check status of AO Supervisor by polling 
                                 // the connection status variable from RTDB.
                                 // Uses the ConnStatus class to set up the 
                                 // status on AOS side
{ 
   MsgBuf *varmsg;
   Variable *theVar;
   int stat,errcod;
   extern ConnStatus _connstat;

   _syncMutex.lock();

   varmsg=thGetVar((char *)_servstatName.c_str(),AoSupTimeout,&errcod);

   if(!varmsg) {
      switch(errcod) {
         case VAR_NOT_FOUND_ERROR:
            _connstat.set(NoArbitrator);
            _msgd_isup=true;
            break;
         case TIMEOUT_ERROR:
            _connstat.set(MsgDTimeout);
            _msgd_isup=false;
            break;
         case THRD_NOT_CONNECTED_ERROR:
            _connstat.set(MsgDNotConnected);
            _msgd_isup=false;
            break;
         default:
            _connstat.set(MsgDUnexpected);
            _msgd_isup=false;
            break;
      }
      
      aos.side[side].connected=_msgd_isup;
      aos.side[side].running=0;
      _syncMutex.unlock();
      return _msgd_isup;
   }

   _msgd_isup=true;
   aos.side[side].connected=_msgd_isup;

   stat=thSendMsg(0, _aoArb.c_str(), ECHO, NOHANDLE_FLAG, NULL);    // Send echo message to AO Arb.
   MsgBuf *echomsg=thWaitMsg(ECHOREPLY,_aoArb.c_str(),stat,500,&stat);    // Wait for reply 500 ms

   if(!echomsg) {
      thRelease(varmsg);
      _connstat.set(NoArbitrator);
      aos.side[side].running=0;
      _syncMutex.unlock();
      return _msgd_isup;
   }
   thRelease(echomsg);

   theVar=thValue(varmsg);
   stat=theVar->Value.Lv[0];
   thRelease(varmsg);

   if(stat==1) 
      _connstat.set(NormalOperating);
   else
      _connstat.set(NormalStandalone);
   
   aos.side[side].running=1;
   _syncMutex.unlock();
   return _msgd_isup;
}

bool AOSAoApp::msgd_up(void) { return _msgd_isup; };

int  AOSAoApp::status(void) {
   extern ConnStatus _connstat;
   return _connstat.get(); 
}

string  AOSAoApp::chstatus(void) {
   extern ConnStatus _connstat;
   return _connstat.getStr(); 
}


int AOSAoApp::wakeup(void)   // Notification that status has changed
{
   int ret=NO_ERROR;
   //
   // TBD: put here all needed  controls

   AOSdbglog("Wakeup received");
   if(!_msgd_isup) {
      AOSwnglog("Wakeup received when MsgD not connected: something is wrong");
      ret=AOS_NOT_RUNNING_ERROR;
   }
   return ret;
}

void AOSAoApp::Quit(int reason)
{
SetTimeToDie(true);
_reasonToDie=reason;
}

int AOSAoApp::whyQuit(void)
{
return _reasonToDie;
}


int AOSAoApp::makeFlatList(string list) { // Creating file for Flat menu
   string fl_file=aos.side[side].adsec.fl_filename;
   ofstream fl(fl_file.c_str(), (ios::out) | (ios::trunc));
   int stat=NO_ERROR;
   mode_t filemode=S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;

   AOSinfolog(string("Creating menu file: "+fl_file));
   if(fl.is_open()) {
       size_t pos=0;
       bool goon=true;

       while(goon) {
          string name;
          size_t semicol=list.find(";",pos);
          if(semicol==string::npos) {
             name=list.substr(pos);
             goon=false;
          } else {
             name=list.substr(pos,semicol-pos);
             pos=semicol+1;
          }
          if(!name.empty()) {
             size_t dotsav=name.find(".sav");
             if(dotsav!=string::npos) name=name.substr(0,dotsav);
             fl<< name << endl;
          }
       }
       fl.close();
       chmod(fl_file.c_str(),filemode);
   } else {
       AOSwnglog(string("Cannot open for write file: "+fl_file));
       stat=AOS_GEN_ERROR;
   }
   return stat;
}

void AOSAoApp::Run(void)     // Here is where some of the job is done
{                            // Actually just DD variable mirroring is
                             // done here. The rest is either performed
                             // by handlers or by calling SendAoCommand()
   extern ConnStatus _connstat;
   bool noRTvars=true;
   int errc;
   AOSinfolog("Connection to MsgD @"+Server()+" established");

   MsgBuf *msg=thGetVar("MsgD.Ident",200,&errc);
   if(msg) {
      Variable *var=thValue(msg);
      int l=strlen(var->Value.Sv);
      aos.side[side].msgdident=std::string(var->Value.Sv,l);
      thRelease(msg);
   } else {
      aos.side[side].msgdident=(char *)"No info available";
   }
   aos.side[side].conntime=lbto::tcs::DateTime::now().getFormattedStr();

   _msgd_isup=true;           // Note: if we get here, the connection with MsgD
                              // has been established
   _connstat.set(NoArbitrator);

   int msgdVers=thVersion("");
   if(msgdVers>=MSGD_REQUIRE)
      AOSinfolog("MsgD Version OK");
   else
      AOSwnglog("MsgD version too old. You may expect unstable behaviour");

   aos.side[side].wfs1.enabled=0;    // Clear WFS status variables
   aos.side[side].wfs2.enabled=0;
   aos.side[side].wfs1.active=0;
   aos.side[side].wfs2.active=0;

   _ofld_ignored=0;                 // Clear counter of ignored offload
                                    // Set up Alert receiver
   arb::RequestFeedback rFeedback(500);
   Command* cmdRply = NULL;
   try {
       cmdRply = arbIntf->requestCommand(&rFeedback);
   } catch(ArbitratorInterfaceException& e) {
       AOSdbglog("AoArbitrator alert registration failed - "+e.what(Hmi));
   } catch(...) {
       AOSdbglog("AoArbitrator alert registration unknow exception");
   }

   defineRTVars();  // Define variables to be notified by RTDB

//                                         Begin main loop
   for(_reasonToDie=0;_reasonToDie==0;) {    
      int vstat;

      if(synchronize()) {
         if(noRTvars) {
            inVars.refreshNotify("");     // Update the notify request
            noRTvars=false;               // For AOARB variables
         }
         vstat=updateTCSVariables();      // Update TCS variables into RTDB
         checkWFS();                      // Check WFS 1 & 2 status
         if(vstat<0) {
            _msgd_isup=false;
            _connstat.set(MsgDNotConnected);
            _reasonToDie=vstat;
         }
      } else {
         _connstat.set(MsgDNotConnected);
         _reasonToDie=AOS_UNEXPEXCP;
         AOSwnglog(string("NO connection. Reason: " + _connstat.getLogStr()));
         break;
      }
      SystemClock::usleep(DDPollingPeriod);
   }
}

// Called to perform mode offload
//
void AOSAoApp::offload(float *zernike, bool ignore)
{
   extern double oflm_gain[OFL_MODES];
   extern double oflm_thrs[OFL_MODES];
   int i,oflm_status;

   AOSdbglog("Offload received @:"+tstamp());

   bool tiptiltfocus=false;
   bool higherord=false;

//                               Verify thresholds and apply gains
   for(i=1;i<4;i++) {
      if(fabs(zernike[i])>oflm_thrs[i]) tiptiltfocus=true;
      zernike[i] *= oflm_gain[i];
   }
   for(i=4;i<OFL_MODES;i++) {
      if(fabs(zernike[i])>oflm_thrs[i]) higherord=true;
      zernike[i] *= oflm_gain[i];
   }

   if((!tiptiltfocus) && (!higherord) ) {
      AOSdbglog("Offload below threshold @:"+tstamp());
      return;
   }

   if(ignore) {            // Count ignored requests
      AOSdbglog("Offload ignored @:"+tstamp());
      _ofld_ignored++; 
      return; 
   } 

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
}

int AOSAoApp::tbd(string src,MsgBuf *imsg)
{
    AOSwnglog(fmtMsg(src,"- service not implemented. ",imsg));
    return TBD_ERROR;
}

string AOSAoApp::fmtMsg(string src, string prefix, MsgBuf *imsg)
{
   std::ostringstream o;
   int code=HDR_CODE(imsg);
   char* sender=HDR_FROM(imsg);
   int seqn=HDR_SEQN(imsg);
   int subC=HDR_PLOAD(imsg);
   int len=HDR_LEN(imsg);

   o << src << prefix << " Sender:"<< sender << " Code:" << hex << code << " SubC:" << dec << subC << " SeqN:" << seqn << " Len:" << len;
   return o.str();
}


}  // namespace lbto
