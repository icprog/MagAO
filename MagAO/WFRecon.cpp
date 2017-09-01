#include "WFRecon.h"

using namespace Arcetri;

//#define _debug


double vectorMedian(std::vector<double> & vec)
{
   double med;
   int N = vec.size();

   std::sort(vec.begin(), vec.end());

   if(N % 2 == 0)
   {
      med = 0.5* (vec[(int) (0.5*N-1)] + vec[(int) (0.5*N)]);
   }
   else
   {
      med = vec[(int)0.5*N];
   }

   return med;
}


WFRecon::WFRecon( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
}

WFRecon::~WFRecon()
{

}



void WFRecon::Create() throw (AOException)
{   
   valid_recmat = 0;

   wfehist_len = 100;
   wfehist_idx = 0;

   wfehist.resize(wfehist_len);


   _tt = 0.0;
   _ho1 = 0.0;
   _ho2 = 0.0;

   pthread_mutex_init(&reconMutex, NULL);

   LoadConfig();

   ampsCBuff = 0;
   nAmps = 30000;
   curAmp = 0;
   
}//void WFRecon::Create()

int WFRecon::LoadConfig()
{
   //Config_File *cfg = &ConfigDictionary();
   try 
   {

   }
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "Missing config data: %s", e.what().c_str());
      throw(e);
   }

#ifdef REC_USE_GPU
   try
   {
      rmat.rec_tech = (int)(ConfigDictionary())["rec_tech"];
   }
   catch(Config_File_Exception)
   {
      rmat.rec_tech = REC_ATLAS;
   }
   _logger->log(Logger::LOG_LEV_INFO, "Set the reconstruction technique to (rec_tech): %i", rmat.rec_tech);
#endif

   try
   {
      rmat.tel_diam = (double)(ConfigDictionary())["tel_diam"];
      rmat.median_r0 = (double)(ConfigDictionary())["median_r0"];     
      rmat.median_r0_lam = (double)(ConfigDictionary())["median_r0_lam"];  
      rmat.fitting_A = (double)(ConfigDictionary())["fitting_A"];      
      rmat.fitting_B = (double)(ConfigDictionary())["fitting_B"];
      rmat.reflection_gain = (double)(ConfigDictionary())["reflection_gain"];
   }
   catch(Config_File_Exception)
   {
      std::cout << "loading defaults\n";
      rmat.tel_diam = 6.5;
      rmat.median_r0 = 18.;     
      rmat.median_r0_lam = .55;  
      rmat.fitting_A = 0.232555;      
      rmat.fitting_B = -0.840466;
      rmat.reflection_gain = 2.;
   }
   return 0;
}//int WFRecon::LoadConfig()

void WFRecon::SetupVars()
{
   //First Figure out which side we're on
   subsys = getenv("ADOPT_SUBSYSTEM");
   std::string wfsadd, adsadd;
   
   if(subsys == "WFS")
   {
      adsadd = "@M_ADSEC";
      Logger::get()->log(Logger::LOG_LEV_INFO, "Attaching to ADOPT_SUBSYSTEM: %s", subsys.c_str());
   }
   else if(subsys == "ADSEC")
   {
      wfsadd = "@M_MAGWFS";
      Logger::get()->log(Logger::LOG_LEV_INFO, "Attaching to ADOPT_SUBSYSTEM: %s", subsys.c_str());
   }
   else
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "Unknown or unset ADOPT_SUBSYSTEM environment variable. %s:%i", __FILE__, __LINE__);
      throw AOException("Unknown or unset ADOPT_SUBSYSTEM environment variable.");
   }



   //Setup the RTDB variables
   try
   {

      var_reconstructor_path = RTDBvar("ADSEC.L", "B0_A"+adsadd, NO_DIR, CHAR_VARIABLE, 256,0);
      updateRecMat();
   
      Notify(var_reconstructor_path, reconstructor_changed);

      var_gain_tt_cur = RTDBvar( "gainset.L", "gain_tt.CUR"+adsadd, NO_DIR, REAL_VARIABLE, 1, 0);
      var_gain_tt_req = RTDBvar( "gainset.L", "gain_tt.REQ"+adsadd, NO_DIR, REAL_VARIABLE, 1, 0);
      var_gain_ho1_cur = RTDBvar( "gainset.L", "gain_ho1.CUR"+adsadd, NO_DIR, REAL_VARIABLE, 1, 0);
      var_gain_ho1_req = RTDBvar( "gainset.L", "gain_ho1.REQ"+adsadd, NO_DIR, REAL_VARIABLE, 1, 0);
      var_gain_ho2_cur = RTDBvar( "gainset.L", "gain_ho2.CUR"+adsadd, NO_DIR, REAL_VARIABLE, 1, 0);
      var_gain_ho2_req = RTDBvar( "gainset.L", "gain_ho2.REQ"+adsadd, NO_DIR, REAL_VARIABLE, 1, 0);

      updateGains();

      Notify(var_gain_tt_cur, gain_changed);
      Notify(var_gain_ho1_cur, gain_changed);
      Notify(var_gain_ho2_cur, gain_changed);
      
   }
   catch (AOVarException &e)  
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%i: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }
}//void WFRecon::SetupVars()

