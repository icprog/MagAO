#ifndef CAMERALENS_H
#define CAMERALENS_H

#include "TTCtrl.h"

//@Class: CameraLens
//
// Defines the status of camera lens
//
// A status can be Current (CUR) or Requested (REQ), and
// in both cases is stored in the RTDB. A CameraLens object allow
// a synchronization between RTDB variables related to CameraLens
// and the values locally managed by TTCtrl.
//@
class CameraLens {
	
	public:

		// Create the CameraLens writing the related vars to RTDB
		CameraLens(TTCtrl *ctrl, int direction);	

		// A CUR CameraLens is valid only if it have been succesfully applied,
		// that is, have been succesfully converted to Waves and sent to BCU.
		// This mean that CUR Waves object stored on RTDB derives from the
		// conversion of CUR/REQ (are identical) CameraLens.
		// When the user directly asks for a change to low-level parameters (Waves),
		// the CUR CameraLens must be set to invalid !!! (see TTCtrl.WavesReqChanged(...))
		void setValid(bool b) { _valid = b; }
		bool Valid() { return _valid; }

		// Setup RTDB vars
		void setupVars();

		// Initialize RTDB vars with default values from Config_File. 
		// If values are not found, get them from TTDefaultSetings.h
		void initVars() throw (Config_File_Exception);

		// Register RTDB vars to be notified on change
		void notifyVars();

		// Copy status (except direction) from another CameraLens object.
		// Shuld be used to copy REQ object to CUR obj.
		void copy(CameraLens *another);

		// Get CameraLens local status
		double Pos_x() throw (AOVarException);
		double Pos_y() throw (AOVarException);

		// Set CameraLens status (without sending to RTDB)
		void setPos_x(const double offset_x) throw (AOVarException) ;
		void setPos_y(const double offset_y) throw (AOVarException) ;	

		// Update the CameraLens status, downloading status form RTDB
		bool update(Variable *var) throw (AOVarException);

		// Save the CameraLens, uploading status to RTDB
		void save() throw (AOVarException);

                // Send values to BCU
                void apply();

	private:

		// A reference to my controller
		TTCtrl *_myCtrl;

		// CUR_VAR or REQ_VAR
		int _direction;

		// Say if the CameraLens status is valid. When a Waves variable is
		// notified for a change from RTDB, _valid become "false".
		// When CameraLens variable is notified from RTDB, _valid 
		// become "true".
		bool _valid;

		// RTDB variable corresponding to object status
		RTDBvar _pos_x,
			_pos_y;
};

#endif //CAMERALENS_H
