/*
 * This class implements the state machine of the AO Arbitrator
 * using as core FSM the code generated automatically by FSMC
 * based on the FSM description in AOArbFsmCore.fsm
 *
 */

#include "AOArbFsm.h"
#include "arblib/aoArb/AOCommands.h"
#include "AOCommandsImpl.h"

using namespace Arcetri::Arbitrator;

AOArbFsm::AOArbFsm(unsigned int commandHistoryMaxSize, string ident): AbstractFsm(commandHistoryMaxSize, ident) 
{ 
// This defines the event map table. When new events are added to the FSM,
// They must be added to the table below.

  EvtTable[(OpCode)AOOpCodes::ACQUIREREF_AO]=AcquireRefAO;
  EvtTable[(OpCode)AOOpCodes::CHECKREF_AO]=CheckRefAO;
  EvtTable[(OpCode)AOOpCodes::CORRECTMODES]=CorrectModes;
  EvtTable[(OpCode)AOOpCodes::MODIFY_AO]=ModifyAO;
  EvtTable[(OpCode)AOOpCodes::OFFSETXY]=OffsetXY;
  EvtTable[(OpCode)AOOpCodes::OFFSETZ]=OffsetZ;
  EvtTable[(OpCode)AOOpCodes::PAUSE]=Pause;
  EvtTable[(OpCode)AOOpCodes::PRESET_AO]=PresetAO;
  EvtTable[(OpCode)AOOpCodes::PRESET_FLAT]=PresetFlat;
  EvtTable[(OpCode)AOOpCodes::REFINE_AO]=RefineAO;
  EvtTable[(OpCode)AOOpCodes::RESUME]=Resume;
  EvtTable[(OpCode)AOOpCodes::START_AO]=StartAO;
  EvtTable[(OpCode)AOOpCodes::STOP]=Stop;
  EvtTable[(OpCode)AOOpCodes::SETZERNIKES]=SetZernikes;

  forceState(Operational);
}

// Implementation of pure virtual base class methods
void AOArbFsm::trigger(OpCode code) 
{
   if ((code == AOOpCodes::POWER_ON_WFS) ||
       (code == AOOpCodes::POWER_OFF_WFS) ||
       (code == AOOpCodes::POWER_ON_ADSEC) ||
       (code == AOOpCodes::POWER_OFF_ADSEC) ||
       (code == AOOpCodes::MIRROR_SET) ||
       (code == AOOpCodes::MIRROR_REST)) {

      processCommand();
      return;
   }

        if(EvtTable.count(code)> 0) {
	    Event e = EvtTable[code];
	    A(e);
        } else {
            throw ArbitratorException("Event table inconsistency",ARB_GENERIC_ERROR);
        }
}
void AOArbFsm::forceFailure() 
{
   AOArbitrator *aoarb = (AOArbitrator*)AbstractArbitrator::getInstance();

   try {
      string curState = aoarb->fsmState();
      if ((curState == "LoopClosed") || (curState == "LoopSuspended")) {
         _logger->log(Logger::LOG_LEV_INFO, "forceFailure() opening loop...");
         stopParams params;
         Arcetri::Arbitrator::Stop stop(10*1000, params);
         Arcetri::Arbitrator::StopImpl stopimpl( &stop, aoarb->getSystems());
         stopimpl.executeImpl();
      }
   } catch (AOException &e) {
      _logger->log(Logger::LOG_LEV_ERROR, "forceFailure() cannot close loop!! Exception: %s", e.what().c_str());
   }

	forceState(Operational);
}

void AOArbFsm::forcePanic() 
{
	trigger(BasicOpCodes::FORCE_PANIC);
}

string AOArbFsm::currentStateAsString() 
{ 
	return string(stateName(currentState())); 
}


// the workaround below is needed to avoid an error due to
// the const method definition
bool AOArbFsm::processCommand() const
{ 
bool ret;

AOArbFsm *pt=const_cast<AOArbFsm *>(this);
	ret =  pt->AbstractFsm::processCommand(); 

   // Special case for PresetAO: go back in SeeingLimited in case the command did not execute
   if (_currentCommandImpl->getCode() == AOOpCodes::PRESET_AO && _currentCommandImpl->getStatus() == Command::CMD_EXECUTION_RETRY) 
      pt->forceState(Operational);

   if(ret)
      _logger->log(Logger::LOG_LEV_DEBUG,"processCommand() retured TRUE (success)");
   else
       _logger->log(Logger::LOG_LEV_DEBUG,"processCommand() retured FALSE (failure)");
   return ret;
}

