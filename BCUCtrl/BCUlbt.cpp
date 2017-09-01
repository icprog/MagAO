//@File: BCU_mg.cpp
//
// Implementation of the BCU class.
//
// This class only have the general BCU functions that can be expected
// to be implemented on all BCUs. Specific applications should derive
// from this class.
//
//@

#include <string>
using namespace std;

#include <stdio.h>		// FILENAME_MAX 
#include <string.h>		// strcpy(), memcpy()
//#include <unistd.h>		// usleep()
#include <stdlib.h>		// free()


extern "C" {
#include "base/thrdlib.h"
#include "commlib.h"
#include "iolib.h"
}

#include "BcuLib/BCUmsg.h"
#include "BCUfile.h"
#include "BCUlbt.h"
#include "BcuLib/bcucommand.h"
#include "Logger.h"
#include "stdconfig.h"

#include "../Housekeeper/HouseKeeperRawStruct.h"
using namespace Arcetri::HouseKeeperRaw;


#include "SlopeComp_memorymap.h"


static int debug=0;


BCUlbt::BCUlbt()
{
}

//+Function: BCUlbt
//
// Standard constructor: sets the IP address and sets pointers to NULL
//-

BCUlbt::BCUlbt( int number, char *clientname)
{
    seqnum = 1;

	if (clientname)
		strcpy( this->clientname, clientname);

	this->number = number;
}

//+Function: ~BCU
//
// Standard deconstructor: frees the allocated buffers
//-

BCUlbt::~BCUlbt()
{
}

//+Function: setDSPprogram
//
// Sets the DSP program file and optionally sends it
//
// A startDSPprogram() command must be sent before the
// usual startDSP()/stopDSP() routines can be used.
//-

int BCUlbt::setDSPprogram( string filename, int send, int startDsp, int lastDsp)
{
	int stat;

	DSPprogramfilename = filename;

	if (send)
		{
		printf("Using number %d\n", number);
		uint32 zero[4] = {0, 0, 0, 0};
		uint32 data1[4] = { 0x00000020, 0, 0, 0};
		uint32 data2[4] = {0, 0, 0x04c8, 0x04c8};

        // Stop di sicurezza
        stopDSP();

		// Primo pacchetto per mandare il DSP in reset (write singolo su 180359)
		if ((stat = SendBuffer( number, startDsp, lastDsp, (unsigned char *)data1, 16, 0x180359, MGP_OP_WRSAME_DSP)) != NO_ERROR)
			return stat;	
		//
		// Pacchetto per resettare il DMA di ricezione (quad word in 180448)
		if ((stat = SendBuffer( number, startDsp, lastDsp, (unsigned char *)zero, 16, 0x180448, MGP_OP_WRSAME_DSP, FLAG_ASQUADWORD)) != NO_ERROR)
			return stat;
		//
		// Pacchetto per resettare il DMA di ricezione (2) (quad word in 18044c)
		if ((stat = SendBuffer( number, startDsp, lastDsp, (unsigned char *)zero, 16, 0x18044C, MGP_OP_WRSAME_DSP, FLAG_ASQUADWORD)) != NO_ERROR)
			return stat;
		//
		// Inizializzazione linkport (quad word in 1804e0)
		if ((stat = SendBuffer( number, startDsp, lastDsp, (unsigned char *)data2, 16, 0x1804E0, MGP_OP_WRSAME_DSP, FLAG_ASQUADWORD)) != NO_ERROR)
			return stat;
		// Interpreta il programma DSP e lo manda alla BCU
		if ((stat = SendDSPprogram( number, filename, startDsp, lastDsp)) != NO_ERROR)
			return stat;
		
		}

	return NO_ERROR;
}

//+Function: startDSPprogram
//
// Actually starts the DSP program. To be done AFTER the
// full dsp configuration.
//
// This is NOT the same as the startDSP() routine. This is the
// final configuration step. After this routine, the DSP can
// be started/stopped as usual.
//+

int BCUlbt::startDSPprogram( int startDsp, int lastDsp)
{
		uint32 zero[4] = {0, 0, 0, 0};
		// Per far ripartire il programma, scrive nel registro VIRPT (0x180730) dell'indirizzo 0
		return SendBuffer( number, startDsp, lastDsp, (unsigned char *)zero, 4, 0x180730, MGP_OP_WRSAME_DSP);
}


