#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "MasterDiagnostic.h"

#include "AdSecConstants.h"
#include "WfsConstants.h"
#include "BcuLib/BcuCommon.h"
#include "Utils.h"

#include "arblib/base/AlertsExport.h"
#include "arblib/base/Alerts.h" // Needed for Alert serialization
#include "arblib/base/ArbitratorInterface.h"



extern "C" {
#include "base/timelib.h"
}


using namespace Arcetri;
using namespace Arcetri::Bcu;


MasterDiagnostic::MasterDiagnostic(int argc, char **argv) throw(MasterDiagnosticException) 
				: AOApp(argc, argv) {
	create();
}

void MasterDiagnostic::create() throw(MasterDiagnosticException) {
	
	// Init loggers for FramesFunnel
	Logger::get("OPTICAL-LOOP", Logger::stringToLevel(ConfigDictionary()["OptLoopLogLevel"]))->printStatus();
	Logger::get("ADAPTIVE-SECONDARY", Logger::stringToLevel(ConfigDictionary()["AdSecLogLevel"]))->printStatus();
	Logger::get("MIRROR-DATA-BOUNCER", Logger::stringToLevel(ConfigDictionary()["AdSecBouncerLevel"]))->printStatus();

	
	// Init loggers for FrameDownloaders
	Logger::get("BCUSWITCH", Logger::stringToLevel(ConfigDictionary()["BcuSwitchLogLevel"]))->printStatus();
	Logger::get("ADSEC_STREAM", Logger::stringToLevel(ConfigDictionary()["AdSecStreamLogLevel"]))->printStatus();
	Logger::get("BCU39", Logger::stringToLevel(ConfigDictionary()["Bcu39LogLevel"]))->printStatus();
	for(int i=0; i<AdSecConstants::BcuMirror::N_CRATES; i++) {
		Logger::get("BCUMIRROR_" + Utils::itoa(i), Logger::stringToLevel(ConfigDictionary()["BcuMirrorLogLevel"]))->printStatus();
	}
	Logger::get("TECHVIEWER", Logger::stringToLevel(ConfigDictionary()["TechViewLogLevel"]))->printStatus();

	
	// ------------------ (1) Init MasterDiagnostic status --------------- //
	// ------------------------------------------------------------------- //
	
	_statusTableOptLoop = NULL;
	_optLoopDiagnEnabled = false;
	_optLoopDignFunnel = NULL;
	BYTE* optLoopDiagnBuffer = NULL;
	_optLoopDiagnBufferHeaders = NULL;
	
	_statusTableAdSec = NULL;
	_adSecDiagnEnabled = false;
	_adSecDignFunnel = NULL;
	_adSecBouncerEnabled = false;
   _adSecBouncer = NULL;
	BYTE* adSecDiagnBuffer = NULL;
	_adSecDiagnBufferHeaders = NULL;
	
	_bcuSwitchFramesDownloader = NULL;
   _adsecStreamFramesDownloader = NULL;
	_bcu39FramesDownloader = NULL;
	_bcuMirrorFramesDownloaders = new FramesDownloader*[AdSecConstants::BcuMirror::N_CRATES];
	for(int i=0; i< AdSecConstants::BcuMirror::N_CRATES; i++) {
		_bcuMirrorFramesDownloaders[i] = NULL;
	}
	
	
	// ------------- (2) Establish which diagnostic must and can be enabled --------------- //
	// ------------------------------------------------------------------------------------ //
	
	// Optical Loop
	if((int32)ConfigDictionary()["EnableOptLoopDiagn"] == 1 && WfsConstants::Bcu39::N_CRATES == 1) {
		_optLoopDiagnEnabled = true;
	}
	
	// Adaptive Secondary
	if((int32)ConfigDictionary()["EnableAdSecDiagn"] == 1 && AdSecConstants::BcuSwitch::N_CRATES == 1 && AdSecConstants::BcuMirror::N_CRATES >= 1) {
		_adSecDiagnEnabled = true;
	}
	
	// AdSec Bouncer
	if((int32)ConfigDictionary()["EnableAdSecBouncer"] == 1 ) {
      if (_adSecDiagnEnabled == false) { 
         _logger->log(Logger::LOG_LEV_ERROR, "AdSec diagnostic is disabled: Bouncer can't be enabled [%s:%d]", __FILE__, __LINE__);
		   throw MasterDiagnosticException("MasterDiagnosticException", "AdSec diagnostic is disabled: Bouncer can't be enabled");
      }
      _adSecBouncerEnabled = true;
	}

	// Tech Viewer
	if((int32)ConfigDictionary()["EnableTechViewer"] == 1 && WfsConstants::Bcu47::N_CRATES == 1) {
		_techViewerEnabled = true;
	}
	
	if(!_optLoopDiagnEnabled && !_adSecDiagnEnabled && !_techViewerEnabled) {
		_logger->log(Logger::LOG_LEV_WARNING, "Optical Loop, Adaptive Secondary diagnostics and Technical Viewer all disabled: check config file (%s) and WfsConstants/AdSecConstants", ConfigFile().c_str());
		throw MasterDiagnosticException("MasterDiagnosticException", "No diagnostics to start");
	}
	

	// ------------------- (3) Load configuration info form the config file ----------------- //
	// -------------------------------------------------------------------------------------- //
	
	int bcu39FrameSizeDw        = ConfigDictionary()["Bcu39FrameSizeDw"];
	int adsecStreamFrameSizeDw  = ConfigDictionary()["AdSecStreamFrameSizeDw"];
	int bcuSwitchFrameSizeDw = ConfigDictionary()["BcuSwitchFrameSizeDw"];
	int bcuMirrorFrameSizeDw = ConfigDictionary()["BcuMirrorFrameSizeDw"];
	_optLoopDiagnBufferCapacity = ConfigDictionary()["OptLoopBufferCapacity"];
	_adSecDiagnBufferCapacity = ConfigDictionary()["AdSecBufferCapacity"];
	
	// Frames history info
	int optLoopDiagnHistorySize = ConfigDictionary()["OptLoopHistorySize"];
	int adSecDiagnHistorySize = ConfigDictionary()["AdSecHistorySize"];
	
	// Those sizes are set to max possible value, according to "Closed Loop" mode: 
	// when the loop is open the OptLoop diagnostic will consider only the Bcu39 subframe !
	int optLoopFullDiagnFrameSizeDw = sizeof(FullDiagnFrameHeader)/Constants::DWORD_SIZE + bcu39FrameSizeDw + adsecStreamFrameSizeDw;
	int	adSecFullDiagnFrameSizeDw = sizeof(FullDiagnFrameHeader)/Constants::DWORD_SIZE + bcuSwitchFrameSizeDw + AdSecConstants::BcuMirror::N_CRATES*bcuMirrorFrameSizeDw;
	int techViewerFrameSizeDw = ConfigDictionary()["TechViewFrameSizeDw"];

	_logger->log(Logger::LOG_LEV_INFO, "");
	_logger->log(Logger::LOG_LEV_INFO, "----------------------- MASTER DIAGNOSTIC GENERAL INFO ---------------------- [%s:%d]", __FILE__, __LINE__);
	_logger->log(Logger::LOG_LEV_INFO, "> Bcu39FrameSize:       %d DWORD", bcu39FrameSizeDw);
	_logger->log(Logger::LOG_LEV_INFO, "> adsecStreamFrameSize: %d DWORD", adsecStreamFrameSizeDw);
	_logger->log(Logger::LOG_LEV_INFO, "> BcuSwitchFrameSize:   %d DWORD", bcuSwitchFrameSizeDw);
	_logger->log(Logger::LOG_LEV_INFO, "> BcuMirrorFrameSize:   %d DWORD", bcuMirrorFrameSizeDw);
	_logger->log(Logger::LOG_LEV_INFO, "");
	_logger->log(Logger::LOG_LEV_INFO, "OPTICAL LOOP DIAGNOSTIC ENABLED: %d", _optLoopDiagnEnabled);
	_logger->log(Logger::LOG_LEV_INFO, "> Full frame size:             %d DWORD, %d Bytes (including header %d DWORD)", optLoopFullDiagnFrameSizeDw, optLoopFullDiagnFrameSizeDw * Constants::DWORD_SIZE, sizeof(FullDiagnFrameHeader)/Constants::DWORD_SIZE);
	_logger->log(Logger::LOG_LEV_INFO, "> Frames history size:         %d frames (%d Bytes)", optLoopDiagnHistorySize, optLoopDiagnHistorySize * optLoopFullDiagnFrameSizeDw * Constants::DWORD_SIZE);
	_logger->log(Logger::LOG_LEV_INFO, "> Fil8 frames buffer capacity: %d frames", _optLoopDiagnBufferCapacity);
	_logger->log(Logger::LOG_LEV_INFO, "");
	_logger->log(Logger::LOG_LEV_INFO, "ADAPTIVE SECONDARY DIAGNOSTIC ENABLED: %d", _adSecDiagnEnabled);
	_logger->log(Logger::LOG_LEV_INFO, "> Full frame size:             %d DWORD, %d Bytes (including header %d DWORD)", adSecFullDiagnFrameSizeDw, adSecFullDiagnFrameSizeDw * Constants::DWORD_SIZE, sizeof(FullDiagnFrameHeader)/Constants::DWORD_SIZE);
	_logger->log(Logger::LOG_LEV_INFO, "> Frames history size:         %d frames (%d Bytes)", adSecDiagnHistorySize, adSecDiagnHistorySize * adSecFullDiagnFrameSizeDw * Constants::DWORD_SIZE);
	_logger->log(Logger::LOG_LEV_INFO, "> Fil8 frames buffer capacity: %d frames", _adSecDiagnBufferCapacity);
	_logger->log(Logger::LOG_LEV_INFO, "");
	_logger->log(Logger::LOG_LEV_INFO, "ADAPTIVE SECONDARY BOUNCER ENABLED: %d", _adSecBouncerEnabled);
	_logger->log(Logger::LOG_LEV_INFO, "");
	_logger->log(Logger::LOG_LEV_INFO, "TECHNICAL VIEWER ENABLED: %d", _techViewerEnabled);
	_logger->log(Logger::LOG_LEV_INFO, "> Base frame size (bin 16x6): %d DWORD", techViewerFrameSizeDw);
	
	
	_logger->log(Logger::LOG_LEV_INFO, "-------------------------------------------------------------------------------");
	_logger->log(Logger::LOG_LEV_INFO, "");
	
	// ---------- (4) Construct Optical Loop and Adaptive Secondary diagnostics ---------- //
	
	// Compute sizes in bytes
	int bcu39FrameSizeBytes 	= bcu39FrameSizeDw * Constants::DWORD_SIZE;
	int bcuSwitchFrameSizeBytes = bcuSwitchFrameSizeDw * Constants::DWORD_SIZE;
	int bcuMirrorFrameSizeBytes = bcuMirrorFrameSizeDw * Constants::DWORD_SIZE;
	
	int optLoopFullDiagnFrameSizeBytes = optLoopFullDiagnFrameSizeDw * Constants::DWORD_SIZE;
	int	adSecFullDiagnFrameSizeBytes = adSecFullDiagnFrameSizeDw * Constants::DWORD_SIZE;
	
	// --- Construct the DIAGNOSTIC for the Optical Loop --- //
	if(_optLoopDiagnEnabled) {
		_logger->log(Logger::LOG_LEV_INFO, "------------------- MASTER DIAGNOSTIC for the Optical Loop --------------------");
		
		// Allocate Buffers 
		_logger->log(Logger::LOG_LEV_INFO, "Allocating buffer for Optical Loop diagnostic...");
		int optLoopDiagnFrameBufSizeBytes = optLoopFullDiagnFrameSizeBytes * _optLoopDiagnBufferCapacity;
		optLoopDiagnBuffer = new BYTE[optLoopDiagnFrameBufSizeBytes];
		memset(optLoopDiagnBuffer, 0, optLoopDiagnFrameBufSizeBytes);	// Needed to clear headers !
		_logger->log(Logger::LOG_LEV_INFO, "> Optical Loop diagn buffer of size %d DWORD (%d Bytes) starting address (from header): %d", optLoopDiagnFrameBufSizeBytes/Constants::DWORD_SIZE, optLoopDiagnFrameBufSizeBytes, (intptr_t)optLoopDiagnBuffer);
		
		// Obtain an array of pointers to buffer headers
		_optLoopDiagnBufferHeaders = new int*[_optLoopDiagnBufferCapacity];
		for(int i=0; i<_optLoopDiagnBufferCapacity; i++) {
			_optLoopDiagnBufferHeaders[i] = (int*)&optLoopDiagnBuffer[i*optLoopFullDiagnFrameSizeBytes];
		}
		_optLoopDiagnFramesNum = 1; // By default the loop is open (only frame from Bcu39)
		initOptLoopFrameBufferHeaders();
		
		// Create Status table
		_logger->log(Logger::LOG_LEV_INFO, "Allocating buffer status table...");
		_statusTableOptLoop = new StatusTable("OPTICAL-LOOP", _optLoopDiagnBufferCapacity, _optLoopDiagnFramesNum);
		
		// Create the FramesFunnel
		_optLoopDignFunnel = new FramesFunnel("OPTICAL-LOOP", &ConfigDictionary(), optLoopDiagnBuffer, _optLoopDiagnBufferCapacity,
										  	  optLoopFullDiagnFrameSizeBytes,
										      _statusTableOptLoop, 
										      MyFullName(),
										      ConfigDictionary()["OptLoopShmBuf"],
										      optLoopDiagnHistorySize);

	
		// Create the Bcu39 FramesDownloader
		_bcu39FramesDownloader = new FramesDownloader("BCU39", 
		             								  ConfigDictionary()["Bcu39Ip"],
													  ConfigDictionary()["Bcu39Port"],
													  ConfigDictionary()["WfsCountOffset"],
													  bcu39FrameSizeDw, 
                                         0, 
													  _statusTableOptLoop);
		for(int i=0; i<_optLoopDiagnBufferCapacity; i++) {
			_bcu39FramesDownloader->addFrameSlot(&optLoopDiagnBuffer[i*optLoopFullDiagnFrameSizeBytes + sizeof(FullDiagnFrameHeader)]);
		}

		
		// Create the adsecStream FramesDownloader
		_adsecStreamFramesDownloader = new FramesDownloader("ADSEC_STREAM", 
		             								  ConfigDictionary()["adsecStreamIp"],
													  ConfigDictionary()["adsecStreamPort"],
													  ConfigDictionary()["WfsCountOffset"],
													  adsecStreamFrameSizeDw, 
                                         0,
													  _statusTableOptLoop);
		for(int i=0; i<_optLoopDiagnBufferCapacity; i++) {
			_adsecStreamFramesDownloader->addFrameSlot(&optLoopDiagnBuffer[i*optLoopFullDiagnFrameSizeBytes + sizeof(FullDiagnFrameHeader) + bcu39FrameSizeBytes]);
		}

		
		_logger->log(Logger::LOG_LEV_INFO, "-------------------------------------------------------------------------------");
		_logger->log(Logger::LOG_LEV_INFO, "");
	}
	
	// --- Construct the DIAGNOSTIC for the Adaptive Secondary --- //
	if (_adSecDiagnEnabled) {
		_logger->log(Logger::LOG_LEV_INFO, "---------------- MASTER DIAGNOSTIC for the Adaptive Secondary -----------------");
		
		// Allocate Buffers 
		_logger->log(Logger::LOG_LEV_INFO, "Allocating buffer for Adeptive Secondary diagnostic...");
		int adSecDiagnFrameBufSizeBytes = adSecFullDiagnFrameSizeBytes * _adSecDiagnBufferCapacity;
		adSecDiagnBuffer   = new BYTE[adSecDiagnFrameBufSizeBytes];
		memset(adSecDiagnBuffer, 0, adSecDiagnFrameBufSizeBytes);		// Needed to clear headers !
                _logger->log(Logger::LOG_LEV_INFO, "> Adaptive Secondary diagn buffer of size %d DWORD (%d Bytes) starting address (from header): %d", adSecDiagnFrameBufSizeBytes/Constants::DWORD_SIZE, adSecDiagnFrameBufSizeBytes, (intptr_t)adSecDiagnBuffer);
		
		// Obtain an array of pointers to buffer headers
		_adSecDiagnBufferHeaders   = new int*[_adSecDiagnBufferCapacity];
		for(int i=0; i<_adSecDiagnBufferCapacity; i++) {
			_adSecDiagnBufferHeaders[i] = (int*)&adSecDiagnBuffer[i*adSecFullDiagnFrameSizeBytes];
		}
		_adSecDiagnFramesNum = 1 + AdSecConstants::BcuMirror::N_CRATES;
		initAdSecFrameBufferHeaders();
		
		// Create Status table
		_logger->log(Logger::LOG_LEV_INFO, "Allocating buffer status table... [%s:%d]", __FILE__, __LINE__);
		_statusTableAdSec = new StatusTable("ADAPTIVE-SECONDARY", _adSecDiagnBufferCapacity, _adSecDiagnFramesNum);
		
		// Create the FramesFunnel
		_adSecDignFunnel = new FramesFunnel("ADAPTIVE-SECONDARY", &ConfigDictionary(), adSecDiagnBuffer, _adSecDiagnBufferCapacity,
											adSecFullDiagnFrameSizeBytes,
											_statusTableAdSec, 
                                 MyFullName(),
											ConfigDictionary()["AdSecShmBuf"],
											adSecDiagnHistorySize);

		// BcuMirror FramesDownloader set
		for(int i=0; i< AdSecConstants::BcuMirror::N_CRATES; i++) {
			_bcuMirrorFramesDownloaders[i] = new FramesDownloader("BCUMIRROR_" + Utils::itoa(i), 
																  ConfigDictionary()["BcuMirror" + Utils::itoa(i) + "Ip"],
																  ConfigDictionary()["BcuMirror" + Utils::itoa(i) + "Port"],
			 													  ConfigDictionary()["MirCountOffset"],
			 													  bcuMirrorFrameSizeDw, 
                                                  0,
			 													  _statusTableAdSec);
			
			for(int j=0; j<_adSecDiagnBufferCapacity; j++) {
				_bcuMirrorFramesDownloaders[i]->addFrameSlot(&adSecDiagnBuffer[j*adSecFullDiagnFrameSizeBytes + sizeof(FullDiagnFrameHeader) + bcuSwitchFrameSizeBytes + i*(bcuMirrorFrameSizeBytes)]);
			}
		}
		
		_logger->log(Logger::LOG_LEV_INFO, "-------------------------------------------------------------------------------");
		_logger->log(Logger::LOG_LEV_INFO, "");
	}
	
	// --- Construct the DIAGNOSTIC for the Adaptive Secondary --- //
	if (_adSecBouncerEnabled) {
		_logger->log(Logger::LOG_LEV_INFO, "---------------- BOUNCER for the Adaptive Secondary -----------------");
      // Create the Frames Bouncer
      string bufname = MyFullName() + ":" + (string)ConfigDictionary()["AdSecShmBuf"];
      _adSecBouncer = new FramesBouncer("MIRROR-DATA-BOUNCER", MyFullName(), bufname);
		_logger->log(Logger::LOG_LEV_INFO, "-------------------------------------------------------------------------------");
		_logger->log(Logger::LOG_LEV_INFO, "");
	}
										
	if(_adSecDiagnEnabled) {
		// --- Construct SwitchFramesDownloader for AdSec diagnostics --- //
		_bcuSwitchFramesDownloader = new FramesDownloader("BCUSWITCH",
                                                ConfigDictionary()["BcuSwitchIp"],
													 			ConfigDictionary()["BcuSwitchPort"],
														 		ConfigDictionary()["MirCountOffset"],
														 		bcuSwitchFrameSizeDw, 
                                                0,
														 		 _statusTableAdSec);
	}
	
	if(_adSecDiagnEnabled) {
		for(int i=0; i<_adSecDiagnBufferCapacity; i++) {
			_bcuSwitchFramesDownloader->addFrameSlot(&adSecDiagnBuffer[i*adSecFullDiagnFrameSizeBytes + sizeof(FullDiagnFrameHeader)]);
		}
	}
	
	// --- Construct the TECHNICAL VIEWER (pseudo-diagnostic) --- //
	if(_techViewerEnabled) {
		_logger->log(Logger::LOG_LEV_INFO, "------------------- TECHNICAL VIEWER --------------------");
		
		// Create the Bcu47 TechnicalViewer
		string  shmBufName;
		int		bitPerPixel;
		
		try {
		    shmBufName = (string)ConfigDictionary()["TechViewerShmBuf"];
		    bitPerPixel = 16;
		}
	    catch(Config_File_Exception& e) {
	    	_logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
    		_logger->log(Logger::LOG_LEV_ERROR, "Impossible to find the shared memory buffer name");
    		throw MasterDiagnosticException("MasterDiagnosticException", "Impossible to start the Technical Viewer");
    	}
    	
		_techViewer = new TechnicalViewer("TECHVIEWER", 
		             					  ConfigDictionary()["Bcu47Ip"],
		             					  ConfigDictionary()["Bcu47Port"],
										  ConfigDictionary()["TechViewCountOffset"],
										  0,	// Download disable: binning still not changed!
										  bitPerPixel,
                                MyFullName(),
										  shmBufName);
										  
		
		_logger->log(Logger::LOG_LEV_INFO, "-------------------------------------------------------------------------------");
		_logger->log(Logger::LOG_LEV_INFO, "");
	}


   // Start assuming open loop (WFS only, no effect on AdSec)
   setCloseLoop(false);
}


