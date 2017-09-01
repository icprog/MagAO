
#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "WfsControl.h"

#include "arblib/base/CommandsExport.h"
#include "arblib/base/AlertsExport.h"
#include "arblib/wfsArb/WfsCommandsExport.h"

#include "Paths.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qmessagebox.h>
#include <qradiobutton.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <knuminput.h>

#include "dirent.h"
#include <algorithm>
#include <math.h>     // fabs()

#define DRIFT_WARNING (5)

using namespace Arcetri;
using namespace Arcetri::Arbitrator;
using namespace Arcetri::Wfs_Arbitrator;

extern resultColor colorTable[];

RTDBvar WfsControl::_VarDriftEnabledCur;
RTDBvar WfsControl::_VarDriftEnabledReq;
RTDBvar WfsControl::_VarDriftAmount;
RTDBvar WfsControl::_VarLensTrack;
RTDBvar WfsControl::_VarFastlinkEnabled;
RTDBvar WfsControl::_VarAutogainStop;;


WfsControl::WfsControl(string name, string configFile, KApplication &kApp): WfsControlGui(), AbstractArbControl(name, configFile, kApp) {
	init();
}


WfsControl::WfsControl(int argc, char* argv[], KApplication &kApp): WfsControlGui(), AbstractArbControl(argc, argv, kApp) {
	init();
}


WfsControl::~WfsControl() {

}

void WfsControl::init() {
   
   qobject = this;

   alertsText->setMaxLogLines(500);
   string logfile = Utils::getAdoptLog() + "/pyarg.log";
   followLogFile(logfile, this);

	// Populate operate modes
	vector<string> opModes = _arbConfig["OP_MODES"];
	vector<string>::iterator iter;
   configComboBox->clear();
	for(iter = opModes.begin(); iter != opModes.end(); iter++) {
		configComboBox->insertItem(*iter);
	}

   // Populate board setups
   string path = Paths::WfsSetupsDir();
   vector<string> setups = Paths::readDir(path, DT_REG);
   for(iter = setups.begin(); iter != setups.end(); iter++) {
      string disp = filename2display( path + "/" + (*iter), *iter);
      *iter = disp;
   }
   sort( setups.begin(), setups.end());
   for(iter = setups.begin(); iter != setups.end(); iter++)
      comboBoardSetup->insertItem( *iter);

   checkBoardSetup->setChecked(0);
   comboBoardSetup->setEnabled(false);
   comboBoardSetup->setCurrentItem( comboBoardSetup->count()-1);

   radioDisturbanceDisabled->setChecked(1);

   _fastlinkEnabled = 0;
   _lensTrackAlert = false;

}

void WfsControl::SetupVarsChild() {
    _logger->log(Logger::LOG_LEV_INFO, "Setting up custom rtdb variables ...");
    try {
       _VarDriftEnabledCur = RTDBvar( "optloopdiag."+Side()+".DRIFT.ENABLE.CUR");
       _VarDriftEnabledReq = RTDBvar( "optloopdiag."+Side()+".DRIFT.ENABLE.REQ");
       _VarDriftAmount     = RTDBvar( "optloopdiag."+Side()+".DRIFT.AMOUNT", REAL_VARIABLE, 1);
       _VarLensTrack      = RTDBvar( "cameralensctrl."+Side()+".TRACKING.CUR", INT_VARIABLE, 1);
       _VarFastlinkEnabled = RTDBvar( "slopecompctrl."+Side()+".FASTLINK.ENABLE.CUR", INT_VARIABLE, 1);
       _VarAutogainStop    = RTDBvar( "autogain."+Side()+".STOP", INT_VARIABLE, 1);

       Notify(_VarDriftEnabledCur, updateDrift);
       Notify(_VarDriftAmount, updateDrift);
       Notify(_VarFastlinkEnabled, updateFastlink, true);

       // Initial update
       updateDrift(dynamic_cast<AOApp *>(this), NULL);


    }  catch (AOException &e) {
       _logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
       throw(e);
    }
}

