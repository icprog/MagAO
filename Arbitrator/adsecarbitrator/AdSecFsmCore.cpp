/* FSM implementation
 * Automatically generated for FSM: AdSecFsmCore by FSMC vers. 1.1.1 */

#include "AdSecFsmCore.h"

void AdSecFsmCore::__processEvent( Event e )
{
   States yOld = __Y;
   bool pass = false;
   switch( __Y ) { //transitions 
   case Failure:
      if( e == RecoverFailure && processCommand() ) { // RecoverFailure
         //outcome actions
         __Y = Ready;
         pass = true;
      }
      else if( e == ForcePanic ) { // <ForcePanic>
         //outcome actions
         __Y = Panic;
         pass = true;
      }
      else if( e == Rest && processCommand() ) { // Rest
         //outcome actions
         __Y = Ready;
         pass = true;
      }
      break;
   case AOPause:
      if( e == ResumeAo && processCommand() ) { // ResumeAo
         //outcome actions
         __Y = AORunning;
         pass = true;
      }
      else if( e == StopAo && processCommand() ) { // StopAo
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == LoadShape && processCommand() ) { // LoadShape
         pass = false;
      }
      else if( e == SaveShape && processCommand() ) { // SaveShape
         pass = false;
      }
      else if( e == SetDisturb && processCommand() ) { // SetDisturb
         pass = false;
      }
      else if( e == SetGain && processCommand() ) { // SetGain
         pass = false;
      }
      else if( e == Rest && processCommand() ) { // Rest
         //outcome actions
         __Y = Ready;
         pass = true;
      }
      else if( e == ForceFailure ) { // <ForceFailure>
         //outcome actions
         __Y = Failure;
         pass = true;
      }
      break;
   case Ready:
      if( e == Reset  && processCommand() ) { // Reset
         //outcome actions
         __Y = PowerOn;
         pass = true;
      }
      else if( e == Calibrate && processCommand() ) { // Calibrate
         pass = false;
      }
      else if( e == SetFlatChop && processCommand() ) { // SetFlatChop
         //outcome actions
         __Y = ChopSet;
         pass = true;
      }
      else if( e == SetFlatAo  && processCommand() ) { // SetFlatAo
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == ForceFailure ) { // <ForceFailure>
         //outcome actions
         __Y = Failure;
         pass = true;
      }
      else if( e == Off && processCommand()
 ) { // Off
         //outcome actions
         __Y = PowerOff;
         pass = true;
      }
      break;
   case ChopRunning:
      if( e == StopChop && processCommand() ) { // StopChop
         //outcome actions
         __Y = ChopSet;
         pass = true;
      }
      else if( e == ForceFailure ) { // <ForceFailure>
         //outcome actions
         __Y = Failure;
         pass = true;
      }
      else if( e == Rest && processCommand() ) { // Rest
         //outcome actions
         __Y = Ready;
         pass = true;
      }
      break;
   case AORunning:
      if( e == StopAo  && processCommand() ) { // StopAo
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == SetGain && processCommand() ) { // SetGain
         pass = false;
      }
      else if( e == ForceFailure ) { // <ForceFailure>
         //outcome actions
         __Y = Failure;
         pass = true;
      }
      else if( e == PauseAo && processCommand() ) { // PauseAo
         //outcome actions
         __Y = AOPause;
         pass = true;
      }
      else if( e == TTOffLoad && processCommand() ) { // TTOffLoad
         pass = false;
      }
      else if( e == SetDataDecimation && processCommand() ) { // SetDataDecimation
         pass = false;
      }
      else if( e == SaveStatus && processCommand() ) { // SaveStatus
         pass = false;
      }
      else if( e == SaveShape && processCommand() ) { // SaveShape
         pass = false;
      }
      else if( e == Rest && processCommand() ) { // Rest
         //outcome actions
         __Y = Ready;
         pass = true;
      }
      break;
   case PowerOff:
      if( e == On  && processCommand() ) { // On
         //outcome actions
         __Y = PowerOn;
         pass = true;
      }
      break;
   case ChopSet:
      if( e == RunChop  && processCommand() ) { // RunChop
         //outcome actions
         __Y = ChopRunning;
         pass = true;
      }
      else if( e == Rest && processCommand() ) { // Rest
         //outcome actions
         __Y = Ready;
         pass = true;
      }
      else if( e == ApplyCommands  && processCommand() ) { // ApplyCommands
         pass = false;
      }
      else if( e == Reset  && processCommand() ) { // Reset
         pass = false;
      }
      else if( e == ChopToAo && processCommand() ) { // ChopToAo
         //outcome actions
         __Y = AOSet;
         pass = true;
      }
      else if( e == ForceFailure ) { // <ForceFailure>
         //outcome actions
         __Y = Failure;
         pass = true;
      }
      break;
   case PowerOn:
      if( e == LoadProgram && processCommand() ) { // LoadProgram
         //outcome actions
         __Y = Ready;
         pass = true;
      }
      else if( e == Off && processCommand() ) { // Off
         //outcome actions
         __Y = PowerOff;
         pass = true;
      }
      break;
   case Panic:
      if( e == RecoverPanic ) { // RecoverPanic
         //outcome actions
         __Y = PowerOff;
         pass = true;
      }
      break;
   case AOSet:
      if( e == Rest && processCommand() ) { // Rest
         //outcome actions
         __Y = Ready;
         pass = true;
      }
      else if( e == RunAo  && processCommand() ) { // RunAo
         //outcome actions
         __Y = AORunning;
         pass = true;
      }
      else if( e == ApplyCommands && processCommand() ) { // ApplyCommands
         pass = false;
      }
      else if( e == AoToChop && processCommand() ) { // AoToChop
         //outcome actions
         __Y = ChopSet;
         pass = true;
      }
      else if( e == ForceFailure ) { // <ForceFailure>
         //outcome actions
         __Y = Failure;
         pass = true;
      }
      else if( e == LoadShape && processCommand() ) { // LoadShape
         pass = false;
      }
      else if( e == SaveShape && processCommand() ) { // SaveShape
         pass = false;
      }
      else if( e == SetDataDecimation && processCommand() ) { // SetDataDecimation
         pass = false;
      }
      else if( e == SaveSlopes && processCommand() ) { // SaveSlopes
         pass = false;
      }
      else if( e == SaveStatus && processCommand() ) { // SaveStatus
         pass = false;
      }
      else if( e == SetRecMat && processCommand() ) { // SetRecMat
         pass = false;
      }
      else if( e == SetGain && processCommand() ) { // SetGain
         pass = false;
      }
      else if( e == SetDisturb && processCommand() ) { // SetDisturb
         pass = false;
      }
      else if( e == RecoverSkipFrame && processCommand() ) { // RecoverSkipFrame
         pass = false;
      }
      else if( e == Reset && processCommand() ) { // Reset
         pass = false;
      }
      else if( e == SetZernikes && processCommand() ) { // SetZernikes
         pass = false;
      }
      else if( e == SelectFocalStation && processCommand() ) { // SelectFocalStation
         pass = false;
      }
      else if( e == Calibrate && processCommand() ) { // Calibrate
         pass = false;
      }
      else if( e == StopAo  && processCommand() ) { // StopAo
         pass = false;
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
   case Failure:
      break;
   case AOPause:
      break;
   case Ready:
      break;
   case ChopRunning:
      break;
   case AORunning:
      break;
   case PowerOff:
      break;
   case ChopSet:
      break;
   case PowerOn:
      break;
   case Panic:
      break;
   case AOSet:
      break;
   case _Unknown:
      break;
   }
}

void AdSecFsmCore::A( Event e )
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

std::string AdSecFsmCore::eventName( Event e )
{
   switch(e) {
   case SetFlatAo:
      return "SetFlatAo";
   case SelectFocalStation:
      return "SelectFocalStation";
   case ResumeAo:
      return "ResumeAo";
   case Reset:
      return "Reset";
   case SetZernikes:
      return "SetZernikes";
   case LoadShape:
      return "LoadShape";
   case SaveStatus:
      return "SaveStatus";
   case LoadProgram:
      return "LoadProgram";
   case RecoverPanic:
      return "RecoverPanic";
   case TTOffLoad:
      return "TTOffLoad";
   case SetRecMat:
      return "SetRecMat";
   case AoToChop:
      return "AoToChop";
   case RecoverFailure:
      return "RecoverFailure";
   case RecoverSkipFrame:
      return "RecoverSkipFrame";
   case ApplyCommands:
      return "ApplyCommands";
   case SetGain:
      return "SetGain";
   case Off:
      return "Off";
   case SetFlatChop:
      return "SetFlatChop";
   case ForceFailure:
      return "ForceFailure";
   case RunAo:
      return "RunAo";
   case PauseAo:
      return "PauseAo";
   case On:
      return "On";
   case Calibrate:
      return "Calibrate";
   case SaveSlopes:
      return "SaveSlopes";
   case SetDisturb:
      return "SetDisturb";
   case SaveShape:
      return "SaveShape";
   case Rest:
      return "Rest";
   case SetDataDecimation:
      return "SetDataDecimation";
   case ForcePanic:
      return "ForcePanic";
   case ChopToAo:
      return "ChopToAo";
   case RunChop:
      return "RunChop";
   case StopAo:
      return "StopAo";
   case StopChop:
      return "StopChop";
   default:
      return "Unknown event";
   }
}

std::string AdSecFsmCore::stateName( States s )
{
   switch(s) {
   case Failure:
      return "Failure";
   case AOPause:
      return "AOPause";
   case Ready:
      return "Ready";
   case ChopRunning:
      return "ChopRunning";
   case AORunning:
      return "AORunning";
   case PowerOff:
      return "PowerOff";
   case ChopSet:
      return "ChopSet";
   case PowerOn:
      return "PowerOn";
   case Panic:
      return "Panic";
   case AOSet:
      return "AOSet";
   default:
      return "Unknown state";
   }
}

AdSecFsmCore::States AdSecFsmCore::stateCode( std::string stateName )
{
   if(stateName.compare("Failure")==0)
      return Failure;
   if(stateName.compare("AOPause")==0)
      return AOPause;
   if(stateName.compare("Ready")==0)
      return Ready;
   if(stateName.compare("ChopRunning")==0)
      return ChopRunning;
   if(stateName.compare("AORunning")==0)
      return AORunning;
   if(stateName.compare("PowerOff")==0)
      return PowerOff;
   if(stateName.compare("ChopSet")==0)
      return ChopSet;
   if(stateName.compare("PowerOn")==0)
      return PowerOn;
   if(stateName.compare("Panic")==0)
      return Panic;
   if(stateName.compare("AOSet")==0)
      return AOSet;
   return _Unknown;
}

void AdSecFsmCore::forceState( States s )
{
    __Y = s;
}

