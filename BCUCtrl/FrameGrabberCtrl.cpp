//+File: SlopeCompCtrl.c
//
// Slope computer controller program
//

#include <string>

extern "C" {
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>

#include "base/thrdlib.h"
#include "commlib.h"  
#include "iolib.h"
}

#include "RTDBvar.h"
#include "BcuLib/BCUmsg.h"
#include "BCUfile.h"
#include "BCUlbt.h"
#include "FrameGrabberCtrl.h"
#include "stdconfig.h"
#include "BcuLib/bcucommand.h"
#include "Paths.h"

extern "C" {
#define NIOS_WFS
#include "../Housekeeper/HouseKeeperRawStruct.h"
}
using namespace Arcetri::HouseKeeperRaw;


// **************************************
// Client name and versioning information
// **************************************

int VersMajor = 1;
int VersMinor = 0;
const char *Date = "Nov 2005";
const char *Author = "A. Puglisi";

// Global variables

int seqnum=0;

int CHAR_VAR_LEN = 128;			// Length of a "filename variable
int ERRMSG_LEN = 128;			// Length of the error message variable

#include "AOStates.h"

//+Function: help
//
// Prints an usage message
//-
// Rimosso perche' non usato (L.F.)
//static void help()
//{
//    printf("\nFrameGrabberCtr�  - Vers. %d.%d.  %s, %s\n\n", VersMajor,VersMinor,Author,Date);
//    printf("Usage: FrameGrabberCtrl [-v] [-f <config_file>] [-s <Server Address>]\n\n");
//    printf("   -s    specify server address (defaults to localhost)\n");
//    printf("   -f    specify configuration file (defaults to FrameGrabberCtrl.config)\n");
//    printf("   -v    verbose mode\n");
//    printf("\n");
//}


// +Main: 
//
// Main function
//-

FrameGrabberCtrl::FrameGrabberCtrl( int argc, char **argv) throw (AOException): AOApp(argc, argv)
{
   Create();
}

void FrameGrabberCtrl::Create() throw (AOException)
{
   try {
     _wfsName     = (char *) ((std::string)ConfigDictionary()["wfsName"]).c_str();
     _bcuNum      = ConfigDictionary()["bcuNum"];
     _nPixels    = ConfigDictionary()["nPixels"];

      try {
         _masterD_enable = ConfigDictionary()["masterD_enable"];
         _masterD_port = ConfigDictionary()["masterD_port"];
         _masterD_decimation = ConfigDictionary()["masterD_decimation"];
         _masterD_ip = (std::string) ConfigDictionary()["masterD_ip"];
         _masterD_mac = (std::string) ConfigDictionary()["masterD_mac"];
      } catch (Config_File_Exception &e) {
         _logger->log( Logger::LOG_LEV_WARNING, "No master diagnostic configuration, will not be enabled. Reason: %s", e.what().c_str());
         _masterD_enable = 0;
      }


   } catch (Config_File_Exception &e) {
      _logger->log( Logger::LOG_LEV_FATAL, "Missing configuration data: %s", e.what().c_str());
     throw AOException("Fatal: Missing configuration data");
   } 

	wfs = new BCUlbt( _bcuNum, (char *) MyFullName().c_str());
}


void FrameGrabberCtrl::Run()
{
   setCurState(STATE_NOCONNECTION);

   while(!TimeToDie())
         {
         try {
            DoFSM();
         } catch (AOException &e) {
               _logger->log( Logger::LOG_LEV_ERROR, "Caugth exception: %s", e.what().c_str());
         }

         msleep(1000);
         }
}


int FrameGrabberCtrl::ccd47BinChanged( void *pt, Variable *var) {
   _logger->log(Logger::LOG_LEV_INFO, "Ccd47 current binning to %d", var->Value.Lv[0]);
   FrameGrabberCtrl *FGC = (FrameGrabberCtrl *)pt;
   try {
      FGC->_ccd47Bin.Set(var);
   }
   catch(AOVarException& e) {
      _logger->log(Logger::LOG_LEV_WARNING, "Local Ccd47 binning updating failed");
      return VAR_WRITE_ERROR;
   }
   return NO_ERROR;
}


//@Function: DoFSM
//
// Do a FSM step. States can be changed asynchronosuly
// from this thread or the listening thread, and this function
// will properly react to a new state
//
// Many states include a msleep() function to slow down the thread
// when immediate action is not necessary

