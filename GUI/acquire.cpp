#define VERS_MAJOR   1
#define VERS_MINOR   0
 

#include "acquire.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qtextedit.h>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <kurlrequester.h>
#include <kapplication.h>
#include <kcmdlineargs.h>


#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
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
#include "base/timelib.h"
}


using namespace Arcetri;
using namespace Arcetri::IntmatLib;


int shortTimeout = 10*1000;
int longTimeout = 40*1000;

// If this is defined, no commands are sent to arbitrators except for status requests
#undef TEST_ONLY

acquire::acquire( string name, string configFile) : AOApp(name, configFile), QThread() {

}

acquire::acquire( int argc, char *argv[]) : AOApp(argc, argv), QThread() {

}

acquire::~acquire() {
   delete _wfsIntf;
   delete _adsecIntf;
}

void acquire::PostInit() {
   init();

   _wfsIntf = new ArbitratorInterface( "wfsarb."+AOApp::Side(), Logger::LOG_LEV_DEBUG);
   _adsecIntf = new ArbitratorInterface(  Utils::getClientname("adsecarb", true, "ADSEC"), Logger::LOG_LEV_DEBUG);

}

void acquire::init() {

}

void acquire::Run() {
   /* Does nothing but does not block! */
   printf("intmatDisturbGui::Run()\n");

}

void acquire::run() {

   printf("intmatDisturbGui::run()\n");
   while (!TimeToDie) {
      msleep(1000);
   }

}

void logCallback( void *argp, string msg)
{
   acquire *gui = (acquire *)argp;
   LogEvent *e = new LogEvent(msg);
   gui->postEvent( gui, e);
}

void warningCallback( void *argp, string msg)
{
   acquire *gui = (acquire *)argp;
   WarningEvent *e = new WarningEvent(msg);
   gui->postEvent( gui, e);
}

void logCallback( string msg, void *argp)
{
   acquire *gui = (acquire *)argp;
   LogEvent *e = new LogEvent(msg);
   gui->postEvent( gui, e);
}

void warningCallback( string msg, void *argp)
{
   acquire *gui = (acquire *)argp;
   WarningEvent *e = new WarningEvent(msg);
   gui->postEvent( gui, e);
}



void acquire::customEvent( QCustomEvent *e) {

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

         //labelFilename->setText(s);
         break;

      case WARNING_EVENT_ID:
         warningEvent = (WarningEvent *)e;
         QMessageBox::warning(this, "Warning", warningEvent->getText());
         break;

      default:
         break;
   }
}

void acquire::buttonAcquire_clicked()
{
    boost::thread* thAcquire = new boost::thread(boost::bind(&acquire::acquireThread, this));
    delete thAcquire;
}


