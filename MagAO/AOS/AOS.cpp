/* AOS.cpp
 *
 * AOS subsystem class for the AOS subsystem.
 *
 * The AOS subsystem is a standard TCS subsystem which manages the 
 * activation of communication with the MsgD-RTDB and executes commands
 * received from other TCS subsystems.
 *
 * For details on command execution see the related methods (in section:
 * Commands)
 *
 * For details on communication management, see the method: execute
 */


#include <cmath>

#include <tcs/networkconfig/NetworkConfig.hpp>
#include <iif/tcs/Position.hpp>
#include <tcs/core/SysLog.hpp>
#include <tcs/commandsequencer/CSQHandle.hpp>
#include <iif/tcs/IIFGlobals.hpp>
#include <pcs/client/PCS.hpp>
#include <pcs/PCSDefines.hpp>

#include "AOSconst.hpp"

#include "AOSEvent.hpp"
#include "AOS.hpp"

#include "AOSTimeouts.hpp"

#include "AOScmdstat.hpp"

#include "ConnStatus.hpp"

extern "C" {
#include "aos/aosupervisor/aoslib/aoscodes.h"
#include "aos/aosupervisor/base/globals.h"
}

#include "aos/aosupervisor/arblib/base/ArbitratorInterface.h"
#include "aos/aosupervisor/arblib/aoArb/AOCommandsExport.h"

#include "aos/aosupervisor/NaNutils.h"

#include "AOSClear.hpp"

