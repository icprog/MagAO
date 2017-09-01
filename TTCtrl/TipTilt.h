#ifndef TT_TIPTILT_H
#define TT_TIPTILT_H

#include "TTCtrl.h"

//@Class: TipTilt
//
// Defines the status of tip-tilt mirror.
//
// A status can be Current (CUR) or Requested (REQ), and
// in both cases is stored in the RTDB. A TipTilt object allow
// a synchronization between RTDB variables related to TipTilt
// and the values locally managed by TTCtrl.
//@
class TipTilt {
	
	public:

		// Create the TipTilt writing the related vars to RTDB
		TipTilt(TTCtrl *ctrl, int direction);	

		// A CUR TipTilt is valid only if it have been succesfully applied,
		// that is, have been succesfully converted to Waves and sent to BCU.
		// This mean that CUR Waves object stored on RTDB derives from the
		// conversion of CUR/REQ (are identical) TipTilt.
		// When the user directly asks for a change to low-level parameters (Waves),
		// the CUR TipTilt must be set to invalid !!! (see TTCtrl.WavesReqChanged(...))
		void setValid(bool b) { _valid = b; }
		bool Valid() { return _valid; }

		// Setup RTDB vars
		void setupVars();

		// Initialize RTDB vars with default values from Config_File. 
		// If values are not found, get them from TTDefaultSetings.h
		void initVars() throw (Config_File_Exception);

		// Register RTDB vars to be notified on change
		void notifyVars();

		// Copy status (except direction) from another TipTilt object.
		// Shuld be used to copy REQ object to CUR obj.
		void copy(TipTilt *another);

		// Get TipTilt local status
		double Freq() throw (AOVarException);
		double Amp() throw (AOVarException);
		double Offset_x() throw (AOVarException);
		double Offset_y() throw (AOVarException);

		// Set TipTilt status (without sending to RTDB)
		void setFreq(const double freq) throw (AOVarException);
		void setAmp(const double amp) throw (AOVarException);
		void setOffset_x(const double offset_x) throw (AOVarException) ;
		void setOffset_y(const double offset_y) throw (AOVarException) ;	

		// Update the TipTilt status, downloading status form RTDB
		bool update(Variable *var) throw (AOVarException);

		// Save the TipTilt, uploading status to RTDB
		void save() throw (AOVarException);


	private:

		// A reference to my controller
		TTCtrl *_myCtrl;

		// CUR_VAR or REQ_VAR
		int _direction;

		// Say if the TipTilt status is valid. When a Waves variable is
		// notified for a change from RTDB, _valid become "false".
		// When TipTilt variable is notified from RTDB, _valid 
		// become "true".
		bool _valid;

		// RTDB variable corresponding to object status
		RTDBvar _freq,
				_amp,
				_offset_x,
				_offset_y;
};

#endif //TT_TIPTILT
