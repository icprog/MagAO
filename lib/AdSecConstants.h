#ifndef ADSECCONSTANTS_H_INCLUDED
#define ADSECCONSTANTS_H_INCLUDED

#include <map>

using namespace std;

namespace Arcetri {

	/*
	 * Configuration of the BCU in the AdSec subsystem
	 * 
	 * The WFS subsystem is composed of 6 (identical) + 1 units called CRATEs, named
	 * BcuMirror(i) and BcuSwitch. BcuSwitch can be enabled or disabled, changing the N_CRATES
	 * value to 1 or 0. The number of BcuMirror(i) can range from 0 to 6 changing the
	 * N_CRATES value.
	 * 
	 * There are 3 different kinds of boards in each crate (BCU, SGN, DSB)
	 * Each DSB (DSP board) has several DSP chips on board. 
	 * 
	 * !!! ATTENTION: the IDs match with those defined in MirrorCtrl config file to bind
	 * the BCU ip address
	 */
	namespace AdSecConstants {
	
		class BcuSwitch {
			public:
				
				static const int ID			 = 2;	// ***unique*** BCU identifier
				static const int N_CRATES 	 = 1;	// number of crates Switch in the AdSec system
				static const int N_BCU_CRATE = 1;   // number of BCU boards in each crate
				static const int N_SGN_CRATE = 0;   // number of SIGGEN boards in each crate
				static const int N_DSB_CRATE = 0;   // number of DSP boards in each crate
				static const int N_DSP_DSB 	 = 0;   // number of DSP chips in each board
				static const int N_CH_DSP_	 = 0;   // number of channels per DSP chips
				
				static const int N_DSP_CRATE = N_DSB_CRATE * N_DSP_DSB;    // number of DSP chips in each crate
				static const int N_CH_CRATE  = N_DSP_CRATE * N_CH_DSP_;    // number of channels in each crate
				
				static const int N_DSP = N_CRATES * N_DSP_CRATE;    // total number of DSP chips in the system
				static const int N_DSB = N_CRATES * N_DSB_CRATE;    // total number of DSB (DSP boards) in the system
				static const int N_SGN = N_CRATES * N_SGN_CRATE;    // total number of SGN in the system
				static const int N_CH  = N_DSP * N_CH_DSP_;         // total number of available channels	
		};
		
		class BcuMirror {
			public:
				
				static const int ID	 		 = 3;   // ***unique*** BCU identifier of the first crate
				static const int N_CRATES 	 = 6;	// number of crates Mirror in the AdSec system
				static const int N_BCU_CRATE = 1;   // number of BCU boards in each crate
				static const int N_SGN_CRATE = 1;   // number of SIGGEN boards in each crate
				static const int N_DSB_CRATE = 14;   // number of DSP boards in each crate
				static const int N_DSP_DSB 	 = 2;   // number of DSP chips in each board
				static const int N_CH_DSP_	 = 4;   // number of channels per DSP chips
				
				static const int N_DSP_CRATE = N_DSB_CRATE * N_DSP_DSB;    // number of DSP chips in each crate (14*2=28)
				static const int N_CH_CRATE  = N_DSP_CRATE * N_CH_DSP_;    // number of channels in each crate  (28*4=112)
				
				static const int N_DSP = N_CRATES * N_DSP_CRATE;    // total number of DSP chips in the system   (6*28=168)
				static const int N_DSB = N_CRATES * N_DSB_CRATE;    // total number of DSB (DSP boards) in the system (6*14=84)
				static const int N_SGN = N_CRATES * N_SGN_CRATE;    // total number of SGN in the system (6*1=6)
				static const int N_CH  = N_DSP * N_CH_DSP_;         // total number of available channels	(168*4=672)
		
				// The number of crates with accelerometers, and the first crate id.
				// Sum ACCELER_ALT_CRATE_NEXT to obtain the following crates with accelerometers.
				// These crates have an alternative id for SIGGEN (SIGGEN_BOARD_ID_ALT)
				static const int ACCELER_CRATES_NUM = 3;
				static const int ACCELER_CRATE_FIRST = 1;
				static const int ACCELER_CRATE_NEXT = 2;
				// Default dsp number for SIGGEN.
				static const int SIGGEN_BOARD_ID = 252;
				// Alternative dsp number for SIGGEN.
				static const int ACCELER_BOARD_ID = 248;
				static const int SIGGEN_BOARD_ID_ALT = ACCELER_BOARD_ID;
		};
		
	} // namespace AdSecConstants
	
	
} // namespace Arcetri

#endif // ADSECCONSTANTS_H_INCLUDED
