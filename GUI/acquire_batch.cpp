
#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "acquire_batch.h"

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
using namespace std;

#include <dirent.h>     // DT_REG
#include <stdlib.h>
#include <sys/stat.h>   // mkdir()
#include <sys/types.h>  // mkdir()


#include "Paths.h"
#include "Utils.h"

#include "intmatlib.h"

extern "C" {
#include "iolib.h"
}


using namespace Arcetri;
using namespace Arcetri::IntmatLib;


int shortTimeout = 10*1000;
int longTimeout = 40*1000;

// If this is defined, no commands are sent to arbitrators except for status requests
#undef TEST_ONLY


acquireBatch::acquireBatch( int argc, char *argv[]) : AOApp( "acquireBatch", "R", "127.0.0.1", 0) {

   if (argc != 2) {
      printf("Usage: acquire_batch <filename>\n");
      exit(0);
   }

   _jobfile = argv[1];

}

acquireBatch::~acquireBatch() {
   delete _wfsIntf;
   delete _adsecIntf;
}

void acquireBatch::PostInit() {
   init();

   _wfsIntf = new ArbitratorInterface( "wfsarb."+AOApp::Side(), Logger::LOG_LEV_DEBUG);
   _adsecIntf = new ArbitratorInterface(  Utils::getClientname("adsecarb", true, "ADSEC"), Logger::LOG_LEV_DEBUG);

}

void acquireBatch::init() {

}


typedef map<string, vector<string> > Parameters;