MasterDiagnostic::~MasterDiagnostic() {
	
	_logger->log(Logger::LOG_LEV_INFO, "DESTROYING THE APPLICATION...");
	
	// Clean the entire app
	delete _optLoopDignFunnel;
	delete _adSecDignFunnel;
   delete _adSecBouncer;
	delete _bcu39FramesDownloader;
   delete _adsecStreamFramesDownloader;
	delete _bcuSwitchFramesDownloader;
	for(int i=0; i< AdSecConstants::BcuMirror::N_CRATES; i++) {
		delete _bcuMirrorFramesDownloaders[i];
	}
	delete _techViewer;

}

void MasterDiagnostic::SetupVars() {
	try {
		// Setup vars for open/close loop support
		if(_optLoopDiagnEnabled) {
			setupCloseLoopVars();
		}
        
        // Setup vars for technical viewer frame size runtime change support
        if(_techViewerEnabled) {
        	setupTechViewerVars();
        }
        
    } catch (AOVarException &e) { 
    	_logger->log(Logger::LOG_LEV_WARNING, "%s", e.what().c_str());
    	throw  MasterDiagnosticException("MasterDiagnosticException", "Impossible to setup RTDB vars");
    }
}

void MasterDiagnostic::setupCloseLoopVars() {
	_closeLoopReq = RTDBvar(MyFullName(), "CLOSELOOP", REQ_VAR, INT_VARIABLE, 1);
   _closeLoopCur = RTDBvar(MyFullName(), "CLOSELOOP", CUR_VAR, INT_VARIABLE, 1);
    Notify(_closeLoopReq, openCloseLoopHdlr);
    _logger->log(Logger::LOG_LEV_INFO, "Open/Close optical-loop RTDB vars support installed...");
} 