namespace lbto {

extern int     side;
extern int     AOSvmajor, AOSvminor;
extern int     logSeqN;

extern ConnStatus _connstat;

string nocommerr("No communication with MsgD");
string complete("Complete");
string started("Started");
string cmdrecv(" command received.");

AOSSubsystem::AOSSubsystem(string aoMsgdIP, string msgdName, int dbgLevel, const string logdir, bool simulation=true) : Signal(SIGHUP,SIGINT)
{
   _simulation=simulation;
   _logdir=logdir;
   _cmdbusy=false;
   _cmdnum=0;
   Zero(side);         // Initialize all variables
   AOSSleepTime = AOS_SLEEP_PERIOD_MS*1000;
   string tv_filename_full("");
   _dbgLevel=dbgLevel;
//  _logger=Logger::get();
//  _logger->setLevel(dbgLevel);
   msgdIP=aoMsgdIP;
   msgdN=msgdName;
   _setInstr((string)iif.side[side].authorizedInstrument,
             (string)iif.side[side].authorizedFocalStation);

//                                      Set up/clear AOS variables
   _connstat.set(MsgDNotConnected);
   aos.side[side].cmds.busy = 0;

    /* Log startup of subsystem */
   AOSinfolog("AOS Subsystem created");

   AOSdbglog("TV auxiliary files: "+string(aos.side[side].wfs1.tv_filename0)+", "+string(aos.side[side].wfs1.tv_filename1));

   if(_simulation) {
      aos.side[side].msgdip = (char *)"x.x.x.x";
   } else {
      aos.side[side].msgdip = aoMsgdIP;
   }

   if ( ConfigureOffload() && ConfigureZernikes() ) {
      timeToQuit = false;   // Configure OK. goon
   } else {
      timeToQuit = true;   // Configure error. terminate
      _quitreason="Error reading config file";
   }

// Start components
   aoapp=NULL;
};

AOSSubsystem::~AOSSubsystem()
{
    /* Log shutdown of subsystem 
    AOSEvent shutdownEvent("aos.shutdown");
    shutdownEvent.setParameter("vmajor",AOSvmajor);
    shutdownEvent.setParameter("vminor",AOSvminor);
    shutdownEvent.logEvent();

    Already logged */

};


// signalHandler - catches signals.
// set up by Signal class
//
void AOSSubsystem::signalHandler(int signum)
{
    if(aoapp) aoapp->Quit(AOS_SIGTERM);
    timeToQuit = true;
};

// Setup WFS related informationb based on WFS spec
bool AOSSubsystem::_setWFS(string wfsSpec)
{
    bool stat;
    if(wfsSpec==AO_WFS_FLAO) {
        _wfsSpec=AO_WFS_FLAO;           // Preset WFS spec to FLAOWFS
        aos.side[side].wfs1.active=1;
        aos.side[side].wfs2.active=0;
        stat=true;
    } else if(wfsSpec==AO_WFS_LBTI) {  
        _wfsSpec=AO_WFS_LBTI;          // Preset WfsSpec to LBTIWFS
        aos.side[side].wfs1.active=0;
        aos.side[side].wfs2.active=1;
        stat=true;
    } else if(wfsSpec==AO_WFS_NONE) {  
        _wfsSpec="";                  // Preset WfsSpec to LBTIWFS
        aos.side[side].wfs1.active=0;
        aos.side[side].wfs2.active=0;
        stat=true;
    } else {
        stat=false;
    }
    return stat;
}


// Method: execute
//
// AOS subsystem main loop
//
// The main task of this method is the management of communication
// with MsgD-RTDB.
//
// The AOS communicates with the MsgD-RTDB to perform essentially four tasks:
//
// 1. Reflect the values of some DD variables into the RTDB.
//
// 2. Reflect the status of some RTDB variables into the DD
//
// 3. Execute commands coming from other TCS subsystems (when the system
//    is supporting an observation, i.e. it is in Observation mode)
//
// 4. Request the TCS to perform some actions in reply to commands
//    coming from the AO Supervisor (when the system is in
//    Engineering mode).
//
//  Tasks 1,2 and 4 are performed by internal loops of the class AOSAoApp
// (see AOSAoApp.cpp).
//
// Task 3 is performed by specific methods of this class (see below, section
// Commands). But again for the sending of messages as a result of execution
// of commands, a method of the AOSAoApp object is used.

void AOSSubsystem::execute(void *arg)
{

// aoapp is a pointer to the communication object. It is NULL at start
   aoapp=NULL;
   aos.side[side].running = true;

   string instr= _authorizedInstr;
   string focalSt=_authorizedFocStn;

   int l_reason;

   if(_simulation) {
      AOSinfolog("Simulation mode: AOSAoApp will not start");
      _connstat.set(Simulation);
   }

// This is the main loop controlling communication  with the MsgD.
//
// The main task of the loop is to check the status of the communication
// with the MsgD-RTDB and react properly

   while(!timeToQuit) {
//     _logger=Logger::get();    // Get Logger again
//     _logger->setLevel(_dbgLevel);

       if(!_simulation) {
           AOSdbglog("trying to create AOSAoApp");
           clearRTDBVars(side);
           try {
              aoapp=new AOSAoApp(msgdIP,msgdN,_dbgLevel,_logdir); 
              AOSdbglog("starting communication loop");
              aoapp->waitForMsgD();      // This sets wait mode
              aoapp->Exec();             // Try to connect to MsgD and start
              l_reason=aoapp->whyQuit();
           } catch(AOException e) {
              AOSerrlog("Error starting AOApp: "+e.what());
              l_reason=AOS_UNEXPEXCP;
           } 
//
//                                   We get here when AOSAoApp terminates or if AOAoApp creation fails
           switch(l_reason) {
              case AOS_MSGDTERM:  // AOS received TERMINATE msg from MsgD
                 _quitreason="TERMINATE from MsgD";
                 timeToQuit=true;
                 break;
              case AOS_SIGTERM:    // AOS received Sigterm
                 _quitreason="SIGTERM";
                 timeToQuit=true;
                 break;
              case AOS_NOTSTARTED: // AOS was not started (No communication with MsgD)
                 _quitreason="still waiting for MsgD to reply";
                 break;            // next loop cycle

              default:             // Other reasons are due to AOS internal errors (already logged)
                 _quitreason="previous error";
                 break;
           }
           if(aoapp) delete aoapp;
           aoapp=NULL;
           AOSdbglog("Communication loop ends. Reason: "+_quitreason);
       }
       usleep(AOSSleepTime);     // Delay AOSSleepTime microseconds
   }
   shutDown(_quitreason);
}

string AOSSubsystem::shutDown(string why)
{
    AOSdbglog("AOS subsystem shutting down due to: "+why);

    aos.side[side].running = false;
	
    CommandReturn cmdRet;
    cmdRet.setSuccess();

    AOSinfolog("AOS shutdown. Reason: "+why);

    if(aoapp) {
        delete aoapp;
        aoapp=NULL;
    }
    return cmdRet.serialize();
};

bool AOSSubsystem::cmdCheck(string cmdName, int msgSeverity, bool sendAlways)
{
   bool retstat=true;
   CmdStatusClear(side);
   AOSdbglog(cmdName + cmdrecv);
   _mutex.lock();
   aos.side[side].cmds.last=cmdName;
   if(_cmdbusy) {
      std::ostringstream o;
      o << cmdName << " rejected. Busy serving " << _cmdname << " ["<< _cmdnum << "]";
      _reason=o.str();
      _reason_code=CHK_BUSY;
      AOSwnglog(_reason);
      _cmd_stat=CMD_FAILURE;
      _mutex.unlock();
      return retstat;
   }

   if(_simulation) {
      _reason_code=CHK_SIMULATION;
      _cmd_stat=CMD_SUCCESS;
      _reason=string(cmdName+" - simulated execution");
      AOSinfo3log(_reason);
      _mutex.unlock();
      return true;
   }

   if(aoapp==NULL) {
      _reason_code=CHK_DISABLED;
      _cmd_stat=CMD_FAILURE;
      _reason=string(cmdName+" rejected. No communication established");
      AOSlog(msgSeverity, _reason);
      _mutex.unlock();
      return true;
   }
      
   bool canproceed= (aoapp->status()==NormalOperating)||(sendAlways && aoapp->msgd_up());

   if(canproceed) {
       _cmdbusy=true;
       aos.side[side].cmds.busy=1;
       aos.side[side].cmds.stat=started;
       aos.side[side].cmds.prio=3;
       _cmdname=cmdName;
       _cmdnum++;
       retstat=false;
      _cmd_stat=CMD_SUCCESS;
      _reason_code=CHK_OK;
    } else {
      _cmd_stat=CMD_FAILURE;
      _reason_code=CHK_DISABLED;
      _reason=string(cmdName+" rejected. Commands are disabled (status:"+aoapp->chstatus()+")");
      AOSlog(msgSeverity, _reason);
      retstat=true;
    }
   _mutex.unlock();
   return retstat;
}

string AOSSubsystem::cmdStatNotify()
{
CommandReturn cmdRet;
   aos.side[side].cmds.stat=_reason;
   if(_cmd_stat==CMD_FAILURE) {
      aos.side[side].cmds.prio=2;
      cmdRet.setError();
      cmdRet.addResult(_reason);  
   } else {
      aos.side[side].cmds.prio=4;
      cmdRet.setSuccess();
   }
   aos.side[side].updateargs+=1;           // Values on command display must be updated
   return cmdRet.serialize();
}

void AOSSubsystem::cmdUnlock()
{
_mutex.lock();
   aos.side[side].cmds.busy=0;
   _cmdbusy=false;
_mutex.unlock();
}


string AOSSubsystem::formatAOparams()
{
    ostringstream o;
    o << " dx:"<< aos.side[side].ao.param.dx;
    o << " dy:" << aos.side[side].ao.param.dy;
    o << " slNull:" << string(aos.side[side].ao.param.slnull);
    o << " f1:" << string(aos.side[side].ao.param.filter1);
    o << " f2:" << string(aos.side[side].ao.param.filter2);
    o << " freq:" << aos.side[side].ao.param.freq;
    o << " mag:" << aos.side[side].ao.param.mag;
    o << " nBins:" << aos.side[side].ao.param.nbins;
    o << " nModes:" << aos.side[side].ao.param.nmodes;
    o << " r0:" << aos.side[side].ao.param.r0;
    o << " snMode:" << aos.side[side].ao.param.snmode;                // T.B.D.
    o << " strehl:" << aos.side[side].ao.param.strehl;
    o << " ttMod:" << aos.side[side].ao.param.ttmodul;
    return o.str();
}

///////////////////////////////////////////////////////////
// COMMANDS
//
// Here follows the implementation of commands

namespace arb = Arcetri::Arbitrator;


string AOSSubsystem::PresetFlat(string flatSpec)
{
    arb::Command* scmd, * scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("PresetFlat");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.presetflat.stat=_cmd_stat;
        return cmdStatNotify();
    }
    