int WfsControl::updateFastlink( void* thisPtr, Variable* var) {
   WfsControl* mySelf = dynamic_cast<WfsControl*>((AOApp*) thisPtr);

   if (var)
      mySelf->_fastlinkEnabled = var->Value.Lv[0];

   mySelf->postRedisplayEvent();
   return NO_ERROR;
}

int WfsControl::updateDrift( void* thisPtr, Variable* var) {
   int v;
   double a;
   WfsControl* mySelf = dynamic_cast<WfsControl*>((AOApp*) thisPtr);

   if (var) {
      _VarDriftEnabledCur.MatchAndSet(var);
      _VarDriftAmount.MatchAndSet(var);
   }

   _VarDriftEnabledCur.Get(&v);
   mySelf->_driftEnabled = (v!=0);

   _VarDriftAmount.Get(&a);
   mySelf->_driftAmount = a;

   mySelf->postRedisplayEvent();
   return NO_ERROR;
}

void WfsControl::displayEnabled( QLabel *label, bool enabled, string enabledText, string disabledText, const QColor *enabledColor, const QColor *disabledColor) {

   if (!enabledColor)
	enabledColor = &Qt::green;
   if (!disabledColor)
	disabledColor = &Qt::lightGray;

   if (enabled) {
      label->setText(enabledText);
      label->setEraseColor( *enabledColor);
   }
   else {
      label->setText(disabledText);
      label->setEraseColor( *disabledColor);
   }

}

void WfsControl::redisplay() {

   const QColor *c;

   string driftStr= (_driftEnabled) ? "Enabled" : "Disabled";
   QColor driftCol = (_driftEnabled) ? Qt::green : Qt::lightGray;
   if (fabs(_driftAmount) > DRIFT_WARNING) {
      driftStr = "Warning";
      driftCol =Qt::yellow;
   }
   
   displayEnabled( labelDriftEnabled, _driftEnabled, driftStr, driftStr, &driftCol, &driftCol);

   string s = Utils::itoa(_driftAmount, 6, 1);
   labelDriftAmount->setText(s);

   displayEnabled( labelRerotTrackStatus, _rerotTrackEnabled);
   displayEnabled( labelAdcTrackStatus, _adcTrackEnabled);

   string lensStr="";
   if (_lensTrackAlert) {
      c = &Qt::yellow;
      lensStr = "Out of range";
   } else {
      c = &Qt::green;
      lensStr = "Enabled";
   }

   displayEnabled( labelLensTrackStatus, _lensTrackEnabled, lensStr, "Disabled", c);

   if (_arbStatus == "LoopClosed")
	c = &Qt::red;
   else
	c = &Qt::lightGray;

   string rangeStr = "";
   QColor targetCol = Qt::green;
   if (_lensTrackOutOfRange) {
       rangeStr = " / Rotation warning";
       targetCol = Qt::yellow;
       c = &Qt::yellow;
   } 

   displayEnabled( labelLensTrackTarget, _lensTrackTarget, "On target"+rangeStr, "Not on target"+rangeStr, &targetCol, c);

   if (_fastlinkEnabled) {
        loopStatusLabel->setText("Closed");
        loopStatusLabel->setEraseColor( QColor( qRgb(0,255,0)));
   }
   else {
        loopStatusLabel->setText("Open");
        loopStatusLabel->setEraseColor( QColor( qRgb(192,192,192)));
   }
}

void WfsControl::buttonRerotTrackOn_clicked() {
   autoTrackParams params;
   params.rerotTrack=true;
   params.adcTrack=_adcTrackEnabled;
   params.lensTrack=_lensTrackEnabled;
   AutoTrack *cmd = new AutoTrack( getShortTimeout(), params);
   sendCommandThread(cmd); // This will delete cmd when thread is finished!
}

void WfsControl::buttonRerotTrackOff_clicked() {
   autoTrackParams params;
   params.rerotTrack=false;
   params.adcTrack=_adcTrackEnabled;
   params.lensTrack=_lensTrackEnabled;
   AutoTrack *cmd = new AutoTrack( getShortTimeout(), params);
   sendCommandThread(cmd); // This will delete cmd when thread is finished!
}

