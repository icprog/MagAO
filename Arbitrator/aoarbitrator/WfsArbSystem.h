#ifndef WFSSYSTEM_H_INCLUDE
#define WFSSYSTEM_H_INCLUDE

#include "framework/AbstractSystem.h"
#include "arblib/base/Alerts.h"

#include "arblib/base/ArbitratorInterface.h"
#include "arblib/wfsArb/WfsInterfaceDefines.h"
#include "arblib/aoArb/AOPTypes.h"

using namespace Arcetri::Arbitrator;
using namespace Arcetri::Wfs_Arbitrator;

namespace Arcetri{
namespace Arbitrator {

    typedef enum {
       JustConnected,
       IsConnected,
       JustDisconnected,
       IsDisconnected,
    }                   ConnectionStatus;


class WfsArbSystem: public AbstractSystem {

	public:

      WfsArbSystem();           // Default constructor

		WfsArbSystem(string sysName, bool simulation=false); // @P{sysName}: WFS specification

		virtual ~WfsArbSystem();

      void setArbInterface(ArbitratorInterface * intf);

      bool sync();

		void powerOn();
		void powerOff();
		void presetAO( Wfs_Arbitrator::prepareAcquireRefParams *params,  Wfs_Arbitrator::prepareAcquireRefResult **result);
		void acquireRefAO( acquireRefAOResult *result);
		void checkRefAO( checkRefAOResult *result);
		void correctModes( correctModesParams params);
		void offsetXY( offsetXYParams params, bool brake=true);
		void offsetZ( offsetZParams params);
		void optimizeGain();
		void pause();
		void modifyAO( Arcetri::Arbitrator::modifyAOParams *params, acquireRefAOResult *result);
		void resume();
		void startAO(bool nocheck=false);
		void stop();
        void autoTrack( Wfs_Arbitrator::autoTrackParams params);

		void userPanic();
      void backToOperate();
      void recoverFailure();
      bool isPowerOn() { return _powerOn; }
      string fsmState() { return _fsmState; }

      // Seeing limited support
      bool enableDisturb( bool wfs, bool ovs);

//    string getStatus() { return _status; }
      bool getClStatReady() { return _clStatReady; }
      bool getLensTrackingTarget() { return _lensTrackingTarget; }

      string getMsgDIdent() { return _msgdId; }

      string getDomain() { return _msgdDomain; }

      void getArbStartTime(struct timeval &time) { time = _arbStartTime; };

      bool FeedbackRequest();

      // Miscellaneous

      ConnectionStatus updateConnectionStatus();

	private:

      Command *simpleCommand( Command *cmd);

      struct timeval _arbStartTime;

		AbstractArbitrator* _arbitrator;

      bool _simulation;
//    string _status;
      string _fsmState;
      bool _powerOn;
      bool _clStatReady;
      bool _lensTrackingTarget;

      string _msgdId;
      string _msgdDomain;
      string _ident;

      RTDBvar _wfsArbHealth;

      bool _wasConnected;

		// Used to control Wfs subsystem
		ArbitratorInterface* _wfsArbIntf;
};

}
}

#endif /*WFSSYSTEM_H_INCLUDE*/
