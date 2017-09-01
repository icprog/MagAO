#ifndef PACKETHEADERS_H_INCLUDE
#define PACKETHEADERS_H_INCLUDE

namespace Arcetri {

	namespace Bcu {
		
		// ----------------------- STRUCTS ------------------------ //
		// -------------------------------------------------------- //
		
		/*
		 * @Struct: UdpHeader
		 * UdpPacket header (alias MGP)
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
		struct UdpHeader {
			uint16 	dummy_word;
		    uint8 	DSPs[3];
		    uint8 	opCode;
		    uint16 	dataLength;		// For each DSP (in DWORD)
		    uint8 	flags;
		    uint8 	msgId;
		    uint8 	memoryAddress[4];
		};


		/*
		 * @Struct: DiagnosticUdpHeader
		 * DiagnosticUdpPacket header (alias TDP)
		 *
		 * DWORD #0:   31..........16 15..........0
		 *             	   (dummy)         (tot_len)
		 *
		 * DWORD #1:   31..........16 15..........0
		 *                 (saddr)      (packetId) 
		 *
		 * DWORD #2:   31..........16
		 *                (frameId)
		 * 
		 * The TDPHeader is only necessary to correctly allocate the
		 * header buffer because of the misalignment of DiagnosticUdpHeader !
		 * @
		 */
		struct DiagnosticUdpHeader {
	        uint32 tot_len;
	        uint32 saddr;
	        uint16 packetId; 	// Restart from zero for each frameId
	        uint16 frameId;     // Frame identifier
	    };
	    
	    /*
	     * @Struct: TDPHeader
	     * @See{DiagnosticUdpHeader}
	     * @
	     */
	    struct TDPHeader {
	    	uint16 dummy;
	        uint16 header[6];
	    };
	    
	}
}
#endif /*PACKETHEADERS_H_INCLUDE*/
