#ifndef ADSECOPCODES_H_INCLUDE
#define ADSECOPCODES_H_INCLUDE

#include "arblib/base/SerializableMessage.h"
#include "arblib/base/BasicOpCodes.h"

using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace AdSec_Arbitrator {

class AdSecOpCodes {

	public:

		// The first available code is 2004 (see BasicOpCodes.h)
		// NOTE: These opcodes must be entered in the AdSecArbFSM table
      //       which maps opcodes to FSM events (see AdSecArbFsm.cpp)
		static const OpCode ON_CMD    	           = 2004; 	//
		static const OpCode OFF_CMD        	        = 2005; 	//
		static const OpCode LOAD_PROGRAM_CMD        = 2006; 	//
		static const OpCode RESET_CMD        	     = 2007; 	//
		static const OpCode SET_FLAT_CHOP_CMD       = 2008;
		static const OpCode SET_FLAT_AO_CMD         = 2009;
		static const OpCode	REST_CMD		           = 2010;
		static const OpCode APPLY_COMMANDS_CMD      = 2011;
		static const OpCode CHOP_TO_AO_CMD          = 2012;
		static const OpCode AO_TO_CHOP_CMD          = 2013;
		static const OpCode RUN_AO_CMD        	     = 2014;
		static const OpCode STOP_AO_CMD             = 2015;
		static const OpCode RUN_CHOP_CMD            = 2016;
		static const OpCode STOP_CHOP_CMD           = 2017;
		static const OpCode SET_GAIN_CMD            = 2018;
		static const OpCode SET_DISTURB_CMD         = 2019;
		static const OpCode LOAD_SHAPE_CMD          = 2020;
		static const OpCode SAVE_SHAPE_CMD          = 2021;
		static const OpCode SET_REC_MAT_CMD         = 2022;
		static const OpCode SAVE_SLOPES_CMD         = 2023;
		static const OpCode SAVE_STATUS_CMD         = 2024;
		static const OpCode DATA_DECIMATION_CMD     = 2025;
		static const OpCode RECOVER_SKIP_FRAME_CMD  = 2026;
		static const OpCode PAUSE_AO_CMD            = 2027;
		static const OpCode RESUME_AO_CMD           = 2028;
		static const OpCode TT_OFFLOAD_CMD          = 2029;
		static const OpCode SET_ZERNIKES_CMD        = 2030;
		static const OpCode CALIBRATE_CMD           = 2031;
  		static const OpCode REQUEST_ADSEC_STATUS_CMD= 2032;
		static const OpCode SELECT_FOCAL_STATION_CMD= 2033;

//		static const OpCode DUMMY_CMD             = 2007; 	//
//		static const OpCode SET_FLAT_FF_CMD    = 2037;
//		static const OpCode IF_ACQUIRE_CMD     = 2038;
//		static const OpCode IF_REDUCE_CMD      = 2039;
//		static const OpCode DUST_TEST_CMD      = 2040;
//		static const OpCode COIL_TEST_CMD      = 2041;
//		static const OpCode CAPSENS_TEST_CMD   = 2042;
//		static const OpCode FF_ACQUIRE_CMD     = 2043;
//		static const OpCode FF_REDUCE_CMD      = 2044;
//		static const OpCode WD_TEST_CMD        = 2045;
//		static const OpCode FL_TEST_CMD        = 2046;
//		static const OpCode PS_TEST_CMD        = 2047;
//		static const OpCode CORRECT_MODES_CMD  = 2048;

};

}
}

#endif /* ADSECOPCODES_H_INCLUDE */