//+Function: getNumber
//
// Returns the assigned BCU number
//-

int BCUlbt::getNumber()
{
	return number;
}


//+Function: startDSP
//
// Starts DSP operation
//-

int BCUlbt::startDSP( int startDsp, int lastDsp)
{
	uint32 addr = 0x0018035A;
	uint32 cmd  = 0xFEFFFFFF;
 
	int stat = SendBuffer( number, startDsp, lastDsp, (unsigned char *)&cmd, 4, addr, MGP_OP_WRSEQ_DSP);
	if (!IS_ERROR(stat))
		Arcetri::Logger::get()->log(Arcetri::Logger::LOG_LEV_INFO, "DSP started");
	else
		Arcetri::Logger::get()->log(Arcetri::Logger::LOG_LEV_ERROR, "Error starting DSP: %s", lao_strerror(stat));

        return stat;
}

//+Function: stopDSP
//
// Stops DSP operation and waits 10 msec to have be reasonable
// sure that the current frame is finished
//-

int BCUlbt::stopDSP( int startDsp, int lastDsp)
{
	uint32 addr = 0x00180359;
	uint32 cmd  = 0x01000000;

	int stat = SendBuffer( number, startDsp, lastDsp, (unsigned char *)&cmd, 4, addr, MGP_OP_WRSEQ_DSP);
	if (!IS_ERROR(stat))
		Arcetri::Logger::get()->log(Arcetri::Logger::LOG_LEV_INFO, "DSP stopped");
	else
		Arcetri::Logger::get()->log(Arcetri::Logger::LOG_LEV_ERROR, "Error stopping DSP: %s", lao_strerror(stat));

	return stat;
}

//@Function: WriteBCU_DSPword
//
// Writes a single word into the BCU DSP memory
//@

int BCUlbt::WriteBCU_DSPword( uint32 addr, uint32 value, int startDsp, int lastDsp)
{
    printf("Writing 0x%08X to DSP 0x%08X\n", value, addr);
    return SendBuffer( number, startDsp, lastDsp, (unsigned char *)&value, sizeof(uint32), addr, MGP_OP_WRSEQ_DSP);
}

//@Function: WriteBCU_SRAMword
//
// Writes a single word into the BCU SRAM memory
//@

int BCUlbt::WriteBCU_SRAMword( uint32 addr, uint32 value, int startDsp, int lastDsp)
{
    printf("Writing 0x%08X to SRAM 0x%08X\n", value, addr);
    return SendBuffer( number, startDsp, lastDsp, (unsigned char *)&value, sizeof(uint32), addr, MGP_OP_WRSEQ_SRAM);
}

//@Function: WriteBCU_RELAYword
//
// Writes a single word into the BCU RELAY board
//@

int BCUlbt::WriteBCU_RELAYword( uint32 addr, uint32 value, int startDsp, int lastDsp)
{
     uint32 buffer[2];

    buffer[0] = 0x01;       // Set command
    buffer[1] = value;

    printf("Writing 0x%08X to RELAY 0x%08X\n", value, addr);
    return SendBuffer( number, startDsp, lastDsp, (unsigned char *)&value, sizeof(uint32)*2, addr, MGP_OP_WRRD_RELAIS_BOARD);
}



//+Function: matrixToREC
//
// Converts an ordinary NxM matrix to BCU reconstructor format
// (with elements interleaved for the DSP calculation)
//
//-

