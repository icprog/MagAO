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
#include <math.h>    // NAN

#include <pthread.h>
 
#include "base/thrdlib.h"

#include "commlib.h"
#include "iolib.h"
}


#include "RTDBvar.h"
#include "BcuLib/BCUmsg.h"
#include "BCUfile.h"
#include "BCUlbt.h"
#include "SlopeComp.h"
#include "SlopeCompCtrl.h"
#include "AOStates.h"
#include "BcuLib/bcucommand.h"
#include "Paths.h"

#include "arblib/base/Alerts.h" 
#include "arblib/base/AlertsExport.h" 



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
const char *Date = "Mar 2005";
const char *Author = "A. Puglisi";

int CHAR_VAR_LEN = 128;			// Length of a "filename variable
int ERRMSG_LEN = 128;			// Length of the error message variable

int seqnum = 0;

// Define this to use the command history on the slope computer
// instead of the switch BCU
//#define USE_COMMANDHISTORY

//
//+Function: help
//
// Prints an usage message
//-
// Rimosso perche' non usato (L.F.)
//static void help()
//{
//    printf("SlopeCompCtrl  - Vers. %d.%d.  %s, %s\n\n", VersMajor,VersMinor,Author,Date);
//    printf("Usage: SlopeCompCtrl [-v] [-s <Server Address>]\n\n");
//    printf("   -s    specify server address (defaults to localhost)\n");
//    printf("   -v    verbose mode\n");
//    printf("\n");
//}


// +Main: 
//
// Main function
//-

SlopeCompCtrl::SlopeCompCtrl( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
   _testFastlink=0;
   //printf("argc: %d\n", argc);
   //for (int i=0; i<argc; i++) { printf("argv[%d] = %s\n", i, argv[i]); }

   if (argc>3) {
      printf("argv[3] =%s\n", argv[3]);
      if (strcmp(argv[3], "testfl") == 0)
         _testFastlink=1;
         printf("Test fastlink enabled\n");
   }
}

SlopeCompCtrl::~SlopeCompCtrl() {
   delete _arbNotif;
}

void SlopeCompCtrl::Create() throw (AOException)
{
   pthread_mutex_init( &dsp_mutex, NULL);

   try {
       _wfsName = (std::string) ConfigDictionary()["wfsName"];
       _bcuNum = ConfigDictionary()["bcuNum"];
       _flFilename = (std::string) ConfigDictionary()["flFilename"];

       _masterD_port = 0;
       _masterD_decimation=0;
       try {
          _masterD_enable = ConfigDictionary()["masterD_enable"];
          _masterD_port = ConfigDictionary()["masterD_port"];
          _masterD_decimation = ConfigDictionary()["masterD_decimation"];

          _arbNotif = new AlertNotifier( (std::string) ConfigDictionary()["wfsArbName"] + "." + Side());

       } catch (Config_File_Exception &e) {
           _logger->log( Logger::LOG_LEV_WARNING, "No master diagnostic configuration, will not be enabled. %s", e.what().c_str());
           _masterD_enable = 0;
       }

     _FLtimeoutEnable = 0;
     _FLtimeoutMax    = 5;

     if (! ConfigDictionary().has_key("FLtimeoutEnable")) {
         _logger->log( Logger::LOG_LEV_WARNING, "No keyword 'FLtimeoutEnable' defined in configuration file. Fastlink timeouts will not be checked.");
     } else {
         _FLtimeoutEnable = ConfigDictionary()["FLtimeoutEnable"];
         if (! ConfigDictionary().has_key("FLtimeoutMax"))
             _logger->log( Logger::LOG_LEV_WARNING, "No keyword 'FLtimeoutMax' defined in configuration file. Using default value of %d", _FLtimeoutMax);
         else
             _FLtimeoutMax = ConfigDictionary()["FLtimeoutMax"];
     }

     _FLCRCEnable = 0;
     _FLCRCMax    = 0;

     if (! ConfigDictionary().has_key("FLCRCEnable")) {
         _logger->log( Logger::LOG_LEV_WARNING, "No keyword 'FLCRCEnable' defined in configuration file. Fastlink CRC errors will not be checked.");
     } else {
         _FLCRCEnable = ConfigDictionary()["FLCRCEnable"];
         if (! ConfigDictionary().has_key("FLCRCMax"))
             _logger->log( Logger::LOG_LEV_WARNING, "No keyword 'FLCRCMax' defined in configuration file. Using default value of %d", _FLCRCMax);
         else
             _FLCRCMax = ConfigDictionary()["FLCRCMax"];
     }


    } catch (Config_File_Exception &e) {
        _logger->log( Logger::LOG_LEV_FATAL, "Missing configuration data: %s", e.what().c_str());
        throw AOException("Fatal: Missing configuration data");
    }


    wfs = new SlopeComp( _bcuNum, (char*)MyFullName().c_str());

}

void SlopeCompCtrl::Run()
{
    setCurState(STATE_NOCONNECTION);

    if (_testFastlink) {
       TestFastlink();
       exit(0);
    }

    while(!TimeToDie()) {
        try {
            DoFSM();
        } catch (AOException &e) {
            _logger->log( Logger::LOG_LEV_ERROR, "Caugth exception: %s", e.what().c_str());
        }

        checkFiber();
        msleep(1000);
   }

}

int SlopeCompCtrl::ccd39BinChanged( void *pt, Variable *var) {
   _logger->log(Logger::LOG_LEV_INFO, "Ccd39 current binning to %d", var->Value.Lv[0]);
   SlopeCompCtrl *SSC = (SlopeCompCtrl *)pt;
   try {
      SSC->_ccd39Bin.Set(var);
   }
   catch(AOVarException& e) {
      _logger->log(Logger::LOG_LEV_WARNING, "Local Ccd39 binning updating failed");
      return VAR_WRITE_ERROR;
   }
   return NO_ERROR;
}
       


//@Function: DoFSM
//
// Do a controller step. States can be changed asynchronosuly
// from this thread or the listening thread, and this function
// will properly react to a new state
//
// Many states include a msleep() function to slow down the thread
// when immediate action is not necessary

int SlopeCompCtrl::DoFSM(void)
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
         _logger->log( Logger::LOG_LEV_ERROR, "Lost communication with slope computer!");
         setCurState(STATE_NOCONNECTION);

         // Notify arbitrator
/*
         try {
            Error myerror("LOST COMMUNICATION WITH SLOPE COMPUTER", false);
            _arbNotif->notifyAlert(&myerror);
            _logger->log(Logger::LOG_LEV_INFO, "Alert to WFS Arbitrator succesfully sent! [%s:%d]", __FILE__,__LINE__);
         } catch(ArbitratorInterfaceException& e) {
            _logger->log(Logger::LOG_LEV_ERROR, "Alert to WFS Arbitrator not sent! [%s:%d]", __FILE__,__LINE__);
         }
*/

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
          setCurState(STATE_READY);
      break;

		case STATE_READY:
		msleep(1000);
		break;

		// Unknown states should not exist
		default:
		msleep(1000);
		break;
		}

	return stat;
}

