
#include "TechnicalViewer.h"

TechnicalViewer::TechnicalViewer(string bcuName, string remoteIp, int remotePort, int frameCounterOffsetDw,
					    		 int frameSizeDw, int bitPerPixel, 
					    		 string shmBufProducerName, string baseShmBufName, 
					    		 int numFrames):
				 AbstractFramesDownloader(bcuName, remoteIp, remotePort, frameSizeDw, numFrames) {

   _MAX_TDP_PACKET_SIZE =  Constants::MAX_TDP_PACKET_SIZE;
   /*if(frameSizeDw != 0) {
      changeFrameSize(frameSizeDw);
   }*/
	// This allow to loop for changing the frame size (locked on mutex)
	_conn->setReceiveTimeout_ms(50);

	_frameCounterOffsetDw = frameCounterOffsetDw;
	_bitPerPixel = bitPerPixel;
	_shmBufProducerName = shmBufProducerName;
	_baseShmBufName = baseShmBufName;
}

TechnicalViewer::~TechnicalViewer() {
	_logger->log(Logger::LOG_LEV_INFO, "TechnicalViewer: CLEANING UP...");
	map<int, FrameParameters*>::iterator iter = _frameParametersByFrameSizePx.begin();
	while(iter != _frameParametersByFrameSizePx.end()) {
		delete (*iter).second;
		iter++;
	}
}


pthread_t TechnicalViewer::start() {
	
	return AbstractFramesDownloader::start();
}


void TechnicalViewer::changeFrameSize(int frameSizePixels) {
	
	_logger->log(Logger::LOG_LEV_INFO, "TechnicalViewer: HDLR changing frame size to %d pixels...", frameSizePixels);
	_curFrameParameters = NULL;
	
	int frameSizeDw = (frameSizePixels * _bitPerPixel) / (Constants::DWORD_SIZE*8) + 8; // 4 DW for header and 4 for footer
	AbstractFramesDownloader::changeFrameSize(frameSizeDw);
	
	// Search the correspondiong frame parameters object
	map<int, FrameParameters*>::iterator iter = _frameParametersByFrameSizePx.find(frameSizePixels);
	if(iter == _frameParametersByFrameSizePx.end()) {
		// If not found, insert it
		_logger->log(Logger::LOG_LEV_INFO, "TechnicalViewer: HDRL has added a new shm for frame size %d pixels", frameSizePixels);
		_frameParametersByFrameSizePx[frameSizePixels] = new FrameParameters(frameSizePixels, _shmBufProducerName, _baseShmBufName);
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "TechnicalViewer: HDRL has found existing shm for frame size %d pixels", frameSizePixels);
		
	}
	
	// Assign it to current object
	_curFrameParameters = _frameParametersByFrameSizePx[frameSizePixels];
	_logger->log(Logger::LOG_LEV_INFO, "TechnicalViewer: HDRL set current shm to %s:%s...", _curFrameParameters->getShmBufInfo()->com.producer, _curFrameParameters->getShmBufInfo()->com.name);
}


void TechnicalViewer::storeFrame() throw(NotInizializedBufferException) {
	_logger->log(Logger::LOG_LEV_INFO, "TechnicalViewer storing frame to shm %s:%s...", _curFrameParameters->getShmBufInfo()->com.producer, _curFrameParameters->getShmBufInfo()->com.name);
	
   //std::cout << "_curFrameParameters " << _curFrameParameters << "\n";
   if(_curFrameParameters == 0)
   {
      std::cout << "skipping 0\n";
      return;
   }
   int ret = bufWrite(_curFrameParameters->getShmBufInfo(), _frame, _curFrameParameters->incrShmBufWriteCounter(), 0);
	if(IS_ERROR(ret)) {
		_logger->log(Logger::LOG_LEV_WARNING, "TechnicalViewer: frame NOT stored to shm (error %d)\n", ret);
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "TechnicalViewer:: frame stored to shm !!!\n");
	}
}

FrameParameters::FrameParameters(int frameSizePixels, string shmBufProducer, string shmBufBaseName) {
	
	_frameSizePixels = frameSizePixels;
	_shmBufWriteCounter = -1;

   _MAX_TDP_PACKET_SIZE = Constants::MAX_TDP_PACKET_SIZE;
   
	_logger = Logger::get("TECHVIEWER");
	_logger->log(Logger::LOG_LEV_DEBUG, "Creating FrameParameters for frame of size %d pixels, %d bytes (with hdr and ftr)...", _frameSizePixels, getFrameSizeBytesWithHdrFtr());
	
	string shmBufName = shmBufBaseName + Utils::itoa(_frameSizePixels);
	
	int ret = bufCreate((char*)shmBufProducer.c_str(), (char*)shmBufName.c_str() , BUFCONT, getFrameSizeBytesWithHdrFtr(), 10, &_shmBufInfo);
	if(IS_ERROR(ret)) {
		if(ret != BUF_DUPL_NAME_ERROR) {
			_logger->log(Logger::LOG_LEV_WARNING, "TechnicalViewer: error creating %s:%s shm buffer (err %d) !!!", _shmBufInfo.com.producer, _shmBufInfo.com.name, ret);
			throw MasterDiagnosticException("TechnicalViewer:ShmBufferCreationException", "Impossible to create the shm buffer");
		}
		else {
			_logger->log(Logger::LOG_LEV_WARNING, "TechnicalViewer: shm buffer %s:%s already exists !", _shmBufInfo.com.producer, _shmBufInfo.com.name);
		}
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "TechnicalViewer: created shm buffer %s:%s", _shmBufInfo.com.producer, _shmBufInfo.com.name);
 	}		
}

FrameParameters::~FrameParameters() {
	_logger->log(Logger::LOG_LEV_INFO, "TechnicalViewer: destroying frame parameters for frame size %d...", _frameSizePixels);  
	int ret = bufRelease(_shmBufInfo.com.producer, &_shmBufInfo);
	if(!IS_ERROR(ret)) {
		_logger->log(Logger::LOG_LEV_INFO, "TechnicalViewer: destroyed shm buffer %s:%s", _shmBufInfo.com.producer, _shmBufInfo.com.name);
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "TechnicalViewer: error destroying shm buffer %s:%s", _shmBufInfo.com.producer, _shmBufInfo.com.name);
	}
}
