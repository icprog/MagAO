#ifndef ADSECARBITRATOR_H_INCLUDE
#define ADSECARBITRATOR_H_INCLUDE

#include "../framework/AbstractArbitrator.h"
#include "arblib/base/ArbitratorInterface.h"
#include "AdamLib.h"

#define SETTSS_TIMEOUT      5000   // Timeout for setTSS command (ms)
#define SETPISTCURR_TIMEOUT 5000   // Timeout for setPistonCurrent command (ms)
#define EXPIRE_TIME         10     // Lifetime for a variable

#define ADSECARBITRATOR_LEDSTATEMAP_STR   "LedState"

using namespace Arcetri;
using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace AdSec_Arbitrator {

enum ElevMode { ElevSet, ElevRest };
enum ElevState { ElevationNotAvailable, ElevationBelowThreshold, ElevationOk };
enum SWAState  { SwaNotAvailable, SwaNotDeployed, SwaDeployed };

class AdSecArbitrator: public AbstractArbitrator {

	public:

		AdSecArbitrator(string config);

		AdSecArbitrator(int argc, char* argv[]);

		virtual ~AdSecArbitrator();

      static int adsecStateChanged( string state);

      string fsmState();

      bool checkTTOffload();

      bool checkLabMode();

      ElevState checkElevation(ElevMode mode);

      SWAState  checkSWA();

      double getElevation();
      bool getCoilsEnabled();
      void setCoilsEnabled( bool enabled);
      bool getTSSEnabled();

      /* 
       * References to wind and elevation variables
       */
      RTDBvar *& varDoOffloadHighOrder() { return _varDoOffloadHighOrder; }
      RTDBvar *& varFocalStation() { return _varFocalStation; }

      /* Status of LO offload request */
      bool   _LOOffloadReq;

      /* Read flat listing and fill the corresponding RTDB variable (for AOS display) */
      void refreshFlatList();

      /* Return whether the AOS telescope guiding variable is active */
      bool isGuiding();


	protected:

		/*
		 * Setup the AdSec vars.
		 */
		void SetupVars();

		/*
		 * Perform extra checks and initializations:
		 * 	-
		 */
		void PostInit();

      /* Static variable handlers */
      static int adsecStateHandler( void *pt, Variable *msgb);

      /*
       * Called by the handler of messages with code ARB_REQ in case
       * the request can't be satisfied by base class
       */
      void serviceRequest(Request* /*req*/, MsgBuf* /*requestMsgBuf*/);

      Command *lastCommand() { return AbstractArbitrator::lastCommand() ; }

      void Periodic();

      /* Handlers for wind speed and elevation variables
       */

      static int doOffloadHighOrderHandler( void *pt, Variable *var);
      static int demoModeHandler( void *pt, Variable *var);

	private: // METHODS
      void create();
      void controlShellSafety();
      void processAlert(Alert* alert);
//    void controlPistonCurrent(bool forceSet=false);
//    void controlTSS(bool forceSet=false);
      void enableTSS(bool enable, bool force=false);
      void setPistonCurrent(double elevation);
      void putToRest(string reason);


      RTDBvar *_varAdsecState;
      int _safeSkipCounter;
      float _safeSkipPercent;
      bool _skipping;
      int  _anem_AOS;
      RTDBvar *_varWind;
      RTDBvar *_varElev;
      RTDBvar *_varSWA;
      RTDBvar *_varGuiding;
      RTDBvar *_varLabmode;
      RTDBvar *_varDoOffloadHighOrder;
      RTDBvar *_varFocalStation;
      RTDBvar *_varCoilsAdam;
      RTDBvar *_varHOOffloadTime;
      RTDBvar *_varTSSEnabled;
      double _elevation;
      int    _last_elevation_step;
      bool   _tss_enabled;
      double _upperWindSpeed;
      double _lowerWindSpeed;
      double _minElevationRest;
      double _minElevationSet;
    //Adam*  _adam;
      bool   _labmode;
      bool   _coilsEnabled;

      static const int   HIGHORDEROFFLOAD_TIMEOUT = 5000;
      static const int   PROCESSDUMP_TIMEOUT = 20000;

      // Led state map

      map<string, int> _ledStateMap;


public:
      RTDBvar *_varLed;
      RTDBvar *_varSafeSkipPercent;
      RTDBvar *_varTTOffloadEnable;
};

}
}

#endif /*ADSECARBITRATOR_H_INCLUDE*/