int BCUlbt::matrixToREC( float32 *from, float32 *to, int rows, int cols)
{
	int x, y, counter;

	if (rows & 0x01)
		{
		fprintf(stderr, "matrixToREC(): error: number of rows must be a multiple of 2\n");
		return VALUE_OUT_OF_RANGE_ERROR;
		}

	if (cols & 0x01)
		{
		fprintf(stderr, "matrixToREC(): error: number of cols must be a multiple of 2\n");
		return VALUE_OUT_OF_RANGE_ERROR;
		}

	counter=0;
	for (y=0; y<rows; y+=2)
		for (x=0; x<cols; x+=4)
			{
			to[ counter  ] = from[ y*cols +x   ];
			to[ counter+1] = from[ y*cols +x +1];
			to[ counter+2] = from[ y*cols +x +2];
			to[ counter+3] = from[ y*cols +x+ 3];

			to[ counter+4] = from[ (y+1)*cols +x   ];
			to[ counter+5] = from[ (y+1)*cols +x +1];
			to[ counter+6] = from[ (y+1)*cols +x +2];
			to[ counter+7] = from[ (y+1)*cols +x +3];

			counter = counter+8;
			}

	return NO_ERROR;
}


//@Function: SendBuffer
//
// Local version of the BCUfunctions::SendBuffer() function.
//
// This version will send a message to the appropriate BCU controller to perform the action
//@

int BCUlbt::SendBuffer( int BCUnumber, int firstDSP, int lastDSP, unsigned char *buffer, int bufferlen, int address, int opcode, int flags, int timeout) throw (AOException)
{
	return thSendBCUcommand( clientname, BCUnumber, firstDSP, lastDSP, opcode, address, buffer, bufferlen, timeout, seqnum++, flags);
}

//@Function: GetBuffer
//
// Local version of the BCUfunctions::GetBuffer() function.
//
// This version will send a message to the appropriate BCU controller to perform the action
// It is actually identical to BCUlbt::SendBuffer(), but it is kept separate in case reading and writing
// end up having to make different checks in the future.
//@

int BCUlbt::GetBuffer( int BCUnumber, int firstDSP, int lastDSP, unsigned char *buffer, int bufferlen, int address, int opcode, int flags, int timeout) throw (AOException)
{
	return thSendBCUcommand( clientname, BCUnumber, firstDSP, lastDSP, opcode, address, buffer, bufferlen, timeout, seqnum++, flags);
}

//@Function: SendBuffer_verify
//
// Sends a buffer to a BCU, reads back the contents and verifies that everything was written correctly
//@

int BCUlbt::SendBuffer_verify( int BCUnumber, int firstDSP, int lastDSP, unsigned char *buf, int buflen, int address, int wr_opcode, int rd_opcode, int flags) throw (AOException)
{
	int stat;
	unsigned char *readbuf;

	stat = SendBuffer( BCUnumber, firstDSP, lastDSP, buf, buflen, address, wr_opcode, flags);
	if (stat != NO_ERROR)
		return stat;

	if ((readbuf = (unsigned char *)malloc(buflen)) == NULL)
		throw AOException("Memory allocation error", MEMORY_ALLOC_ERROR, __FILE__, __LINE__);

	stat = GetBuffer( BCUnumber, firstDSP, lastDSP, readbuf, buflen, address, rd_opcode, flags);
	if (stat != NO_ERROR)
		{
		free(readbuf);
		return stat;
		}

	if (memcmp( buf, readbuf, buflen))
		stat = COMMUNICATION_ERROR;
	else
		stat = NO_ERROR;

	free(readbuf);
	return stat;
}

//+Function: SendDSPprogram
//
// Converts a DSP program to a complete buffer, expanding blocks as needed,
// and sends it to the BCU.
//
// Returns an error code.
//-

