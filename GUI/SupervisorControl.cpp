//@File: SupervisorControl.cpp
//
// Supervisor Control: GUI to send high-level commands to the AO Supervisor.
// Implements the same commands that the AOS can send.
//@

#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "SupervisorControl.h"

#include "arblib/base/CommandsExport.h"
#include "arblib/base/AlertsExport.h"
#include "arblib/aoArb/AOCommandsExport.h"

#include "AOGlobals.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qtextedit.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <knuminput.h>


using namespace Arcetri;
using namespace Arcetri::Arbitrator;


SupervisorControl::SupervisorControl(string name, string configFile, KApplication &kApp): SupervisorControlGui(), AbstractArbControl(name, configFile, kApp) {
	init();
}


SupervisorControl::SupervisorControl(int argc, char* argv[], KApplication &kApp): SupervisorControlGui(), AbstractArbControl(argc, argv, kApp) {
	init();
}


SupervisorControl::~SupervisorControl() {

}

void SupervisorControl::init() {

    qobject=this;

    string logfile = Utils::getAdoptLog() + "/AOARB."+Side()+".log";
    followLogFile(logfile, this);

    logfile = Utils::getAdoptLog() + "/"+MyName()+"."+Side()+".log";
    printf("%s\n", logfile.c_str());
    followLogFile2(logfile, this);
}

void SupervisorControl::slotDataAvail() {

    if (_tailProc)
        while (_tailProc->canReadLineStdout())
            logText(_tailProc->readLineStdout().latin1());
}

void SupervisorControl::slotDataAvail2() {

    if (_tailProc2)
        while (_tailProc2->canReadLineStdout())
            logText(_tailProc2->readLineStdout().latin1());
}



void SupervisorControl::PostInit() {

    // does proper internal postinit
    AbstractArbControl::PostInit();
    logText("Initialized");
}




void SupervisorControl::setArbitratorStatus(string arbStatus, string lastCmdDescr, string lastCmdResult) {
	aoArbitratorStatusLabel->setText(arbStatus);
	lastCommandLabel->setText(lastCmdDescr);
	lastCommandResultLabel->setText(lastCmdResult);
}


void SupervisorControl::enableGui(bool enable) {

    if (_lastCmdResult == "EXECUTING")
        enable = false;

    bool canPreset = (_arbStatus == "Observation") || (_arbStatus == "ReadyToAcquire") || (_arbStatus == "RefAcquired");
    bool canAcquire = (_arbStatus == "ReadyToAcquire") || (_arbStatus == "RefAcquired");
    bool canCheck = (_arbStatus == "ReadyToAcquire") || (_arbStatus == "RefAcquired");
    bool canModify =  (_arbStatus == "RefAcquired");

    buttonAcquireRef->setEnabled( (canAcquire) ? enable : false);
	presetFlatButton->setEnabled( (_arbStatus == "Observation") ? enable : false);
	buttonPresetAO->setEnabled( (canPreset ) ? enable : false);
    buttonCheckRefAO->setEnabled( (canCheck) ? enable : false);
    buttonModifyAO->setEnabled( (canModify) ? enable : false);
    buttonClose->setEnabled( (_arbStatus == "RefAcquired") ? enable : false);
    buttonOpen->setEnabled( ((_arbStatus == "LoopClosed") || (_arbStatus == "LoopSuspended")) ? enable : false);
    buttonPause->setEnabled( (_arbStatus == "LoopClosed") ? enable : false);
    buttonResume->setEnabled( (_arbStatus == "LoopSuspended") ? enable : false);
    buttonStopFlat->setEnabled( (_arbStatus == "SeeingLimited") ? enable : false);

	presetFlatComboBox->setEnabled(enable );
}

void SupervisorControl::logText(const char* fmt, ...) {

    char message[MAXLOGLINELEN];   

    va_list argp;
    va_start(argp, fmt);
    vsnprintf(message, MAXLOGLINELEN-1, fmt, argp);
    va_end(argp);

    logsText->append(message);
}

void SupervisorControl::presetFlatButton_clicked() {
    presetFlatParams params;
    string flat = presetFlatComboBox->currentText();
    params.flatSpec = flat;
    
    logText("PresetFlat(%s)", flat.c_str());
    Command *cmd = new PresetFlat( getTimeout(), params);
    sendCommandThread(cmd);
}

void SupervisorControl::buttonClose_clicked() {

    logText("StartAO()");
    Command *cmd = new StartAO( getTimeout());
    sendCommandThread(cmd);
	// TODO
}


void SupervisorControl::presetAOButton_clicked() {

    presetAOParams params;
    string mode = comboAOMode->currentText();
    params.aoMode = mode;
    params.focStation = AO_WFS_FLAO;
    params.instr="";
    params.soCoord[0] = kDoubleNumInputSOX->value();
    params.soCoord[1] = kDoubleNumInputSOY->value();
    params.roCoord[0] = kDoubleNumInputROX->value();
    params.roCoord[1] = kDoubleNumInputROY->value();
    params.elevation = kDoubleNumInputTelEl->value();
    params.rotAngle = kDoubleNumInputDerotPos->value();
    params.gravAngle = kDoubleNumInputGravAngle->value();
    params.mag = kDoubleNumInputStarMag->value();
    params.color = kDoubleNumInputStarColor->value();
    params.r0 = 0;
    params.skyBrgt = 0;
    params.windSpeed = 0;
    params.windDir = 0;
    logText("PresetAO()");
    Command *cmd = new PresetAO( getTimeout(), params);
    sendCommandThread(cmd);
}


void SupervisorControl::buttonCheckRefAO_clicked() {

    logText("CheckRefAO()");
    Command *cmd = new CheckRefAO( getTimeout());
    sendCommandThread(cmd);
}