void MasterDiagnostic::setupTechViewerVars() {
	
	try {
		_techViewerFrameSize = RTDBvar(MyFullName(), "TV_NPIXELS", NO_DIR);
		Notify(_techViewerFrameSize, techViewFrameSizeChangedHdlr);
		_techViewRuntimeFrameChangeEnabled = true;
		_logger->log(Logger::LOG_LEV_INFO, " -> SUCCESFULLY installed support to run-time change Technical Viewer frame size !");
	}
	catch (AOVarException& e) {
		_logger->log(Logger::LOG_LEV_INFO, " -> NOT installed support to run-time change Technical Viewer frame size !");
	}
    
}

void MasterDiagnostic::InstallHandlers() {
	int stat;
	_logger->log(Logger::LOG_LEV_INFO, "Installing dump-history handlers...");
    if((stat=thHandler(MASTDIAGN_DUMP_ADSEC_FRAMES,"*",0,dumpAdSecHistoryHdlr,"dumpAdSecHistory", _adSecDignFunnel))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
        SetTimeToDie(true);
    }
    if((stat=thHandler(MASTDIAGN_DUMP_OPTLOOP_FRAMES,"*",0,dumpOptLoopHistoryHdlr,"dumpOptLoopHistory", _optLoopDignFunnel))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
        SetTimeToDie(true);
    }
    if((stat=thHandler(MASTDIAGN_SET_DECIMATION,"*",0,decimationHdlr,"setDataDecimation", _adSecBouncer))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
        SetTimeToDie(true);
    }
    if((stat=thHandler(MASTDIAGN_SET_BOUNCER_ADDR,"*",0,bouncerAddrHdlr,"setBouncerAddr", _adSecBouncer))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
        SetTimeToDie(true);
    }
}

