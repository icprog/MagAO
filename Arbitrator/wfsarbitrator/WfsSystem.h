#ifndef WFSSYSTEM_H_INCLUDE
#define WFSSYSTEM_H_INCLUDE

#include "framework/AbstractSystem.h"
#include "WfsInterface.h"

using namespace Arcetri::Arbitrator;

namespace Arcetri{
namespace Wfs_Arbitrator {

class WfsSystem: public AbstractSystem {

	public:

		WfsSystem();

		virtual ~WfsSystem();

		/*
		 * If 'config' is "LAST", use the previously saved config (may be the default one!)
		 */
		void operate( powerOnParams params);
		void off();
		void prepareAcquireRef(prepareAcquireRefParams params, prepareAcquireRefResult *outputParams);
		void acquireRef( acquireRefResult *outputParams);
		void modifyAO(modifyAOparams params, modifyAOparams *result);
		AbstractSystemCommandResult closeLoop();
		void refineLoop(wfsLoopParams wfsLoopPar);
		void pauseLoop();
		void resumeLoop();
      void offsetXY( offsetXYparams params);
      void offsetZ( offsetZparams params);
      void optimizeGain();
      void correctModes( correctModesParams params);
      void emergencyOff();

      void checkRef( checkRefResult *params);

		AbstractSystemCommandResult stopLoop();

		void saveStatus( saveStatusParams params);
      void savePsf( string filename, int nFrames);
      void saveIrtc( string filename, int nFrames);
      void savePisces( string filename, int nFrames);

      int getDecimation();
      float getLoopFreq();
      int getBinning();
      float getModulation();
      string getSourceName();
      float getSourceMag();
      float getRangeMin();
      float getRangeMax();

      void setInstrument( string instrument);

		// Asynchronous commands
		void setHoArmPolicy(SetHoArmPolicy hoArmPolicy);
		void setTvArmPolicy(SetTvArmPolicy tvArmPolicy);
		void setPointAndSourcePolicy(PointAndSourcePolicy pointAndSourcePolicy);

      // Calibration commands
      void setSource( setSourceParams params);
      void calibrateHODark( int nframes);
      void calibrateTVDark( int nframes);
      void calibrateIRTCDark( int nframes);
      void calibratePISCESDark( int nframes);
      void calibrateMovements();
      void calibrateSlopenull( int nframes);

      void getTVSnap( snapParams *params);

      void enableDisturb( enableDisturbParams params);


	private:

		// Used to notify alerts
		AbstractArbitrator* _arbitrator;

		// Used to control Wfs subsystem
		WfsInterface _wfsInterface;

		// Operation mode set in "Operate" command.
		// This mode is used when "Operate" is called without
		// parameter (i.e. when recovering from a failure)
		static string OP_MODE_DEFAULT_CONFIG;
		string _opModeConfig;

};

}
}

#endif /*WFSSYSTEM_H_INCLUDE*/