void WfsControl::buttonADCTrackOn_clicked() {
   autoTrackParams params;
   params.rerotTrack=_rerotTrackEnabled;
   params.adcTrack=true;
   params.lensTrack=_lensTrackEnabled;
   AutoTrack *cmd = new AutoTrack( getShortTimeout(), params);
   sendCommandThread(cmd); // This will delete cmd when thread is finished!
}

void WfsControl::buttonADCTrackOff_clicked() {
   autoTrackParams params;
   params.rerotTrack=_rerotTrackEnabled;
   params.adcTrack=false;
   params.lensTrack=_lensTrackEnabled;
   AutoTrack *cmd = new AutoTrack( getShortTimeout(), params);
   sendCommandThread(cmd); // This will delete cmd when thread is finished!
}

void WfsControl::buttonLensTrackOn_clicked() {
   autoTrackParams params;
   params.rerotTrack=_rerotTrackEnabled;
   params.adcTrack=_adcTrackEnabled;
   params.lensTrack=true;
   AutoTrack *cmd = new AutoTrack( getShortTimeout(), params);
   sendCommandThread(cmd); // This will delete cmd when thread is finished!
}

void WfsControl::buttonLensTrackOff_clicked() {
   autoTrackParams params;
   params.rerotTrack=_rerotTrackEnabled;
   params.adcTrack=_adcTrackEnabled;
   params.lensTrack=false;
   AutoTrack *cmd = new AutoTrack( getShortTimeout(), params);
   sendCommandThread(cmd); // This will delete cmd when thread is finished!
}



string WfsControl::filename2display( string filename, string display_filename) {

   struct stat buf;
   stat( filename.c_str(), &buf);

   return Utils::printableDate( buf.st_mtime) + " " + display_filename;
}

string WfsControl::display2filename( string display) {
   return display.substr(11);
}



void WfsControl::setArbitratorStatus(string arbStatus, string lastCmdDescr, string lastCmdResult) {
	wfsArbitratorStatusLabel->setText(arbStatus);
	wfsArbitratorStatusLabel->setPaletteForegroundColor( QColor( qRgb(0,0,0)));
	lastCommandLabel->setText(lastCmdDescr);
	lastCommandResultLabel->setText(lastCmdResult);

   for (int i=0; colorTable[i].text[0] != '\0'; i++)
      if (colorTable[i].text == lastCmdResult)
         lastCommandResultLabel->setPaletteForegroundColor( QColor(colorTable[i].rgb));

   enableGui( (lastCmdResult.size()>0) && (lastCmdResult != "EXECUTING"), lastCmdDescr == "OptimizeGain");

}

void WfsControl::slotDataAvail() {
   if (!_tailProc)
      return;

   while (_tailProc->canReadLineStdout())
      logText( _tailProc->readLineStdout().latin1());
}

void WfsControl::logAlert(string log) {
        string s = Utils::loggerAsciiDateAndTime()+" Alert: "+log;
	alertsText->append(s);
        QMessageBox::warning( NULL, "Alert", log);
}

void WfsControl::logText(string log) {
	alertsText->append(log);
}


void WfsControl::enableGui(bool enable, bool enableStop) {

	closeLoopButton->setEnabled(enable);
	pauseLoopButton->setEnabled(enable);
	resumeLoopButton->setEnabled(enable);	
	stopLoopButton->setEnabled(enable);
	operateButton->setEnabled(false);
	offButton->setEnabled(false);
   buttonFreqApply->setEnabled(enable);
   buttonXYoffset->setEnabled(enable);
   buttonZoffset->setEnabled(enable);

	configComboBox->setEnabled(enable);
   
   calibrateHODarkButton->setEnabled(enable);
   calibrateTVDarkButton->setEnabled(enable);
   calibrateIRTCDarkButton->setEnabled(enable);
   calibrateMovementsButton->setEnabled(enable);
   calibrateSlopenullButton->setEnabled(enable);
   buttonGainOptimize->setEnabled(enable);

   buttonStopAutogain->setEnabled(enableStop && (!enable));

   radioDisturbanceDisabled->setEnabled(enable);
   radioDisturbanceEnabledWFS->setEnabled(enable);

   buttonAntiDriftOn->setEnabled(enable);
   buttonAntiDriftOff->setEnabled(enable);

   if ((enable) && (checkBoardSetup->isChecked()))
     comboBoardSetup->setEnabled(true);
   else
     comboBoardSetup->setEnabled(false);

   buttonRecoverFailure->setEnabled( (_arbStatus == "Failure") ? enable : false);


}