    aos.side[side].cmds.presetflat.flatspec=flatSpec;

    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.addText("(flat:" + flatSpec +")");
    cmdStartEv.logEvent();

    aos.side[side].cmds.presetflat.stat=CMD_RUNNING;

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    try {
        arb::presetFlatParams params;
        params.flatSpec=flatSpec;
        scmd = new arb::PresetFlat(aos.side[side].cmds.presetflat.tmout,params);
        scmdR = aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.presetflat.stat=CMD_SUCCESS;
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();          // Only unrecoverable errors are TCS errors
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.presetflat.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.presetflat.stat=CMD_FAILURE;
    } 
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].cmds.busy=0;
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};

string AOSSubsystem::PresetAO(string AOmode, Position refStar)
{
    double apMag,cIndex;
    MagnitudeType *mag;
    string strRet;

    AOSdbglog("PresetAO command (IIF mode) received");

    _refstar=refStar;

    if (refStar.hasMagnitude()) {
	mag    = refStar.Getmagnitude();
        apMag=mag->apparentMagnitude;
        cIndex=mag->color;
    }  else {
        AOSwnglog( "Reference star does not provide magnitude");
        apMag=numeric_limits<double>::quiet_NaN();
        cIndex=numeric_limits<double>::quiet_NaN();
    }

    PCSClient pcsclient=PCSClient();
    CSQHandle handle = pcsclient.computeKFPCoordinates(side, refStar);
    handle.block();
    CommandReturn result;
    result = handle.getResult();
    if(result.isSuccess()) {
        result.getArgument(&_refstar_x, 0);
        result.getArgument(&_refstar_y, 1);
        strRet=PresetAOg(AOmode,_wfsSpec,_refstar_x,_refstar_y,apMag,cIndex);
    } else { 
        strRet=logTCSerror(result,"computeKFPCoordinates(). PCS says:");
        aos.side[side].cmds.presetao.stat=CMD_FAILURE;
    }
    aos.side[side].cmds.busy=0;
    aos.side[side].updateargs+=1;
    return strRet;
};

string AOSSubsystem::PresetAOg(string AOmode, string wfsSpec, double rocoordx, 
                               double rocoordy, double mag, double cindex)
{
    arb::PresetAO* scmd;
    arb::PresetAO* scmdR;
    CommandReturn cmdRet;
    string cmdName("PresetAO");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.presetao.stat=_cmd_stat;
        return cmdStatNotify();
    }

    PresetAOClear(side);
    AOParamClear(side);
    RefineAOClear(side);
    ModifyAOClear(side);
    OffsetXYClear(side);
    OffsetZClear(side);

    string errMsg;
    double NaN=Arcetri::NaNutils::dNaN();
    double elev=Arcetri::NaNutils::dNaN();
    double rotang=Arcetri::NaNutils::dNaN();
    double gravang=Arcetri::NaNutils::dNaN();
    double windspeed=Arcetri::NaNutils::dNaN();
    double winddir=Arcetri::NaNutils::dNaN();

    aos.side[side].cmds.presetao.stat=CMD_RUNNING;

    double socoord1,socoord2;

    socoord1=pcs.side[side].pointingStatus.pointingOrigin.x;
    socoord2=pcs.side[side].pointingStatus.pointingOrigin.y;

    aos.side[side].cmds.presetao.rocoordx=rocoordx;
    aos.side[side].cmds.presetao.rocoordy=rocoordy;

    aos.side[side].cmds.presetao.mag=mag;
    aos.side[side].cmds.presetao.cindex=cindex;

    aos.side[side].cmds.presetao.aomode=AOmode;
    aos.side[side].cmds.presetao.instr=_authorizedInstr;
    aos.side[side].cmds.presetao.focStation=_authorizedFocStn;
    aos.side[side].cmds.presetao.wfsSpec=wfsSpec;
    aos.side[side].cmds.presetao.r0=NaN;           // Unused: value got from AOS.DIMM.SEEING
    aos.side[side].cmds.presetao.skybrgt=NaN;      // Currently not available

    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    ostringstream o;
    o<<"(aomode:"   << AOmode << ",";
    o<<"focalst:"   << _authorizedFocStn << ",";
    o<<"instr:"     << _authorizedInstr << ",";
    o<<"wfs:"       << wfsSpec << ",";
    o<<"so1:"       << socoord1 << ",";
    o<<"so2:"       << socoord2 << ",";
    o<<"ro1:"       << rocoordx << ",";
    o<<"ro2:"       << rocoordy << ",";
    o<<"mag:"       << (double)aos.side[side].cmds.presetao.mag << ",";
    o<<"cindex:"    << (double)aos.side[side].cmds.presetao.cindex << ",";
    o<<"elev:"      << elev << ",";
    o<<"rotang:"    << rotang << ",";
    o<<"gravang:"   << gravang << ",";
    o<<"r0:"        << (double)aos.side[side].cmds.presetao.r0 << ",";
    o<<"skybrgt:"   << (double)aos.side[side].cmds.presetao.skybrgt << ",";
    o<<"windspeed:" << windspeed << ",";
    o<<"winddir:"   << winddir << ")";
    cmdStartEv.addText(o.str());
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    if(!_setWFS(wfsSpec)) {          // Check WFS setting
        string errMsg="Illegal WFS spec: "+wfsSpec;
        cmdEndEv.setFailure(errMsg);
        cmdRet.addResult(errMsg);
        aos.side[side].cmds.stat=errMsg;
        aos.side[side].cmds.presetao.stat=CMD_FAILURE;
        aos.side[side].updateargs+=1;
        cmdUnlock();
        return cmdRet.serialize();
    }
    presetAOParams params;
    params.aoMode=aos.side[side].cmds.presetao.aomode;
    params.focStation=aos.side[side].cmds.presetao.focStation;
    params.instr=aos.side[side].cmds.presetao.instr;
    params.wfsSpec=wfsSpec;
    params.soCoord[0]=socoord1;
    params.soCoord[1]=socoord2;
    params.roCoord[0]=rocoordx;
    params.roCoord[1]=rocoordy;
    params.elevation=elev;
    params.rotAngle=rotang;
    params.gravAngle=gravang;
    params.mag=aos.side[side].cmds.presetao.mag;
    params.color=aos.side[side].cmds.presetao.cindex;
    params.r0=aos.side[side].cmds.presetao.r0;
    params.skyBrgt=aos.side[side].cmds.presetao.skybrgt;
    params.windSpeed=windspeed;
    params.windDir=winddir;
    try {
        scmd = new arb::PresetAO(aos.side[side].cmds.presetao.tmout,params);
        scmdR = (arb::PresetAO *)aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.presetao.stat=CMD_SUCCESS;
                                                          // Get return values
            arb::acquireRefAOResult result=scmdR->getResult();
            aos.side[side].ao.param.dx=Arcetri::NaNutils::dNaN();
            aos.side[side].ao.param.dy=Arcetri::NaNutils::dNaN();
            aos.side[side].ao.param.slnull=result.slNull;
            aos.side[side].ao.param.filter1=result.f1spec;
            aos.side[side].ao.param.filter2=result.f2spec;
            aos.side[side].ao.param.freq=result.freq;
            aos.side[side].ao.param.mag=result.starMag;
            aos.side[side].ao.param.nbins=result.nBins;
            aos.side[side].ao.param.nmodes=result.nModes;
            aos.side[side].ao.param.r0=result.r0;
            aos.side[side].ao.param.snmode=0.0;                // T.B.D.
            aos.side[side].ao.param.strehl=result.strehl;
            aos.side[side].ao.param.ttmodul=result.ttMod;
            string msg=formatAOparams();
            cmdEndEv.setSuccess(msg);
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.presetao.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.presetao.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};


