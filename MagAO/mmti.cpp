#include "mmti.h"

mmti::mmti(int argc, char **argv) throw (AOException) : MagAOIClient(argc, argv)
{
   Create();
}
      
mmti::mmti(std::string name, const std::string &conffile) throw(AOException): MagAOIClient(name, conffile)
{
   Create();
}

void mmti::Create(void) throw(AOException)
{
   pthread_mutex_init( &thmutex, NULL);

   s00 = 0;
   LoadConfig();
 
   //Initialize the tcp connection threads
   tcpth_v.resize(n_th);
   for(int i=0;i<n_th;i++)
   {
      tcpth_v[i].th = 0;
      tcpth_v[i].connected = 0;
   }

   offset_params_expected = 0;

   offset_az = 0.;
   offset_el = 0.;
   
   offset_az_total = 0.;
   offset_el_total = 0.;

   focus_param_expected = 0;
   focus_val = 0.;
}


int mmti::LoadConfig()
{
   try
   {
      port = (int) ConfigDictionary()["port"];
   }
   catch(Config_File_Exception &e)
   {
      port = 7102;
   }

   try
   {
      n_th = (int) ConfigDictionary()["no_connections"];
   }
   catch(Config_File_Exception &e)
   {
      n_th = 5;
   }

   return 0;
}

void mmti::SetupVars()
{
   MagAOIClient::SetupVars();
}

void mmti::Run()
{
   _logger->log( Logger::LOG_LEV_INFO, "Running...");

   init_DD();
   //std::cout << aoi.cat.obsinst << "\n";
   std::cout << "running\n";
   while(!TimeToDie()) 
   {
      try 
      {

         //Check if an MMT server thread is still running
         pthread_mutex_lock(&thmutex);

         for(int i=0; i<n_th; i++)
         {
            if(tcpth_v[i].th != 0)
            {
               if(pthread_tryjoin_np(tcpth_v[i].th, 0) == 0)
               {
                  //Ok, it has exited so we move it to back of the line
                  for(int j=i;j<n_th-1;j++)
                  {
                     tcpth_v[j].th = tcpth_v[j+1].th;
                     tcpth_v[j].connected = tcpth_v[j+1].connected;
                  }
                  tcpth_v[n_th-1].th = 0;
                  tcpth_v[n_th-1].connected = 0;
                  i--;
               }
            }
         }
         //Now make sure there is an unconnected thread listening.
         for(int i=0; i<n_th; i++)
         {
            if(tcpth_v[i].connected) continue;
            else if(tcpth_v[i].th == 0)
            {
               //This means there is not unconnected server thread running
               pthread_create(&tcpth_v[i].th, NULL, &__listen_on_socket, (void *) this);
               break;
            }
            else break; //Found a thread that is alive, but not connected.
         }

         pthread_mutex_unlock(&thmutex);

      } 
      catch (AOException &e) 
      {
         pthread_mutex_unlock(&thmutex);
         _logger->log( Logger::LOG_LEV_ERROR, "Caught exception (at %s:%d): %s", __FILE__, __LINE__, e.what().c_str());
                 
         sleep(1);
      }
      sleep(1);
   }
}

 
void mmti::post_update_DD_var(DD_RTDBVar &var)
{
   return; //just holding the place for now.
}