int BCUlbt::SendDSPprogram( int BCUnumber, string filename, int startDsp, int lastDsp)
{
        unsigned char *buffer, *tmpbuf;
        int len, bufferlen, i, stat;
        uint32 val32, databuf_startaddr, databuf_len;

        buffer = ReadASCIIFile( (char *)filename.c_str(), &bufferlen);
        if (!buffer)
                return FILE_ERROR;

        printf("File read, len = %d\n", bufferlen);

        // Skip the first 256 dwords (unused boot loader program)
//        memmove( buffer, buffer+256*4, bufferlen-256*4);
        len = bufferlen;

        for (i=0; i<len; )
                {
                val32 = *((uint32 *)&(buffer[i]));
                i+=4;

		printf("Len: %d, i:%d\n", len, i);
                // Check for data buffer
                if ((val32 & 0xF0000000) == 0x40000000)
                        {
                        databuf_len = val32 & 0x0FFFFFFF;
                        // Get databuffer start address
                        databuf_startaddr = *((uint32 *)&(buffer[i]));
                        i+=4;

                        if (debug)
                                printf("Data buffer (addr %X, len %d)\n", databuf_startaddr, databuf_len);

                        // Here there should be some way to align the buffer to 0x04...
                        stat = SendBuffer( BCUnumber, startDsp, lastDsp, buffer+i, databuf_len*4, databuf_startaddr, MGP_OP_WRSAME_DSP);
                        if (stat != NO_ERROR)
                                break;

                        i+= databuf_len*4;
                        }

                // Check for a buffer to zero out
                if ((val32 & 0xF0000000) == 0x80000000)
                        {
                        databuf_len = val32 & 0x0FFFFFFF;

                        // Get databuffer start address
                        databuf_startaddr = *((uint32 *)&(buffer[i]));
                        i+=4;

                        if (debug)
                                printf("Zero buffer (addr %X, len %d)\n", databuf_startaddr, databuf_len);

                        tmpbuf = (unsigned char *)calloc(databuf_len*4, 1);

                        // Here there should be some way to align the buffer to 0x04...
                        stat = SendBuffer( BCUnumber, startDsp, lastDsp, tmpbuf, databuf_len*4, databuf_startaddr, MGP_OP_WRSAME_DSP);
                        if (stat != NO_ERROR)
                                break;

                        free(tmpbuf);
                        }

                // Check for a DSP code buffer (it's the last one)
                if (val32== 0x00000000)
                        {
                        databuf_startaddr = 0;
                     if (debug)
                                printf("Program area (addr %X, len%d)\n", 0, len-i);

                        stat = SendBuffer( BCUnumber, startDsp, lastDsp, buffer+i, len-i, databuf_startaddr, MGP_OP_WRSAME_DSP);
                        if (stat != NO_ERROR)
                                break;
                        i = len;                // stop the loop
                        }

                }

	printf("Returning with stat = %d\n", stat);
        free(buffer);

        return stat;
}




//@Function: sendPixelLUT
//
// Reads a PixelLUT file and sends it to the BCU
// File must be in hexadecimal format
//@

int BCUlbt::sendPixelLUT( string filename)
{
    int stat, bufferlen;
    unsigned char *buffer;

    buffer = ReadASCIIFile( (char *)filename.c_str(), &bufferlen);
    if (!buffer)
       throw AOException(filename, FILE_ERROR, __FILE__, __LINE__);

    printf("Writing pixel LUT at 0x%08X, len = 0x%08X words\n", PIXEL_LUT_ADDRESS, bufferlen/4);

    printf("Sending new LUT\n");
    if ((stat = SendBuffer( number, BCU_DSP, BCU_DSP, buffer, bufferlen, PIXEL_LUT_ADDRESS, PIXEL_LUT_OPCODE)) != NO_ERROR)
        {
        printf("Error in SendBuffer: (%d) %s\n", stat, lao_strerror(stat));
        free(buffer);
        throw AOException(filename, stat, __FILE__, __LINE__);
        }

    printf("Done\n");
    free(buffer);
    return NO_ERROR;
}




