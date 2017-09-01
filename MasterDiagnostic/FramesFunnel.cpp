
#include "FramesFunnel.h"
#include "MasterDiagnosticExceptions.h"
#include "BcuLib/BcuCommon.h"

using namespace Arcetri;
using namespace Arcetri::Bcu;


FramesFunnel::FramesFunnel(string name, Config_File *conf,
                           BYTE* framesBuffer, int framesBufferCapacity, int fullFrameSizeBytes,
						   StatusTable* statusTable,
						   string shmCreatorName, string shmBufName,
						   uint32 framesHistorySize) {

	_logger = Logger::get(name);

	_name = name;
	_statusTable = statusTable;
	_fullFrameSizeBytes = fullFrameSizeBytes;
	_framesBufferCapacity = framesBufferCapacity;
	_framesBuffer = framesBuffer;

	_shmBufInfo = new BufInfo();
	_shmCreatorName = shmCreatorName;
	_shmBufName = shmBufName;
   _checkFilotto = true;
   _conf  = conf;

	_framesHistory = NULL;
	if(framesHistorySize != 0) {
		_framesHistory = new OFramesHistory(name, framesHistorySize, fullFrameSizeBytes);
	}

	_logger->log(Logger::LOG_LEV_INFO, "Constructing FramesFunnel %s...[%s:%d]", _name.c_str(), __FILE__, __LINE__);
	_logger->log(Logger::LOG_LEV_INFO, "> FramesBufferCapacity: %d", _framesBufferCapacity);
	_logger->log(Logger::LOG_LEV_INFO, "> FullFrameSize:        %d Bytes (including 4 bytes header)", _fullFrameSizeBytes);
	_logger->log(Logger::LOG_LEV_INFO, "> ShmBufName:           %s", (_shmCreatorName + ":" + _shmBufName).c_str());

}

FramesFunnel::~FramesFunnel() {
	bufRelease((char*)_shmCreatorName.c_str(), _shmBufInfo);
	delete _shmBufInfo;
	delete [] _framesBuffer;
	_logger->log(Logger::LOG_LEV_INFO, "FRAMES-FUNNEL %s succesfully destroyed !", _name.c_str());
}


pthread_t FramesFunnel::start() {
	pthread_t framesFunnel;
	pthread_create(&framesFunnel, NULL, &execute, (void*)this);
    return framesFunnel;
}


void* FramesFunnel::execute(void* thisPtr) throw(FramesFunnelException) {

	FramesFunnel* ff = (FramesFunnel*)thisPtr;
	try {
		ff->run();
	}
	catch(FramesFunnelException& e) {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
		throw;	// Rethrow it to stop the application !!!
	}
	return NULL;
}

void FramesFunnel::run() throw(FramesFunnelException) {

    int stat;

	if(IS_ERROR(stat=bufCreate((char*)_shmCreatorName.c_str(), (char*)_shmBufName.c_str(), BUFCONT, _fullFrameSizeBytes, 10, _shmBufInfo))) {
 		_logger->log(Logger::LOG_LEV_WARNING, "FramesFunnel %s: error creating shm buffer %s:%s %d error %d!!!",
                _name.c_str(),(char*)_shmCreatorName.c_str(),(char*)_shmBufName.c_str() ,_fullFrameSizeBytes, stat  );
 		throw FramesFunnelException("FRAME FUNNEL PANIC EXCEPTION", "Impossible to create the shm buffer !!!");
 	}
 	else {
 		_logger->log(Logger::LOG_LEV_INFO, "FramesFunnel %s: Shm buffer %s:%s of size %d bytes succesfully created", _name.c_str(), _shmCreatorName.c_str(), _shmBufName.c_str(), _fullFrameSizeBytes);

 		_logger->log(Logger::LOG_LEV_INFO, "FramesFunnel %s STARTED !", _name.c_str());
		int readyFrame;	// Ready frame slot, ranging from 0 to _framesBufferCapacity
		int shmBufWriteCounter = 0;

		struct timeval start, end;
    	static int loops = 100;
    	double timeSec;

		// --- Infinite loop
		while(true) {

			// Start time
    		if(!(shmBufWriteCounter%loops)) {
    			gettimeofday(&start,NULL);
    		}

			// --- Wait for a full frame notify --- //
			_logger->log(Logger::LOG_LEV_INFO, "");
			_logger->log(Logger::LOG_LEV_INFO, "--------------------------------------------------------------------");
			_logger->log(Logger::LOG_LEV_INFO, "FramesFunnel %s waiting for a frame ready...", _name.c_str());
			readyFrame = _statusTable->waitReadyEntry(); // --> When exit from here, the entry is locked !!!
			_logger->log(Logger::LOG_LEV_INFO, "FramesFunnel %s: notified for the entry %d", _name.c_str(), readyFrame);

			// --- Store the full diagnostic frame to shared memory --- //
			// Check that the K headers match !!!
			if (checkFramesLine(_framesBuffer + readyFrame*_fullFrameSizeBytes + 4)) { // 4 = sizeof(FullDiagnFrameHeader)
            _logger->log(Logger::LOG_LEV_WARNING, "Wrong filotto. Frame skipped.");
         }
         else {

			   if(IS_ERROR(bufWrite(_shmBufInfo, _framesBuffer + readyFrame*_fullFrameSizeBytes, shmBufWriteCounter++, 0))) {
			      _logger->log(Logger::LOG_LEV_WARNING, "FramesFunnel %s: frame [entry %d] NOT stored to shm :-(", _name.c_str(), readyFrame);
			      _logger->log(Logger::LOG_LEV_INFO, "--------------------------------------------------------------------");
			   }
			   else {
			      _logger->log(Logger::LOG_LEV_INFO, "FramesFunnel %s: frame [entry %d] stored to shm, oh yeaaahhh :-) !!!", _name.c_str(), readyFrame);
			      _logger->log(Logger::LOG_LEV_INFO, "--------------------------------------------------------------------");
			      // If history enabled, save the frame
			      if(_framesHistory) {
			         _framesHistory->pushFrame(_framesBuffer + readyFrame*_fullFrameSizeBytes);
			         _logger->log(Logger::LOG_LEV_INFO, "FramesFunnel %s: frame saved on history!", _name.c_str());
			      } 
            }
         }

			// --- Release the entry --- //
			_statusTable->releaseReadyEntry();

			if(!(shmBufWriteCounter%loops)) {
				gettimeofday(&end,NULL);
	   			timeSec = (end.tv_usec-start.tv_usec)/1E6 + (end.tv_sec - start.tv_sec);
	 			_logger->log(Logger::LOG_LEV_INFO, "Frequency: %.2f Hz (%d)", loops/timeSec, shmBufWriteCounter);
			}
		}
 	}
}

