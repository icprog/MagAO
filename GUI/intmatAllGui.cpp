
#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "intmatAllGui.h"
#include "intmatlib.h"


#include <qapplication.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qtextedit.h>
#include <qspinbox.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <kurlrequester.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kled.h>


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

using namespace Arcetri;
using namespace Arcetri::IntmatLib;

extern "C" {
#include "iolib.h"
#include "base/timelib.h"
}


// If this is defined, no commands are sent to arbitrators except for status requests
#undef TEST_ONLY

void logCallback( void *argp, string msg)
{
   intmatAllGui *gui = (intmatAllGui *)argp;
   LogEvent *e = new LogEvent(msg);
   gui->postEvent( gui, e);
}

void warningCallback( void *argp, string msg)
{
   intmatAllGui *gui = (intmatAllGui *)argp;
   WarningEvent *e = new WarningEvent(msg);
   gui->postEvent( gui, e);
}

intmatAllGui::intmatAllGui( string name, string configFile) : AOApp(name, configFile), QThread() {

}

intmatAllGui::intmatAllGui( int argc, char *argv[]) : AOApp(argc, argv), QThread() {

}

intmatAllGui::~intmatAllGui() {
   delete _wfsIntf;
   delete _adsecIntf;
}

void intmatAllGui::PostInit() {
   init();

   _wfsIntf = new ArbitratorInterface( "wfsarb."+AOApp::Side(), Logger::LOG_LEV_DEBUG);
   _adsecIntf = new ArbitratorInterface(  Utils::getClientname("adsecarb", true, "ADSEC"), Logger::LOG_LEV_DEBUG);

}

void intmatAllGui::init() {

   string path = Paths::M2CDir( "", true);
   vector<string> M2Cs = Paths::readDir(path, DT_DIR);
   vector<string>::iterator iter;

   comboM2C->clear();

   for (iter = M2Cs.begin(); iter != M2Cs.end(); iter++)
      comboM2C->insertItem( *iter);

   comboM2C_activated( comboM2C->currentText());
}

void intmatAllGui::comboM2C_activated( const QString &s )
{

   string path = Paths::ModesAmpDir( s.latin1(), true);
   vector<string> amps = Paths::readDir(path, DT_REG);
   vector<string>::iterator iter;

   comboAmpFile->clear();

   for (iter = amps.begin(); iter != amps.end(); iter++)
      comboAmpFile->insertItem( *iter);
}

void intmatAllGui::buttonChangeDisturbance_clicked()
{
   string path = Paths::DisturbDir( comboM2C->currentText().latin1(), true);
   QString qs = QFileDialog::getOpenFileName( path, "CmdDisturb*");
   if (qs != QString::null) {
      acceptModulation(qs);
   }
}

void intmatAllGui::buttonChangeAcquisition_clicked()
{
   string path = Paths::IntmatAcqDir( comboM2C->currentText().latin1(), "", true);
   QString qs = QFileDialog::getOpenFileName( path, "*");
   if (qs != QString::null) {
      acceptAcquisition(qs);
   }
}

void intmatAllGui::acceptModulation( string filename) {
    QFileInfo fi( filename);
    labelModulationFilename->setText( Utils::getTracknum( fi.baseName()));

    labelModulation->setText("Done");
    kLedModulation->on();

    labelAcquisition->setText("Missing");
    kLedAcquisition->off();

    labelAnalysis->setText("Missing");
    kLedAnalysis->off();
}


void intmatAllGui::acceptAcquisition( string filename) {
    QFileInfo fi( filename);
    labelAcquireFilename->setText( Utils::getTracknum( fi.baseName()));
   
   // [TODO] get the corresponding modulation 
    labelModulation->setText("Done");
    kLedModulation->on();

    labelAcquisition->setText("Done");
    kLedAcquisition->on();

    labelAnalysis->setText("Missing");
    kLedAnalysis->off();
}






void intmatAllGui::radioPP_clicked()
{
   radioSin->setChecked(0);

   editFramesNum->setEnabled(true);
   editCyclesNum->setEnabled(true);
   comboAmpFile->setEnabled(true);
   checkOptimizeAmp->setEnabled(true);

   kURLfreqFile->setEnabled(false);
   kURLampFileSin->setEnabled(false);
}

