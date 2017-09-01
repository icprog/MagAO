//+File: FrameGrabberCtrl.h
//
// Header file for CCD47 FrameGrabber controller program
//
// This processes only job is to manage the LUT links and notifiy Master Diagnostic when binning changes.
// Don't use a script since we must handle changes from the VisAO system as well as from the arbitrator or supervisor.
//-


#ifndef __MagFrameGrabberCtrl_h__
#define __MagFrameGrabberCtrl_h__

#include <string>

#include "AOApp.h"

namespace VisAO
{
   
class FrameGrabberCtrl : public AOApp {

public:
   FrameGrabberCtrl(std::string name, const std::string &conffile) throw (AOException);

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

protected:

   RTDBvar _masterD_npix;
   
   RTDBvar _ccd47_x_bin;
   RTDBvar _ccd47_y_bin;
   int cur_x_bin;
   int x_bin_changed;
   int cur_y_bin;
   int y_bin_changed;
   
   RTDBvar _ccd47_x_win;
   RTDBvar _ccd47_y_win;


   int cur_x_win;
   int x_win_changed;
   int cur_y_win;
   int y_win_changed;
   
   static int ccd47_x_binChanged( void *pt, Variable *var);
   static int ccd47_y_binChanged( void *pt, Variable *var);
   
   static int ccd47_x_winChanged( void *pt, Variable *var);
   static int ccd47_y_winChanged( void *pt, Variable *var);


   int set_xbin(int nx);
   int set_ybin(int ny);
   int set_xwin(int nx);
   int set_ywin(int ny);

   int set_binning();

   std::string get_curLUT();
   std::string get_LUT(int bin);

   RTDBvar dark_cur, dark_req;
   
};

} //namespace VisAO


#endif //__MagFrameGrabberCtrl_h__
