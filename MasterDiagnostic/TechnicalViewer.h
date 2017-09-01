#ifndef TECHNICALVIEWER_H_INCLUDE
#define TECHNICALVIEWER_H_INCLUDE

#include "AbstractFramesDownloader.h"
#include "AOApp.h"	// Used to ask RTDB vars notify

extern "C" {
	#include "base/thrdlib.h"
}

#include <map>

using namespace std;

class FrameParameters;

class TechnicalViewer: public AbstractFramesDownloader {

	public:
	
		TechnicalViewer(string bcuName, 					
					    string remoteIp, int remotePort, 
					    int frameCounterOffsetDw,
					    int frameSizeDw,
					    int bitPerPixel,
					    string shmBufProducerName, 
					    string baseShmBufName,		
					    int numFrames = -1);
					    
		virtual ~TechnicalViewer();
		
		/*
		 * Create shm buffers and call base class start
		 */
		pthread_t start();
		
		
		/*
		 * 
		 */
		void changeFrameSize(int frameSizePixels);
		
		/*
		 * Store the frame to shared memory buffer corresponding to the 
		 * current binning (that is frame size)
		 */
		virtual void storeFrame() throw(NotInizializedBufferException);
		
	private:
	
		// Base name of the shm buffer: the real name is made appending 
		//the string "binX_X_binY"
		string _shmBufProducerName;
		string _baseShmBufName;
	
		// "xBinXyBin" -> frame parameters
		map<int, FrameParameters*> _frameParametersByFrameSizePx;
		FrameParameters* _curFrameParameters;
		
		int _frameCounterOffsetDw;
		int _bitPerPixel;
	
};

class FrameParameters {
	public:
	
		FrameParameters(int frameSizePixels, string shmBufProducer, string shmBufBaseName);
		
		~FrameParameters();
		
		int incrShmBufWriteCounter() { _shmBufWriteCounter++; return _shmBufWriteCounter; }
		
		int getPacketsNumber() { 
			int packetsNum = getFrameSizeBytesWithHdrFtr()/_MAX_TDP_PACKET_SIZE;
			if(getFrameSizeBytesWithHdrFtr() % _MAX_TDP_PACKET_SIZE) {
				packetsNum++;
			}
			return packetsNum; 
		}
		
		BufInfo* getShmBufInfo() { return &_shmBufInfo; }

      int _MAX_TDP_PACKET_SIZE;
        
	private:

		int   	_frameSizePixels;
		int 	_shmBufWriteCounter;
		BufInfo _shmBufInfo;

         
		Logger* _logger;
		
		int getFrameSizeBytesWithHdrFtr() { return _frameSizePixels*2 + 32; }	// 1 pixel = 2 bytes
																				// Add 16 bytes for header and 16 bytes for footer
};

#endif /*TECHNICALVIEWER_H_INCLUDE*/
