
#ifndef WFSCONSTANTS_H_INCLUDED
#define WFSCONSTANTS_H_INCLUDED

namespace Arcetri {

	/*
	 * Configuration of the BCU in the WFS subsystem
	 * 
	 * The WFS subsystem is composed of 2 units called CRATEs, named
	 * Bcu39 and Bcu47. Each can be enabled or disabled, changing the N_CRATES
	 * value to 1 or 0.
	 * 
	 * There are 2 different kinds of boards in each crate (BCU, WGN)
	 * 
	 * !!! ATTENTION: the IDs match with those defined in MirrorCtrl config file to bind
	 * the ip address
	 */
	namespace WfsConstants {
	
		class Bcu39 {
			public:
				
				static const int ID			 = 0;	// ***unique*** BCU identifier
				static const int N_CRATES 	 = 1;	// number of crates 39 in the Wfs system
				static const int N_BCU_CRATE = 1;   // number of BCU boards in each crate
				static const int N_WGN_CRATE = 0;   // number of WAVEGEN boards in each crate
				static const int N_SGN_CRATE = 0;   // number of SIGGEN boards in each crate
				static const int N_DSB_CRATE = 0;   // number of DSP boards in each crate
				static const int N_DSP_DSB 	 = 0;   // number of DSP chips in each board
				static const int N_CH_DSP_	 = 0;   // number of channels per DSP chips
				
				static const int N_DSP_CRATE = N_DSB_CRATE * N_DSP_DSB;    // total number of DSP chips in each crate
				
				static const int N_DSP = N_CRATES * N_DSP_CRATE;    // total number of DSP chips in the system
				static const int N_CH  = N_DSP * N_CH_DSP_;         // total number of available channels	

                static const int N_BCU = N_CRATES * N_BCU_CRATE;
                static const int N_WGN = N_CRATES * N_WGN_CRATE;

		};
		
		class Bcu47 {
			public:
				
				static const int ID			 = 1;	// ***unique*** BCU identifier
				static const int N_CRATES 	 = 1;	// number of crates 47 in the Wfs system
				static const int N_BCU_CRATE = 1;   // number of BCU boards in each crate
				static const int N_WGN_CRATE = 1;   // number of WAVEGEN boards in each crate
				static const int N_SGN_CRATE = 0;   // number of SIGGEN boards in each crate
				static const int N_DSB_CRATE = 0;   // number of DSP boards in each crate
				static const int N_DSP_DSB 	 = 0;   // number of DSP chips in each board
				static const int N_CH_DSP_	 = 0;   // number of channels per DSP chips
				
				static const int N_DSP_CRATE = N_DSB_CRATE * N_DSP_DSB;    // total number of DSP chips in each crate
				
				static const int N_DSP = N_CRATES * N_DSP_CRATE;    // total number of DSP chips in the system
				static const int N_CH  = N_DSP * N_CH_DSP_;         // total number of available channels	

                static const int N_BCU = N_CRATES * N_BCU_CRATE;
                static const int N_WGN = N_CRATES * N_WGN_CRATE;
		};
		
	} // namespace WfsConstants
	
	
} // namespace Arcetri

#endif //WFSCONSTANTS_H_INCLUDED
