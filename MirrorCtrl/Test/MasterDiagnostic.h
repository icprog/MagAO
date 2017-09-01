/*
 * ATTENTION: this is the first version of the MasterDiagnostic, not yet maintained !!!
 * The good and uodated version is in Supervisor/MasterDiagnostic.
 */


#ifndef MASTERDIAGNOSTIC_H_INCLUDE
#define MASTERDIAGNOSTIC_H_INCLUDE

#include "UdpConnection.h"
#include "aotypes.h"
#include "AOExcept.h"
#include "Logger.h"
using namespace Arcetri;

extern "C" {
	#include "base/thrdlib.h"
}


/*
 * DiagnosticUdpPacket header (from commlib::BCU_commandbuffer)
 * 
 * [todo] To check !!!
 *
 * DWORD #0:   31......20 19........8 7...0
 *             (firstDSP)  (lastDSP)  (command)
 *
 * DWORD #1:   31...........16 15...8 7...0
 *                (length)     (flags) (ID)
 *
 * DWORD #2:   31.........................0
 *                (memory address)
 */
typedef struct {	
    uint16 frameId;		// Frame identifier
    uint16 packetId;	// Restart from zero for each frameId
    uint16 tot_len;
    uint16 saddr;
    uint32 tdp_data[]; 
} DiagnosticUdpHeader;


class InvalidDiagnosticPacketException:AOException {
	public:
		InvalidDiagnosticPacketException():AOException("Frame not got") { exception_id = "InvalidFrameException";}
		~InvalidDiagnosticPacketException() {}
};

class MasterDiagnostic {

	public:

		MasterDiagnostic(string remoteIp, int remotePort, string shmBufName, int frameSizeDw, int numFrames = -1);
		virtual ~MasterDiagnostic();
	
		void start();
		
	private:
	
		void checkPacket(BYTE* rawPacket, uint16 frameId, uint16 packetId) throw(InvalidDiagnosticPacketException);
	
		char* _shmCreatorName;
		char* _shmBufferName;
		
		int   _numFrames;
		bool  _forever;		// Set to true if _numFrames == -1
		int   _frameSizeBytes;
		int	  _packetsPerFrame;
		BYTE* _frame;
	
		UdpConnection* _conn;
		
		BufInfo* _shmBufInfo;
		
		Logger* _logger;

};


#endif