string AOSSubsystem::CheckRefAO()
{
    arb::CheckRefAO* scmd, *scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("CheckRef");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.checkref.stat=_cmd_stat;
        return cmdStatNotify();
    }

    OffsetXYClear(side);
    OffsetZClear(side);
    AOTVImagesClear(side);

    aos.side[side].cmds.checkref.stat=CMD_RUNNING;

    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    try {
        scmd = new arb::CheckRefAO(aos.side[side].cmds.checkref.tmout);
        scmdR = (arb::CheckRefAO *)aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            arb::checkRefAOResult result=scmdR->getResult();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.checkref.stat=CMD_SUCCESS;
            aos.side[side].ao.param.dx=result.deltaXY[0];
            aos.side[side].ao.param.dy=result.deltaXY[1];
            aos.side[side].ao.param.mag=result.starMag;
            ostringstream o;
            o << "dx:" << (double)aos.side[side].ao.param.dx<<",";
            o << "dy:" << (double)aos.side[side].ao.param.dy<<",";
            o << "mag:" << (double)aos.side[side].ao.param.mag;
            cmdEndEv.setSuccess(o.str());
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.checkref.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.checkref.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};

string AOSSubsystem::AcquireRefAO(bool rePoint)
{
    arb::AcquireRefAO *aqscmd, *aqscmdR;
    arb::CheckRefAO *ckscmd, *ckscmdR;
    CommandReturn cmdRet;
    string errMsg;
    bool checkrefOK;
    string cmdName("AcquireRef");

    RefineAOClear(side);
    ModifyAOClear(side);
    OffsetXYClear(side);
    OffsetZClear(side);
    AOTVImagesClear(side);

    printf("Repoint: %d\n",rePoint);
    aos.side[side].cmds.acquireref.repoint=rePoint?1:0;

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.acquireref.stat=_cmd_stat;
        return cmdStatNotify();
    }

    aos.side[side].cmds.acquireref.stat=CMD_RUNNING;

    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    checkrefOK=true;
    if(rePoint) {              // We want the stages not to move
        double x,y;
        AOSinfolog("Computing telescope offset for AcquireRefAO");
        try {                     // So we get reference star position offset
            ckscmd = new arb::CheckRefAO(aos.side[side].cmds.checkref.tmout);
            ckscmdR = (arb::CheckRefAO *)aoapp->arbIntf->requestCommand(ckscmd);
            if(ckscmdR->isSuccess()) {
                cmdRet.setSuccess();
                arb::checkRefAOResult result=ckscmdR->getResult();
                ostringstream o;
                o << "dx:" << result.deltaXY[0] << ",";
                o << "dy:" << result.deltaXY[1] <<",";
                AOSinfolog(o.str());
                x=(double)aos.side[side].cmds.presetao.rocoordx;
                x+=result.deltaXY[0];
                y=(double)aos.side[side].cmds.presetao.rocoordy;
                y+=result.deltaXY[1];
                o.str("");
                o << "Telescope offset required - x: " << aos.side[side].cmds.presetao.rocoordx << " -> " << x;
                o << ";  y: " << aos.side[side].cmds.presetao.rocoordy << " -> " << y;
                AOSinfolog( o.str());
                                   // Adjust telescope pointing
                PCSClient pcsclient=PCSClient();
                CSQHandle handle = pcsclient.updateReferenceOrigin (side, x, y);
                handle.block ();
                CommandReturn res;
                res = handle.getResult();
                if(res.isError()) {
                    string strRet=logTCSerror(res,"updateReferenceOrigin(). PCS says:");
                    aos.side[side].cmds.prio=1;
                    aos.side[side].cmds.stat=strRet;
                    aos.side[side].cmds.offsetxy.stat=CMD_FAILURE;
                    checkrefOK=false;
                    cmdRet.addResult(strRet);
                }
            } else {
                if(ckscmdR->isError()) {
                    cmdRet.setError();
                    aos.side[side].cmds.prio=1;
                } else {
                    cmdRet.setError();
                    aos.side[side].cmds.prio=2;
                }
                errMsg=ckscmdR->getErrorString();
                checkrefOK=false;
                aos.side[side].cmds.acquireref.stat=CMD_FAILURE;
                aos.side[side].cmds.stat=errMsg;
            }
        } catch(ArbitratorInterfaceException& e) {
            cmdRet.setError();
            errMsg=e.what(Terse);
            aos.side[side].cmds.prio=1;
            aos.side[side].cmds.stat=e.what(Hmi);
            aos.side[side].cmds.acquireref.stat=CMD_FAILURE;
            checkrefOK=false;
        }
        delete ckscmd;
    }
    if(checkrefOK) {
        try {
            aqscmd = new arb::AcquireRefAO(aos.side[side].cmds.acquireref.tmout);
            aqscmdR = (arb::AcquireRefAO *)aoapp->arbIntf->requestCommand(aqscmd);
            if(aqscmdR->isSuccess()) {
                cmdRet.setSuccess();
                aos.side[side].cmds.stat=complete;
                aos.side[side].cmds.acquireref.stat=CMD_SUCCESS;
                                                              // Get return values
                arb::acquireRefAOResult result=aqscmdR->getResult();
                aos.side[side].ao.param.dx=result.deltaXY[0];
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
                aos.side[side].ao.param.ttmodul=result.ttMod;
                ostringstream o;
                o << "Stages offset applied - dx: " << result.deltaXY[0] << ";  dy: " << result.deltaXY[1];
                AOSinfolog( o.str());

                aoapp->tvUpdateAcquisition(result.TVframe,256*256+2*sizeof(int));
                string msg=formatAOparams();
                cmdEndEv.setSuccess(msg);
            } else {
                if(aqscmdR->isError()) {
                    cmdRet.setError();
                    aos.side[side].cmds.prio=1;
                } else {
                    cmdRet.setError();
                    aos.side[side].cmds.prio=2;
                }
                errMsg=aqscmdR->getErrorString();
                cmdEndEv.setFailure(errMsg);
                cmdRet.addResult(errMsg);
                aos.side[side].cmds.stat=errMsg;
                aos.side[side].cmds.acquireref.stat=CMD_FAILURE;
            }
        } catch(ArbitratorInterfaceException& e) {
            cmdRet.setError();
            cmdEndEv.setFailure(e.what(Terse));
            aos.side[side].cmds.prio=1;
            aos.side[side].cmds.stat=e.what(Hmi);
            aos.side[side].cmds.acquireref.stat=CMD_FAILURE;
        }
        delete aqscmd;
     }

    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};

