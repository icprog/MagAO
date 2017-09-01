#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "OptLoopDiagnGui.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <kcmdlineargs.h>


#include "../OptLoopDiagnostic/OptLoopDiagnosticStructs.h"

#include "arblib/wfsArb/WfsInterfaceDefines.h"
#include "arblib/base/CommandsExport.h"
#include "arblib/wfsArb/WfsCommandsExport.h"

extern "C" {
#include "base/timelib.h"
}


OptLoopDiagnGui::OptLoopDiagnGui( string name, string configFile, KApplication &kApp) : AOAppGui(name, configFile, kApp), OptLoopDiagn() {

}

OptLoopDiagnGui::OptLoopDiagnGui( int argc, char *argv[], KApplication &kApp) : AOAppGui(argc, argv, kApp), OptLoopDiagn() {

}

OptLoopDiagnGui::~OptLoopDiagnGui() {
   delete _wfsIntf;
}

void OptLoopDiagnGui::PostInit() {
   _wfsIntf = new ArbitratorInterface( "wfsarb."+AOApp::Side(), Logger::LOG_LEV_DEBUG);
   checkSavePsf_clicked();
   _saving=false;
}

void OptLoopDiagnGui::InstallHandlers() {

   AOAppGui::InstallHandlers();

   int stat;;
   _logger->log(Logger::LOG_LEV_INFO, "Installing custom handlers ...");
   if((stat=thHandler(OPTSAVE_REPLY, (char *)"*", 0, optsavereply_handler, (char *)"optsavereply_handler", this))<0) {
      _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
      SetTimeToDie(true);
   }
}

int OptLoopDiagnGui::optsavereply_handler(MsgBuf *msgb, void *argp, int /* hndlrQueueSize */) {

    OptLoopDiagnGui* myself = (OptLoopDiagnGui*)argp;

    ReplyEvent *e = new ReplyEvent(* ((optsave_reply *) MSG_BODY(msgb)));
    postEvent(myself, e);

    return NO_ERROR;
}

void OptLoopDiagnGui::customEvent( QCustomEvent *e) {

   ReplyEvent *replyEvent;
   WarningEvent *warningEvent;
   SaveFileEvent *saveFileEvent;
   optsave_reply reply;
   ostringstream oss;
   string s;

   switch(e->type()) {

      case REPLY_EVENT_ID:
      replyEvent = (ReplyEvent *)e;
      reply = replyEvent->getReply();

      s = reply.outfile;
      if (s.size()>15)
         s = s.substr( s.size()-16, 15);

      labelFilename->setText( s);

      oss << "Frames saved: " << reply.saved << "  Speed: " << reply.speed << " Hz";
      labelSpeed->setText(oss.str());
      break;

      case SAVEFILE_EVENT_ID:
      saveFileEvent = (SaveFileEvent *)e;
      s = saveFileEvent->getText();

      labelFilename->setText("Saved: "+s);
      labelFilename->setPaletteForegroundColor( QColor( qRgb(0,128,0)));
      break;


      case WARNING_EVENT_ID:
      warningEvent = (WarningEvent *)e;
      QMessageBox::warning(this, "Warning", warningEvent->getText());
      labelFilename->setText("Error");
      labelFilename->setPaletteForegroundColor( QColor( qRgb(255,0,0)));
      break;

      default:
      break;
   }
}


void OptLoopDiagnGui::checkSaveCcd47_clicked() {

   if (checkSaveCcd47->isChecked())
      editCcd47Num->setEnabled(true);
   else
      editCcd47Num->setDisabled(true);
}

void OptLoopDiagnGui::checkSaveIrtc_clicked() {

   if (checkSaveIrtc->isChecked())
      editIrtcNum->setEnabled(true);
   else
      editIrtcNum->setDisabled(true);
}

void OptLoopDiagnGui::checkSavePisces_clicked() {

   if (checkSavePisces->isChecked())
      editPiscesNum->setEnabled(true);
   else
      editPiscesNum->setDisabled(true);
}


void OptLoopDiagnGui::buttonSave_clicked() {

   if (_saving)
      return;

   if (checkSaveIrtc->isChecked())
   	QMessageBox::warning(this, "Turn off IRTC live", "Remember to turn off the LIVE button on IRTC");

   int nframes = atoi( editFramesNum->text().latin1());
   if (nframes <=1) {
   	QMessageBox::warning(this, "No frames to save", "Cannot save less than 2 frames");
        return;
	}


   _saving=true;
   labelFilename->setText("Saving...");
   labelFilename->setPaletteForegroundColor( QColor( qRgb(128,128,128)));

   boost::thread* thSave = new boost::thread(boost::bind(&OptLoopDiagnGui::saveThread, this));
   delete thSave;
}

void OptLoopDiagnGui::saveThread()
{
   Wfs_Arbitrator::saveOptLoopDataParams params;

   params._nFrames = atoi( editFramesNum->text().latin1());
   if (params._nFrames<1)
      params._nFrames=1;
   params._saveFrames =  checkSaveFrames->isChecked() ? true : false;
   params._saveSlopes = checkSaveSlopes->isChecked() ? true : false;
   params._saveModes  = checkSaveModes->isChecked() ? true : false;
   params._saveCommands = checkSaveCommands->isChecked() ? true : false;
   params._savePositions = checkSavePositions->isChecked() ? true : false;
   params._savePsf = checkSaveCcd47->isChecked() ? true : false;
   params._saveIrtc = checkSaveIrtc->isChecked() ? true : false;
   params._savePisces = checkSavePisces->isChecked() ? true : false;
   if (checkSaveCcd47->isChecked())
      params._nFramesPsf = atoi( editCcd47Num->text().latin1());
   if (checkSaveIrtc->isChecked())
      params._nFramesIrtc = atoi( editIrtcNum->text().latin1());
   if (checkSavePisces->isChecked())
      params._nFramesPisces = atoi( editPiscesNum->text().latin1());

   int timeout = 1000*1000;
   Command *cmd = new Wfs_Arbitrator::SaveOptLoopData( timeout, params);

   Command *reply=NULL;
   try {
      reply = _wfsIntf->requestCommand(cmd);
   } catch (ArbitratorInterfaceException &e) {
      WarningEvent *we = new WarningEvent(e.what());
      postEvent(this, we);
      _saving = false;
   }

   if (!reply) {
      WarningEvent *e = new WarningEvent("Error saving optical loop data");
      postEvent(this, e);
   }
   else if (! reply->isSuccess()) {
      WarningEvent *e = new WarningEvent("Error saving optical loop data");
      postEvent(this, e);
      reply->log();
   }
   else {
      SaveFileEvent *e = new SaveFileEvent( ((Wfs_Arbitrator::SaveOptLoopData *)reply)->getParams()._trackNum);
      postEvent(this, e);
      reply->log();
   }
      

   _saving = false;
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

   OptLoopDiagnGui *optgui = NULL;

   try {
      if (argc>1)
         optgui = new OptLoopDiagnGui( argc, argv, kApp);
      else {
         optgui = new OptLoopDiagnGui("optloopdiagngui", AOApp::getConffile("optloopdiagngui"), kApp);
      }


      optgui->doGui(optgui);

      delete optgui;
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
  

