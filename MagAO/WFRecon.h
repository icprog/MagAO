#ifndef __WFRecon_h__
#define __WFRecon_h__


#include "AOApp.h"
#include "AOStates.h"


#include "bcu_diag.h"

#include "recmat.h"


#include <vector>
#include <fstream>

#include <fftw3.h>

/// A Single WFE measurement
struct wfemeas
{
   unsigned frameno;
   
   float tot_wfe;
   float tt_wfe;
   float ho1_wfe;
   float ho2_wfe;
};


/// A class to reconstruct BCU 39 slope frames in near-real-time
class WFRecon : public AOApp
{
public:
   WFRecon( int argc, char **argv) throw(AOException);
   ~WFRecon();


protected:

   std::ostringstream logss; ///< Conveninence string stream for building log messages.

   ///The subsystem, WFS or ADSEC
   std::string subsys;

   
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



   /** @name Reconstruction
     */
   //@{

   recmat rmat;
   RTDBvar  var_reconstructor_path; ///<The path to the current reconstructor
   std::string reconstructor_path;

   /// Handler for a change in the reconstructor
   static int reconstructor_changed(void *pt, Variable *msgb);


   int valid_recmat; ///< true if the reconstructor is read properly
   int n_modes; ///< number of modes in the reconstructor
   int ho_middle; ///< split point between HO1 and HO2

   int updateRecMat(); ///< called when the reconstructor changes
   
   float tot_wfe; ///< The total RMS WFE
   float tt_wfe; ///< The RMS WFE of T/T
   float ho1_wfe; ///< The RMS WFE of high order group 1
   float ho2_wfe; ///< The RMS WFE of high order group 2

   int wfehist_len; ///< Length of the WFE circular buffer
   int wfehist_idx; ///< Current position in the WFE circular buffer
   std::vector<wfemeas> wfehist; ///< The WFE circular buffer
   
   pthread_mutex_t reconMutex; ///< Mutex for the reconstructor pointer

   RTDBvar var_n_modes;
   RTDBvar var_tot_wfe;

   float * ampsCBuff;
   int nAmps;
   int curAmp;

   //@}

   /** Gains
     * Support for changing gains via the gainSetter process on ADSEC sup
     */
   //@{
   RTDBvar var_gain_tt_cur;
   RTDBvar var_gain_tt_req;

   double _tt; ///< current TT gain

   RTDBvar var_gain_ho1_cur;
   RTDBvar var_gain_ho1_req;

   double _ho1; ///< current HO 1 gain

   RTDBvar var_gain_ho2_cur;
   RTDBvar var_gain_ho2_req;

   double _ho2; ///< current HO 2 gain

   /// RTDB handler for a gain change notification
   static int gain_changed(void *pt, Variable *msgb);

   /// Called when the gain CUR variables change
   int updateGains();


  
  

   //@}



   pthread_t telemetry_th; ///<Identifier for the telemetry thread

   int start_telemetry();

   pthread_t psd_th; ///<Identifier for the telemetry thread

   int start_psd();

public:
   void telemetry_processor();

   void psd_processor();

};


///Thread starter for the telemetry processor.
/** Casts the void pointer to WFRecon and calls WFRecon::telemetry_processor().
  */ 
void * __start_telemetry(void *); 

///Thread starter for the psd processor.
/** Casts the void pointer to WFRecon and calls WFRecon::psd_processor().
  */ 
void * __start_psd(void *); 

#endif //__WFRecon_h))