void SlopeCompCtrl::SetupVars()
{
	int files_len = CHAR_VAR_LEN;

   try {
      var_name = RTDBvar( MyFullName() , "NAME", NO_DIR, CHAR_VARIABLE, _wfsName.size()+1);
      var_errmsg = RTDBvar( MyFullName() , "ERRMSG", NO_DIR, CHAR_VARIABLE, ERRMSG_LEN);

      var_go_req = RTDBvar( MyFullName() , "GO", REQ_VAR);
      var_go_cur = RTDBvar( MyFullName() , "GO", CUR_VAR);

      var_dark_req = RTDBvar( MyFullName() , "DARK", REQ_VAR, CHAR_VARIABLE, files_len);
      var_dark_cur = RTDBvar( MyFullName() , "DARK", CUR_VAR, CHAR_VARIABLE, files_len);

      var_pixellut_req = RTDBvar( MyFullName() , "PIXELLUT", REQ_VAR, CHAR_VARIABLE, files_len);
      var_pixellut_cur = RTDBvar( MyFullName() , "PIXELLUT", CUR_VAR, CHAR_VARIABLE, files_len);

      var_pixelgains_req = RTDBvar( MyFullName() , "PIXELGAINS", REQ_VAR, CHAR_VARIABLE, files_len);
      var_pixelgains_cur = RTDBvar( MyFullName() , "PIXELGAINS", CUR_VAR, CHAR_VARIABLE, files_len);

      var_slopenull_req = RTDBvar( MyFullName() , "SLOPENULL", REQ_VAR, CHAR_VARIABLE, files_len);
      var_slopenull_cur = RTDBvar( MyFullName() , "SLOPENULL", CUR_VAR, CHAR_VARIABLE, files_len);

      var_dspprog_req = RTDBvar( MyFullName() , "DSP_PROGRAM", REQ_VAR, CHAR_VARIABLE, files_len);
      var_dspprog_cur = RTDBvar( MyFullName() , "DSP_PROGRAM", CUR_VAR, CHAR_VARIABLE, files_len);

      var_th_vector_req = RTDBvar( MyFullName() , "TIMEHISTORY.VECTOR", REQ_VAR, CHAR_VARIABLE, files_len);
      var_th_vector_cur = RTDBvar( MyFullName() , "TIMEHISTORY.VECTOR", CUR_VAR, CHAR_VARIABLE, files_len);

      var_th_amp_req = RTDBvar( MyFullName() , "TIMEHISTORY.VECTOR.AMP", REQ_VAR, REAL_VARIABLE);
      var_th_amp_cur = RTDBvar( MyFullName() , "TIMEHISTORY.VECTOR.AMP", CUR_VAR, REAL_VARIABLE);

      var_th_const_req = RTDBvar( MyFullName() , "TIMEHISTORY.VECTOR.CONST", REQ_VAR, REAL_VARIABLE);
      var_th_const_cur = RTDBvar( MyFullName() , "TIMEHISTORY.VECTOR.CONST", CUR_VAR, REAL_VARIABLE);

      var_th_periods_req = RTDBvar( MyFullName() , "TIMEHISTORY.VECTOR.PERIODS", REQ_VAR, REAL_VARIABLE);
      var_th_periods_cur = RTDBvar( MyFullName() , "TIMEHISTORY.VECTOR.PERIODS", CUR_VAR, REAL_VARIABLE);

      var_th_offsets_req = RTDBvar( MyFullName() , "TIMEHISTORY.OFFSETS", REQ_VAR, CHAR_VARIABLE, files_len);
      var_th_offsets_cur = RTDBvar( MyFullName() , "TIMEHISTORY.OFFSETS", CUR_VAR, CHAR_VARIABLE, files_len);

      var_th_enable_req = RTDBvar( MyFullName() , "TIMEHISTORY.ENABLE", REQ_VAR);
      var_th_enable_cur = RTDBvar( MyFullName() , "TIMEHISTORY.ENABLE", CUR_VAR);

      var_cmd_offsets_req = RTDBvar( MyFullName() , "CMD_HISTORY.OFFSETS", REQ_VAR, CHAR_VARIABLE, files_len);
      var_cmd_offsets_cur = RTDBvar( MyFullName() , "CMD_HISTORY.OFFSETS", CUR_VAR, CHAR_VARIABLE, files_len);

      var_cmd_enable_req = RTDBvar( MyFullName() , "CMD_HISTORY.ENABLE", REQ_VAR);
      var_cmd_enable_cur = RTDBvar( MyFullName() , "CMD_HISTORY.ENABLE", CUR_VAR);

      var_fl_cmd_req = RTDBvar( MyFullName() , "FASTLINK.CMD", REQ_VAR, CHAR_VARIABLE, files_len);
      var_fl_cmd_cur = RTDBvar( MyFullName() , "FASTLINK.CMD", CUR_VAR, CHAR_VARIABLE, files_len);

      var_fl_enable_req = RTDBvar( MyFullName() , "FASTLINK.ENABLE", REQ_VAR);
      var_fl_enable_cur = RTDBvar( MyFullName() , "FASTLINK.ENABLE", CUR_VAR);

      var_disturb_enable_req = RTDBvar( MyFullName(), "DISTURBANCE.WFS.ENABLE", REQ_VAR);
      var_disturb_enable_cur = RTDBvar( MyFullName(), "DISTURBANCE.WFS.ENABLE", CUR_VAR);

      var_adsecdiagn_enable_req = RTDBvar( MyFullName() , "ADSECDIAGN.ENABLE", REQ_VAR);
      var_adsecdiagn_enable_cur = RTDBvar( MyFullName() , "ADSECDIAGN.ENABLE", CUR_VAR);

      var_calcmode_req = RTDBvar( MyFullName() , "CALC_MODE", REQ_VAR);
      var_calcmode_cur = RTDBvar( MyFullName() , "CALC_MODE", CUR_VAR);

      var_fluxgain_req = RTDBvar( MyFullName() , "FLUXGAIN", REQ_VAR, REAL_VARIABLE);
      var_fluxgain_cur = RTDBvar( MyFullName() , "FLUXGAIN", CUR_VAR, REAL_VARIABLE);

      var_pixelorig_req = RTDBvar( MyFullName() , "PIXELORIG", REQ_VAR);
      var_pixelorig_cur = RTDBvar( MyFullName() , "PIXELORIG", CUR_VAR);

      var_nsubaps_req = RTDBvar( MyFullName() , "NSUBAPS", REQ_VAR);
      var_nsubaps_cur = RTDBvar( MyFullName() , "NSUBAPS", CUR_VAR);

      var_masterd_decimation_req = RTDBvar( MyFullName() , "MASTERD.DECIMATION", REQ_VAR);
      var_masterd_decimation_cur = RTDBvar( MyFullName() , "MASTERD.DECIMATION", CUR_VAR);

      var_clip_req = RTDBvar( MyFullName() , "SLOPECLIP", REQ_VAR, REAL_VARIABLE);
      var_clip_cur = RTDBvar( MyFullName() , "SLOPECLIP", CUR_VAR, REAL_VARIABLE);

      Notify( var_go_req, GoReqChanged);
      Notify( var_dark_req, DarkReqChanged);
      Notify( var_pixellut_req, PixelLUTReqChanged);
      Notify( var_pixelgains_req, PixelGainsReqChanged);
      Notify( var_slopenull_req, SlopenullReqChanged);
      Notify( var_dspprog_req, DspProgReqChanged);
      Notify( var_th_vector_req, ThVectorReqChanged);
      Notify( var_th_offsets_req, ThOffsetsReqChanged);
      Notify( var_th_amp_req, ThAmpReqChanged);
      Notify( var_th_const_req, ThConstReqChanged);
      Notify( var_th_periods_req, ThPeriodsReqChanged);
      Notify( var_th_enable_req, ThEnableReqChanged);
#ifdef USE_COMMANDHISTORY
      Notify( var_cmd_offsets_req, CmdOffsetsReqChanged);
      Notify( var_cmd_enable_req, CmdEnableReqChanged);
#endif
      Notify( var_fl_cmd_req, FastlinkCmdReqChanged);
      Notify( var_fl_enable_req, FastlinkEnableReqChanged);
      Notify( var_adsecdiagn_enable_req, AdsecDiagnEnableReqChanged);
      Notify( var_calcmode_req, CalcModeReqChanged);
      Notify( var_fluxgain_req, FluxGainReqChanged);
      Notify( var_nsubaps_req, NSubapsReqChanged);
      Notify( var_pixelorig_req, PixelOrigReqChanged);
      Notify( var_masterd_decimation_req, MasterDiagnDecimationReqChanged);
      Notify( var_disturb_enable_req, DisturbEnableReqChanged);
      Notify( var_clip_req, ClipReqChanged);

     // Configure a notification on ccd39 binning variable
     string ccd39Process = (string) ConfigDictionary()["ccdProcess"]+"."+Side();

     // Wait for ccd39 control process to become ready, otherwise there's no variable to attach!
     while(1) {
        int ret;
        _logger->log( Logger::LOG_LEV_INFO, "Waiting for %s controller to become ready....", ccd39Process.c_str());
        if ((ret = thWaitReady(ccd39Process.c_str(), 60 * 1000)) == NO_ERROR)
           break;
        if (ret != TIMEOUT_ERROR)
           throw AOException("Error waiting for ccd39 to become ready", ret, __FILE__, __LINE__);
     }

     _ccd39Bin = RTDBvar(ccd39Process + ".XBIN.CUR");  // Also get the value
     Notify(_ccd39Bin, ccd39BinChanged);

     _ccd39Enable = RTDBvar(ccd39Process + ".ENABLE.REQ");
     _ccd39EnableCur = RTDBvar(ccd39Process + ".ENABLE.CUR");
   
      } catch (AOVarException &e) {
         _logger->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
         throw AOException("Error creating RTDB variables");
      }
}

