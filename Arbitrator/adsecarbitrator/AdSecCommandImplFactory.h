#ifndef ADSECCOMMANDIMPLFACTORY_H_INCLUDE
#define ADSECCOMMANDIMPLFACTORY_H_INCLUDE

#include "framework/CommandImplFactory.h"

#include "adsecarbitrator/AdSecCommandsImpl.h"

using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace AdSec_Arbitrator {

class AdSecCommandImplFactory: public CommandImplFactory {

	public:

		AdSecCommandImplFactory(): CommandImplFactory() {}

		virtual ~AdSecCommandImplFactory() {}

		virtual CommandImpl* implementCommand(Command* cmd, map<string, AbstractSystem*> systems) {
			_logger->log(Logger::LOG_LEV_DEBUG, "AdSec CommandImplFactory retrieving command implementation...");
			switch(cmd->getCode()) {

				case AdSecOpCodes::ON_CMD:
					return new OnImpl((On*)cmd, systems);

				case AdSecOpCodes::OFF_CMD:
					return new OffImpl((Off*)cmd, systems);

				case AdSecOpCodes::LOAD_PROGRAM_CMD:
					return new LoadProgramImpl((LoadProgram*)cmd, systems);

				case AdSecOpCodes::RESET_CMD:
					return new ResetImpl((Reset*)cmd, systems);

				case AdSecOpCodes::SET_FLAT_CHOP_CMD:
					return new SetFlatChopImpl((SetFlatChop*)cmd, systems);

//				case AdSecOpCodes::SET_FLAT_SL_CMD:
//					return new SetFlatSlImpl((SetFlatSl*)cmd, systems);

				case AdSecOpCodes::SET_FLAT_AO_CMD:
					return new SetFlatAoImpl((SetFlatAo*)cmd, systems);

				case AdSecOpCodes::REST_CMD:
					return new RestImpl((Rest*)cmd, systems);

				case AdSecOpCodes::APPLY_COMMANDS_CMD:
					return new ApplyCommandsImpl((ApplyCommands*)cmd, systems);

//				case AdSecOpCodes::CHOP_TO_SL_CMD:
//					return new ChopToSlImpl((ChopToSl*)cmd, systems);

//				case AdSecOpCodes::AO_TO_SL_CMD:
//					return new AoToSlImpl((AoToSl*)cmd, systems);

//				case AdSecOpCodes::SL_TO_AO_CMD:
//					return new SlToAoImpl((SlToAo*)cmd, systems);

				case AdSecOpCodes::CHOP_TO_AO_CMD:
					return new ChopToAoImpl((ChopToAo*)cmd, systems);

				case AdSecOpCodes::AO_TO_CHOP_CMD:
					return new AoToChopImpl((AoToChop*)cmd, systems);

//				case AdSecOpCodes::SL_TO_CHOP_CMD:
//					return new SlToChopImpl((SlToChop*)cmd, systems);

				case AdSecOpCodes::RUN_CHOP_CMD:
					return new RunChopImpl((RunChop*)cmd, systems);

				case AdSecOpCodes::STOP_CHOP_CMD:
					return new StopChopImpl((StopChop*)cmd, systems);

				case AdSecOpCodes::RUN_AO_CMD:
					return new RunAoImpl((RunAo*)cmd, systems);

				case AdSecOpCodes::SET_ZERNIKES_CMD:
					return new SetZernikesImpl((SetZernikes*)cmd, systems);

				case AdSecOpCodes::STOP_AO_CMD:
					return new StopAoImpl((StopAo*)cmd, systems);

				case AdSecOpCodes::PAUSE_AO_CMD:
					return new PauseAoImpl((PauseAo*)cmd, systems);

				case AdSecOpCodes::RESUME_AO_CMD:
					return new ResumeAoImpl((ResumeAo*)cmd, systems);

				case AdSecOpCodes::SET_GAIN_CMD:
					return new SetGainImpl((SetGain*)cmd, systems);

				case AdSecOpCodes::SET_DISTURB_CMD:
					return new SetDisturbImpl((SetDisturb*)cmd, systems);

				case AdSecOpCodes::SET_REC_MAT_CMD:
					return new SetRecMatImpl((SetRecMat*)cmd, systems);

				case AdSecOpCodes::LOAD_SHAPE_CMD:
					return new LoadShapeImpl((LoadShape*)cmd, systems);

				case AdSecOpCodes::SAVE_SHAPE_CMD:
					return new SaveShapeImpl((SaveShape*)cmd, systems);

				case AdSecOpCodes::SAVE_SLOPES_CMD:
					return new SaveSlopesImpl((SaveSlopes*)cmd, systems);

				case AdSecOpCodes::SAVE_STATUS_CMD:
					return new SaveStatusImpl((SaveStatus*)cmd, systems);

				case AdSecOpCodes::DATA_DECIMATION_CMD:
					return new SetDataDecimationImpl((SetDataDecimation*)cmd, systems);

				case AdSecOpCodes::SELECT_FOCAL_STATION_CMD:
					return new SelectFocalStationImpl((SelectFocalStation*)cmd, systems);

				case AdSecOpCodes::TT_OFFLOAD_CMD:
					return new TTOffloadImpl((TTOffload*)cmd, systems);

				case AdSecOpCodes::RECOVER_SKIP_FRAME_CMD:
					return new RecoverSkipFrameImpl((RecoverSkipFrame*)cmd, systems);

//				case AdSecOpCodes::SET_FLAT_FF_CMD:
//					return new SetFlatFFImpl((SetFlatFF*)cmd, systems);

//				case AdSecOpCodes::IF_ACQUIRE_CMD:
//					return new IfAcquireImpl((IfAcquire*)cmd, systems);

//				case AdSecOpCodes::IF_REDUCE_CMD:
//					return new IfReduceImpl((IfReduce*)cmd, systems);

//				case AdSecOpCodes::DUST_TEST_CMD:
//					return new DustTestImpl((DustTest*)cmd, systems);

//				case AdSecOpCodes::COIL_TEST_CMD:
//					return new CoilTestImpl((CoilTest*)cmd, systems);

//				case AdSecOpCodes::CAPSENS_TEST_CMD:
//					return new CapsensTestImpl((CapsensTest*)cmd, systems);

//				case AdSecOpCodes::FF_ACQUIRE_CMD:
//					return new FFAcquireImpl((FFAcquire*)cmd, systems);

//				case AdSecOpCodes::FF_REDUCE_CMD:
//					return new FFReduceImpl((FFReduce*)cmd, systems);

//				case AdSecOpCodes::WD_TEST_CMD:
//					return new WdTestImpl((WdTest*)cmd, systems);

//				case AdSecOpCodes::FL_TEST_CMD:
//					return new FlTestImpl((FlTest*)cmd, systems);

//				case AdSecOpCodes::PS_TEST_CMD:
//					return new PsTestImpl((PsTest*)cmd, systems);

				case AdSecOpCodes::CALIBRATE_CMD:
					return new CalibrateImpl((CalibrateCmd*)cmd, systems);

				case BasicOpCodes::RECOVER_FAILURE:
					return new RecoverFailureImpl((RecoverFailure*)cmd, systems);

				case BasicOpCodes::RECOVER_PANIC:
					return new OffImpl((RecoverPanic*)cmd, systems);

				default:
					return CommandImplFactory::implementCommand(cmd, systems);
			}
		}

};

}
}

#endif /*ADSECCOMMANDIMPLFACTORY_H_INCLUDE*/