int mmti::mmt_request(std::string &_req, FILE *fp)
{
   std::ostringstream o;

   std::string req = strip_newl(_req);
   
   req = str_to_lower(req);
   
   if(req == "all")
   {
      send_all(fp);
      return 0;
   }
   
   if(req == "?" || req == "h" || req == "help")
   {
      log_msg("Received help request.  Doing nothing.");
      return 0;
   }
   
   if(req == "offset")
   {
      log_msg("Received offset request, waiting on params.");
      offset_params_expected = 1;
      return 0;
   }
   
   if(req == "focus")
   {
      log_msg("Received focus request, waiting on param.");
      focus_param_expected = 1;
      return 0;
   }
   
   if(req == "set_home")
   {
      log_msg("Received set_home request");
      sleep(1);
      mmti_fputs("OK: Processing", fp);
      offset_az_total = 0;
      offset_el_total = 0;
      sleep(1);
      mmti_fputs("Done:",fp);
      log_msg("Set home to current position");
      return 0;
   }
   
   if(req == "go_home")
   {
      log_msg("Received go_home request");
      offset(-offset_az_total, -offset_el_total, fp);
      //mmti_fputs("Done\n",fp);
      return 0;
   }

   if(offset_params_expected == 1)
   {
      o << "Received offset AZ param: ";
      offset_az = strtod(req.c_str(),0);
      o << offset_az;

      log_msg(o.str().c_str());

      offset_params_expected = 2;
      return 0;
   }
   else if(offset_params_expected == 2)
   {
      o << "Received offset EL param: ";
     
      offset_el = strtod(req.c_str(),0);
      o << offset_el;
      log_msg(o.str().c_str());

      offset_params_expected = 0;

      offset(offset_az, offset_el, fp);
      
      return 0;
   }

   if(focus_param_expected)
   {
      o << "Received offset FOCUS param: ";
      
      focus_val = strtod(req.c_str(),0);
      o << focus_val;
      log_msg(o.str().c_str());

      focus(focus_val, fp);
      focus_param_expected = 0;

      return 0;
   }
   
   std::cout << req << "| unknown request\n";
   
   return 0;
}//int mmti::mmt_request(std::string &_req, FILE *fp)

