#ifndef WFSOPCODES_H_INCLUDE
#define WFSOPCODES_H_INCLUDE

#include "arblib/base/SerializableMessage.h"
#include "arblib/base/BasicOpCodes.h"

using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace Wfs_Arbitrator {

class WfsOpCodes {

	public:

		// The first available code is 2002 (see BasicOpCodes.h), last is 2999
		// NOTE: subtracting BasicOpCodes::FORCE_FAILURE code from an OpCode you
		//       must obtain the corresponding FSM event (see AOFsm.h, Event enum)
		static const OpCode OPERATE_CMD    	  		= 2004;
		static const OpCode OFF_CMD    	      		= 2005;
		static const OpCode PREPARE_ACQUIRE_REF_CMD = 2006;
		static const OpCode ACQUIRE_REF_CMD	 		= 2007;
		static const OpCode CLOSE_LOOP_CMD		  	= 2008;
		static const OpCode PAUSE_LOOP_CMD	  	 	= 2009;
		static const OpCode RESUME_LOOP_CMD	  		= 2010;
		static const OpCode STOP_LOOP_CMD	  		= 2011;
		static const OpCode MODIFY_AO_CMD	  		= 2012;
		static const OpCode REFINE_LOOP_CMD	  		= 2013;

      static const OpCode CALIBRATE_HODARK_CMD    = 2014;
      static const OpCode CALIBRATE_MOVEMENTS_CMD = 2015;
      static const OpCode CALIBRATE_SLOPENULL_CMD = 2017;
      static const OpCode CALIBRATE_TVDARK_CMD    = 2018;
      static const OpCode GET_TV_SNAP_CMD         = 2019;
      static const OpCode SAVE_STATUS_CMD         = 2020;
      static const OpCode SAVE_OPTLOOPDATA_CMD    = 2022;
      static const OpCode ENABLE_DISTURB_CMD      = 2023;

      static const OpCode OFFSET_XY               = 2024;
      static const OpCode OFFSET_Z                = 2025;
      static const OpCode CORRECT_MODES           = 2026;
      static const OpCode EMERGENCY_OFF           = 2027;
      static const OpCode CHECK_REF_CMD           = 2028;

      static const OpCode SETSOURCE_CMD           = 2029;
      static const OpCode REQUEST_WFS_STATUS_CMD      = 2030;

      static const OpCode CALIBRATE_IRTCDARK_CMD  = 2031;
      static const OpCode PREPARE_ADSEC_CMD = 2032;
      static const OpCode STOPLOOP_ADSEC_CMD = 2033;

      static const OpCode ANTIDRIFT_CMD  = 2034;

      static const OpCode AUTOTRACK_CMD  = 2035;
      static const OpCode CALIBRATE_PISCESDARK_CMD  = 2036;

      static const OpCode OPTIMIZE_GAIN_CMD  = 2037;
};

}
}

#endif /* WFSOPCODES_H_INCLUDE */