void MasterDiagnostic::initOptLoopFrameBufferHeaders() {
	// Used to print the header mask in binary format
	char binStr[33];
	memset(binStr, 48, 32);
	binStr[32] = '\0';
	
	for(int j=0; j<_optLoopDiagnBufferCapacity; j++) {
		*(_optLoopDiagnBufferHeaders[j]) = 0;
		for(int i=0; i<_optLoopDiagnFramesNum; i++) {
			*(_optLoopDiagnBufferHeaders[j]) |= ((uint32)1 << i);
		}
		Utils::itoStrBin(binStr, *(_optLoopDiagnBufferHeaders[j]));
		_logger->log(Logger::LOG_LEV_TRACE, "> Header mask set to %32s for OpticalLoop buffer[%d]", binStr, j);
	}
}

void MasterDiagnostic::initAdSecFrameBufferHeaders() {
	// Used to print the header mask in binary format
	char binStr[33];
	memset(binStr, 48, 32);
	binStr[32] = '\0';
	
	for(int j=0; j<this->_adSecDiagnBufferCapacity; j++) {
		*(_adSecDiagnBufferHeaders[j]) = 0;
		for(int i=0; i<_adSecDiagnFramesNum; i++) {
			*(_adSecDiagnBufferHeaders[j]) |= ((uint32)1 << i);
		}
		Utils::itoStrBin(binStr, *(_adSecDiagnBufferHeaders[j]));
		_logger->log(Logger::LOG_LEV_DEBUG, "> Header mask set to %32s for AdSec buffer[%d]", binStr, j);
	}
}

