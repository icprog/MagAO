
#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "intmatAcquireGui.h"
#include "arblib/wfsArb/WfsInterfaceDefines.h"

#include "arblib/base/CommandsExport.h"
#include "arblib/wfsArb/WfsCommandsExport.h"
#include "arblib/adSecArb/AdSecCommandsExport.h"



#include <qlabel.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qtextedit.h>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <kapplication.h>
#include <kurlrequester.h>
#include <kcmdlineargs.h>
#include <kfiledialog.h>


#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#include <dirent.h>     // DT_REG
#include <stdlib.h>
#include <sys/stat.h>   // mkdir()
#include <sys/types.h>  // mkdir()


#include "Paths.h"
#include "Utils.h"
#include "RecPreview.h"


extern "C" {
#include "base/timelib.h"
}


using namespace Arcetri;
using namespace Arcetri::Arbitrator;
using namespace Arcetri::Wfs_Arbitrator;
using namespace Arcetri::AdSec_Arbitrator;

extern "C" {
#include "iolib.h"
}

// If this is defined, no commands are sent to arbitrators except for status requests
#undef TEST_ONLY

intmatAcquireGui::intmatAcquireGui( string name, string configFile, KApplication &kApp) : AOAppGui(name, configFile, kApp) {

}

intmatAcquireGui::intmatAcquireGui( int argc, char *argv[], KApplication &kApp) : AOAppGui(argc, argv, kApp) {

}

intmatAcquireGui::~intmatAcquireGui() {
   delete _wfsIntf;
   delete _adsecIntf;
}

void intmatAcquireGui::PostInit() {
   init();

   _wfsIntf = new ArbitratorInterface( "wfsarb."+AOApp::Side(), Logger::LOG_LEV_DEBUG);
   //_adsecIntf = new ArbitratorInterface( "adsecarb."+AOApp::Side());
   _adsecIntf = new ArbitratorInterface( "adsecarb."+AOApp::Side()+"@M_ADSEC",  Logger::LOG_LEV_DEBUG);

   _varCcdFreq= RTDBvar("ccd39."+AOApp::Side()+".FRMRT.CUR", REAL_VARIABLE, 1);
//   Notify(_varCcdFreq, varHandler, 1);

   _varDriftEnabledCur = RTDBvar( "optloopdiag."+Side()+".DRIFT.ENABLE.CUR", INT_VARIABLE, 1);
//   Notify(_varDriftEnabledCur, varHandler, 1);

}

int intmatAcquireGui::varHandler(void* thisPtr, Variable* var) {

  intmatAcquireGui *mySelf = (intmatAcquireGui *)thisPtr;
  mySelf->varCcdFreq().MatchAndSet(var);
  mySelf->varDriftEnabledCur().MatchAndSet(var);
  return NO_ERROR;
}


void intmatAcquireGui::init() {

   string path = Paths::M2CDir( "", true);
   vector<string> M2Cs = Paths::readDir(path, DT_DIR);
   vector<string>::iterator iter;


   comboM2C->clear();

   for (iter = M2Cs.begin(); iter != M2Cs.end(); iter++)
      comboM2C->insertItem( *iter);

   for (iter = M2Cs.begin(); iter != M2Cs.end(); iter++)
      comboM2Cloop->insertItem( *iter);

   comboM2C_activated( comboM2C->currentText());
   //recalcLen();
}

void intmatAcquireGui::comboM2C_activated( const QString &s )
{
   string path = Paths::DisturbDir( s.latin1(), true);
   vector<string> amps = Paths::readDir(path, DT_REG);
   sort(amps.begin(), amps.end());
   vector<string>::iterator iter;

   comboDisturb->clear();

   for (iter = amps.begin(); iter != amps.end(); iter++)
      if (iter->compare(0,10, "CmdDisturb") == 0)
         comboDisturb->insertItem( *iter);

   comboDisturb->setCurrentItem( comboDisturb->count()-1);
}

void intmatAcquireGui::comboDisturb_activated( const QString & /* s */ )
{
   recalcLen();
}

void intmatAcquireGui::spinBoxIterations_valueChanged( int /* value */)
{
   recalcLen();
}

int intmatAcquireGui::recalcLen()
{
   string disturbFile = Paths::DisturbDir(  comboM2C->currentText().latin1(), true) + comboDisturb->currentText().latin1();
   int iterations = spinBoxIterations->value();

   int ndims;
   long *dims;
   int datatype;

   int stat = PeekFitsFile( (char*)disturbFile.c_str(), &datatype, &ndims, &dims);
   if (stat != NO_ERROR) {
      labelAcqLen->setText("Error");
      return -1;
   }

   char str[32];
   int len = dims[0] * iterations;
   sprintf(str, "%d", len);
   labelAcqLen->setText(str);

   return len;
}


