#ifndef ADSECSYSTEM_H_INCLUDE
#define ADSECSYSTEM_H_INCLUDE

#include "framework/AbstractSystem.h"
#include "arblib/base/Alerts.h"

#include "arblib/base/ArbitratorInterface.h"
#include "arblib/adSecArb/AdSecPTypes.h"

// using namespace Arcetri::Arbitrator;

namespace Arcetri{
namespace Arbitrator {

class AdsecArbSystem: public AbstractSystem {

	public:

		AdsecArbSystem(bool simulation=false);

		virtual ~AdsecArbSystem();

      void setArbInterface(ArbitratorInterface * intf);

      bool sync();

		void powerOn();
		void powerOff();
		void mirrorSet();
		void mirrorRest();
		void presetFlat(string flatSpec);
		void presetAO( float gain, string m2c, string rec, string filtering, float freq, int decimation);
		void presetAO( string gain, string m2c, string rec, string filtering, float freq, int decimation);
		void checkRefAO();
		void setZernikes(setZernikesParams params);
		void modifyAO();
		void offsetXY();
		void offsetZ();
		void pause();
		void refineAO();
		void resume();
		void startAO();
		void stop(bool hold=false);
		void userPanic();
      void recoverFailure();
      void enableTTOffload( bool enable);
      void selectFocalStation( string focalStation);

      bool isPowerOn() { return _powerOn; }
      bool isSet() { return _mirrorSet; }

      // Seeing-limited simulation support
      bool setDisturbance( string filename);
      bool setGain( float gain);
      bool setGain( string filename);
      bool set10modesGain();

      string getMsgDIdent() { return _msgdName; }
      string getStatus() { return _adSecStatus; }
      bool getClStatReady() { return _clStatReady; }
      double getTipOffload() { return _tipOffload; }
      double getTiltOffload() { return _tiltOffload; }
      double getFocusOffload() { return _focusOffload; }
      double getSafeSkipPercent() { return _safeSkipPercent; }

      void getArbStartTime(struct timeval &time) { time = _arbStartTime; };

      bool FeedbackRequest();

      void reduceGain();        // Reduce gain before offset
      void restoreGain();       // Restore previous gain
      void zeroGain();          // Zero gain


      int optimizeGain(double start_val, double range, double ttratio, int steps, int samples);

      void setAcquireRefResult( acquireRefAOResult &result);
      acquireRefAOResult& getAcquireRefResult();


	private:

      bool _simulation;


      int _decimation;
      float _freq;
      float _ovsFreq;

      bool _powerOn;
      bool _mirrorSet;
      string _adSecStatus;
      bool _clStatReady;
      double _tipOffload;
      double _tiltOffload;
      double _focusOffload;
      double _safeSkipPercent;

      struct timeval _arbStartTime;

      RTDBvar _adsecCurrGain;
      static  string _adsecLastGainFilename;

      string _msgdName;
      AbstractArbitrator* _arbitrator;

      // Used to control AdSec subsystem
      ArbitratorInterface* _adSecArbIntf;

      bool simpleCommand( Command *cmd);

      int getModesDiff(double *rms);
      int getModesDiffRms(double *rms, int samples);
      double getModesDiffRmsPart(int order1, int order2, int samples);
      int sweepGain(double gainbase[], double start, double end, double ttratio, int steps, int order1, int order2, int samples, double results[], double gainbest[], int &stepbest, double &peaktopeak);

      // Remember AcquireRefAO result
      acquireRefAOResult result;

};

}
}

#endif /*ADSECSYSTEM_H_INCLUDE*/
