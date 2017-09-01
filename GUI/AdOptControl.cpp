#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "AdOptControl.h"

#include "arblib/base/CommandsExport.h"
#include "arblib/base/AlertsExport.h"
#include "arblib/aoArb/AOCommandsExport.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qtextedit.h>
#include <qprogressbar.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <knuminput.h>
#include <kled.h>

#include <map>


using namespace Arcetri;
using namespace Arcetri::Arbitrator;

map<string, string> wfsState2Disp;
map<string, string> adsecState2Disp;
map<string, QColor> wfsState2Color;
map<string, QColor> adsecState2Color;

RTDBvar AdOptControl::_aoSubstate;
RTDBvar AdOptControl::_wfsSource;
RTDBvar AdOptControl::_globalProgress;

AdOptControl::AdOptControl(string name, string configFile, KApplication &kApp): AdOptControlGui(), AbstractArbControl(name, configFile, kApp) {
	init();
}


AdOptControl::AdOptControl(int argc, char* argv[], KApplication &kApp): AdOptControlGui(), AbstractArbControl(argc, argv, kApp) {
	init();
}


AdOptControl::~AdOptControl() {

}

void AdOptControl::init() {

   qobject=this;

   _wfsOn = false;
   _adSecOn = false;
   _adsecStatus = "";
   _wfsStatus = "";
   _aoMode = "";
   _arbStatus = "";
   pthread_mutex_init( &_updateStatusMutex, NULL);

   string logfileWfs = Utils::getAdoptLog() + "/adoptcontrol."+Utils::getAdoptSide()+".log";
   followLogFile(logfileWfs, this);

   string logfileAdSec = Utils::getAdoptLog() + "/adsecarb."+Utils::getAdoptSide()+".log";
   followLogFile2(logfileAdSec, this);

   wfsState2Disp["Disconnected"]    = "---------";
   wfsState2Disp["Failure"]    = "FAILURE";
   wfsState2Disp["Panic"]      = "PANIC";
   wfsState2Disp["PowerOff" ]  = "OFF";
   wfsState2Disp["Operating"]  = "OPERATING";
   wfsState2Disp["AOPrepared"] = "OPERATING";
   wfsState2Disp["AOSet"]      = "OPERATING";
   wfsState2Disp["LoopClosed"] = "OPERATING";
   wfsState2Disp["LoopPaused"] = "OPERATING";

   wfsState2Color["Disconnected" ]  = QColor( qRgb( 192, 192, 192));
   wfsState2Color["Failure"]    = QColor( qRgb( 255, 0, 0));
   wfsState2Color["Panic"]      = QColor( qRgb( 255, 0, 0)); 
   wfsState2Color["PowerOff" ]  = QColor( qRgb( 192, 192, 192));
   wfsState2Color["Operating"]  = QColor( qRgb( 0, 255, 0));
   wfsState2Color["AOPrepared"] = QColor( qRgb( 0, 255, 0));
   wfsState2Color["AOSet"]      = QColor( qRgb( 0, 255, 0));
   wfsState2Color["LoopClosed"] = QColor( qRgb( 0, 255, 0));
   wfsState2Color["LoopPaused"] = QColor( qRgb( 0, 255, 0));

   adsecState2Disp["Disconnected"]    = "---------";
   adsecState2Disp["PowerOff"]    = "OFF";
   adsecState2Disp["PowerOn"]     = "POWER ON";
   adsecState2Disp["Ready"]       = "READY";
   adsecState2Disp["ChopRunning"] = "MIRROR FLAT";
   adsecState2Disp["AORunning"]   = "MIRROR FLAT";
   adsecState2Disp["AOPause"]     = "MIRROR FLAT";
   adsecState2Disp["ChopSet"]     = "MIRROR FLAT";
   adsecState2Disp["AOSet"]       = "MIRROR FLAT";
   adsecState2Disp["SLSet"]       = "MIRROR FLAT";
   adsecState2Disp["Panic"]       = "PANIC";
   adsecState2Disp["Failure"]     = "FAILURE";

   adsecState2Color["Disconnected" ]  = QColor( qRgb( 192, 192, 192));
   adsecState2Color["PowerOff"]    = QColor( qRgb( 192, 192, 192));
   adsecState2Color["PowerOn"]     = QColor( qRgb( 192, 192, 192));
   adsecState2Color["Ready"]       = QColor( qRgb( 192, 192, 192));
   adsecState2Color["ChopRunning"] = QColor( qRgb( 0, 255, 0));
   adsecState2Color["AORunning"]   = QColor( qRgb( 0, 255, 0));
   adsecState2Color["AOPause"]     = QColor( qRgb( 0, 255, 0));
   adsecState2Color["ChopSet"]     = QColor( qRgb( 0, 255, 0));
   adsecState2Color["AOSet"]       = QColor( qRgb( 0, 255, 0));
   adsecState2Color["SLSet"]       = QColor( qRgb( 0, 255, 0));
   adsecState2Color["Panic"]       = QColor( qRgb( 255, 0, 0));
   adsecState2Color["Failure"]     = QColor( qRgb( 255, 0, 0));

}