int FrameGrabberCtrl::DoFSM()
{
	int status;
	int stat = NO_ERROR;

   status = getCurState();

	// Always check communication
   if (status != STATE_NOCONNECTION) {
      bool ok=false;
      int ntry =3;
      while (--ntry >0) {
        if (TestComm() == NO_ERROR) {
           ok=true;
           break;
        }
        sleep(1);
      }
      if (!ok) {
         _logger->log( Logger::LOG_LEV_ERROR, "Lost communication with frame grabber!");
         setCurState(STATE_NOCONNECTION);
      }
   }

   status = getCurState();

	switch(status)
		{
      case STATE_NOCONNECTION:
      if (TestComm() == NO_ERROR)
            setCurState(STATE_CONNECTED);
      break;

		case STATE_CONNECTED:
      stat = FirstConfig();
      if (stat == NO_ERROR)
         {
         _logger->log( Logger::LOG_LEV_DEBUG, "Setting state %d", STATE_READY);
         setCurState(STATE_READY);
         }
      break;

		case STATE_READY:
      case STATE_OPERATING:
//		msleep(1000);
		break;


		// Unknown states should not exist
		default:
//		msleep(5000);
		break;
		}

	return stat;
}


void FrameGrabberCtrl::SetupVars()
{
	int files_len = CHAR_VAR_LEN;

	// Standard WFS variable set

   try {
	var_name = RTDBvar( MyFullName(), "NAME", NO_DIR, CHAR_VARIABLE, _wfsName.size()+1);
	var_errmsg = RTDBvar( MyFullName(), "ERRMSG", NO_DIR, CHAR_VARIABLE, ERRMSG_LEN);

	var_go_req = RTDBvar( MyFullName(), "GO", REQ_VAR);
	var_go_cur = RTDBvar( MyFullName(), "GO", CUR_VAR);

	var_pixellut_req = RTDBvar( MyFullName(), "PIXELLUT", REQ_VAR, CHAR_VARIABLE, files_len);
	var_pixellut_cur = RTDBvar( MyFullName(), "PIXELLUT", CUR_VAR, CHAR_VARIABLE, files_len);

	var_dspprog_req = RTDBvar( MyFullName(), "DSPPROG", REQ_VAR, CHAR_VARIABLE, files_len);
	var_dspprog_cur = RTDBvar( MyFullName(), "DSPPROG", CUR_VAR, CHAR_VARIABLE, files_len);

   var_numpixels_req = RTDBvar( MyFullName(), "NPIXELS", REQ_VAR);
	var_numpixels_cur = RTDBvar( MyFullName(), "NPIXELS", CUR_VAR);

   var_dark_req = RTDBvar( MyFullName() , "DARK", REQ_VAR, CHAR_VARIABLE, files_len);
   var_dark_cur = RTDBvar( MyFullName() , "DARK", CUR_VAR, CHAR_VARIABLE, files_len);


	var_name.Set(_wfsName);
   var_numpixels_cur.Set(0);
   
   Notify( var_go_req, GoReqChanged);
   Notify( var_pixellut_req, PixelLUTReqChanged);
   Notify( var_dspprog_req, DspProgReqChanged);
   Notify( var_numpixels_req, NumPixelsReqChanged);
   Notify( var_dark_req, DarkReqChanged);

     // Configure a notification on ccd47 binning variable
     string ccd47Process = (string)ConfigDictionary()["ccdProcess"]+"."+Side();

     // Wait for ccd47 control process to become ready, otherwise there's no variable to attach!
     while(1) {
         int ret;
         _logger->log( Logger::LOG_LEV_INFO, "Waiting for ccd47 controller to become ready....");
         if ((ret = thWaitReady( ccd47Process.c_str(), 60 * 1000)) == NO_ERROR)
             break;
         if (ret != TIMEOUT_ERROR)
            throw AOException("Error waiting for ccd47 to become ready", ret, __FILE__, __LINE__);
     }

     _ccd47Bin = RTDBvar(ccd47Process + ".XBIN.CUR");  // Also get the value
     Notify(_ccd47Bin, ccd47BinChanged);



   } catch (AOVarException &e) {
       _logger->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
       throw AOException("Error creating RTDB variables");
    }

}