void mmti::send_all(FILE *fp)
{
   int tmpi;
   double tmp;
   std::string tmpstr;
   char string[256];
   
   log_msg("Received all request");

   //ut
   snprintf(string, 256, "ut %i", aoi.ut);
   mmti_fputs(string,fp);

   //st
   snprintf(string, 256, "st  %i", aoi.st);
   mmti_fputs(string,fp);

   //ra
   snprintf(string, 256, "ra  %0.6f", aoi.ra);
   mmti_fputs(string,fp);

   //dec
   snprintf(string, 256, "dec %0.6f", aoi.dec);
   mmti_fputs(string,fp);

   //epoch
   snprintf(string, 256, "epoch %0.6f", aoi.epoch);
   mmti_fputs(string,fp);

   //Hour Angle
   snprintf(string, 256, "ha  %0.6f", aoi.ha);
   mmti_fputs(string,fp);

   //Airmass
   snprintf(string, 256, "am  %0.6f", aoi.am);
   mmti_fputs(string,fp);


   //Azimuth
   snprintf(string, 256, "az %0.6f", aoi.az.position);
   mmti_fputs(string,fp);

   //Altitude
   snprintf(string, 256, "alt %0.6f", aoi.el.position);
   mmti_fputs(string,fp);

   //Zenith Distance
   snprintf(string, 256, "zd %0.6f", aoi.zd);
   mmti_fputs(string,fp);

   //Rotator encoder angle
   snprintf(string, 256, "rotang %0.6f", aoi.rotator.angle);
   mmti_fputs(string,fp);

   //Rotator offset
   snprintf(string, 256, "rotoffset %0.6f", aoi.rotator.offset);
   mmti_fputs(string,fp);

   //Parallactic Angle
   snprintf(string, 256, "parang %0.6f", aoi.pa);
   mmti_fputs(string,fp);

   //Sec X
   snprintf(string, 256, "secx %0.6f", aoi.hexapod.set_pos[0]);
   mmti_fputs(string,fp);

   //Sec Y
   snprintf(string, 256, "secy %0.6f", aoi.hexapod.set_pos[1]);
   mmti_fputs(string,fp);

   //Sec Z
   snprintf(string, 256, "secz %0.6f", aoi.hexapod.set_pos[2]);
   mmti_fputs(string,fp);

   //Sec H
   snprintf(string, 256, "sech %0.6f", aoi.hexapod.set_pos[3]);
   mmti_fputs(string,fp);

   //Sec V
   snprintf(string, 256, "secv %0.6f", aoi.hexapod.set_pos[4]);
   mmti_fputs(string,fp);


//    //rotpa
//    //code to get rot pa
//    tmp = -1000.;
//    snprintf(string, 256, "rotpa %0.6f", tmp);   
//    mmti_fputs(string,fp);
// 
//    //offrots
//    //code to get offrots - what is offrots 
//    tmp = -1000.;
//    snprintf(string, 256, "offrots %0.6f", tmp);
//    mmti_fputs(string,fp);

   //cat_id
   tmpstr = aoi.cat.obj;
   snprintf(string, 256, "cat_id * %s", tmpstr.c_str());
   mmti_fputs(string,fp);

   //cat_ra
   snprintf(string, 256, "cat_ra  %0.6f", aoi.cat.ra);
   mmti_fputs(string,fp);

   //cat_dec
   snprintf(string, 256, "cat_dec  %0.6f", aoi.cat.dec);
   mmti_fputs(string,fp);

   //cat_epoch
   snprintf(string, 256, "cat_epoch %0.2f", aoi.epoch);
   mmti_fputs(string,fp);

   //cat_rotoff
   snprintf(string, 256, "cat_rotoff %0.6f", aoi.cat.rotOff);
   mmti_fputs(string,fp);

   //cat_rotmode
   tmpstr = aoi.cat.rotMode;
   snprintf(string, 256, "cat_rotmode * %s", tmpstr.c_str());
   mmti_fputs(string,fp);

   tmpstr = aoi.cat.obsinst;
   snprintf(string, 256, "obsinst * %s", tmpstr.c_str());
   mmti_fputs(string,fp);

   tmpstr = aoi.cat.obsname;
   snprintf(string, 256, "obsname * %s", tmpstr.c_str());
   mmti_fputs(string,fp);

   //Wx temp
   snprintf(string, 256, "wxtemp %0.6f", aoi.environ.wxtemp);
   mmti_fputs(string,fp);

   //Wx pressure
   snprintf(string, 256, "wxpress %0.6f", aoi.environ.wxpres);
   mmti_fputs(string,fp);

   //Wx Humidity
   snprintf(string, 256, "wxhumid %0.6f", aoi.environ.wxhumid);
   mmti_fputs(string,fp);

   //Wx wind
   snprintf(string, 256, "wxwind %0.6f", aoi.environ.wxwind);
   mmti_fputs(string,fp);

   //Wx wdir
   snprintf(string, 256, "wxwdir %0.6f", aoi.environ.wxwdir);
   mmti_fputs(string,fp);

   //Wx dewpoint
   snprintf(string, 256, "wxdewpoint %0.6f", aoi.environ.wxdewpoint);
   mmti_fputs(string,fp);

   //Wx pwvest
   snprintf(string, 256, "wxpwvest %0.6f", aoi.environ.wxpwvest);
   mmti_fputs(string,fp);

   //Truss Temp
   snprintf(string, 256, "ttruss %0.6f", aoi.environ.ttruss);
   mmti_fputs(string,fp);

   //Primary cell temp
   snprintf(string, 256, "tcell %0.6f", aoi.environ.tcell);
   mmti_fputs(string,fp);

   //Ambient temp
   snprintf(string, 256, "tambient %0.6f", aoi.environ.tambient);
   mmti_fputs(string,fp);

   snprintf(string, 256, "dimm_fwhm %0.2f", aoi.environ.dimmfwhm);
   mmti_fputs(string,fp);

   snprintf(string, 256, "dimm_time %i", aoi.environ.dimmtime);
   mmti_fputs(string,fp);

   snprintf(string, 256, "mag1_fwhm %0.2f", aoi.environ.mag1fwhm);
   mmti_fputs(string,fp);

   snprintf(string, 256, "mag1_time %i", aoi.environ.mag1time);
   mmti_fputs(string,fp);

   snprintf(string, 256, "mag2_fwhm %0.2f", aoi.environ.mag2fwhm);
   mmti_fputs(string,fp);

   snprintf(string, 256, "mag2_time %i", aoi.environ.mag2time);
   mmti_fputs(string,fp);

   //Loop_Running
   //code to get Loop_Running
   if(aoi.side.wfs1.status == "LoopClosed") tmpi = 1;
   else tmpi = 0;
   snprintf(string, 256, "Loop_Running %i", tmpi);
   mmti_fputs(string,fp);

   //Loop_Gain
//    tmp = 0.4;
//    snprintf(string, 256, "Loop_Gain %0.6f", tmp);
//    mmti_fputs(string,fp);*/

   //Loop_Modes
   snprintf(string, 256, "Loop_Modes %i", aoi.side.ao.correctedmodes);
   mmti_fputs(string,fp);

   //WFSC_Frequency
   snprintf(string, 256, "WFSC_Frequency %0.3f", aoi.side.wfs1.ccdfreq);
   mmti_fputs(string,fp);

   //WFSC_Binning
   snprintf(string, 256, "WFSC_Binning %i", aoi.side.wfs1.ccdbin);
   mmti_fputs(string,fp);

   //WFSC_Counts
   snprintf(string, 256, "WFSC_Counts %i", aoi.side.wfs1.ccdbin);
   mmti_fputs(string,fp);

   //WFSC_FW1
   snprintf(string, 256, "WFSC_FW1 %0.3f", aoi.side.wfs1.filter1_pos);
   mmti_fputs(string,fp);

   //WFSC_FW1_Name
   snprintf(string, 256, "WFSC_FW1_Name * %s", aoi.side.wfs1.filter1.c_str());
   mmti_fputs(string,fp);

   //Loop_Mod_Freq1
   snprintf(string, 256, "Loop_Mod_Freq1 %0.3f", aoi.side.ao.tt_freq[0]);
   mmti_fputs(string,fp);

   //Loop_Mod_Freq2
   snprintf(string, 256, "Loop_Mod_Freq2 %0.3f", aoi.side.ao.tt_freq[1]);
   mmti_fputs(string,fp);

   //Loop_Mod_Amp1
   snprintf(string, 256, "Loop_Mod_Amp1 %0.3f", aoi.side.ao.tt_amp[0]);
   mmti_fputs(string,fp);

   //Loop_Mod_Amp2
   snprintf(string, 256, "Loop_Mod_Amp2 %0.3f", aoi.side.ao.tt_amp[1]);
   mmti_fputs(string,fp);

   //Loop_Mod_Off1
   snprintf(string, 256, "Loop_Mod_Off1 %0.3f", aoi.side.ao.tt_offset[0]);
   mmti_fputs(string,fp);

   //Loop_Mod_Off2
   snprintf(string, 256, "Loop_Mod_Off2 %0.3f", aoi.side.ao.tt_offset[1]);
   mmti_fputs(string,fp);

   //Avg WFE
   snprintf(string, 256, "AVG_WFE %0.3f", aoi.avgwfe);
   mmti_fputs(string,fp);

   //Std WFE
   snprintf(string, 256, "STD_WFE %0.3f", aoi.stdwfe);
   mmti_fputs(string,fp);

   //Inst WFE
   snprintf(string, 256, "INST_WFE %i", -1);
   mmti_fputs(string,fp);

   //Bayside x
   snprintf(string, 256, "BAYSIDE_X %0.3f", aoi.side.wfs1.baysidex);
   mmti_fputs(string,fp);

   //Bayside y
   snprintf(string, 256, "BAYSIDE_Y %0.3f", aoi.side.wfs1.baysidey);
   mmti_fputs(string,fp);

   //Bayside z
   snprintf(string, 256, "BAYSIDE_Z %0.3f", aoi.side.wfs1.baysidez);
   mmti_fputs(string,fp);

   //Loop gains
   snprintf(string, 256, "AOLOOP_GAIN_VECT * %s", aoi.side.ao.loop_gains.c_str());
   mmti_fputs(string,fp);

   //Loop gains
   snprintf(string, 256, "AORECMAT * %s", aoi.side.ao.reconstructor.c_str());
   mmti_fputs(string,fp);

   snprintf(string, 256, "EOF");
   mmti_fputs(string,fp);



}

