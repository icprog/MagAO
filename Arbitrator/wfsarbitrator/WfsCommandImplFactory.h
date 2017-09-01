#ifndef WFSCOMMANDIMPLFACTORY_H_INCLUDE
#define WFSCOMMANDIMPLFACTORY_H_INCLUDE

#include "framework/CommandImplFactory.h"

#include "wfsarbitrator/WfsCommandsImpl.h"


using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace Wfs_Arbitrator {

class WfsCommandImplFactory: public CommandImplFactory {

	public:

		WfsCommandImplFactory(): CommandImplFactory() {}

		virtual ~WfsCommandImplFactory() {}

		virtual CommandImpl* implementCommand(Command* cmd, map<string, AbstractSystem*> systems) {
			_logger->log(Logger::LOG_LEV_DEBUG, "Wfs CommandImplFactory retrieving command implementation...");
			switch(cmd->getCode()) {

				case WfsOpCodes::OPERATE_CMD:
					return new OperateImpl((Operate*)cmd, systems);

				case WfsOpCodes::OFF_CMD:
					return new OffImpl((Off*)cmd, systems);

				case WfsOpCodes::PREPARE_ACQUIRE_REF_CMD:
					return new PrepareAcquireRefImpl((PrepareAcquireRef*)cmd, systems);

				case WfsOpCodes::ACQUIRE_REF_CMD:
					return new AcquireRefImpl((AcquireRef*)cmd, systems);

				case WfsOpCodes::MODIFY_AO_CMD:
					return new ModifyAOImpl((ModifyAO*)cmd, systems);

				case WfsOpCodes::CLOSE_LOOP_CMD:
					return new CloseLoopImpl((CloseLoop*)cmd, systems);

				case WfsOpCodes::PREPARE_ADSEC_CMD:
					return new PrepareAdsecImpl((PrepareAdsec*)cmd, systems);

				case WfsOpCodes::REFINE_LOOP_CMD:
					return new RefineLoopImpl((RefineLoop*)cmd, systems);

				case WfsOpCodes::PAUSE_LOOP_CMD:
					return new PauseLoopImpl((PauseLoop*)cmd, systems);

				case WfsOpCodes::RESUME_LOOP_CMD:
					return new ResumeLoopImpl((ResumeLoop*)cmd, systems);

				case WfsOpCodes::STOP_LOOP_CMD:
					return new StopLoopImpl((StopLoop*)cmd, systems);

				case WfsOpCodes::STOPLOOP_ADSEC_CMD:
					return new StopLoopAdsecImpl((StopLoopAdsec*)cmd, systems);

				case WfsOpCodes::CALIBRATE_HODARK_CMD:
					return new CalibrateHODarkImpl((CalibrateHODark*)cmd, systems);

				case WfsOpCodes::CALIBRATE_TVDARK_CMD:
					return new CalibrateTVDarkImpl((CalibrateTVDark*)cmd, systems);

				case WfsOpCodes::CALIBRATE_IRTCDARK_CMD:
					return new CalibrateIRTCDarkImpl((CalibrateIRTCDark*)cmd, systems);

				case WfsOpCodes::CALIBRATE_PISCESDARK_CMD:
					return new CalibratePISCESDarkImpl((CalibratePISCESDark*)cmd, systems);

            case WfsOpCodes::CALIBRATE_MOVEMENTS_CMD:
					return new CalibrateMovementsImpl((CalibrateMovements*)cmd, systems);

            case WfsOpCodes::CALIBRATE_SLOPENULL_CMD:
					return new CalibrateSlopenullImpl((CalibrateSlopenull*)cmd, systems);

            case WfsOpCodes::SAVE_STATUS_CMD:
					return new SaveStatusImpl((SaveStatus*)cmd, systems);

            case WfsOpCodes::GET_TV_SNAP_CMD:
					return new GetTVSnapImpl((GetTVSnap*)cmd, systems);

            case WfsOpCodes::SAVE_OPTLOOPDATA_CMD:
					return new SaveOptLoopDataImpl((SaveOptLoopData*)cmd, systems);

            case WfsOpCodes::ENABLE_DISTURB_CMD:
					return new EnableDisturbImpl((EnableDisturb*)cmd, systems);

            case WfsOpCodes::OFFSET_XY:
					return new OffsetXYImpl((OffsetXY*)cmd, systems);

            case WfsOpCodes::OFFSET_Z:
					return new OffsetZImpl((OffsetZ*)cmd, systems);

            case WfsOpCodes::CORRECT_MODES:
					return new CorrectModesImpl((CorrectModes*)cmd, systems);

            case WfsOpCodes::CHECK_REF_CMD:
					return new CheckRefImpl((CheckRef*)cmd, systems);

            case WfsOpCodes::ANTIDRIFT_CMD:
					return new AntiDriftImpl((AntiDrift*)cmd, systems);

            case WfsOpCodes::OPTIMIZE_GAIN_CMD:
					return new OptimizeGainImpl((OptimizeGain*)cmd, systems);

            case WfsOpCodes::AUTOTRACK_CMD:
					return new AutoTrackImpl((AutoTrack*)cmd, systems);

            case WfsOpCodes::SETSOURCE_CMD:
					return new SetSourceImpl((SetSource*)cmd, systems);

            case WfsOpCodes::EMERGENCY_OFF:
					return new EmergencyOffImpl((EmergencyOff*)cmd, systems);

				case BasicOpCodes::RECOVER_FAILURE:
					return new OperateImpl((RecoverFailure*)cmd, systems);

				case BasicOpCodes::RECOVER_PANIC:
					return new OffImpl((RecoverPanic*)cmd, systems);

				default:
					return CommandImplFactory::implementCommand(cmd, systems);
			}
		}

};

}
}

#endif /*WFSCOMMANDIMPLFACTORY_H_INCLUDE*/