void acquire::acquireThread()
{ 
   // Command *reply;

   string cmdDisturbFile = kURLinputFile->url().latin1();

   // int shortTimeout = 10*1000;
   // int longTimeout = 40*1000;

   LogEvent *e;
   // Command *cmd;

   ifstream ifs(cmdDisturbFile.c_str());

   char line[1024];
   // int count=0;
   while( ifs.good()) {

         ifs.getline(line, 1023);

         string buf; // Have a buffer string
         stringstream ss(line); // Insert the string into a stream

         vector<string> tokens; // Create vector to hold our words
         while (ss >> buf) {
            printf("Splitted stream: %s\n", buf.c_str());
            tokens.push_back(buf);
         }

         if (tokens.size() >0) {

            // Skip comments
            if (tokens[0].substr(0,1).compare("#") == 0)
               continue;

            // Autocenter/focus command
            if (tokens[0].compare("autocenter") == 0) {
               e = new LogEvent("Autocenter/focus....");
               postEvent(this, e);

               system("/usr/local/adopt/bin/runCenterPupils.py -stages -center");
               system("/usr/local/adopt/bin/runCenterPupils.py -center -focus");
               continue;
            }

  
            // Change params command
            // Syntax: params <binning> <loop frequency> <modulation>

            if (tokens[0].compare("params") == 0) {

                int binning =  atoi(tokens[1].c_str());
                float freq =  atof(tokens[2].c_str());
                float mod = atof(tokens[3].c_str());

              ostringstream oss;
              oss << "Setting parameters: binning " << binning << ", freq " << freq << ", modulation " << mod;
              e = new LogEvent(oss.str());
              postEvent(this, e);

              int stat= setLoopFreq( _wfsIntf, binning, freq, mod);
              if (stat != NO_ERROR) {
                    WarningEvent *we = new WarningEvent("SetLoopFreq command failed");
                    postEvent( this, we);
                    return;
                    }
              
              continue;
            }

            // Intmat_optimize command
            // intmat_optimize <loop freq> <nmodes> <iterations> <m2c name> <amplitude file>
           if (tokens[0].compare("intmat_optimize") == 0) {

               float freq = atof(tokens[1].c_str());
               int nmodes = atoi(tokens[2].c_str());
               int iterations = atoi(tokens[3].c_str());
               string m2c = tokens[4];
               string ampFilename = tokens[5];

               string ampEnvelopeFile = Paths::AmpEnvelopeFile( m2c);
               string newAmpFile = Paths::ModesAmpDir( m2c) + ampFilename + "_fixed.fits";
               string ampFile = Paths::ModesAmpDir( m2c) + ampFilename;


               string disturb_tracknum = generatePPDisturb( nmodes, 5, 2, m2c, ampFile, ampEnvelopeFile);

               string disturb_filename = "CmdDisturb_"+disturb_tracknum+".fits";

               string tracknum = acquireIntmat_old( _wfsIntf, _adsecIntf, freq, m2c, disturb_filename, 1, logCallback,warningCallback, this );

               string intmatAcqDir = Paths::IntmatAcqDir(  m2c) + tracknum;
               string intmatFile = Paths::RECsDir(  m2c) + "Intmat_" + disturb_tracknum + ".fits";

               // Optimize amplitude
               int stat = testAmplitude( intmatAcqDir, ampFile, newAmpFile, 2, 2, nmodes, warningCallback, this);
               if (stat != 0) {
                    WarningEvent *we = new WarningEvent("Error calling testAmplitude()");
                    postEvent( this, we);
                    return;
               }

              // Ri-generate disturbance
              string timestamp = generatePPDisturb( nmodes, 5, 2, m2c, newAmpFile, ampEnvelopeFile);
              if (timestamp == "") {
                 WarningEvent *e = new WarningEvent("Error generating disturbance file");
                 postEvent(this, e);
                 return;
              }
              disturb_filename = "CmdDisturb_"+timestamp+".fits";


              // Re-acquire intmat
              tracknum = acquireIntmat_old( _wfsIntf, _adsecIntf, freq, m2c, disturb_filename, iterations, logCallback,warningCallback, this );



              continue;
           }
           

            // Intmat command
            // intmat <loop freq> <iterations> <m2c name> <disturb name>
           if (tokens[0].compare("intmat") == 0) {

               float freq = atof(tokens[1].c_str());
               int iterations = atoi(tokens[2].c_str());
               string m2c = tokens[3];
               string disturb_filename = tokens[4];

               string tracknum = acquireIntmat_old( _wfsIntf, _adsecIntf, freq, m2c, disturb_filename, iterations, logCallback,warningCallback, this );

               continue;
           }
           
           if (tokens[0].compare("cloop") == 0) {

              // int nFrames = atoi(tokens[1].c_str());

              //string tracknum = doCloop( nFrames);
              continue;
           }


         // Old compatibility with sinusoidal matrix acquisition

         if (tokens.size() != 4)
            continue;

         int seconds = atoi(tokens[0].c_str());
         int nframes = atoi(tokens[1].c_str());
         string disturb = tokens[2];
         string outfile = tokens[3];

         printf("Accepted data: disturb %s, seconds %d, outfile %s\n", disturb.c_str(), seconds, outfile.c_str());

         sinus_acquire( _wfsIntf, _adsecIntf, seconds, nframes, disturb, outfile, logCallback, warningCallback, this);

      } // if (tokens.size == 4)

   }  // While( ifs.good())

   e = new LogEvent("Acquisition done");
   postEvent(this, e);

}


/*
int acquire::sendCommandWithReply( ArbitratorInterface *intf, Command* cmd, Command **reply) {

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
*/




static const KCmdLineOptions options[] =
{
      {"f <file>", ("Configuration file"), 0 },
      {"i <identity>", ("MsgD identity"), 0 },
         KCmdLineLastOption
};




int main( int argc, char *argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

   KCmdLineArgs::init( argc, argv, "acquire", "acquire", "acquire", "1.0" );
   KCmdLineArgs::addCmdLineOptions(options);
   KApplication kApp;

   acquire *gui = NULL;

   try {
      if (argc>1)
         gui = new acquire( argc, argv);
      else {
         gui = new acquire("acquire", AOApp::getConffile("acquire"));
      }


      kApp.setMainWidget(gui);
      gui->Exec();

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
  




