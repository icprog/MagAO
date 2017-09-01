
#include "intmatlib.h"

#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#include <dirent.h>     // DT_REG
#include <stdlib.h>
#include <unistd.h>     // link()
#include <sys/stat.h>   // mkdir()
#include <sys/types.h>  // mkdir()


#include "Paths.h"
#include "Utils.h"

#include "arblib/wfsArb/WfsInterfaceDefines.h"
#include "arblib/base/CommandsExport.h"
#include "arblib/wfsArb/WfsCommandsExport.h"
#include "arblib/adSecArb/AdSecCommandsExport.h"

#include "bcu_diag.h"

using namespace Arcetri;
using namespace Arcetri::Arbitrator;
using namespace Arcetri::Wfs_Arbitrator;
using namespace Arcetri::AdSec_Arbitrator;

extern "C" {
#include "iolib.h"
}


namespace Arcetri {
   namespace IntmatLib {

   static const int shortTimeout = 2*1000;
   static const int longTimeout = 40*1000;


//@Function: generatePPDisturb
//
// Generates a push-pull disturbance file
//
// modesNum: how many modes to modulate
// framesNum: how many frames each mode should occypy
// cyclesNum: how many times a given mode cycles in his alloted framesNum
// string m2c: m2c name (not full path)
// string ampFile: full path to ampFile (vector of one float per mode = amplitude relative to ampEnvelope file, 1.0 = max).
// string ampEnvelopeFile: full path to ampEnvelopeFile( vector of one float per mode = maximum amplitude allowed by mirror, in meters).
//
// Returns: (string) timestamp of generated disturb. Empty string in case of errors
//@

string generatePPDisturb( int modesNum, int framesNum, int cyclesNum, string m2c, string ampFile, string ampEnvelopeFile)
{

   string timestamp = Utils::asciiDateAndTimeCompact();

   string modalFile = Paths::DisturbDir( m2c) + "ModalDisturb_"+timestamp+".fits";
   string cmdFile = Paths::DisturbDir( m2c) + "CmdDisturb_"+timestamp+".fits";
   string m2cFile = Paths::M2CFile( m2c);

   ostringstream cmd;
   cmd << "idl -e \"intmat_disturb, " << modesNum <<"," << framesNum << "," << cyclesNum << ",'" << Paths::AdSecCalibDir(true) << "', '" << ampFile << "', '"<< modalFile << "', '" << cmdFile << "', '" << m2cFile << "', '" << ampEnvelopeFile << "'\"";
   printf("%s\n", cmd.str().c_str());
   int stat = system(cmd.str().c_str());

   if (stat == 0)
      return timestamp;
   else
      return "";
}


//@Function: calcLen
//
// Calculates the length (in frames) of a certain disturb file with N iterations.
// 
// disturbFile: full path to disturb file (FITS)
// iterations: how many times the disturb file must be repeated
//
// Returns: length of disturb, or -1 in case of errors.

int calcLen( string disturbFile, int iterations)
{
   int ndims;
   long *dims;
   int datatype;

   int stat = PeekFitsFile( (char*)disturbFile.c_str(), &datatype, &ndims, &dims);
   if (stat != NO_ERROR)
      return -1;

   return dims[0] * iterations;
}


//@Function: acquireIntmat
//
// Acquires an interaction matrix
//
// wfsIntf: interface to WFS arbitrator
// adsecIntf: interface to AdSec arbitrator
// m2c: m2c name (not full path)
// disturbTracknum: tracking number of disturbance 
// savePath: path where to save partial disturbance files, relative to adsec_data (an intmatAcq directory)
// closeloop: acquire the intmat in closed loop
// iterations: how many times the measure should be averaged
//
// logCallback: function to call to log a message
// warningCallback: function to call to log a warning
// argp: argument that will be passed back to callbacks (fist argument. Second argument is a string).
//
// savedFiles: (output, reference): string vector that will contain an array of directories (Data_) with the data saved.
//
// Returns: error code

//@

int acquireIntmat( ArbitratorInterface *wfsIntf, ArbitratorInterface *adsecIntf,
                    string m2c, string disturbTracknum, string savePath,
                    bool closeloop, int iterations,
                    vector<string> &savedFiles,
                    void (*logCallback)(void *, string), void (*warningCallback)(void *, string), void *argp)
{ 
   Command *reply;
   int stat;

   int adsec_disturbLen = 4000;
   int adsec_bufferLen = 16000;
   int adsec_iterations = adsec_bufferLen / adsec_disturbLen;

   string cmdDisturbFilename = "CmdDisturb_"+ disturbTracknum +".fits";
   string modalDisturbFile = Paths::DisturbDir(  m2c, true) + "ModalDisturb"+ disturbTracknum +".fits";


   string m2cFile = Paths::M2CFile(  m2c, true);


   // Create output directory
   string absSavePath = "/towerdata/adsec_calib/" + savePath;
   mkdir( absSavePath.c_str(), 0777);

   Command *cmd;
   
   savedFiles.clear();


   // Check Arbitrators status
   if (logCallback)
      logCallback( argp, "Checking arbitrator status");

   cmd = new RequestStatus( shortTimeout);
   sendCommandWithReply( wfsIntf, cmd, &reply);
   string arbStatus = ((RequestStatus *)reply)->getArbitratorStatus();
   delete reply;

   if ( arbStatus != "AOSet") {
      if (warningCallback)
         warningCallback( argp,"WFS is not in AOSet state");
      return NOT_INIT_ERROR;
   }

   cmd = new RequestStatus( shortTimeout);
   sendCommandWithReply( adsecIntf, cmd, &reply);
   arbStatus = ((RequestStatus *)reply)->getArbitratorStatus();
   delete reply;

   if ( arbStatus != "AOSet") {
      if (warningCallback)
         warningCallback( argp,"AdSec is not in AOSet state");
      return NOT_INIT_ERROR;
   }

   // Split measurement over multiple files. First by disturbance part, then by iterations over the same part 

   int totLen = calcLen( Paths::DisturbDir( m2c, true) + cmdDisturbFilename, iterations);
   int disturbLen = totLen / iterations;

   int nparts = disturbLen /adsec_disturbLen;
   int nrep = iterations / adsec_iterations;
   if (iterations % adsec_iterations)
      nrep ++;

   // Iterate over disturbance parts

   for (int part=0; part<nparts; part++) {

      // Iterate over repetions of the same disturbance part

      for (int rep=0; rep<nrep; rep++) {

         int n_acq = part*nrep + rep;

         ostringstream msg;
         msg << "Starting acquisition " << (n_acq+1) << " of " << nparts*nrep;
         if (logCallback)
            logCallback( argp, msg.str());

         if (logCallback)
            logCallback( argp, "Generating partial disturbance commands...");

         // Generate filenames for partial disturbance
         string cmdDisturbFile = Paths::DisturbDir(  m2c) + cmdDisturbFilename;

         ostringstream suffix;
         suffix << "_acq" << n_acq << ".fits";

         cmdDisturbFilename.replace( cmdDisturbFilename.end()-5, cmdDisturbFilename.end(), suffix.str());
         string acqCmdDisturbFile = savePath + cmdDisturbFilename;

         cmdDisturbFilename.replace( cmdDisturbFilename.begin(), cmdDisturbFilename.begin()+3, "Modal");
         string acqModalDisturbFile = savePath + cmdDisturbFilename;

         // Generate partial disturbance
         ostringstream cmd1, cmd2;
         cmd1 << "idl -e \"split_disturb, '" << Paths::AdSecCalibDir( true) << "', '" << cmdDisturbFile << "', '" << acqCmdDisturbFile << "', " << adsec_disturbLen << ", " << part << "\"";
         printf("%s\n", cmd1.str().c_str());
         stat = system( cmd1.str().c_str());
         if (stat != 0) {
            if (warningCallback)
               warningCallback( argp, "Error calling IDL split_disturb.pro");
            return ARB_COMMAND_EXEC_ERROR;
         }
         cmd2 << "idl -e \"split_disturb, '" << Paths::AdSecCalibDir( true) << "', '" << modalDisturbFile << "', '" << acqModalDisturbFile << "', " << adsec_disturbLen << ", " << part << "\"";
         printf("%s\n", cmd2.str().c_str());
         stat = system( cmd2.str().c_str());
         if (stat != 0) {
            if (warningCallback)
               warningCallback( argp, "Error calling IDL split_disturb.pro");
            return ARB_COMMAND_EXEC_ERROR;
         }

         // Load disturbance

         disturbParams dist;
         dist._disturbFile = Paths::AdSecCalibDir( true) + acqCmdDisturbFile;
         dist._on = true;
         cmd = new SetDisturb( longTimeout, dist); 

         if (logCallback)
            logCallback( argp,"Setting disturb file: "+acqCmdDisturbFile);

         if (sendCommandWithReply( adsecIntf, cmd) != NO_ERROR) {
            if (warningCallback)
              warningCallback( argp,"SetDisturb command failed");
            return ARB_COMMAND_EXEC_ERROR;
         }

         // Set reconstruction matrix
         if (closeloop) {
            recMatParams recparams;
            string modalBasis = "mirrorStandard";
            string path = Paths::M2CDir( modalBasis, true);
            string filterPath = path + "filtering/pureIntegrator/";


            recparams._m2cFile = path + "m2c.fits";
            recparams._recMatFile = path + "RECs/rec_200m_new3.fits";
            recparams._aDelayFile = filterPath + "mode_delay.fits";
            recparams._bDelayFile = filterPath + "slope_delay.fits";

            cmd = new SetRecMat( longTimeout, recparams);
            if (logCallback)
               logCallback( argp,"Setting reconstruction matrix");

            if (sendCommandWithReply( adsecIntf, cmd) != NO_ERROR) {
               if (warningCallback)
                  warningCallback( argp,"SetRecMat command failed");
               return ARB_COMMAND_EXEC_ERROR;
            }
         }

   
         // Set gain
         gainParams g;
         if (closeloop)
            g._gainFile = Paths::GainDir( m2c, true) +"gain0.01_50modi.fits";
         else
            g._gainFile = Paths::GainDir( m2c, true) +"gain0.fits";

         cmd = new SetGain( longTimeout, g);

         if (logCallback)
            logCallback( argp,"Setting gain");

         if (sendCommandWithReply( adsecIntf, cmd) != NO_ERROR) {
            if (warningCallback)
              warningCallback( argp,"SetGain command failed");
            return ARB_COMMAND_EXEC_ERROR;
         }

         // Start data saving

         int myrep= adsec_iterations;
         if ( iterations - rep*adsec_iterations < adsec_iterations)
               myrep = iterations - rep*adsec_iterations;

         int framesToSave = adsec_disturbLen * myrep + 1000;
         optsave_cmd savecmd;
         savecmd.saveSlopes = true;
         savecmd.saveFrames = true;
         savecmd.saveModes = false;
         savecmd.saveFFCommands = true;
         savecmd.saveDistAverages = true;
         savecmd.nFrames = framesToSave;
         string savedFile;
         OptLoopSave_Start(savecmd, savedFile, false);

         // Close loop
         cmd = new CloseLoop(shortTimeout);

         if (logCallback)
            logCallback( argp,"Closing loop");

         if (sendCommandWithReply( wfsIntf, cmd) != NO_ERROR) {
            if (warningCallback)
              warningCallback( argp,"CloseLoop command failed");
            return ARB_COMMAND_EXEC_ERROR;
         }

         // Wait for the right number of frames to be saved
         OptLoopSave_Sync( framesToSave);
         savedFiles.push_back(savedFile);

         // Link new data
         ostringstream ss;
         ss << savePath << "Data_acq" << n_acq;
         link( savedFile.c_str(), ss.str().c_str());

         if (logCallback)
            logCallback( argp,"Data saved as: "+savedFile);

         // Open loop
         cmd = new StopLoop(shortTimeout);

         if (logCallback)
            logCallback( argp,"Opening loop");

         if (sendCommandWithReply( wfsIntf, cmd) != NO_ERROR) {
            if (warningCallback)
              warningCallback( argp,"StopLoop command failed");
            return ARB_COMMAND_EXEC_ERROR;
         }

      }
   }


   return NO_ERROR;
}



int analyseIntmat( string intmatAcqDir, string intmatFile, string recFile, int skipFrame, int avgFrame, int nModes, int cutModes, void (*warningCallback)(void *, string), void *argp) {

   ostringstream cmd;
   int stat;

   stat = genIntmat( intmatAcqDir, intmatFile, skipFrame, avgFrame, nModes, warningCallback, argp);

   if (stat != 0) 
      return stat;

   ostringstream cmd2;
   cmd2 << "idl -e \"gen_reconstructor_gui, PATH_PREFIX='" << Paths::AdSecCalibDir( true) << "', INPUT_FILE='" << intmatFile << "', OUTPUT_FILE='" << recFile << "', FINAL_DIM=[1600,672], CUT_MODES= " << cutModes << "\"";
   printf("%s\n", cmd2.str().c_str());
   stat = system(cmd2.str().c_str());
   if (stat != 0) {
      if (warningCallback)
         warningCallback( argp, "Error calling IDL gen_reconstructor_gui.pro");
      return stat;
   }


   return NO_ERROR;
}

int genIntmat( string intmatAcqDir, string intmatFile, int skipFrame, int avgFrame, int nModes, void (*warningCallback)(void *, string), void *argp) {

   ostringstream cmd;
   int stat;

   int binning=1;/// [TODO]

   cmd << "idl -e \"gen_intmat_gui, PATH_PREFIX='" << Paths::AdSecCalibDir( true) << "', INTMAT_ACQ_DIR='" << intmatAcqDir << "', OUTPUT_FILE='" << intmatFile << "', BINNING=" << binning << ", SKIP_FRAME=" << skipFrame << ", AVG_FRAME=" << avgFrame << ", NMODES = " << nModes << "\"";

   printf("%s\n", cmd.str().c_str());
   stat = system(cmd.str().c_str());
   if (stat != 0)  {
      if (warningCallback)
         warningCallback( argp, "Error calling IDL gen_intmat_gui.pro");
      return stat;
   }

   return NO_ERROR;
}

int testAmplitude(  string intmatAcqDir, string ampFile, string outputFile, int skipFrame, int avgFrame, int nModes, void (*warningCallback)(void *, string), void *argp) {

   string intmatFile = "/tmp/intmat.fits";   // Will end up inside adsec_calib
   string dataFile = intmatFile+"_data.sav";

   int stat = genIntmat( intmatAcqDir, intmatFile, skipFrame, avgFrame, nModes, warningCallback, argp);
   if (stat != 0) {
      if (warningCallback)
         warningCallback( argp, "Error calling IDL gen_intmat_gui.pro");
      return stat;
   }

   ostringstream cmd;

   string pathPrefix = Paths::AdSecCalibDir(  true);

   cmd << "idl -e \"fix_mode_amplitude, PATH_PREFIX='" << pathPrefix << "', DATA_FILE='" << dataFile << "', INTMAT_FILE = '" << intmatFile << "', AMP_FILE='" << ampFile << "', OUTPUT_FILE='" << outputFile << "'\"";
   printf("%s\n", cmd.str().c_str());
   stat = system(cmd.str().c_str());
   if (stat != 0) {
      if (warningCallback)
         warningCallback( argp, "Error calling IDL gen_intmat_gui.pro");
      return stat;
   }

   return NO_ERROR;
}


int sendCommandWithReply( ArbitratorInterface *intf, Command* cmd, Command **reply) {

   Command* cmdReply;
   int success;

   try {
      //_logger->log(Logger::LOG_LEV_INFO, " >> Sending command %s request...", cmd->getDescription().c_str());
      cmdReply = intf->requestCommand(cmd);
      //_logger->log(Logger::LOG_LEV_INFO, " >> Command reply (%s) received!", cmdReply->getStatusAsString().c_str());
      cmdReply->log();
      success = cmdReply->isSuccess();
      if (reply)
         *reply = cmdReply;
      else
         delete cmdReply;
   }
   catch(...) {
      //_logger->log(Logger::LOG_LEV_ERROR, "Command %s error", cmd->getDescription().c_str());
      delete cmd;
      return -1;
   }

   delete cmd;

   if (!success)
         return -1;
   else
      return NO_ERROR;

}





void sinus_acquire( ArbitratorInterface *wfsIntf, ArbitratorInterface *adsecIntf, int seconds, int nframes, string disturb, string outfile,
                      void (*logCallback)(string, void *), void (*warningCallback)(string, void *), void *argp)
{


   // Check Arbitrators status
   logCallback("Checking arbitrator status", argp);

   Command *reply=NULL;
   Command *cmd = new RequestStatus( shortTimeout);
   sendCommandWithReply( wfsIntf, cmd, &reply);
   string arbStatus = ((RequestStatus *)reply)->getArbitratorStatus();
   delete reply;

   if ( arbStatus != "AOSet") {
      warningCallback("WFS is not in AOSet state", argp);
      return;
   }

   cmd = new RequestStatus( shortTimeout);
   sendCommandWithReply( adsecIntf, cmd, &reply);
   arbStatus = ((RequestStatus *)reply)->getArbitratorStatus();
   delete reply;

   if ( arbStatus != "AOSet") {
      warningCallback("AdSec is not in AOSet state", argp);
      return;
   }


         ostringstream msg;
         msg << "Starting acquisition " << outfile;
         logCallback(msg.str(), argp);

         // Load disturbance

         disturbParams dist;
         dist._disturbFile = disturb;
         dist._on = true;
         cmd = new SetDisturb( longTimeout, dist); 

         logCallback("Setting disturb file: "+ disturb, argp);

         if (sendCommandWithReply( adsecIntf, cmd) != NO_ERROR) {
            warningCallback("SetDisturb command failed", argp);
            return;
         }

         enableDisturbParams distParams;
         distParams._enableWFS = true;
         distParams._enableOVS = true;
         cmd = new EnableDisturb( longTimeout, distParams);

         logCallback("Enable disturb", argp);

         if (sendCommandWithReply( wfsIntf, cmd) != NO_ERROR) {
            warningCallback("EnableDisturb command failed", argp);
            return;
         }

         // Set gain
         //gainParams g;
         //g._gainFile = Paths::GainDir(  "mirrorStandard", true) +"gain0.fits";

         //cmd = new SetGain( longTimeout, g);

         //logCallback("Setting gain zero", argp);

         //if (sendCommandWithReply( adsecIntf, cmd) != NO_ERROR) {
         //   warningCallback("SetGain command failed", argp);
         //   return;
        // }

         // Close loop
         cmd = new CloseLoop(shortTimeout);

         logCallback("Closing loop", argp);

         if (sendCommandWithReply( wfsIntf, cmd) != NO_ERROR) {
            warningCallback("CloseLoop command failed", argp);
            return;
         }


         ostringstream s;
         s << "Acquiring for " << seconds << " seconds...";
         logCallback(s.str(), argp);

         // Wait for acquisition
         sleep(seconds);

         distParams._enableWFS = false;
         distParams._enableOVS = false;
         cmd = new EnableDisturb( longTimeout, distParams);

         logCallback("Enable disturb", argp);

         if (sendCommandWithReply( wfsIntf, cmd) != NO_ERROR) {
            warningCallback("EnableDisturb command failed", argp);
            return;
         }

         usleep(100*1000);

         // Open loop
         cmd = new StopLoop(shortTimeout);

         logCallback("Opening loop", argp);

         if (sendCommandWithReply( wfsIntf, cmd) != NO_ERROR) {
            warningCallback("StopLoop command failed", argp);
            return;
         }


         saveSlopesParams ss_params;
         ss_params._filename = outfile;
         ss_params._nFrames = nframes;

         cmd = new SaveSlopes(longTimeout, ss_params);

         logCallback("Saving data...", argp);

         if (sendCommandWithReply( adsecIntf, cmd) != NO_ERROR) {
            warningCallback("SaveSlopes command failed", argp);
            return;
         }
   

}


string acquireIntmat_old( ArbitratorInterface *wfsIntf, ArbitratorInterface *adsecIntf, float loopfreq, string m2c, string cmddisturb, int iterations,
                        void (*logCallback)(string, void *), void (*warningCallback)( string, void *), void *argp)
  
{ 
   Command *reply;

   int adsec_disturbLen = 4000;
   int adsec_bufferLen = 16000;
   int adsec_iterations = adsec_bufferLen / adsec_disturbLen;

   string m2cFile = Paths::M2CFile( m2c);
   string filename = cmddisturb;
   bool closeloop = false;

   string cmdDisturbFile = Paths::DisturbDir( m2c) + filename;
   filename.replace( filename.begin(), filename.begin()+3, "Modal");
   string modalDisturbFile = Paths::DisturbDir( m2c) + filename;

   string timestamp = Utils::asciiDateAndTimeCompact();


   // Create output directory
   string savePath = Paths::IntmatAcqDir( m2c, timestamp);
   string absSavePath = Paths::IntmatAcqDir( m2c, timestamp, true);
   mkdir( absSavePath.c_str(), 0777);

   Command *cmd;


   // Check Arbitrators status
   logCallback("Checking arbitrator status", argp);

   cmd = new RequestStatus( shortTimeout);
   sendCommandWithReply( wfsIntf, cmd, &reply);
   string arbStatus = ((RequestStatus *)reply)->getArbitratorStatus();
   delete reply;

   if ( arbStatus != "AOSet") {
      warningCallback("WFS is not in AOSet state", argp);
      return "";
   }

   cmd = new RequestStatus( shortTimeout);
   sendCommandWithReply( adsecIntf, cmd, &reply);
   arbStatus = ((RequestStatus *)reply)->getArbitratorStatus();
   delete reply;

   if ( arbStatus != "AOSet") {
      warningCallback("AdSec is not in AOSet state", argp);
      return "";
   }


   // Split measurement over multiple files. First by disturbance part, then by iterations over the same part 

   int totLen = calcLen( Paths::DisturbDir( m2c, true) + filename, iterations );
   int disturbLen = totLen / iterations;

   int nparts = disturbLen /adsec_disturbLen;
   int nrep = iterations / adsec_iterations;
   if (iterations % adsec_iterations)
      nrep ++;

   // Iterate over disturbance parts

   for (int part=0; part<nparts; part++) {

      // Iterate over repetions of the same disturbance part

      for (int rep=0; rep<nrep; rep++) {

         int n_acq = part*nrep + rep;

         ostringstream msg;
         msg << "Starting acquisition " << (n_acq+1) << " of " << nparts*nrep;
         logCallback(msg.str(), argp);

         logCallback("Generating partial disturbance commands...", argp);

         // Generate filenames for partial disturbance
         string filename =  cmddisturb;

         ostringstream suffix;
         suffix << "_acq" << n_acq << ".fits";

         filename.replace( filename.end()-5, filename.end(), suffix.str());
         string acqCmdDisturbFile = savePath + filename;

         filename.replace( filename.begin(), filename.begin()+3, "Modal");
         string acqModalDisturbFile = savePath + filename;

         // Generate partial disturbance
         ostringstream cmd1, cmd2;
         cmd1 << "idl -e \"split_disturb, '" << Paths::AdSecCalibDir( true) << "', '" << cmdDisturbFile << "', '" << acqCmdDisturbFile << "', " << adsec_disturbLen << ", " << part << "\"";
         system( cmd1.str().c_str());
         cmd2 << "idl -e \"split_disturb, '" << Paths::AdSecCalibDir( true) << "', '" << modalDisturbFile << "', '" << acqModalDisturbFile << "', " << adsec_disturbLen << ", " << part << "\"";
         system( cmd2.str().c_str());

         // Generate filename for slopes file
         ostringstream ss;
         ss << savePath << "SwitchBCU_" << timestamp << "_acq" << n_acq << ".sav";
         string saveFile = ss.str();
        
         // Load disturbance

         disturbParams dist;
         dist._disturbFile = Paths::AdSecCalibDir( true) + acqCmdDisturbFile;
         dist._on = true;
         cmd = new SetDisturb( longTimeout, dist); 

         logCallback("Setting disturb file: "+acqCmdDisturbFile, argp);

         if (sendCommandWithReply( adsecIntf, cmd) != NO_ERROR) {
            warningCallback("SetDisturb command failed", argp);
            return "";
         }

         enableDisturbParams enableParams;
         enableParams._enableWFS = true;
         enableParams._enableOVS = false;
         cmd = new EnableDisturb( longTimeout, enableParams);

         logCallback("Enabling disturbance ", argp);

         if (sendCommandWithReply( wfsIntf, cmd) != NO_ERROR) {
            warningCallback("EnableDisturb command failed", argp);
            return "";
         }

         // Set reconstruction matrix
         if (closeloop) {
            recMatParams recparams;
            string modalBasis = "mirrorStandard";
            string path = Paths::M2CDir( modalBasis, true);
            string filterPath = path + "filtering/pureIntegrator/";


            recparams._m2cFile = path + "m2c.fits";
            recparams._recMatFile = path + "RECs/rec_200m_new3.fits";
            recparams._aDelayFile = filterPath + "mode_delay.fits";
            recparams._bDelayFile = filterPath + "slope_delay.fits";

            cmd = new SetRecMat( longTimeout, recparams);
            logCallback("Setting reconstruction matrix", argp);

            if (sendCommandWithReply( adsecIntf, cmd) != NO_ERROR) {
               warningCallback("SetRecMat command failed", argp);
               return "";
            }
         }

   
         // Set gain
         gainParams g;
         if (closeloop)
            g._gainFile = Paths::GainDir( m2c, true) +"gain0.01_50modi.fits";
         else
            g._gainFile = Paths::GainDir( m2c, true) +"gain0.fits";

         cmd = new SetGain( longTimeout, g);

         logCallback("Setting gain", argp);

         if (sendCommandWithReply( adsecIntf, cmd) != NO_ERROR) {
            warningCallback("SetGain command failed", argp);
            return "";
         }

         // Close loop
         cmd = new CloseLoop(shortTimeout);

         logCallback("Closing loop", argp);

         if (sendCommandWithReply( wfsIntf, cmd) != NO_ERROR) {
            warningCallback("CloseLoop command failed", argp);
            return "";
         }


         int myrep= adsec_iterations;
         if ( iterations - rep*adsec_iterations < adsec_iterations)
            myrep = iterations - rep*adsec_iterations;

         int time = int( ( (float)adsec_disturbLen/ loopfreq) * (float)myrep);
         time +=3;   // Acquire a bit more slopes

         ostringstream s;
         s << "Acquiring for " << time << " seconds...";
         logCallback(s.str(), argp);

         // Wait for acquisition
         sleep(time);

         // Open loop
         cmd = new StopLoop(shortTimeout);

         logCallback("Opening loop", argp);

         if (sendCommandWithReply( wfsIntf, cmd) != NO_ERROR) {
            warningCallback("StopLoop command failed", argp);
            return "";
         }



         // Save wfs status and slopes data
         Wfs_Arbitrator::saveStatusParams params;
         params._outputFile = Paths::IntmatAcqDir( m2c, timestamp, true)+"system.fits";

         cmd = new Wfs_Arbitrator::SaveStatus( longTimeout, params);
         logCallback("Saving WFS status", argp);

         if (sendCommandWithReply( wfsIntf, cmd) != NO_ERROR) {
            warningCallback("SaveStatus command failed", argp);
            return "";
         }


         saveSlopesParams ss_params;
         ss_params._filename = Paths::AdSecCalibDir( true) + saveFile;
         ss_params._nFrames = adsec_disturbLen * (myrep+1);

         cmd = new SaveSlopes(longTimeout, ss_params);

         logCallback("Saving data...", argp);

         if (sendCommandWithReply( adsecIntf, cmd) != NO_ERROR) {
            warningCallback("SaveSlopes command failed", argp);
            return "";
         }

         // Modify saved file
         ostringstream idlcmd;

         idlcmd << "idl -e \"params=create_struct('M2C', '" << m2c << "', 'MODAL_DISTURB_FILE', '" << modalDisturbFile << "', 'CMD_DISTURB_FILE', '" << cmdDisturbFile << "', 'PART_MODAL_DISTURB_FILE', '" << acqModalDisturbFile << "', 'PART_CMD_DISTURB_FILE', '" << acqCmdDisturbFile << "', 'FRAMES_LEN', " << (adsec_disturbLen * myrep) << ", 'PART', " << n_acq << ", 'TOTAL_PARTS', " << nparts*nrep << ")";
         idlcmd << "& add_to_sav, '" << Paths::AdSecCalibDir( true) + saveFile << "', params\"";

         logCallback("Adding parameters to save file....", argp);

         system(idlcmd.str().c_str());


      }
   }

   return timestamp;

}

string doCloop( int /* nFrames */) { return ""; }




int setLoopFreq( ArbitratorInterface *wfsIntf, int binning, float freq, float modulation)
{
    modifyAOparams params;
    params.Binning = binning;
    params.freq = freq;
    params.TTmod = modulation;
    params.Fw1Pos = -1;
    params.Fw2Pos = -1;
    params.checkCameralens = false;

    Command *cmd = new ModifyAO( longTimeout, params);

    return sendCommandWithReply( wfsIntf, cmd);
}


int setRec( ArbitratorInterface *adsecIntf,  string m2c, string rec)
{
    recMatParams recparams;
    string path = Paths::M2CDir( m2c, true);
    string filterPath = path + "filtering/pureIntegrator/";

    if (rec.substr(0,4) != "Rec_")
       rec = "Rec_"+rec;
    if (rec.substr( rec.size()-5, 5) != ".fits")
       rec = rec + ".fits";

    recparams._m2cFile = path + "m2c.fits";
    recparams._recMatFile = path + "RECs/" +rec;
    recparams._aDelayFile = filterPath + "mode_delay.fits";
    recparams._bDelayFile = filterPath + "slope_delay.fits";

    Command *cmd = new SetRecMat( longTimeout, recparams);
    return sendCommandWithReply( adsecIntf, cmd);
}

int setGain( ArbitratorInterface *adsecIntf, string m2c, float gain)
{
   char gainfile[FILENAME_MAX];
   sprintf( gainfile, "%sgain%3.1f.fits", Paths::GainDir( m2c, true).c_str(), gain);

   //printf("%s\n", gainfile);

   gainParams par;
   par._gainFile = gainfile;
   Command *cmd = new SetGain( longTimeout, par);
   return sendCommandWithReply( adsecIntf, cmd);
}

int setDisturb( ArbitratorInterface *adsecIntf, string disturb)
{
   if (disturb.substr( disturb.size()-5, 5) != ".fits")
       disturb += ".fits";

   disturbParams dist;
   dist._disturbFile = Paths::DisturbDir( "", true) + disturb;
   dist._on = true;

   //printf("%s\n", dist._disturbFile.c_str());
   Command *cmd = new SetDisturb( longTimeout, dist);

   return sendCommandWithReply( adsecIntf, cmd);
}

int enableDisturb( ArbitratorInterface *wfsIntf, bool wfs, bool adsec)
{
   enableDisturbParams distParams;
   distParams._enableWFS = wfs;
   distParams._enableOVS = adsec;
   Command *cmd = new EnableDisturb( longTimeout, distParams);

   return sendCommandWithReply( wfsIntf, cmd);
}

int closeLoop( ArbitratorInterface *wfsIntf)
{
   Command *cmd = new CloseLoop( shortTimeout);
   return sendCommandWithReply( wfsIntf, cmd);
}

int openLoop( ArbitratorInterface *wfsIntf)
{
   Command *cmd = new StopLoop( shortTimeout);
   return sendCommandWithReply( wfsIntf, cmd);
}

string saveOptData( ArbitratorInterface *wfsIntf, int nFrames, int nCcd47, int nIrtc)
{
   Wfs_Arbitrator::saveOptLoopDataParams params;

   params._nFrames = (nFrames>0) ? nFrames : 1;
   params._saveFrames = true;
   params._saveSlopes = true;
   params._saveModes  = true;
   params._saveCommands = true;
   params._savePositions = true;
   params._savePsf = (nCcd47>0) ? true : false;
   params._saveIrtc = (nIrtc>0) ? true : false;
   if (nCcd47>0)
      params._nFramesPsf = nCcd47;
   if (nIrtc>0)
      params._nFramesIrtc = nIrtc;

   int timeout = 1000*1000;
   Command *cmd = new Wfs_Arbitrator::SaveOptLoopData( timeout, params);
   Command *reply=NULL;
   try {
      reply = wfsIntf->requestCommand(cmd);
   } catch (ArbitratorInterfaceException &e) {
      printf("Error saving optical loop data\n");
      return "";
   }

   if (!reply) {
     printf("Error saving optical loop data\n");
     return "";
   }
   else if (! reply->isSuccess()) {
     printf("Error saving optical loop data\n");
     return "";
   }
   else {
      printf("%s", ((Wfs_Arbitrator::SaveOptLoopData *)reply)->getParams()._trackNum.c_str());
      return ((Wfs_Arbitrator::SaveOptLoopData *)reply)->getParams()._trackNum;
   }
}














} // namespace IntmatLib 
} // namespace Arcetri