void intmatAcquireGui::customEvent( QCustomEvent *e) {

   string s;
   LogEvent *logEvent;
   SaveFileEvent *saveFileEvent;
   WarningEvent *warningEvent;

   switch(e->type()) {

      case LOG_EVENT_ID:
         logEvent = (LogEvent *)e;
         s = logEvent->getText();

         logText->append(s);
         break;

      case SAVEFILE_EVENT_ID:
         saveFileEvent = (SaveFileEvent *)e;
         s = saveFileEvent->getText();

         labelFilename->setText(s);
         break;

      case WARNING_EVENT_ID:
         warningEvent = (WarningEvent *)e;
         QMessageBox::warning(this, "Warning", warningEvent->getText());
         break;

      default:
         break;
   }
}

void intmatAcquireGui::buttonAcquire_clicked()
{
    _varCcdFreq.Get(&_freq);

    if ((_freq<500) || (_freq>700)) {
	ostringstream oss;
	oss << "Loop speed is " << int(_freq) << " Hz. Acquire anyway?";
        int ret = QMessageBox::question( NULL, "Warning", oss.str(), QMessageBox::Yes, QMessageBox::No);
	if (ret != QMessageBox::Yes)
		return;
    }

    int drift;
    _varDriftEnabledCur.Get(&drift);

    if (drift) {
	ostringstream oss;
	oss << "Anti-drift is enabled. Please disable it before acquiring";
        QMessageBox::warning( NULL, "Warning", oss.str());
        return;
    }

    boost::thread* thAcquire = new boost::thread(boost::bind(&intmatAcquireGui::acquireThread, this));
    delete thAcquire;
}

void intmatAcquireGui::kURLRec_openFileDialog( KURLRequester *kurl ) {
   string m2c = comboM2Cloop->currentText();
   kurl->setURL( Paths::RECsDir( m2c, true));
   kurl->setFilter("Rec_*");
   KFileDialog *dlg = kurl->fileDialog();
   RecPreview *preview = new RecPreview(NULL);
   dlg->setPreviewWidget(preview);
}

void intmatAcquireGui::kURLGain_openFileDialog( KURLRequester *kurl ) {
   string m2c = comboM2Cloop->currentText();
   kurl->setURL( Paths::GainDir( m2c, true));
}



