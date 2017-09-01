#ifndef WFSCOMMANDSIMPL_H_INCLUDE
#define WFSCOMMANDSIMPL_H_INCLUDE

#include "wfsarbitrator/WfsCommandImpl.h"
#include "arblib/wfsArb/WfsCommands.h"
#include "arblib/base/Commands.h"

using namespace Arcetri::Arbitrator;


namespace Arcetri {
namespace Wfs_Arbitrator {


class OperateImpl: public WfsCommandImpl {

	public:

		OperateImpl(Operate* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		OperateImpl(RecoverFailure* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}


		virtual ~OperateImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};


class OffImpl: public WfsCommandImpl {

	public:

		OffImpl(Off* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		/*
		 * Off is actually used to implement a RecoverPanic
		 */
		OffImpl(RecoverPanic* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~OffImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};


class PrepareAcquireRefImpl: public WfsCommandImpl {

	public:

		PrepareAcquireRefImpl(PrepareAcquireRef* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~PrepareAcquireRefImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class AcquireRefImpl: public WfsCommandImpl {

	public:

		AcquireRefImpl(AcquireRef* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~AcquireRefImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};


class ModifyAOImpl: public WfsCommandImpl {

	public:

		ModifyAOImpl(ModifyAO* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~ModifyAOImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class EnableDisturbImpl: public WfsCommandImpl {

	public:

		EnableDisturbImpl(EnableDisturb* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~EnableDisturbImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};


class CloseLoopImpl: public WfsCommandImpl {

	public:

		CloseLoopImpl(CloseLoop* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~CloseLoopImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class PrepareAdsecImpl: public WfsCommandImpl {

	public:

		PrepareAdsecImpl(PrepareAdsec* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~PrepareAdsecImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};


class PauseLoopImpl: public WfsCommandImpl {

	public:

		PauseLoopImpl(PauseLoop* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~PauseLoopImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};


class RefineLoopImpl: public WfsCommandImpl {

	public:

		RefineLoopImpl(RefineLoop* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~RefineLoopImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};


class ResumeLoopImpl: public WfsCommandImpl {

	public:

		ResumeLoopImpl(ResumeLoop* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~ResumeLoopImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};


class StopLoopImpl: public WfsCommandImpl {

	public:

		StopLoopImpl(StopLoop* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~StopLoopImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class StopLoopAdsecImpl: public WfsCommandImpl {

	public:

		StopLoopAdsecImpl(StopLoopAdsec* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~StopLoopAdsecImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class SetSourceImpl: public WfsCommandImpl {

	public:

		SetSourceImpl(SetSource* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~SetSourceImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class GetTVSnapImpl: public WfsCommandImpl {

	public:

		GetTVSnapImpl(GetTVSnap* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~GetTVSnapImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class CheckRefImpl: public WfsCommandImpl {

	public:

		CheckRefImpl(CheckRef* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~CheckRefImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class CalibrateHODarkImpl: public WfsCommandImpl {

	public:

		CalibrateHODarkImpl(CalibrateHODark* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~CalibrateHODarkImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class CalibrateTVDarkImpl: public WfsCommandImpl {

	public:

		CalibrateTVDarkImpl(CalibrateTVDark* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~CalibrateTVDarkImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class CalibrateIRTCDarkImpl: public WfsCommandImpl {

	public:

		CalibrateIRTCDarkImpl(CalibrateIRTCDark* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~CalibrateIRTCDarkImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class CalibratePISCESDarkImpl: public WfsCommandImpl {

	public:

		CalibratePISCESDarkImpl(CalibratePISCESDark* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~CalibratePISCESDarkImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class CalibrateMovementsImpl: public WfsCommandImpl {

	public:

		CalibrateMovementsImpl(CalibrateMovements* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~CalibrateMovementsImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class CalibrateSlopenullImpl: public WfsCommandImpl {

	public:

		CalibrateSlopenullImpl(CalibrateSlopenull* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~CalibrateSlopenullImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class SaveStatusImpl: public WfsCommandImpl {

	public:

		SaveStatusImpl(SaveStatus* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~SaveStatusImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class SaveOptLoopDataImpl: public WfsCommandImpl {

	public:

		SaveOptLoopDataImpl(SaveOptLoopData* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}
		virtual ~SaveOptLoopDataImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);
      bool cancelImpl() throw(CommandTerminationException);

};

class OffsetXYImpl: public WfsCommandImpl {

	public:

		OffsetXYImpl(OffsetXY* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~OffsetXYImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class OffsetZImpl: public WfsCommandImpl {

	public:

		OffsetZImpl(OffsetZ* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~OffsetZImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class CorrectModesImpl: public WfsCommandImpl {

	public:

		CorrectModesImpl(CorrectModes* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~CorrectModesImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class AntiDriftImpl: public WfsCommandImpl {

	public:

		AntiDriftImpl(AntiDrift* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~AntiDriftImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class AutoTrackImpl: public WfsCommandImpl {

	public:

		AutoTrackImpl(AutoTrack* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~AutoTrackImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class OptimizeGainImpl: public WfsCommandImpl {

	public:

		OptimizeGainImpl(OptimizeGain* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~OptimizeGainImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};

class EmergencyOffImpl: public WfsCommandImpl {

	public:

		EmergencyOffImpl(EmergencyOff* command, map<string, AbstractSystem*> systems):WfsCommandImpl(command, systems) {}

		virtual ~EmergencyOffImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

};


}
}

#endif /*WFSCOMMANDSIMPL_H_INCLUDE*/
