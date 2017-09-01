/****************************************************************************
** Form implementation generated from reading ui file 'AdSecCalibration_gui.ui'
**
** Created by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "AdSecCalibration_gui.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a AdSecCalibration_gui as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AdSecCalibration_gui::AdSecCalibration_gui( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "AdSecCalibration_gui" );

    buttonGroup3 = new QButtonGroup( this, "buttonGroup3" );
    buttonGroup3->setGeometry( QRect( 208, 11, 191, 358 ) );
    buttonGroup3->setColumnLayout(0, Qt::Vertical );
    buttonGroup3->layout()->setSpacing( 6 );
    buttonGroup3->layout()->setMargin( 11 );
    buttonGroup3Layout = new QVBoxLayout( buttonGroup3->layout() );
    buttonGroup3Layout->setAlignment( Qt::AlignTop );

    layout6 = new QVBoxLayout( 0, 0, 6, "layout6"); 

    buttonPowerSupplyTest = new QPushButton( buttonGroup3, "buttonPowerSupplyTest" );
    buttonPowerSupplyTest->setEnabled( FALSE );
    layout6->addWidget( buttonPowerSupplyTest );

    buttonFastLinkTest = new QPushButton( buttonGroup3, "buttonFastLinkTest" );
    buttonFastLinkTest->setEnabled( FALSE );
    layout6->addWidget( buttonFastLinkTest );

    buttonCoilsTest = new QPushButton( buttonGroup3, "buttonCoilsTest" );
    buttonCoilsTest->setEnabled( FALSE );
    layout6->addWidget( buttonCoilsTest );

    buttonCapSensTest = new QPushButton( buttonGroup3, "buttonCapSensTest" );
    buttonCapSensTest->setEnabled( FALSE );
    layout6->addWidget( buttonCapSensTest );

    buttonWatchdogTest = new QPushButton( buttonGroup3, "buttonWatchdogTest" );
    buttonWatchdogTest->setEnabled( FALSE );
    layout6->addWidget( buttonWatchdogTest );

    buttonDustTest = new QPushButton( buttonGroup3, "buttonDustTest" );
    buttonDustTest->setEnabled( FALSE );
    layout6->addWidget( buttonDustTest );
    buttonGroup3Layout->addLayout( layout6 );

    buttonGroup1 = new QButtonGroup( this, "buttonGroup1" );
    buttonGroup1->setGeometry( QRect( 11, 11, 191, 358 ) );

    QWidget* privateLayoutWidget = new QWidget( buttonGroup1, "layout6" );
    privateLayoutWidget->setGeometry( QRect( 10, 20, 170, 320 ) );
    layout6_2 = new QVBoxLayout( privateLayoutWidget, 11, 6, "layout6_2"); 

    buttonAccelerometer = new QPushButton( privateLayoutWidget, "buttonAccelerometer" );
    layout6_2->addWidget( buttonAccelerometer );

    buttonTurbulenceTrack = new QPushButton( privateLayoutWidget, "buttonTurbulenceTrack" );
    layout6_2->addWidget( buttonTurbulenceTrack );

    buttonDynamicResponse = new QPushButton( privateLayoutWidget, "buttonDynamicResponse" );
    layout6_2->addWidget( buttonDynamicResponse );

    buttonFFMatrix = new QPushButton( privateLayoutWidget, "buttonFFMatrix" );
    layout6_2->addWidget( buttonFFMatrix );

    buttonTestNoise = new QPushButton( privateLayoutWidget, "buttonTestNoise" );
    layout6_2->addWidget( buttonTestNoise );

    buttonStepResponse = new QPushButton( privateLayoutWidget, "buttonStepResponse" );
    layout6_2->addWidget( buttonStepResponse );

    buttonFindTouch = new QPushButton( privateLayoutWidget, "buttonFindTouch" );
    layout6_2->addWidget( buttonFindTouch );

    buttonAcq4DIntMat = new QPushButton( privateLayoutWidget, "buttonAcq4DIntMat" );
    layout6_2->addWidget( buttonAcq4DIntMat );

    buttonFlat4DMirror = new QPushButton( privateLayoutWidget, "buttonFlat4DMirror" );
    layout6_2->addWidget( buttonFlat4DMirror );
    languageChange();
    resize( QSize(414, 383).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( buttonAccelerometer, SIGNAL( clicked() ), this, SLOT( buttonAccelerometer_clicked() ) );
    connect( buttonTurbulenceTrack, SIGNAL( clicked() ), this, SLOT( buttonTurbulenceTrack_clicked() ) );
    connect( buttonDynamicResponse, SIGNAL( clicked() ), this, SLOT( buttonDynamicResponse_clicked() ) );
    connect( buttonFFMatrix, SIGNAL( clicked() ), this, SLOT( buttonFFMatrix_clicked() ) );
    connect( buttonTestNoise, SIGNAL( clicked() ), this, SLOT( buttonTestNoise_clicked() ) );
    connect( buttonStepResponse, SIGNAL( clicked() ), this, SLOT( buttonStepResponse_clicked() ) );
    connect( buttonFindTouch, SIGNAL( clicked() ), this, SLOT( buttonFindTouch_clicked() ) );
    connect( buttonAcq4DIntMat, SIGNAL( clicked() ), this, SLOT( buttonAcq4DIntMat_clicked() ) );
    connect( buttonFlat4DMirror, SIGNAL( clicked() ), this, SLOT( buttonFlat4DMirror_clicked() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AdSecCalibration_gui::~AdSecCalibration_gui()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AdSecCalibration_gui::languageChange()
{
    setCaption( tr( "AdsecCalibration" ) );
    buttonGroup3->setTitle( tr( "Utilities" ) );
    buttonPowerSupplyTest->setText( tr( "POWER SUPPLY TEST" ) );
    buttonFastLinkTest->setText( tr( "FASTLINK TEST" ) );
    buttonCoilsTest->setText( tr( "COILS TEST" ) );
    buttonCapSensTest->setText( tr( "CAPSENS TEST" ) );
    buttonWatchdogTest->setText( tr( "WATCHDOG TEST" ) );
    buttonDustTest->setText( tr( "DUST TEST" ) );
    buttonGroup1->setTitle( tr( "Calibration" ) );
    buttonAccelerometer->setText( tr( "ACCELEROMETER" ) );
    buttonTurbulenceTrack->setText( tr( "TURBULENCE TRACK" ) );
    buttonDynamicResponse->setText( tr( "DYNAMIC RESPONSE" ) );
    buttonFFMatrix->setText( tr( "FF MATRIX" ) );
    buttonTestNoise->setText( tr( "TEST NOISE" ) );
    buttonStepResponse->setText( tr( "STEP RESPONSE" ) );
    buttonFindTouch->setText( tr( "FIND TOUCH" ) );
    buttonAcq4DIntMat->setText( tr( "ACQ 4D INTMAT" ) );
    buttonFlat4DMirror->setText( tr( "FLAT 4D MIRROR" ) );
}

void AdSecCalibration_gui::ffReduce_clicked()
{
    qWarning( "AdSecCalibration_gui::ffReduce_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::ffAcquire_clicked()
{
    qWarning( "AdSecCalibration_gui::ffAcquire_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::capsensTest_clicked()
{
    qWarning( "AdSecCalibration_gui::capsensTest_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::coilsTest_clicked()
{
    qWarning( "AdSecCalibration_gui::coilsTest_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::watchdogTest_clicked()
{
    qWarning( "AdSecCalibration_gui::watchdogTest_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::powersupplyTest_clicked()
{
    qWarning( "AdSecCalibration_gui::powersupplyTest_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::dustTest_clicked()
{
    qWarning( "AdSecCalibration_gui::dustTest_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::password_returnPressed()
{
    qWarning( "AdSecCalibration_gui::password_returnPressed(): Not implemented yet" );
}

void AdSecCalibration_gui::fastlinkTest_clicked()
{
    qWarning( "AdSecCalibration_gui::fastlinkTest_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::buttonAccelerometer_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonAccelerometer_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::buttonTurbulenceTrack_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonTurbulenceTrack_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::buttonDynamicResponse_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonDynamicResponse_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::buttonFFMatrix_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonFFMatrix_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::buttonTestNoise_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonTestNoise_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::buttonStepResponse_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonStepResponse_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::buttonFindTouch_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonFindTouch_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::buttonAcq4DIntMat_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonAcq4DIntMat_clicked(): Not implemented yet" );
}

void AdSecCalibration_gui::buttonFlat4DMirror_clicked()
{
    qWarning( "AdSecCalibration_gui::buttonFlat4DMirror_clicked(): Not implemented yet" );
}

