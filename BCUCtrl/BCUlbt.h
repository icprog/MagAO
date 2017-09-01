//@File: BCUlbt.h
//
// Class definition for the BCU object
//
// This class only have the general BCU functions that can be expected
// to be implemented on all BCUs. Specific applications should derive
// from this class.

//@

#ifndef BCULBT_H_INCLUDED
#define BCULBT_H_INCLUDED

#define PIXEL_LUT_ADDRESS   (0x0000)
#define PIXEL_LUT_OPCODE    MGP_OP_WR_SCIMEASURE_RAM

extern "C" {
#include "base/common.h"
}

#include "AOExcept.h"

enum
{
    ORIGIN_CCD = 0,
    ORIGIN_RANDOM = 1,
    ORIGIN_RAM = 2
};

class BCUlbt
{
public:
    BCUlbt();
	BCUlbt( int number, char *clientname);	// Standard constructor
	virtual ~BCUlbt();	 				// Standard deconstructor

public:
	// High-level functions
	int getNumber(); 			// Returns the assigned number

	int startDSP( int startDsp = 0xFF, int lastDsp = 0xFF);				// Starts DSP operation
	int stopDSP( int startDsp = 0xFF, int lastDsp = 0xFF);				// Stops DSP operation

   // VIRTUAL functions
   virtual int sendPixelLUT( std::string filename);  // Reads a PixelLUT file and sends it

	// Lower-level functions

	int setDSPprogram( std::string filename, int send=1, int startDsp = 0xFF, int lastDsp = 0xFF);	// Reads a DSP program and optionally sends it
    int startDSPprogram( int startDsp = 0xFF, int lastDsp = 0xFF);

    int WriteBCU_DSPword( uint32 addr, uint32 value, int startDsp = 0xFF, int lastDsp = 0xFF);
    int WriteBCU_SRAMword( uint32 addr, uint32 value, int startDsp = 0xFF, int lastDsp = 0xFF);
    int WriteBCU_RELAYword( uint32 addr, uint32 value, int startDsp = 0xFF, int lastDsp = 0xFF);


	int SendBuffer( int BCUnumber, int firstDSP, int lastDSP, unsigned char *buf, int buflen, int address, int opcode, int flags=0, int timeout = 1000) throw (AOException);
	int GetBuffer( int BCUnumber, int firstDSP, int lastDSP, unsigned char *buf, int buflen, int address, int opcode, int flags=0, int timeout = 1000) throw (AOException);

	int SendBuffer_verify( int BCUnumber, int firstDSP, int lastDSP, unsigned char *buf, int buflen, int address, int rw_opcode, int rd_opcode, int flags=0) throw (AOException);

   int selectPixelOrigin( int origin);
   int configureMasterDiagnostics( int enable, uint8 ipAddr[4], uint8 macAddr[6], int udp_port, int decimation);

        string getIpAddr();   // Returns the BCU ip address

protected:
	int matrixToREC( float32 *from, float32 *to, int rows, int cols); // Converts from normal to DSP ordering

	int SendDSPprogram( int BCUnumber, std::string filename, int startDsp = 0xFF, int lastDsp = 0xFF);

	int number;				// Assigned BCU number;

   std::string DSPprogramfilename;		// Last filename used for DSP program
   std::string LUTfilename;             // Last filename used for pixel LUT

	char clientname[PROC_NAME_LEN+1];
	int seqnum;

};

#endif // BCU_mg_H_INCLUDED