int SlopeCompCtrl::MasterDiagnDecimationReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
   int decimation = var->Value.Lv[0];

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting masterdiagnostic decimation to %d", decimation);

      ctrl->safeStop(&status);
      ctrl->_masterD_decimation=decimation;
      ctrl->wfs->configureMasterDiagnostics( ctrl->_masterD_enable, ctrl->_masterIP, ctrl->_masterMAC, ctrl->_masterD_port, ctrl->_masterD_decimation);
      ctrl->safeStart(status);

      ctrl->var_masterd_decimation_cur.Set(decimation);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Masterdiagnostic decimation set to %d", decimation);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting masterdiagnostic decimation: %s", e.what().c_str());
   } catch (...) {}
   ctrl->unlock();

   return NO_ERROR;
}


int SlopeCompCtrl::NSubapsReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
   int nsubaps = var->Value.Lv[0];

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting n. of subapertures to %d", nsubaps);

      ctrl->safeStop(&status);
      ctrl->wfs->setNsubap(nsubaps);
      ctrl->var_nsubaps_cur.Set(nsubaps);
      ctrl->safeStart(status);

      Logger::get()->log( Logger::LOG_LEV_INFO, "N. of subapertures set to %d", nsubaps);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting n. of subapertures: %s", e.what().c_str());
   } catch (...) {}
   ctrl->unlock();

	return NO_ERROR;
}

int SlopeCompCtrl::PixelGainsReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	char *pixelgains = var->Value.Sv;

   ctrl->lock();
   try {
      int status;

      std::string gainsfile = Arcetri::Paths::gainsDir()+"/";
      gainsfile += pixelgains;

      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting pixelgains to %s", gainsfile.c_str());

	   ctrl->safeStop(&status);
		ctrl->wfs->setPixelGains(gainsfile);
		ctrl->var_pixelgains_cur.Set(pixelgains);
		ctrl->safeStart(status);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Pixelgains set to %s", gainsfile.c_str());

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting pixelgains: %s", e.what().c_str());
   } catch (...) {}
   ctrl->unlock();

   return NO_ERROR;
}

	
int SlopeCompCtrl::DarkReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	char *dark = var->Value.Sv;

   ctrl->lock();
   try {
      int status;
      int binning;
      ctrl->_ccd39Bin.Get(&binning);
      std::string darkfile = Arcetri::Paths::BackgDir( "ccd39", binning) + "/" ;
      if (strcmp(dark, "") == 0)
         darkfile = "";
      else
	      darkfile += dark;

      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting dark to %s", darkfile.c_str());

      //	   ctrl->safeStop(&status);
      ctrl->wfs->stopDSP();
      msleep(100);
	   

      if (darkfile != "")
		   ctrl->wfs->setPixelOffsets( darkfile, 1);
      else
         ctrl->wfs->setPixelOffsets(0, 1);

 
      msleep(100);
      //		ctrl->safeStart(status);
      ctrl->wfs->startDSP();

		ctrl->var_dark_cur.Set(dark, FORCE_SEND);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Dark set to %s", darkfile.c_str());
   
   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting dark: %s", e.what().c_str());
   } catch (...) {}
   ctrl->unlock();

	return NO_ERROR;
}

int SlopeCompCtrl::PixelLUTReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
   char *pixellut = var->Value.Sv;

   ctrl->lock();
   try {   
      //std::string lutfile = Arcetri::Paths::LUTsDir( "ccd39");
      std::string lutfile = "";
	   lutfile += pixellut;

      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting pixel LUT to %s", lutfile.c_str());

      ctrl->wfs->stopDSP();
		ctrl->wfs->sendPixelLUT( lutfile);
		ctrl->var_pixellut_cur.Set(pixellut, FORCE_SEND);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Pixel LUT set to %s", lutfile.c_str());

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting pixel LUT: %s", e.what().c_str());
   } catch (...) {}
   ctrl->unlock();

   return NO_ERROR;
}

