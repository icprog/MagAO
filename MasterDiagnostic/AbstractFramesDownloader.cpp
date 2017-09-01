#include "AbstractFramesDownloader.h"
#include "Utils.h"


extern "C" {
#include "base/timelib.h"
}



using namespace Arcetri;

AbstractFramesDownloader::AbstractFramesDownloader(string bcuName, string remoteIp, int remotePort, 
								  		 		   int frameSizeDw, int numFrames) {
	
	_logger = Logger::get(bcuName);
	_bcuName = bcuName;
	
	// -- Setup udp connection
	// NOTES:
	//   - BCU is able to reply only on the same port :-(
	//   - The udp receive timeout is set to wait-forever (0), because in general
	//     the AbstractFramesDownloader doesn't need to loop if nothing is received
	//	   (for an exception see TechnicalViewer's constructor)
	int localPort = remotePort;
	_conn = new UdpConnection(localPort, remoteIp, remotePort, 0);
	
	// -- Setup the number of frames to download
	_numFrames = numFrames;
	
	// -- Setup the frame size
	_frame = NULL;
	_frameSizeBytes = 0;
	_packetsPerFrame = 0;
	_frameSizeChanging = false;
	pthread_mutex_init(&_frameSizeChanged_mutex, NULL);
   _MAX_TDP_PACKET_SIZE =  Constants::MAX_TDP_PACKET_SIZE;
	if(frameSizeDw != 0) {
		changeFrameSize(frameSizeDw);
	}
	_logger->log(Logger::LOG_LEV_INFO, "");
	_logger->log(Logger::LOG_LEV_INFO, "--- FRAMES-DOWNLOADER %s CONSTRUCTED ---", _bcuName.c_str());
	_logger->log(Logger::LOG_LEV_INFO, "> Remote address:  %s:%d", remoteIp.c_str(), remotePort);
	_logger->log(Logger::LOG_LEV_INFO, "> FrameSizeBytes:  %d (%d dword)", _frameSizeBytes, _frameSizeBytes/Constants::DWORD_SIZE);
	_logger->log(Logger::LOG_LEV_INFO, "> PacketsPerFrame: %d", _packetsPerFrame);
}

AbstractFramesDownloader::~AbstractFramesDownloader() {
	delete _conn;
	delete _frame;
	_logger->log(Logger::LOG_LEV_INFO, "FRAMES-DOWNLOADER %s succesfully destroyed !", _bcuName.c_str());
}


void AbstractFramesDownloader::changeFrameSize(int frameSizeDw) {

	// Critical session with main loop
	_logger->log(Logger::LOG_LEV_INFO, "FramesDownloader %s: HDLR requested for changing frame size...", _bcuName.c_str());
	_frameSizeChanging = true;
	pthread_mutex_lock(&_frameSizeChanged_mutex);	
	_logger->log(Logger::LOG_LEV_INFO, "FramesDownloader %s: HDLR allowed to change frame size...", _bcuName.c_str());
	delete _frame;
	_frame = NULL;
	
	// -- Setup the incoming frame
	_frameSizeBytes = frameSizeDw * Constants::DWORD_SIZE;
	_frame = new BYTE[_frameSizeBytes];
	_frameHeader = _frame;
 	_frameFooter = _frame + _frameSizeBytes - _FRAME_HEADER_SIZE_DW*Constants::DWORD_SIZE;
	_packetsPerFrame = _frameSizeBytes / _MAX_TDP_PACKET_SIZE;
	if(_frameSizeBytes % _MAX_TDP_PACKET_SIZE) {
		_packetsPerFrame++;
	}
	std::cout << _packetsPerFrame << "\n";
   
	_logger->log(Logger::LOG_LEV_INFO, "FramesDownloader %s: HDLR frame size succesfully changed !", _bcuName.c_str());
	_frameSizeChanging = false;
	pthread_mutex_unlock(&_frameSizeChanged_mutex);	
}