void FramesFunnel::setFilottoCheck( bool enable) {
   _checkFilotto = enable;
}

int FramesFunnel::checkFramesLine(BYTE* fullFrameWithoutMasterHeader) throw(FramesFunnelException) {

	if(_name == "ADAPTIVE-SECONDARY") {
		int bcuSwitchFrameSizeBytes = (int)(*_conf)["BcuSwitchFrameSizeDw"] * Constants::DWORD_SIZE;
		int bcuMirrorFrameSizeBytes = (int)(*_conf)["BcuMirrorFrameSizeDw"] * Constants::DWORD_SIZE;

		int res;

		int dwordToCompare = 3;

		// Compare BcuSwitch frame header with BcuMirror_0 frame header
		res = memcmp(fullFrameWithoutMasterHeader, fullFrameWithoutMasterHeader + bcuSwitchFrameSizeBytes, dwordToCompare * Constants::DWORD_SIZE);
		if(res != 0) {
			dumpFrameHeader("BCU_SWITCH", fullFrameWithoutMasterHeader, dwordToCompare);
			dumpFrameHeader("BCU_MIRROR_0", fullFrameWithoutMasterHeader + bcuSwitchFrameSizeBytes, dwordToCompare);
         _logger->log(Logger::LOG_LEV_ERROR, "Detected a wrong filotto (BcuSwitch frame header don't match BcuMirror_0 header) !!!");
         return -1;
		}

		// Compare BcuMirror_i frame header with BcuMirror_i-1 frame header
		for(int i=1; i<AdSecConstants::BcuMirror::N_CRATES; i++) {
			res = memcmp(fullFrameWithoutMasterHeader + bcuSwitchFrameSizeBytes + i*bcuMirrorFrameSizeBytes,
				   		 fullFrameWithoutMasterHeader + bcuSwitchFrameSizeBytes + (i-1)*bcuMirrorFrameSizeBytes,
				   		 dwordToCompare * Constants::DWORD_SIZE);
			if(res != 0) {
				dumpFrameHeader("BCU_MIRROR_" + Utils::itoa(i), fullFrameWithoutMasterHeader + bcuSwitchFrameSizeBytes + i*bcuMirrorFrameSizeBytes, dwordToCompare);
				dumpFrameHeader("BCU_MIRROR_" + Utils::itoa(i-1), fullFrameWithoutMasterHeader + bcuSwitchFrameSizeBytes + (i-1)*bcuMirrorFrameSizeBytes, dwordToCompare);
            _logger->log(Logger::LOG_LEV_ERROR, "Detected a wrong filotto (BcuMirror_" + Utils::itoa(i) + " frame header don't match BcuMirror_" + Utils::itoa(i+1) + " header) !!!");
            return -1;
			}
		}
	}
	else if(_name == "OPTICAL-LOOP") {

      if (!_checkFilotto) {
		   _logger->log(Logger::LOG_LEV_INFO, "Filotto check disabled, frame is considered good.");
         return 0;
      }

      int bcu39FrameSizeBytes    = (int)(*_conf)["Bcu39FrameSizeDw"] * Constants::DWORD_SIZE;

      // Compare WFS frame counter with the same frame counter coming from AdSec

      uint32 WFScounter = ((uint32*)fullFrameWithoutMasterHeader)[0];
      uint32 AdSecCounter = ((uint32*)(fullFrameWithoutMasterHeader + bcu39FrameSizeBytes))[0];

		_logger->log(Logger::LOG_LEV_INFO, "Filotto: %d and %d", WFScounter, AdSecCounter);

      if (WFScounter != AdSecCounter) {
		   _logger->log(Logger::LOG_LEV_INFO, "Rejected wrong filotto between WFS and AdSec");
         return -1;
      }

	}

   return 0;

}

void FramesFunnel::dumpFrameHeader(string frameName, BYTE* frame, int dwordToDump) {
	_logger->log(Logger::LOG_LEV_ERROR, " > FRAME %s HEADER: ", frameName.c_str());
	if(_logger->getLevel() >= Logger::LOG_LEV_ERROR) {
		for(int i=0; i<dwordToDump; i++) {
			printf("%08X ", ((uint32*)frame)[i]);
		}
		printf("\n");
	}
}