int SlopeCompCtrl::SlopenullReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
   char *slopenull = var->Value.Sv;

   ctrl->lock();
   try {
      int status;
      int binning;
      ctrl->_ccd39Bin.Get(&binning);
      std::string slopefile = Arcetri::Paths::slopenullDir( binning)+"/";
	   slopefile += slopenull;

      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting slopenull to %s", slopefile.c_str());

	   ctrl->safeStop(&status);
		ctrl->wfs->setSlopeOffsets( slopefile, 1);
		ctrl->var_slopenull_cur.Set(slopenull, FORCE_SEND);
		ctrl->safeStart(status);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Slopenull set to %s", slopefile.c_str());

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting slopenull: %s", e.what().c_str());
   } catch (...) {}
   ctrl->unlock();

   return NO_ERROR;
}

int SlopeCompCtrl::PixelOrigReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
   int origin = var->Value.Lv[0];

   ctrl->lock();
   try {
      int status;

	   ctrl->safeStop(&status);
		ctrl->wfs->selectPixelOrigin(origin);
		ctrl->var_pixelorig_cur.Set(origin);
		ctrl->safeStart(status);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting pixel origin: %s", e.what().c_str());
   } catch (...) {}
   ctrl->unlock();

   return NO_ERROR;
}


int SlopeCompCtrl::GoReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
   int go = var->Value.Lv[0];
   int status;

   ctrl->lock();
   try {
      status = ctrl->getCurState();

      Logger::get()->log( Logger::LOG_LEV_INFO, "Received %s DSP request", (go)?"start":"stop");

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
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}
	
int SlopeCompCtrl::DspProgReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	char *dspprog = var->Value.Sv;

   ctrl->lock();
   try {
      int status;
	   string dspfile = Arcetri::Paths::dspDir()+"/";
	   dspfile += dspprog;

      Logger::get()->log( Logger::LOG_LEV_INFO, "Loading DSP program %s", dspfile.c_str());

	   ctrl->safeStop(&status);
		ctrl->wfs->setDSPprogram(dspfile,1);
		ctrl->safeStart(status);
      ctrl->var_dspprog_cur.Set(dspprog);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Loaded DSP program %s", dspfile.c_str());

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error loading DSP program: %s", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();
	
	return NO_ERROR;
}	

int SlopeCompCtrl::ThAmpReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	double amp = var->Value.Dv[0];
   double periods;

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting time history amplitude to %f", amp);

      ctrl->safeStop(&status);
      ctrl->var_th_periods_cur.Get(&periods);
      ctrl->wfs->setTimeHistoryVector( amp, periods);
      ctrl->safeStart(status);
      ctrl->var_th_amp_cur.Set(amp);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Time history amplitude set to %f", amp);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting time history amplitude: %s", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}


int SlopeCompCtrl::ThPeriodsReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
   double periods = var->Value.Dv[0];
   double amp;

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting time history periods to %f", periods);

      ctrl->safeStop(&status);
      ctrl->var_th_amp_req.Get(&amp);
      ctrl->wfs->setTimeHistoryVector( amp, periods);
      ctrl->safeStart(status);
      ctrl->var_th_periods_cur.Set(periods);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Set time history periods to %f", periods);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting time history periods: %s", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}

	// ---------------------------------------
	// Time history start/stop

int SlopeCompCtrl::ThEnableReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	int enable = var->Value.Lv[0];

   ctrl->lock(); 
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "%s time history", (enable)?"Enabling":"Disabling");

      ctrl->safeStop(&status);
      if (enable)
          ctrl->wfs->TimeHistoryStart();
      else
          ctrl->wfs->TimeHistoryStop();
      ctrl->safeStart(status);
      ctrl->var_th_enable_cur.Set( (enable)?1:0);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error %s time history: %s", (enable)?"Enabling":"Disabling", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}

	// ---------------------------------------
	// Command history start/stop

#ifdef USE_COMMANDHISTORY
int SlopeCompCtrl::CmdEnableReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	int enable = var->Value.Lv[0];

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "%s command history", (enable)?"Enabling":"Disabling");

      ctrl->safeStop(&status);
		if (enable)
			ctrl->wfs->CommandHistoryStart();
		else
		   ctrl->wfs->CommandHistoryStop();

		ctrl->var_cmd_enable_cur.Set( (enable)?1:0);
      ctrl->safeStart(status);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error %s command history: %s", (enable)?"Enabling":"Disabling", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}

	// -------------------
	// Command history file

int SlopeCompCtrl::CmdOffsetsReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	char *filename = var->Value.Sv;

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting command history offsets to %s", filename);

      ctrl->safeStop(&status);
      ctrl->wfs->setCommandHistoryArea(filename);
	   ctrl->var_cmd_offsets_cur.Set(filename);
      ctrl->safeStart(status);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Command history offsets set to %s", filename);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting command history: %s", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}
#endif
	
	// -----------------
	// Time history offsets file

int SlopeCompCtrl::ThOffsetsReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	char *filename = var->Value.Sv;

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting time history offsets to %s", filename);

      ctrl->safeStop(&status);
      ctrl->wfs->setTimeHistoryOffsets(filename);
	   ctrl->var_th_offsets_cur.Set(filename, FORCE_SEND);
      ctrl->safeStart(status);

      // TODO
      //ctrl->wfs->setPixelGains(0);
      //ctrl->wfs->setPixelOffsets(0);


      Logger::get()->log( Logger::LOG_LEV_INFO, "Time history offsets set to %s", filename);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting time history offsets: %s", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}
	// -----------------
	// Time history vector file

int SlopeCompCtrl::ThVectorReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	char *filename = var->Value.Sv;

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting time history vector to %s", filename);

      ctrl->safeStop(&status);
      ctrl->wfs->setTimeHistoryVector(filename);
	   ctrl->var_th_vector_cur.Set(filename);
      ctrl->safeStart(status);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Time history vector set to %s", filename);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting time history vector: %s", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}

	// ------------------
	// Time history constant value

int SlopeCompCtrl::ThConstReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	double constant = var->Value.Dv[0];

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting time history constant to %f", constant);

      ctrl->safeStop(&status);
      ctrl->wfs->setTimeHistoryVector(constant);
	   ctrl->var_th_const_cur.Set(constant);
      ctrl->safeStart(status);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Time history constant set to %f", constant);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting time history constant: %s", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}

int SlopeCompCtrl::FastlinkCmdReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	char *filename = var->Value.Sv;

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting fastlink commands to %s", filename);

      //ctrl->safeStop(&status);
      ctrl->wfs->setFastLinkArea(filename);
	   ctrl->var_fl_cmd_cur.Set(filename);
      //ctrl->safeStart();

      Logger::get()->log( Logger::LOG_LEV_INFO, "Fastlink commands set to %s", filename);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting fastlink commands: %s", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}

// ----------------
// Disturbance enable/disable