pthread_t AbstractFramesDownloader::start() {
	pthread_t framesDownloader;
	pthread_create(&framesDownloader, NULL, &execute, (void*)this);
    return framesDownloader;
}


void* AbstractFramesDownloader::execute(void* thisPtr) {

	// Set a Real-Time priority for this thread
	Utils::setSchedulingRR(99);

	AbstractFramesDownloader* aFd = (AbstractFramesDownloader*)thisPtr;
	aFd->run();
	return NULL;
}


void AbstractFramesDownloader::run() {
	
	// --- Frames info --- //
	//bool previousFrameCompleted;
	int currentFrameId;
	int remainingFrameBytes; // Bytes to download for the current frame
	BYTE* frameCursor;		 // Used to traverse the _frame while receiving packets
	
	// --- Packets info --- //
	bool packetGot;
	int downloadedPackets;		// Used also as "next expected packet"
	int curPacketSizeBytes;		// Full size of the current packet in download, except the udp header
	
	BYTE packetReceiveBuffer[Constants::TDP_PACKET_HEADER_SIZE + _MAX_TDP_PACKET_SIZE];
	DiagnosticUdpHeader* header = (DiagnosticUdpHeader*) &((TDPHeader*)packetReceiveBuffer)->header; // Only to simplify the code
	
	packetGot = false; 
	
	// --- Loop forever, without any fear !!! --- //
	while(true) {
		// PRevent from frame size changed
		pthread_mutex_lock(&_frameSizeChanged_mutex);
		_logger->log(Logger::LOG_LEV_INFO, "FramesDownloader %s: READY TO DOWNLOAD FRAMES OF SIZE %d BYTES (0 = disabled)...", _bcuName.c_str(), _frameSizeBytes);
		
		
		// Check a frame size change by the changeFrameSize handler
		while(!_frameSizeChanging) {

			// The diagostic is disabled if _frameSizeBytes is zero
			if(_frameSizeBytes != 0) {
				
				try {
					// (0) --- RESET THE FRAME STATUS --- //
					downloadedPackets = 0; 
					remainingFrameBytes = _frameSizeBytes;
					frameCursor = _frame;
				
					// (1) --- RECEIVE ZERO PACKET--- //
					// The ZERO packet can be already received previously as an unexpected one...
					if(packetGot) {
						// Discard the previous frame
						_logger->log(Logger::LOG_LEV_WARNING, "FramesDownloader %s: frame %d (not completed) succesfully discarded !\n", _bcuName.c_str(), currentFrameId);
					}
					//.. or we must try to receive it now
					else {
						curPacketSizeBytes = computePacketSize(remainingFrameBytes);
						_conn->receive(packetReceiveBuffer, curPacketSizeBytes + Constants::TDP_PACKET_HEADER_SIZE);					
						packetGot = true;
					}
					
					// (2) --- ATTACH THE NEW FRAME  --- //
					currentFrameId = header->frameId;
					// Succesfully attached
					if(header->packetId == 0) {
						_logger->log(Logger::LOG_LEV_INFO, "FramesDownloader %s: downloading diagnostic frame %d...", _bcuName.c_str(), currentFrameId);

               /*   
               printf("Raw packet dump, %d bytes:\n", curPacketSizeBytes + Constants::TDP_PACKET_HEADER_SIZE);
                  for (int i=0; i< curPacketSizeBytes/4 + Constants::TDP_PACKET_HEADER_SIZE/4; i++) {
                  if (i%4 == 0)
                     printf("\n%08X: ", i);
                      printf("%08X ", ((uint32*) packetReceiveBuffer)[i]);
                   }
                  printf("\n");
                 */
                           

						
						// Save the packet in the correct frame slot
						memcpy(frameCursor, packetReceiveBuffer + Constants::TDP_PACKET_HEADER_SIZE, curPacketSizeBytes);

						
						// Update the frame status
						frameCursor += curPacketSizeBytes; 
						remainingFrameBytes -= curPacketSizeBytes;	
						downloadedPackets++;
						
						_logger->log(Logger::LOG_LEV_DEBUG, " > FramesDownloader %s: got packet %d (frame %d) of size %d bytes (%d dword)", _bcuName.c_str(), 0, currentFrameId, curPacketSizeBytes, curPacketSizeBytes/Constants::DWORD_SIZE);
                  /*
                     printf("Correct frame:\n");
			       		dumpPacketHeader(header);
                     */
						packetGot = false;
					}
					// Not attached
					else {
						// Was waiting for the ZERO packet !!!
						packetGot = false;
						throw InvalidDiagnosticPacketException(currentFrameId, currentFrameId, 0, header->packetId);
					}
					
					// (3) --- RECEIVE NOT-ZERO PACKETS FOR ATTACHED FRAME --- //
					while(downloadedPackets < _packetsPerFrame) {
						// Receive a packet
						curPacketSizeBytes = computePacketSize(remainingFrameBytes);
						_conn->receive(packetReceiveBuffer, curPacketSizeBytes + Constants::TDP_PACKET_HEADER_SIZE);					
						packetGot = true;
						
						// Check if is the next expected packet for the current frame
						checkPacket(currentFrameId, header->frameId, downloadedPackets, header->packetId);

                  /*
               printf("Raw packet dump, %d bytes:\n", curPacketSizeBytes + Constants::TDP_PACKET_HEADER_SIZE);
                  for (int i=0; i< curPacketSizeBytes/4 + Constants::TDP_PACKET_HEADER_SIZE/4; i++) {
                  if (i%4 == 0)
                      printf("\n %08X: ", i);
                      printf("%08X ", ((uint32*) packetReceiveBuffer)[i]);
                   }
                    printf("\n");

                     printf("Correct frame:\n");
			       		dumpPacketHeader(header);
                     */
						
						// Save the packet in the correct frame slot
						memcpy(frameCursor, packetReceiveBuffer + Constants::TDP_PACKET_HEADER_SIZE, curPacketSizeBytes);
						
						
						// Update the frame status
						frameCursor += curPacketSizeBytes; 
						remainingFrameBytes -= curPacketSizeBytes;	
						downloadedPackets++;
						_logger->log(Logger::LOG_LEV_TRACE, "FramesDownloader %s: got packet %d (frame %d) of size %d bytes (%d dword)", _bcuName.c_str(), downloadedPackets-1, currentFrameId, curPacketSizeBytes, curPacketSizeBytes/Constants::DWORD_SIZE);
						packetGot = false;
					}
					
					// (4) --- STORE THE FRAME --- //
					if(_logger->getLevel() >= Logger::LOG_LEV_INFO) {
			       		dumpFrameHeader();
			        }

               /*
               printf("Frame dump:\n");
                  printf("%s\n", _bcuName.c_str());
                  for (int i=0; i< _frameSizeBytes/4; i++) {
                  if (i%4 == 0)
                      printf("\n%08X: ", i);
                      printf("%08X ", ((uint32*) _frame)[i]);
                   }
                      printf("\n");
                */           

					checkFrame(); // Can throw InvalidFrameException (derived from FramesDownloaderException)
		            storeFrame();
				}
				catch(UdpTimeoutException& e) {
					_logger->log(Logger::LOG_LEV_DEBUG, "FramesDownloader %s timed out... (waiting %d packets for frame)", _bcuName.c_str(), _packetsPerFrame);
				}
				// Invalid packet: unexpected
				catch(InvalidDiagnosticPacketException& e) {
					//previousFrameCompleted = false;
					// Dump the packet header
			        if(_logger->getLevel() >= Logger::LOG_LEV_TRACE) {
                     printf("Wrong frame:\n");
			       		dumpPacketHeader(header);
			        }
					_logger->log(Logger::LOG_LEV_WARNING, "FramesDownloader %s: %s", _bcuName.c_str(), e.what().c_str());
					_logger->log(Logger::LOG_LEV_WARNING, "FramesDownloader %s: packet succesfully ignored !", _bcuName.c_str());
				}
				// Invalid frame: headers and footers don't match (threated as completed)
				catch(InvalidDiagnosticFrameException& e) {
					// Dump the frame header
			        if(_logger->getLevel() >= Logger::LOG_LEV_DEBUG) {
			       		dumpFrameHeader();
			        }
					_logger->log(Logger::LOG_LEV_WARNING, "FramesDownloader %s: %s", _bcuName.c_str(), e.what().c_str());
					_logger->log(Logger::LOG_LEV_WARNING, "FramesDownloader %s: frame %d succesfully discarded !\n", _bcuName.c_str(), currentFrameId);
				}
			}
			// Do nothing
			else {
				msleep(500);
			}
		}
	
		_logger->log(Logger::LOG_LEV_INFO, "FramesDownloader %s: waiting frame size changed...", _bcuName.c_str());
		pthread_mutex_unlock(&_frameSizeChanged_mutex);
		while(_frameSizeChanging) {
			_logger->log(Logger::LOG_LEV_INFO, "FramesDownloader %s: waiting frame size changed...", _bcuName.c_str());
			msleep(100);
		}
		_logger->log(Logger::LOG_LEV_INFO, "FramesDownloader %s: frame size changed !!!", _bcuName.c_str());
	}
}

