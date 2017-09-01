// Lib

#include "BcuLib/BcuCommon.h"
#include "Utils.h"
using namespace Arcetri;
using namespace Arcetri::Bcu;

#include "DiagnosticManager.h"
#include "Commons/TimeToDie.h"

extern "C" {
#include "base/timelib.h"
}


DiagnosticManager::DiagnosticManager(string mirrorCtrlName, string configFile, int id, int priority, int logLevel) 
				   throw (DiagnosticManagerException) : AbstractBcuRequestManager("DIAGNOSTICMANAGER", id, priority, logLevel) 
				   {
				   	
	 _started = false;
    
    _mirrorCtrlName = mirrorCtrlName;
    
    _shmBufInfo = NULL;
    
    BcuIdMap bcuIdMap;
    
    // --- Read the config file --- //
    _logger->log(Logger::LOG_LEV_INFO, "DiagnosticManager [ID:%s] reading config (%s)...",  getObjId().c_str(), configFile.c_str());
    _config = Config_File(configFile);
    try {
	    // --- Retrieve the diagnostic group and frame size
	    // See _diagnManagerGroup field declaration in DiagnosticManager.h for more info.
	    _diagnManagerGroup = (uint)_config["GroupId"];
		_frameSizePx = ((uint32)_config["FrameLen"]-((uint32)_config["HdrLen"])*2)/2;	// - Remove header and footer
																						// - 1px = 16 bit = 2 bytes
	    			
	    // --- Retrieve the shared memory buffer name
	    // Search for the shm buffer name in the diagnostic manager config file; if fails,
	    // search in the config file specified by "TechViewer_conf" config parameter.
	    // This is because each (one for existing binning) Technical Viewer diagnostics 
	    // doesn't specify the shm buffer name, that's specified in the TechViewer config file.
	    try {
	    	_shmBufName = (string)_config["ShmName"];
	    }
	    catch(Config_File_Exception& e) {
	    	try {
		    	_logger->log(Logger::LOG_LEV_WARNING, "Shared memory buffer name not found in config file %s", configFile.c_str());
		    	string TVconfigFile = _config.getDir()+"/"+(string)_config["TechViewer_conf"];
		    	_logger->log(Logger::LOG_LEV_WARNING, "Searching shared memory buffer name in config file %s...", TVconfigFile.c_str());
		    	Config_File techViewConfig = Config_File(TVconfigFile);
		    	_shmBufName = (string)techViewConfig["ShmName"] + Utils::itoa((int)_frameSizePx);
	    	}
	    	catch(Config_File_Exception& e) {
	    		_logger->log(Logger::LOG_LEV_ERROR, "Impossible to find the shared memory buffer name");
	    		throw;
	    	}
	    }
	    _logger->log(Logger::LOG_LEV_INFO, "Found shared memory buffer name: %s", _shmBufName.c_str());
	    																				//- 16 bits per pixel => 1 pixel = 2 bytes
	  	// --- Retrieve all others config parameters ---//
	    string memType, frameCounterMemType;
	    int freqHertz, firstBcu, lastBcu, firstDsp, lastDsp, frameAddressDw, frameLenBytes, numFrames,
	    	hdrLenBytes, frameCounterBcu, frameCounterDsp, frameCounterAddressDw, pointerFlag;
    
	    freqHertz = _config["Freq"];
		memType = (string)_config["MemType"];	
		firstBcu = bcuIdMap[_config["FirstBcu"]];
		lastBcu = bcuIdMap[_config["LastBcu"]];
		firstDsp = _config["FirstDsp"];
		lastDsp = _config["LastDsp"];
		frameAddressDw = (uint32)_config["Address"]; 
		frameLenBytes = (uint32)_config["FrameLen"];	
		numFrames = (uint32)_config["FramesNum"];
		hdrLenBytes = _config["HdrLen"];
		frameCounterMemType = (string)_config["FrameCountMem"];
		frameCounterBcu = bcuIdMap[(string)_config["FrameCountBcu"]];
		frameCounterDsp = _config["FrameCountDsp"];	
		frameCounterAddressDw = (uint32)_config["FrameCountAddr"];
		pointerFlag = _config["PointerFlag"];
	
		_logger->log(Logger::LOG_LEV_DEBUG, "Config line parameters found:");
		if(_logger->getLevel() >= Logger::LOG_LEV_DEBUG) {
			_logger->log(Logger::LOG_LEV_DEBUG, "> shmBufName:          %s", _shmBufName.c_str());
			_logger->log(Logger::LOG_LEV_DEBUG, "> shmBufType:          BUFCONT (always)");
			_logger->log(Logger::LOG_LEV_DEBUG, "> freqHertz:           %d (%d ms)", freqHertz, 1000/freqHertz);
			_logger->log(Logger::LOG_LEV_DEBUG, "> memType:             %s", memType.c_str());
			_logger->log(Logger::LOG_LEV_DEBUG, "> firstBcu:            %s", ((string)_config["FirstBcu"]).c_str());
			_logger->log(Logger::LOG_LEV_DEBUG, "> lastBcu:             %s", ((string)_config["LastBcu"]).c_str());
			_logger->log(Logger::LOG_LEV_DEBUG, "> firstDsp:            %d", firstDsp);
			_logger->log(Logger::LOG_LEV_DEBUG, "> lastDsp:             %d", lastDsp);
			_logger->log(Logger::LOG_LEV_DEBUG, "> frameAddressDw:      0x%x", frameAddressDw);
			_logger->log(Logger::LOG_LEV_DEBUG, "> frameLenBytes:       0x%x (including header and footer)", frameLenBytes);
			_logger->log(Logger::LOG_LEV_DEBUG, "> frameSizePixel:      0x%x", _frameSizePx);
			_logger->log(Logger::LOG_LEV_DEBUG, "> numFrames:           0x%x", numFrames);
			_logger->log(Logger::LOG_LEV_DEBUG, "> hdrLenBytes:         %d", hdrLenBytes);
			_logger->log(Logger::LOG_LEV_DEBUG, "> Pointer flag:        %d", pointerFlag);
			_logger->log(Logger::LOG_LEV_DEBUG, "> frameCountMemType:   %s", frameCounterMemType.c_str());
			_logger->log(Logger::LOG_LEV_DEBUG, "> frameCountBcu:       %d", frameCounterBcu);
			_logger->log(Logger::LOG_LEV_DEBUG, "> frameCountDsp:       %d", frameCounterDsp);
			_logger->log(Logger::LOG_LEV_DEBUG, "> frameCountAddressDw: 0x%x", frameCounterAddressDw);
		}
	
		// !!! Init the output queue pool !!!
		initBcuPool(firstBcu, lastBcu-firstBcu+1);
		
		// --- Create the BcuRequests --- //
		_freqUs = (int)(1E6/freqHertz);
		_bcuFramesNum = numFrames;
		_frameHeaderSizeBytes = hdrLenBytes;
		int opCode = OpCodes::getReadOpCode(frameCounterMemType);
		_frameCounterRequest = BcuRequest(frameCounterBcu, frameCounterBcu, frameCounterDsp, frameCounterDsp, 
										  opCode, Constants::WANTREPLY_FLAG,
										  frameCounterAddressDw, Constants::DWORD_SIZE,
										  -1);
		
		
		opCode = OpCodes::getReadOpCode(memType);
		_frameRequest = BcuRequest(firstBcu, lastBcu, firstDsp,  lastDsp, 
								   opCode, Constants::WANTREPLY_FLAG,
								   frameAddressDw, frameLenBytes,
								   -1);
		
		// --- Crete the frame buffer --- //
		_pointerFlag = (pointerFlag == 1);
		int numDsp = (lastDsp-firstDsp)/OpCodes::getDspFactor(opCode)+1;
	 	_frame = new BYTE[_frameRequest.getDataSizeBytes()*numDsp*_bcuNum];
	 	// Clear the frame only for safety...
	 	memset(_frame, 0, _frameRequest.getDataSizeBytes()*numDsp*_bcuNum);
	 	for(int i=0; i < _frameRequest.getDataSizeBytes()*numDsp*_bcuNum; i++) {
	 		_frame[i] = 0;
	 	}
	 	
	 	_logger->log(Logger::LOG_LEV_INFO, ">> Full frame size: %d bytes (%d bytes x %d bcu x %d dsp)", _frameRequest.getDataSizeBytes()*numDsp*_bcuNum, _frameRequest.getDataSizeBytes(), _bcuNum, numDsp);
		_logger->log(Logger::LOG_LEV_INFO, "DiagnosticManager [ID:%s]: frame and frame/frame counter request created !!!", getObjId().c_str());
	 	
	 } 
    // ***NOTE*** that, whether an exception is thrown in a constructor, the base
    // class destructor is called !!!
    catch(AOException& e) { // Should be only BcuIdMapException or Config_File_Exception
    	_logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
    	throw DiagnosticManagerException((string)"Impossible to construct the DiagnosticManager [ID:" + getObjId().c_str() + "]");
    }	
}

