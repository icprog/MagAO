// commlib.h



// UDP communication library header file

#ifndef COMMLIB_H_INCLUDED
#define COMMLIB_H_INCLUDED

#include <netinet/in.h>
#include <arpa/inet.h>
#include "aotypes.h"

#define USE_NEW_OPCODES


// ------------------------------------------------------------------------- //
// --- ALL THE FOLLOWING FUNCTIONS ARE NOT YET USED IN THE NEW MirrorCtr --- //
// ------------------------------------------------------------------------- //
// >> All code used putside the MirrorCtrl have been moved to BcuCommon library. 
// >> The "define" have been replaced by static constants in MirrorCtrl/BcuPacket->UdpHeader, 
//    because they arent't used outside the MirrorCtrl !

// Max. data size for each UDP packet (in 32 bit words)
#define MAX_DATA_TRANSFER	(1456/4)

// Bit mask for the WantReply flag
#define FLAG_WANTREPLY		0x01
#define FLAG_ASQUADWORD		0x02

// Maximum UDP packet length
#define MAX_UDP_PACKET_LENGTH 65536	// Wrong !!!

// Warning: the following macros may evaluate their parameters more than one time.
// Be careful when choosing calling style.
// Example:
//
// n = GetDataLength( GetNextBuffer());         // Unsafe (may call GetNextBuffer() multiple times)
//
//
// cmd = GetNextBuffer();
// n = GetDataLength(cmd);                                      // Safe (argument is static data)
//
// Buffer is defined as follows:
//
// DWORD #0:   31......20 19........8 7...0
//             (firstDSP)  (lastDSP)  (command)
//
// DWORD #1:   31...........16 15...8 7...0
//                (length)     (flags) (ID)
//
// DWORD #2:   31.........................0
//                (memory address)


// Binary command buffer
// >> Replaced bu MirrorCtrl/BcuPacket->UdpHeader. Isn't used outside the MirrorCtrl !
//
// Unfortunately, we lack a 12-bit integer type....
struct BCU_commandbuffer
{
        uint16 dummy_word;

        uint8 DSPs[3];
        uint8 opcode;

        uint16 length;
        uint8 flags;
        uint8 msgID;

        uint8 address[4];
};

struct BCU_commandbuffer_read
{
        struct BCU_commandbuffer cmd;
        uint8 readlength[4];
};

typedef struct BCU_commandbuffer BCUcommand;
typedef struct BCU_commandbuffer_read BCUcommand_read; //Seems unused...

#define MPG_HEADERLEN (sizeof(DSP_commandbuffer))

// Macros to extract parameters from a binary command buffer
#define GetFirstDSP(a)          ( (a).DSPs[0] <<4 | ((a).DSPs[1] & 0xF0)>>4)
#define GetLastDSP(a)           ( ((a).DSPs[1] & 0x0F) << 8 | (a).DSPs[2])
#define GetCmdCode(a)           ( (a).opcode)
#define GetDataLength(a)        ( (a).length)
#define GetMemoryAddress(a)     ( (a).address[0])
#define GetMsgID(a)             ( (a).msgID)
#define GetFlag_wantreply(a)    ( (a).flags & FLAG_WANTREPLY)
#define GetFlag_asquadword(a)   ( (a).flags & FLAG_ASQUADWORD)
#define GetReadLength(a)        ( (a).readlength[0] | (a).readlength[1]<<8 | (a).readlength[2]<<16 | (a).readlength[3]<<24)


// Macros to write parameters into a binary command buffer
#define SetFirstDSP(a,b)                (((a).DSPs[0] = (b&0x00F)<<4), ((a).DSPs[1] = (b&0x0F0)>>4)) //Wrong ?!
#define SetLastDSP(a,b)                 ((a).DSPs[2] = (b&0xFF)) 



#define SetCmdCode(a,b)                 ( (a).opcode = b)
#define SetDataLength(a,b)              ( (a).length = b)
#define SetMemoryAddress(a,b)           ( (a).address[0] = b&0xFF, (a).address[1] = (b>>8)&0xFF, (a).address[2] = (b>>16)&0xFF, (a).address[3]=(b>>24)&0xFF)
#define SetMsgID(a,b)                   ( (a).msgID = b)
#define SetFlag_wantreply(a,b)          ( ((a).flags |= (b)? FLAG_WANTREPLY:0x00), ((a).flags &= (b)? 0xFF:~FLAG_WANTREPLY))
#define SetReadLength(a,b)              ( (a).readlength[0] = b&0xFF, (a).readlength[1] = (b>>8)&0xFF, (a).readlength[2] = (b>>16)&0xFF, (a).readlength[3]=(b>>24)&0xFF)


int IsDataMemory( int address);


//-------------------------------------------------------------------------- //
//                           Generic UDP functions                           //
// ------------------------------------------------------------------------- //
// >> Replaced by UdpConnection class