int SlopeCompCtrl::DisturbEnableReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	int enable = var->Value.Lv[0];

   ctrl->lock();
   try {
      int status;

      Logger::get()->log( Logger::LOG_LEV_INFO, "Changing disturbance %d", enable);

      ctrl->wfs->stopDSP();
      //ctrl->safeStop(&status);
      switch(enable) {

         case 0:
         ctrl->wfs->ParamBlockSelector_disable( PBS_DISTURB_WFS);
         ctrl->wfs->ParamBlockSelector_disable( PBS_DISTURB_OVS);
         ctrl->var_disturb_enable_cur.Set(enable);
         break;

         case 1:
         ctrl->wfs->ParamBlockSelector_enable( PBS_DISTURB_WFS);
         ctrl->wfs->ParamBlockSelector_disable( PBS_DISTURB_OVS);
         ctrl->var_disturb_enable_cur.Set(enable);
         break;

         case 2:
         ctrl->wfs->ParamBlockSelector_disable( PBS_DISTURB_WFS);
         ctrl->wfs->ParamBlockSelector_enable( PBS_DISTURB_OVS);
         ctrl->var_disturb_enable_cur.Set(enable);
         break;

         case 3:
         ctrl->wfs->ParamBlockSelector_enable( PBS_DISTURB_WFS);
         ctrl->wfs->ParamBlockSelector_enable( PBS_DISTURB_OVS);
         ctrl->var_disturb_enable_cur.Set(enable);
         break;

         default:
         Logger::get()->log( Logger::LOG_LEV_INFO, "Invalid value for disturbance: %d. Valid values are 0-3.", enable);
         break;
      }
      ctrl->wfs->startDSP();
      //ctrl->safeStart(status);

    } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting disturbance to %d: %s", enable, e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}





    // ---------------------
    // Fastlink enable/disable

int SlopeCompCtrl::FastlinkEnableReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	int enable = var->Value.Lv[0];

   ctrl->lock();
   try {
      int status;

      Logger::get()->log( Logger::LOG_LEV_INFO, "%s fastlink", (enable)?"Enabling":"Disabling");

      //ctrl->wfs->stopDSP();
      if (enable) {
	  msleep(100);
          ctrl->wfs->ParamBlockSelector_enable( PBS_FASTLINK);
      }
      else {
          ctrl->wfs->ParamBlockSelector_disable( PBS_FASTLINK);
	  msleep(110);
      }
      //ctrl->wfs->startDSP();

      ctrl->var_fl_enable_cur.Set( (enable)?1:0, 0, FORCE_SEND);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error %s fastlink: %s", (enable)?"Enabling":"Disabling", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}
    // ---------------------
    // Adsec diagn enable/disable

int SlopeCompCtrl::AdsecDiagnEnableReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	int enable = var->Value.Lv[0];

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "%s adsec diagnostic", (enable)?"Enabling":"Disabling");

      ctrl->safeStop(&status);
      if (enable)
          ctrl->wfs->ParamBlockSelector_enable( PBS_DIAGNOSTIC);
      else
          ctrl->wfs->ParamBlockSelector_disable( PBS_DIAGNOSTIC);

      ctrl->safeStart(status);

      ctrl->var_adsecdiagn_enable_cur.Set( (enable)?1:0, 0, FORCE_SEND);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error %s adsec diagnostic: %s", (enable)?"Enabling":"Disabling", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}


     // ---------------------
     // Slope calculation method (SH/tomografy/constant)

int SlopeCompCtrl::CalcModeReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	int mode = var->Value.Lv[0];

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting calculation method to %d", mode);

      ctrl->safeStop(&status);
      if (mode == CALC_METHOD_SH)
         ctrl->wfs->ParamBlockSelector_enable( PBS_SH);
      else if (mode == CALC_METHOD_TOMOGRAPHY)
         ctrl->wfs->ParamBlockSelector_enable( PBS_TOMOGRAPHY);
      else if (mode == CALC_METHOD_CONSTANT) { 
         ctrl->wfs->ParamBlockSelector_enable( PBS_CONSTANT);
         // Put a NaN here just for fun
         float32 nan = NAN;
         ctrl->wfs->SendBuffer( ctrl->wfs->getNumber(), 0xFF, 0xFF, (unsigned char *)&nan, sizeof(float32), SLPCOMP__BSCFB_SLOPECONSTANT, MGP_OP_WRSEQ_DSP);
      }

      ctrl->safeStart(status);

      ctrl->var_calcmode_cur.Set(mode);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Calculation method set to %d", mode);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting calculation method: %s", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}

     // ---------------------
     // Total flux gain (for tomography)

int SlopeCompCtrl::FluxGainReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
	double gain = var->Value.Dv[0];

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting flux gain to %f", gain);

      ctrl->safeStop(&status);
      ctrl->wfs->setRealNsubaps(gain);
      ctrl->safeStart(status);
      ctrl->var_fluxgain_cur.Set(gain);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Flux gain set to %f", gain);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting flux gain: %s", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

   return NO_ERROR;
}

     // ---------------------
     // Slope clip value