string AOSSubsystem::RefineAO(string method)
{
    arb::RefineAO *scmd,  *scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName=("Refine");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.refine.stat=_cmd_stat;
        return cmdStatNotify();
    }

    RefineAOClear(side);
    ModifyAOClear(side);
    OffsetXYClear(side);
    OffsetZClear(side);

    aos.side[side].cmds.refine.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.addText("(method:" + method+")");
    cmdStartEv.logEvent();

    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    aos.side[side].cmds.refine.method=method;
    
    aos.side[side].updateargs+=1;           // Values on command display must be updated

    try {
        refineAOParams params;
        params.method=method;
        scmd = new arb::RefineAO(aos.side[side].cmds.refine.tmout,params);
        scmdR = (arb::RefineAO *)aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.refine.stat=CMD_SUCCESS;
            arb::acquireRefAOResult result=scmdR->getResult();
            aos.side[side].ao.param.dx=result.deltaXY[0];
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
            aos.side[side].ao.param.ttmodul=result.ttMod;

            aoapp->tvUpdateAcquisition(result.TVframe,256*256+2*sizeof(int));
            string msg=formatAOparams();
            cmdEndEv.setSuccess(msg);
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.refine.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.refine.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};


string AOSSubsystem::ModifyAO(int nModes, double freq, int nBins, double ttMod, string f1, string f2)
{
    arb::ModifyAO *scmd, *scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("Modify");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.modify.stat=_cmd_stat;
        return cmdStatNotify();
    }

    ModifyAOClear(side);
    OffsetXYClear(side);
    OffsetZClear(side);
    AOTVImagesClear(side);

    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    ostringstream o;
    o<<"(nmodes:"<< nModes << ",";
    o<<"freq:"<< freq << ",";
    o<<"nbins:"<< nBins << ",";
    o<<"ttmod:"<< ttMod << ",";
    o<<"f1:"<< f1 << ",";
    o<<"f2:"<< f2 << ")";
    cmdStartEv.addText(o.str());
    cmdStartEv.logEvent();

    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);
    aos.side[side].cmds.modify.stat=CMD_RUNNING;

    aos.side[side].cmds.modify.nmodes=nModes;
    aos.side[side].cmds.modify.freq=freq;
    aos.side[side].cmds.modify.nbins=nBins;
    aos.side[side].cmds.modify.ttmod=ttMod;
    aos.side[side].cmds.modify.filter1=f1;
    aos.side[side].cmds.modify.filter2=f2;

    aos.side[side].updateargs+=1;           // Values on command display must be updated

    try {
        modifyAOParams params;
        params.nModes= nModes; 
        params.freq= freq; 
        params.binning= nBins; 
        params.TTmod= ttMod; 
        params.f1spec=f1;
        params.f2spec=f2;

        scmd = new arb::ModifyAO(aos.side[side].cmds.modify.tmout,params);
        scmdR = (arb::ModifyAO *)aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.modify.stat=CMD_SUCCESS;

                                                          // Get return values
            arb::acquireRefAOResult result=scmdR->getResult();
            aos.side[side].ao.param.dx=result.deltaXY[0];
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
            aos.side[side].ao.param.ttmodul=result.ttMod;
            aoapp->tvUpdateAcquisition(result.TVframe,256*256+2*sizeof(int));
            string msg=formatAOparams();
            cmdEndEv.setSuccess(msg);
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.modify.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.modify.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};

string AOSSubsystem::StartAO()
{
    extern time_t  _startTime;
    arb::Command* scmd, * scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("StartAO");


    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.startao.stat=_cmd_stat;
        return cmdStatNotify();
    }

    _startTime=time(NULL);        // Preset session timer
    OffsetXYClear(side);
    OffsetZClear(side);
    StopClear(side);

    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.logEvent();

    aos.side[side].cmds.startao.stat=CMD_RUNNING;
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    aos.side[side].updateargs+=1;           // Values on command display must be updated

    try {
        scmd = new arb::StartAO(aos.side[side].cmds.startao.tmout);
        scmdR = aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.startao.stat=CMD_SUCCESS;
            aos.side[side].ao.loopon=LOOP_ON;   // Force status (to avoid timing
                                                // problems in variable update)
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.startao.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.startao.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};

