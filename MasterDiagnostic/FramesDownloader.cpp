
#include "FramesDownloader.h"


FramesDownloader::FramesDownloader(string bcuName, string remoteIp, int remotePort, 
								   int frameCounterOffsetDw,
								   int frameSizeDw,
                           int dataStartOffsetDw,
								   StatusTable* statusTable,
								   int numFrames): 
				  AbstractFramesDownloader(bcuName, remoteIp, remotePort, frameSizeDw, numFrames) {

    _conn->setReceiveTimeout_ms(1000);

	
	_frameCounterOffsetDw = frameCounterOffsetDw;
	_diagnFrameBufferMaxCapacity = statusTable->getSize();
	_diagnFrameBufferCurCapacity = 0;
   _dataStartOffsetDw = dataStartOffsetDw;
	_diagnFrameBuffer = new BYTE*[_diagnFrameBufferMaxCapacity];  // Modified by addFrameSlot(...)
	_statusTable = statusTable;

	_logger->log(Logger::LOG_LEV_INFO, "> Frame buffer max capacity: %d", _diagnFrameBufferMaxCapacity);
	_logger->log(Logger::LOG_LEV_INFO, "");

   enable(true);
}

FramesDownloader::~FramesDownloader() {}

void FramesDownloader::enable( bool enable) {
   _enabled = enable;
}

void FramesDownloader::addFrameSlot(BYTE* frameSlot) throw(BufferSlotsOutOfBoundsException) {
	if(_diagnFrameBufferCurCapacity == _diagnFrameBufferMaxCapacity) {
		throw BufferSlotsOutOfBoundsException(_bcuName);
	}
	else {
		_diagnFrameBuffer[_diagnFrameBufferCurCapacity] = frameSlot;
		_diagnFrameBufferCurCapacity++;
		_logger->log(Logger::LOG_LEV_DEBUG, "FramesDownloader %s: frame slot addr:%u added !", _bcuName.c_str(), frameSlot);
	}
}


void FramesDownloader::storeFrame() throw(NotInizializedBufferException) {
	
	if(_diagnFrameBufferCurCapacity == 0) {
			throw NotInizializedBufferException(_bcuName);
	}

   if (!_enabled) {
         _logger->log(Logger::LOG_LEV_INFO, "FramesDownloader %s: enable if OFF, frame discarded", _bcuName.c_str());
         return;
   }
	
	// Extract frameCounter from the frame header using the offset
	// defined in config file
	uint32* currentFrameIdPtr = (uint32*)_frame + _frameCounterOffsetDw;
	uint32 currentFrameId = *currentFrameIdPtr;
	
	bool fullFrameReady;

	// Lock the table for updating: if full frame ready, completely lock the table 
	fullFrameReady = _statusTable->updateBufferEntry(currentFrameId);
	
	// Store the frame
	_logger->log(Logger::LOG_LEV_INFO, "FramesDownloader %s: storing frame with COUNTER %d [to addr:%u, size %d]", _bcuName.c_str(), currentFrameId, _diagnFrameBuffer[currentFrameId%_diagnFrameBufferCurCapacity], _frameSizeBytes);
	memcpy(_diagnFrameBuffer[currentFrameId%_diagnFrameBufferCurCapacity], _frame+ _dataStartOffsetDw*Constants::DWORD_SIZE, _frameSizeBytes - _dataStartOffsetDw*Constants::DWORD_SIZE);

   /*
   printf("%s\n", _bcuName.c_str());
      for (int i=0; i< _frameSizeBytes/4; i++) {
         printf("%08X ", ((uint32*) _frame)[i]);
         if (i%4 == 0)
            printf("\n");
      }
      */

	
	// Notify for full frame ready
	if(fullFrameReady) {
		_statusTable->notifyEntryReady();
	}
	else {
		_statusTable->releaseUpdating();
	}
}