void WFRecon::PostInit()
{
   
  
   

}//void WFRecon::PostInit()

void WFRecon::Run()
{


   start_telemetry();
   start_psd();



   while(!TimeToDie()) 
   {
      try 
      {
         DoFSM();
      } 
      catch (AOException &e) 
      {
         _logger->log( Logger::LOG_LEV_ERROR, "Caught exception (at %s:%d): %s", __FILE__, __LINE__, e.what().c_str());
      }
   }
}//void gainHunger::Run()


int WFRecon::DoFSM()
{
   int status;

   status = STATE_OPERATING;
   
   setCurState(status);


   usleep( (unsigned int)(1.0 * 1e6));

   return NO_ERROR;

}// int WFRecon::DoFSM()

int WFRecon::reconstructor_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   WFRecon * wfr = (WFRecon*) pt;

   return wfr->updateRecMat();

}

int WFRecon::updateRecMat()
{
   

   std::string path, newpath;

   var_reconstructor_path.Update();
   path = var_reconstructor_path.Get();

   int p = path.find("/M2C");

   if(p < 0)
   {
      valid_recmat = 0;
      return 0;
   }


   if(subsys == "WFS")
   {      
      newpath = "/towerdata";
      newpath += path.substr(p, path.length()-p);
   }
   else
   {
      newpath = path;
   }

   if(newpath == reconstructor_path) return 0;

   reconstructor_path = newpath;

//   std::cout << reconstructor_path << "\n";

   pthread_mutex_lock(&reconMutex);

   if(rmat.load_recmat_LBT(reconstructor_path) == 0)
   {
      
      
      logss.str("");
      logss << "set recmat: " << reconstructor_path;
      _logger->log(Logger::LOG_LEV_INFO, logss.str().c_str());
      
      n_modes = rmat.n_modes;
      ho_middle = 200;
      if(n_modes <= 200) ho_middle = 150;
      if(n_modes <= 100) ho_middle = 66;
      if(n_modes <= 28) ho_middle = 6;

      
      std::cout << "n_modes = " <<  n_modes << "\n";
      std::cout << "ho_middle = " << ho_middle << "\n";
//       pthread_mutex_unlock(&reconMutex);

      
      

      if(ampsCBuff)
      {
         delete ampsCBuff;
      }

      ampsCBuff = new float[nAmps*n_modes];

      curAmp = 0;

      valid_recmat = 1;

      pthread_mutex_unlock(&reconMutex);

      return 0;      
   }
   else
   {   
      std::cout << "setting invalid recmat\n";
      valid_recmat = 0;

      logss.str("");
      logss << "failed to set recmat: " << reconstructor_path;
      _logger->log(Logger::LOG_LEV_ERROR, logss.str().c_str());
      
      n_modes = 0;
//       ho_middle = 0;
      
      std::cout << n_modes << "\n";
//       std::cout << ho_middle << "\n";
//       
//       pthread_mutex_unlock(&reconMutex);

      pthread_mutex_unlock(&reconMutex);
      return -1;
   }

}//void WFRecon::setRecMat(std::string path)