void acquireBatch::Run() {

   int stat;
   string tracknum = "None";

   Command *reply;
   Command *cmd;
   Parameters par;

   ifstream ifs(_jobfile.c_str());

   char line[1024];
   int count=0;
   string key, v; 
   while( ifs.good()) {

       vector<string> tokens; // Create vector to hold our words

       tokens.clear();
       ifs.getline(line, 1023);
       stringstream ss(line); // Insert the string into a stream

       ss >> key;
       ss >> v;

       if (v == "=")
          while (ss >> v)
             tokens.push_back(v);

       if (tokens.size() <1)
          continue;

       // Skip comments
       if (tokens[0].substr(0,1).compare("#") == 0)
          continue;

       if (tokens.size() >0) 
           par[key] = tokens;
   }

   // Check the mandatory elements are present.
   string check[] = { "Bin", "Freq", "Mod", "ModalBasis", "Rec", "Gain", "Disturbance", "DistEnable", "Slopenull", "Lamp", "SaveFrames", "SaveIRTC", "MailTo" };
   vector<string> tocheck(check, check +sizeof(check)/sizeof(string));
   vector<string> missing;

   for ( vector<string>::iterator it = tocheck.begin(); it != tocheck.end(); it++) 
      if (par.find(*it) == par.end())
         missing.push_back(*it);

   if (missing.size()>0) {
      printf("Missing parameters:\n");
      for (int i=0; i<missing.size(); i++)
         printf( "%s\n", missing[i].c_str());
      exit(0);
   }

   // Load board setup
   //string setup = par["Setup"][0];
   //ostringstream oss1;
   //oss1 << getenv("ADOPT_ROOT") << "/bin/runBoardSetup.py -load \"" << Paths::WfsSetupsDir() << "/" << setup << "\"";

   //printf("Loading setup: %s\n", setup.c_str());
   //printf("%s\n", oss1.str().c_str());
   //system(oss1.str().c_str());

   int start_binning = atoi(par["Bin"][0].c_str());
   float start_freq  = atof(par["Freq"].begin()->c_str());
   float start_mod   = atof(par["Mod"].begin()->c_str());

   printf("Setting paramers: binning=%d freq=%f mod=%f\n", start_binning, start_freq, start_mod);
   fflush(stdout);

   if (IS_ERROR(stat= setLoopFreq( _wfsIntf, start_binning, start_freq, start_mod))) {
      printf("Warning: parameter setting failed: (%d) %s!\n", stat, lao_strerror(stat));
      return;
   }

   // Loop over the iterable elements

   for (vector<string>::iterator freq_iter  = par["Freq"].begin();  freq_iter  != par["Freq"].end();  freq_iter++)
   for (vector<string>::iterator mod_iter   = par["Mod"].begin();   mod_iter   != par["Mod"].end();   mod_iter++) 
   for (vector<string>::iterator rec_iter   = par["Rec"].begin();   rec_iter   != par["Rec"].end();   rec_iter++) 
   for (vector<string>::iterator gain_iter  = par["Gain"].begin();  gain_iter   != par["Gain"].end(); gain_iter++)
   for (vector<string>::iterator lamp_iter  = par["Lamp"].begin();  lamp_iter   != par["Lamp"].end(); lamp_iter++)
   for (vector<string>::iterator dist_iter  = par["Disturbance"].begin();  dist_iter   != par["Disturbance"].end(); dist_iter++)
   for (vector<string>::iterator distenable_iter  = par["DistEnable"].begin();  distenable_iter   != par["DistEnable"].end(); distenable_iter++)
   for (vector<string>::iterator slpn_iter  = par["Slopenull"].begin();  slpn_iter   != par["Slopenull"].end(); slpn_iter++) {

      // Non-iterable elements
      string *bin_iter = &(par["Bin"][0]);
      string *m2c_iter = &(par["ModalBasis"][0]);

      int binning =  atoi(bin_iter->c_str());
      float freq =  atof(freq_iter->c_str());
      float mod = atof(mod_iter->c_str());

      if ((binning != start_binning) || (freq != start_freq) || (mod != start_mod)) {

         printf("Setting paramers: binning=%d freq=%f mod=%f\n", binning, freq, mod);
         fflush(stdout);

         if (IS_ERROR(stat= setLoopFreq( _wfsIntf, binning, freq, mod))) {
            printf("Warning: parameter setting failed: (%d) %s!\n", stat, lao_strerror(stat));
            return;
         }

         start_binning = binning;
         start_freq = freq;
         start_mod = mod;
      }

      int lamp = atoi(lamp_iter->c_str());
      printf( "Setting lamp to %d\n", lamp);
      fflush(stdout);
      ostringstream oss2;
      oss2 << getenv("ADOPT_ROOT") << "/bin/thaoshell.py -e \"lamp.setIntensity(" << lamp << ", waitTimeout=120)\" > /dev/null";
      //printf("%s\n", oss2.str().c_str());
      system(oss2.str().c_str());

      printf( "Loading slopenull: %s\n", slpn_iter->c_str());
      fflush(stdout);
      string slopenull = *slpn_iter;
      if (slopenull == "zero")
         slopenull = "00zero1600.fits";
      if (slopenull.substr( slopenull.size()-5, 5) != ".fits")
         slopenull += ".fits";

      ostringstream oss3;
      oss3 << getenv("ADOPT_ROOT") << "/bin/thaoshell.py -e \"sc.set_slopenull('" << slopenull << "')\" > /dev/null";
      //printf("%s\n", oss3.str().c_str());
      system(oss3.str().c_str());

      float gain = atof( gain_iter->c_str());
      printf("Setting REC: %s - gain %f\n", rec_iter->c_str(), gain); 
      fflush(stdout);

      if (IS_ERROR(stat= setRec( _adsecIntf, *m2c_iter, *rec_iter))) {
         printf("Warning: REC setting failed: (%d) %s!\n", stat, lao_strerror(stat));
         return;
      }

      if (IS_ERROR(stat= setGain( _adsecIntf, *m2c_iter, gain))) {
         printf("Warning: Gain setting failed: (%d) %s!\n", stat, lao_strerror(stat));
         return;
      }

      printf("Setting Disturbance: %s\n", dist_iter->c_str()); 
      fflush(stdout);


      if (*distenable_iter != "OFF") {
         if (IS_ERROR(stat= setDisturb( _adsecIntf, *dist_iter))) {
            printf("Warning: Disturbance setting failed: (%d) %s!\n", stat, lao_strerror(stat));
            return;
         }
      }

      bool wfsDist=false;
      bool adsecDist=false;

      if (*distenable_iter == "WFS") {
         wfsDist = true;
      }
      if (*distenable_iter == "OVS") {
         wfsDist = adsecDist = true;
      }

      if (IS_ERROR(stat= enableDisturb( _wfsIntf, wfsDist, adsecDist))) {
         printf("Warning: Disturbance setting failed: (%d) %s!\n", stat, lao_strerror(stat));
         return;
      }

      // Close the loop
      if (IS_ERROR(stat= closeLoop( _wfsIntf))) {
         printf("Warning: Close loop failed: (%d) %s!\n", stat, lao_strerror(stat));
         return;
      }

      // Save data
      int nFrames = atoi(par["SaveFrames"][0].c_str());
      int nCcd47 = atoi(par["SaveCCD47"][0].c_str());
      int nIrtc = atoi(par["SaveIRTC"][0].c_str());
      printf("Saving data: %d loop frames - %d ccd47 frames  - %d IRTC frames\n", nFrames, nCcd47, nIrtc);
      fflush(stdout);
      tracknum = saveOptData( _wfsIntf, nFrames, nCcd47, nIrtc);
      if (tracknum == "") {
         printf("Warning: Data saving failed\n");
         // Do not return, open the loop anyway
      }

      if (IS_ERROR(stat= enableDisturb( _wfsIntf, false, false))) {
         printf("Warning: Removal of disturbance  failed: (%d) %s!\n", stat, lao_strerror(stat));
         return;
      }
      nusleep(100*1000);

      // Open loop
      if (IS_ERROR(stat= openLoop( _wfsIntf))) {
         printf("Warning: Open loop failed: (%d) %s!\n", stat, lao_strerror(stat));
         return;
      }

      printf("\nAcquisition done. Tracking number: %s\n\n", tracknum.c_str());      
      fflush(stdout);
      string tempfile = "/tmp/tempfile.txt";

      ostringstream idlcmd;
      idlcmd << "idl -e \"ao_init & ee = getaoelab('" << tracknum << "')";
      idlcmd << " & openw, unit, '" << tempfile << "', /GET_LUN";
      idlcmd << " & printf, unit, 'IsOK: ', ee->isOK()";
      idlcmd << " & printf, unit, 'Mag : ', ee->mag()";
      idlcmd << " & printf, unit, 'SR  : ', ee->sr_from_positions()";
      idlcmd << " & close, unit & free_lun, unit";
      idlcmd << "\" >& /dev/null";
      //printf("%s\n", idlcmd.str().c_str());
      system(idlcmd.str().c_str());

      ifstream in(tempfile.c_str());
      if (in)
         cout << in.rdbuf();

      printf("\n\n-------------------------------------\n\n");
      fflush(stdout);

   }


}



int main( int argc, char *argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

   try {
      acquireBatch *acq = new acquireBatch( argc, argv);
      acq->Exec();
      delete acq;
   }
   catch (LoggerFatalException &e) {
      // In this case the logger can't log!!!
      printf("%s\n", e.what().c_str());
   }
   catch (AOException &e) {
      printf("Error in acquireBatch: %s", e.what().c_str());
      Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error in acquireBatch: %s", e.what().c_str());
   }

   return 0;
}
  




