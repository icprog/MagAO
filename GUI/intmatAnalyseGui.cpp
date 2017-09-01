
#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "intmatAnalyseGui.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <kapplication.h>
#include <kurlrequester.h>
#include <kcmdlineargs.h>


#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>
using namespace std;

#include <dirent.h>
#include <stdlib.h>

#include "Paths.h"
#include "Utils.h"

extern "C" {
#include "base/timelib.h"
}



using namespace Arcetri;

intmatAnalyseGui::intmatAnalyseGui( string name, string configFile, KApplication &kApp) : AOAppGui(name, configFile, kApp) {

}

intmatAnalyseGui::intmatAnalyseGui( int argc, char *argv[], KApplication &kApp) : AOAppGui(argc, argv, kApp) {

}

void intmatAnalyseGui::PostInit() {
   init();

}

void intmatAnalyseGui::init() {

   string path = Paths::M2CDir( "", true);
   printf("%s\n", path.c_str());
   vector<string> M2Cs = Paths::readDir(path, DT_DIR);
   vector<string>::iterator iter;

   comboM2C->clear();

   for (iter = M2Cs.begin(); iter != M2Cs.end(); iter++)
      comboM2C->insertItem( *iter);

   comboM2C_activated( comboM2C->currentText());

   editSkipFrame->setText("2");
   editAvgFrame->setText("2");

}

void intmatAnalyseGui::comboM2C_activated( const QString &s )
{
   string path = Paths::IntmatAcqDir( s.latin1(), "", true);
   vector<string> amps = Paths::readDir(path, DT_DIR);
   sort(amps.begin(), amps.end());
   vector<string>::iterator iter;

   comboSavefile->clear();

   for (iter = amps.begin(); iter != amps.end(); iter++)
      comboSavefile->insertItem( *iter);

   comboSavefile->setCurrentItem( comboSavefile->count()-1);
}


void intmatAnalyseGui::buttonAnalyse_clicked()
{

   string m2c = comboM2C->currentText();
   string m2cFile = Paths::M2CFile( m2c);

   string intmatAcqDir = Paths::IntmatAcqDir(  m2c) + comboSavefile->currentText().latin1();

   string timestamp = Utils::asciiDateAndTimeCompact();
   string intmatFile = Paths::RECsDir(  m2c) + "Intmat_" + timestamp + ".fits";
   string recFile = Paths::RECsDir(  m2c) + "Rec_"+ timestamp + ".fits";
   
   int skipFrame = atoi( editSkipFrame->text().latin1());
   int avgFrame = atoi( editSkipFrame->text().latin1());

   int checkSat = checkSaturation->isChecked();
   int removeTT = checkRemoveTT->isChecked();

   ostringstream cmd;
   cmd << "idl -e \"gen_intmat_gui, PATH_PREFIX='" << Paths::AdSecCalibDir( true) << "', INTMAT_ACQ_DIR='" << intmatAcqDir << "', OUTPUT_FILE='" << intmatFile << "', SKIP_FRAME=" << skipFrame << ", AVG_FRAME=" << avgFrame << ", SIG_CHECK=" << checkSat << ", REMOVE_TILT=" << removeTT << ", /INTERACTIVE\"";

   printf("%s\n", cmd.str().c_str());
   system(cmd.str().c_str());

   ostringstream cmd2;
   cmd2 << "idl -e \"gen_reconstructor_gui, PATH_PREFIX='" << Paths::AdSecCalibDir( true) << "', INPUT_FILE='" << intmatFile << "', OUTPUT_FILE='" << recFile << "', FINAL_DIM=[1600,672]\"";
   printf("%s\n", cmd2.str().c_str());
   system(cmd2.str().c_str());

   labelFilename->setText( recFile);

}



static const KCmdLineOptions options[] =
{
    {"f <file>", ("Configuration file"), 0 },
    {"i <identity>", ("MsgD identity"), 0 },
    KCmdLineLastOption
};




int main( int argc, char *argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

   KCmdLineArgs::init( argc, argv, "intmatAnalyseGui", "intmatAnalyseGui", "intmatAnalyseGui", "1.0" );
   KCmdLineArgs::addCmdLineOptions(options);
   KApplication kApp;

   intmatAnalyseGui *gui = NULL;

   try {
      if (argc>1)
         gui = new intmatAnalyseGui( argc, argv, kApp);
      else {
         gui = new intmatAnalyseGui("intmatanalysegui", AOApp::getConffile("intmatanalysegui"), kApp);
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
  