DiagnosticManager::~DiagnosticManager() {
			
	detachShm();  
	_logger->log(Logger::LOG_LEV_INFO, "DiagnosticManager [ID:%s]: shm detached !!!", getObjId().c_str()); 	
	
	delete [] _frame;
	_logger->log(Logger::LOG_LEV_INFO, "DiagnosticManager [ID:%s] succesfully destroyed !!!",  getObjId().c_str());
}



pthread_t DiagnosticManager::start() {
	pthread_t diagnMan;
	_started = true;
	pthread_create(&diagnMan, NULL, &execute, (void*)this);
	return diagnMan;
}


void DiagnosticManager::stop() {
	_started = false;
}


void* DiagnosticManager::execute(void *pthis) {
	DiagnosticManager* diagnMan = (DiagnosticManager*)pthis;
	try {
		diagnMan->diagnostic(); 
	}
	catch(DiagnosticManagerException& e) {
		diagnMan->getLogger()->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
		diagnMan->getLogger()->log(Logger::LOG_LEV_ERROR, "DiagnosticManager %s failed", diagnMan->getObjId().c_str());
	}
	catch(MirrorControllerShutdownException& e) {
		diagnMan->getLogger()->log(Logger::LOG_LEV_WARNING, "DiagnosticManager %s shutting down...", diagnMan->getObjId().c_str());
	}
	catch(AbstractBcuRequestManagerException& e) {
		diagnMan->getLogger()->log(Logger::LOG_LEV_WARNING, e.what().c_str());
		diagnMan->getLogger()->log(Logger::LOG_LEV_ERROR, "DiagnosticManager %s failed", diagnMan->getObjId().c_str());
	}
	return NULL;
}