void intmatAcquireGui::acquireThread()
{ 
   Command *reply;

   int adsec_disturbLen = 4000;
   int adsec_bufferLen = 16000;
   int adsec_iterations = adsec_bufferLen / adsec_disturbLen;

   string m2c = comboM2C->currentText();
   string m2cLoop = comboM2Cloop->currentText();
   string m2cFile = Paths::M2CFile( m2c);
   string filename =  comboDisturb->currentText().latin1();
   bool closeloop = checkCloseloop->isChecked();

   double loopSpeed = _freq;


   string recFile="";
   string gainFile="";

   if (closeloop) {
      recFile = kURLRec->url().latin1();
      gainFile = kURLGain->url().latin1();
   }


   string cmdDisturbFile = Paths::DisturbDir( m2c) + filename;
   filename.replace( filename.begin(), filename.begin()+3, "Modal");
   string modalDisturbFile = Paths::DisturbDir( m2c) + filename;

   string timestamp = Utils::asciiDateAndTimeCompact();

   int shortTimeout = 10*1000;
   int longTimeout = 120*1000;

   // Check arb. communication
   if (! _wfsIntf->check()) {
      WarningEvent *e = new WarningEvent("No connection with WFS arbitrator");
      postEvent(this, e);
      return;
   }
   if (! _adsecIntf->check()) {
      WarningEvent *e = new WarningEvent("No connection with AdSec arbitrator");
      postEvent(this, e);
      return;
   }
   

   // Create output directory
   string savePath = Paths::IntmatAcqDir( m2c, timestamp);
   string absSavePath = Paths::IntmatAcqDir( m2c, timestamp, true);
   mkdir( absSavePath.c_str(), 0777);

   LogEvent *e;
   Command *cmd;

#ifndef TEST_ONLY

   // Check Arbitrators status
   e = new LogEvent("Checking arbitrator status");
   postEvent(this, e);

   cmd = new RequestStatus( shortTimeout);
   sendCommandWithReply( _wfsIntf, cmd, &reply);
   string arbStatus = ((RequestStatus *)reply)->getArbitratorStatus();
   delete reply;

   if ( arbStatus != "AOSet") {
      WarningEvent *e = new WarningEvent("WFS is not in AOSet state");
      postEvent(this, e);
      return;
   }

   cmd = new RequestStatus( shortTimeout);
   sendCommandWithReply( _adsecIntf, cmd, &reply);
   arbStatus = ((RequestStatus *)reply)->getArbitratorStatus();
   delete reply;

   if ( arbStatus != "AOSet") {
      WarningEvent *e = new WarningEvent("AdSec is not in AOSet state");
      postEvent(this, e);
      return;
   }
printf("5\n");

#endif

   // Split measurement over multiple files. First by disturbance part, then by iterations over the same part 

   int iterations = spinBoxIterations->value();
   int totLen = recalcLen();
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
         e = new LogEvent(msg.str());
         postEvent(this, e);

         e = new LogEvent("Generating partial disturbance commands...");
         postEvent(this, e);

         // Generate filenames for partial disturbance
         string filename =  comboDisturb->currentText().latin1();

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
        
#ifndef TEST_ONLY
         // Load disturbance

         disturbParams dist;
         dist._disturbFile = Paths::AdSecCalibDir( true) + acqCmdDisturbFile;
         dist._on = true;
         cmd = new SetDisturb( longTimeout, dist); 

         e = new LogEvent("Setting disturb file: "+acqCmdDisturbFile);
         postEvent(this, e);

         if (sendCommandWithReply( _adsecIntf, cmd) != NO_ERROR) {
            WarningEvent *e = new WarningEvent("SetDisturb command failed");
            postEvent(this, e);
            return;
         }

         enableDisturbParams enableParams;
         enableParams._enableWFS = true;
         enableParams._enableOVS = false;
         cmd = new EnableDisturb( longTimeout, enableParams);

         e = new LogEvent("Enabling disturbance");
         postEvent(this, e);

         if (sendCommandWithReply( _wfsIntf, cmd) != NO_ERROR) {
            WarningEvent *e = new WarningEvent("EnableDisturb command failed");
            postEvent(this, e);
            return;
         }


         // Set reconstruction matrix
         if (closeloop) {
            recMatParams recparams;
            string path = Paths::M2CDir( m2cLoop, true);
            string filterPath = path + "filtering/pureIntegrator/";


            recparams._m2cFile = path + "m2c.fits";
            recparams._recMatFile = recFile;
            recparams._aDelayFile = filterPath + "mode_delay.fits";
            recparams._bDelayFile = filterPath + "slope_delay.fits";

            cmd = new SetRecMat( longTimeout, recparams);
            e = new LogEvent("Setting reconstruction matrix");
            postEvent(this, e);

            if (sendCommandWithReply( _adsecIntf, cmd) != NO_ERROR) {
               WarningEvent *e = new WarningEvent("SetRecMat command failed");
               postEvent(this, e);
               return;
            }
         }

   
         // Set gain
         gainParams g;
         if (closeloop)
            g._gainFile = gainFile;
         else
            g._gainFile = Paths::GainDir( m2c, true) +"gain0.fits";

         cmd = new SetGain( longTimeout, g);

         e = new LogEvent("Setting gain");
         postEvent(this, e);

         if (sendCommandWithReply( _adsecIntf, cmd) != NO_ERROR) {
            WarningEvent *e = new WarningEvent("SetGain command failed");
            postEvent(this, e);
            return;
         }

         // Close loop
         cmd = new CloseLoop(shortTimeout);

         e = new LogEvent("Closing loop");
         postEvent(this, e);

         if (sendCommandWithReply( _wfsIntf, cmd) != NO_ERROR) {
            WarningEvent *e = new WarningEvent("CloseLoop command failed");
            postEvent(this, e);
            return;
         }


         int myrep= adsec_iterations;
         if ( iterations - rep*adsec_iterations < adsec_iterations)
            myrep = iterations - rep*adsec_iterations;

         int time = int( ( (float)adsec_disturbLen/loopSpeed) * (float)myrep);
	 if (loopSpeed>900) time +=1;
         else time +=3;   // Acquire a bit more slopes

         ostringstream s;
         s << "Acquiring for " << time << " seconds...";
         e = new LogEvent(s.str());
         postEvent(this, e);

         // Wait for acquisition
         msleep(1000*time);

         enableParams._enableWFS = false;
         enableParams._enableOVS = false;
         cmd = new EnableDisturb( longTimeout, enableParams);

         e = new LogEvent("Disabling disturbance");
         postEvent(this, e);

         if (sendCommandWithReply( _wfsIntf, cmd) != NO_ERROR) {
            WarningEvent *e = new WarningEvent("EnableDisturb command failed");
            postEvent(this, e);
            return;
         }

         msleep(100*time);

         // Open loop
         cmd = new StopLoop(shortTimeout);

         e = new LogEvent("Opening loop");
         postEvent(this, e);

         if (sendCommandWithReply( _wfsIntf, cmd) != NO_ERROR) {
            WarningEvent *e = new WarningEvent("StopLoop command failed");
            postEvent(this, e);
            return;
         }
#else
         int myrep=1;
#endif



         // Save wfs status and slopes data
         Wfs_Arbitrator::saveStatusParams params;
         params._outputFile = Paths::IntmatAcqDir( m2c, timestamp, true)+"system.fits";

         cmd = new Wfs_Arbitrator::SaveStatus( longTimeout, params);
         e = new LogEvent("Saving WFS status");
         postEvent( this, e);

         if (sendCommandWithReply( _wfsIntf, cmd) != NO_ERROR) {
            WarningEvent *e = new WarningEvent("SaveStatus command failed");
            postEvent(this, e);
            return;
         }


         saveSlopesParams ss_params;
         ss_params._filename = Paths::AdSecCalibDir( true) + saveFile;
         ss_params._nFrames = adsec_disturbLen * (myrep+1);

         cmd = new SaveSlopes(longTimeout, ss_params);

         e = new LogEvent("Saving data....");
         postEvent(this, e);

         if (sendCommandWithReply( _adsecIntf, cmd) != NO_ERROR) {
            WarningEvent *e = new WarningEvent("SaveSlopes command failed");
            postEvent(this, e);
            return;
         }

         // Modify saved file
         ostringstream idlcmd;

         idlcmd << "idl -e \"params=create_struct('M2C', '" << m2c << "', 'MODAL_DISTURB_FILE', '" << modalDisturbFile << "', 'CMD_DISTURB_FILE', '" << cmdDisturbFile << "', 'PART_MODAL_DISTURB_FILE', '" << acqModalDisturbFile << "', 'PART_CMD_DISTURB_FILE', '" << acqCmdDisturbFile << "', 'FRAMES_LEN', " << (adsec_disturbLen * myrep) << ", 'PART', " << n_acq << ", 'TOTAL_PARTS', " << nparts*nrep << ", 'RECFILE', '" << recFile << "', 'GAINFILE', '" << gainFile << "')";
         idlcmd << "& add_to_sav, '" << Paths::AdSecCalibDir( true) + saveFile << "', params\"";

         e = new LogEvent("Adding parameters to save file....");
         postEvent(this, e);

         system(idlcmd.str().c_str());


      }
   }

   e = new LogEvent("Acquisition done");
   postEvent(this, e);


   SaveFileEvent *se = new SaveFileEvent(savePath);
   postEvent( this, se);

}


