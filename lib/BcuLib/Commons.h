#ifndef CONSTANTS_H_INCLUDE
#define CONSTANTS_H_INCLUDE

#include "sys/types.h"
#include "aotypes.h"
#include <string>

#include "BcuLib/PacketHeaders.h"
#include "WfsConstants.h"
#include "AdSecConstants.h"

using namespace std;
using namespace Arcetri;
using namespace Arcetri::Bcu;

namespace Arcetri {

	namespace Bcu {
		
		/*
		 * Poll the sdram to know if one or more BCUs are ready.
		 * 
		 * The polling is performed for timeout_s seconds, with a delay of delay_ms ms
		 * at each loop. Polls at least one time before checking if the timeout expired.
		 * 
		 * The bcu id can be retrieved by name using the class BcuIdMap.
		 * Note that only Bcu ranges admitted by MirrorCtrl can be addressed 
		 * (BCU_MIRROR_0 - BCU_MIRROR_X and subranges, depending on AdSecConstants.h)
		 * 
		 */
		bool isBcuReady(int firstBcuId, int lastBcuId, double timeout_s, int delay_ms = 1000);
		

		/*
		 * @Class{API}: Constants
		 * Generic constants about the BCU
		 * 
		 * All configuration info about the BCUs are stored in:
		 *  - AdSecConstants.h
		 *  - WfsConstants.h
		 * @
		 */
		class Constants {
		
			public:
			
				static const int MAX_BCU_NUM = 9;
		
				//--- COMMONS ---//
				static const int MAX_ETH_PACKET_SIZE = 1470;
				static const int MAX_ID = 255;
		    	static const int DWORD_SIZE = sizeof(DWORD);
		
				//--- MGP PACKET ---//
		    	static const int PACKET_HEADER_SIZE = sizeof(UdpHeader);
		    	static const int MAX_PACKET_SIZE = MAX_ETH_PACKET_SIZE - PACKET_HEADER_SIZE;	// Except the header !
				static const int MAX_PACKET_SIZE_DW = MAX_PACKET_SIZE/DWORD_SIZE;				// Except the header ! 
					
				static const uint8 WANTREPLY_FLAG 	= 0x01;
				static const uint8 ASQUADWORD_FLAG 	= 0x02;
		
		    	// -- TDP packet -- //
		    	static const int TDP_PACKET_HEADER_SIZE = sizeof(TDPHeader);
		       	static const int MAX_TDP_PACKET_SIZE = MAX_ETH_PACKET_SIZE - TDP_PACKET_HEADER_SIZE; // Except the header !
		       	static const int MAX_TDP_PACKET_SIZE_DW = MAX_TDP_PACKET_SIZE/DWORD_SIZE; 			// Except the header ! 
		};
		
		/*
		 * Defined BCU memory types
		 */
		class MemoryTypes {
			
			public:
				static const int DSP_MEMORY	  = 0;
				static const int SRAM_MEMORY  = 1;
				static const int SDRAM_MEMORY = 2;
				static const int FPGA_MEMORY  = 3;
		};
		
		/*
		 * Defines BCU commands
		 * 
		 * Note that "SEQ" or "SAME" is interpreted (by MirrorCtrl) for ***both*** DSP 
		 * and BCU range addressed.
		 */
		class OpCodes {
			
			public:
				static bool isSuccess(int opcode) { return opcode == MGP_OP_CMD_SUCCESS; }
				static bool isFault(int opcode) { return opcode == MGP_OP_CMD_FAULT; }
				static bool isCATimeout(int opcode) { return opcode == MGP_OP_CMD_TIMEOUT; }
				static bool isRead(int opcode);
				static bool isWrite(int opcode);
				static bool isSame(int opcode);
				static bool isSeq(int opcode);
				static int  getDspFactor(int opcode);
				
				static int getReadOpCode(string memType);
				
				static bool isReadSdram(int opcode) { return opcode == MGP_OP_RDSEQ_SDRAM; }
			
			public:

