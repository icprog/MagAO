
#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "psfOptimizationGui.h"
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
#include <kurlrequester.h>
#include <kcmdlineargs.h>
#include <knuminput.h>


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

psfOptimizationGui::psfOptimizationGui( string name, string configFile, KApplication &kApp) : AOAppGui(name, configFile, kApp) {

}

psfOptimizationGui::psfOptimizationGui( int argc, char *argv[], KApplication &kApp) : AOAppGui(argc, argv, kApp) {

}

psfOptimizationGui::~psfOptimizationGui() {
   delete _wfsIntf;
   delete _adsecIntf;
}

void psfOptimizationGui::PostInit() {
   init();

   _wfsIntf = new ArbitratorInterface( "wfsarb."+AOApp::Side(), Logger::LOG_LEV_DEBUG);
   _adsecIntf = new ArbitratorInterface(  Utils::getClientname("adsecarb", true, "ADSEC"), Logger::LOG_LEV_DEBUG);

}

void psfOptimizationGui::init() {

   string path = Paths::M2CDir( "", true);
   vector<string> M2Cs = Paths::readDir(path, DT_DIR);
   vector<string>::iterator iter;

   comboM2C->clear();

   for (iter = M2Cs.begin(); iter != M2Cs.end(); iter++)
      comboM2C->insertItem( *iter);

   comboM2C_activated( comboM2C->currentText());
}

void psfOptimizationGui::comboM2C_activated( const QString &s )
{

   // Fill REC combobox

   string path = Paths::RECsDir( s.latin1(), true);
   vector<string> recs = Paths::readDir(path, DT_REG);
   vector<string>::iterator iter;

   comboRec->clear();

   for (iter = recs.begin(); iter != recs.end(); iter++)
      if (iter->compare(0,4, "Rec_") == 0)
         comboRec->insertItem( *iter);

   // Fill IM combobox

   comboIM->clear();

   for (iter = recs.begin(); iter != recs.end(); iter++)
      if (iter->compare(0,7, "Intmat_") == 0)
         if (iter->compare( iter->size()-5, 5, ".fits")== 0)
             comboIM->insertItem( *iter);

}

void psfOptimizationGui::kURLGain_openFileDialog( KURLRequester *kurl ) {
      kurl->setURL( Paths::GainDir( comboM2C->currentText(), true));
}


void psfOptimizationGui::customEvent( QCustomEvent *e) {

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

void psfOptimizationGui::buttonStart_clicked()
{
    boost::thread* thAcquire = new boost::thread(boost::bind(&psfOptimizationGui::acquireThread, this));
    delete thAcquire;
}


void psfOptimizationGui::acquireThread()
{ 
   Command *reply;

   string m2c = comboM2C->currentText();
   string m2cFile = Paths::M2CFile( m2c);
   string recFile = Paths::RECsDir(  m2c, true) + comboRec->currentText();
   string gainFile = kURLGain->url();
   string IMfile = Paths::RECsDir(  m2c, true) + comboIM->currentText();

   int nModes = spinBoxNumModes->value();
   double ampFactor = kDoubleAmplitude->value();
   int nSteps = spinBoxSteps->value();

   string timestamp = Utils::asciiDateAndTimeCompact();

     ostringstream pythoncmd;
     pythoncmd << "thaoshell.py -e \"from psfOptimization import *; optimizePSF(app, '" << IMfile << "', " << nModes << ", " << ampFactor << ", " << nSteps << ") \" ";
     printf("%s\n", pythoncmd.str().c_str());

   int shortTimeout = 10*1000;
   int longTimeout = 40*1000;

   // Create output directory
   string savePath = Paths::PsfOptDir( m2c, timestamp);
   string absSavePath = Paths::PsfOptDir( m2c, timestamp, true);
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

#endif

    recMatParams recparams;
    string path = Paths::M2CDir( m2c, true);
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

   
    // Set gain
    gainParams g;
    g._gainFile = gainFile;

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


         // Save wfs status and slopes data
     /*
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
*/

        // Spawn python script
     /*
     ostringstream pythoncmd;

     pythoncmd << "thaoshell.py -e \"from psfOptimization import *; optimizePSF(app, '" << IMfile << "', " << nModes << ", " << ampFactor << ", " << nSteps << ") \" ";
     printf("%s\n", pythoncmd.str().c_str());

     */

   // int dummy = system(pythoncmd.str().c_str());

     // Open loop
     cmd = new StopLoop(shortTimeout);

     e = new LogEvent("Opening loop");
     postEvent(this, e);

     if (sendCommandWithReply( _wfsIntf, cmd) != NO_ERROR) {
        WarningEvent *e = new WarningEvent("StopLoop command failed");
        postEvent(this, e);
        return;
     }




   e = new LogEvent("Acquisition done");
   postEvent(this, e);


   SaveFileEvent *se = new SaveFileEvent(savePath);
   postEvent( this, se);

}

int psfOptimizationGui::sendCommandWithReply( ArbitratorInterface *intf, Command* cmd, Command **reply) {

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

   KCmdLineArgs::init( argc, argv, "psfOptimizationGui", "psfOptimizationGui", "psfOptimizationGui", "1.0" );
   KCmdLineArgs::addCmdLineOptions(options);
   KApplication kApp;

   psfOptimizationGui *gui = NULL;

   try {
      if (argc>1)
         gui = new psfOptimizationGui( argc, argv, kApp);
      else {
         gui = new psfOptimizationGui("psfoptimizationgui", AOApp::getConffile("psfoptimizationgui"), kApp);
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
  

