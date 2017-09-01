#ifndef FRAMESBOUNCER_H_INCLUDED
#define FRAMESBOUNCER_H_INCLUDED

#include "MasterDiagnosticExceptions.h"
#include "StatusTable.h"
#include "Logger.h"
#include "mastdiagnlib.h"
#include "AdsecDiagnLib.h"

extern "C" {
	#include "base/thrdlib.h"
   #include "sys/types.h"
   #include "sys/socket.h"
   #include "netinet/in.h"
}

using namespace Arcetri;

#include <string>
#include <pthread.h>
using namespace std;


/*
 * This thread gather waits for new data added to the shared memory buffer
 * and sends them over the network to a specified host.
 */
class FramesBouncer {

	public:

      FramesBouncer( string name,             // Name of bouncer object (for logs)
                     string MyName,           // AOApp client name
                     string bufname);         // Complete name of the shm buffer

      virtual ~FramesBouncer();
		
      /*
       * Set or modify the destination address
       */
      int setDestination( string host, int port) throw (FramesBouncerException);

		/*
		 * Start the thread.
		 */
		pthread_t start();

      /*
       * Set the bouncer decimation.
       */
      int setDecimation( int decimation);
		
	private:
	
		static void* execute(void* thisPtr) throw(FramesBouncerException);
		void run() throw(FramesBouncerException);
      void die();
		
	private:

      void sbobinaFrame( AdsecDiagnRaw *mirrorDataBuf, MirrorOutputDiagn *outputDataBuf) throw(FramesBouncerException);
      void sendFrame( MirrorOutputDiagn *outputDataBuf);

      string _name;   
		string _MyName;
		string _bufname;

      int _decimation;

      AdsecDiagnRaw mirrorDataBuf;
      MirrorOutputDiagn outputDataBuf;
		
		BufInfo* _shmBufInfo;
		Logger* _logger;
      bool _timeToDie;

      struct sockaddr_in _sa;
      int _socket;
};

#endif /*FRAMESBOUNCER_H_INCLUDED*/
