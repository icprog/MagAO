#ifndef __gainSetter_h__
#define __gainSetter_h__


#include "AOApp.h"
#include "AOStates.h"

#include "Paths.h"

extern "C"
{
#include "iolib.h" //ReadBinaryFile
}



#include "arblib/base/ArbitratorInterface.h"



class gainSetter : public AOApp
{
public:
   gainSetter( int argc, char **argv) throw(AOException);
   ~gainSetter();


protected:

   /** @name RTDB Variables
    */
   //@{

   RTDBvar var_gain_tt_cur;
   RTDBvar var_gain_tt_req;
   RTDBvar var_gain_ho1_cur;
   RTDBvar var_gain_ho1_req;
   RTDBvar var_gain_ho2_cur;
   RTDBvar var_gain_ho2_req;


   RTDBvar var_gain_tt0_cur;
   RTDBvar var_gain_tt0_req;

   RTDBvar var_gain_tt1_cur;
   RTDBvar var_gain_tt1_req;


   RTDBvar var_nmodes;
   RTDBvar var_homiddle;
   
   RTDBvar var_gain_file;
   RTDBvar var_recmat_file;


   //@}

   std::string modalBasis;
   std::string recMat;
   int nModes;
   int hoMiddle;

   ///The interface to the AdSec Arbitrator
   ArbitratorInterface* _arbIntf;
   std::string _arbName;  
   std::string _arbMsgdId; 

   /** @name AOAPP Business
     */
   //@{

   void Create(void) throw (AOException);

   ///Load the configuration details from the file
   int LoadConfig();

   ///Setup variables in RTDB (overridden virtual)
   void SetupVars();

   void  PostInit();

   // VIRTUAL - Run
   void Run();
      
   int DoFSM();
   
   //@}


   /** @name RTDB Handlers
     */
   //@{

   /// RTDB handler for a TT gain request command via MsgD.
   static int gain_tt_req_changed(void *pt, Variable *msgb);

   /// RTDB handler for a HO1 gain request command via MsgD.
   static int gain_ho1_req_changed(void *pt, Variable *msgb);

   /// RTDB handler for a HO2 gain request command via MsgD.
   static int gain_ho2_req_changed(void *pt, Variable *msgb);

   /// RTDB handler for a change in the gain vector via MsgD.
   static int gain_file_changed(void *pt, Variable *msgb);

   /// RTDB handler for a change in the reconstructor via MsgD.
   static int recmat_file_changed(void *pt, Variable *msgb);


   /// RTDB handler for a TT gain request command via MsgD.
   static int gain_tt0_req_changed(void *pt, Variable *msgb);

   /// RTDB handler for a TT gain request command via MsgD.
   static int gain_tt1_req_changed(void *pt, Variable *msgb);


   //@}

   int setGains(double tt, double ho1, double ho2);
   int setGains(double tt0, double tt1, double ho1, double ho2);

   void sendCommandWithReply(Command* cmd, Command **reply=NULL);
   void sendCommand(Command* cmd);
   void sendCommandThread(Command* cmd);

   int parseGains(std::string & file, bool updateCur );
   int parseModalBasis(std::string & file);

   int updateRecmat(std::string & file);

   std::string writeGains( double tt, double ho1, double ho2);
   std::string writeGains( double tt0, double tt1, double ho1, double ho2);

};












#endif //__gainSetter_h))