int SlopeCompCtrl::ClipReqChanged( void *pt, Variable *var)
{
   SlopeCompCtrl *ctrl = (SlopeCompCtrl *)pt;
   double clip = var->Value.Dv[0];

   ctrl->lock();
   try {
      int status;
      Logger::get()->log( Logger::LOG_LEV_INFO, "Setting slope clip to %f", clip);

      ctrl->safeStop(&status);
      ctrl->wfs->setSlopeClip(clip);
      ctrl->safeStart(status);
      ctrl->var_clip_cur.Set(clip);

      Logger::get()->log( Logger::LOG_LEV_INFO, "Slope clip set to %f", clip);

   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error setting slope clip: %s", e.what().c_str());
   } catch (...) {}		
   ctrl->unlock();

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

int SlopeCompCtrl::TestComm(void)
{
   int buffer;
   try {
      return wfs->GetBuffer( _bcuNum, 0xFF, 0xFF, (unsigned char *)&buffer, 4, 0, MGP_OP_RDSEQ_SDRAM, 0, 1000);
   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error in TestComm(): %s", e.what().c_str());
      return COMMUNICATION_ERROR;
   }
}	


int SlopeCompCtrl::TestFastlink(void) {


   wfs->stopDSP();
   string filename = Arcetri::Paths::dspDir()+"/TestFaslink.ldr";

    int num_cycles = 60;
    int stat;

    int BANK_1             = 0x00080000;
    int BANK_2             = 0x00100000;

    int p_flinterface        = 0x1000;
    int p_write_buffer_len   = 0x80000;
    int p_first_dsp          = 0x80001;
    int p_first_crt          = 0x80002;
    int p_last_dsp           = 0x80003;
    int p_last_crt           = 0x80004;
    int p_tot_num_dsp        = 0x80005;
    int p_remote_addr        = 0x80006;
    int p_counter            = 0x80080;
    int p_counter1           = 0x80087;
    int p_memcmp_good_cnt    = 0x80082;
    int p_timeout_wr_err_cnt = 0x80084;
    int p_timeout_rd_err_cnt = 0x80086;
    int p_crc_wr_err_cnt     = 0x80088;
    int p_crc_rd_err_cnt     = 0x8008a;
    int p_memcmp_err_cnt     = 0x8008c;
    int p_enabled            = 0x8008e;
    int p_irq_signal         = 0x8008f;

    int p_write_buffer       = 0x080800;
    int p_read_buffer        = 0x100000;

    int FIRST_DSP          = 0xFF; 
    int LAST_DSP           = 0xFF; 
    int tot_num_dsp        = 1;
    int FIRST_CRATE        = 0;
    int LAST_CRATE         = 0;
    int WRITE_BUFFER_SIZE  = 0xB000;
    int READ_BUFFER_SIZE   = 0x10000;

    int write_buffer_len = 20;
    int remote_addr = 0x8000;

    if (( stat = wfs->setDSPprogram(filename)) != NO_ERROR) {
        Logger::get()->log( Logger::LOG_LEV_ERROR, "Error in setDSPprogram: (%d) %s, file %s\n", stat, lao_strerror(stat), filename.c_str());
        return stat;
        }

     wfs->WriteBCU_DSPword( p_write_buffer_len, write_buffer_len);
     wfs->WriteBCU_DSPword( p_first_dsp, FIRST_DSP);
     wfs->WriteBCU_DSPword( p_first_crt, FIRST_CRATE);
     wfs->WriteBCU_DSPword( p_last_dsp, LAST_DSP);
     wfs->WriteBCU_DSPword( p_last_crt, LAST_CRATE);
     wfs->WriteBCU_DSPword( p_tot_num_dsp, tot_num_dsp);
     wfs->WriteBCU_DSPword( p_remote_addr, remote_addr);
     wfs->WriteBCU_DSPword( p_enabled, 1);


     for (int i=0; i< num_cycles; i++) {

       uint32 buffer[14];
       uint32 addr = p_counter;;
       int opcode = MGP_OP_RDSEQ_DSP;
       wfs->GetBuffer( _bcuNum, 0xFF, 0xFF, (unsigned char *)&buffer, sizeof(uint32)*14, addr, opcode);

       long long *buf64 = (long long *)buffer;
       printf("TIM_WR: %d; TIM_RD: %d; CRC_WR: %d; CRC_RD: %d; MEM_CMP: %d\n", buf64[2], buf64[3], buf64[4], buf64[5], buf64[6]);

       if (TimeToDie()) break;
       sleep(1);
     }

}

 

//@Function: FirstConfig
//
// Sets the Slope computer in a known state, ready
// to be used by application programs
//@

int SlopeCompCtrl::FirstConfig(void)
{	
	int i, offset, stat;
	na_bcu_nios_fixed_area_struct dummy_struct;

    wfs->stopDSP();
    msleep(1000);
    string filename = Arcetri::Paths::dspDir()+"/SlopeCalculator_5_02.ldr";

	if (( stat = wfs->setDSPprogram(filename)) != NO_ERROR)
        {
        Logger::get()->log( Logger::LOG_LEV_ERROR, "Error in setDSPprogram: (%d) %s, file %s\n", stat, lao_strerror(stat), filename.c_str());
        return stat;
        }


	// Reconfigure ALL dsp memory (pointers, parameters and high speed link)

        var_nsubaps_cur.Set(DEFAULT_NUMSLOPES*2);
	wfs->setNsubap(DEFAULT_NUMSLOPES*2);            // Also writes SLPCOMP_NUMSLOPES

	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_NUMSH,                   DEFAULT_NUMSH);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_NULLSLOPECNT,            DEFAULT_NULLSLOPECNT);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCFB_SLOPECONSTANT,           DEFAULT_SLOPECONSTANT);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_NUMCONTURPIXELS,         DEFAULT_NUMCONTURPIXELS);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_NUMTOTPIXELS,            DEFAULT_NUMTOTPIXELS);

	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_PARAMSELECTOR,           DEFAULT_PARAMSELECTOR);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_NUMSLOPESTODOPTR,        DEFAULT_NUMSLOPESTODOPTR);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_SLOPEPIXELAREAPTR,       DEFAULT_SLOPEPIXELAREAPTR);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_PARAMSLOPEPIXELAREAPTR,  DEFAULT_PARAMSLOPEPIXELAREAPTR);

	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_CONTURPIXELAREAPTR,      DEFAULT_CONTURPIXELAREAPTR);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_PARAMCONTURPIXELAREAPTR, DEFAULT_PARAMCONTURPIXELAREAPTR);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_AVERAGEFLUXAREAPTR,      DEFAULT_AVERAGEFLUXAREAPTR);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_PARAMAVERAGEFLUXAREAPTR, DEFAULT_PARAMAVERAGEFLUXAREAPTR);

	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_SLOPELUTAREAPTR,         DEFAULT_SLOPELUTAREAPTR);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCFB_SLOPELUTDEPTH,           DEFAULT_SLOPELUTDEPTH);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_SLOPEOUTAREAPTR,         DEFAULT_SLOPEOUTAREAPTR);

	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_STARTRTRPTR,             DEFAULT_STARTRTRPTR);
//	wfs->WriteBCU_DSPword( SLPCOMP_FFCOMMANDVECTORPTR,      DEFAULT_FFCOMMANDVECTORPTR);
//	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_STARTFFPTR,              DEFAULT_STARTFFPTR);
//	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_MODESVECTORPTR,          DEFAULT_MODESVECTORPTR);
//	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_STARTMMPTR,              DEFAULT_STARTMMPTR);

	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_TIMEHISTORYACT,          DEFAULT_TIMEHISTORYACT);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_TIMEHISTORYPTR,          DEFAULT_TIMEHISTORYPTR);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_TIMEHISTORYENDPTR,       DEFAULT_TIMEHISTORYEND);

#ifdef USE_COMMANDHISTORY
	wfs->WriteBCU_DSPword( SLPCOMP_MODEHISTORYACT,       DEFAULT_MODEHISTORYACT);
	wfs->WriteBCU_DSPword( SLPCOMP_MODEHISTORYPTR,       DEFAULT_MODEHISTORYPTR);
	wfs->WriteBCU_DSPword( SLPCOMP_MODEHISTORYEND,       DEFAULT_MODEHISTORYEND);
#endif

	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_REPLYRECEIVED,           DEFAULT_REPLYRECEIVED);
//	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_NUMACTUATORS,            DEFAULT_NUMACTUATORS);
	wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_NUMFLTIMEOUT,            DEFAULT_NUMFLTIMEOUT);

//    wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_REPLYVECTORPTR,          DEFAULT_REPLYVECTORPTR);
    wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_NUMFLCRCERR,                DEFAULT_NUMCRC);


    // Zero startRTR
