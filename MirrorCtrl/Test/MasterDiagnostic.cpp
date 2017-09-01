/*
 * ATTENTION: this is the first version of the MasterDiagnostic, not yet maintained !!!
 * The good and uodated version is in Supervisor/MasterDiagnostic.
 */

#include "MasterDiagnostic.h"

#include "BcuLib/BcuCommon.h"
#include "Commons/BcuPacket.h"
using namespace Arcetri;
using namespace Arcetri::Bcu;

#include <string>
using namespace std;

MasterDiagnostic::MasterDiagnostic(string remoteIp, int remotePort, string shmBufName, int frameSizeDw, int numFrames) {
	
	_logger = Logger::get("MASTERDIAGNOSTIC", Logger::LOG_LEV_DEBUG);
	
	// -- Setup udp connection
	// BCU is able to reply only on the same port :-(
	int localPort = remotePort;
	_conn = new UdpConnection(localPort, remoteIp, remotePort, 5);
	
	// -- Setup incoming frame
	_numFrames = numFrames;
	_forever = false;
	if(_numFrames == -1) {
		_forever = true;
	}
	_frame = new BYTE[frameSizeDw*Constants::DWORD_SIZE];
	_frameSizeBytes = frameSizeDw * Constants::DWORD_SIZE;
	memset(_frame, 0, _frameSizeBytes);
	_packetsPerFrame = _frameSizeBytes / Constants::MAX_PACKET_SIZE;
	if(_frameSizeBytes % Constants::MAX_PACKET_SIZE) {
		_packetsPerFrame++;
	}
	
	// -- Setup shared memory
	_shmBufInfo = new BufInfo();
	_shmCreatorName = "MASTERDIAGN";
	_shmBufferName = (char*)(shmBufName.c_str());
	thInit(_shmCreatorName);
	thStart("127.0.0.1", 1);
 	bufCreate(_shmCreatorName, 
 			  _shmBufferName, 
 			  BUFCONT, 
 			  _frameSizeBytes, 1,
 			  _shmBufInfo);
	
	_logger->log(Logger::LOG_LEV_INFO, "------- MASTER-DIAGNOSTIC CONSTRUCTED ------");
	_logger->log(Logger::LOG_LEV_INFO, "Remote address: \t%s:%d", remoteIp.c_str(), remotePort);
	_logger->log(Logger::LOG_LEV_INFO, "FrameSizeBytes:\t%d (%d dword)", _frameSizeBytes, _frameSizeBytes/Constants::DWORD_SIZE);
	_logger->log(Logger::LOG_LEV_INFO, "PacketsPerFrame:\t%d", _packetsPerFrame);
	_logger->log(Logger::LOG_LEV_INFO, "ShmCreatorName:\t%s", _shmCreatorName);
	_logger->log(Logger::LOG_LEV_INFO, "ShmBufferName:\t%s", _shmBufferName);
	_logger->log(Logger::LOG_LEV_INFO, "----------------------------------------");
}


MasterDiagnostic::~MasterDiagnostic() {

	bufRelease(_shmCreatorName, _shmBufInfo);
	delete _conn;
}
	