void intmatAllGui::radioSin_clicked()
{
   radioPP->setChecked(0);

   editFramesNum->setEnabled(false);
   editCyclesNum->setEnabled(false);
   comboAmpFile->setEnabled(false);
   checkOptimizeAmp->setEnabled(false);

   kURLfreqFile->setEnabled(true);
   kURLampFileSin->setEnabled(true);
}

void intmatAllGui::buttonStartModulation_clicked()
{
   if (radioSin->isChecked()) {
      QMessageBox::warning( this, "Not implemented", "Sinusoidal matrix not implemented");
      return;
   }

   boost::thread* thGenerate = new boost::thread(boost::bind(&intmatAllGui::generateThread, this));
   delete thGenerate;
}

void intmatAllGui::buttonStartAcquire_clicked()
{
   boost::thread* thGenerate = new boost::thread(boost::bind(&intmatAllGui::acquireThread, this));
   delete thGenerate;
}

void intmatAllGui::buttonStartAnalyse_clicked()
{
   boost::thread* thAnalyse = new boost::thread(boost::bind(&intmatAllGui::analyseThread, this));
   delete thAnalyse;
}

void intmatAllGui::analyseThread()
{
   string m2c = comboM2C->currentText();
   string intmatAcqDir = Paths::IntmatAcqDir(  m2c) + labelAcquireFilename->text().latin1();

   string timestamp = Utils::asciiDateAndTimeCompact();
   string intmatFile = Paths::RECsDir(  m2c) + "Intmat_" + timestamp + ".fits";
   string recFile = Paths::RECsDir(  m2c) + "Rec_"+ timestamp + ".fits";

   int skipFrame = atoi( editSkipFrame->text().latin1());
   int avgFrame = atoi( editSkipFrame->text().latin1());
   int nModes   = atoi( editRetainModes->text().latin1());
   int cutModes = atoi( editCutModes->text().latin1());

   int stat = analyseIntmat( intmatAcqDir, intmatFile, recFile, skipFrame, avgFrame, nModes, cutModes, warningCallback, this);
}

void intmatAllGui::acquireThread()
{
   string m2c = comboM2C->currentText();
   string disturbTracknum = Utils::getTracknum( labelModulationFilename->text());
   bool closeloop = checkCloseloop->isChecked();
   int iterations = spinBoxIterations->value();
   


   string timestamp = Utils::asciiDateAndTimeCompact();

   // Create output directory
   string savePath = Paths::IntmatAcqDir( m2c, timestamp);
   string absSavePath = Paths::IntmatAcqDir( m2c, timestamp, true);
   mkdir( absSavePath.c_str(), 0777);
   vector<string> savedFiles;

   int stat = acquireIntmat( _wfsIntf, _adsecIntf, m2c, timestamp, savePath, closeloop, iterations, savedFiles, logCallback, warningCallback, this);
   if (stat != 0) {
      WarningEvent *we = new WarningEvent("Error calling acquireIntmat()");
      postEvent( this, we);
      return;
   }

   GenerateEvent *e = new GenerateEvent(timestamp);
   postEvent(this, e);
   LogEvent *e2 = new LogEvent("Done");
   postEvent( this, e2);
   return;
}