void DiagnosticManager::diagnostic() throw (AbstractBcuRequestManagerException, DiagnosticManagerException, MirrorControllerShutdownException) {
 		
 	// Structures used to respect the requested frequency
 	struct timeval startTime, endTime;
 	int elapsedTimeUs;
 	int timeoutUs; 
 	
 	int shmBufWriteCounter = 0;
 	int shmBufWriteRet;
 		
 	attachShm();  // Can't attach in constructor because thrdLib isn't initialized
 		
 	// --- Loop to download diagnostic frames --- //
 	// Frame address initial value: must be incremented in the loop 
 	// according to frameCounter
 	int frameCounterBaseAddressDw = _frameCounterRequest.getDataAddressDw();	// Save it because is increased in sendBcuRequest()
 	int frameBaseAddressDw = _frameRequest.getDataAddressDw();					// Save it for the same reason as above and to add the offset to it
 	int frameSizeBytes = _frameRequest.getDataSizeBytes();	
 	
 	BYTE* frameHeader = _frame ;
 	BYTE* frameFooter = _frame + frameSizeBytes - _frameHeaderSizeBytes;
 	
 	// --- TEST --- //
// 	int framesToDownload = 100;
//  struct timeval startTest, endTest;
//  gettimeofday(&startTest,NULL);
//  _logger->log(Logger::LOG_LEV_WARNING, "!!! ATTENTION: RUNNING TEST: getting %d frames of size %d bytes...", framesToDownload, frameSizeBytes*Constants::DWORD_SIZE);; 
    // ------------ //
    
    uint32 frameCounter = 0;
	uint32 lastFrameCounter = 0;
    bool frameCounterRequestFailed = false;
	while(_started && !TimeToDie::check()) {	// --- TEST

		timeoutUs = _freqUs;
		elapsedTimeUs = 0;
		gettimeofday(&startTime, NULL);
		
		// (1) --- Read the current frame counter/pointer --- //
		setBcuRequestInfo(&_frameCounterRequest, (BYTE*)(&frameCounter));
		sendBcuRequest();
		_frameCounterRequest.setDataAddressDw(frameCounterBaseAddressDw);
		waitBcuReply();
		frameCounterRequestFailed = _currentRequestInfo->isFailed();
		clearBcuRequestInfo();
		
		// (2) --- Check frame counter succesfully got  and udjust it --- //
		// ATTENTION: when I read a frameCounter I MUST always read the PREVIOUS 
		// frame, because frameCounter refers to the currently writed frame.
		// Note: be carefull when frameCounter is ZERO - the previous frame is 
		// the last one !
		if(!frameCounterRequestFailed) {

            _logger->log(Logger::LOG_LEV_INFO, ">>> DiagnosticManager [ID:%s]: FRAME COUNTER from BCU (address %u) is %u ", getObjId().c_str(), _frameCounterRequest.getDataAddressDw(), frameCounter);
			
			if(_pointerFlag) {	
				// Check if I have to read the last frame
				// ATTENTION: the frameCounter value is in BYTES !!!
				if(frameCounter == 0) {
					frameCounter = (_bcuFramesNum-1) * frameSizeBytes;
				}
				else {
					frameCounter -= frameSizeBytes;
				}

            // Reset frame counter to the last valid start of frame (we may get a pointer in the middle of a frame!)
            frameCounter -= frameCounter % frameSizeBytes;
			}
			else {
				// Check if I have to read the last frame
				// ATTENTION: the frameCounter value is an index !!!

	            // Since sometimes we read an old frame, here is an implementation for a shift to past frames
	            //unsigned int shift=2;
	            //if (frameCounter<shift)
	            //   frameCounter = _bcuFramesNum -shift + frameCounter;
	            //else
	            //   frameCounter -= shift;
	
	            _logger->log(Logger::LOG_LEV_INFO, ">>> DiagnosticManager [ID:%s]: FRAME COUNTER from BCU shifted to %u ", getObjId().c_str(), frameCounter);

				if(frameCounter == 0) {	
					frameCounter = (_bcuFramesNum-1) * frameSizeBytes;
				}
				else {
				   frameCounter = frameBaseAddressDw*Constants::DWORD_SIZE + (((frameCounter-1) % _bcuFramesNum) * frameSizeBytes);
				}
			}

            _logger->log(Logger::LOG_LEV_INFO, ">>> DiagnosticManager [ID:%s]: FRAME POINTER is %u ", getObjId().c_str(), frameCounter);
			
			// (3) --- Check if frame counter has been changed ---//
			if(frameCounter != lastFrameCounter) {
				
				_logger->log(Logger::LOG_LEV_INFO, ">>> DiagnosticManager %s: Dowloading frame...", getObjId().c_str());
				
				// (4) --- Read the dagnostic frame using the frame counter/pointer
				setBcuRequestInfo(&_frameRequest, _frame);
				sendBcuRequest();
				_frameRequest.setDataAddressDw(frameCounter/Constants::DWORD_SIZE);
				waitBcuReply();
				if(!_currentRequestInfo->isFailed()) {
					// Check for frame header and footer to be equal
					if(memcmp(frameHeader, frameFooter, _frameHeaderSizeBytes) == 0) {
						// Copy the content of _frame into shm
						shmBufWriteRet = bufWrite(_shmBufInfo, _frame, ++shmBufWriteCounter, 0);
						if (IS_ERROR(shmBufWriteRet)) {
					 		throw DiagnosticManagerException("Impossible to write to the shared memory buffer (" + Utils::itoa(shmBufWriteRet) + ")");
					 	}
						_logger->log(Logger::LOG_LEV_INFO, ">>> DiagnosticManager [ID:%s]: FRAME %u GOT and stored to SHM ! (%s:%s) <<<", getObjId().c_str(), frameCounter, _shmBufInfo->com.producer, _shmBufInfo->com.name);
						lastFrameCounter = frameCounter;
					}
					else {
						_logger->log(Logger::LOG_LEV_WARNING, ">>> DiagnosticManager [ID:%s]: FRAME %u NOT GOT  (header and footer don't match) <<<", getObjId().c_str(), frameCounter);
						printData(frameHeader, _frameHeaderSizeBytes/Constants::DWORD_SIZE);
						printData(frameFooter, _frameHeaderSizeBytes/Constants::DWORD_SIZE);
					}
				}
				else {
					_logger->log(Logger::LOG_LEV_WARNING, ">>> DiagnosticManager [ID:%s]: FRAME %u NOT GOT  (request failed) <<<", getObjId().c_str(), frameCounter);
				}
				clearBcuRequestInfo(); 
			}
            else {
                _logger->log(Logger::LOG_LEV_WARNING, ">>> DiagnosticManager [ID:%s]: FRAME %u NOT GOT  (old frame) <<<", getObjId().c_str(), frameCounter);
            }
		}
		else {
			_logger->log(Logger::LOG_LEV_WARNING, ">>> DiagnosticManager [ID:%s]: FRAME COUNTER NOT GOT (Bcu not started)", getObjId().c_str());
		}
		
		// Wait to ask the next frame
		gettimeofday(&endTime, NULL);
		elapsedTimeUs = ((endTime.tv_sec-startTime.tv_sec)*(int)1E6 + (endTime.tv_usec-startTime.tv_usec));
		timeoutUs -= elapsedTimeUs;
		//-->//_logger->log(Logger::LOG_LEV_INFO, "DiagnosticManager [ID:%s]: ELAPSED TIME: %d us (%d ms)...", getObjId().c_str(), elapsedTimeUs, elapsedTimeUs/1000);
		if(timeoutUs>0) {
			//_logger->log(Logger::LOG_LEV_WARNING, "DiagnosticManager [ID:%s]: WAITING %d us (%d ms)...", getObjId().c_str(), timeoutUs, timeoutUs/1000);
			nusleep(timeoutUs);
		}
	}
	// --- TEST --- //
//	gettimeofday(&endTest,NULL);
//	double time = (endTest.tv_usec-startTest.tv_usec) + (endTest.tv_sec - startTest.tv_sec)*1000000;
// 	_logger->log(Logger::LOG_LEV_WARNING, ">>> TEST FINISHED: TOTAL TIME ELAPSED time: %.0f ms", time/1000);
	// ------------ //
}