int AbstractFramesDownloader::computePacketSize(int packetBytesToDownload) {
	if(packetBytesToDownload < _MAX_TDP_PACKET_SIZE) {
		return packetBytesToDownload;
	}
	else {
		return _MAX_TDP_PACKET_SIZE;
	}
}


void AbstractFramesDownloader::checkPacket(int expectedFrameId, int currentFrameId, int expectedPacketId, int currentPacketId) throw(InvalidDiagnosticPacketException) {
	if(currentFrameId != expectedFrameId || currentPacketId != expectedPacketId) {
		throw InvalidDiagnosticPacketException(expectedFrameId, currentFrameId, expectedPacketId, currentPacketId);
	}
}


void AbstractFramesDownloader::checkFrame() throw(InvalidDiagnosticFrameException) {
	if(memcmp(_frameHeader, _frameFooter, _FRAME_HEADER_SIZE_DW*Constants::DWORD_SIZE) != 0) {
	   throw InvalidDiagnosticFrameException();
	}
}

void AbstractFramesDownloader::dumpFrameHeader() {
	
	char str[200];
	memset(str, 0, 200);
	
    for (int i=0; i<4; i++) {
       	sprintf(str+i*11, " 0x%08X", ((uint32 *)_frameHeader)[i]);
    }
    _logger->log(Logger::LOG_LEV_INFO, (string)"FramesDownloader " + _bcuName + (string)": frame header:" + str);
    
//    _logger->log(Logger::LOG_LEV_INFO, (string)"FramesDownloader " + _bcuName + (string)" full frame: ");
//    for (int i=0; i<_frameSizeBytes/Constants::DWORD_SIZE; i++) {
//       	printf(" 0x%08X", ((uint32 *)_frame)[i]);
//    }
//    printf("\n");
    
    memset(str, 0, 200);
    for (int i=0; i<4; i++) {
       sprintf(str+i*11, " 0x%08X", ((uint32 *)_frameFooter)[i]);
    }
    _logger->log(Logger::LOG_LEV_INFO, (string)"FramesDownloader " + _bcuName + (string)": frame footer:" + str);
}

void AbstractFramesDownloader::dumpPacketHeader(DiagnosticUdpHeader* header) {
    for (int i=0; i<3; i++)
        printf("0x%08X\n", ((uint32 *)header)[i]);

    printf(" > frameID : %d\n", header->frameId);
    printf(" > packetID: %d\n", header->packetId);
    printf(" > tot_len : %d\n", header->tot_len);
    printf(" > saddr   : %d\n", header->saddr);
}