				static const int MGP_OP_WRSAME_DSP = 0;	// Write same on DSP and BCU (by MirrorCtrl)
				static const int MGP_OP_WRSEQ_DSP  = 1;	// Write seq on DSP and BCU (by MirrorCtrl)
				static const int MGP_OP_RDSEQ_DSP  = 2; // Read seq on DSP and BCU (by MirrorCtrl)
			
				static const int MGP_OP_RESET_DEVICES	  = 10;		// Resets the specified devices
				static const int MGP_OP_WRRD_RELAIS_BOARD = 11; 	// Write to relay board
				static const int MGP_OP_LOCK_FLASH		  = 128;	// Lock flash area
				static const int MGP_OP_UNLOCK_FLASH	  = 129;	// Unlock a portion of the flash area
				static const int MGP_OP_CLEAR_FLASH		  = 130;	// Clear a portion of the flash area
				static const int MGP_OP_WRITE_FLASH		  = 131;	// Write into flash area
				static const int MGP_OP_RDSEQ_FLASH		  = 132;	// Read from flash area
			
				static const int MGP_OP_WR_SCIMEASURE_RAM = 135;	// 0x87 MGP scimeasure ram write
				static const int MGP_OP_WRITE_SIGGEN_RAM  = 136;   	// 0x88 MGP signal generator ram write
				static const int MGP_OP_WRITE_WAVEGEN_RAM = 137; 	// Write into waveform generator RAM
			
				static const int MGP_OP_CLEAR_SDRAM	 = 140;	// Clear a portion of the SDRAM
				static const int MGP_OP_WRSAME_SDRAM = 141;	// Write same into SDRAM
				static const int MGP_OP_WRSEQ_SDRAM	 = 142;	// Write seq into SDRAM
				static const int MGP_OP_RDSEQ_SDRAM	 = 143;	// Read seq into SDRAM
			
				static const int MGP_OP_CLEAR_SRAM	= 145;	// Clear a portion of the SRAM
				static const int MGP_OP_WRSAME_SRAM	= 146;	// Write same into SRAM
				static const int MGP_OP_WRSEQ_SRAM	= 147;	// Write seq into SRAM
				static const int MGP_OP_RDSEQ_SRAM	= 148;	// Read seq from SRAM
			
				static const int MGP_OP_WRSAME_DIAGBUFF	= 150;	//  0x96 MGP diagnostic buffer ram write same buffer
				static const int MGP_OP_WRSEQ_DIAGBUFF	= 151;  //  0x97 MGP diagnostic buffer ram write seq buffer
				static const int MGP_OP_RDSEQ_DIAGBUFF	= 152;  //  0x98 MGP diagnostic buffer ram read buffer

            	static const int MGP_OP_WRITE_CCDI  = 155;   // CCD interface
            	static const int MGP_OP_READ_CCDI  = 156;   // CCD interface
			
				static const int MGP_OP_CMD_SUCCESS	= 200;	// Command successfull
				static const int MGP_OP_CMD_FAULT	= 201;	// Command error
				static const int MGP_OP_CMD_TIMEOUT	= 202;	// BCU internal timeout
				static const int MGP_OP_CMD_WARNING	= 203;	// Command warning - to be defined
			
				static const int MGP_OP_RESERVED_1 = 240;   //  0xf0 MGP reserved command (utilizzando per rimappare il crate ID delle schede DSP)
				static const int MGP_OP_RESERVED_2 = 241;   //  0xf1 MGP reserved command (comando che fa partire la sequenza di test e taratura degli otto canali dell'ADC)
				static const int MGP_OP_RESERVED_3 = 242;   //  0xf2 MGP reserved command (comando che fa partire la sequenza di test e taratura degli otto canali del DAC)
			
				static const int MGP_OP_CMD_NULL = 255;		// Null command - no effect	
		};
	
		/*
		 *  Defines ext BCU commands
		 */
		class ExtOpCodes {
			
			public:
				static const int  MGP_EXT_CMD_TIMEOUT = 254;
		};
		
	}

}

#endif /*CONSTANTS_H_INCLUDE*/