//    wfs->WriteBCU_DSPword( DEFAULT_STARTRTRPTR, 0);
//    wfs->WriteBCU_DSPword( DEFAULT_STARTRTRPTR+1, 0);
//    wfs->WriteBCU_DSPword( DEFAULT_STARTRTRPTR+2, 0);
//    wfs->WriteBCU_DSPword( DEFAULT_STARTRTRPTR+3, 0);

	// Synchronize local copy with BCU
	wfs->ParamBlockSelector_enable( DEFAULT_PARAMSELECTOR);
        var_fl_enable_cur.Set((DEFAULT_PARAMSELECTOR & PBS_FASTLINK) ? 1 : 0);

        string flFilename = Arcetri::Paths::dspDir() + "/";
        flFilename += _flFilename.c_str();
	wfs->setFastLinkArea( flFilename);
	var_fl_cmd_cur.Set(flFilename);

	// Reconfigure diagnostic transfer to SDRAM

   // Size of diagnostic buffer:
   // hdr_size + tot_pixels/2 + avg_area_size + footer_size
   // that is
   // 4 + tot_pixels/2 + 4 + 4
   //
   //
   // Pointer to diagnostic buffer:
   // Average_area_ptr - (tot_pixels/2) - 4.
   // And the most significant bit must be turned on too.

   uint32 record_len = 4 + (DEFAULT_NUMTOTPIXELS*8)/2 + DEFAULT_NUMSLOPES*4 + 4 + 4;
   Logger::get()->log( Logger::LOG_LEV_DEBUG, "Record len: %d (0x%08X)", record_len, record_len);
   uint32 record_ptr = DEFAULT_AVERAGEFLUXAREAPTR - (DEFAULT_NUMTOTPIXELS*8)/2 -4;
   record_ptr |= 0x80000000;
   Logger::get()->log( Logger::LOG_LEV_DEBUG, "Record ptr: %d (0x%08X)", record_ptr&0xEFFFFFFF, record_ptr&0xEFFFFFFF);

	offset = ((char *) &(dummy_struct.diagnostic_record_ptr) - ((char *) &dummy_struct))/ sizeof(uint32);
	wfs->WriteBCU_SRAMword( offset + AddressMap::BCU_NIOS_FIXED_AREA, record_ptr);
	offset = ((char *) &(dummy_struct.diagnostic_record_len) - ((char *) &dummy_struct))/ sizeof(uint32);
	wfs->WriteBCU_SRAMword( offset + AddressMap::BCU_NIOS_FIXED_AREA, record_len);

   // Master diagnostic config.

   size_t pos=0;
   bool found=false;

   memset( &_masterIP, 0, sizeof(uint8)*4);
   memset( &_masterMAC, 0, sizeof(uint8)*6);

   if (_masterD_enable) {      
      string bcuIp = Utils::resolveHost(wfs->getIpAddr());
      string myIp, myMac, netmask;

      for (i=0; i<7; i++) {
          ostringstream oss;
          oss << "eth" << i;
          stat = Utils::getMacAddr(oss.str(), myMac, myIp, netmask);
          if (!IS_ERROR(stat)) {
              if (Utils::matchIP( bcuIp, myIp, netmask)) {

                  pos=0;          
                  for (i=0; i<4; i++) {
                      _masterIP[i] = atoi( myIp.substr(pos).c_str());
                      pos = myIp.find(".", pos)+1;
                  }

                  pos=0;
                  for (i=0; i<6; i++) {
                      _masterMAC[i] = strtol( myMac.substr(pos, 2).c_str(), NULL, 16);
                      pos = myMac.find(":", pos)+1;
                  }
              Logger::get()->log(Logger::LOG_LEV_INFO, "Configuring master diagnostic: IP %s, MAC %s, port %d, decimation %d", myIp.c_str(), myMac.c_str(), _masterD_port, _masterD_decimation);
              printf("Configuring master diagnostic: IP %s, MAC %s, port %d, decimation %d\n", myIp.c_str(), myMac.c_str(), _masterD_port, _masterD_decimation);
              found = true;
              break;
              }
          }
      }
      if (!found)
          Logger::get()->log(Logger::LOG_LEV_ERROR, "No interface found for BCU IP %s. Masterdiagnostic will be disabled", bcuIp.c_str());
   }
   else
          Logger::get()->log(Logger::LOG_LEV_INFO, "Masterdiagnostic disabled in configuration file");

   wfs->configureMasterDiagnostics( _masterD_enable, _masterIP, _masterMAC, _masterD_port, _masterD_decimation); 


   // TODO
	wfs->setPixelGains(1, NO_SEND);	// pixel gains at 1, don't send
	wfs->setPixelOffsets(0, NO_SEND);	// pixel offsets at 0, don't send

	//wfs->setTimeHistoryOffsets(1,0);
   //wfs->setTimeHistoryOffsets("../config/slopes/timehistory48.fits",0);

    wfs->setTimeHistoryOffsets(0,0);

	wfs->setSlopeOffsets(0, FORCE_SEND);	// slope offsets at 0, send
   var_dark_cur.Set("");               // No current dark

#ifdef USE_COMMANDHISTORY
    wfs->setCommandHistoryArea("../config/commands/disturbance.fits");
#endif

	uint32 intuno, intzero;
	float floatuno = 1;
	float floatzero = 0;
	intuno = *((uint32 *)&floatuno);
	intzero = *((uint32 *)&floatzero);

	// Configure the ParamConturPixelArea
	float32 *buffer = new float32[3200*2];
	for (i=0; i<3200; i++)
		{
		buffer[i*2] = 0.0;
		buffer[i*2+1] = 0.0;
		}

   wfs->SendBuffer( _bcuNum, BCU_DSP, BCU_DSP, (unsigned char *)buffer, sizeof(float32)*3200*2, DEFAULT_PARAMCONTURPIXELAREAPTR, MGP_OP_WRSEQ_DSP, 0, 0);


	// Configure the AverageFluxArea
	wfs->WriteBCU_DSPword( DEFAULT_PARAMAVERAGEFLUXAREAPTR, intzero);	// guadagno on contur pixels
	wfs->WriteBCU_DSPword( DEFAULT_PARAMAVERAGEFLUXAREAPTR+1, intzero);  // not used
	wfs->WriteBCU_DSPword( DEFAULT_PARAMAVERAGEFLUXAREAPTR+2, intuno);	// guadagno sull'inverso della sommatoria dei pixel
	wfs->WriteBCU_DSPword( DEFAULT_PARAMAVERAGEFLUXAREAPTR+3, intzero);	// INTEGRATORE !!!!

	// Configure the TimeHistory vector

	wfs->setTimeHistoryVector( 1.0,1 );
	wfs->TimeHistoryStart();


   std::string _origin;

   try {
      _origin = (std::string) ConfigDictionary()["pixelOrigin"];
   } catch (Config_File_Exception &e) {
      _origin = "ccd";
   }


   if (_origin == "ccd")
      stat = wfs->selectPixelOrigin( ORIGIN_CCD);
   else if (_origin == "random")
      stat = wfs->selectPixelOrigin( ORIGIN_RANDOM);
   else if (_origin == "sdram")
      stat = wfs->selectPixelOrigin( ORIGIN_RAM);
   else
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "Error setting pixel origin: unknown setting %s", _origin.c_str());
      return INVALID_ARGUMENT_ERROR;
   }

   if (stat !=NO_ERROR)
      {
      stat = PLAIN_ERROR(stat);
      Logger::get()->log(Logger::LOG_LEV_ERROR, "Error setting pixel origin: (%d) %s", stat, lao_strerror(stat));
      }

    // Configure "NotUsed" pixels to a harmless value

    wfs->WriteBCU_DSPword( SLPCOMP__BSCUB_NOTUSEDPIXELAREAPTR,  DEFAULT_NOTUSEDPIXELAREAPTR);

    // Configure clipping and default MasterD decimation
    wfs->setSlopeClip( float(DEFAULT_SLOPECLIP));

    // Show status in RTDB
    var_clip_cur.Set( float(DEFAULT_SLOPECLIP));
    var_masterd_decimation_cur.Set(_masterD_decimation);

	wfs->startDSPprogram();
   Logger::get()->log(Logger::LOG_LEV_DEBUG, "Configuration done");
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