void AdOptControl::slotDataAvail() {
   if (!_tailProc)
      return;

   while (_tailProc->canReadLineStdout())
      logWfs->append( _tailProc->readLineStdout().latin1());
}


void AdOptControl::slotDataAvail2() {
   if (!_tailProc2)
      return;

   while (_tailProc2->canReadLineStdout())
      logAdsec->append( _tailProc2->readLineStdout().latin1());
}

void AdOptControl::customEvent(QCustomEvent *e) {

   switch(e->type()) {

          default:
            break;
    }

  // Call base class method
  AbstractArbControl::customEvent(e);
}



void AdOptControl::setArbitratorStatus(string arbStatus, string lastCmdDescr, string lastCmdResult) {
	aoArbitratorStatusLabel->setText(arbStatus);
   _arbStatus = arbStatus;
	lastCommandLabel->setText(lastCmdDescr);
	lastCommandResultLabel->setText(lastCmdResult);
    _aoSubstate.Update();
    aoArbitratorSubStatusLabel->setText(_aoSubstate.Get());
    _wfsSource.Update();
    labelWfsSource->setText(_wfsSource.Get());
    progressBar->setProgress(atoi(_globalProgress.Get().c_str()));
    enableGui( (lastCmdResult.size()>0) && (lastCmdResult != "EXECUTING"));
}


void AdOptControl::SetupVarsChild() {

    try {
        _aoSubstate = RTDBvar( "AOARB."+Side()+".FSM_SUBSTATE", CHAR_VARIABLE, 50);
        _wfsSource = RTDBvar( "AOARB."+Side()+".WFS_SOURCE", CHAR_VARIABLE, 50);
        _globalProgress = RTDBvar( "AOARB."+Side()+".PROGRESS", CHAR_VARIABLE, 50);
        Notify(_globalProgress, OnNotify);
    } catch (AOException &e) {
        _logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
        throw(e);
    }
}

