//+File: SlopeCompTest.h
//
// Header file for Slopecomputer controller program
//
//-

#ifndef SLOPE_COMPUTER_TEST_INCLUDED
#define SLOPE_COMPUTER_TEST_INCLUDED

#include <string>

#include "AOApp.h"
#include "arblib/base/ArbitratorInterface.h"


class SlopeCompTest : public AOApp {

public:
   SlopeCompTest( int argc, char **argv) throw (AOException);

   ~SlopeCompTest();

protected:

   void Create(void) throw (AOException);

   // Safe stop/start functions 
   int safeStop( int *previous_state);
   int safeStart( int previous_state);

   int TestComm(void);

protected:
   // VIRTUAL - Setup variables in RTDB
   void SetupVars(void);

   // VIRTUAL - Exec loop
   void Run(void);

protected:
   // BCU object
   SlopeComp *wfs;

   // BCU number assigned to the slope computer
   int _bcuNum;

};


#define CALC_METHOD_SH         (0)
#define CALC_METHOD_TOMOGRAPHY (1)
#define CALC_METHOD_CONSTANT   (2)


#define PIXEL_LUT_ADDRESS	(0x0000)
#define DSP_PROGRAM_ADDRESS	(0x0000)
#define VOUT_OFFSET_ADDRESS	(0x020C)
#define GAINOFFSET_ADDRESS	(0x0220)
#define RTR0_ADDRESS		(0x80000)
#define RTR1_ADDRESS		(0x100000)
#define TIMEFILTER_ADDRESS	(0x40A0)
#define VOUT0_ADDRESS		(0x8000)
#define VOUT1_ADDRESS		(0xC000)

#define PIXEL_LUT_OPCODE	MGP_OP_WR_SCIMEASURE_RAM
#define DSP_PROGRAM_OPCODE	MGP_OP_WRSEQ_DSP
#define GAINOFFSET_OPCODE	MGP_OP_WRSEQ_DSP
#define RTR_OPCODE		MGP_OP_WRSEQ_DSP
#define TIMEFILTER_OPCODE	MGP_OP_WRSEQ_DSP
#define VOUT_OPCODE		MGP_OP_WRSEQ_DSP
#define VOUT_OFFSET_OPCODE	MGP_OP_WRSEQ_DSP

#define FRAMENUMBER_ADDRESS	(0x04ADB)
#define FRAMENUMBER_OPCODE	(MGP_OP_RDSEQ_SRAM)

#define RTRCONFIG_VARIABLE_LEN	(64)

#endif // SLOPE_COMPUTER_CTRL_INCLUDED
