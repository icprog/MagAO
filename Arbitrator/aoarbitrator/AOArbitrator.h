#ifndef AOARBITRATOR_H_INCLUDE
#define AOARBITRATOR_H_INCLUDE

#include "framework/AbstractArbitrator.h"
#include "arblib/wfsArb/WfsInterfaceDefines.h"
#include "arblib/aoArb/AOPTypes.h"

#include "aoarbitrator/WfsArbSystem.h"

#include "AOGlobals.h"


#define AOARBITRATOR_STATESSYNC_STR  "StatesSync"
#define AOARBITRATOR_STATESWFS_STR   "States.Wfs"
#define AOARBITRATOR_STATESADSEC_STR "States.Adsec"
#define AOARBITRATOR_WFS_STATEVAR    "WfsStateVar"
#define AOARBITRATOR_ADSEC_STATEVAR  "AdsecStateVar"

#define FAKE_PAUSE

using namespace Arcetri;
using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace Arbitrator {



    class AOArbStatesSyncStruct {
      public:
        string state;
        string substate;
    };

    
/*
 * Top level arbitrator for the AO-Supervisor.
 *
 * It receives commands from AOS and coordinates the activity
 * of the lower level arbitrators (ADSecArb and WfsArb).
 *
 * It uses the TCS (i.e. to request modes offloading) sending
 * request to AOS. It also log to TCS important informations on
 * AO Supervisor activity (TODO).
 */
class AOArbitrator: public AbstractArbitrator {

	public:

		AOArbitrator(string cfgFile);
		AOArbitrator( int argc, char **argv);

		virtual ~AOArbitrator();

      ConnectionStatus updateAdsecConnectionStatus();
		/*
		 * Get the AO mode
		 */
		AOModeType getAOMode();

		/*
		 * Set the AO MODE (FullAO or SeeingLimited)
		 */
		void setAOMode(AOModeType mode);

		/*
		 * Set/get active WFS
		 */
		void setActiveWFS(string wfsSpec);

		/*
		 * Get ADC tracking flag
		 */
		bool getAdcTracking() { return _adcTracking; }
		void setAdcTracking( bool enable) { _adcTracking = enable; }

      WfsArbSystem *getActiveWFS() { return _activeWFS; }

    void doSpeak( string key);   

      // Refreshes AO parameters in RTDB

      void updateRtdb( Wfs_Arbitrator::prepareAcquireRefResult *params);
      void updateRtdb( acquireRefAOResult *params);
      void updateRtdb( checkRefAOResult *params);

      void updateRtdb( presetFlatParams *params);

		RTDBvar	_loopOn;
		RTDBvar	_stopCmd;
		RTDBvar	_clearAOI;

       // Large offset
      void toggleLargeOffset();
      bool largeOffset();

      void speak( string msg);

      void clearStopCmd();
      bool checkStopCmd();

	protected:

      /* Initialize Arbitrator */
      void create();

		/*
		 * Install thrdlib handlers
		 */
		void InstallHandlers();

		/*
		 * Setup the AO vars
		 */
		void SetupVars();

		/*
		 * Perform extra checks and initializations:
		 *  - ---
		 */
		void PostInit();


      /* Function called once per second from main loop.
         Used to update the sub-arbitrtor status.
         */

      void Periodic();

        // state change notification
      static int StateChange(void *, Variable *);


	private: // METHODS

		void TCSNotify(Alert* alert);
        void TCSNotify(string msg, int level = MASTER_LOG_LEV_INFO);
		void processAlert(Alert* alert);

		// Virtual in AbstractArbitrator
		void serviceRequest(Request* req, MsgBuf* requestMsgBuf);

      bool wfsConnected(string wfsSpec);

      int updateStatus();


	private: // FIELDS

		string _FLAOwfsArbMsgdId;
		string _LBTIwfsArbMsgdId;
		string _MAGwfsArbMsgdId;
		string _adSecArbMsgdId;

      bool _adsecWasConnected;
      bool _adsecConnected;

      bool _FLAOwfsConnected;
      bool _MAGwfsConnected;
      bool _LBTIwfsConnected;
     
      bool _largeOffset;
      bool _adcTracking;

		/*
		 * AO system mode (FullAO or SeeingLimited)
		 * For valid values see lib/AOServicesStates.h
		 */
		RTDBvar	_aoMode;
		RTDBvar	_aoServStat;
      RTDBvar _aoLabMode;

      /*
       * AO parameters for mirroring by AOS.
       */

      RTDBvar _aoparamNModes;
      RTDBvar _aoparamStrehl;
      RTDBvar _aoMsg;

//    RTDBvar _adsecMsg;
//    RTDBvar _adsecMsgLev;
      RTDBvar _adsecHealth;

      RTDBvar _wfsSource;

      WfsArbSystem *_activeWFS;
    /*
     * States sync
     */
    RTDBvar *_FLAOwfsStateVar, *_LBTIwfsStateVar, *_MAGwfsStateVar, *_adsecStateVar, *_activeWfsStateVar;
    RTDBvar _fsmSubStateVar, _SLReady, _AOReady;
    vector<string> statesWfs, statesAdsec;
    map<string, AOArbStatesSyncStruct> _statesSyncMap;

};

}
}

#endif /*AOARBITRATOR_H_INCLUDE*/