string AOSSubsystem::OffsetXY()
{
    double px,py;
    double dx,dy;
    string strRet;

    AOSdbglog("OffsetXY (IIF) command received");

    CommandReturn result;
    PCSClient pcsclient=PCSClient();
    CSQHandle handle = pcsclient.computeKFPCoordinates(side, _refstar);
    handle.block ();
    result = handle.getResult();
    if(result.isSuccess()) {
        result.getArgument(&px, 0);
        result.getArgument(&py, 1);
        dx=px-_refstar_x;
        dy=py-_refstar_y;
        strRet = OffsetXYg(dx,dy);
        if(aos.side[side].cmds.offsetxy.stat=CMD_SUCCESS) { _refstar_x=px; _refstar_y=py; }
    } else { 
        strRet=logTCSerror(result,"computeKFPCoordinates(). PCS says:");
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=strRet;
        aos.side[side].cmds.offsetxy.stat=CMD_FAILURE;
    }
    aos.side[side].updateargs+=1;
    return strRet;
};

string AOSSubsystem::OffsetXYg(double ofsX, double ofsY)
{
    CommandReturn cmdRet;
    arb::Command* scmd, * scmdR;
    string errMsg;
    string cmdName("OffsetXY");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.offsetxy.stat=_cmd_stat;
        return cmdStatNotify();
    }

    OffsetXYClear(side);

    aos.side[side].cmds.offsetxy.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    ostringstream o;
    o << "(ofsx:" << ofsX << ",";
    o << "ofsy:" << ofsY << ")";
    cmdStartEv.addText(o.str());
    cmdStartEv.logEvent();

    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    aos.side[side].cmds.offsetxy.x=ofsX;
    aos.side[side].cmds.offsetxy.y=ofsY;

    aos.side[side].updateargs+=1;           // Values on command display must be updated

    offsetXYParams params;
    params.deltaXY[0]=ofsX;
    params.deltaXY[1]=ofsY;
    if(aos.side[side].ao.loopon==(long)LOOP_ON)       // Check the loop status
       aos.side[side].cmds.offsetxy.tmout=(int)(max(fabs(ofsX),fabs(ofsY))*_OffsetSpeedFactorCL)+_AOOffsetXYTmo;
    else
       aos.side[side].cmds.offsetxy.tmout=(int)(max(fabs(ofsX),fabs(ofsY))*_OffsetSpeedFactorOL)+_AOOffsetZTmo;
    try {
        scmd = new arb::OffsetXY(aos.side[side].cmds.offsetxy.tmout,params);
        scmdR = aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.offsetxy.stat=CMD_SUCCESS;
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.offsetxy.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.offsetxy.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};

string AOSSubsystem::OffsetZ(double dz)
{
    arb::OffsetZ* scmd, * scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("OffsetZ");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.offsetz.stat=_cmd_stat;
        return cmdStatNotify();
    }

    OffsetZClear(side);

    aos.side[side].cmds.offsetz.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    ostringstream o;
    o << "(ofstz:" << dz <<")";
    cmdStartEv.addText(o.str());
    cmdStartEv.logEvent();

    aos.side[side].cmds.offsetz.z=dz;

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    offsetZParams params;
    params.deltaZ=dz;
    if(aos.side[side].ao.loopon==(long)LOOP_ON)       // Check the loop status
       aos.side[side].cmds.offsetz.tmout=(int)(fabs(dz)*_OffsetSpeedFactorCL)+10000;
    else
       aos.side[side].cmds.offsetz.tmout=(int)(fabs(dz)*_OffsetSpeedFactorOL)+10000;
    try {
        scmd = new arb::OffsetZ(aos.side[side].cmds.offsetz.tmout,params);
        scmdR = (arb::OffsetZ *)aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.offsetz.stat=CMD_SUCCESS;
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.offsetz.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.offsetz.stat=CMD_FAILURE;
    }
    delete scmd;

    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};

string AOSSubsystem::CorrectModes(double modes[N_CORRECTMODES])
{
    arb::CorrectModes *scmd, *scmdR;
    correctModesParams params;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("CorrectModes");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.correctmodes.stat=_cmd_stat;
        return cmdStatNotify();
    }

    aos.side[side].cmds.correctmodes.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    params.deltaM[0]=modes[0]*TO_METERS;
    ostringstream o;
    o<<"(deltaM:"<< modes[0];
    for(int i=1;i<N_CORRECTMODES;i++) {
       o<<","<< modes[i];
       params.deltaM[i]=modes[i]*TO_METERS;
    }
    o<<")";
    cmdStartEv.addText(o.str());
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    try {
        scmd = new arb::CorrectModes(aos.side[side].cmds.correctmodes.tmout,params);
        scmdR = (arb::CorrectModes *)aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.correctmodes.stat=CMD_SUCCESS;
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.correctmodes.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.correctmodes.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};

string AOSSubsystem::SetZernikes(int n, double zernikes[N_ZERN])
{
extern double zern_gain[N_ZERN];
arb::SetZernikes *scmd, *scmdR;
setZernikesParams params;
CommandReturn cmdRet;
string errMsg;
string cmdName("SetZernikes");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.setzernikes.stat=_cmd_stat;
        return cmdStatNotify();
    }
    aos.side[side].cmds.setzernikes.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);

    for(int i=0;i<N_ZERN;i++) {
       aos.side[side].cmds.setzernikes.zernikes[i]=zernikes[i];
       params.deltaM[i]=zernikes[i]*zern_gain[i];
    }

    ostringstream o;
    o<<"(n:" << n << ", zernikes:" << zernikes[0];
    for(int i=1;i<n;i++) o<<","<< zernikes[i];
    o<<")";
    cmdStartEv.addText(o.str());
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    try {
        scmd = new arb::SetZernikes(aos.side[side].cmds.setzernikes.tmout,params);
        scmdR = (arb::SetZernikes *)aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.setzernikes.stat=CMD_SUCCESS;
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.setzernikes.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.setzernikes.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};


