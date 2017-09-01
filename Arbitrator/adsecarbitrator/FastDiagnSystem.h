//@File: FastDiagnSystem.h
//
// Definitions for Fast Diagn System
//@

#ifndef FASTDIAGNSYSTEM_H_INCLUDE
#define FASTDIAGNSYSTEM_H_INCLUDE

#include "framework/AbstractSystem.h"


#define FASTDIAGNSYSTEM_PROCESS "fastdiagn"
#define FASTDIAGNSYSTEM_BACKUP_PARAMS "fastdiagn/fastdiagn_backup.param"
#define FASTDIAGNSYSTEM_RIPPING_PARAMS "fastdiagn/fastdiagn_ripping.param"
#define DIAGNSYSTEM_TIMEOUT (30000)

enum {
    FASTDIAGNSYSTEM_PREVIOUS_SET = -1,
    FASTDIAGNSYSTEM_RIPPING_SET = 0
};


using namespace Arcetri::Arbitrator;

namespace Arcetri{
namespace AdSec_Arbitrator {
	
class FastDiagnSystem: public AbstractSystem {
	
	public:
	
		FastDiagnSystem(bool simulation=false);
		
		virtual ~FastDiagnSystem();
		
		void startObs();
		void terminateObs();
		
		void cancelCommand();

      int getSafeSkipCounter();
      float getSafeSkipPercent();

      // Offload request
      bool isTTOffloadEnabled();

      // Offload start/stop
      void startTTOffload();
      void stopTTOffload();

      RTDBvar *& varOffload() { return _varOffload; }
      double getTipOffload();
      double getTiltOffload();
      double getFocusOffload();

      // Variable handler
      static int offloadHandler( void *pt, Variable *msgb);


      // thresholds configuration change
      int setDiagThresholdsSet(int set);


   private:
      bool _simulation;     // Simulation mode flag

   protected:

      void setTTOffload( bool enable);

      string _target;
      AbstractArbitrator* _arbitrator;
      bool _offloadEnabled;
      RTDBvar *_varOffload;

};

}
}

#endif /*FASTDIAGNSYSTEM_H_INCLUDE*/