int FrameGrabberCtrl::PixelLUTReqChanged( void *pt, Variable *var)
{
   FrameGrabberCtrl *ctrl = (FrameGrabberCtrl *)pt;
   char *lutfile = var->Value.Sv;

   try
   {
       Logger::get()->log( Logger::LOG_LEV_DEBUG, "Setting pixel LUT to %s", lutfile);

       ctrl->wfs->stopDSP();
       ctrl->wfs->sendPixelLUT( lutfile);
       ctrl->var_pixellut_cur.Set(lutfile, FORCE_SEND);


       Logger::get()->log( Logger::LOG_LEV_INFO, "Pixel LUT set to %s", lutfile);
   } catch (AOException &e) {
       Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting pixel LUT: %s", e.what().c_str());
   }

   return NO_ERROR;
}

int FrameGrabberCtrl::GoReqChanged( void *pt, Variable *var)
{
   FrameGrabberCtrl *ctrl = (FrameGrabberCtrl *)pt;
   int go = var->Value.Lv[0]; 
   int status;

   try {
      status = ctrl->getCurState();

      Logger::get()->log( Logger::LOG_LEV_DEBUG, "Received %s DSP request", (go)?"start":"stop");

      if ((go) && (status == STATE_READY))
         {
         ctrl->wfs->startDSP(); 
         status = STATE_OPERATING;
         }
     else if ((!go) && (status == STATE_OPERATING))
         {
         ctrl->wfs->stopDSP();
         status = STATE_READY;
         }

      ctrl->setCurState(status);
      ctrl->var_go_cur.Set( (status == STATE_OPERATING) ? 1 : 0);

      Logger::get()->log( Logger::LOG_LEV_INFO, "DSP %s", (go)?"started":"stopped");

   } catch (AOException &e) {
       Logger::get()->log( Logger::LOG_LEV_ERROR, "Error %s DSP: %s", (go)?"starting":"stopping",e.what().c_str());
   }


    return NO_ERROR;
}



int FrameGrabberCtrl::DspProgReqChanged( void *pt, Variable *var)
{
   FrameGrabberCtrl *ctrl = (FrameGrabberCtrl *)pt;
   char *dspprog = var->Value.Sv;

      try {
         int status;
         std::string dspfile = Arcetri::Paths::dspDir();
         dspfile += dspprog;

         Logger::get()->log( Logger::LOG_LEV_DEBUG, "Loading DSP program %s", dspfile.c_str());

         ctrl->safeStop(&status);
         ctrl->wfs->setDSPprogram(dspfile,1);
         ctrl->safeStart(status);
         ctrl->var_dspprog_cur.Set(dspprog);
                                                            
         Logger::get()->log( Logger::LOG_LEV_INFO, "Loaded DSP program %s", dspfile.c_str());

      } catch (AOException &e) {
         Logger::get()->log( Logger::LOG_LEV_ERROR, "Error loading DSP program: %s", e.what().c_str());
      }

   return NO_ERROR;
}




int FrameGrabberCtrl::NumPixelsReqChanged( void *pt, Variable *var)
{
   FrameGrabberCtrl *ctrl = (FrameGrabberCtrl *)pt;
   int numpixels = var->Value.Lv[0];

   if ((numpixels <0) || (numpixels> ctrl->_nPixels))
       {
       Logger::get()->log( Logger::LOG_LEV_ERROR, "Invalid no. of pixels: %d\n", numpixels);
       return VALUE_OUT_OF_RANGE_ERROR;
       }

   Logger::get()->log( Logger::LOG_LEV_DEBUG, "Setting no. of pixels: %d\n", numpixels);

   try {
      int status;

      ctrl->safeStop(&status);
      ctrl->wfs->WriteBCU_DSPword(FRMGRAB_TOTFRAMEPIXELS, numpixels/8);

	   na_bcu_nios_fixed_area_struct dummy_struct;
      int offset = ((char *) &(dummy_struct.diagnostic_record_len) - ((char *) &dummy_struct))/ sizeof(uint32);
      ctrl->wfs->WriteBCU_SRAMword( offset + AddressMap::BCU_NIOS_FIXED_AREA, numpixels/2 );

      offset = ((char *) &(dummy_struct.wr_diagnostic_record_ptr) - ((char *) &dummy_struct))/ sizeof(uint32);
      ctrl->wfs->WriteBCU_SRAMword( offset + AddressMap::BCU_NIOS_FIXED_AREA, 0);
      printf("Offset: %08X\n", offset);

      ctrl->safeStart(status);

      ctrl->var_numpixels_cur.Set(numpixels);

    } catch (AOException &e) {
         Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting number of pixels: %s", e.what().c_str());
    }

	return NO_ERROR;
}

