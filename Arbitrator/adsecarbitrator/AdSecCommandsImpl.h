#ifndef ADSECCOMMANDSIMPL_H_INCLUDE
#define ADSECCOMMANDSIMPL_H_INCLUDE

#include "adsecarbitrator/AdSecCommandImpl.h"
#include "arblib/adSecArb/AdSecCommands.h"
#include "arblib/base/Commands.h"

using namespace Arcetri::Arbitrator;


namespace Arcetri {
namespace AdSec_Arbitrator {

class AoToChopImpl: public AdSecCommandImpl {

	public:

		AoToChopImpl(AoToChop* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~AoToChopImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

/* L.F removed
class AoToSlImpl: public AdSecCommandImpl {

	public:

		AoToSlImpl(AoToSl* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~AoToSlImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};
  end of remove */

class ApplyCommandsImpl: public AdSecCommandImpl {

	public:

		ApplyCommandsImpl(ApplyCommands* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~ApplyCommandsImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class RecoverSkipFrameImpl: public AdSecCommandImpl {

	public:

		RecoverSkipFrameImpl(RecoverSkipFrame* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~RecoverSkipFrameImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class ChopToAoImpl: public AdSecCommandImpl {

	public:

		ChopToAoImpl(ChopToAo* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~ChopToAoImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


/*  L.F removed
class ChopToSlImpl: public AdSecCommandImpl {

	public:

		ChopToSlImpl(ChopToSl* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~ChopToSlImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};
  end of remove */


class LoadProgramImpl: public AdSecCommandImpl {

	public:

		LoadProgramImpl(LoadProgram* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~LoadProgramImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class OffImpl: public AdSecCommandImpl {

	public:

		OffImpl(Off* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		/*
		 * Off is actually used to implement a RecoverPanic
		 */
		OffImpl(RecoverPanic* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~OffImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class OnImpl: public AdSecCommandImpl {

	public:

		OnImpl(On* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~OnImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class RecoverFailureImpl: public AdSecCommandImpl {

	public:

		RecoverFailureImpl(RecoverFailure* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~RecoverFailureImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class ResetImpl: public AdSecCommandImpl {

	public:

		ResetImpl(Reset* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~ResetImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class RestImpl: public AdSecCommandImpl {

	public:

		RestImpl(Rest* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~RestImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class RunAoImpl: public AdSecCommandImpl {

	public:

		RunAoImpl(RunAo* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~RunAoImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class SetZernikesImpl: public AdSecCommandImpl {

	public:

		SetZernikesImpl(SetZernikes* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SetZernikesImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class PauseAoImpl: public AdSecCommandImpl {

	public:

		PauseAoImpl(PauseAo* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~PauseAoImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class ResumeAoImpl: public AdSecCommandImpl {

	public:

		ResumeAoImpl(ResumeAo* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~ResumeAoImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class SetDataDecimationImpl: public AdSecCommandImpl {

	public:

		SetDataDecimationImpl(SetDataDecimation* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SetDataDecimationImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class SelectFocalStationImpl: public AdSecCommandImpl {

	public:

		SelectFocalStationImpl(SelectFocalStation* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SelectFocalStationImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class RunChopImpl: public AdSecCommandImpl {

	public:

		RunChopImpl(RunChop* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~RunChopImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class SetFlatAoImpl: public AdSecCommandImpl {

	public:

		SetFlatAoImpl(SetFlatAo* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SetFlatAoImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class SetFlatChopImpl: public AdSecCommandImpl {

	public:

		SetFlatChopImpl(SetFlatChop* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SetFlatChopImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


/*  L.F removed
class SetFlatSlImpl: public AdSecCommandImpl {

	public:

		SetFlatSlImpl(SetFlatSl* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SetFlatSlImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};
  end of remove */


class SetGainImpl: public AdSecCommandImpl {

	public:

		SetGainImpl(SetGain* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SetGainImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class SaveSlopesImpl: public AdSecCommandImpl {

	public:

		SaveSlopesImpl(SaveSlopes* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SaveSlopesImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class SaveStatusImpl: public AdSecCommandImpl {

	public:

		SaveStatusImpl(SaveStatus* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SaveStatusImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class SetDisturbImpl: public AdSecCommandImpl {

	public:

		SetDisturbImpl(SetDisturb* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SetDisturbImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class SetRecMatImpl: public AdSecCommandImpl {

	public:

		SetRecMatImpl(SetRecMat* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SetRecMatImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class TTOffloadImpl: public AdSecCommandImpl {

	public:

		TTOffloadImpl(TTOffload* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~TTOffloadImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class LoadShapeImpl: public AdSecCommandImpl {

	public:

		LoadShapeImpl(LoadShape* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~LoadShapeImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class SaveShapeImpl: public AdSecCommandImpl {

	public:

		SaveShapeImpl(SaveShape* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SaveShapeImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


/*  L.F removed
class SlToAoImpl: public AdSecCommandImpl {

	public:

		SlToAoImpl(SlToAo* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SlToAoImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class SlToChopImpl: public AdSecCommandImpl {

	public:

		SlToChopImpl(SlToChop* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SlToChopImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};
  end of remove */


class StopAoImpl: public AdSecCommandImpl {

	public:

		StopAoImpl(StopAo* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~StopAoImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};


class StopChopImpl: public AdSecCommandImpl {

	public:

		StopChopImpl(StopChop* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~StopChopImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

/*  L.F removed
class SetFlatFFImpl: public AdSecCommandImpl {

	public:

		SetFlatFFImpl(SetFlatFF* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~SetFlatFFImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class IfAcquireImpl: public AdSecCommandImpl {

	public:

		IfAcquireImpl(IfAcquire* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~IfAcquireImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class IfReduceImpl: public AdSecCommandImpl {

	public:

		IfReduceImpl(IfReduce* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~IfReduceImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class DustTestImpl: public AdSecCommandImpl {

	public:

		DustTestImpl(DustTest* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~DustTestImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class CoilTestImpl: public AdSecCommandImpl {

	public:

		CoilTestImpl(CoilTest* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~CoilTestImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class CapsensTestImpl: public AdSecCommandImpl {

	public:

		CapsensTestImpl(CapsensTest* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~CapsensTestImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class FFAcquireImpl: public AdSecCommandImpl {

	public:

		FFAcquireImpl(FFAcquire* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~FFAcquireImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class FFReduceImpl: public AdSecCommandImpl {

	public:

		FFReduceImpl(FFReduce* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~FFReduceImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class WdTestImpl: public AdSecCommandImpl {

	public:

		WdTestImpl(WdTest* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~WdTestImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class FlTestImpl: public AdSecCommandImpl {

	public:

		FlTestImpl(FlTest* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~FlTestImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};

class PsTestImpl: public AdSecCommandImpl {

	public:

		PsTestImpl(PsTest* command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}

		virtual ~PsTestImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);

		bool undoImpl() throw(CommandUndoingException);

};
  end of remove */

    class CalibrateImpl: public AdSecCommandImpl {
      public:
        CalibrateImpl(CalibrateCmd * command, map<string, AbstractSystem*> systems): AdSecCommandImpl(command, systems) {}
		virtual ~CalibrateImpl() {}

	private:

		bool validateImpl() throw(CommandValidationException);
		AbstractSystemCommandResult doExecuteImpl() throw(CommandExecutionException, CommandCanceledException);
		bool undoImpl() throw(CommandUndoingException);
};

}
}

#endif /*ADSECCOMMANDSIMPL_H_INCLUDE*/
