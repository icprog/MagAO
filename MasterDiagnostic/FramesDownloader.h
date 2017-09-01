#ifndef FRAMESDOWNLOADER_H_INCLUDE
#define FRAMESDOWNLOADER_H_INCLUDE


#include "AbstractFramesDownloader.h"
#include "StatusTable.h"

using namespace Arcetri;
using namespace Arcetri::Bcu;


/*
 * Frame downloader for Bcu39 and BcuMirror_i
 * 
 * Store the frame in a single buffer. The buffer is initialized
 * using addFrameSlot(BYTE* frameSlot).
 */
class FramesDownloader: public AbstractFramesDownloader {
	
	public:
	
		/*
		 * Create a basic frame downloader.
		 */
		FramesDownloader(string bcuName, 					
								  string remoteIp, int remotePort, 
								  int frameCounterOffsetDw,
								  int diagnFrameSizeDw, 
                          int dataStartOffsetDw,   
								  StatusTable* statusTable,		// Contains info about the frames buffer,
								  								// including its max capacity
								  int numFrames = -1);
		
		virtual ~FramesDownloader();
		
		/*
		 * Add an available slot to the frames buffer: the slot must refer to
		 * a valid memory location of size "diagnFrameSizeDw" (see constructor).
		 * 
		 * Throw a FramesDownloaderException if the number of slots exceed the 
		 * frames buffer max capacity.
		 */
		void addFrameSlot(BYTE* frameSlot) throw(BufferSlotsOutOfBoundsException);

      void enable( bool enable);
		
	
	private:
		
		virtual void storeFrame() throw(NotInizializedBufferException);
		
		// Info needed to store the frame in the buffer
		int    _frameCounterOffsetDw;
      int    _dataStartOffsetDw;
		BYTE** _diagnFrameBuffer;
		int	   _diagnFrameBufferMaxCapacity;
		int	   _diagnFrameBufferCurCapacity;

      bool _enabled;
		
		StatusTable* _statusTable;
};

#endif
