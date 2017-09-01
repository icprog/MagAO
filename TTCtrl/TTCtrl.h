#ifndef TTCTRL_H
#define TTCTRL_H

class TTCtrl;
class TipTilt;
class Waves;
class Converter;
class CameraLens;

#include "AOApp.h"
#include "BcuLib/BcuCommon.h"
#include "RTDBvar.h"
using namespace Arcetri::Bcu;

#include "TTCtrlExceptions.h"
#include "TipTilt.h"
#include "CameraLens.h"
#include "Waves.h"
#include "Converter.h"

#include "stdarg.h"
#include <pthread.h>
using namespace std;

//@Class: TTCtrl
//
// Extends an AOApp to control the tip-tilt mirror
//
// TTCtrl manages low-level and high-level parameters for a the Tip-Tilt 
// mirror with 3 actuators (see above).
//
// High-level parameters are relative to mirror rotation (frequence [Hz],
// amplitude [Volts] and offset_x/y [Volts] of the rotation center) and are wrapped 
// in the TipTilt class. TTCtrl reference 2 TipTilt object instances, one
// for REQ (requested) and one for CUR (current) high-level parameters. 
// Both objects map their status in RTDB.
// The user asks for a change of high-level parameters by meaning of updating
// RTDB corresponding REQ variables and asking to TTCtrl to start/restart
// the mirror (RTDB "GO" variable). The TTCtrl advise for starting/restarting
// done by updating CUR variables.
// High-level parameters are translated into low-level parameters in order
// to perform mirror rotation (Converter class).
//
// Low-level parameters units:
// 	- Freq: 	hertz
// 	- Amp:		volts
// 	- Offset:	volts
//	- Phase:	degree (0-360)
//
// If user directly ask for a change to low-level parameters, high-level 
// parameters become invalid, and are not more used unless they are explicitly
// updated by the user. Consequently RTDB high-level CUR variables haven't 
// any meaning.
//
// Low-level parameters are relative to K actuators (this version supports only K=3;
// a modify to Waves and Converter classes is required to support a different number)
// and are wrapped in Waves class. TTCtrl reference 2 Waves object instances, one
// for REQ and one for CUR low-level parameters.
// TTCtrl control the mirror applying low-level parameters to actuators.
//
// The Converter class perform all conversion from high-level to low-level
// parameters, using some conversion parameters specified on config file.	
//
//		#Conversion parameters
// 		ZV_TO_XV	double		K	# X volts on actuator_x corresponds to a translation of -K*X volts
//									# along its axis direction.
//
// 		ROT_ANG		double 		0 	# Rotation af actuators axis with respect to default position
//									# Default position is actuator_1 matching Y-axis on sensor.
//									# I.e. 2.094 = 120° expressed in radiants
//
// Low-level parameters are subject to some bounds, to limit mirror movement. 
// Config file must specify low-level ranges for frequence and voltage: the 
// others low-level bounds and high-level bounds are computed from these.
//  
// 		# Low-level setting ranges (for actuators)
// 		MAX_FREQ	double		1000
// 		MIN_FREQ	double		0
// 		MAX_VOLT	double		10		# "Actuator zero" = (MAX_VOLT - MIN_VOLT) / 2
// 		MIN_VOLT	double		0
//
// Ranges are wrapped  in Conversion class for convenience reasons (it's the only class
// interested on them!).
//
// Default values for high-level parameters must be specified on config file:
// 
//		# Hi-Level default settings
// 		DEFAULT_FREQ 		double		1000 	# [Hertz]
// 		DEFAULT_AMP			double		0		# [Volts]
// 		DEFAULT_OFFSET_X	double 		0	 	# [Volts] By default rotates on mirror phisical center:
// 		DEFAULT_OFFSET_Y	double 		0	 	# => DEFAULT_OFFSET = 0  ==>  actuator on zero
//
//@
class TTCtrl: public AOApp {
	
	public:

      	TTCtrl(int argc, char **argv) throw (AOException);
      	
      	virtual ~TTCtrl();
               
        int getActNum() { return _actNum; }
        
        bool isCcd39CtrlReady();
        bool isCcd39Operating();
        bool isBcu39Operating();
        void initTT();
        void restTT();
		
		// Return requested and current hi-level (TipTilt obj) 
		// and low-level (Waves obj) status
		TipTilt *TipTiltReq() { return _tiptilt_req; }
		TipTilt *TipTiltCur() { return _tiptilt_cur; }
                CameraLens *CameraLensReq() { return _cameralens_req; }
                CameraLens *CameraLensCur() { return _cameralens_cur; }
		Waves *WavesReq() { return _waves_req; }
		Waves *WavesCur() { return _waves_cur; }

		// Obvious :-)
		void start() throw (AOVarException);
		void stop() throw (AOVarException);
		void updateRotationAngle(double rotAngle);
		void updateCcd39Ready(Variable* var);
		void updateCcd39Status(Variable* var);
		void updateCcd39Freq(Variable* var);
		void updateBcu39Status(Variable* var);

		// RTDB vars change handlers
		static int GoChanged(void *pt, Variable *msgb);
		static int RotationAngleReqChanged(void *pt, Variable *var);
		static int Ccd39ReadyChanged(void *pt, Variable *var);
		static int Ccd39FreqChanged(void *pt, Variable *var);
		static int Ccd39StatusChanged(void *pt, Variable *var);
		static int Bcu39StatusChanged(void *pt, Variable *var);
		static int TipTiltReqChanged(void *pt, Variable *msgb);
		static int CameraLensReqChanged(void *pt, Variable *msgb);
		static int WavesReqChanged(void *pt, Variable *msgb);	
		static int SetReqChanged(void *pt, Variable *var);

        void *engine() { return _engine; }

        double checkAmp( double freq, double amp);

    protected:

      	// Creates all the Waves and TipTilt objects
      	void CreateTT() throw (AOException);
      
      	// Set the Tiptilt using the current parameters
      	void setTT() throw (AOVarException, WavesException);
      	void setCameraLens();

		// VIRTUAL - inherited
		// Setup its own variables in RTDB
		void SetupVars();

        // VIRTUAL - inherited
		// Perform some post-initialization settings
		// - Load conversion parameters and ranges
		// - Load default configuration and save it on RTDB req vars
		// - Register req vars to be notified on change
		// - Compute default waves
		void PostInit();


	private:
	
		int _actNum;

		// RTDB var storing start/stop commands
		RTDBvar _var_go;
		RTDBvar _setReq, _setCur;

		// Converter for hi-level and low-level parameters
		Converter *_converter;
		RTDBvar _rotAngleReq;
		RTDBvar _rotAngleCur;
		
		RTDBvar* _ccd39Status;	// Need to set it to NULL
		RTDBvar* _ccd39Ready;	// Need to set it to NULL
		RTDBvar* _ccd39Freq;	// Need to set it to NULL

      RTDBvar _bcu39Status; 

		// Current and requested tiptilt parameters (hi-level))
		TipTilt *_tiptilt_cur, 
				*_tiptilt_req;

		// Current and requested tiptilt parameters (hi-level))
		CameraLens *_cameralens_cur, 
				*_cameralens_req;

		// Current and requested waves parameters (low-level)
		Waves *_waves_cur,
			  *_waves_req;
			  
		
		static pthread_mutex_t	_updatingMutex;
		
		string _ccd39CtrlFullName;
		void attachJoeCtrl();

      string _pingerPrefix;

      void *_engine;		
};


#endif //TTCTRL_H