void intmatAcquireGui::checkCloseloop_clicked() {

   if (checkCloseloop->isChecked()) {
      kURLRec->setEnabled(true);
      kURLGain->setEnabled(true);
   } else {
      kURLRec->setEnabled(false);
      kURLGain->setEnabled(false);
   }
}

int intmatAcquireGui::sendCommandWithReply( ArbitratorInterface *intf, Command* cmd, Command **reply) {

   Command* cmdReply;
   int success;

   try {
      _logger->log(Logger::LOG_LEV_INFO, " >> Sending command %s request...", cmd->getDescription().c_str());
      cmdReply = intf->requestCommand(cmd);
      _logger->log(Logger::LOG_LEV_INFO, " >> Command reply (%s) received!", cmdReply->getStatusAsString().c_str());
      cmdReply->log();
      success = cmdReply->isSuccess();
      if (reply)
         *reply = cmdReply;
      else
         delete cmdReply;
   }
   catch(...) {
      _logger->log(Logger::LOG_LEV_ERROR, "Command %s error", cmd->getDescription().c_str());
      delete cmd;
      return -1;
   }

   delete cmd;

   if (!success)
         return -1;
   else
      return NO_ERROR;

}





static const KCmdLineOptions options[] =
{
      {"f <file>", ("Configuration file"), 0 },
      {"i <identity>", ("MsgD identity"), 0 },
         KCmdLineLastOption
};




int main( int argc, char *argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

   KCmdLineArgs::init( argc, argv, "intmatAcquireGui", "intmatAcquireGui", "intmatAcquireGui", "1.0" );
   KCmdLineArgs::addCmdLineOptions(options);
   KApplication kApp;

   intmatAcquireGui *gui = NULL;

   try {
      if (argc>1)
         gui = new intmatAcquireGui( argc, argv, kApp);
      else {
         gui = new intmatAcquireGui("imacqgui", AOApp::getConffile("imacqgui"), kApp);
      }

      gui->doGui(gui);

      delete gui;
      }
   catch (LoggerFatalException &e) {
      // In this case the logger can't log!!!
      printf("%s\n", e.what().c_str());
   }
   catch (AOException &e) {
      Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error: %s", e.what().c_str());
   }

   return 0;
}
  

