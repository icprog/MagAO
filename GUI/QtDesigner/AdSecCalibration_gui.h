/****************************************************************************
** Form interface generated from reading ui file 'AdSecCalibration_gui.ui'
**
** Created by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef ADSECCALIBRATION_GUI_H
#define ADSECCALIBRATION_GUI_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QButtonGroup;
class QPushButton;

class AdSecCalibration_gui : public QDialog
{
    Q_OBJECT

public:
    AdSecCalibration_gui( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~AdSecCalibration_gui();

    QButtonGroup* buttonGroup3;
    QPushButton* buttonPowerSupplyTest;
    QPushButton* buttonFastLinkTest;
    QPushButton* buttonCoilsTest;
    QPushButton* buttonCapSensTest;
    QPushButton* buttonWatchdogTest;
    QPushButton* buttonDustTest;
    QButtonGroup* buttonGroup1;
    QPushButton* buttonAccelerometer;
    QPushButton* buttonTurbulenceTrack;
    QPushButton* buttonDynamicResponse;
    QPushButton* buttonFFMatrix;
    QPushButton* buttonTestNoise;
    QPushButton* buttonStepResponse;
    QPushButton* buttonFindTouch;
    QPushButton* buttonAcq4DIntMat;
    QPushButton* buttonFlat4DMirror;

public slots:
    virtual void ffReduce_clicked();
    virtual void ffAcquire_clicked();
    virtual void capsensTest_clicked();
    virtual void coilsTest_clicked();
    virtual void watchdogTest_clicked();
    virtual void powersupplyTest_clicked();
    virtual void dustTest_clicked();
    virtual void password_returnPressed();
    virtual void fastlinkTest_clicked();
    virtual void buttonAccelerometer_clicked();
    virtual void buttonTurbulenceTrack_clicked();
    virtual void buttonDynamicResponse_clicked();
    virtual void buttonFFMatrix_clicked();
    virtual void buttonTestNoise_clicked();
    virtual void buttonStepResponse_clicked();
    virtual void buttonFindTouch_clicked();
    virtual void buttonAcq4DIntMat_clicked();
    virtual void buttonFlat4DMirror_clicked();

protected:
    QVBoxLayout* buttonGroup3Layout;
    QVBoxLayout* layout6;
    QVBoxLayout* layout6_2;

protected slots:
    virtual void languageChange();

};

#endif // ADSECCALIBRATION_GUI_H