int WFRecon::gain_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   WFRecon * gh = (WFRecon *) pt;

   return gh->updateGains();
}

int WFRecon::updateGains()
{
   var_gain_tt_cur.Update();
   var_gain_tt_cur.Get(&_tt);

   var_gain_ho1_cur.Update();
   var_gain_ho1_cur.Get(&_ho1);

   var_gain_ho2_cur.Update();
   var_gain_ho2_cur.Get(&_ho2);

   //std::cout << "Gains: " << _tt << " " << _ho1 << " " << _ho2 << "\n";
   return 0;
}




int WFRecon::start_telemetry()
{
   struct sched_param schedpar;
   pthread_attr_t attr;
   
   pthread_attr_init(&attr);
   
   //Start the signal catcher as a lower priority thread.
      
   schedpar.sched_priority = 0;

   pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
   pthread_attr_setschedparam(&attr, &schedpar);      
   
   //This is weird, but the doc for pthread_create says it returns an "error number" instead of -1
   if(pthread_create(&telemetry_th, &attr, &__start_telemetry, (void *) this) == 0) return 0;
   else return -1;

}

int WFRecon::start_psd()
{
   struct sched_param schedpar;
   pthread_attr_t attr;
   
   pthread_attr_init(&attr);
   
   //Start the signal catcher as a lower priority thread.
      
   schedpar.sched_priority = 0;

   pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
   pthread_attr_setschedparam(&attr, &schedpar);      
   
   //This is weird, but the doc for pthread_create says it returns an "error number" instead of -1
   if(pthread_create(&psd_th, &attr, &__start_psd, (void *) this) == 0) return 0;
   else return -1;

}

void WFRecon::telemetry_processor()
{
   char MySelf[64];
   char bufnam[32];
   BufInfo info;
   int stat;

   snprintf(MySelf,64,"%s",MyFullName().c_str());    // Set up client name
   snprintf(bufnam, 32, "%s", "masterdiagnostic.L:OPTLOOPBUF");

   if((stat=bufRequest(MySelf,bufnam,BUFCONT,&info))!=NO_ERROR) 
   {
      printf("Error attaching to shared buffer: %s\n",bufnam);
      printf("Error code: %d (errno:%s)\n\n",CLEAN_ERROR(stat),strerror(errno));
   }

   char *pp;
   if((info.com.accMode&ACCMODE_MODE_MASK)==BUFCONT)
        pp="continous";
   else
        pp="snapshot";
      printf("Connected to %s mode shared buf: %s:%s Slot:%d Size:%d MagicNum:%d\n",
              pp,info.com.producer,info.com.name,info.com.slot,info.com.lng,info.com.uniq);
    

   #define BUF_LEN 1575

  
   int count=0;
   int no_sync_cnt=0;
  
   int i;
   int cntr_expect= -1;
   int prev_time = -1;
   int count_time = 0;
   int freq =0;
   int prev_counter=0;
   int freq_hw=0;
   
   int verbose = 0;

   int tmout = 0;
   no_sync_cnt=0;

   OptLoopDiagFrame * diagf;

   if(sizeof(OptLoopDiagFrame) != info.com.lng)
   {
      std::cout << "wrong size\n";
   }

   diagf = (OptLoopDiagFrame *)malloc(sizeof(OptLoopDiagFrame)); //info.com.lng);   // Allocate memory for buffer

   if(diagf==NULL) 
   {
      printf("Error allocating memory for data\n");
      printf("  SYSERROR:%s\n\n",strerror(errno));
      return;
   }

   //if (filename) fp = fopen(filename, "w");

   for(i=0;;count++) 
   {  // Loop forever reading data from buffer
     
      if((stat=bufRead(&info,diagf,tmout))<0) 
      {
         printf("Error reading from shared buffer\n");
         printf("Error code: %d (errno:%s)\n\n",CLEAN_ERROR(stat),strerror(errno));
         return;
      }
  
      if(cntr_expect>=0 && cntr_expect!=stat)  
      {
         no_sync_cnt++;
         if(verbose)
            printf("Received buffer #: %d  expected: %d\n",stat,cntr_expect);
      }
      cntr_expect=stat+1;

      

      if(valid_recmat)
      {
         pthread_mutex_lock(&reconMutex);
         rmat.reconstruct(diagf->slopecomp.slopes);
         pthread_mutex_unlock(&reconMutex);

         for(int i=0; i< n_modes; ++i)
         {
            ampsCBuff[i*nAmps + curAmp] = rmat.amp[i];
         } 
         ++curAmp;
         if(curAmp >= nAmps)curAmp = 0;

         rmat.calc_sumvar(&tot_wfe, 0, -1,false);
         rmat.calc_sumvar(&tt_wfe, 0, 2, false);
         rmat.calc_sumvar(&ho1_wfe, 2, ho_middle, false);
         rmat.calc_sumvar(&ho2_wfe, ho_middle, -1, false);

         int next_idx = wfehist_idx + 1;
         if(next_idx >= wfehist_len) next_idx = 0;

         wfehist[next_idx].frameno = diagf->slopecomp.framenumber;
         wfehist[next_idx].tot_wfe = sqrt(tot_wfe);
         wfehist[next_idx].tt_wfe = sqrt(tt_wfe);
         wfehist[next_idx].ho1_wfe = sqrt(ho1_wfe);
         wfehist[next_idx].ho2_wfe = sqrt(ho2_wfe);

         int last_idx = next_idx - 1;
         if( last_idx < 0) last_idx = wfehist_len - 1;

         int df = 0;//wfehist[last_idx].frameno - wfehist[next_idx].frameno;

         wfehist_idx = next_idx;

         std::cout << "\r" << freq << " " << df << " " << diagf->slopecomp.framenumber << " " << sqrt(tot_wfe) <<  " " << sqrt(tt_wfe) << " " << sqrt(ho1_wfe) << " " << sqrt(ho2_wfe);
      }


      // Compute frequency
      count_time++;
      if (time(NULL) > prev_time)
      {
         freq = count_time;
         freq_hw = ((int*)diagf)[0] - prev_counter;
         count_time =0;
         prev_time = time(NULL);
         prev_counter= ((int*)diagf)[0];  
      }
   }
}