void AdOptControl::enableGui(bool enable) {

   // Lock this because we are using variables updated by updateArbStatus()
   pthread_mutex_lock( &_updateStatusMutex);

   AOModeLabel->setText( _aoMode);

   labelAdSecState->setText( "FSM state: " + _adsecStatus);
   labelWfsState->setText( "FSM state: " + _wfsStatus);

   labelWfsReadyHW->setText( wfsState2Disp[_wfsStatus]);
   labelWfsReadyHW->setEraseColor( wfsState2Color[_wfsStatus]);

   labelAdSecReadyHW->setText( adsecState2Disp[_adsecStatus]);
   labelAdSecReadyHW->setEraseColor( adsecState2Color[_adsecStatus]);

   labelWfsReadySW->setText( _wfsSoftware ? "UP" : "DOWN");
   labelWfsReadySW->setEraseColor( _wfsSoftware ? QColor(qRgb(0, 255, 0)) : QColor(qRgb(255, 0, 0)));

   labelAdSecReadySW->setText( _adsecSoftware ? "UP" : "DOWN");
   labelAdSecReadySW->setEraseColor( _adsecSoftware ? QColor(qRgb(0, 255, 0)) : QColor(qRgb(255, 0, 0)));


   buttonPowerOnAdSec->setEnabled(false); 
   buttonPowerOffAdSec->setEnabled(false); 
   buttonPowerOnWfs->setEnabled(false); 
   buttonPowerOffWfs->setEnabled(false); 
   buttonAdSecSwStart->setEnabled(false);
   buttonAdSecSwStop->setEnabled(false);
   buttonWfsSwStart->setEnabled(false);
   buttonWfsSwStop->setEnabled(false);
   buttonAdsecMirrorSet->setEnabled(false);
   buttonAdsecMirrorRest->setEnabled(false);


   if (_wfsOn) {
      buttonPowerOnWfs->setEnabled(false);
      buttonPowerOffWfs->setEnabled(enable);
      buttonWfsSwStop->setEnabled(false);
      buttonWfsSwStart->setEnabled(false);
   }
   else {
      buttonPowerOnWfs->setEnabled(enable);
      buttonPowerOffWfs->setEnabled(false);
      if (_wfsSoftware) {
         buttonWfsSwStop->setEnabled(true);
         buttonWfsSwStart->setEnabled(false);
      } else {
         buttonWfsSwStop->setEnabled(false);
         buttonWfsSwStart->setEnabled(true);
      }
   }

   if (_adSecOn) {
      buttonPowerOnAdSec->setEnabled(false);
      buttonPowerOffAdSec->setEnabled(enable);
      buttonAdSecSwStart->setEnabled(false);
      buttonAdSecSwStop->setEnabled(false);
      if (_adSecSet) {
         buttonAdsecMirrorSet->setEnabled(false);
         buttonAdsecMirrorRest->setEnabled(true);
      } else {
         buttonAdsecMirrorSet->setEnabled(true);
         buttonAdsecMirrorRest->setEnabled(false);
      }

   }
   else {
      buttonPowerOnAdSec->setEnabled(true);
      buttonPowerOffAdSec->setEnabled(true);
      buttonAdsecMirrorSet->setEnabled(false);
      buttonAdsecMirrorRest->setEnabled(false);
      if (_adsecSoftware) {
         buttonAdSecSwStop->setEnabled(true);
         buttonAdSecSwStart->setEnabled(false);
      } else {
         buttonAdSecSwStop->setEnabled(false);
         buttonAdSecSwStart->setEnabled(true);
      }
   }


   if (_wfsStatus == "Failure")
      buttonRecoverWfsFailure->setEnabled(enable);
   else
      buttonRecoverWfsFailure->setEnabled(false);

   if (_adsecStatus == "Failure")
      buttonRecoverAdSecFailure->setEnabled(true);
   else
      buttonRecoverAdSecFailure->setEnabled(false);

   if (_arbStatus == "Failure")
      buttonRecoverAdSecFailure->setEnabled(true);


   pthread_mutex_unlock( &_updateStatusMutex);

}


int AdOptControl::OnNotify(void* thisPtr, Variable* var) {

    _globalProgress.Update();

    return NO_ERROR;
}

void AdOptControl::setMessage(string text) {
	if(text.size() > 50) {
		messageLabel->setText(text.substr(0,50) + "...");
	}
	else {
		messageLabel->setText(text);
	}
}

void AdOptControl::buttonPowerOnAdSec_clicked() {
    Command *cmd = new PowerOnAdSec(getLongTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);	// This will delete cmd when thread is finished!
}

void AdOptControl::buttonPowerOffAdSec_clicked() {
    Command *cmd = new PowerOffAdSec(getLongTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);	// This will delete cmd when thread is finished!
}

void AdOptControl::buttonAdsecMirrorSet_clicked() {
    Command *cmd = new MirrorSet(getLongTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);	// This will delete cmd when thread is finished!
}

void AdOptControl::buttonAdsecMirrorRest_clicked() {
   Command *cmd = new MirrorRest(getLongTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);	// This will delete cmd when thread is finished!
}
void AdOptControl::buttonPowerOnWfs_clicked() {
    wfsOnOffParams wfsid = {""}; // new, should get the wfsid id from somewhere
    Command *cmd = new PowerOnWfs(getLongTimeout(), wfsid, _globalProgress.complete_name());
	sendCommandThread(cmd);	// This will delete cmd when thread is finished!
}

void AdOptControl::buttonPowerOffWfs_clicked() {
    wfsOnOffParams wfsid = {""}; // new, should get the wfsid id from somewhere
    Command *cmd = new PowerOffWfs(getLongTimeout(), wfsid, _globalProgress.complete_name());
	sendCommandThread(cmd);	// This will delete cmd when thread is finished!
}

void AdOptControl::buttonRecoverAdSecFailure_clicked() {
   Command *cmd = new FaultRecovery(getLongTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);	// This will delete cmd when thread is finished!
}

void AdOptControl::buttonRecoverWfsFailure_clicked() {
   Command *cmd = new RecoverWfsFailure(getLongTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);	// This will delete cmd when thread is finished!
}

