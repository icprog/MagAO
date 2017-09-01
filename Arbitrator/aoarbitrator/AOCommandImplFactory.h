#ifndef AOCOMMANDIMPLFACTORY_H_INCLUDE
#define AOCOMMANDIMPLFACTORY_H_INCLUDE

#include "framework/CommandImplFactory.h"

#include "aoarbitrator/AOCommandsImpl.h"

namespace Arcetri {
namespace Arbitrator {

class AOCommandImplFactory: public CommandImplFactory {

	public:

		AOCommandImplFactory(): CommandImplFactory() {}

		virtual ~AOCommandImplFactory() {}

		virtual CommandImpl* implementCommand(Command* cmd, map<string, AbstractSystem*> systems) {
			_logger->log(Logger::LOG_LEV_DEBUG, "AO CommandImplFactory retrieving command implementation...");
			switch(cmd->getCode()) {

				case AOOpCodes::POWER_ON_WFS:
					return new PowerOnWfsImpl((PowerOnWfs*)cmd, systems);

				case AOOpCodes::POWER_OFF_WFS:
					return new PowerOffWfsImpl((PowerOffWfs*)cmd, systems);

				case AOOpCodes::POWER_ON_ADSEC:
					return new PowerOnAdSecImpl((PowerOnAdSec*)cmd, systems);

				case AOOpCodes::POWER_OFF_ADSEC:
					return new PowerOffAdSecImpl((PowerOffAdSec*)cmd, systems);

				case AOOpCodes::PRESET_FLAT:
					return new PresetFlatImpl((PresetFlat*)cmd, systems);

				case AOOpCodes::PRESET_AO:
					return new PresetAOImpl((PresetAO*)cmd, systems);

				case AOOpCodes::ACQUIREREF_AO:
					return new AcquireRefAOImpl((AcquireRefAO*)cmd, systems);

				case AOOpCodes::CHECKREF_AO:
					return new CheckRefAOImpl((CheckRefAO*)cmd, systems);

				case AOOpCodes::MIRROR_SET:
					return new MirrorSetImpl((MirrorSet*)cmd, systems);

				case AOOpCodes::MIRROR_REST:
					return new MirrorRestImpl((MirrorRest*)cmd, systems);

				case AOOpCodes::FAULT_RECOVERY:
					return new FaultRecoveryImpl((FaultRecovery*)cmd, systems);

				case AOOpCodes::CORRECTMODES:
					return new CorrectModesImpl((CorrectModes*)cmd, systems);

				case AOOpCodes::SETZERNIKES:
					return new SetZernikesImpl((SetZernikes*)cmd, systems);

				case AOOpCodes::MODIFY_AO:
					return new ModifyAOImpl((ModifyAO*)cmd, systems);

				case AOOpCodes::OFFSETXY:
					return new OffsetXYImpl((OffsetXY*)cmd, systems);

				case AOOpCodes::OFFSETZ:
					return new OffsetZImpl((OffsetZ*)cmd, systems);

				case AOOpCodes::PAUSE:
					return new PauseImpl((Pause*)cmd, systems);

            case AOOpCodes::REFINE_AO:
					return new RefineAOImpl((RefineAO*)cmd, systems);

            case AOOpCodes::RESUME:
					return new ResumeImpl((Resume*)cmd, systems);

				case AOOpCodes::START_AO:
					return new StartAOImpl((StartAO*)cmd, systems);
/*
				case AOOpCodes::RECOVER_WFS_FAILURE:
					return new RecoverWfsFailureImpl((RecoverWfsFailure*)cmd, systems);

				case AOOpCodes::RECOVER_ADSEC_FAILURE:
					return new RecoverAdSecFailureImpl((RecoverAdSecFailure*)cmd, systems);
               */

				case AOOpCodes::STOP:
					return new StopImpl((Stop*)cmd, systems);

				case AOOpCodes::USER_PANIC:
					return new UserPanicImpl((UserPanic*)cmd, systems);

				default:
					return CommandImplFactory::implementCommand(cmd, systems);

			}
		}

};

}
}

#endif /*COMMANDIMPLFACTORY_H_INCLUDE*/
