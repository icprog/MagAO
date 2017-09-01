/* FSM implementation
 * Automatically generated for FSM: AOArbFsmCore by FSMC vers. 1.1.1 */

#include "AOArbFsmCore.h"

void AOArbFsmCore::__processEvent( Event e )
{
   States yOld = __Y;
   bool pass = false;
   switch( __Y ) { //transitions 
   case Failure:
      break;
   case Operational:
      if( e == PresetAO && processCommand() ) { // PresetAO
         //outcome actions
         __Y = ReadyToAcquire;
         pass = true;
      }
      else if( e == PresetFlat && processCommand() ) { // PresetFlat
         //outcome actions
         __Y = Operational;
         pass = true;
      }
      else if( e == SetZernikes && processCommand() ) { // SetZernikes
         //outcome actions
         __Y = Operational;
         pass = true;
      }
      else if( e == Stop && processCommand() ) { // Stop
         //outcome actions
         __Y = Operational;
         pass = true;
      }
      break;
   case RefAcquired:
      if( e == RefineAO && processCommand() ) { // RefineAO
         //outcome actions
         __Y = RefAcquired;
         pass = true;
      }
      else if( e == ModifyAO && processCommand() ) { // ModifyAO
         //outcome actions
         __Y = RefAcquired;
         pass = true;
      }
      else if( e == StartAO && processCommand() ) { // StartAO
         //outcome actions
         if( !__stateStack.empty() ) {
            __Y = __stateStack.top();
            __stateStack.pop();
         }
         else {
            __Y = LoopClosed;
         pass = true;
         }
      }
      else if( e == PresetAO && processCommand() ) { // PresetAO
         //outcome actions
         __Y = ReadyToAcquire;
         pass = true;
      }
      else if( e == OffsetZ && processCommand() ) { // OffsetZ
         pass = false;
      }
      else if( e == Stop && processCommand() ) { // Stop
         //outcome actions
         __Y = ReadyToAcquire;
         pass = true;
      }
      else if( e == AcquireRefAO && processCommand() ) { // AcquireRefAO
         //outcome actions
         __Y = RefAcquired;
         pass = true;
      }
      break;
   case LoopClosed:
      if( e == Pause && processCommand() ) { // Pause
         //outcome actions
         __Y = LoopSuspended;
         pass = true;
      }
      else if( e == Stop && processCommand() ) { // Stop
         //outcome actions
         __Y = RefAcquired;
         pass = true;
      }
      else if( e == OffsetZ && processCommand() ) { // OffsetZ
         pass = false;
      }
      else if( e == OffsetXY && processCommand() ) { // OffsetXY
         pass = false;
      }
      else if( e == CorrectModes && processCommand() ) { // CorrectModes
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      else if( e == AdjustGain && processCommand() ) { // AdjustGain
         pass = false;
      }
      break;
   case LoopSuspended:
      if( e == Resume && processCommand() ) { // Resume
         //outcome actions
         __Y = LoopClosed;
         pass = true;
      }
      else if( e == Stop && processCommand() ) { // Stop
         //outcome actions
         __Y = Operational;
         pass = true;
      }
      else if( e == OffsetZ && processCommand() ) { // OffsetZ
         //outcome actions
         __Y = LoopSuspended;
         pass = true;
      }
      else if( e == OffsetXY && processCommand() ) { // OffsetXY
         //outcome actions
         __Y = LoopSuspended;
         pass = true;
      }
      break;
   case ReadyToAcquire:
      if( e == CheckRefAO && processCommand() ) { // CheckRefAO
         //outcome actions
         __Y = ReadyToAcquire;
         pass = true;
      }
      else if( e == OffsetZ && processCommand() ) { // OffsetZ
         //outcome actions
         __Y = ReadyToAcquire;
         pass = true;
      }
      else if( e == AcquireRefAO && processCommand() ) { // AcquireRefAO
         //outcome actions
         __Y = RefAcquired;
         pass = true;
      }
      else if( e == StartAO && processCommand() ) { // StartAO
         //outcome actions
         if( !__stateStack.empty() ) {
            __Y = __stateStack.top();
            __stateStack.pop();
         }
         else {
            __Y = LoopClosed;
         pass = true;
         }
      }
      else if( e == Stop && processCommand() ) { // Stop
         //outcome actions
         __Y = Operational;
         pass = true;
      }
      else if( e == PresetFlat && processCommand() ) { // PresetFlat
         //outcome actions
         __Y = Operational;
         pass = true;
      }
      else if( e == PresetAO && processCommand() ) { // PresetAO
         //outcome actions
         __Y = ReadyToAcquire;
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
   case Failure:
      break;
   case Operational:
      break;
   case RefAcquired:
      break;
   case LoopClosed:
      break;
   case LoopSuspended:
      break;
   case ReadyToAcquire:
      break;
   case _Unknown:
      break;
   }
}

void AOArbFsmCore::A( Event e )
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

std::string AOArbFsmCore::eventName( Event e )
{
   switch(e) {
   case OffsetXY:
      return "OffsetXY";
   case SetZernikes:
      return "SetZernikes";
   case PowerOnWfs:
      return "PowerOnWfs";
   case Pause:
      return "Pause";
   case OffsetZ:
      return "OffsetZ";
   case StartAO:
      return "StartAO";
   case PowerOffWfs:
      return "PowerOffWfs";
   case AcquireRefAO:
      return "AcquireRefAO";
   case PowerOnAdSec:
      return "PowerOnAdSec";
   case SetSafeMode:
      return "SetSafeMode";
   case ModifyAO:
      return "ModifyAO";
   case CheckRefAO:
      return "CheckRefAO";
   case PowerOffAdSec:
      return "PowerOffAdSec";
   case PresetAO:
      return "PresetAO";
   case CorrectModes:
      return "CorrectModes";
   case RefineAO:
      return "RefineAO";
   case Resume:
      return "Resume";
   case PresetFlat:
      return "PresetFlat";
   case Stop:
      return "Stop";
   case AdjustGain:
      return "AdjustGain";
   case SetSeeingLmtd:
      return "SetSeeingLmtd";
   default:
      return "Unknown event";
   }
}

std::string AOArbFsmCore::stateName( States s )
{
   switch(s) {
   case Failure:
      return "Failure";
   case Operational:
      return "Operational";
   case RefAcquired:
      return "RefAcquired";
   case LoopClosed:
      return "LoopClosed";
   case LoopSuspended:
      return "LoopSuspended";
   case ReadyToAcquire:
      return "ReadyToAcquire";
   default:
      return "Unknown state";
   }
}

AOArbFsmCore::States AOArbFsmCore::stateCode( std::string stateName )
{
   if(stateName.compare("Failure")==0)
      return Failure;
   if(stateName.compare("Operational")==0)
      return Operational;
   if(stateName.compare("RefAcquired")==0)
      return RefAcquired;
   if(stateName.compare("LoopClosed")==0)
      return LoopClosed;
   if(stateName.compare("LoopSuspended")==0)
      return LoopSuspended;
   if(stateName.compare("ReadyToAcquire")==0)
      return ReadyToAcquire;
   return _Unknown;
}

void AOArbFsmCore::forceState( States s )
{
    __Y = s;
}