// --------------------- HANDLERS ------------------------- //

int MasterDiagnostic::openCloseLoopHdlr(void *mdPtr, Variable *var) {
	MasterDiagnostic* md = (MasterDiagnostic*)mdPtr;
	int closeLoop = (var->Value.Lv[0] != 0) ? 1 : 0;
	if(closeLoop) {
		md->setCloseLoop(true);
	}
	else {
		md->setCloseLoop(false);
	}
	md->_closeLoopCur.Set(closeLoop);
	return NO_ERROR;
}

int MasterDiagnostic::techViewFrameSizeChangedHdlr(void* thisPtr, Variable* var) {
	MasterDiagnostic* md = (MasterDiagnostic*)thisPtr;
	md->techViewChangeFrameSize(var);
	return NO_ERROR;
}

int MasterDiagnostic::dumpAdSecHistoryHdlr(MsgBuf *msgb, void *argp, int /* hndlrQueueSize */) {
	FramesFunnel* adSecFF = (FramesFunnel*)argp;
	string dumpFilename = adSecFF->dumpHistory();

   // Send Notify to Arbitrator
   try {
      AlertNotifier* adSecArbNotifier = new AlertNotifier((string)(AOApp::ConfigDictionary()["AdSecArbName"])+"."+Side());
      DumpSaved dumpSaved( dumpFilename);
      adSecArbNotifier->notifyAlert((Alert*)(&dumpSaved));
      _logger->log(Logger::LOG_LEV_INFO,"Alert for dump to adsec arbitrator successfully sent");
   } catch (AOException &e) {
      _logger->log(Logger::LOG_LEV_ERROR,"Alert to adsec arbitrator failed: %s", e.what().c_str());
   }

   thRelease(msgb);
	return NO_ERROR;
}
		
