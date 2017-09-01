
#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "intmatDisturbGui.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <kurlrequester.h>
#include <kcmdlineargs.h>
#include <kapplication.h>


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

intmatDisturbGui::intmatDisturbGui( string name, string configFile, KApplication &kApp) : AOAppGui(name, configFile, kApp) {

}

intmatDisturbGui::intmatDisturbGui( int argc, char *argv[], KApplication &kApp) : AOAppGui(argc, argv, kApp) {

}

void intmatDisturbGui::PostInit() {
   init();

}

void intmatDisturbGui::init() {

   string path = Paths::M2CDir( "", true);
 
   vector<string> M2Cs = Paths::readDir(path, DT_DIR);
   vector<string>::iterator iter;


   comboM2C->clear();

   for (iter = M2Cs.begin(); iter != M2Cs.end(); iter++)
      comboM2C->insertItem( *iter);

   comboM2C_activated( comboM2C->currentText());
   radioPP_clicked();

}

void intmatDisturbGui::comboM2C_activated( const QString &s )
{
   string path = Paths::ModesAmpDir( s.latin1(), true);
   vector<string> amps = Paths::readDir(path, DT_REG);
   sort(amps.begin(), amps.end());
   vector<string>::iterator iter;

   comboAmpFile->clear();

   for (iter = amps.begin(); iter != amps.end(); iter++)
      comboAmpFile->insertItem( *iter);
}


void intmatDisturbGui::radioPP_clicked()
{
   radioSin->setChecked(0);

   editFramesNum->setEnabled(true);
   editCyclesNum->setEnabled(true);
   comboAmpFile->setEnabled(true);

   kURLfreqFile->setEnabled(false);
   kURLampFileSin->setEnabled(false);
}

void intmatDisturbGui::radioSin_clicked()
{
   radioPP->setChecked(0);

   editFramesNum->setEnabled(false);
   editCyclesNum->setEnabled(false);
   comboAmpFile->setEnabled(false);

   kURLfreqFile->setEnabled(true);
   kURLampFileSin->setEnabled(true);
}

int intmatDisturbGui::recalcDisturbLength()
{
   int modesNum = atoi( editModesNum->text().latin1());
   int framesNum = atoi( editFramesNum->text().latin1());
   int cyclesNum = atoi( editCyclesNum->text().latin1());

   return modesNum * framesNum * cyclesNum *2+100;
}

int intmatDisturbGui::setDisturbLengthLabel( int len)
{
   ostringstream oss;
   oss << "Disturbance length: ";
   if (len <=4000)
      oss << "<font color=darkgreen>" << len << "</font>";
   else
      oss << "<font color=red>" << len << "</font>";

   oss << " (limit: 4000)";
   labelDisturbLength->setText(oss.str());
}

void intmatDisturbGui::editFramesNum_textChanged( const QString & )
{
   setDisturbLengthLabel( recalcDisturbLength());
}

void intmatDisturbGui::editCyclesNum_textChanged( const QString & )
{
   setDisturbLengthLabel( recalcDisturbLength());
}

void intmatDisturbGui::editModesNum_textChanged( const QString & )
{
   setDisturbLengthLabel( recalcDisturbLength());
}

void intmatDisturbGui::kURLfileLgs_openFileDialog( KURLRequester *kurl ) {
   string path = Paths::DisturbDir("", true);
   printf("%s\n", path.c_str());
   kurl->setURL( path);
}


void intmatDisturbGui::buttonGenerate_clicked()
{
   if (radioSin->isChecked()) {
      QMessageBox::warning( this, "Not implemented", "Sinusoidal matrix not implemented");
      return;
   }

   int modesNum = atoi( editModesNum->text().latin1());
   int framesNum = atoi( editFramesNum->text().latin1());
   int cyclesNum = atoi( editCyclesNum->text().latin1());
   string m2c = comboM2C->currentText();
   string m2cFile = Paths::M2CFile( m2c);
   string ampEnvelopeFile = Paths::AmpEnvelopeFile( m2c);
   string ampFile = Paths::ModesAmpDir( m2c) + comboAmpFile->currentText().latin1();

   string timestamp = Utils::asciiDateAndTimeCompact();

   string modalFile = Paths::DisturbDir( m2c) + "ModalDisturb_"+timestamp+".fits";
   string cmdFile = Paths::DisturbDir( m2c) + "CmdDisturb_"+timestamp+".fits";

    ostringstream cmd;
    cmd << "idl -e \"intmat_disturb, " << modesNum <<"," << framesNum << "," << cyclesNum << ",'" << Paths::AdSecCalibDir(true) << "', '" << ampFile << "', '"<< modalFile << "', '" << cmdFile << "', '" << m2cFile << "', '" << ampEnvelopeFile << "'";
    if (checkAddLGS->isChecked())
        cmd << ", ADDFILE='" << kURLfileLgs->url().latin1() << "'";
    cmd  << "\"";
    printf("%s\n", cmd.str().c_str());
    system(cmd.str().c_str());

    labelFilename->setText( cmdFile);
}



static const KCmdLineOptions options[] =
{
   {"f <file>", ("Configuration file"), 0 },
   {"i <identity>", ("MsgD identity"), 0 },
   KCmdLineLastOption
};




int main( int argc, char *argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

   KCmdLineArgs::init( argc, argv, "intmatDisturbGui", "intmatDisturbGui", "intmatDisturbGui", "1.0" );
   KCmdLineArgs::addCmdLineOptions(options);
   KApplication kApp;

   intmatDisturbGui *gui = NULL;

   try {
      if (argc>1)
         gui = new intmatDisturbGui( argc, argv, kApp);
      else {
         gui = new intmatDisturbGui("intmatdisturbgui", AOApp::getConffile("intmatdisturbgui"), kApp);
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
  

