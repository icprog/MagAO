/* FSM implementation
 * Automatically generated for FSM: WfsFsmCore by FSMC vers. 1.1.1 */

#include "WfsFsmCore.h"

void WfsFsmCore::__processEvent( Event e )
{
   States yOld = __Y;
   bool pass = false;
   switch( __Y ) { //transitions 
   case Operating:
      if( e == Off && processCommand() ) { // Off
         //outcome actions
         __Y = PowerOff;
         pass = true;
      }
      else if( e == Operate && processCommand() ) { // Operate
         pass = false;
      }
      else if( e == PrepareAcquireRef && processCommand() ) { // PrepareAcquireRef
         //outcome actions
         __Y = AOPrepared;
         pass = true;
      }
      else if( e == StopLoop && processCommand() ) { // StopLoop
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      else if( e == AutoTrack && processCommand() ) { // AutoTrack
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      else if( e == AntiDrift && processCommand() ) { // AntiDrift
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      else if( e == EnableDisturb && processCommand() ) { // EnableDisturb
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      else if( e == SaveOptLoopData && processCommand() ) { // SaveOptLoopData
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      else if( e == SaveStatus && processCommand() ) { // SaveStatus
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      else if( e == CalibrateHODark && processCommand() ) { // CalibrateHODark
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      else if( e == CalibrateTVDark && processCommand() ) { // CalibrateTVDark
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      else if( e == CalibrateIRTCDark && processCommand() ) { // CalibrateIRTCDark
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      else if( e == CalibrateSlopenull && processCommand() ) { // CalibrateSlopenull
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      else if( e == ModifyAO && processCommand() ) { // ModifyAO
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == CalibratePISCESDark && processCommand() ) { // CalibratePISCESDark
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      break;
   case Failure:
      if( e == RecoverFailure && processCommand() ) { // RecoverFailure (Operate)
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      break;
   case LoopClosed:
      if( e == PauseLoop && processCommand() ) { // PauseLoop
         //outcome actions
         __Y = LoopPaused;
         pass = true;
      }
      else if( e == StopLoop && processCommand()
 ) { // StopLoop
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == RefineLoop && processCommand() ) { // RefineLoop
         pass = false;
      }
      else if( e == OffsetXY && processCommand() ) { // OffsetXY
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      else if( e == OffsetZ && processCommand() ) { // OffsetZ
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      else if( e == AntiDrift && processCommand() ) { // AntiDrift
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      else if( e == AutoTrack && processCommand() ) { // AutoTrack
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      else if( e == EnableDisturb && processCommand() ) { // EnableDisturb
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      else if( e == SaveOptLoopData && processCommand() ) { // SaveOptLoopData
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      else if( e == SaveStatus && processCommand() ) { // SaveStatus
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      else if( e == CalibrateIRTCDark && processCommand() ) { // CalibrateIRTCDark
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      else if( e == CalibratePISCESDark && processCommand() ) { // CalibratePISCESDark
         pass = false;
      }
      else if( e == OptimizeGain && processCommand() ) { // OptimizeGain
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      else if( e == ModifyAO && processCommand() ) { // ModifyAo
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      break;
   case AOPrepared:
      if( e == AcquireRef && processCommand() ) { // AcquireRef
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == Operate && processCommand() ) { // Operate
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      else if( e == StopLoop && processCommand() ) { // StopLoop
         //outcome actions
         __Y = AOPrepared;
         pass = true;
      }
      else if( e == AntiDrift && processCommand() ) { // AntiDrift
         //outcome actions
         __Y = AOPrepared;
         pass = true;
      }
      else if( e == AutoTrack && processCommand() ) { // AutoTrack
         //outcome actions
         __Y = AOPrepared;
         pass = true;
      }
      else if( e == CalibrateHODark && processCommand() ) { // CalibrateHODark
         //outcome actions
         __Y = AOPrepared;
         pass = true;
      }
      else if( e == SaveOptLoopData && processCommand() ) { // SaveOptLoopData
         //outcome actions
         __Y = AOPrepared;
         pass = true;
      }
      else if( e == SaveStatus && processCommand() ) { // SaveStatus
         //outcome actions
         __Y = AOPrepared;
         pass = true;
      }
      else if( e == CheckRef && processCommand() ) { // CheckRef
         pass = false;
      }
      else if( e == ModifyAO && processCommand() ) { // ModifyAO
         //outcome actions
         __Y = AOPrepared;
         pass = true;
      }
      else if( e == Off && processCommand() ) { // Off
         //outcome actions
         __Y = PowerOff;
         pass = true;
      }
      else if( e == PrepareAcquireRef && processCommand() ) { // PrepareAcquireRef
         //outcome actions
         __Y = AOPrepared;
         pass = true;
      }
      else if( e == CalibratePISCESDark && processCommand() ) { // CalibratePISCESDark
         //outcome actions
         __Y = AOPrepared;
         pass = true;
      }
      else if( e == CalibrateIRTCDark && processCommand() ) { // CalibrateIRTCDark
         //outcome actions
         __Y = AOPrepared;
         pass = true;
      }
      break;
   case LoopPaused:
      if( e == ResumeLoop && processCommand() ) { // ResumeLoop
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      else if( e == StopLoop && processCommand() ) { // StopLoop
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == OffsetXY && processCommand() ) { // OffsetXY
         //outcome actions
         __Y = LoopPaused;
         pass = true;
      }
      else if( e == OffsetZ && processCommand() ) { // OffsetZ
         //outcome actions
         __Y = LoopPaused;
         pass = true;
      }
      else if( e == AntiDrift && processCommand() ) { // AntiDrift
         //outcome actions
         __Y = LoopPaused;
         pass = true;
      }
      else if( e == AutoTrack && processCommand() ) { // AutoTrack
         //outcome actions
         __Y = LoopPaused;
         pass = true;
      }
      else if( e == EnableDisturb && processCommand() ) { // EnableDisturb
         //outcome actions
         __Y = LoopPaused;
         pass = true;
      }
      else if( e == SaveOptLoopData && processCommand() ) { // SaveOptLoopData
         //outcome actions
         __Y = LoopPaused;
         pass = true;
      }
      else if( e == SaveStatus && processCommand() ) { // SaveStatus
         //outcome actions
         __Y = LoopPaused;
         pass = true;
      }
      else if( e == CalibrateIRTCDark && processCommand() ) { // CalibrateIRTCDark
         //outcome actions
         __Y = LoopPaused;
         pass = true;
      }
      else if( e == CalibrateHODark && processCommand() ) { // CalibrateHODark
         //outcome actions
         __Y = LoopPaused;
         pass = true;
      }
      break;
   case PowerOff:
      if( e == Operate && processCommand() ) { // Operate
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      break;
   case AOSet:
      if( e == CloseLoop && processCommand() ) { // CloseLoop
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      else if( e == ModifyAO && processCommand()
 ) { // ModifyAo
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == AcquireRef && processCommand()
 ) { // AcquireRef
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == Operate && processCommand() ) { // Operate
         //outcome actions
         __Y = Operating;
         pass = true;
      }
      else if( e == PrepareAcquireRef && processCommand() ) { // PrepareAcquireRef
         //outcome actions
         __Y = AOPrepared;
         pass = true;
      }
      else if( e == StopLoop && processCommand() ) { // StopLoop
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == AutoTrack && processCommand() ) { // AutoTrack
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == AntiDrift && processCommand() ) { // AntiDrift
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == EnableDisturb && processCommand() ) { // EnableDisturb
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == SaveOptLoopData && processCommand() ) { // SaveOptLoopData
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == SaveStatus && processCommand() ) { // SaveStatus
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == CalibrateHODark && processCommand() ) { // CalibrateHODark
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == CalibrateTVDark && processCommand() ) { // CalibrateTVDark
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == CheckRef && processCommand() ) { // CheckRef
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == Off && processCommand() ) { // Off
         //outcome actions
         __Y = PowerOff;
         pass = true;
      }
      else if( e == PrepareAdsec && processCommand() ) { // PrepareAdsec
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == StopLoopAdsec && processCommand() ) { // StopLoopAdsec
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == CalibrateSlopenull && processCommand() ) { // CalibrateSlopenull
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == CalibrateIRTCDark && processCommand() ) { // CalibrateIRTCDark
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == CalibratePISCESDark && processCommand() ) { // CalibratePISCESDark
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      break;
   case _Unknown:
      pass=false;
      break;
   }

   if( yOld == __Y && !pass ) {
      return;
   }

   switch( __Y ) { // income actions
   case Operating:
      break;
   case Failure:
      break;
   case LoopClosed:
      break;
   case AOPrepared:
      break;
   case LoopPaused:
      break;
   case PowerOff:
      break;
   case AOSet:
      break;
   case _Unknown:
      break;
   }
}

void WfsFsmCore::A( Event e )
{
   bool __empty = __events.empty();
   __events.push( e );
   if( __empty ) {
      while( !__events.empty() ) {
         __processEvent( __events.front() );
         __events.pop();
      }
   }
}

std::string WfsFsmCore::eventName( Event e )
{
   switch(e) {
   case AntiDrift:
      return "AntiDrift";
   case CalibratePISCESDark:
      return "CalibratePISCESDark";
   case ResumeLoop:
      return "ResumeLoop";
   case OffsetXY:
      return "OffsetXY";
   case OptimizeGain:
      return "OptimizeGain";
   case SaveStatus:
      return "SaveStatus";
   case RecoverPanic:
      return "RecoverPanic";
   case CalibrateIRTCDark:
      return "CalibrateIRTCDark";
   case OffsetZ:
      return "OffsetZ";
   case RecoverFailure:
      return "RecoverFailure";
   case CalibrateSlopenull:
      return "CalibrateSlopenull";
   case PrepareAdsec:
      return "PrepareAdsec";
   case StopLoopAdsec:
      return "StopLoopAdsec";
   case Off:
      return "Off";
   case SetSource:
      return "SetSource";
   case AutoTrack:
      return "AutoTrack";
   case SaveOptLoopData:
      return "SaveOptLoopData";
   case CalibrateTVDark:
      return "CalibrateTVDark";
   case ForceFailure:
      return "ForceFailure";
   case ModifyAO:
      return "ModifyAO";
   case Operate:
      return "Operate";
   case CheckRef:
      return "CheckRef";
   case PauseLoop:
      return "PauseLoop";
   case CorrectModes:
      return "CorrectModes";
   case StopLoop:
      return "StopLoop";
   case CloseLoop:
      return "CloseLoop";
   case GetTVSnap:
      return "GetTVSnap";
   case PrepareAcquireRef:
      return "PrepareAcquireRef";
   case EmergencyOff:
      return "EmergencyOff";
   case EnableDisturb:
      return "EnableDisturb";
   case CalibrateHODark:
      return "CalibrateHODark";
   case AcquireRef:
      return "AcquireRef";
   case ForcePanic:
      return "ForcePanic";
   case SetLoopFreq:
      return "SetLoopFreq";
   case RefineLoop:
      return "RefineLoop";
   default:
      return "Unknown event";
   }
}

std::string WfsFsmCore::stateName( States s )
{
   switch(s) {
   case Operating:
      return "Operating";
   case Failure:
      return "Failure";
   case LoopClosed:
      return "LoopClosed";
   case AOPrepared:
      return "AOPrepared";
   case LoopPaused:
      return "LoopPaused";
   case PowerOff:
      return "PowerOff";
   case AOSet:
      return "AOSet";
   default:
      return "Unknown state";
   }
}

WfsFsmCore::States WfsFsmCore::stateCode( std::string stateName )
{
   if(stateName.compare("Operating")==0)
      return Operating;
   if(stateName.compare("Failure")==0)
      return Failure;
   if(stateName.compare("LoopClosed")==0)
      return LoopClosed;
   if(stateName.compare("AOPrepared")==0)
      return AOPrepared;
   if(stateName.compare("LoopPaused")==0)
      return LoopPaused;
   if(stateName.compare("PowerOff")==0)
      return PowerOff;
   if(stateName.compare("AOSet")==0)
      return AOSet;
   return _Unknown;
}

void WfsFsmCore::forceState( States s )
{
    __Y = s;
}