void WfsControl::buttonRecoverFailure_clicked() {
   powerOnParams params;

   // An empty Operate command will switch to Operating state without doing anything
   params.config = "";
   params.boardSetup = "";
   params.sourceSetup = "";
   params.opticalSetup = false;
   Operate* cmd = new Operate(getLongTimeout(), params);
   sendCommandThread(cmd); // This will delete cmd when thread is finished!
}


void WfsControl::setMessage(string text) {
	messageLabel->setText(text);
}

void WfsControl::setAsynchMessage(string text) {
	asynchMessageLabel->setText(text);
}

void WfsControl::buttonAntiDriftOn_clicked() {
   antiDriftParams params;
   params.enable=true;
   AntiDrift *cmd = new AntiDrift( getShortTimeout(), params);
   sendCommandThread(cmd); // This will delete cmd when thread is finished!
}

void WfsControl::buttonGainOptimize_clicked() {
   OptimizeGain *cmd = new OptimizeGain( getLongTimeout());
   sendCommandThread(cmd); // This will delete cmd when thread is finished!
}

void WfsControl::buttonAntiDriftOff_clicked() {
   antiDriftParams params;
   params.enable=false;
   AntiDrift *cmd = new AntiDrift( getShortTimeout(), params);
   sendCommandThread(cmd); // This will delete cmd when thread is finished!
}

void WfsControl::checkBoardSetup_clicked() {
   if (checkBoardSetup->isChecked())
      comboBoardSetup->setEnabled(true);
   else
      comboBoardSetup->setEnabled(false);
}

void WfsControl::operateButton_clicked() {

   int ret = QMessageBox::question( NULL, "Are you sure?", "Confirm power on of WFS unit?", QMessageBox::Yes, QMessageBox::No);
   if (ret != QMessageBox::Yes)
            return;

   powerOnParams params;
   string s = configComboBox->currentText().latin1();
   params.config = s;
   if (checkBoardSetup->isChecked())
      params.boardSetup = display2filename( comboBoardSetup->currentText().latin1());
   else
      params.boardSetup = "";
   params.opticalSetup  = false;
   params.sourceSetup = "CALIBRATION";

	Operate* cmd = new Operate(getLongTimeout(), params);
	sendCommandThread(cmd);	// This will delete cmd when thread is finished!
}

void WfsControl::offButton_clicked() {

   int ret = QMessageBox::question( NULL, "Are you sure?", "Confirm power off of WFS unit?", QMessageBox::Yes, QMessageBox::No);
   if (ret != QMessageBox::Yes)
            return;

	Command* cmd = new Off(getTimeout());
	sendCommandThread(cmd); // This will delete cmd when thread is finished!
}

void WfsControl::radioDisturbanceDisabled_clicked() {
   enableDisturbParams params = { false, false };
   Command *cmd = new EnableDisturb( getTimeout(), params);
   sendCommandThread(cmd);
}

void WfsControl::radioDisturbanceEnabledWFS_clicked() {
   enableDisturbParams params = { true, false };
   Command *cmd = new EnableDisturb( getTimeout(), params);
   sendCommandThread(cmd);
}

void WfsControl::radioDisturbanceEnabledAlways_clicked() {
   enableDisturbParams params = { true, true };
   Command *cmd = new EnableDisturb( getTimeout(), params);
   sendCommandThread(cmd);
}