string AOSSubsystem::Stop(string msg)
{
    arb::Command* scmd, * scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("Stop");

    if(cmdCheck(cmdName,MASTER_LOG_LEV_INFO)) {
        if(_reason_code==CHK_SIMULATION) {
            aos.side[side].cmds.stop.stat=CMD_SUCCESS;
            return cmdStatNotify();
        }
 // Note: Failure to send command because the AO system is not ready
 //       is not considered a fault for Stop command, so we return success
        cmdRet.setSuccess();
        aos.side[side].cmds.stat=complete;
        aos.side[side].cmds.stop.stat=CMD_SUCCESS;
        aos.side[side].updateargs+=1;
        return cmdRet.serialize();
    }

    aos.side[side].cmds.stop.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.addText("(msg:"+ msg +")");
    cmdStartEv.logEvent();

    aos.side[side].cmds.stop.msg=msg;

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    stopParams params;
    params.msg=msg;
    try {
        scmd = new arb::Stop(aos.side[side].cmds.stop.tmout, params);
        scmdR = aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.stop.stat=CMD_SUCCESS;
            aos.side[side].ao.loopon=LOOP_OFF;   // Force status (to avoid timing
                                                 // problems in variable update)
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setSuccess();
                errMsg=scmdR->getErrorString();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.stop.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.stop.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};

string AOSSubsystem::Pause()
{
    arb::Command* scmd, * scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("Pause");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.pause.stat=_cmd_stat;
        return cmdStatNotify();
    }

    aos.side[side].cmds.pause.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    try {
        scmd = new arb::Pause(aos.side[side].cmds.pause.tmout);
        scmdR = aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.pause.stat=CMD_SUCCESS;
            aos.side[side].ao.loopon=LOOP_PAUSED;   // Force status (to avoid timing
                                                    // problems in variable update)
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.pause.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.pause.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};

string AOSSubsystem::Resume()
{
    arb::Command* scmd, * scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("Resume");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.resume.stat=_cmd_stat;
        return cmdStatNotify();
    }

    aos.side[side].cmds.resume.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    try {
        scmd = new arb::Resume(aos.side[side].cmds.resume.tmout);
        scmdR = aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.resume.stat=CMD_SUCCESS;
            aos.side[side].ao.loopon=LOOP_ON;   // Force loop status (to avoid timing
                                                // problems in variable update)
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.resume.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.resume.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};


string AOSSubsystem::UserPanic(string reason)
{
    arb::Command* scmd, * scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("UserPanic");

    if(cmdCheck(cmdName,MASTER_LOG_LEV_WARNING,true)) {
        aos.side[side].cmds.userpanic.stat=_cmd_stat;
        return cmdStatNotify();
    }

    aos.side[side].cmds.userpanic.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.addText("(reason:"+reason+")");
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    if(aoapp && aoapp->msgd_up()) {
        userPanicParams params;
        params.msg=reason;
        try {
            scmd = new arb::UserPanic(aos.side[side].cmds.userpanic.tmout,params);
            scmdR = aoapp->arbIntf->requestCommand(scmd);
            if(scmdR->isSuccess()) {
                cmdRet.setSuccess();
                cmdEndEv.setSuccess();
                aos.side[side].cmds.stat=complete;
                aos.side[side].cmds.userpanic.stat=CMD_SUCCESS;
            } else {
                if(scmdR->isError()) {
                    cmdRet.setError();
                    aos.side[side].cmds.prio=1;
                } else {
                    cmdRet.setError();
                    aos.side[side].cmds.prio=2;
                }
                errMsg=scmdR->getErrorString();
                cmdRet.addResult(errMsg);
                cmdEndEv.setFailure(errMsg);
                aos.side[side].cmds.stat=errMsg;
                aos.side[side].cmds.userpanic.stat=CMD_FAILURE;
            }
        } catch(ArbitratorInterfaceException& e) {
            cmdRet.setError();
            cmdEndEv.setFailure(e.what(Terse));
            aos.side[side].cmds.prio=1;
            aos.side[side].cmds.stat=e.what(Hmi);
            aos.side[side].cmds.userpanic.stat=CMD_FAILURE;
        }
        delete scmd;
    } else {
        cmdEndEv.setFailure(nocommerr);
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=nocommerr;
        cmdRet.setError();
        aos.side[side].cmds.userpanic.stat=CMD_FAILURE;
    }
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};

void AOSSubsystem::_setInstr(string instr, string focalSt)
{
    extern map<string,string> instrFlats;

    _authorizedFocStn=focalSt;
    _authorizedInstr=instr;   

                              // The following is an heuristic to
                              // Determine the WFS type from SetNew Instrument command
    _wfsSpec="";
    if(focalSt==PCSFS_BENTGREGFRONT) { // When Bentgregorian front is selected,
        _setWFS(AO_WFS_FLAO);          // Preset WfsSpec to FLAOWFS
    } else if(instr==PCSIN_LBTI) {     // When instrument spec is LBTI
        _setWFS(AO_WFS_LBTI);          // Preset WfsSpec to LBTIWFS
    } else {
        _setWFS(AO_WFS_NONE); 
    }
    aos.side[side].cmds.presetao.instr=_authorizedInstr;
    aos.side[side].cmds.presetao.focStation=_authorizedFocStn;
    aos.side[side].cmds.presetao.wfsSpec=_wfsSpec;

                              // Now the Flat specification for
    _flatSpec="";             // the given instrument is selected
    map<string,string>::iterator flat=instrFlats.find(instr);
    if(flat==instrFlats.end()) {  
       _flatSpec="default";
    } else {
       _flatSpec=(*flat).second;
    }
    aos.side[side].cmds.presetflat.flatspec=_flatSpec;

    aos.side[side].updateargs+=1;           // Values on command display must be updated
}

string AOSSubsystem::SetNewInstrument(string instr, string focalSt)
{
    CommandReturn cmdRet;
    _setInstr(instr,focalSt);

    ostringstream o;
    o << "Received SetNewInstrument("<<instr<<","<<focalSt<<"). Set Flat to: "<<_flatSpec<<"  Set WFS spec to: "<<_wfsSpec;
    AOSinfolog(o.str());

    if(aos.side[side].ao.sl_ready==1L) return PresetFlat(_flatSpec);

    cmdRet.setSuccess();
    return cmdRet.serialize();
};

string AOSSubsystem::AdsecOn()
{
    arb::Command* scmd, * scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("AdsecOn");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.adsecon.stat=_cmd_stat;
        return cmdStatNotify();
    }

    aos.side[side].cmds.adsecon.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    try {
        scmd = new arb::PowerOnAdSec(aos.side[side].cmds.adsecon.tmout);
        scmdR = aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.adsecon.stat=CMD_SUCCESS;
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.adsecon.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.adsecon.stat=CMD_FAILURE;
    }
    delete scmd;

    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};