void mmti::offset(double oaz, double oel, FILE *fp)
{
   char string[256];
   
   snprintf(string, 256, "OK: Processing");
   
   mmti_fputs(string,fp);

   //Offset code here
   //sleep(1);
   logss.str("");
   logss << "Offset: " << oaz << " " << oel;
   log_msg(logss.str());
   logss.str("");
   logss << "Total Offset: " << offset_az_total << " " << offset_el_total;
   log_msg(logss.str());

   NodRaDec(oaz, oel);

   offset_az_total += oaz;
   offset_el_total += oel;



   snprintf(string, 256, "Done:");
   
   mmti_fputs(string,fp);

   logss.str("");
   logss << "Done Sent.";
   log_msg(logss.str());

   

   return;
}

void mmti::focus(double fv, FILE *fp)
{
   char string[256];
   
   snprintf(string, 256, "OK: Processing");
   
   mmti_fputs(string,fp);
   
   //Focus code here
   //sleep(1);

   MagAOIClient::Focus(fv);

   logss.str("");
   logss <<  "Focus offset: " << fv;
   log_msg(logss.str());

   snprintf(string, 256, "Done:");
   
   mmti_fputs(string,fp);
   
   return;
}

void mmti::listen_on_socket()
{
   //int s;
   int fd;
   FILE *fp;
   char string[100];

   std::string req;

   pthread_t mynum = pthread_self();
   
   //Create the listen socket if not done yet.
   if(s00 == 0)
   {
      if ((s00 = lsocket (port)) < 0) 
      {
         fprintf (stderr, "lsocket (%i)\n", port);
         exit (-4);
      }
   }

   //Now accept the connection and get the file descriptor.
   if ((fd = accept (s00, (struct sockaddr *)0, (socklen_t *)0)) < 0) 
   {
      perror ("accept s");
      exit (-5);
   }
   
   //And open for reading.
   if ((fp = fdopen (fd, "r+")) == NULL) 
   {
      exit (-6);
   }

   pthread_mutex_lock(&thmutex);

   //First have to figure out which thread we are
   int mydx = -1;
   for(int i=0; i<n_th; i++)
   {
      if(tcpth_v[i].th == mynum)
      {
         mydx = i;
         break;
      }
   }
   //Now set status to connected.
   tcpth_v[mydx].connected = 1;
   pthread_mutex_unlock(&thmutex);

   logss.str("");
   logss << "Accepted connection on port " <<  port;
   log_msg(logss.str());

   while (!mmti_fgets(string,100,fp))
   {
      req = string;
      mmt_request(req, fp);
   }

   logss.str("");
   logss << "Connection on port " << port << " closed.";
   log_msg(logss.str());

   fclose(fp);
  
   return;
}