void WfsControl::buttonXYoffset_clicked() {
   offsetXYparams params;
   params.offsetX = atof( editXoffset->text());
   params.offsetY = atof( editYoffset->text());
   params.brake = true;
   Command *cmd = new OffsetXY( getTimeout(), params);
   sendCommandThread(cmd);
}

void WfsControl::buttonZoffset_clicked() {
   offsetZparams params;
   params.offsetZ = atof( editZoffset->text());
   Command *cmd = new OffsetZ( getTimeout(), params);
   sendCommandThread(cmd);
}


void WfsControl::closeLoopButton_clicked() {
   Command *reply = NULL;

   // Re-send disturbance setting
   bool wfs,ovs;

   if (radioDisturbanceDisabled->isChecked()) 
      wfs = ovs = false;
   if (radioDisturbanceEnabledWFS->isChecked()) {
      wfs = true;
      ovs = false;
   }
   if (radioDisturbanceEnabledAlways->isChecked()) 
      wfs = ovs = true;

   enableDisturbParams params = {wfs, ovs};
   Command *cmd = new EnableDisturb( getTimeout(), params);
   sendCommandWithReply(cmd, &reply);
   if (reply)
      delete reply;
   reply = NULL;


   Command* cmd1 = new PrepareAdsec(getShortTimeout());
   sendCommandWithReply(cmd1, &reply);

   if ((reply) && (reply->isSuccess())) {
      delete reply;
      reply = NULL;
 
      Command* cmd2 = new CloseLoop(getShortTimeout());
      sendCommandWithReply(cmd2, &reply);

      if ((!reply) || (!reply->isSuccess())) {
         // Try to abort the adsec close loop
	 stopLoopAdsecParams params;
         params.hold = true;
         Command *cmd3 = new StopLoopAdsec( getLongTimeout(), params);
         sendCommandThread(cmd3);
      }

      if (reply)
         delete reply;
   }

}

void WfsControl::pauseLoopButton_clicked() {
	Command* cmd = new PauseLoop(getTimeout());
	sendCommandThread(cmd);
}

void WfsControl::resumeLoopButton_clicked() {
	Command* cmd = new ResumeLoop(getTimeout());
	sendCommandThread(cmd);
}

void WfsControl::stopLoopButton_clicked() {

   Command *reply = NULL;

   Command* cmd1 = new StopLoop(getShortTimeout());
   sendCommandWithReply(cmd1, &reply);

   if ((reply) && (reply->isSuccess())) {
      delete reply;
      msleep(100);
      stopLoopAdsecParams params;
      params.hold = ! checkBoxRestoreShape->isChecked();
      Command *cmd2 = new StopLoopAdsec(getLongTimeout(), params);
      sendCommandThread(cmd2);
   }
}

void WfsControl::calibrateHODarkButton_clicked() {
   int nframes = atoi( HOdarkEdit->text().latin1());
	Command* cmd = new CalibrateHODark(getTimeout(), nframes);
	sendCommandThread(cmd);
}

void WfsControl::calibrateTVDarkButton_clicked() {
   int nframes = atoi( TVdarkEdit->text().latin1());
	Command* cmd = new CalibrateTVDark(getTimeout(), nframes);
	sendCommandThread(cmd);
}

void WfsControl::calibrateIRTCDarkButton_clicked() {

   QMessageBox::warning( NULL, "Stop irtc live", "Remember to turn off the LIVE button on the IRTC");

   int nframes = atoi( IRTCdarkEdit->text().latin1());
	Command* cmd = new CalibratePISCESDark(getTimeout(), nframes);
	sendCommandThread(cmd);
}

void WfsControl::calibrateMovementsButton_clicked() {
	Command* cmd = new CalibrateMovements(getTimeout());
	sendCommandThread(cmd);
}

void WfsControl::calibrateSlopenullButton_clicked() {
   int nframes = atoi( SlopenullEdit->text().latin1());
	Command* cmd = new CalibrateSlopenull(getTimeout(), nframes);
	sendCommandThread(cmd);
}

