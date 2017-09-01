#ifndef COMMANDS_H_INCLUDE
#define COMMANDS_H_INCLUDE

#include <string>
using namespace std;

#define BURST_NAME_MAX_SIZE 40
#define MEAS_PREFIX_MAX_SIZE 40
#define REMOTE_PATH_MAX_SIZE 160


//********************************************************//
//  THIS SOLUTION IS HIDEOUS!!! Do it with serialization! //
//********************************************************//

namespace Arcetri {

	/*
	 * Struct sent in the message body of a MsgD message with code
	 * INTF_GET_MEAS by the method
	 * 	InterferometerInterface::getMeasurement(string measPrefix)
	 * and sent back to the client wit OUT PARAM set
	 */
	struct MeasurementParams {
		char measPrefix[MEAS_PREFIX_MAX_SIZE+1];	// IN PARAM
		char remoteFileOut[REMOTE_PATH_MAX_SIZE+1];	// OUT PARAM
	};

	/*
	 * Struct sent in the message body of a MsgD message with code
	 * INTF_GET_BURST by the method
	 * 	InterferometerInterface::getMeasurementBurst(int measNum, string burstname)
	 * and sent back to the client wit OUT PARAM set
	 */
	struct MeasurementBurstParams {
		int  measNum;								// IN PARAM
		char burstName[BURST_NAME_MAX_SIZE+1];		// IN PARAM
		char remotePathOut[REMOTE_PATH_MAX_SIZE+1];	// OUT PARAM
	};

	/*
	 * Struct sent in the message body of a MsgD message with code
	 * INTF_POST_PROC by the method
	 * 	InterferometerInterface::postProcess(string remotePath, string measPrefix, int measNum)
	 * and sent back to the client wit OUT PARAM set
	 */
	struct PostProcessParams {
		char remotePath[REMOTE_PATH_MAX_SIZE+1];	// IN PARAM
		char measPrefix[MEAS_PREFIX_MAX_SIZE+1];	// IN PARAM
		int  measNum;								// IN PARAM
		char remotePathOut[REMOTE_PATH_MAX_SIZE+1];	// OUT PARAM (if "NONE" means post/proc not done
													//			  bacause set to NONE)
	};

	/*
	 * Creates parameters structs taking standard C++ types
	 */
	class ParamsFactory {

		public:

			static MeasurementParams CreateMeasurementParams(string measPrefix) {
				MeasurementParams params;
				measPrefix.resize(MEAS_PREFIX_MAX_SIZE, '\0');
				memcpy((char*)params.measPrefix, measPrefix.c_str(), MEAS_PREFIX_MAX_SIZE);
				params.measPrefix[MEAS_PREFIX_MAX_SIZE] = '\0';	// NULL termination
				memset(params.remoteFileOut, '\0', REMOTE_PATH_MAX_SIZE+1);
				return params;
			};

			static void SetMeasurementParamsRemoteFileOut(MeasurementParams& params, string remoteFileOut) {
				remoteFileOut.resize(REMOTE_PATH_MAX_SIZE, '\0');
				memcpy(params.remoteFileOut, remoteFileOut.c_str(), REMOTE_PATH_MAX_SIZE);
				params.remoteFileOut[REMOTE_PATH_MAX_SIZE] = '\0';	// NULL termination
			}

			static MeasurementBurstParams CreateMeasurementBurstParams(int measNum, string burstName) {
				MeasurementBurstParams params;
				params.measNum = measNum;
				burstName.resize(BURST_NAME_MAX_SIZE, '\0');
				memcpy((char*)params.burstName, burstName.c_str(), BURST_NAME_MAX_SIZE);
				params.burstName[BURST_NAME_MAX_SIZE] = '\0';
				memset(params.remotePathOut, '\0', REMOTE_PATH_MAX_SIZE+1);
				return params;
			};

			static void SetMeasurementBurstParamsRemotePathOut(MeasurementBurstParams& params, string remotePathOut) {
				remotePathOut.resize(REMOTE_PATH_MAX_SIZE, '\0');
				memcpy(params.remotePathOut, remotePathOut.c_str(), REMOTE_PATH_MAX_SIZE);
				params.remotePathOut[REMOTE_PATH_MAX_SIZE] = '\0';	// NULL termination
			}

			static PostProcessParams CreatePostProcessParams(string remotePath, string measPrefix, int measNum) {
				PostProcessParams params;
				remotePath.resize(REMOTE_PATH_MAX_SIZE, '\0');
				memcpy((char*)params.remotePath, remotePath.c_str(), REMOTE_PATH_MAX_SIZE);
				params.remotePath[REMOTE_PATH_MAX_SIZE] = '\0';
				measPrefix.resize(MEAS_PREFIX_MAX_SIZE, '\0');
				memcpy((char*)params.measPrefix, measPrefix.c_str(), MEAS_PREFIX_MAX_SIZE);
				params.measPrefix[MEAS_PREFIX_MAX_SIZE] = '\0';
				params.measNum = measNum;
				memset(params.remotePathOut, '\0', REMOTE_PATH_MAX_SIZE+1);
				return params;
			};

			static void SetPostProcessParamsRemotePathOut(PostProcessParams& params, string remotePathOut) {
				remotePathOut.resize(REMOTE_PATH_MAX_SIZE, '\0');
				memcpy(params.remotePathOut, remotePathOut.c_str(), REMOTE_PATH_MAX_SIZE);
				params.remotePathOut[REMOTE_PATH_MAX_SIZE] = '\0';	// NULL termination
			}

	};

}

#endif /*COMMANDS_H_INCLUDE*/