int MasterDiagnostic::dumpOptLoopHistoryHdlr(MsgBuf *msgb, void *argp, int /* hndlrQueueSize */) {
	FramesFunnel* optLoopFF = (FramesFunnel*)argp;
	optLoopFF->dumpHistory();
   thRelease(msgb);
	return NO_ERROR;
}

int MasterDiagnostic::decimationHdlr( MsgBuf *msgb, void *argp, int /* hndlrQueueSize */) {
   FramesBouncer *adsecBouncer = (FramesBouncer*)argp;
   _logger->log(Logger::LOG_LEV_INFO, "Got message SetDecimation: %d ", (int) ((int*)MSG_BODY(msgb))[0]);
   int ret = NOT_INIT_ERROR;
   if (adsecBouncer)
      ret = adsecBouncer->setDecimation( ((int*)MSG_BODY(msgb))[0]);
   thReplyMsg( MASTDIAGN_SET_DECIMATION, sizeof(int), &ret, msgb);
   return ret;
}   

int MasterDiagnostic::bouncerAddrHdlr( MsgBuf *msgb, void *argp, int /* hndlrQueueSize */) {
   size_t pos;
   string addr;
   int    port=0;
   FramesBouncer *adsecBouncer = (FramesBouncer*)argp;
   addr = (char *)MSG_BODY(msgb);
   _logger->log(Logger::LOG_LEV_INFO, "Got message SetBouncerAddr: %s", addr.c_str());
   pos = addr.find(":");
   if (pos>0) {
      port = atoi(addr.substr(pos+1).c_str());
      addr = addr.substr(0,pos);
   }
   int ret = NOT_INIT_ERROR;
   if (adsecBouncer) 
      ret = adsecBouncer->setDestination( addr, port);
   thReplyMsg( MASTDIAGN_SET_BOUNCER_ADDR, sizeof(int), &ret, msgb);
   return ret;
}   



