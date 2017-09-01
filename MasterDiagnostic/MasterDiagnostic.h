#ifndef MASTERDIAGNOSTIC_H_INCLUDE
#define MASTERDIAGNOSTIC_H_INCLUDE

#include "AOApp.h"

#include "FramesFunnel.h"
#include "FramesDownloader.h"
#include "TechnicalViewer.h"
#include "FramesBouncer.h"

/*
 *
 */
class MasterDiagnostic: public AOApp {

	public:

		MasterDiagnostic(int argc, char **argv) throw(MasterDiagnosticException);
		
		virtual ~MasterDiagnostic();
		
		virtual void SetupVars();
		
		virtual void InstallHandlers();
		
		TechnicalViewer* getTechViewer() { return _techViewer; }
		
		/*
		 * Handler notified for open/close loop var change.
		 */
		static int openCloseLoopHdlr(void *mdPtr, Variable *var);
		
		
		/*
		 * Handler notified for tech viewer frame size var change.
		 */
		static int techViewFrameSizeChangedHdlr(void *thisPtr, Variable *var);
		
		/*
		 * Handler notified when a MASTDIAGN_DUMP_ADSEC_FRAMES message arrive
		 */
		static int dumpAdSecHistoryHdlr(MsgBuf *msgb, void *argp, int hndlrQueueSize);
		
		/*
		 * Handler notified when a MASTDIAGN_DUMP_OPTLOOP_FRAMES message arrive
		 */
		static int dumpOptLoopHistoryHdlr(MsgBuf *msgb, void *argp, int hndlrQueueSize);
		
		/*
		 * Handler notified when a MASTDIAGN_SET_DECIMATION message arrive
		 */
		static int decimationHdlr(MsgBuf *msgb, void *argp, int hndlrQueueSize);
		
		/*
		 * Handler notified when a MASTDIAGN_SET_BOUNCER_ADDR message arrive
		 */
		static int bouncerAddrHdlr(MsgBuf *msgb, void *argp, int hndlrQueueSize);
		
		
	private:
	
		uint8 _optLoopDiagnFramesNum;	// Depends on loop open (1) or close (2) !!!
		uint8 _adSecDiagnFramesNum;		// Depends only on AdSecConstants
	
		// Info about the Optical Loop diagnostic
		bool  _optLoopDiagnEnabled;			// GOT FROM CONFIG FILE	
		int   _optLoopDiagnBufferCapacity;	
		int** _optLoopDiagnBufferHeaders;	// Size 32 bits, obtained from the previous buffer allocated in constructor
		StatusTable* _statusTableOptLoop;
		FramesFunnel* _optLoopDignFunnel;
		
		// Info about the Adaptive Secondary diagnostic
		bool  _adSecDiagnEnabled;			// GOT FROM CONFIG FILE
		bool  _adSecBouncerEnabled;		// GOT FROM CONFIG FILE
		int   _adSecDiagnBufferCapacity;
		int** _adSecDiagnBufferHeaders;		// Size 32 bits, obtained from the diagnostic buffer allocated in constructor
		StatusTable* _statusTableAdSec;
		FramesFunnel* _adSecDignFunnel;
      FramesBouncer *_adSecBouncer;
		
		// Info about the technical Viewer
		bool  _techViewerEnabled;			// GOT FROM CONFIG FILE
		
		// Frames downloader: 1 for Bcu39, 1 for BcuSwitch, N for BcuMirror_i, 1 for the adsec output stream
		FramesDownloader*  	    _bcu39FramesDownloader;
		FramesDownloader*        _bcuSwitchFramesDownloader;
		FramesDownloader** 		_bcuMirrorFramesDownloaders;
		FramesDownloader* 		_adsecStreamFramesDownloader;
		
		// Technical viewer (derives from AsbstractFrameDownloader)
		TechnicalViewer*	_techViewer;
		
	private:

		void create() throw(MasterDiagnosticException);
	
		// Vars for loop close/open
		// [todo] Should be moved to OpticalLoop FramesFunnel
		RTDBvar _closeLoopReq;
		RTDBvar _closeLoopCur;
		
		// Vars for changing technical viewer frame size
		bool 	_techViewRuntimeFrameChangeEnabled;
		RTDBvar _techViewerFrameSize;
		
		void setupCloseLoopVars();
		
		void setupTechViewerVars();
		
	
		/*
		 * Initialize the header of the full diagnostic frame for the Optical
		 * Loop diagnostic depending on _optLoopDiagnFramesNum
		 */
		void initOptLoopFrameBufferHeaders();
		
		/*
		 * Initialize the header of the full diagnostic frame for the Adaptive
		 * Secondary diagnostic depending on _adSecDiagnFramesNum
		 */
		void initAdSecFrameBufferHeaders();
	
		/*
		 * Set the Optical Loop diagnostic to work in close-loop mode.
		 * This doesn't affect the Adaptive Secondary diagnostic.
		 */
		void setCloseLoop(bool closeLoop);	

		/*
		 * Change the current tech viewer frame size
		 */
		void techViewChangeFrameSize(Variable* var);
		
	
		virtual void Run();

};

struct FullDiagnFrameHeader {
	uint32 validFrames;		// Bitmask giving the  valid frames in the full diagnostic frame 
};

#endif