string AOSSubsystem::AdsecOff()
{
    arb::Command* scmd, * scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("AdsecOff");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.adsecoff.stat=_cmd_stat;
        return cmdStatNotify();
    }

    aos.side[side].cmds.adsecoff.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    try {
        scmd = new arb::PowerOffAdSec(aos.side[side].cmds.adsecoff.tmout);
        scmdR = aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.adsecoff.stat=CMD_SUCCESS;
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdEndEv.setFailure(errMsg);
            cmdRet.addResult(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.adsecoff.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.adsecoff.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};


string AOSSubsystem::AdsecSet()
{
    arb::Command* scmd, * scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("AdsecSet");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.adsecset.stat=_cmd_stat;
        return cmdStatNotify();
    }

    aos.side[side].cmds.adsecset.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    try {
        scmd = new arb::MirrorSet(aos.side[side].cmds.adsecset.tmout);
        scmdR = aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.adsecset.stat=CMD_SUCCESS;
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdRet.addResult(errMsg);
            cmdEndEv.setFailure(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.adsecset.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.adsecset.stat=CMD_FAILURE;
    }
    delete scmd;
                        // Now, if necessary and possible, send a pesetFlat command
    if((aos.side[side].cmds.adsecset.stat==CMD_SUCCESS) 
                            &&  (_flatSpec!="default") ) {
        AOSinfolog("Doing PresetFlat("+_flatSpec+")");
        try {
            arb::presetFlatParams params;
            params.flatSpec=_flatSpec;
            scmd = new arb::PresetFlat(aos.side[side].cmds.presetflat.tmout,params);
            scmdR = aoapp->arbIntf->requestCommand(scmd);
            if(!scmdR->isSuccess()) {
                if(scmdR->isError()) {
                    cmdRet.setError();          // Only unrecoverable errors are TCS errors
                    aos.side[side].cmds.prio=1;
                } else {
                    aos.side[side].cmds.prio=2;
                }
                errMsg=scmdR->getErrorString();
                cmdEndEv.setFailure(errMsg);
                cmdRet.addResult(errMsg);
                aos.side[side].cmds.stat=errMsg;
                aos.side[side].cmds.adsecset.stat=CMD_FAILURE;
            }
        } catch(ArbitratorInterfaceException& e) {
            cmdRet.setError();
            cmdEndEv.setFailure(e.what(Terse));
            aos.side[side].cmds.prio=1;
            aos.side[side].cmds.stat=e.what(Hmi);
            aos.side[side].cmds.presetflat.stat=CMD_FAILURE;
        } 
        delete scmd;
    } 
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};


string AOSSubsystem::AdsecRest()
{
    arb::Command* scmd, * scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("AdsecRest");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.adsecrest.stat=_cmd_stat;
        return cmdStatNotify();
    }

    aos.side[side].cmds.adsecrest.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    try {
        scmd = new arb::MirrorRest(aos.side[side].cmds.adsecrest.tmout);
        scmdR = aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.adsecrest.stat=CMD_SUCCESS;
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdRet.addResult(errMsg);
            cmdEndEv.setFailure(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.adsecrest.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.adsecrest.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};


string AOSSubsystem::WfsOn(string wfsId)
{
    arb::Command* scmd, * scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("WfsOn");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.wfson.stat=_cmd_stat;
        return cmdStatNotify();
    }

    aos.side[side].cmds.wfson.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.addText("(wfs:"+wfsId+")");
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

   wfsOnOffParams param;
   param.wfsid=wfsId;
    try {
        scmd = new arb::PowerOnWfs(aos.side[side].cmds.wfson.tmout,param);
        scmdR = aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.wfson.stat=CMD_SUCCESS;
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdRet.addResult(errMsg);
            cmdEndEv.setFailure(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.wfson.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.wfson.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};


string AOSSubsystem::WfsOff(string wfsId)
{
    arb::Command* scmd, * scmdR;
    CommandReturn cmdRet;
    string errMsg;
    string cmdName("WfsOff");

    if(cmdCheck(cmdName)) {
        aos.side[side].cmds.wfsoff.stat=_cmd_stat;
        return cmdStatNotify();
    }

    aos.side[side].cmds.wfsoff.stat=CMD_RUNNING;
    AOSCmdStartEvent cmdStartEv(cmdName,_cmdnum);
    cmdStartEv.addText("(wfs:"+wfsId+")");
    cmdStartEv.logEvent();

    aos.side[side].updateargs+=1;           // Values on command display must be updated
    AOSCmdEndEvent cmdEndEv(cmdName,_cmdnum);

    wfsOnOffParams param;
    param.wfsid=wfsId;
    try {
        scmd = new arb::PowerOffWfs(aos.side[side].cmds.wfsoff.tmout,param);
        scmdR = aoapp->arbIntf->requestCommand(scmd);
        if(scmdR->isSuccess()) {
            cmdRet.setSuccess();
            cmdEndEv.setSuccess();
            aos.side[side].cmds.stat=complete;
            aos.side[side].cmds.wfsoff.stat=CMD_SUCCESS;
        } else {
            if(scmdR->isError()) {
                cmdRet.setError();
                aos.side[side].cmds.prio=1;
            } else {
                cmdRet.setError();
                aos.side[side].cmds.prio=2;
            }
            errMsg=scmdR->getErrorString();
            cmdRet.addResult(errMsg);
            cmdEndEv.setFailure(errMsg);
            aos.side[side].cmds.stat=errMsg;
            aos.side[side].cmds.wfsoff.stat=CMD_FAILURE;
        }
    } catch(ArbitratorInterfaceException& e) {
        cmdRet.setError();
        cmdEndEv.setFailure(e.what(Terse));
        aos.side[side].cmds.prio=1;
        aos.side[side].cmds.stat=e.what(Hmi);
        aos.side[side].cmds.wfsoff.stat=CMD_FAILURE;
    }
    delete scmd;
    
    string ret=cmdEndEv.logEvent();
    cmdRet.addResult(ret);
    aos.side[side].updateargs+=1;
    cmdUnlock();
    return cmdRet.serialize();
};

} // namespace lbto
