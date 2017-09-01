#ifndef DIAGNOSTIC_MANAGER_H_INCLUDED
#define DIAGNOSTIC_MANAGER_H_INCLUDED

#include <string> 
using namespace std;


#include "Commons/AbstractBcuRequestManager.h"

#include "stdconfig.h"
using namespace Arcetri;


/*
 * Thread performing the download of a diagnostic.
 * 
 * A diagnostic is defined by a diagnostic config-file, and consists of
 * an indefinite stream of frames downloaded by a pool of BCUs.
 * The DiagnosticManager provide a buffer for 1 frame, and copy it in the 
 * shm buffer when the full frame is ready.
 * 
 * Each DiagnosticManager belongs to a "group", defined by a groupId.
 * The groups corresponds to different diagnostic, and
 * each group can have only a 
 * 
 * The shm buffer has name [MirrorCtrlName]:[ShmName] where:
 * 	- [MirrorCtrlName] is defined in MirrorCtrl config file
 * 	- [ShmName] is defined in the specific diagnosti config file
 * 
 * The BcuRequest used to read a diagnostic frame is always the same, except
 * for the dataAddress field: for each frame the dataAddress is obtained
 * from the current value of the frame counter.
 */
class DiagnosticManager: public AbstractBcuRequestManager {

    public:
    
    	/*
    	 * Constructor.
    	 * 
    	 * The objId is an ***unique*** integer: different instances must have 
    	 * differents ids (only used to create and get the logger !)
    	 */
        DiagnosticManager(string mirrorCtrlName, string configFile, int id, int priority, int logLevel) throw (DiagnosticManagerException);
        
        virtual ~DiagnosticManager(); 
        
        /*
         * Check if the diagnostic manager is started or not
         */
        bool isStarted() { return _started; }
        
        // Start/stop the diagnostic in a separate thread
        pthread_t start(); 
        void	  stop();
        
        unsigned int getGroupId()     { return _diagnManagerGroup; }
        string 	     getShmBufName()  { return _shmBufName; }
        unsigned int getFrameSizePx() { return _frameSizePx; }
        
        
    private: // --- METHODS
    	
        /*
         * Entry point for the thread, used in start()->pthread_create(...)
         */
        static void* execute(void *pthis);
        
        void attachShm() throw (DiagnosticManagerException);
        void detachShm() throw (DiagnosticManagerException);
        
        /*
         * Perform the diagnostic
         */
        void diagnostic() throw (AbstractBcuRequestManagerException, DiagnosticManagerException, MirrorControllerShutdownException);
        
        
    private: // --- FIELDS
    
    	// The couple (FrameGroup, FrameSizePx) is the unique identifier for a diagnostic manager. 
    	// This is used to start only a single DiagnosticManager for each group. See the file
    	// DiagnosticManagerPool.h for more info.
    	unsigned int _diagnManagerGroup;
    	unsigned int _frameSizePx;
    
    	// These two fields are used to create/attach/destroy the shm buffer
    	string 	_mirrorCtrlName;
 
 		// Dictionary read from a config file containing all diagnostic parameters
    	Config_File _config;
    	
    	// Flag to enable/disable the diagnostic
    	bool _started;
    	
    	// Diagnostic frequency expressed as microseconds interval
    	int	  _freqUs;			
    	
    	// Info about the shm buffer (always type BUFCONT) where frame is saved
    	string 	 _shmBufName;
    	BufInfo* _shmBufInfo; 

		BcuRequest _frameCounterRequest;	// Created from the config line
    	BcuRequest _frameRequest;			// Created from the config line
    	
    	// Frame counter and diagnostic frame info
    	int		_bcuFramesNum;		// Number of frames saved in the BCU buffer
    	bool	_pointerFlag;		// Flag indicating if the frame counter is a pointer or a counter
    	int		_frameHeaderSizeBytes;
    	BYTE* 	_frame;				// Mem temporary hosting the frame (before bufWrite)
    
    	void printData(BYTE* data, int dataSizeDw);
    
};

#endif
