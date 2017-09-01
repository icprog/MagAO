
#include "arblib/adSecArb/AdSecPTypes.h"
#include "arblib/adSecArb/AdSecCommands.h"
#include "AdSecControl.h"
#include "AdSecCalibration.h"
#include <qmessagebox.h>
#include <qpushbutton.h>


using namespace Arcetri;
using namespace Arcetri::Arbitrator;
using namespace Arcetri::AdSec_Arbitrator;


//
AdSecCalibration::AdSecCalibration(AbstractArbControl *arbcontrol):
    _arbctl(arbcontrol)
{
    enableButtons();

    timer = new QTimer( this);
    connect( timer, SIGNAL(timeout()), SLOT(Periodic()));
    timer->start(1000);
}

//
void AdSecCalibration::Periodic() {
    enableButtons();
}

//
void AdSecCalibration::enableButtons() {
    
    static string last_state = "";
    string state = _arbctl->getArbStatus();
    
    if (state != last_state) {
        last_state = state;

        buttonAccelerometer->setEnabled((state == "AOSet") || (state == "Ready"));
        buttonTurbulenceTrack->setEnabled(state == "AOSet");
        buttonDynamicResponse->setEnabled((state == "AOSet") || (state == "Ready"));
        buttonFFMatrix->setEnabled(state == "Ready");
        buttonTestNoise->setEnabled((state == "AOSet") || (state == "Ready"));
        buttonStepResponse->setEnabled(state == "Ready");
        buttonFindTouch->setEnabled(state == "Ready");
        buttonAcq4DIntMat->setEnabled(state == "AOSet");
        buttonFlat4DMirror->setEnabled(state == "AOSet");
    }
}




void AdSecCalibration::buttonAccelerometer_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonAccelerometer_clicked(): Not implemented yet" );
}

void AdSecCalibration::buttonTurbulenceTrack_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonTurbulenceTrack_clicked(): Not implemented yet" );
}

void AdSecCalibration::buttonDynamicResponse_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonDynamicResponse_clicked(): Not implemented yet" );
}

void AdSecCalibration::buttonFFMatrix_clicked()
{
    calibrateParams params;

    params._procname = "ff_data";
    params._param = 0;

    Command *cmd = new CalibrateCmd(_arbctl->getLongTimeout(), params);
    _arbctl->sendCommandThread(cmd);
}

void AdSecCalibration::buttonTestNoise_clicked()
{
    calibrateParams params;

    params._procname = "noise_data";
    params._param = 0;

    Command *cmd = new CalibrateCmd(_arbctl->getLongTimeout(), params);
    _arbctl->sendCommandThread(cmd);
}

void AdSecCalibration::buttonStepResponse_clicked()
{
    calibrateParams params;

    params._procname = "step_data";
    params._param = 0;

    Command *cmd = new CalibrateCmd(_arbctl->getLongTimeout(), params);
    _arbctl->sendCommandThread(cmd);

}

void AdSecCalibration::buttonFindTouch_clicked()
{
    calibrateParams params;

    params._procname = "touch_data";
    params._param = 0;

    Command *cmd = new CalibrateCmd(_arbctl->getLongTimeout(), params);
    _arbctl->sendCommandThread(cmd);

}

void AdSecCalibration::buttonAcq4DIntMat_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonAcq4DIntMat_clicked(): Not implemented yet" );
}

void AdSecCalibration::buttonFlat4DMirror_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonFlat4DMirror_clicked(): Not implemented yet" );
}