void DiagnosticManager::attachShm() throw (DiagnosticManagerException) {
	
	_logger->log(Logger::LOG_LEV_INFO, "Creating the shm buffer %s...", _shmBufName.c_str());
	
	if(_shmBufInfo == NULL) {
	
		string	memType = _config["MemType"];	
		int		firstBcu = _config["FirstBcu"];
		int		lastBcu = _config["LastBcu"];
		int		firstDsp = _config["FirstDsp"];
		int		lastDsp = _config["LastDsp"];
		uint	frameLenBytes = (uint)_config["FrameLen"];	
		string	frameCounterMemType = _config["FrameCountMem"];
		
		_shmBufInfo = new BufInfo();
		
		//--- Create the shm ---//
	 	int numBcu = lastBcu-firstBcu+1;
	 	int numDsp = (lastDsp-firstDsp)/OpCodes::getDspFactor(OpCodes::getReadOpCode(memType))+1;
	 	int ret = NO_ERROR;
	 	ret = bufCreate((char*)_mirrorCtrlName.c_str(), 
	 					(char*)_shmBufName.c_str(), 
	 					BUFCONT, 
	 					frameLenBytes*numDsp*numBcu, 10,
	 					_shmBufInfo);
	 						
	 	if (IS_ERROR(ret)) {
	 		throw DiagnosticManagerException("Impossible to create the shared memory buffer " + _shmBufName + " (bufCreate returned " + Utils::itoa(ret) + ")");
	 	}
	 						
	 	_logger->log(Logger::LOG_LEV_INFO, "Shm buffer %s created with size: %d bytes (%d bytes x %d bcu x %d dsp)", (char*)(_mirrorCtrlName+":"+_shmBufName).c_str(), frameLenBytes*numDsp*numBcu, frameLenBytes, numBcu, numDsp);
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "Shm buffer %s already existing... OK !", _shmBufName.c_str());
	}
}



void DiagnosticManager::detachShm() throw (DiagnosticManagerException) {
	
	if(_shmBufInfo) {
		string shmClientName = _mirrorCtrlName;
		string shmBufCompleteName = shmClientName + ":" + _shmBufName;
		_logger->log(Logger::LOG_LEV_INFO, "Detaching from shm %s...", shmBufCompleteName.c_str());
		
		// Detach from the shm
		int ret = NO_ERROR;
		ret = bufRelease((char*)shmClientName.c_str(), _shmBufInfo);
		if (IS_ERROR(ret)) {
	 		throw DiagnosticManagerException("Impossible to destroy the shared memory buffer (" + Utils::itoa(ret) + ")");
	 	}
		else {
			_logger->log(Logger::LOG_LEV_INFO, "Succesfully detached from shm %s", shmBufCompleteName.c_str());
		}
		
		delete _shmBufInfo;
	}
}

void DiagnosticManager::printData(BYTE* data, int dataSizeDw) {
	if(_logger->getLevel() >= Logger::LOG_LEV_DEBUG) {
		printf("DATA: ");
		for(int i=0; i<dataSizeDw; i++) {
			printf("0x%08X ", ((uint32*)data)[i]);
		}
		printf("\n");
	}
}