void SupervisorControl::buttonAcquireRef_clicked() {

    logText("AcquireRefAO()");
    Command *cmd = new AcquireRefAO( getTimeout());
    sendCommandThread(cmd);
}

void SupervisorControl::buttonStopFlat_clicked() {
    stopParams params;
    params.msg = "StopFlat";

    logText("Stop(%s)", params.msg.c_str());
    Command *cmd = new Stop( getTimeout(), params);
    sendCommandThread(cmd);
}

void SupervisorControl::buttonPause_clicked() {

    logText("Pause()");
    Command *cmd = new Pause( getTimeout());
    sendCommandThread(cmd);
}

void SupervisorControl::buttonResume_clicked() {

    logText("Resume()");
    Command *cmd = new Resume( getTimeout());
    sendCommandThread(cmd);
}

void SupervisorControl::buttonOpen_clicked() {
    stopParams params;
    params.msg = "Open";
    logText("Stop(%s)", params.msg.c_str());
    Command *cmd = new Stop( getTimeout(), params);
    sendCommandThread(cmd);
}

void SupervisorControl::buttonStopPreset_clicked() {
    stopParams params;
    params.msg = "StopPreset";
    logText("Stop(%s)", params.msg.c_str());
    Command *cmd = new Stop( getTimeout(), params);
    sendCommandThread(cmd);
}

void SupervisorControl::buttonModifyAO_clicked() {

    modifyAOParams params;

    params.nModes = kIntNumModes->value();
    params.freq  = kDoubleNumLoopSpeed->value();
    params.binning = kIntNumBinning->value();
    params.TTmod = kDoubleNumModulation->value();
    params.f1spec = kIntNumFw1Pos->value();
    params.f2spec = kIntNumFw2Pos->value();

    logText("ModifyAO()");
    Command *cmd = new ModifyAO( getTimeout(), params);
    sendCommandThread(cmd);
}

void SupervisorControl::processReply( Command *reply) {

    logText("ProcessReply ...");

    if (reply->getStatus() != Command::CMD_EXECUTION_SUCCESS) {
        ostringstream oss;
        oss << "Command " << reply->getDescription() << " failed with status " << reply->getStatusAsString() << "\n" << reply->getErrorString();
        postAlert( oss.str());
    }

    if ((reply->getCode() == AOOpCodes::PRESET_AO) && (reply->isSuccess())) {
        acquireRefAOResult result = ((PresetAO *)reply)->getResult();
        displayAcquireRefResult(result, "Preset");
    }

    if ((reply->getCode() == AOOpCodes::ACQUIREREF_AO) && (reply->isSuccess())) {
        acquireRefAOResult result = ((AcquireRefAO *)reply)->getResult();
        displayAcquireRefResult(result, "AcquireRef");
    }

    if ((reply->getCode() == AOOpCodes::MODIFY_AO) && (reply->isSuccess())) {
        acquireRefAOResult result = ((ModifyAO *)reply)->getResult();
        displayAcquireRefResult(result, "ModifyAO");
    }

    if ((reply->getCode() == AOOpCodes::CHECKREF_AO) && (reply->isSuccess())) {
        checkRefAOResult result = ((CheckRefAO *)reply)->getResult();
        char str[32];
        sprintf(str, "%5.3f, %5.3f", result.deltaXY[0], result.deltaXY[1]);
        labelCheckRefDelta->setText(str);
        sprintf(str, "%5.3f", result.starMag);
        labelCheckRefMag->setText(str);
    } 


}

void SupervisorControl::displayAcquireRefResult( acquireRefAOResult result, string source) {

    char str[32];
    sprintf( str, "%5.3f, %5.3f", result.deltaXY[0], result.deltaXY[1]);
    labelDeltaXY->setText(str);
    sprintf( str, "%d", result.nModes);
    labelNmodes->setText(str);

    sprintf( str, "%5.3f", result.freq);
    labelLoopSpeed->setText(str);

    sprintf( str, "%d", result.nBins);
    labelBinning->setText(str);

    sprintf( str, "%5.3f", result.ttMod);
    labelModulation->setText(str);

    labelFw1pos->setText( result.f1spec);
    labelFw2pos->setText( result.f2spec);

    labelSetBy->setText(source);
}




static const KCmdLineOptions options[] =
{  
    {"f <file>", ("Configuration file"), 0 },
    {"i <identity>", ("MsgD identity"), 0 },
    KCmdLineLastOption
};

int main(int argc, char* argv[]) {
      
    SetVersion(VERS_MAJOR,VERS_MINOR);

    KCmdLineArgs::init( argc, argv, "SupervisorControl", "SupervisorControl", "SupervisorControl", "1.0" );

    KCmdLineArgs::addCmdLineOptions(options);
    KApplication a;
	SupervisorControl* supervisorControl = NULL;

	try {
		if(argc > 1) {
			supervisorControl = new SupervisorControl(argc, argv, a);
		}
		else {
			supervisorControl = new SupervisorControl("supervisorcontrol", AOApp::getConffile("supervisorcontrol"), a);
		}

        a.setMainWidget(supervisorControl);

		// Start AOApp (do only MSGD connection and initializations to not block in Run!)
		supervisorControl->Exec();

		// Show the GUI
		supervisorControl->show();

		// Start the QThread main loop that updates the GUI
		supervisorControl->start();

		int res = a.exec();
        supervisorControl->die();
		delete supervisorControl;
		return res;

	}
	catch (LoggerFatalException &e) {
		// In this case the logger can't log!!!
		printf("%s\n", e.what().c_str());
	}
	catch (AOException &e) {
		Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error in SupervisorCtrlGui: %s", e.what().c_str());
	}

    return -1;
}