int FrameGrabberCtrl::DarkReqChanged( void *pt, Variable *var)
{
   FrameGrabberCtrl *ctrl = (FrameGrabberCtrl *)pt;
   char *dark = var->Value.Sv;

   try {
      //int status;
      int binning;
      ctrl->_ccd47Bin.Get(&binning);
      std::string darkfile = Arcetri::Paths::BackgDir( "ccd47", binning);
      darkfile += dark;

      //Logger::get()->log( Logger::LOG_LEV_INFO, "WARNING: frame grabber BCU has no dark functionality", darkfile.c_str());

      Logger::get()->log( Logger::LOG_LEV_DEBUG, "Setting dark to %s", darkfile.c_str());

      //ctrl->safeStop(&status);
      //ctrl->wfs->setPixelOffsets( darkfile, 1);
      ctrl->var_dark_cur.Set(dark);
      //ctrl->safeStart(status);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Dark set to %s", darkfile.c_str());

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting dark: %s", e.what().c_str());
   }

   return NO_ERROR;
}











		
//@Function: TestComm
//
// Tests the communication with the BCU hardware. Since this
// process is not intended to directly communicate with
// the BCU, a successful test requires also the appropriate
// BCU controller to be running.
//
// Returns NO_ERROR if ok or an error code.
//@

int FrameGrabberCtrl::TestComm(void)
{
	int buffer;
   try {
	   return wfs->GetBuffer( _bcuNum,0xFF, 0xFF, (unsigned char *)&buffer, 4, 0, MGP_OP_RDSEQ_SDRAM, 0, 1000);
   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error in TestComm(): %s", e.what().c_str());
      return COMMUNICATION_ERROR;
   }  

}	

//@Function: FirstConfig
//
// Sets the Slope computer in a known state, ready
// to be used by application programs
//@

