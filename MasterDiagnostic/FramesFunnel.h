#ifndef FRAMESFUNNEL_H_INCLUDED
#define FRAMESFUNNEL_H_INCLUDED

#include "MasterDiagnosticExceptions.h"
#include "StatusTable.h"
#include "Logger.h"
#include "stdconfig.h"
#include "mastdiagnlib.h"

extern "C" {
	#include "base/thrdlib.h"
}

using namespace Arcetri;

#include <string>
#include <pthread.h>
using namespace std;


/*
 * This thread gather frames for a single diagnostic and store them
 * to a shared memory
 */
class FramesFunnel {

	public:
			
		explicit FramesFunnel(string name,			
                       Config_File *conf,       // MasterDiagnostic conf file
							  BYTE* framesBuffer, 		 // The buffer where frames are stored		
							  int framesBufferCapacity,  // The frames capacity of the buffer
							  int fullFrameSizeBytes, 	 // Size in BYTES of the full diagnostic frame (including full frame header)
							  StatusTable* statusTable,	 // Table storing the status of the buffer
							  string shmCreatorName, 	 // Name of the shm creator
							  string shmBufName,		 // Name of the shm buffer	
							  uint32 framesHistorySize); // Number of frames to save in history		
		
		virtual ~FramesFunnel();
		
		/*
		 * Start the thread.
		 */
		pthread_t start();
		
		/*
		 * Dump the frames history to file
		 */
		string dumpHistory() {
			string dumpFilename = "";
			if(_framesHistory) {
				dumpFilename = _framesHistory->dumpToFile();
			}
			return dumpFilename;
		}

      void setFilottoCheck(bool enable);     // Enable/disable the filotto check
		
		
	private:
	
		static void* execute(void* thisPtr) throw(FramesFunnelException);
		void run() throw(FramesFunnelException);
		
		int checkFramesLine(BYTE* fullFrameWithoutMasterHeader) throw(FramesFunnelException);
		void dumpFrameHeader(string frameName, BYTE* frame, int dwordToDump);
		
	private:
		
		string _name;
		
		string _shmCreatorName;
		string _shmBufName;
	
		StatusTable* _statusTable;
		
		int   _fullFrameSizeBytes;	// Including full frame header
		uint8 _framesBufferCapacity;
		BYTE* _framesBuffer;
		
		BufInfo* _shmBufInfo;
	
		Logger* _logger;
      Config_File *_conf;
      
      bool _checkFilotto;
		
	private:	// Status related to the frames history and its streming to disk
			
		OFramesHistory* _framesHistory;
		
		
};

#endif /*FRAMESFUNNEL_H_INCLUDED*/