/*
 * Wait on socket to receive an UDP packet
 */
int WaitUDPpacket(
	       	int port,					// port to wait on 
	       	char *buffer,				// buffer to receive data (previously allocated!)
	       	int bufferlen,				// max buffer length
	       	int *token,					// connection token - should be a pointer to a zero integer the first time
			int timeout,				// timeout value in milliseconds (zero = forever)
			struct sockaddr_in *from);	// filled with client information

/*
 * Send an UDP packet by socket
 */
int SendUDPpacket(
	       	char *buffer,				// Pointer to buffer of data
	       	int bufferlen,				// buffer length in bytes
			unsigned int remote_addr,	// Remote hostname inet format 
			int remote_port,			// Remote port
			int *socket_out);			// pointer to socket to use (optional, set to 0 if no existing sockets are to be used)


/*
 * Wrapper on SendUDPpacket, but resolves host names
 */
int SendUDPpacketToHost(
	       	char *buffer,		// Pointer to buffer of data
	       	int bufferlen,		// buffer length in bytes
			char *remote_addr,	// Remote hostname or IP address
			int remote_port,	// Remote port
			int *socket_out);	// pointer to socket to use (optional, set to 0 if no existing sockets are to be used)


/*
 * Create the socket. Internally used by SendUDPpacket and WaitUDPpacket
 */
int CreateUDPsocket(int port,		// Port to wait on
					int *token);	// connection token (initial value discarded)

/*
 * Delete the socket
 */
int DeleteUDPSocket(int token);


// -------------------------------------------------------------------------
// DSP communication routines - Obsoleted: use high-level functions instead
// -------------------------------------------------------------------------

// Write multiple buffers to multiple DSP

int WriteToDSPSeq(
		char *remote_addr,		// remote IP address in dotted format
		int firstDSP,			// First DSP to be addressed
		int lastDSP,			// Last DSP to be adressed
		unsigned int address,		// address in DSP memory
//		int pm,				// 1= program memory, 0 = data memory
		BYTE *data,				// data buffer (address of FIRST buffer)
		int datalen,				// data buffer length for each DSP
		int reset			// reset DSP after writing if true 
		);

// Read multiple buffers from multiple DSP

int ReadFromDSPSeq(
		char *remote_addr,		// remote IP address in dotted format
		int firstDSP,			// First DSP to be addressed
		int lastDSP,			// Last DSP to be addressed
		unsigned int address,		// address in DSP memory
//		int pm,				// 1= program memory, 0 = data memory
		BYTE *data,			// buffer to receive data
		int datalen,			// requested length (buffer must be big enough to contain datalen*n_dsp bytes)
		int timeout,			// timeout in milliseconds( 0 = forever)
		int reset			// reset DSP after reading if true 
		);

// ---------------------
// High level functions
// ---------------------

int ReadFromBCU(
		char *remote_addr,		// remote IP address in dotted format
		int firstDSP,			// First DSP to be addressed
		int lastDSP,			// Last DSP to be addressed
		int opcode,			// command opcode (RD_SAME_DSP)
		unsigned int address,		// address in DSP memory
		int datalen,			// requested length (in 32 bit words, for each DSP)
		int flags,			// Additional flags
		unsigned char *firstID,		// first msgID sent
		unsigned char *lastID		// last msgID sent
		);

int WriteToBCU(
		char *remote_addr,		// remote IP address in dotted format
		int firstDSP,			// First DSP to be addressed
		int lastDSP,			// Last DSP to be addressed
		int opcode,			// Write opcode
		unsigned int address,		// address in DSP memory
		BYTE *data,			// data buffer to be sent
		int datalen,			// data buffer length (in 32 bit words)
		int flags,			// Additional flags
		unsigned char *firstID,		// first msgID sent
		unsigned char *lastID		// last msgID sent
		);

int GetPacketNum(
		int opcode,				// command opcode
		int firstDSP,			// First DSP to be addressed
		int lastDSP,			// last DSP to be addressed
		int BCU_address,		// Address in BCU memory
		int len,				// data buffer length (in 32 bit words)
		unsigned char *firstID  // filled with the first available packet ID
		);

// -------------------------------------------------------------------------
// DSP internal structures & functions
// -------------------------------------------------------------------------


/*
 * Defined BCU memory types
 */
enum BCU_memorytypes
{
	DSP_MEMORY	= 0,
	SRAM_MEMORY	= 1,
	SDRAM_MEMORY	= 2,
	FPGA_MEMORY	= 3,
};

/*
 *  Defined BCU commands
 */
enum mgp_BCU_opcodes
{
	MGP_OP_WRSAME_DSP	=0,	// Write same on DSP
	MGP_OP_WRSEQ_DSP	=1,	// Write seq on DSP
	MGP_OP_RDSEQ_DSP	=2, 	// Read seq on DSP

