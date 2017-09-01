//+File: FrameGrabberCtrl.h
//
// Header file for FrameGrabber controller program
//
//-


#ifndef FRAME_GRABBER_CTRL_INCLUDED
#define FRAME_GRABBER_CTRL_INCLUDED

#include <string>

#include "AOApp.h"

class FrameGrabberCtrl : public AOApp {

public:
   FrameGrabberCtrl( int argc, char **argv) throw (AOException);

protected:
   void Create() throw (AOException);

   // Local FSM
   int DoFSM();

   // Configure in an initial known state
   int FirstConfig();

   // Safe stop/start functions 
   int safeStop( int *previous_state);
   int safeStart( int previous_state);

protected:
   // VIRTUAL - Setup variables in RTDB
     void SetupVars(void);

   // VIRTUAL - Exec loop
   void Run(void);

   int TestComm();

   // RTDB handlers
   static int PixelLUTReqChanged( void *pt, Variable *var);
   static int GoReqChanged( void *pt, Variable *var);
   static int NumPixelsReqChanged( void *pt, Variable *var);
   static int DspProgReqChanged( void *pt, Variable *var);

   static int DarkReqChanged( void *pt, Variable *var);



protected:
   // BCU object
   BCUlbt *wfs;

   // BCU number assigned
   int _bcuNum;

   // WFS number assigned
   int _wfsNum;

   // Display name
   std::string _wfsName;

   // N. of pixels in the ccd
   int _nPixels;

   // Master diagnostic configuration
   int _masterD_enable;
   int _masterD_port;
   int _masterD_decimation;
   std::string _masterD_ip;
   std::string _masterD_mac;


   // RTDB vars
   RTDBvar var_status, var_go_req, var_go_cur, var_errmsg;
   RTDBvar var_pixellut_req, var_pixellut_cur;
   RTDBvar var_name;
   RTDBvar var_dspprog_req, var_dspprog_cur;
   RTDBvar var_numpixels_req, var_numpixels_cur;

   RTDBvar var_dark_req, var_dark_cur;

   RTDBvar _ccd47Bin;
   static int ccd47BinChanged( void *pt, Variable *var);


};


#include "FrameGrabber_memorymap.h"
#include "FrameGrabber_defaults.h"

#define PIXEL_LUT_ADDRESS	(0x0000)
#define DSP_PROGRAM_ADDRESS	(0x0000)

#define PIXEL_LUT_OPCODE	MGP_OP_WR_SCIMEASURE_RAM
#define DSP_PROGRAM_OPCODE	MGP_OP_WRSEQ_DSP

#define FRAMENUMBER_ADDRESS	(0x04ADB)
#define FRAMENUMBER_OPCODE	(MGP_OP_RDSEQ_SRAM)

#endif // FRAME_GRABBER_CTRL_INCLUDED