void intmatAllGui::generateThread()
{
   int stat;
   LogEvent *e1 = new LogEvent("Generating disturbance commands....");
   postEvent( this, e1);

   int modesNum = atoi( editModesNum->text().latin1());
   int framesNum = atoi( editFramesNum->text().latin1());
   int cyclesNum = atoi( editCyclesNum->text().latin1());
   string m2c = comboM2C->currentText();
   string ampEnvelopeFile = Paths::AmpEnvelopeFile( m2c);
   string ampFile = Paths::ModesAmpDir( m2c) + comboAmpFile->currentText().latin1();
   string newAmpFile = Paths::ModesAmpDir( m2c) + comboAmpFile->currentText().latin1() + "_fixed.fits";
   
   string timestamp = generatePPDisturb( modesNum, framesNum, cyclesNum, m2c, ampFile, ampEnvelopeFile);

   if (timestamp == "") {
      WarningEvent *e = new WarningEvent("Error generating disturbance file");
      postEvent(this, e);
      return;
   }


   if (!checkOptimizeAmp->isChecked()) {
      GenerateEvent *e = new GenerateEvent(timestamp);
      postEvent(this, e);
      LogEvent *e2 = new LogEvent("Done");
      postEvent( this, e2);
      return;
   }

   // Acquire a test intmat to check the amplitude
   string tmpDir = "/tmp";
   vector<string> savedFiles;

   stat = acquireIntmat( _wfsIntf, _adsecIntf, m2c, timestamp, tmpDir, false, 1, savedFiles, logCallback, warningCallback, this);
   if (stat != 0) {
      WarningEvent *we = new WarningEvent("Error calling acquireIntmat()");
      postEvent( this, we);
      return;
   }
   stat = testAmplitude( tmpDir, ampFile, newAmpFile, 2, 2, modesNum, warningCallback, this);
   if (stat != 0) {
      WarningEvent *we = new WarningEvent("Error calling testAmplitude()");
      postEvent( this, we);
      return;
   }

   // Ri-generate disturbance

   timestamp = generatePPDisturb( modesNum, framesNum, cyclesNum, m2c, newAmpFile, ampEnvelopeFile);

   if (timestamp == "") {
      WarningEvent *e = new WarningEvent("Error generating disturbance file");
      postEvent(this, e);
      return;
   }

   AcquisitionEvent *e = new AcquisitionEvent(timestamp);
   postEvent(this, e);
   LogEvent *e2 = new LogEvent("Done");
   postEvent( this, e2);
   return;

}





void intmatAllGui::spinBoxIterations_valueChanged( int /* value */)
{
   recalcLen();
}

int intmatAllGui::recalcLen()
{
   /*
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
   */
   return 0;
}


void intmatAllGui::Run() {
   /* Does nothing but does not block! */
   printf("intmatDisturbGui::Run()\n");

}

void intmatAllGui::run() {

   printf("intmatDisturbGui::run()\n");
   while (!TimeToDie) {
      msleep(1000);
   }

}

void intmatAllGui::customEvent( QCustomEvent *e) {

   string s;
   LogEvent *logEvent;
   SaveFileEvent *saveFileEvent;
   WarningEvent *warningEvent;
   GenerateEvent *generateEvent;
   AcquisitionEvent *acquisitionEvent;

   switch(e->type()) {

      case LOG_EVENT_ID:
         logEvent = (LogEvent *)e;
         s = logEvent->getText();

         logText->append(s);
         break;

      case SAVEFILE_EVENT_ID:
         saveFileEvent = (SaveFileEvent *)e;
         s = saveFileEvent->getText();

         //labelFilename->setText(s);
         break;

      case WARNING_EVENT_ID:
         warningEvent = (WarningEvent *)e;
         QMessageBox::warning(this, "Warning", warningEvent->getText());
         break;

      case GENERATE_EVENT_ID:
         generateEvent = (GenerateEvent *)e;
         acceptModulation( generateEvent->getText());
         break;

      case ACQUISITION_EVENT_ID:
         acquisitionEvent = (AcquisitionEvent *)e;
         acceptAcquisition( acquisitionEvent->getText());
         break;

      default:
         break;
   }
}

void intmatAllGui::buttonAcquire_clicked()
{
    boost::thread* thAcquire = new boost::thread(boost::bind(&intmatAllGui::acquireThread, this));
    delete thAcquire;
}



static const KCmdLineOptions options[] =
{
      {"f <file>", ("Configuration file"), 0 },
      {"i <identity>", ("MsgD identity"), 0 },
         KCmdLineLastOption
};




int main( int argc, char *argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

   KCmdLineArgs::init( argc, argv, "intmatAllGui", "intmatAllGui", "intmatAllGui", "1.0" );
   KCmdLineArgs::addCmdLineOptions(options);
   KApplication kApp;

   intmatAllGui *gui = NULL;

   try {
      if (argc>1)
         gui = new intmatAllGui( argc, argv);
      else {
         gui = new intmatAllGui("intmatAllGui", AOApp::getConffile("intmatAllGui.conf"));
      }


      kApp.setMainWidget(gui);
      gui->Exec(true);

      gui->show();
      kApp.exec();

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
  