	MGP_OP_RESET_DEVICES	=10,	// Resets the specified devices
	MGP_OP_WRRD_RELAIS_BOARD=11,    // Write to relay board
	MGP_OP_LOCK_FLASH	=128,	// Lock flash area
	MGP_OP_UNLOCK_FLASH	=129,	// Unlock a portion of the flash area
	MGP_OP_CLEAR_FLASH	=130,	// Clear a portion of the flash area
	MGP_OP_WRITE_FLASH	=131,	// Write into flash area
	MGP_OP_RDSEQ_FLASH	=132,	// Read from flash area

	MGP_OP_WR_SCIMEASURE_RAM=135,      //  0x87   MGP scimeasure ram write
	MGP_OP_WRITE_SIGGEN_RAM	=136,      //  0x88   MGP signal generator ram write
	MGP_OP_WRITE_WAVEGEN_RAM=137, // Write into waveform generator RAM

	MGP_OP_CLEAR_SDRAM	=140,	// Clear a portion of the SDRAM
	MGP_OP_WRSAME_SDRAM	=141,	// Write same into SDRAM
	MGP_OP_WRSEQ_SDRAM	=142,	// Write seq into SDRAM
	MGP_OP_RDSEQ_SDRAM	=143,	// Read seq into SDRAM

	MGP_OP_CLEAR_SRAM	=145,	// Clear a portion of the SRAM
	MGP_OP_WRSAME_SRAM	=146,	// Write same into SRAM
	MGP_OP_WRSEQ_SRAM	=147,	// Write seq into SRAM
	MGP_OP_RDSEQ_SRAM	=148,	// Read seq from SRAM

	MGP_OP_WRSAME_DIAGBUFF	=150,      //  0x96   MGP diagnostic buffer ram write same buffer
	MGP_OP_WRSEQ_DIAGBUFF	=151,      //  0x97   MGP diagnostic buffer ram write seq buffer
	MGP_OP_RDSEQ_DIAGBUFF	=152,      //  0x98   MGP diagnostic buffer ram read buffer

   MGP_OP_WRITE_CCDI       = 155,
   MGP_OP_READ_CCDI        = 156,

	MGP_OP_CMD_SUCCESS	=200,	// Command successfull
	MGP_OP_CMD_FAULT	=201,	// Command error
	MGP_OP_CMD_TIMEOUT	=202,	// BCU internal timeout
	MGP_OP_CMD_WARNING	=203,	// Command warning - to be defined

	MGP_OP_RESERVED_1	=240,      //  0xf0   MGP reserved command (utilizzando per rimappare il crate ID delle schede DSP)
	MGP_OP_RESERVED_2	=241,      //  0xf1   MGP reserved command (comando che fa partire la sequenza di test e taratura degli otto canali dell'ADC)
	MGP_OP_RESERVED_3	=242,      //  0xf2   MGP reserved command (comando che fa partire la sequenza di test e taratura degli otto canali del DAC)

	MGP_OP_CMD_NULL		=255,	// Null command - no effect	
};

enum MGP_EXT
{
	MGP_EXT_CMD_TIMEOUT 	= 254
};

int ReadBCUinternal(
		char *remote_addr,		// remote IP address in dotted format
		int firstDSP,			// First DSP to be addressed
		int lastDSP,			// Last DSP to be addressed
		unsigned int address,		// address in DSP memory
		int datalen,			// requested length (in 32 bit words for each DSP)
		int command,			// requested command (RD_SEQ or RD_SYNC ?)
		int flags,			// Additional flags
		BCUcommand *packet_buffer,	// optional external storage for sent packet
		int *sent_len,			// optional return value (sent packet length)
		unsigned char msgID		// messageID to use
		);

int WriteBCUinternal(
		char *remote_addr,		// remote IP address in dotted format
		int firstDSP,			// First DSP to be addressed
		int lastDSP,			// Last DSP to be adressed
		unsigned int address,		// address in DSP memory
		BYTE *data,			// data buffer
		int datalen,			// data buffer length for each DSP (in 32 bit words)
		int command,			// Requested command: WR_SAME_DSP and others
		int flags,			// Additional flags
		BCUcommand *packet_buffer,	// optional external storage for sent packet
		int *sent_len,			// optional return value (sent packet length)
		unsigned char msgID		// message ID for this packet
		);

int WaitBCUanswer(
		char *buffer,			// buffer where to place data
		int max_len,			// buffer length
		int timeout			// timeout in milliseconds
		); 

int ResendPacket( BCUcommand *cmd, int buffer_len, char *remote_addr);

int UDPsetup( int local_port, int remote_port);
int UDPsetupSocket( int local_port, int remote_port, int *socket);


int IsReadOpcode(int opcode);
int IsWriteOpcode(int opcode);
int IsSameOpcode(int opcode);
int IsSeqOpcode(int opcode);
int GetDSPfactor(int opcode);
int DumpMemory(unsigned char *, int);


#endif // COMMLIB_H_INCLUDED
