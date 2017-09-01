#ifndef TT_WAVES_H
#define TT_WAVES_H

#include "TTCtrl.h"
#include "TTCtrlExceptions.h"

#include "BcuLib/BcuCommon.h"
using namespace Arcetri::Bcu;



//@Class: Waves
//
// Defines K sine waves for the tip-tilt generator, that is
// the low-level parameters for te TTCtrl.
// (Note: this version supports only K = 3 !!!)
// 
// K, the number of actuators, match the number of waves.
// A Waves object can be Requested (REQ), if it maps a requested state 
// of the mirror, or Current (CUR), if it maps the current status of the 
// mirror. In both cases a Waves object mantains its status using some 
// RTDB variables, and allow a synchroniztion with them (matchAndRetrieve method).
//@
class Waves {
	
	public:

		// Create the Waves writing the related vars to RTDB
		Waves(TTCtrl *ctrl, 	// The TipTilt Controller instantiating this object
			  int direction)	// direction is CUR_VAR or REQ_VAR 
			  throw(WavesException);
			  
		~Waves();

		// Setup RTDB variables storing object status
		void setupVars() throw (AOVarException);
		
		void initVars() throw (Config_File_Exception);
		
		void notifyVars();

		// Copy status (except direction) from another Waves object
		void copy(Waves *another) throw (AOVarException);

		// Get Waves local status, copying values into received parameter.
		void Freqs(double freqs[]) throw (AOVarException);
		void Amps(double amps[]) throw (AOVarException);
		void Offsets(double offsets[]) throw (AOVarException);
		void Phases(double phases[]) throw (AOVarException);

		// Set Waves local status, getting values from received parameters.
		// (doesn't send changes to RTDB. Use save() for this purpose)
		void setFreqs(const double freqs[]) throw (AOVarException);
		void setAmps(const double amps[]) throw (AOVarException);
		void setOffsets(const double offsets[]) throw (AOVarException);
		void setPhases(const double phases[]) throw (AOVarException);	

		// Update the Waves status, downloading parameters form RTDB
		bool update(Variable *var) throw (AOVarException);

		// Apply Waves (sends values to BCU) 
		void apply() throw (AOVarException, WavesException);

		// Save Waves status to RTDB
		void save() throw (AOVarException);


	private:

		// A reference to my controller
		TTCtrl *_myCtrl;

		// CUR_VAR or REQ_VAR
		int _direction;

		// Number of actuators, that is the number of waves.
		// This version supports only 3 actuators (_n_act = 3) !!!
		int _n_act;

		// Each of this vars contains an array[WAVES] 
		// with values corresponding to WAVES (=3) waves
		RTDBvar _freqs,
				_amps,
				_offsets,
				_phases;
				
		// Stores waves in binary format ready for the BCU
		struct wave_params {
			float freq;
			float phase;
			float offset;
			float amp;
		};
		
		// Object used to send commands to MirrorCtrl and
		// timeout 
		CommandSender* _comSender;
		int _timeout_ms;
};

#endif //TT_WAVES_H