// --------------------- HANDLERS IMPL ------------------------- //

void MasterDiagnostic::setCloseLoop(bool closeLoop) {
	
	if(_optLoopDiagnEnabled) {
		_logger->log(Logger::LOG_LEV_DEBUG, "");
		
		// Update the Optical Loop StatusTable
		if(closeLoop) {
			_logger->log(Logger::LOG_LEV_INFO, "Setting CLOSE optical loop...");
			_optLoopDiagnFramesNum = 2;	// Bcu39 and AdSecStream frames
         _optLoopDignFunnel->setFilottoCheck(true);
         _adsecStreamFramesDownloader->enable(true);
		}
		else {
			_logger->log(Logger::LOG_LEV_INFO, "Setting OPEN optical loop...");
			_optLoopDiagnFramesNum = 1; // Only Bcu39 frame
         _optLoopDignFunnel->setFilottoCheck(false);
         _adsecStreamFramesDownloader->enable(false);
		}
		_statusTableOptLoop->setExpectedFrames(_optLoopDiagnFramesNum);
		
		// Re-init the frame buffer headers for the Otpical Loop buffer
		initOptLoopFrameBufferHeaders();
		
		_logger->log(Logger::LOG_LEV_INFO, "...done");
	}
}

void MasterDiagnostic::techViewChangeFrameSize(Variable* var) {
	if(_techViewerFrameSize.MatchAndSet(var)) {
		_logger->log(Logger::LOG_LEV_INFO, "Changing frame size for technical viewer to %d pixels...", var->Value.Lv[0]);
		_techViewer->changeFrameSize(var->Value.Lv[0]);
	}
}