void AdOptControl::buttonAdSecSwStart_clicked() {
   string cmd = (std::string) ConfigDictionary()["AdSecStartCmd"];
   ex(cmd);
}

void AdOptControl::buttonAdSecSwStop_clicked() {
   string cmd = (std::string) ConfigDictionary()["AdSecStopCmd"];
   ex(cmd);
}

void AdOptControl::buttonWfsSwStart_clicked() {
   string cmd = (std::string) ConfigDictionary()["WfsStartCmd"];
   ex(cmd);
}

void AdOptControl::buttonWfsSwStop_clicked() {
   string cmd = (std::string) ConfigDictionary()["WfsStopCmd"];
   ex(cmd);
}

void AdOptControl::ex(string cmd) {

   cmd = "xterm -e \"" + cmd + "\"";
   int ret = system(cmd.c_str());
   if (ret != 0) {
      QMessageBox::warning( this, "Error", "Error executing command");
   }

}

void AdOptControl::updateArbStatus() {

   // Can be called from different threads!!
   pthread_mutex_lock( &_updateStatusMutex);


   _wfsOn = false;
   _adSecOn = false;
   _adSecSet = false;
   _wfsStatus = "Disconnected";
   _adsecStatus = "Disconnected";
   _wfsSoftware = false;
   _adsecSoftware = false;

   try {

      RequestAOStatus  arbStatus(150);
      RequestAOStatus  *arbStatusReply = NULL;

      updateArbStatusInternal( &arbStatus, (RequestStatus **)&arbStatusReply);
      

      if (arbStatusReply) {
         _wfsOn = arbStatusReply->wfsPower();
         _adSecOn = arbStatusReply->adsecPower();
         _adSecSet = arbStatusReply->adsecSet();
         _wfsStatus = arbStatusReply->wfsStatus();
         _adsecStatus = arbStatusReply->adsecStatus();
         _wfsSoftware = arbStatusReply->wfsClStatReady();
         _adsecSoftware = arbStatusReply->adsecClStatReady();
         _aoMode = arbStatusReply->aoMode();
//       _arbStatus = arbStatus;

         delete arbStatusReply;
      } else {
         pthread_mutex_unlock( &_updateStatusMutex);
         throw AOException("UpdateStatus()");
      }

   } catch (AOException &e) {
		Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error in updateArbStatus: %s", e.what().c_str());
      pthread_mutex_unlock( &_updateStatusMutex);
      throw;
   } catch (...) {
      pthread_mutex_unlock( &_updateStatusMutex);
      throw;
   }

   pthread_mutex_unlock( &_updateStatusMutex);

}


void *connectThread( void *argp) {

   AdOptControl *app = (AdOptControl*)argp;
   while (1) {
      app->Exec(true);
      thCleanup();
      msleep(2000);
      app->SetTimeToDie(false);
   }
}

void AdOptControl::processReply( Command *reply) {

   if (reply->getStatus() != Command::CMD_EXECUTION_SUCCESS) {
      ostringstream oss;
      oss << "Command " << reply->getDescription() << " failed with status " << reply->getStatusAsString()  << "\n" << reply->getErrorString();
      postAlert( oss.str());
      return;
      }
}

static const KCmdLineOptions options[] =
{
    {"f <file>", ("Configuration file"), 0 },
    {"i <identity>", ("MsgD identity"), 0 },
    KCmdLineLastOption
};

int main(int argc, char* argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

   KCmdLineArgs::init( argc, argv, "AdOptControl", "AdOptControl", "AdOptControl", "1.0" );

   KCmdLineArgs::addCmdLineOptions(options);
   KApplication a;
   AdOptControl* adOptControl = NULL;

   pthread_attr_t thrdAttribute;
   pthread_t myth;

	try {
		if(argc > 1) {
			adOptControl = new AdOptControl(argc, argv, a);
		}
		else {
			adOptControl = new AdOptControl("adoptcontrol", AOApp::getConffile("adoptcontrol"), a);
		}

		adOptControl->doGui(adOptControl, true, true);

		// Stop the AOApp 
		adOptControl->die();

		delete adOptControl;
		return 0;

	}
	catch (LoggerFatalException &e) {
		// In this case the logger can't log!!!
		 printf("%s\n", e.what().c_str());
	}
	catch (AOException &e) {
		Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error in AdOptControl: %s", e.what().c_str());
	}

   return -1;
}
