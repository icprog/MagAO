#ifndef AOCOMMANDSIMPL_H_INCLUDE
#define AOCOMMANDSIMPL_H_INCLUDE

#include "AOCommandImpl.h"
#include "arblib/aoArb/AOCommands.h"
#include "arblib/base/Commands.h"


namespace Arcetri {
namespace Arbitrator {

class AdjustGainImpl: public AOCommandImpl {

	public:
		AdjustGainImpl(AdjustGain* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) { }
		virtual ~AdjustGainImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);
};

class AdjustIntTimeImpl: public AOCommandImpl {

	public:
		AdjustIntTimeImpl(AdjustIntTime* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) { }
		virtual ~AdjustIntTimeImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);
};



class PowerOnWfsImpl: public AOCommandImpl {

	public:
		PowerOnWfsImpl(PowerOnWfs* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) { }
		virtual ~PowerOnWfsImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};


class PowerOffWfsImpl: public AOCommandImpl {

	public:
		PowerOffWfsImpl(PowerOffWfs* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) { }
		virtual ~PowerOffWfsImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);
};


class PowerOnAdSecImpl: public AOCommandImpl {

	public:
		PowerOnAdSecImpl(PowerOnAdSec* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) { }
		virtual ~PowerOnAdSecImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};


class PowerOffAdSecImpl: public AOCommandImpl {

	public:
		PowerOffAdSecImpl(PowerOffAdSec* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) { }
		virtual ~PowerOffAdSecImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);
};


class MirrorRestImpl: public AOCommandImpl {

	public:
		MirrorRestImpl(MirrorRest* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) { }
		virtual ~MirrorRestImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};


class MirrorSetImpl: public AOCommandImpl {
	public:
		MirrorSetImpl(MirrorSet* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) { }
		virtual ~MirrorSetImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);
};


class FaultRecoveryImpl: public AOCommandImpl {
	public:
		FaultRecoveryImpl(FaultRecovery* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) { }
		virtual ~FaultRecoveryImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);
};


class StandaloneFailImpl: public AOCommandImpl {
	public:
		StandaloneFailImpl(StandaloneFail* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) { }
		virtual ~StandaloneFailImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);
};


class SeeingLmtdFailImpl: public AOCommandImpl {
	public:
		SeeingLmtdFailImpl(SeeingLmtdFail* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) { }
		virtual ~SeeingLmtdFailImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);
};



class PresetAOImpl: public AOCommandImpl {

	public:
		PresetAOImpl(PresetAO* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~PresetAOImpl() {}

   private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};


class PresetFlatImpl: public AOCommandImpl {

	public:
		PresetFlatImpl(PresetFlat* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~PresetFlatImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};


class AcquireRefAOImpl: public AOCommandImpl {

	public:
		AcquireRefAOImpl(AcquireRefAO* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~AcquireRefAOImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw (Arcetri::Arbitrator::CommandExecutionException, Arcetri::Arbitrator::CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

      void startSeeingLimited();
      void stopSeeingLimited();


};

class CheckRefAOImpl: public AOCommandImpl {

	public:
		CheckRefAOImpl(CheckRefAO* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~CheckRefAOImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};

class CorrectModesImpl: public AOCommandImpl {

	public:
		CorrectModesImpl(CorrectModes* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~CorrectModesImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};

class SetZernikesImpl: public AOCommandImpl {

	public:
		SetZernikesImpl(SetZernikes* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~SetZernikesImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};


class ModifyAOImpl: public AOCommandImpl {

	public:
		ModifyAOImpl(ModifyAO* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~ModifyAOImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};

class OffsetXYImpl: public AOCommandImpl {

	public:
		OffsetXYImpl(OffsetXY* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) { _skip=false; }
		virtual ~OffsetXYImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);
        bool _skip;

};

class OffsetZImpl: public AOCommandImpl {

	public:
		OffsetZImpl(OffsetZ* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~OffsetZImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};

class PauseImpl: public AOCommandImpl {

	public:
		PauseImpl(Pause* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~PauseImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};

class RefineAOImpl: public AOCommandImpl {

	public:
		RefineAOImpl(RefineAO* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~RefineAOImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};

class ResumeImpl: public AOCommandImpl {

	public:
		ResumeImpl(Resume* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~ResumeImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};

class StartAOImpl: public AOCommandImpl {

	public:
		StartAOImpl(StartAO* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~StartAOImpl() {}
        bool checkStop();

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

      void stopSeeingLimited();
};

/*
class RecoverWfsFailureImpl: public AOCommandImpl {

	public:
		RecoverWfsFailureImpl(RecoverWfsFailure* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~RecoverWfsFailureImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};

class RecoverAdSecImpl: public AOCommandImpl {

	public:
		RecoverAdSecImpl(RecoverAdSec* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~RecoverAdSecImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};
*/

class StopImpl: public AOCommandImpl {

	public:
		StopImpl(Stop* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~StopImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};

class UserPanicImpl: public AOCommandImpl {

	public:
		UserPanicImpl(UserPanic* command, map<string, AbstractSystem*> wfsAndAdSec): AOCommandImpl(command, wfsAndAdSec) {}
		virtual ~UserPanicImpl() {}

	private:
		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException, Arcetri::Arbitrator::ArbitratorInterfaceException);

};


}
}

#endif /*AOCOMMANDSIMPL_H_INCLUDE*/