void WfsControl::buttonFreqApply_clicked() {

   modifyAOparams params;
   params.freq = atof( editLoopSpeed->text().latin1());
   params.Binning = spinBoxBinning->value();
   params.TTmod = kDoubleNumInputModulation->value();
   params.Fw1Pos = -1;
   params.Fw2Pos = -1;
   params.checkCameralens = false;

   Command *cmd = new ModifyAO( getTimeout(), params);

   sendCommandThread(cmd);
}

void WfsControl::buttonCCD39_clicked() {
   ostringstream oss;
   oss << getenv("ADOPT_ROOT") << "/bin/ccd_viewer.py ccd39 &";
   if (system( oss.str().c_str()))
       QMessageBox::warning( NULL, "Warning", "Cannot start ccd39 viewer");

}

void WfsControl::buttonCCD47_clicked() {
   ostringstream oss;
   oss << getenv("ADOPT_ROOT") << "/bin/ccd_viewer.py ccd47 &";
   if (system( oss.str().c_str()))
       QMessageBox::warning( NULL, "Warning", "Cannot start ccd47 viewer");
}

void WfsControl::updateArbStatus() {

   RequestWfsStatus arbStatus(150);
   RequestWfsStatus *arbStatusReply = NULL;

   updateArbStatusInternal( &arbStatus, (RequestStatus **)&arbStatusReply);

   if (arbStatusReply) {
      CalibUnitEvent *e = new CalibUnitEvent( arbStatusReply->getSourceName());
      _rerotTrackEnabled = arbStatusReply->getRerotTracking();
      _adcTrackEnabled = arbStatusReply->getAdcTracking();
      _lensTrackEnabled = arbStatusReply->getLensTracking();
      _lensTrackTarget = arbStatusReply->getLensTrackingTarget();
      _lensTrackOutOfRange  = arbStatusReply->getLensTrackingOutOfRange();
      _lensTrackAlert  = arbStatusReply->getLensTrackingAlert();
      postEvent(this, e);
      postRedisplayEvent();

      delete arbStatusReply;
   }

}


void WfsControl::processReply( Command *reply) {

   if (reply->getStatus() != Command::CMD_EXECUTION_SUCCESS) {
      ostringstream oss;
      oss << "Command " << reply->getDescription() << " failed with status " << reply->getStatusAsString() << "\n" << reply->getErrorString();
      postAlert( oss.str());
   }

}


void WfsControl::customEvent(QCustomEvent *e) {

   CalibUnitEvent *calibUnitEvent;

      // Call base class method
      AbstractArbControl::customEvent(e);
} 

void WfsControl::showStatusButton_clicked() {
   ostringstream oss;
   oss << getenv("ADOPT_ROOT") << "/bin/engGui.py &";
   if (system( oss.str().c_str()))
      QMessageBox::warning( NULL, "Warning", "Cannot start status GUI");

}

void WfsControl::buttonStopAutogain_clicked() {

   _VarAutogainStop.Set( 1, 0, FORCE_SEND);

}


static const KCmdLineOptions options[] =
{  
      {"i <identity>", ("MsgD identity"), 0 },
      {"f <file>", ("Configuration file"), 0 },
         KCmdLineLastOption
};

int main(int argc, char* argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

   KCmdLineArgs::init( argc, argv, "WfsControl", "WfsControl", "WfsControl", "1.0" );
   KCmdLineArgs::addCmdLineOptions(options);
   KApplication kApp;

   WfsControl* wfsControl = NULL;

	try {
		if(argc > 1) {
			wfsControl = new WfsControl(argc, argv, kApp);
		}
		else {
			wfsControl = new WfsControl( "wfscontrol", AOApp::getConffile("wfscontrol"), kApp);
		}

		wfsControl->doGui(wfsControl, true, true);

		// Stop the AOApp 
		wfsControl->die();

		delete wfsControl;
		return 0;

	}
	catch (LoggerFatalException &e) {
		// In this case the logger can't log!!!
		printf("%s\n", e.what().c_str());
	}
	catch (AOException &e) {
      printf("Exception: %s", e.what().c_str());
		Logger::get()->log(Logger::LOG_LEV_ERROR, "Error: %s", e.what().c_str());
	}

   return -1;
}
