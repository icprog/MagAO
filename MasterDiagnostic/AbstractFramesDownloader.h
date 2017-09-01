#ifndef ABSTRACTFRAMEDOWNLOADER_H_
#define ABSTRACTFRAMEDOWNLOADER_H_

#include "BcuLib/BcuCommon.h"
#include "UdpConnection.h"
#include "aotypes.h"
#include "Logger.h"

#include "MasterDiagnosticExceptions.h"

using namespace Arcetri;
using namespace Arcetri::Bcu;

/*
 * This abstract class defines the main features of a thread wich
 * downloads a dignostic frames from a single Bcu.
 * 
 * This is an abstract class because the derived subclass must
 * implement the storeFrame(...) method, to save the frames somewhere.
 */
class AbstractFramesDownloader {
	
	public:
		AbstractFramesDownloader(string bcuName, 		// Name of the Bcu (only for logging)
										  string remoteIp, 		// Bcu ip address
										  int remotePort, 		// Bcu ip port
								  		  int frameSizeDw = 0,	// Size of the diagnostic frame
								  		  int numFrames = -1);	// Number of frames to download: -1 means
								  								// "download forever"
								  		 
		virtual ~AbstractFramesDownloader();
		
		pthread_t start();
		
		/*
		 * Change the size of the frame to download.
		 */
		virtual void changeFrameSize(int frameSizeDw);
	
	protected:
      //A local variable so that we can switch to jumbo frames.  
	   int _MAX_TDP_PACKET_SIZE;
      
		static void* execute(void* thisPtr);
   public:
		void run();
		
		int computePacketSize(int packetBytesToDownload);
		
		
		/*
		 * Must be implemented by the derived class.
		 */
		virtual void storeFrame()throw(NotInizializedBufferException) = 0;
	
		/*
		 * Check if a received packet is the expected one
		 */
		void checkPacket(int expectedFrameId, int currentFrameId, int expectedPacketId, int currentPacketId) throw(InvalidDiagnosticPacketException);
		
		/*
		 * Check (weakly) if a fully received frame is "complete", that is the
		 * header and footer match.
		 */
		void checkFrame() throw(InvalidDiagnosticFrameException);
	
		/*
		 * Dump a packet header
		 */
		void dumpPacketHeader(DiagnosticUdpHeader* header);
		
		/*
		 * Dump a packet header
		 */
		void dumpFrameHeader();
	
	protected: // These field need to derived subclass to store the frame
	
		string _bcuName;
		
		UdpConnection* _conn;
	
		BYTE* _frameHeader;
		BYTE* _frameFooter;
		BYTE* _frame;		// Used to temporarely store the frame before checking its id
							// and save it in the correct buffer position (by storeFrame(...) method)
		int   _frameSizeBytes;
		
		Logger* _logger;
	
		
	private:
		
		bool			_frameSizeChanging;
		pthread_mutex_t _frameSizeChanged_mutex;
	
		
		
		static const int   _FRAME_HEADER_SIZE_DW = 4;
		
		int	  _packetsPerFrame;		// Number of packet for each frame: computed from frameSize
		int   _numFrames;			// Number of frames to download (-1 means "download forever") - UNUSED !!!
		
};

#endif /*ABSTRACTFRAMEDOWNLOADER_H_*/
