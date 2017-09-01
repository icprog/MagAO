
#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "pupOptimizeGui.h"
#include "arblib/wfsArb/WfsInterfaceDefines.h"

#include "arblib/base/CommandsExport.h"
#include "arblib/wfsArb/WfsCommandsExport.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <kcmdlineargs.h>

#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#include <stdlib.h>


#include "Paths.h"
#include "Utils.h"

extern "C" {
#include "base/timelib.h"
}

#define BINNING (5)

using namespace Arcetri;
using namespace Arcetri::Arbitrator;
using namespace Arcetri::Wfs_Arbitrator;


// If this is defined, no commands are sent to arbitrators except for status requests
#undef TEST_ONLY

pupOptimizeGui::pupOptimizeGui( string name, string configFile, KApplication &kApp) : AOAppGui(name, configFile, kApp) {

}

pupOptimizeGui::pupOptimizeGui( int argc, char *argv[], KApplication &kApp) : AOAppGui(argc, argv, kApp) {
}

pupOptimizeGui::~pupOptimizeGui() {
   delete _wfsIntf;
}

void pupOptimizeGui::PostInit() {
   _wfsIntf = new ArbitratorInterface( "wfsarb."+AOApp::Side(), Logger::LOG_LEV_DEBUG);

   //_varCcdBin= RTDBvar("ccd39."+AOApp::Side()+".XBIN.CUR", INT_VARIABLE, 1);
   //Notify(_varCcdBin, ccdBinHandler, 1);

   _tracknum = "";
}

int pupOptimizeGui::ccdBinHandler(void* thisPtr, Variable* var) {

  pupOptimizeGui *mySelf = (pupOptimizeGui *)thisPtr;
  mySelf->varCcdBin().MatchAndSet(var);
  return NO_ERROR;
}



void pupOptimizeGui::buttonAcquire_clicked()
{

   labelTracknum->setPaletteForegroundColor( QColor( qRgb(128,128,128)));

   int binning;
   //_varCcdBin.Get(&binning);
   binning= BINNING;;

   int ret = QMessageBox::question( NULL, "Are you sure?", "Are you sure? A new calibration will be needed.", QMessageBox::Yes, QMessageBox::No);
   if (ret != QMessageBox::Yes)
            return;

   if ((binning<1) || (binning>5)) {
        ostringstream oss;
        oss << "Current binning is " << binning <<". Valid values are between 1 and 4 included.";
   	QMessageBox::warning( NULL, "Problem", oss.str());
        return;
   }

   ostringstream oss;
   int enlarge[] = { 0, 1, 1, 1, 1, 0};

   char *pythoncmd = "from AdOpt import makePupils; makePupils.makePupilsFromRTDBVars(app, enlarge=";
   oss << "thaoshell.py -e '" << pythoncmd << enlarge[binning] << ")" << "'";

   // Capture output and get last token 
   istringstream iss(Utils::exec(oss.str().c_str()));
   string s;
   while(iss) {
      iss >> s;
      //printf("Token: %s\n", s.c_str());
   }

   s = s.substr(1, s.size()-2);
   labelTracknum->setText(s);
   labelTracknum->setPaletteForegroundColor( QColor( qRgb(0,128,0)));

   // Reload pupils
   oss.str("");
   oss << "change_binning.py " << binning << " last";
   system(oss.str().c_str());
}


void pupOptimizeGui::buttonTest_clicked()
{
   Command *cmd;
   Command *reply;

   labelAcceptedTracknum->setPaletteForegroundColor( QColor( qRgb(128,128,128)));
   labelNumSubaps->setPaletteForegroundColor( QColor( qRgb(128,128,128)));

   int binning;
   //_varCcdBin.Get(&binning);
   binning= BINNING;

   if (binning<1) {
      QMessageBox::warning( NULL, "Warning", "Error: ccd is not ready (binning=0)");
      return;
   }

   if ((_tracknum == "") || (checkForceReacquire->isChecked())) {
      int shortTimeout = 1000;
      int longTimeout = 60*1000;

      saveOptLoopDataParams params;

      params._nFrames = 1000;
      params._saveFrames= true;
      params._saveSlopes= true;
      params._saveModes = false;
      params._saveCommands = false;
      params._savePositions= false;
      params._savePsf= false;
      params._saveIrtc= false;
      params._savePisces= false;
      params._nFramesPsf= 0;
      params._nFramesIrtc= 0;

      cmd = new SaveOptLoopData( longTimeout, params);
      if (sendCommandWithReply( _wfsIntf, cmd, &reply) != NO_ERROR) {
         ostringstream oss;
         oss << "Command " << reply->getDescription() << " failed with status " << reply->getStatusAsString() << "\n" << reply->getErrorString();
         QMessageBox::warning( NULL, "Warning", oss.str());
         return;
      }

      _tracknum = ((SaveOptLoopData *)reply)->getParams()._trackNum;
   }

   ostringstream idlcmd, outdir;
   outdir << Paths::LUTsDir("ccd39") << "/bin" << binning;
   double th = atof(editThreshold->text().latin1());

   idlcmd << "idl -e \"pupacq, '" << _tracknum << "', OUTDIR= '" << outdir.str() << "', THRESHOLD= " << th << ", /WAITKEY \" ";
   printf("%s\n", idlcmd.str().c_str());

   istringstream iss(Utils::exec(idlcmd.str().c_str()));
   string s;
   while(iss) {
      iss >> s;
      //printf("Token: %s\n", s.c_str());
   }
   labelNumSubaps->setText(s);
   labelNumSubaps->setPaletteForegroundColor( QColor( qRgb(0,128,0)));
}

void pupOptimizeGui::buttonAccept_clicked() {

   labelAcceptedTracknum->setPaletteForegroundColor( QColor( qRgb(128,128,128)));

   int binning;
   //_varCcdBin.Get(&binning);
   binning= BINNING;

  // Generate new pupils
   ostringstream pythoncmd, oss;
   pythoncmd << "from AdOpt import makePupils; makePupils.makePupils_hot( " << 80 << "," << binning << ")"; 
   oss << "thaoshell.py -e '" << pythoncmd.str() << "'";

   // Capture output and get last token 
   istringstream iss(Utils::exec(oss.str().c_str()));
   string s;
   while(iss) {
      iss >> s;
      //printf("Token: %s\n", s.c_str());
   }
   s = s.substr(1, s.size()-2);

   labelAcceptedTracknum->setText(s);
   labelAcceptedTracknum->setPaletteForegroundColor( QColor( qRgb(0,128,0)));

   oss.str("");
   oss << "change_binning.py " << binning << " last";
   system(oss.str().c_str());
}



int pupOptimizeGui::sendCommandWithReply( ArbitratorInterface *intf, Command* cmd, Command **reply) {

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

   KCmdLineArgs::init( argc, argv, "pupOptimizeGui", "pupOptimizeGui", "pupOptimizeGui", "1.0" );
   KCmdLineArgs::addCmdLineOptions(options);
   KApplication kApp;

   pupOptimizeGui *gui = NULL;

   try {
      if (argc>1)
         gui = new pupOptimizeGui( argc, argv, kApp);
      else {
         gui = new pupOptimizeGui("pupoptimizegui", AOApp::getConffile("pupoptimizegui"), kApp);
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
  

