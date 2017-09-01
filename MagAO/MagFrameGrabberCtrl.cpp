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
#include "../BCUCtrl/BCUfile.h"
#include "../BCUCtrl/BCUlbt.h"
#include "MagFrameGrabberCtrl.h"
#include "stdconfig.h"
#include "BcuLib/bcucommand.h"
#include "Paths.h"

extern "C" {
#define NIOS_WFS
#include "../Housekeeper/HouseKeeperRawStruct.h"
}
using namespace Arcetri::HouseKeeperRaw;
  
// Global variables

int seqnum=0;

int CHAR_VAR_LEN = 128;                 // Length of a "filename variable
int ERRMSG_LEN = 128;                   // Length of the error message variable

#include "AOStates.h"

namespace VisAO
{
   
FrameGrabberCtrl::FrameGrabberCtrl(std::string name, const std::string &conffile) throw (AOException): AOApp(name, conffile)
{
   Create();
}

FrameGrabberCtrl::FrameGrabberCtrl( int argc, char **argv) throw (AOException): AOApp(argc, argv)
{
   Create();
}

void FrameGrabberCtrl::Create() throw (AOException)
{
   x_bin_changed = 0;
   y_bin_changed = 0;
   x_win_changed = 0;
   y_win_changed = 0;
}


void FrameGrabberCtrl::Run()
{

   setCurState(STATE_READY);
   
   while(!TimeToDie())
   {
      msleep(1000);
   }
}

int FrameGrabberCtrl::set_xbin(int nx)
{
   if(nx != cur_x_bin && nx > 0)
   {
      cur_x_bin = nx;
      x_bin_changed = 1;
   }
   return set_binning();
}

int FrameGrabberCtrl::set_ybin(int ny)
{
   if(ny != cur_y_bin && ny > 0)
   {
      cur_y_bin = ny;
      y_bin_changed = 1;
   }
   
   return set_binning();
}

int FrameGrabberCtrl::set_xwin(int nx)
{
   if(nx != cur_x_win)
   {
      cur_x_win = nx;
      x_win_changed = 1;
   }
   return set_binning();
}

int FrameGrabberCtrl::set_ywin(int ny)
{
   if(ny != cur_y_win)
   {
      cur_y_win = ny;
      y_win_changed = 1;
   }
   
   return set_binning();
}

int FrameGrabberCtrl::set_binning()
{
   if(((x_bin_changed && y_bin_changed) || (x_win_changed && y_win_changed)) && ((cur_x_bin == cur_y_bin) && (cur_x_win == cur_y_win)))
   {
      int fake_binning;

      if(cur_x_win == 0 || cur_x_bin == 0)
      {
         fake_binning = 1;
      }
      else fake_binning = 1024/cur_x_win*cur_x_bin;

      
      //Here's where we delete the old LUT directory
      //And the link the new LUT directory
      
      std::string curLUT = get_curLUT();
      std::string newLUT = get_LUT(fake_binning);
      
      std::string cmd = "rm -rf ";
      cmd += curLUT;
      system(cmd.c_str());

      cmd = "ln -s ";
      cmd += newLUT;
      cmd += " ";
      cmd += curLUT;
      system(cmd.c_str());

      //Notification of new frame size for Master Diagnostic
      int n_pixels = (1024/fake_binning) * (1024/fake_binning);

      
      _masterD_npix.Set(n_pixels, 0, FORCE_SEND);
      
      x_bin_changed = 0;
      y_bin_changed = 0;
      x_win_changed = 0;
      y_win_changed = 0;
   }
   return 0;
}

std::string FrameGrabberCtrl::get_curLUT()
{
   std::string path = getenv("ADOPT_ROOT");
   path += "/calib/wfs/current/ccd47/LUTs/current";

   return path;
}

std::string FrameGrabberCtrl::get_LUT(int bin)
{
   char bindir[10];
   std::string path = getenv("ADOPT_ROOT");
   path += "/calib/wfs/current/ccd47/LUTs/";

   snprintf(bindir, 10, "bin%i", bin);

   path += bindir;
   
   return path;
}

   
int FrameGrabberCtrl::ccd47_x_binChanged( void *pt, Variable *var)
{
   _logger->log(Logger::LOG_LEV_INFO, "Ccd47 current X binning to %d", var->Value.Lv[0]);
   FrameGrabberCtrl *FGC = (FrameGrabberCtrl *)pt;
   FGC->set_xbin(var->Value.Lv[0]);

   return NO_ERROR;
}

int FrameGrabberCtrl::ccd47_y_binChanged( void *pt, Variable *var)
{
   _logger->log(Logger::LOG_LEV_INFO, "Ccd47 current Y binning to %d", var->Value.Lv[0]);
   FrameGrabberCtrl *FGC = (FrameGrabberCtrl *)pt;
   FGC->set_ybin(var->Value.Lv[0]);

   return NO_ERROR;
}
    
int FrameGrabberCtrl::ccd47_x_winChanged( void *pt, Variable *var)
{
   _logger->log(Logger::LOG_LEV_INFO, "Ccd47 current x window to %d", var->Value.Lv[0]);
   FrameGrabberCtrl *FGC = (FrameGrabberCtrl *)pt;
   FGC->set_xwin(var->Value.Lv[0]);
   

   return NO_ERROR;
}

int FrameGrabberCtrl::ccd47_y_winChanged( void *pt, Variable *var)
{
   _logger->log(Logger::LOG_LEV_INFO, "Ccd47 current x window to %d", var->Value.Lv[0]);
   FrameGrabberCtrl *FGC = (FrameGrabberCtrl *)pt;
   FGC->set_ywin(var->Value.Lv[0]);
   
   return NO_ERROR;
}
    



void FrameGrabberCtrl::SetupVars()
{
   std::cout << "Setting up variables\n";
   
   try
   {

      string masterDProcess = (string)ConfigDictionary()["MasterDiagnostic_name"]+"."+Side();
      
      _masterD_npix = RTDBvar(masterDProcess + ".TV_NPIXELS");
      
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
      
      _ccd47_x_bin = RTDBvar(ccd47Process + ".XBIN.CUR");  // Also get the value
      _ccd47_x_bin.Get(&cur_x_bin);
      _ccd47_y_bin = RTDBvar(ccd47Process + ".YBIN.CUR");
      _ccd47_y_bin.Get(&cur_y_bin);
      
      _ccd47_x_win = RTDBvar(ccd47Process + ".DX");
      _ccd47_x_win.Get(&cur_x_win);
      _ccd47_y_win = RTDBvar(ccd47Process + ".DY");
      _ccd47_y_win.Get(&cur_y_win);

      x_bin_changed = 1;
      y_bin_changed = 1;
      x_win_changed = 1;
      y_win_changed = 1;

      set_binning();
      
      Notify(_ccd47_x_bin, ccd47_x_binChanged);
      Notify(_ccd47_y_bin, ccd47_y_binChanged);
      
      Notify(_ccd47_x_win, ccd47_x_winChanged);
      Notify(_ccd47_y_win, ccd47_y_winChanged);

      dark_cur = RTDBvar(MyFullName(), "DARK", CUR_VAR, CHAR_VARIABLE, 1, 1);
      dark_req = RTDBvar(MyFullName(), "DARK", REQ_VAR, CHAR_VARIABLE, 1 , 1);
      
      
   } catch (AOVarException &e) {
      _logger->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }
   
}

} //namespace VisAO