void WFRecon::psd_processor()
{
   int N = 500;

   double *in;
   fftw_complex *out;
   fftw_plan p;
   
   in = (double*) fftw_malloc(sizeof(double) * N);
   out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * ((int) N ));

   //p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

   p = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);

   std::vector<std::string> files(585);

   std::ofstream fout;
//   files.resize(585);
   char fnames[256];

   for(int i=0; i< files.size(); ++i)
   {
      snprintf(fnames, 256, "/dev/shm/mode%d.dat", i);
      files[i] = fnames;
   }            
   
   int nextSt = 0;
   while(1)
   {

      while( !valid_recmat || curAmp - nextSt < N && (curAmp - nextSt) > 0)
      {
         msleep(5);
      }
      nextSt += N;
      if(nextSt >= nAmps) nextSt = 0;

      
      for(int i=0; i< 10; ++i)
      {
         int ja = nextSt;
         //fout.open(files[i].c_str());
         for(int j=0; j< N; ++j)
         {
             in[j] = ampsCBuff[i*nAmps + ja];
            --ja;
            if(ja < 0) ja = nAmps-1;
         }

         fftw_execute(p); /* repeat as needed */

         fout.open(files[i].c_str());
         for(int j=0; j< 0.5*N+1; ++j)
         {
            fout << (out[j][0]*out[j][0] + out[j][1]*out[j][1]) << "\n";
         }
         fout.close();
      }
      //msleep(10);
   }

   fftw_destroy_plan(p);
   fftw_free(in); 
   fftw_free(out);


}

void * __start_telemetry(void * ptr)
{
   WFRecon * gh = (WFRecon *) ptr;

   gh->telemetry_processor();

   return 0;
} 

void * __start_psd(void * ptr)
{
   WFRecon * gh = (WFRecon *) ptr;

   gh->psd_processor();

   return 0;
} 