int BCUlbt::selectPixelOrigin( int origin)
{
   int stat;

   uint32 zero = 0x00000000;
   uint32 uno  = 0x00000001;
   uint32 due  = 0x00000002;
   uint32 tre  = 0x00000003;

   // Disabilito l'uscita
   stat = SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&zero, sizeof(uint32), 0, MGP_OP_WRITE_CCDI);
   if (stat != NO_ERROR)
         return stat;

   switch(origin)
      {
      case ORIGIN_CCD:
      if ((stat = SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&uno, sizeof(uint32), 0, MGP_OP_WRITE_CCDI)) != NO_ERROR)
         return stat;
      Arcetri::Logger::get()->log(Arcetri::Logger::LOG_LEV_INFO, "Selected CCD pixel origin");
      printf("Selected ccd pixel origin\n");
      break;

      case ORIGIN_RANDOM:
      // TODO mettere il valore giusto
      if ((stat = SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&zero, sizeof(uint32), 1, MGP_OP_WRITE_CCDI)) != NO_ERROR)
         return stat;
      if ((stat = SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&due, sizeof(uint32), 0, MGP_OP_WRITE_CCDI)) != NO_ERROR)
         return stat;
      Arcetri::Logger::get()->log(Arcetri::Logger::LOG_LEV_INFO, "Selected RANDOM pixel origin");
      printf("Selected random pixel origin\n");
      break;

      case ORIGIN_RAM:
      // TODO mettere il valore giusto
      if ((stat = SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&zero, sizeof(uint32), 2, MGP_OP_WRITE_CCDI)) != NO_ERROR)
         return stat;
      if ((stat = SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&tre, sizeof(uint32), 0, MGP_OP_WRITE_CCDI)) != NO_ERROR)
         return stat;
      Arcetri::Logger::get()->log(Arcetri::Logger::LOG_LEV_INFO, "Selected RAM pixel origin");
      printf("Selected ram pixel origin\n");
      break;

      default:
      Arcetri::Logger::get()->log(Arcetri::Logger::LOG_LEV_ERROR, "Incorrect value for pixel origin");
      return VALUE_OUT_OF_RANGE_ERROR;
      }


   return stat;
}

// Configures the Master diagnostic from BCU.

int BCUlbt::configureMasterDiagnostics( int enable, uint8 ipAddr[4], uint8 macAddr[6], int udp_port, int decimation)
{

  struct littleMaster {
     uint16 enable_master_diag;                      // abilita(1) / disabilita(0) la spedizione della diagnostica in modo automatico
     uint16 decimation_factor;                       // fattore di decimazione dei frame spediti via Master BCU
     uint8  remote_mac_address[6];                   // indirizzo mac della scheda dove spedire la diagnostica
     uint8  remote_ip_address[4];                    // indirizzo ip della scheda dove spedire la diagnostica
     uint16 remote_udp_port;
     };

    uint32 dsp_decimation = (uint32) decimation;

    struct littleMaster localcopy;

    na_bcu_nios_fixed_area_struct dummy_struct;
    uint32 offset;

    // IP address
    localcopy.enable_master_diag = enable;
    localcopy.decimation_factor = 0;
    memcpy( localcopy.remote_mac_address, macAddr, sizeof(uint8)*6);
    memcpy( localcopy.remote_ip_address, ipAddr, sizeof(uint8)*4);
    localcopy.remote_udp_port = udp_port;

    Arcetri::Logger::get()->log( Arcetri::Logger::LOG_LEV_DEBUG, "MasterDiagnostic: IP is %d.%d.%d.%d", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
    Arcetri::Logger::get()->log( Arcetri::Logger::LOG_LEV_DEBUG, "MasterDiagnostic: UDP port: %d", udp_port);
    Arcetri::Logger::get()->log( Arcetri::Logger::LOG_LEV_DEBUG, "MasterDiagnostic: Decimation: %d", decimation);
    Arcetri::Logger::get()->log( Arcetri::Logger::LOG_LEV_DEBUG, "MasterDiagnostic: Enable: %d", enable);

    // Send everything at once
    offset = ((char *) &(dummy_struct.enable_master_diag) - ((char *) &dummy_struct))/ sizeof(uint32);
    Arcetri::Logger::get()->log( Arcetri::Logger::LOG_LEV_INFO, "Writing to address: %d", offset +AddressMap::BCU_NIOS_FIXED_AREA);


    SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&localcopy, sizeof(struct littleMaster), offset + AddressMap::BCU_NIOS_FIXED_AREA, MGP_OP_WRSEQ_SRAM);

    SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&dsp_decimation, sizeof(uint32), SLPCOMP__BSCUB_DIAGNOSTICFRAMEDEC, MGP_OP_WRSEQ_DSP);
    

    return NO_ERROR;
}


string BCUlbt::getIpAddr() {

    string mirrorctrl = "mirrorctrl";
    string filename = Utils::getConffile(mirrorctrl);
    Config_File *cfgmirror = new Config_File(filename);

    ostringstream oss;
    oss << "Bcu_" << getNumber();
    return (std::string) (*cfgmirror)[oss.str()];
}