void mmti::log_msg(std::string msg, int lvl)
{
   _logger->log( lvl, msg);

   if(lvl == Logger::LOG_LEV_ERROR)
   {
      std::cerr << msg << "\n";
   }
   else
   {
      std::cout << msg << "\n";
   }
}

int lsocket (int port)
{
   unsigned short sport;
   int   s;
   struct  sockaddr_in sockaddr;
   int   on;
   
   /* start listening for connection
    */
   sport = htons((short)port);
   if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror ("socket");
      return (-1);
   }
   on = 1;
   if (setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on)) < 0)
   {
      perror ("setsockopt");
      return (-1);
   }
   memset ((char *)&sockaddr, 0, sizeof(sockaddr));
   sockaddr.sin_family     = AF_INET;
   sockaddr.sin_port    = sport;
   sockaddr.sin_addr.s_addr   = INADDR_ANY;
   if (bind (s, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0)
   {
      perror ("bind");
      return (-1);
   }
   if (listen (s, 10) < 0)
   {
      perror ("listen");
      return (-1);
   }
   return (s);
}//int lsocket (int port)

void * __listen_on_socket(void *vmmti)
{
   mmti * mtf;
   
   mtf = (mmti *) vmmti;
   mtf->listen_on_socket();
   return 0;
   
}//void * __listen_on_socket(void *vmmti)

std::string strip_newl(std::string &nl)
{
   std::string ret = nl;
   
   int i = ret.find_first_of("\r\n",0);
   
   if(i >= 0) ret.erase(i, ret.length()-i);
   
   return ret;
}//std::string strip_newl(std::string &nl)

std::string str_to_lower(std::string &up)
{
   std::string ret = up;
   
   for(unsigned i=0; i<ret.length(); i++) ret[i] = tolower(ret[i]);
   
   return ret;
}//std::string str_to_lower(std::string &up)

int mmti_fputs(char *string,FILE *fp)
{
   if (fseek (fp, 0L, SEEK_CUR) != 0) {
      ;
   }
   if (fputs (string, fp) < 0) {
      return (-1);
   }
   if (fputc ('\n', fp) < 0) {
      return (-2);
   }
   if (fflush (fp) != 0) {
      return (-3);
   }
   return (0);
}//int mmti_fputs(char *string,FILE *fp)

int mmti_fgets(char *string,int length,FILE *fp)
{
   if (fseek (fp, 0L, SEEK_CUR) != 0)
   {
      //Do nothing?
      ;
   }
   if (fgets (string, length, fp) == NULL)
   {
      return (-1);
   }
   if (string[strlen(string)-1] == '\n') string[strlen(string)-1] = 0;
   return (0);
}//int mmti_fgets(char *string,int length,FILE *fp)