int SlopeCompCtrl::safeStop( int *previous_state)
{
	int status = getCurState();

        // Stop CCD integration
	stopCCD39();

	// Always force stop, for now
	wfs->stopDSP();

	if (previous_state)
	 *previous_state = status;

	if (status == STATE_OPERATING)
		setCurState(STATE_READY);

	// Wait until the longest possible frame has been processed. Min speed = 100 hz
	usleep( 100*1000);

	return NO_ERROR;
}	

int SlopeCompCtrl::stopCCD39()
{
  int timeout = 20; // 200 ms timeout
	int ccd39 = 1;

        _ccd39Enable.Set(0);
	do {
	  msleep(10);
	  timeout--;
	  _ccd39EnableCur.Update();
	  _ccd39EnableCur.Get(&ccd39);
	} while ((timeout>0) && (ccd39 == 1));
	if (timeout == 0)
	  Arcetri::Logger::get()->log(Arcetri::Logger::LOG_LEV_WARNING, "Stop CCD integration timeout!");

	return NO_ERROR;
}


int SlopeCompCtrl::startCCD39()
{
	int timeout = 20; // 200 ms timeout
	int ccd39 = 0;

	_ccd39Enable.Set(1);
	do {
	  msleep(10);
	  timeout--;
	  _ccd39EnableCur.Update();
	  _ccd39EnableCur.Get(&ccd39);
	} while ((timeout>0) && (ccd39 == 0));
	if (timeout == 0)
	  Arcetri::Logger::get()->log(Arcetri::Logger::LOG_LEV_WARNING, "Restart CCD integration timeout!");

	return NO_ERROR;
}

void SlopeCompCtrl::lock() {
    pthread_mutex_lock(&dsp_mutex);
}

void SlopeCompCtrl::unlock() {
    pthread_mutex_unlock(&dsp_mutex);
}

int SlopeCompCtrl::safeStart( int previous_state)
{

	int status = getCurState();

	if ((status == STATE_READY) && (previous_state == STATE_OPERATING)) {
        	// Restart CCD integration
	        startCCD39();

		wfs->startDSP();
		setCurState(STATE_OPERATING);
	}

	return NO_ERROR;
}

//@Function: checkFiber
//
// Check fiber timeouts and CRC errors
//
// Returns NO_ERROR if everything ok, otherwise an error code.
// If a fiber timeout or CRC error is detected, an alert is sent to the arbitrator and an error is logger
//@

int SlopeCompCtrl::checkFiber() {

    int stat;
    static bool first=true;
    static unsigned int prev_fl_timeouts=0;
    static unsigned int prev_crc_errors=0;
    static struct timeval before;
    unsigned int fl_timeouts, crc_errors;
    Error *myerror=NULL;

    return NO_ERROR;
    int status = getCurState();
    if (status == STATE_NOCONNECTION)
        return NO_ERROR;

    struct timeval now;
    gettimeofday(&now, NULL);

    try { 
        if (_FLtimeoutEnable) {
           stat = wfs->readFLtimeouts(&fl_timeouts);
           if (IS_ERROR(stat)) {
               Logger::get()->log(Logger::LOG_LEV_ERROR, "Cannot check fastlink timeouts: (%d) %s [%s:%d]", stat, lao_strerror(stat), __FILE__, __LINE__);
               return stat;
            }
       }

        if (_FLCRCEnable) {
            stat = wfs->readCRCerrors(&crc_errors);
            if (IS_ERROR(stat)) {
                Logger::get()->log(Logger::LOG_LEV_ERROR, "Cannot check CRC errors: (%d) %s [%s:%d]", stat, lao_strerror(stat), __FILE__, __LINE__);
                return stat;
            }
        }
    } catch (AOException &e) {
            Logger::get()->log( Logger::LOG_LEV_ERROR, "Error reading CRC and Timeout counters: %s [%s:%d]", e.what().c_str(), __FILE__, __LINE__);
            return NO_ERROR;
   }

    if (!first) {
        int num_fl_timeouts = fl_timeouts - prev_fl_timeouts; 
        int num_crc_errors = crc_errors - prev_crc_errors; 
 
        double t = Utils::timediff( &now, &before);

        if (_FLCRCEnable) {
            if (num_crc_errors > _FLCRCMax) {
                Logger::get()->log(Logger::LOG_LEV_ERROR, "CRC errors detected on fastlink fiber: %d errors in %5.1f seconds", num_crc_errors, t);
                myerror = new Error("CRC errors detected on fastlink fiber", false);
            }
            else if (num_crc_errors > 0) {
                Logger::get()->log(Logger::LOG_LEV_WARNING, "CRC errors detected on fastlink fiber: %d errors in %5.1f seconds", num_crc_errors, t);
                Logger::get()->log(Logger::LOG_LEV_WARNING, "No alert sent because limit is %d errors", _FLCRCMax);
            }
        }

        if (_FLtimeoutEnable) {
            if (num_fl_timeouts > _FLtimeoutMax) {
                Logger::get()->log(Logger::LOG_LEV_ERROR, "Timeouts detected on fastlink fiber: %d timeouts in %5.1f seconds", num_fl_timeouts, t);
                myerror = new Error("Timeout errors detected on fastlink fiber", false);
            }
            else if (num_fl_timeouts > 0) {
                Logger::get()->log(Logger::LOG_LEV_WARNING, "Timeouts detected on fastlink fiber: %d timeouts in %5.1f seconds", num_fl_timeouts, t);
                Logger::get()->log(Logger::LOG_LEV_WARNING, "No alert sent because limit is %d timeouts", _FLtimeoutMax);
            }
        }

        if (myerror) {
            // Notify arbitrator
            try {
                _arbNotif->notifyAlert(myerror);
                _logger->log(Logger::LOG_LEV_INFO, "Alert to WFS Arbitrator succesfully sent! [%s:%d]", __FILE__,__LINE__);
            } catch(ArbitratorInterfaceException& e) {
                _logger->log(Logger::LOG_LEV_ERROR, "Alert to WFS Arbitrator not sent! [%s:%d]", __FILE__,__LINE__);
            }
        delete myerror;
        }
    }
    else {
        first = false;
    }

    prev_fl_timeouts = fl_timeouts;
    prev_crc_errors = crc_errors;
    before = now;
    return NO_ERROR;

}

int main( int argc, char **argv) {

   SetVersion(VersMajor,VersMinor);

   try {
      SlopeCompCtrl *c;

      c = new SlopeCompCtrl( argc, argv);

      c->Exec();

      delete c;
   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}