int FrameGrabberCtrl::FirstConfig(void)
{	
	int offset,stat;
	na_bcu_nios_fixed_area_struct dummy_struct;

    wfs->stopDSP();
    msleep(1000);

    // Download code
    if (( stat = wfs->setDSPprogram(Arcetri::Paths::dspDir()+"/BCUDsp_FrameGrabber_1_00_cont.ldr")) != NO_ERROR)
    {
        fprintf(stderr, "Error in setDSPprogram: (%d) %s\n", stat, lao_strerror(stat));
        return stat;
    }

	// Initialize Frame Grabber parameters
    wfs->WriteBCU_DSPword(FRMGRAB_FRAMESCOUNTER,     FRMGRAB_FRAMESCOUNTER_DEFAULT);
    wfs->WriteBCU_DSPword(FRMGRAB_RESETFRAME,        FRMGRAB_RESETFRAME_DEFAULT);
    wfs->WriteBCU_DSPword(FRMGRAB_FRAMESCOUNTERIRQ,  FRMGRAB_FRAMESCOUNTERIRQ_DEFAULT );
    wfs->WriteBCU_DSPword(FRMGRAB_RESETFRAMEIRQ,     FRMGRAB_RESETFRAMEIRQ_DEFAULT );
    wfs->WriteBCU_DSPword(FRMGRAB_PIXELAREASIZE,     FRMGRAB_PIXELAREASIZE_DEFAULT  );
    wfs->WriteBCU_DSPword(FRMGRAB_TOTFRAMEPIXELS,    FRMGRAB_TOTFRAMEPIXELS_DEFAULT );
    wfs->WriteBCU_DSPword(FRMGRAB_NUMPIXELSTOMOVEPTR,FRMGRAB_NUMPIXELSTOMOVEPTR_DEFAULT);
    wfs->WriteBCU_DSPword(FRMGRAB_INPIXELAREAPTR,    FRMGRAB_INPIXELAREAPTR_DEFAULT );
    wfs->WriteBCU_DSPword(FRMGRAB_OUTPIXELAREAPTR,   FRMGRAB_OUTPIXELAREAPTR_DEFAULT );

    // Start DSP code
	wfs->startDSPprogram();

    // Setup NIOS irq diagnostic parameters
	offset = ((char *) &(dummy_struct.enable_pos_acc) - ((char *) &dummy_struct))/ sizeof(uint32);
    wfs->WriteBCU_SRAMword( offset + AddressMap::BCU_NIOS_FIXED_AREA, FRMGRAB_ENABLE_POS_ACC_DEFAULT );

	offset = ((char *) &(dummy_struct.diagnostic_record_ptr) - ((char *) &dummy_struct))/ sizeof(uint32);
    wfs->WriteBCU_SRAMword( offset + AddressMap::BCU_NIOS_FIXED_AREA, FRMGRAB_DIAGNOSTIC_BUFF_PTR_DEFAULT );

	offset = ((char *) &(dummy_struct.diagnostic_record_len) - ((char *) &dummy_struct))/ sizeof(uint32);
	wfs->WriteBCU_SRAMword( offset + AddressMap::BCU_NIOS_FIXED_AREA, FRMGRAB_DIAGNOSTIC_BUFF_LEN_DEFAULT );

	offset = ((char *) &(dummy_struct.enable_master_diag) - ((char *) &dummy_struct))/ sizeof(uint32);
    wfs->WriteBCU_SRAMword( offset + AddressMap::BCU_NIOS_FIXED_AREA, FRMGRAB_ENABLE_MASTER_DIAG_DEFAULT );
    
    // Download LUT - not necessary until the first change binning
	//wfs->sendPixelLUT("../config/lookuptables/eev47/bin1/techviewer.txt");	// lut, send

   // Master diagnostic config.


    uint8 masterIP[4], masterMAC[6];

    if (_masterD_enable)
      {
      int pos =0, pos2, i;
      for ( i=0; i<4; i++)
         {
         pos2 = _masterD_ip.find(".", pos);
         masterIP[i] = atoi( _masterD_ip.substr( pos, pos2-pos).c_str());
         pos = pos2+1;
         }

      pos = 0;
      for (i=0; i<6; i++)
         {
         pos2 = _masterD_mac.find(":", pos);
         masterMAC[i] = strtol( _masterD_ip.substr( pos, pos2-pos).c_str(), NULL, 16);
         pos = pos2+1;
         }
      }
    else
      {
      memset( &masterIP, 0, sizeof(uint8)*4);
      memset( &masterMAC, 0, sizeof(uint8)*6);
      _masterD_port = 0;
      _masterD_decimation = 0;
      }

    wfs->configureMasterDiagnostics( _masterD_enable, masterIP, masterMAC, _masterD_port, _masterD_decimation);


   // Set the CCD origin
   wfs->selectPixelOrigin( ORIGIN_CCD);

	printf("Configuration done\n");
	return NO_ERROR;
}


//@Function: safeStop and safeStart
//
// These two function can stop and re-start the DSP loop
// always keeping up-to-date status information in the MsgD-RTDB.
//
// The loop is re-started only if it was already running before,
// otherwise the safeStart() function does nothing. It needs the
// <previous state> parameter, returned by the safeStop() function.
//@

int FrameGrabberCtrl::safeStop( int *previous_state)
{
	int status, stat=NO_ERROR;
	status = getCurState();

	// Always force stop, for now
	stat = wfs->stopDSP();

	if (previous_state)
		*previous_state = status;

	if (status == STATE_OPERATING)
		{
		stat = wfs->stopDSP();
		if (stat == NO_ERROR)
         setCurState(STATE_READY);
		}

	return stat;
}	

int FrameGrabberCtrl::safeStart( int previous_state)
{
	int status, stat = NO_ERROR;
   status = getCurState();

	if ((status == STATE_READY) && (previous_state == STATE_OPERATING))
		{
		stat = wfs->startDSP();
		if (stat == NO_ERROR)
			setCurState(STATE_OPERATING);
		}

	return stat;
}


int main( int argc, char **argv) {

   SetVersion(VersMajor,VersMinor);

   try {
      FrameGrabberCtrl *c;
            
      c = new FrameGrabberCtrl( argc, argv);

      c->Exec();

      delete c;
   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}