void MasterDiagnostic::Run() {
	
	_logger->log(Logger::LOG_LEV_INFO, "");
	_logger->log(Logger::LOG_LEV_INFO, "------------------- STARTING MASTER DIAGNOSTIC SYSTEM --------------------");
	Utils::setSchedulingRR(99);


	// --------------- Start frames funnels -------------- //
	if(_optLoopDignFunnel != NULL) {
		_logger->log(Logger::LOG_LEV_INFO, " > STARTING OPTICAL LOOP DIAGNOSTIC...");
		_optLoopDignFunnel->start();
	}
	if(_adSecDignFunnel != NULL) {
		_logger->log(Logger::LOG_LEV_INFO, " > STARTING ADAPTIVE SECONDARY DIAGNOSTIC...");
		_adSecDignFunnel->start();
	}
	
	if(_techViewer != NULL) {
		_logger->log(Logger::LOG_LEV_INFO, " > STARTING TECHNICAL VIEWER...");
		_techViewer->start();
	}

	// --------------- Start frames downloaders -------------- //
	if(_bcu39FramesDownloader != NULL) {
		_bcu39FramesDownloader->start();
	}
	
	if(_bcuSwitchFramesDownloader != NULL) {
		_bcuSwitchFramesDownloader->start();
	}

	if(_adsecStreamFramesDownloader != NULL) {
		_adsecStreamFramesDownloader->start();
	}
	
	for(int i=0; i< AdSecConstants::BcuMirror::N_CRATES; i++) {
		if(_bcuMirrorFramesDownloaders[i] != NULL) {
			_bcuMirrorFramesDownloaders[i]->start();
		}
	}

   // --------------- Start frame bouncer -----------------
   if (_adSecBouncer != NULL) {
      _adSecBouncer->start();
   }

	// This loop try to install support for runtime Tech Viewer 
	// frame size changing and wait for :
	// - Open/close loop for Optical Loop
	// - Tech Viewer frame size change
	// - TimeToDie 
	while(!TimeToDie()) {
		if(_techViewerEnabled && !_techViewRuntimeFrameChangeEnabled) {
			setupTechViewerVars();
		}
		msleep(1000);
	}
}

int main(int argc, char **argv) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

	MasterDiagnostic* md = NULL;
	try {
		md = new MasterDiagnostic(argc, argv);
		md->Exec();
	}
   catch (UdpFatalException *e) {
      printf("Exception: %s\n", e->what().c_str());
   }
	catch(LoggerFatalException &e) {
		// In this case the logger can't log!!!
      	printf("%s\n", e.what().c_str());
   	} 
	catch(AOException& e) {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
	}
	
	delete md;
	return 0;

}
