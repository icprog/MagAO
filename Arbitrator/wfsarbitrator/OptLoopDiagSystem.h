#ifndef OPTLOOPDIAGSYSTEM_H_INCLUDE
#define OPTLOOPDIAGSYSTEM_H_INCLUDE

#include "framework/AbstractSystem.h"
#include "arblib/wfsArb/WfsInterfaceDefines.h"


using namespace Arcetri::Arbitrator;

namespace Arcetri{
namespace Wfs_Arbitrator {
	
class OptLoopDiagSystem: public AbstractSystem {
	
	public:
	
		OptLoopDiagSystem();
		
		virtual ~OptLoopDiagSystem();
		
		AbstractSystemCommandResult save( saveOptLoopDataParams params, bool join=true, string senderClient="");
      void join();
      void enableAntiDrift( bool enable=true);
      bool isAntiDriftEnabled();

      static int optsavereply_handler(MsgBuf *msgb, void *argp, int hndlrQueueSize);

		void cancelCommand();

      string getTrackingNum();
      string getFilename();

   protected:

      string _target;
      AbstractArbitrator* _arbitrator;

      pthread_mutex_t  _replyMutex;
      pthread_mutex_t  _firstReplyMutex;
      pthread_mutex_t  _dataMutex;

   public:
      int _framesToSave;
      string _tracking;
      string _filename;
      bool _saving;
      string _senderClient;
      pthread_cond_t  _replyCond;
      pthread_cond_t  _firstReplyCond;

};

}
}

#endif /*OPTLOOPDIAGSYSTEM_H_INCLUDE*/
