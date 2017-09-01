//+File: SlopeCompCtrl.h
//
// Header file for Slopecomputer controller program
//
//-

#ifndef SLOPE_COMPUTER_CTRL_INCLUDED
#define SLOPE_COMPUTER_CTRL_INCLUDED

#include <string>

#include "AOApp.h"
#include "arblib/base/ArbitratorInterface.h"
#include <pthread.h>


class SlopeCompCtrl : public AOApp {

public:
   SlopeCompCtrl( int argc, char **argv) throw (AOException);

   ~SlopeCompCtrl();

   void lock();
   void unlock();

protected:

   AlertNotifier *_arbNotif;
   
   void Create(void) throw (AOException);

   // Local FSM
   int DoFSM(void);

   // Test communication link
   int TestComm(void);

   // Configure in an initial known state
   int FirstConfig(void);

   // Safe stop/start functions 
   int safeStop( int *previous_state);
   int safeStart( int previous_state);
   int stopCCD39();
   int startCCD39();

   // Fiber check
   int checkFiber(void);

   int TestFastlink(void);

   // Mutex to lock DSP stop/start
   pthread_mutex_t dsp_mutex;

protected:
   // VIRTUAL - Setup variables in RTDB
   void SetupVars(void);

   // VIRTUAL - Exec loop
   void Run(void);

   // RTDB handlers
   static int GoReqChanged( void *pt, Variable *var);
   static int DarkReqChanged( void *pt, Variable *var);
   static int PixelLUTReqChanged( void *pt, Variable *var);
   static int PixelGainsReqChanged( void *pt, Variable *var);
   static int SlopenullReqChanged( void *pt, Variable *var);
   static int DspProgReqChanged( void *pt, Variable *var);
   static int ThVectorReqChanged( void *pt, Variable *var);
   static int ThOffsetsReqChanged( void *pt, Variable *var);
   static int ThAmpReqChanged( void *pt, Variable *var);
   static int ThConstReqChanged( void *pt, Variable *var);
   static int ThPeriodsReqChanged( void *pt, Variable *var);
   static int ThEnableReqChanged( void *pt, Variable *var);
   static int CmdOffsetsReqChanged( void *pt, Variable *var);
   static int CmdEnableReqChanged( void *pt, Variable *var);
   static int FastlinkCmdReqChanged( void *pt, Variable *var);
   static int FastlinkEnableReqChanged( void *pt, Variable *var);
   static int AdsecDiagnEnableReqChanged( void *pt, Variable *var);
   static int CalcModeReqChanged( void *pt, Variable *var);
   static int FluxGainReqChanged( void *pt, Variable *var);
   static int NSubapsReqChanged( void *pt, Variable *var);
   static int PixelOrigReqChanged( void *pt, Variable *var);
   static int MasterDiagnDecimationReqChanged( void *pt, Variable *var);
   static int DisturbEnableReqChanged( void *pt, Variable *var);
   static int ClipReqChanged( void *pt, Variable *var);



protected:
   // BCU object
   SlopeComp *wfs;

   // BCU number assigned to the slope computer
   int _bcuNum;

   // Display name
   std::string _wfsName;

   // Fastlink configuration file
   std::string _flFilename;

   // Fastlink check flag
   int _testFastlink;

   // Master diagnostic configuration
   int _masterD_enable;
   int _masterD_port;
   int _masterD_decimation;
   uint8 _masterIP[4], _masterMAC[6];

   // Maximum CRC and Timeout errors allowed
   int _FLtimeoutEnable;
   int _FLtimeoutMax;
   int _FLCRCEnable;
   int _FLCRCMax;

   // RTDB vars
   RTDBvar var_status, var_go_req, var_go_cur, var_errmsg;
   RTDBvar var_dark_req, var_slopenull_req, var_pixellut_req;
   RTDBvar var_dark_cur, var_slopenull_cur, var_pixellut_cur;
   RTDBvar var_name, var_nsubaps_req, var_nsubaps_cur;
   RTDBvar var_pixelorig_req, var_pixelorig_cur;
   RTDBvar var_dspprog_req, var_dspprog_cur, var_pixelgains_req, var_pixelgains_cur;
   RTDBvar var_th_vector_req, var_th_offsets_req, var_th_amp_req, var_th_periods_req, var_th_enable_req;
   RTDBvar var_th_vector_cur, var_th_offsets_cur, var_th_amp_cur, var_th_periods_cur, var_th_enable_cur;
   RTDBvar var_th_const_req, var_th_const_cur;
   RTDBvar var_cmd_offsets_req, var_cmd_enable_req, var_fl_cmd_req, var_fl_enable_req;
   RTDBvar var_cmd_offsets_cur, var_cmd_enable_cur, var_fl_cmd_cur, var_fl_enable_cur;
   RTDBvar var_calcmode_req, var_calcmode_cur;
   RTDBvar var_fluxgain_req, var_fluxgain_cur;
   RTDBvar var_adsecdiagn_enable_req, var_adsecdiagn_enable_cur;
   RTDBvar var_masterd_decimation_req, var_masterd_decimation_cur;
   RTDBvar var_disturb_enable_req, var_disturb_enable_cur;
   RTDBvar var_clip_req, var_clip_cur;

   RTDBvar _ccd39Bin;
   RTDBvar _ccd39Enable;
   RTDBvar _ccd39EnableCur;
   static int ccd39BinChanged( void *pt, Variable *var);

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