void MasterDiagnostic::start() {

	// Frames info
	uint16 downloadedFrames;	// Used also as "next expected frame"
	BYTE* frameReceiveBuffer = NULL;
	int shmBufWriteCounter;

	// Packets for frame info
	uint16 downloadedPackets;	// Used also as "next expected packet"
	int bytesToDownload;
	int curPacketSizeBytes;	// Except udp header
	BYTE packetReceiveBuffer_Storage[sizeof(DiagnosticUdpHeader) + Constants::MAX_PACKET_SIZE];
	BYTE* packetReceiveBuffer = (BYTE*)(&packetReceiveBuffer_Storage);
	
	// Loop for numFrames (or forever)
	bool frameSkipped = false;
	downloadedFrames = 0;
	shmBufWriteCounter = 0;
	_logger->log(Logger::LOG_LEV_INFO, "Downloading %d frames... (-1=infinite)", _numFrames);
	while(_forever || downloadedFrames < _numFrames) {
		
		try {
			// --- Download a frame
			_logger->log(Logger::LOG_LEV_DEBUG, "");
			_logger->log(Logger::LOG_LEV_DEBUG, "Downloading diagnostic frame %d...", downloadedFrames);
			downloadedPackets = 0;
			
			bytesToDownload = _frameSizeBytes;
			frameReceiveBuffer = _frame;
			
			// --- Download all packets composing the frame
			while(downloadedPackets < _packetsPerFrame) {
				
				// Compute the size of data to download in the current packet
				if(bytesToDownload < Constants::MAX_PACKET_SIZE) {  // Last packet (not full)
					curPacketSizeBytes = bytesToDownload;
				}
				else {
					curPacketSizeBytes = Constants::MAX_PACKET_SIZE;
				}
				
				// Receive the packet
				try {
					_conn->receive((BYTE*)(&packetReceiveBuffer), curPacketSizeBytes + sizeof(DiagnosticUdpHeader));
					checkPacket(packetReceiveBuffer, downloadedFrames, downloadedPackets); // Can throw InvalidFrameException
					frameSkipped = false;
					memcpy(frameReceiveBuffer, packetReceiveBuffer+sizeof(DiagnosticUdpHeader), curPacketSizeBytes);
					frameReceiveBuffer += curPacketSizeBytes; 
					bytesToDownload -= curPacketSizeBytes;	
//					_logger->log(Logger::LOG_LEV_DEBUG, " > Got packet %d of size %d bytes (%d dword)", downloadedPackets, curPacketSizeBytes, curPacketSizeBytes/Constants::DWORD_SIZE);
					downloadedPackets++;
					
				}
				catch(UdpTimeoutException& e) {
					//_logger->log(Logger::LOG_LEV_WARNING, " > Packet timed out...");		  
				}
			}
			
			// --- Test: print frame !!! ---
//			_logger->log(Logger::LOG_LEV_DEBUG, " ----------------------- FRAME %d -----------------------", downloadedFrames);
//			for(int i = 0; i < _frameSizeBytes; i++) {
//				printf("%08X", _frame[i]);
//			}
//			printf("\n");
			// ---------------------------------------		
			
			// --- Copy it to shared memory
			bufWrite(_shmBufInfo, _frame, ++shmBufWriteCounter, 0);
			downloadedFrames++;
		}
		catch(InvalidDiagnosticPacketException& e) { 
			_logger->log(Logger::LOG_LEV_WARNING, " > Frame discarded !");
			if(!frameSkipped) {
				downloadedFrames++;	// Increase it to wait the next frame !
				frameSkipped = true;
			}
		}
	}
	_logger->log(Logger::LOG_LEV_INFO, "Downloading of %d frames completed", _numFrames);

}


void MasterDiagnostic::checkPacket(BYTE* rawPacket, uint16 frameId, uint16 packetId) throw(InvalidDiagnosticPacketException){
	
	DiagnosticUdpHeader* header = (DiagnosticUdpHeader*)rawPacket;
	if(header->frameId != frameId || header->packetId != packetId) {
		_logger->log(Logger::LOG_LEV_WARNING, " > Got unexpected packet %d for frame %d ", header->packetId, header->frameId);
		throw InvalidDiagnosticPacketException();
	}
}

int main(int argc, char* argv[]) {
	if(argc < 5)  {
		printf("\tUsage: 'MasterDiagnostic ip port shmBufName frameSizeDw [frameNum]'\n");
		printf("\t\tIf frameSizeDw is expressed in DWORDS\n");
		printf("\t\tIf frameNum not specified, loop forever\n");
	}
	else {
		MasterDiagnostic* md;
		
		string ip = string(argv[1]);
		int port = atoi(argv[2]);
		string shmBufName = string(argv[3]);
		int	frameSize = atoi(argv[4]);
		if(argc == 6) {
			int	frameNum  = atoi(argv[5]);
			md = new MasterDiagnostic(ip, port, shmBufName, frameSize, frameNum);
		}
		else {
			md = new MasterDiagnostic(ip, port, shmBufName, frameSize);
		}
		
		md->start();
		delete md;
	}
}
