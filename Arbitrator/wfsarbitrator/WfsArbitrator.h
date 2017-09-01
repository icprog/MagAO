#ifndef WFSARBITRATOR_H_INCLUDE
#define WFSARBITRATOR_H_INCLUDE

#include "framework/AbstractArbitrator.h"
#include "arblib/base/ArbitratorInterface.h"

// Define this to make the Pause and Resume commands a no-op
#undef FAKE_PAUSE

#include "adcLut.h"

using namespace Arcetri;
using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace Wfs_Arbitrator {


class WfsArbitrator: public AbstractArbitrator {

	public:

		WfsArbitrator(string config);

		WfsArbitrator(int argc, char* argv[]);

		virtual ~WfsArbitrator();

      string fsmState();

	protected:

		/*
		 * Setup the AdSec vars.
		 */
		void SetupVars();

		/*
		 * Install handlers for:
		 * 	- WfsHoArmPolicy
		 *  - WfsTvArmPolicy
		 *  - WfsSetAndPointSourcePolicy
		 */
		void InstallHandlers();

		/*
		 * Perform extra checks and initializations:
		 * 	-
		 */
		void PostInit();

      /*
       * Update RTDB status variables
       */

      void updateRtdb();

      /*
       * Handler for variable mirroring into MSGD.
       */

      static int mirrorHandler( void *pt, Variable *msgb);

      map<string, string> _toMirror;

      /*
       * Called by the handler of messages with code ARB_REQ in case
       * the request can't be satisfied by base class
       */
      void serviceRequest(Request* /*req*/, MsgBuf* /*requestMsgBuf*/);



	private:

		/*
		 * Registered handler for messages with code:
		 * 	- WFS_ARB_SET_HO_POLICY
		 * 	- WFS_ARB_SET_TV_POLICY
		 * 	- WFS_ARB_SET_AND_POINT_SOURCE_POLICY
		 *
		 * This kind of messages contain respectively a simple struct of type:
		 *  	- WfsHoArmPolicy
		 *  	- WfsTvArmPolicy
		 *  	- WfsSetAndPointSourcePolicy
		 *  and not a serialized object: this allow a (python) client to send a
		 *  simple message without using boost.
		 */
		static int handleSetHoArmPolicy(MsgBuf* requestMsgBuf, void *argp, int hndlrQueueSize);
		static int handleSetTvArmPolicy(MsgBuf* requestMsgBuf, void *argp, int hndlrQueueSize);
		static int handleSetAndPointSourcePolicy(MsgBuf* requestMsgBuf, void *argp, int hndlrQueueSize);


	private: // METHODS

		void create();

		void processAlert(Alert* alert);


      string getCalibUnitState();

	private:	// FIELDS

		static const int HNDLR_QUEUE_MESSAGES_LIMIT = 10;

      // WFS variables in RTDB, written by this arbitrator

      RTDBvar _ccdFreqLimits;
      RTDBvar _modAmpl;
      RTDBvar _msg;
      RTDBvar _pyramidPos;

      // Rerotator & ADC tracking

public:
      bool _rerotTracking;
      bool _adcTracking;
      bool _lensTracking;
      bool _lensTrackingTarget;
      bool _lensTrackingOutOfRange;
      bool _lensTrackingAlert;
      double _lensTrackingTH;
      double _rotatorOffset[5];
      double _rotatorChangeOffset[5];
      double _adcOffset1, _adcOffset2;
      double _rotatorSign;
      double _maxMasterSpeed;
      int _labmode;
      double _focalPlaneRotation;

      string _wfsSpec;            // Wfs specification (AO_WFS_FLAO, AO_WFS_LBTI, etc)

      RTDBvar _telElevation, _telDerotator;
      RTDBvar _offsetBin1Req, _offsetBin1Cur;
      RTDBvar _offsetBin2Req, _offsetBin2Cur;
      RTDBvar _offsetBin3Req, _offsetBin3Cur;
      RTDBvar _offsetBin4Req, _offsetBin4Cur;
      RTDBvar _offsetBin5Req, _offsetBin5Cur;

      RTDBvar _offsetAdc1Cur, _offsetAdc1Req;
      RTDBvar _offsetAdc2Cur, _offsetAdc2Req;

      RTDBvar _clRefX;
      RTDBvar _clRefY;
      RTDBvar _clRefRot;

      RTDBvar _ccd39Pup0;
      RTDBvar _ccd39Pup1;
      RTDBvar _ccd39Pup2;
      RTDBvar _ccd39Pup3;

      RTDBvar _clXcur, _clXreq, _clYcur, _clYreq, _clGo;
      RTDBvar _clXErr, _clYErr, _clErrCorrecting;
      RTDBvar _led;
      RTDBvar _bcu39sx;
      RTDBvar _flEnabled;
      RTDBvar _pupils;
      RTDBvar _slopeCompPupils;
      RTDBvar _safeSkipPercent;
      RTDBvar _adsecGain;
      RTDBvar _clTrackReq;
      RTDBvar _aoLabMode;
      RTDBvar _curSlopenull;
      RTDBvar _reqSlopenull;

      

      void enableRerotTracking(bool enable) { _rerotTracking = enable; }
      void enableAdcTracking(bool enable) { _adcTracking = enable; }
      void enableLensTracking( bool enable);

      static int trackingHandler( void *pt, Variable *msgb);
      static int pupTrackingHandler( void *pt, Variable *msgb);
      static int offsetReqChanged( void *pt, Variable *msgb);
      static int pupChangedHandler( void *pt, Variable *msgb);
      static int safeSkipPercentHandler( void *pt, Variable *msgb);
      static int adsecGainHandler( void *pt, Variable *msgb);
      static int clTrackReqChanged( void *pt, Variable *var);
      static int labModeHandler( void *pt, Variable *var);

      static int isLoopClosed( WfsArbitrator *arb);

      void speak( string msg);


      void adcTrack();
      void rerotTrack(); 
      void lensTrack(); 
      void updateSlopenull( double derotPos);

      bool _telElOk;
      bool _slopenullOk;
      bool _safeSkipOk;
      bool _adsecGainOk;

      double derot2rerot( double derotPos);
      double elev2adc( double elevPos);

      void rotate( double x, double y, double *newx, double *newy);

      // ADC lookup table
      bool _adcLutValid;
      adcMap _adc1, _adc2;

      int _adc1Limits;
      double _adc1Min;
      double _adc1Max;

      //If Zero Track is 1, then the ADCs are set to 0 dispersion position and don't move
      RTDBvar _adcZeroTrack;
      
      // Sync led variable and FSM state with the actual WFS status
      void sync();


       /*
        * Function called roughly once per second.
        */

      virtual void Periodic();


      /*
       * Voice configuration
       */
      std::string _voiceHost;
      int _voicePort;
      std::string _voiceName;
      std::string _voiceCameraLensWarning;
      std::string _voiceADCTrackingEnabled;
      std::string _voiceADCZeroTrackEnabled;
      std::string _voiceADCMinimumWarning;
      std::string _voiceADCMaximumWarning;

};

}
}

#endif /*WFSARBITRATOR_H_INCLUDE*/
