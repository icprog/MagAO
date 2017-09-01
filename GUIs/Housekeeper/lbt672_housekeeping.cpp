/****************************************************************************
** Form implementation generated from reading ui file '.ui/lbt672_housekeeping.ui'
**
** Created by User Interface Compiler
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

extern "C"{
//	#include "msglib.h"
	#include "thrdlib.h"
	#include "commlib.h"
	#include "errlib.h"
	#include "errordb.h"
	#include "buflib.h"
	#include "rtdblib.h" 
	#include "common.h"
}


#include "diagnlib.h"
#include "DiagnWhich.h"
#include "DiagnValue.h"

using namespace Arcetri;
using namespace Arcetri::Diagnostic;

/*
#include "Logger.h"
#include "BcuCommon.h"
#include "adsec_comm.h"



//Global variables definition
//char *server_ip    = "127.0.0.1";
//char MyName[PROC_NAME_LEN];
//int ErrC = 0;
//int flags = 0;
*/


/*
 * Command sender used to communicate with the new MirrorCtrl
 * and a logger.
 */
/*
BcuIdMap 	    _bcuIdMap;
BcuMasterUdpMap _bcuMasterUdpMap;
BcuIpMap 	    _bcuIpMap;
CommandSender*  _comSender = NULL;
Logger*		    _logger = Logger::get("MAIN", Logger::LOG_LEV_DEBUG);	// Use the same name of the AOApp main logger

*/

#include "lbt672_housekeeping.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <kled.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "kled.h"

/* INITIALIZATION */
/****  Housekeeper Family Names,  from housekeeper.param ***********************/
char *FamilyName[200] = {
 "DiagnAppFrameTimestamp",                // DiagnApp fixed general data: 1 value/line    1beg------
 "DiagnAppFrameCounter",                  // next 7 lines are
 "DiagnAppFastVarsCounter",               // shown on "System Summary" left panel
 "DiagnAppSlowVarsCounter",
 "DiagnAppFrameRate",
 "DiagnAppFastVarsRate",
 "DiagnAppSlowVarsRate",                  // ----------------------------------------------1end------
 "BCULocalCurrentThreshold",              // fixed for BCU data: 1 value/line              2beg------
 "BCUVPSet",                              // next 4 lines are
 "BCUTotalCurrentThresholdPos",           // shown on "Crate BCU2" right/down panel
 "BCUTotalCurrentThresholdNeg",           // ----------------------------------------------2end------
 "BCUCrateID",                            // fixed for each BCU crate data: 1 value/crate, 2beg------
 "BCUWhoAmI",                             //  6 crates    next 11 lines are
 "BCUSoftwareRelease",                    // shown on "Crate BCU1" left/upper panel
 "BCULogicRelease",                       // in correspondance to crate number
 "BCUIPAddress",                          //
 "BCUFramesCounter",                      // for each line there are 6 values
 "BCUSerialNumber",
 "BCUPowerBackplaneSerial",
 "BCUEnableMasterDiagnostic",
 "BCUDecimationFactor",
 "BCURemoteIPAddress",                    // ----------------------------------------------3end------
 "SwitchCrateID",                         // fixed for BCU Switch data: 1 value/switch,    4beg------
 "SwitchWhoAmI",                          //  1 switch     next 11 lines are
 "SwitchSoftwareRelease",                 // shown on "Crate BCU1" left/upper panel
 "SwitchLogicRelease",                    // when the "Switch BCU" is chosen instead
 "SwitchIPAddress",                       //  of "Crate number"
 "SwitchFramesCounter",
 "SwitchSerialNumber",                    
 "SwitchPowerBackplaneSerial",            // is not applied for Switch, should be shown deem
 "SwitchEnableMasterDiagnostic",
 "SwitchDecimationFactor",
 "SwitchRemoteIPAddress",                 // ----------------------------------------------4end------
 "DSPWhoAmI",                             // fixed for DSP data: 1 value/line              5beg------
 "DSPSoftwareRelease",                    // next 8 lines are
 "DSPLogicRelease",                       // shown on "Crate DSP" left/upper panel
 "DSPSerialNumber",                       
 "DSPDiagnosticRecordPtr",                
 "DSPDiagnosticRecordLen",                
 "DSPRdDiagnosticRecordPtr",
 "DSPWrDiagnosticRecordPtr",              // ----------------------------------------------5end------
 "DSPADCspiOffset",                       // 1 value/channel(8)/board(14)/crate(6)         6beg------
 "DSPADCspiGain",                         // next 6  lines are
 "DSPADCOffset",                          // shown on "Crate DSP" right/down panel
 "DSPADCGain",                            // for each channel, board and crate
 "DSPDACOffset",
 "DSPDACGain",                            // ----------------------------------------------6end------
 "SGNWhoAmI",                             // fixed for SIGGEN data: 1 value/line           7beg------
 "SGNSoftwareRelease",                    // next 4 lines are
 "SGNLogicRelease",                       // shown on "Crate BCU2" middle/down panel
 "SGNSerialNumber",                       // ----------------------------------------------7end------
 "SGNStratixTemp",                        // SIGGEN data: 1 value/crate = 6                8beg------
 "SGNPowerTemp",                          // next 3 lines are shown  for each crate
 "SGNDspsTemp",                           // on "Crate BCU1" right/down panel -------------8end------
 "DSPStratixTemp",                        // DSP data: 1 val/board(14)/crate(6) = 84       9beg------
 "DSPPowerTemp",                          // next 4 lines are shown  for each crate and board
 "DSPDspsTemp",                           // on "Crate DSP" left/middle panel
 "DSPDriverTemp",                         // ----------------------------------------------9end------
 "DSPCoilCurrent",                        // 8 val for each crate & board are shown on "Crate DSP" midle/upper panel
 "SwitchStratixTemp",                     //fixed 1 val/line                               11beg-----
 "SwitchPowerTemp",                       // 2 lines  are shown on "Crate BCU1"  middle/down panel when "Switch BCU" is chosen
 "BCUStratixTemp",                        //fixed 1 val/crate                              12beg-----
 "BCUPowerTemp",                          // 2 lines  are shown on "Crate BCU1"  middle/down panel  when any "Crate BCU" is chosen
 "BCUVoltageVCCL",                        // BCU data: 1 value/crate = 6                   13beg------
 "BCUVoltageVCCA",                        // next 10 lines are shown  for each crate
 "BCUVoltageVSSA",                        // on "Crate BCU1" right/upper panel
 "BCUVoltageVCCP",
 "BCUVoltageVSSP",
 "BCUCurrentVCCL",
 "BCUCurrentVCCA",
 "BCUCurrentVSSA",
 "BCUCurrentVCCP",
 "BCUCurrentVSSP",                        // ----------------------------------------------13end-----
 "BCUCoolerIn0Temp",                      // BCU data: 1 value/crate = 6                   14beg-----
 "BCUCoolerIn1Temp",                      // next 6 lines are shown  for each crate
 "BCUCoolerOut0Temp",                     // on "Crate BCU1" middle/upper panel
 "BCUCoolerOut1Temp",
 "BCUCoolerPressure",
 "BCUHumidity"                            // ----------------------------------------------14end-----
 "TotalCurrentVCCP",                      // BCU data: 1 value                             15beg-----
 "TotalCurrentVSSP",                      // 3 lines are shown 
 "TotalCurrentVP",                        // on "Crate BCU2" right/upper panel             15end-----
 "BCUDigitalIODriverEnabled",             // BCU data: 1 value/crate                       16beg-----
 "BCUDigitalIOCrateIsMaster",             // next 10 lines are shown  for each crate
 "BCUDigitalIOBusSysFault",               // on "Crate BCU2" middle/upper panel    
 "BCUDigitalIOVccFault",
 "BCUDigitalIOACPowerFault0",
 "BCUDigitalIOACPowerFault1",
 "BCUDigitalIOACPowerFault2",
 "BCUDigitalIOICDisconnected",
 "BCUDigitalIOOvercurrent",
 "BCUDigitalIOCrateID",                   // ----------------------------------------------16end-----
 "BCUResetStatusFPGAReset",               // BCU data: 1 value/crate                       17beg-----
 "BCUResetStatusBUSReset",                // next 10 lines are shown  for each crate
 "BCUResetStatusDSPReset",                // on "Crate BCU2" left/upper panel
 "BCUResetStatusFLASHReset",
 "BCUResetStatusPCIReset",
 "BCUResetStatusBusDriverEnable",
 "BCUResetStatusBusDriverEnableStatus",
 "BCUResetStatusBusPowerFaultCtrl",
 "BCUResetStatusBusPowerFault",
 "BCUResetStatusSystemWatchdog",          // ----------------------------------------------17end-----
 "DSPDriverStatusFPGAReset",              // DSP data: 1 value/board(14)/crate(6) =84      18beg-----
 "DSPDriverStatusDSP0Reset",              // next 10 lines are shown  for each crate & board
 "DSPDriverStatusDSP1Reset",              // on "Crate DSP" right/upper panel
 "DSPDriverStatusFLASHReset",
 "DSPDriverStatusBusPowerFaultCtrl",
 "DSPDriverStatusBusPowerFault",
 "DSPDriverStatusBusDriverEnable",
 "DSPDriverStatusEnableDSPWatchdog",
 "DSPDriverStatusDSP0WatchdogExpired",
 "DSPDriverStatusDSP1WatchdogExpired",    // ----------------------------------------------18end-----
 "DSPDriverStatusDriverEnable",           // 8 channels for each Crate & board are shown on "Crate DSP" middle/down panel
 "ExternalTemperature",                   // fixed data: 1 value/line                      20beg-----
 "FluxRateIn",                            // next 9 lines are shown 
 "WaterMainInlet",                        // on "System Summary" right panel
 "WaterMainOutlet",
 "WaterColdplateInlet",
 "WaterColdplateOutlet",
 "ExternalHumidity",
 "DewPoint",
 "CheckDewPoint"                          // ----------------------------------------------20end-----
};

/*
 *  Constructs a topLBT672 as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
topLBT672::topLBT672(string configFile, QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ), AOApp(configFile)
{
    int i;

    if ( !name )
	setName( "topLBT672" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setSizeGripEnabled( TRUE );
    topLBT672Layout = new QVBoxLayout( this, 11, 6, "topLBT672Layout"); 

    tabWidget = new QTabWidget( this, "tabWidget" );

    tabSYS = new QWidget( tabWidget, "tabSYS" );
    tabSYSLayout = new QHBoxLayout( tabSYS, 11, 6, "tabSYSLayout"); 

    frameDiagn = new QFrame( tabSYS, "frameDiagn" );
    frameDiagn->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameDiagn->sizePolicy().hasHeightForWidth() ) );
    frameDiagn->setFrameShape( QFrame::TabWidgetPanel );
    frameDiagn->setFrameShadow( QFrame::Raised );
    frameDiagnLayout = new QGridLayout( frameDiagn, 1, 1, 11, 6, "frameDiagnLayout"); 

    labelDiagn = new QLabel( frameDiagn, "labelDiagn" );
    labelDiagn->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelDiagn_font(  labelDiagn->font() );
    labelDiagn_font.setFamily( "Bitstream Charter" );
    labelDiagn_font.setBold( TRUE );
    labelDiagn_font.setItalic( TRUE );
    labelDiagn->setFont( labelDiagn_font ); 

    frameDiagnLayout->addWidget( labelDiagn, 0, 1 );

    textLabel1 = new QLabel( frameDiagn, "textLabel1" );

    frameDiagnLayout->addWidget( textLabel1, 1, 0 );

    DiagnAppDataLab[0] = new QLabel( frameDiagn, "DiagnAppDataLab[0]" );

    frameDiagnLayout->addWidget( DiagnAppDataLab[0], 1, 1 );

    DiagnAppDataLed[0] = new KLed( frameDiagn, "DiagnAppDataLed[0]" );

    frameDiagnLayout->addWidget( DiagnAppDataLed[0], 1, 2 );

    textLabel2 = new QLabel( frameDiagn, "textLabel2" );

    frameDiagnLayout->addWidget( textLabel2, 2, 0 );

    DiagnAppDataLab[1] = new QLabel( frameDiagn, "DiagnAppDataLab[1]" );

    frameDiagnLayout->addWidget( DiagnAppDataLab[1], 2, 1 );

    DiagnAppDataLed[1] = new KLed( frameDiagn, "DiagnAppDataLed[1]" );

    frameDiagnLayout->addWidget( DiagnAppDataLed[1], 2, 2 );

    textLabel3 = new QLabel( frameDiagn, "textLabel3" );

    frameDiagnLayout->addWidget( textLabel3, 3, 0 );

    DiagnAppDataLab[2] = new QLabel( frameDiagn, "DiagnAppDataLab[2]" );

    frameDiagnLayout->addWidget( DiagnAppDataLab[2], 3, 1 );

    DiagnAppDataLed[2] = new KLed( frameDiagn, "DiagnAppDataLed[2]" );

    frameDiagnLayout->addWidget( DiagnAppDataLed[2], 3, 2 );

    textLabel4 = new QLabel( frameDiagn, "textLabel4" );

    frameDiagnLayout->addWidget( textLabel4, 4, 0 );

    DiagnAppDataLab[3] = new QLabel( frameDiagn, "DiagnAppDataLab[3]" );

    frameDiagnLayout->addWidget( DiagnAppDataLab[3], 4, 1 );

    DiagnAppDataLed[3] = new KLed( frameDiagn, "DiagnAppDataLed[3]" );

    frameDiagnLayout->addWidget( DiagnAppDataLed[3], 4, 2 );

    textLabel5 = new QLabel( frameDiagn, "textLabel5" );

    frameDiagnLayout->addWidget( textLabel5, 5, 0 );

    DiagnAppDataLab[4] = new QLabel( frameDiagn, "DiagnAppDataLab[4]" );

    frameDiagnLayout->addWidget( DiagnAppDataLab[4], 5, 1 );

    DiagnAppDataLed[4] = new KLed( frameDiagn, "DiagnAppDataLed[4]" );

    frameDiagnLayout->addWidget( DiagnAppDataLed[4], 5, 2 );

    textLabel6 = new QLabel( frameDiagn, "textLabel6" );

    frameDiagnLayout->addWidget( textLabel6, 6, 0 );

    DiagnAppDataLab[5] = new QLabel( frameDiagn, "DiagnAppDataLab[5]" );

    frameDiagnLayout->addWidget( DiagnAppDataLab[5], 6, 1 );

    DiagnAppDataLed[5] = new KLed( frameDiagn, "DiagnAppDataLed[5]" );

    frameDiagnLayout->addWidget( DiagnAppDataLed[5], 6, 2 );

    textLabel7 = new QLabel( frameDiagn, "textLabel7" );

    frameDiagnLayout->addWidget( textLabel7, 7, 0 );

    DiagnAppDataLab[6] = new QLabel( frameDiagn, "DiagnAppDataLab[6]" );
    for (i=0; i<7; i++) {
        DiagnAppDataLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, DiagnAppDataLab[6]->sizePolicy().hasHeightForWidth() ) );
        DiagnAppDataLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        DiagnAppDataLab[i]->setFrameShape( QLabel::LineEditPanel );
        DiagnAppDataLab[i]->setFrameShadow( QLabel::Sunken );
        DiagnAppDataLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameDiagnLayout->addWidget( DiagnAppDataLab[6], 7, 1 );

    DiagnAppDataLed[6] = new KLed( frameDiagn, "DiagnAppDataLed[6]" );

    frameDiagnLayout->addWidget( DiagnAppDataLed[6], 7, 2 );
    tabSYSLayout->addWidget( frameDiagn );

    frameTemper = new QFrame( tabSYS, "frameTemper" );
    frameTemper->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameTemper->sizePolicy().hasHeightForWidth() ) );
    frameTemper->setFrameShape( QFrame::TabWidgetPanel );
    frameTemper->setFrameShadow( QFrame::Raised );
    frameTemperLayout = new QGridLayout( frameTemper, 1, 1, 11, 6, "frameTemperLayout"); 

    labelTemper = new QLabel( frameTemper, "labelTemper" );
    labelTemper->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelTemper_font(  labelTemper->font() );
    labelTemper_font.setFamily( "Bitstream Charter" );
    labelTemper_font.setBold( TRUE );
    labelTemper_font.setItalic( TRUE );
    labelTemper->setFont( labelTemper_font ); 

    frameTemperLayout->addWidget( labelTemper, 0, 1 );

    textLabel11 = new QLabel( frameTemper, "textLabel11" );

    frameTemperLayout->addWidget( textLabel11, 1, 0 );

    ExternalLab[0] = new QLabel( frameTemper, "ExternalLab[0]" );

    frameTemperLayout->addWidget( ExternalLab[0], 1, 1 );

    ExternalLed[0] = new KLed( frameTemper, "ExternalLed[0]" );

    frameTemperLayout->addWidget( ExternalLed[0], 1, 2 );

    ExternalBut[0] = new QPushButton( frameTemper, "ExternalBut[0]" );
    ExternalBut[0]->setAutoDefault( FALSE );

    frameTemperLayout->addWidget( ExternalBut[0], 1, 3 );

    textLabel12 = new QLabel( frameTemper, "textLabel12" );

    frameTemperLayout->addWidget( textLabel12, 2, 0 );

    ExternalLab[1] = new QLabel( frameTemper, "ExternalLab[1]" );

    frameTemperLayout->addWidget( ExternalLab[1], 2, 1 );

    ExternalLed[1] = new KLed( frameTemper, "ExternalLed[1]" );

    frameTemperLayout->addWidget( ExternalLed[1], 2, 2 );

    ExternalBut[1] = new QPushButton( frameTemper, "ExternalBut[1]" );
    ExternalBut[1]->setAutoDefault( FALSE );

    frameTemperLayout->addWidget( ExternalBut[1], 2, 3 );

    textLabel13 = new QLabel( frameTemper, "textLabel13" );

    frameTemperLayout->addWidget( textLabel13, 3, 0 );

    ExternalLab[2] = new QLabel( frameTemper, "ExternalLab[2]" );

    frameTemperLayout->addWidget( ExternalLab[2], 3, 1 );

    ExternalLed[2] = new KLed( frameTemper, "ExternalLed[2]" );

    frameTemperLayout->addWidget( ExternalLed[2], 3, 2 );

    ExternalBut[2] = new QPushButton( frameTemper, "ExternalBut[2]" );
    ExternalBut[2]->setAutoDefault( FALSE );

    frameTemperLayout->addWidget( ExternalBut[2], 3, 3 );

    textLabel14 = new QLabel( frameTemper, "textLabel14" );

    frameTemperLayout->addWidget( textLabel14, 4, 0 );

    ExternalLab[3] = new QLabel( frameTemper, "ExternalLab[3]" );

    frameTemperLayout->addWidget( ExternalLab[3], 4, 1 );

    ExternalLed[3] = new KLed( frameTemper, "ExternalLed[3]" );

    frameTemperLayout->addWidget( ExternalLed[3], 4, 2 );

    ExternalBut[3] = new QPushButton( frameTemper, "ExternalBut[3]" );
    ExternalBut[3]->setAutoDefault( FALSE );

    frameTemperLayout->addWidget( ExternalBut[3], 4, 3 );

    textLabel15 = new QLabel( frameTemper, "textLabel15" );

    frameTemperLayout->addWidget( textLabel15, 5, 0 );

    ExternalLab[4] = new QLabel( frameTemper, "ExternalLab[4]" );

    frameTemperLayout->addWidget( ExternalLab[4], 5, 1 );

    ExternalLed[4] = new KLed( frameTemper, "ExternalLed[4]" );

    frameTemperLayout->addWidget( ExternalLed[4], 5, 2 );

    ExternalBut[4] = new QPushButton( frameTemper, "ExternalBut[4]" );
    ExternalBut[4]->setAutoDefault( FALSE );

    frameTemperLayout->addWidget( ExternalBut[4], 5, 3 );

    textLabel16 = new QLabel( frameTemper, "textLabel16" );

    frameTemperLayout->addWidget( textLabel16, 6, 0 );

    ExternalLab[5] = new QLabel( frameTemper, "ExternalLab[5]" );

    frameTemperLayout->addWidget( ExternalLab[5], 6, 1 );

    ExternalLed[5] = new KLed( frameTemper, "ExternalLed[5]" );

    frameTemperLayout->addWidget( ExternalLed[5], 6, 2 );

    ExternalBut[5] = new QPushButton( frameTemper, "ExternalBut[5]" );
    ExternalBut[5]->setAutoDefault( FALSE );

    frameTemperLayout->addWidget( ExternalBut[5], 6, 3 );

    textLabel17 = new QLabel( frameTemper, "textLabel17" );

    frameTemperLayout->addWidget( textLabel17, 7, 0 );

    ExternalLab[6] = new QLabel( frameTemper, "ExternalLab[6]" );

    frameTemperLayout->addWidget( ExternalLab[6], 7, 1 );

    ExternalLed[6] = new KLed( frameTemper, "ExternalLed[6]" );

    frameTemperLayout->addWidget( ExternalLed[6], 7, 2 );

    ExternalBut[6] = new QPushButton( frameTemper, "ExternalBut[6]" );
    ExternalBut[6]->setAutoDefault( FALSE );

    frameTemperLayout->addWidget( ExternalBut[6], 7, 3 );

    textLabel18 = new QLabel( frameTemper, "textLabel18" );

    frameTemperLayout->addWidget( textLabel18, 8, 0 );

    ExternalLab[7] = new QLabel( frameTemper, "ExternalLab[7]" );

    frameTemperLayout->addWidget( ExternalLab[7], 8, 1 );

    ExternalLed[7] = new KLed( frameTemper, "ExternalLed[7]" );

    frameTemperLayout->addWidget( ExternalLed[7], 8, 2 );

    ExternalBut[7] = new QPushButton( frameTemper, "ExternalBut[7]" );
    ExternalBut[7]->setAutoDefault( FALSE );

    frameTemperLayout->addWidget( ExternalBut[7], 8, 3 );

    textLabel19 = new QLabel( frameTemper, "textLabel19" );

    frameTemperLayout->addWidget( textLabel19, 9, 0 );

    ExternalLab[8] = new QLabel( frameTemper, "ExternalLab[8]" );
    for (i=0; i<9; i++) {
        ExternalLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, ExternalLab[8]->sizePolicy().hasHeightForWidth() ) );
        ExternalLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        ExternalLab[i]->setFrameShape( QLabel::LineEditPanel );
        ExternalLab[i]->setFrameShadow( QLabel::Sunken );
        ExternalLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameTemperLayout->addWidget( ExternalLab[8], 9, 1 );

    ExternalLed[8] = new KLed( frameTemper, "ExternalLed[8]" );

    frameTemperLayout->addWidget( ExternalLed[8], 9, 2 );

    ExternalBut[8] = new QPushButton( frameTemper, "ExternalBut[8]" );
    ExternalBut[8]->setAutoDefault( FALSE );

    frameTemperLayout->addWidget( ExternalBut[8], 9, 3 );
    tabSYSLayout->addWidget( frameTemper );
    tabWidget->insertTab( tabSYS, QString::fromLatin1("") );

    unnamed = new QWidget( tabWidget, "unnamed" );
    unnamedLayout = new QGridLayout( unnamed, 1, 1, 11, 6, "unnamedLayout"); 

    frameNIOS = new QFrame( unnamed, "frameNIOS" );
    frameNIOS->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameNIOS->sizePolicy().hasHeightForWidth() ) );
    frameNIOS->setFrameShape( QFrame::TabWidgetPanel );
    frameNIOS->setFrameShadow( QFrame::Raised );
    frameNIOSLayout = new QGridLayout( frameNIOS, 1, 1, 11, 6, "frameNIOSLayout"); 

    labelNIOS = new QLabel( frameNIOS, "labelNIOS" );
    labelNIOS->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelNIOS_font(  labelNIOS->font() );
    labelNIOS_font.setFamily( "Bitstream Charter" );
    labelNIOS_font.setBold( TRUE );
    labelNIOS_font.setItalic( TRUE );
    labelNIOS->setFont( labelNIOS_font ); 

    frameNIOSLayout->addWidget( labelNIOS, 0, 1 );

    textLabelBCU1_1 = new QLabel( frameNIOS, "textLabelBCU1_1" );

    frameNIOSLayout->addWidget( textLabelBCU1_1, 1, 0 );

    BCU1NiosLab[0] = new QLabel( frameNIOS, "BCU1NiosLab[0]" );

    frameNIOSLayout->addWidget( BCU1NiosLab[0], 1, 1 );

    BCU1NiosLed[0] = new KLed( frameNIOS, "BCU1NiosLed[0]" );

    frameNIOSLayout->addWidget( BCU1NiosLed[0], 1, 2 );

    textLabelBCU1_2 = new QLabel( frameNIOS, "textLabelBCU1_2" );

    frameNIOSLayout->addWidget( textLabelBCU1_2, 2, 0 );

    BCU1NiosLab[1] = new QLabel( frameNIOS, "BCU1NiosLab[1]" );

    frameNIOSLayout->addWidget( BCU1NiosLab[1], 2, 1 );

    BCU1NiosLed[1] = new KLed( frameNIOS, "BCU1NiosLed[1]" );

    frameNIOSLayout->addWidget( BCU1NiosLed[1], 2, 2 );

    textLabelBCU1_3 = new QLabel( frameNIOS, "textLabelBCU1_3" );

    frameNIOSLayout->addWidget( textLabelBCU1_3, 3, 0 );

    BCU1NiosLab[2] = new QLabel( frameNIOS, "BCU1NiosLab[2]" );

    frameNIOSLayout->addWidget( BCU1NiosLab[2], 3, 1 );

    BCU1NiosLed[2] = new KLed( frameNIOS, "BCU1NiosLed[2]" );

    frameNIOSLayout->addWidget( BCU1NiosLed[2], 3, 2 );

    textLabelBCU1_4 = new QLabel( frameNIOS, "textLabelBCU1_4" );

    frameNIOSLayout->addWidget( textLabelBCU1_4, 4, 0 );

    BCU1NiosLab[3] = new QLabel( frameNIOS, "BCU1NiosLab[3]" );

    frameNIOSLayout->addWidget( BCU1NiosLab[3], 4, 1 );

    BCU1NiosLed[3] = new KLed( frameNIOS, "BCU1NiosLed[3]" );

    frameNIOSLayout->addWidget( BCU1NiosLed[3], 4, 2 );

    textLabelBCU1_5 = new QLabel( frameNIOS, "textLabelBCU1_5" );

    frameNIOSLayout->addWidget( textLabelBCU1_5, 5, 0 );

    BCU1NiosLab[4] = new QLabel( frameNIOS, "BCU1NiosLab[4]" );

    frameNIOSLayout->addWidget( BCU1NiosLab[4], 5, 1 );

    BCU1NiosLed[4] = new KLed( frameNIOS, "BCU1NiosLed[4]" );

    frameNIOSLayout->addWidget( BCU1NiosLed[4], 5, 2 );

    textLabelBCU1_6 = new QLabel( frameNIOS, "textLabelBCU1_6" );

    frameNIOSLayout->addWidget( textLabelBCU1_6, 6, 0 );

    BCU1NiosLab[5] = new QLabel( frameNIOS, "BCU1NiosLab[5]" );

    frameNIOSLayout->addWidget( BCU1NiosLab[5], 6, 1 );

    BCU1NiosLed[5] = new KLed( frameNIOS, "BCU1NiosLed[5]" );

    frameNIOSLayout->addWidget( BCU1NiosLed[5], 6, 2 );

    textLabelBCU1_7 = new QLabel( frameNIOS, "textLabelBCU1_7" );

    frameNIOSLayout->addWidget( textLabelBCU1_7, 7, 0 );

    BCU1NiosLab[6] = new QLabel( frameNIOS, "BCU1NiosLab[6]" );

    frameNIOSLayout->addWidget( BCU1NiosLab[6], 7, 1 );

    BCU1NiosLed[6] = new KLed( frameNIOS, "BCU1NiosLed[6]" );

    frameNIOSLayout->addWidget( BCU1NiosLed[6], 7, 2 );

    textLabelBCU1_8 = new QLabel( frameNIOS, "textLabelBCU1_8" );

    frameNIOSLayout->addWidget( textLabelBCU1_8, 8, 0 );

    BCU1NiosLab[7] = new QLabel( frameNIOS, "BCU1NiosLab[7]" );

    frameNIOSLayout->addWidget( BCU1NiosLab[7], 8, 1 );

    BCU1NiosLed[7] = new KLed( frameNIOS, "BCU1NiosLed[7]" );

    frameNIOSLayout->addWidget( BCU1NiosLed[7], 8, 2 );

    textLabelBCU1_9 = new QLabel( frameNIOS, "textLabelBCU1_9" );

    frameNIOSLayout->addWidget( textLabelBCU1_9, 9, 0 );

    BCU1NiosLab[8] = new QLabel( frameNIOS, "BCU1NiosLab[8]" );

    frameNIOSLayout->addWidget( BCU1NiosLab[8], 9, 1 );

    BCU1NiosLed[8] = new KLed( frameNIOS, "BCU1NiosLed[8]" );

    frameNIOSLayout->addWidget( BCU1NiosLed[8], 9, 2 );

    textLabelBCU1_10 = new QLabel( frameNIOS, "textLabelBCU1_10" );

    frameNIOSLayout->addWidget( textLabelBCU1_10, 10, 0 );

    BCU1NiosLab[9] = new QLabel( frameNIOS, "BCU1NiosLab[9]" );

    frameNIOSLayout->addWidget( BCU1NiosLab[9], 10, 1 );

    BCU1NiosLed[9] = new KLed( frameNIOS, "BCU1NiosLed[9]" );

    frameNIOSLayout->addWidget( BCU1NiosLed[9], 10, 2 );

    textLabelBCU1_11 = new QLabel( frameNIOS, "textLabelBCU1_11" );

    frameNIOSLayout->addWidget( textLabelBCU1_11, 11, 0 );

    BCU1NiosLab[10] = new QLabel( frameNIOS, "BCU1NiosLab[10]" );
    for (i=0; i<11; i++) {
        BCU1NiosLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU1NiosLab[10]->sizePolicy().hasHeightForWidth() ) );
        BCU1NiosLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU1NiosLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU1NiosLab[i]->setFrameShadow( QLabel::Sunken );
        BCU1NiosLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameNIOSLayout->addWidget( BCU1NiosLab[10], 11, 1 );

    BCU1NiosLed[10] = new KLed( frameNIOS, "BCU1NiosLed[10]" );

    frameNIOSLayout->addWidget( BCU1NiosLed[10], 11, 2 );

    unnamedLayout->addWidget( frameNIOS, 0, 0 );

    crateNumber = new QFrame( unnamed, "crateNumber" );
    crateNumber->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, crateNumber->sizePolicy().hasHeightForWidth() ) );
    crateNumber->setFrameShape( QFrame::TabWidgetPanel );
    crateNumber->setFrameShadow( QFrame::Raised );
    crateNumberLayout = new QGridLayout( crateNumber, 1, 1, 11, 6, "crateNumberLayout"); 


    crateBCU1_gr = new QButtonGroup( crateNumber, "crateBCU1_gr" );
    crateBCU1_gr->setPaletteForegroundColor( QColor( 85, 0, 127 ) );
    crateBCU1_gr->setPaletteBackgroundColor( QColor( 166, 192, 167 ) );
    QFont crateBCU1_gr_font(  crateBCU1_gr->font() );
    crateBCU1_gr_font.setFamily( "Bitstream Charter" );
    crateBCU1_gr_font.setBold( TRUE );
    crateBCU1_gr_font.setItalic( TRUE );
    crateBCU1_gr->setFont( crateBCU1_gr_font ); 
    crateBCU1_gr->setColumnLayout(0, Qt::Vertical );
    crateBCU1_gr->layout()->setSpacing( 6 );
    crateBCU1_gr->layout()->setMargin( 0 );
    crateBCU1_grLayout = new QHBoxLayout( crateBCU1_gr->layout() );
    crateBCU1_grLayout->setAlignment( Qt::AlignTop );

    BCU1RBut[0] = new QRadioButton( crateBCU1_gr, "BCU1RBut[0]" );
    BCU1RBut[0]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    BCU1RBut[0]->setChecked( TRUE );
    crateBCU1_grLayout->addWidget( BCU1RBut[0] );

    BCU1RBut[1] = new QRadioButton( crateBCU1_gr, "BCU1RBut[1]" );
    BCU1RBut[1]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateBCU1_grLayout->addWidget( BCU1RBut[1] );

    BCU1RBut[2] = new QRadioButton( crateBCU1_gr, "BCU1RBut[2]" );
    BCU1RBut[2]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateBCU1_grLayout->addWidget( BCU1RBut[2] );

    BCU1RBut[3] = new QRadioButton( crateBCU1_gr, "BCU1RBut[3]" );
    BCU1RBut[3]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateBCU1_grLayout->addWidget( BCU1RBut[3] );

    BCU1RBut[4] = new QRadioButton( crateBCU1_gr, "BCU1RBut[4]" );
    BCU1RBut[4]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateBCU1_grLayout->addWidget( BCU1RBut[4] );

    BCU1RBut[5] = new QRadioButton( crateBCU1_gr, "BCU1RBut[5]" );
    BCU1RBut[5]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateBCU1_grLayout->addWidget( BCU1RBut[5] );

    BCU1RBut[6] = new QRadioButton( crateBCU1_gr, "BCU1RBut[6]" );
    BCU1RBut[6]->setPaletteForegroundColor( QColor( 0, 85, 0 ) );
    crateBCU1_grLayout->addWidget( BCU1RBut[6] );

    crateNumberLayout->addWidget( crateBCU1_gr, 0, 0 );

    unnamedLayout->addWidget( crateNumber, 1, 0 );

    frameEnvirBCU1 = new QFrame( unnamed, "frameEnvirBCU1" );
    frameEnvirBCU1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameEnvirBCU1->sizePolicy().hasHeightForWidth() ) );
    frameEnvirBCU1->setFrameShape( QFrame::TabWidgetPanel );
    frameEnvirBCU1->setFrameShadow( QFrame::Raised );
    frameEnvirBCU1Layout = new QGridLayout( frameEnvirBCU1, 1, 1, 11, 6, "frameEnvirBCU1Layout"); 

    labelEnvBCU1 = new QLabel( frameEnvirBCU1, "labelEnvBCU1" );
    labelEnvBCU1->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelEnvBCU1_font(  labelEnvBCU1->font() );
    labelEnvBCU1_font.setFamily( "Bitstream Charter" );
    labelEnvBCU1_font.setBold( TRUE );
    labelEnvBCU1_font.setItalic( TRUE );
    labelEnvBCU1->setFont( labelEnvBCU1_font ); 

    frameEnvirBCU1Layout->addWidget( labelEnvBCU1, 0, 1 );

    textLabelBCU1b_1 = new QLabel( frameEnvirBCU1, "textLabelBCU1b_1" );

    frameEnvirBCU1Layout->addWidget( textLabelBCU1b_1, 1, 0 );

    BCU1EnvLab[0] = new QLabel( frameEnvirBCU1, "BCU1EnvLab[0]" );

    frameEnvirBCU1Layout->addWidget( BCU1EnvLab[0], 1, 1 );

    BCU1EnvLed[0] = new KLed( frameEnvirBCU1, "BCU1EnvLed[0]" );

    frameEnvirBCU1Layout->addWidget( BCU1EnvLed[0], 1, 2 );

    BCU1EnvBut[0] = new QPushButton( frameEnvirBCU1, "BCU1EnvBut[0]" );
    BCU1EnvBut[0]->setAutoDefault( FALSE );

    frameEnvirBCU1Layout->addWidget( BCU1EnvBut[0], 1, 3 );

    textLabelBCU1b_2 = new QLabel( frameEnvirBCU1, "textLabelBCU1b_2" );
    textLabelBCU1b_2->setFrameShape( QLabel::NoFrame );
    textLabelBCU1b_2->setFrameShadow( QLabel::Plain );

    frameEnvirBCU1Layout->addWidget( textLabelBCU1b_2, 2, 0 );

    BCU1EnvLab[1] = new QLabel( frameEnvirBCU1, "BCU1EnvLab[1]" );

    frameEnvirBCU1Layout->addWidget( BCU1EnvLab[1], 2, 1 );

    BCU1EnvLed[1] = new KLed( frameEnvirBCU1, "BCU1EnvLed[1]" );

    frameEnvirBCU1Layout->addWidget( BCU1EnvLed[1], 2, 2 );

    BCU1EnvBut[1] = new QPushButton( frameEnvirBCU1, "BCU1EnvBut[1]" );
    BCU1EnvBut[1]->setAutoDefault( FALSE );

    frameEnvirBCU1Layout->addWidget( BCU1EnvBut[1], 2, 3 );

    textLabelBCU1b_3 = new QLabel( frameEnvirBCU1, "textLabelBCU1b_3" );

    frameEnvirBCU1Layout->addWidget( textLabelBCU1b_3, 3, 0 );

    BCU1EnvLab[2] = new QLabel( frameEnvirBCU1, "BCU1EnvLab[2]" );

    frameEnvirBCU1Layout->addWidget( BCU1EnvLab[2], 3, 1 );

    BCU1EnvLed[2] = new KLed( frameEnvirBCU1, "BCU1EnvLed[2]" );

    frameEnvirBCU1Layout->addWidget( BCU1EnvLed[2], 3, 2 );

    BCU1EnvBut[2] = new QPushButton( frameEnvirBCU1, "BCU1EnvBut[2]" );
    BCU1EnvBut[2]->setAutoDefault( FALSE );

    frameEnvirBCU1Layout->addWidget( BCU1EnvBut[2], 3, 3 );

    textLabelBCU1b_4 = new QLabel( frameEnvirBCU1, "textLabelBCU1b_4" );

    frameEnvirBCU1Layout->addWidget( textLabelBCU1b_4, 4, 0 );

    BCU1EnvLab[3] = new QLabel( frameEnvirBCU1, "BCU1EnvLab[3]" );

    frameEnvirBCU1Layout->addWidget( BCU1EnvLab[3], 4, 1 );

    BCU1EnvLed[3] = new KLed( frameEnvirBCU1, "BCU1EnvLed[3]" );

    frameEnvirBCU1Layout->addWidget( BCU1EnvLed[3], 4, 2 );

    BCU1EnvBut[3] = new QPushButton( frameEnvirBCU1, "BCU1EnvBut[3]" );
    BCU1EnvBut[3]->setAutoDefault( FALSE );

    frameEnvirBCU1Layout->addWidget( BCU1EnvBut[3], 4, 3 );

    textLabelBCU1b_5 = new QLabel( frameEnvirBCU1, "textLabelBCU1b_5" );

    frameEnvirBCU1Layout->addWidget( textLabelBCU1b_5, 5, 0 );

    BCU1EnvLab[4] = new QLabel( frameEnvirBCU1, "BCU1EnvLab[4]" );

    frameEnvirBCU1Layout->addWidget( BCU1EnvLab[4], 5, 1 );

    BCU1EnvLed[4] = new KLed( frameEnvirBCU1, "BCU1EnvLed[4]" );

    frameEnvirBCU1Layout->addWidget( BCU1EnvLed[4], 5, 2 );

    BCU1EnvBut[4] = new QPushButton( frameEnvirBCU1, "BCU1EnvBut[4]" );
    BCU1EnvBut[4]->setAutoDefault( FALSE );

    frameEnvirBCU1Layout->addWidget( BCU1EnvBut[4], 5, 3 );

    textLabelBCU1b_6 = new QLabel( frameEnvirBCU1, "textLabelBCU1b_6" );

    frameEnvirBCU1Layout->addWidget( textLabelBCU1b_6, 6, 0 );

    BCU1EnvLab[5] = new QLabel( frameEnvirBCU1, "BCU1EnvLab[5]" );
    for (i=0; i<6; i++) {
        BCU1EnvLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU1EnvLab[5]->sizePolicy().hasHeightForWidth() ) );
        BCU1EnvLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU1EnvLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU1EnvLab[i]->setFrameShadow( QLabel::Sunken );
        BCU1EnvLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameEnvirBCU1Layout->addWidget( BCU1EnvLab[5], 6, 1 );

    BCU1EnvLed[5] = new KLed( frameEnvirBCU1, "BCU1EnvLed[5]" );

    frameEnvirBCU1Layout->addWidget( BCU1EnvLed[5], 6, 2 );

    BCU1EnvBut[5] = new QPushButton( frameEnvirBCU1, "BCU1EnvBut[5]" );
    BCU1EnvBut[5]->setAutoDefault( FALSE );

    frameEnvirBCU1Layout->addWidget( BCU1EnvBut[5], 6, 3 );

    unnamedLayout->addWidget( frameEnvirBCU1, 0, 1 );

    frameTempBCU1 = new QFrame( unnamed, "frameTempBCU1" );
    frameTempBCU1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameTempBCU1->sizePolicy().hasHeightForWidth() ) );
    frameTempBCU1->setFrameShape( QFrame::TabWidgetPanel );
    frameTempBCU1->setFrameShadow( QFrame::Raised );
    frameTempBCU1Layout = new QGridLayout( frameTempBCU1, 1, 1, 11, 6, "frameTempBCU1Layout"); 

    labelTempBCU1 = new QLabel( frameTempBCU1, "labelTempBCU1" );
    labelTempBCU1->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelTempBCU1_font(  labelTempBCU1->font() );
    labelTempBCU1_font.setFamily( "Bitstream Charter" );
    labelTempBCU1_font.setBold( TRUE );
    labelTempBCU1_font.setItalic( TRUE );
    labelTempBCU1->setFont( labelTempBCU1_font ); 

    frameTempBCU1Layout->addWidget( labelTempBCU1, 0, 1 );

    textLabelBCU1a_1 = new QLabel( frameTempBCU1, "textLabelBCU1a_1" );

    frameTempBCU1Layout->addWidget( textLabelBCU1a_1, 1, 0 );

    BCU1TempLab[0] = new QLabel( frameTempBCU1, "BCU1TempLab[0]" );

    frameTempBCU1Layout->addWidget( BCU1TempLab[0], 1, 1 );

    BCU1TempLed[0] = new KLed( frameTempBCU1, "BCU1TempLed[0]" );

    frameTempBCU1Layout->addWidget( BCU1TempLed[0], 1, 2 );

    BCU1TempBut[0] = new QPushButton( frameTempBCU1, "BCU1TempBut[0]" );
    BCU1TempBut[0]->setAutoDefault( FALSE );

    frameTempBCU1Layout->addWidget( BCU1TempBut[0], 1, 3 );

    textLabelBCU1a_2 = new QLabel( frameTempBCU1, "textLabelBCU1a_2" );

    frameTempBCU1Layout->addWidget( textLabelBCU1a_2, 2, 0 );

    BCU1TempLab[1] = new QLabel( frameTempBCU1, "BCU1TempLab[1]" );
    for (i=0; i<2; i++) {
        BCU1TempLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU1TempLab[1]->sizePolicy().hasHeightForWidth() ) );
        BCU1TempLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU1TempLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU1TempLab[i]->setFrameShadow( QLabel::Sunken );
        BCU1TempLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameTempBCU1Layout->addWidget( BCU1TempLab[1], 2, 1 );

    BCU1TempLed[1] = new KLed( frameTempBCU1, "BCU1TempLed[1]" );

    frameTempBCU1Layout->addWidget( BCU1TempLed[1], 2, 2 );

    BCU1TempBut[1] = new QPushButton( frameTempBCU1, "BCU1TempBut[1]" );
    BCU1TempBut[1]->setAutoDefault( FALSE );

    frameTempBCU1Layout->addWidget( BCU1TempBut[1], 2, 3 );

    unnamedLayout->addWidget( frameTempBCU1, 1, 1 );

    frameVoltBCU1 = new QFrame( unnamed, "frameVoltBCU1" );
    frameVoltBCU1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameVoltBCU1->sizePolicy().hasHeightForWidth() ) );
    frameVoltBCU1->setFrameShape( QFrame::TabWidgetPanel );
    frameVoltBCU1->setFrameShadow( QFrame::Raised );
    frameVoltBCU1Layout = new QGridLayout( frameVoltBCU1, 1, 1, 11, 6, "frameVoltBCU1Layout"); 

    labelVoltBCU1 = new QLabel( frameVoltBCU1, "labelVoltBCU1" );
    labelVoltBCU1->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelVoltBCU1_font(  labelVoltBCU1->font() );
    labelVoltBCU1_font.setFamily( "Bitstream Charter" );
    labelVoltBCU1_font.setBold( TRUE );
    labelVoltBCU1_font.setItalic( TRUE );
    labelVoltBCU1->setFont( labelVoltBCU1_font ); 

    frameVoltBCU1Layout->addWidget( labelVoltBCU1, 0, 1 );

    textLabelBCU1c_1 = new QLabel( frameVoltBCU1, "textLabelBCU1c_1" );

    frameVoltBCU1Layout->addWidget( textLabelBCU1c_1, 1, 0 );

    BCU1VolLab[0] = new QLabel( frameVoltBCU1, "BCU1VolLab[0]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLab[0], 1, 1 );

    BCU1VolLed[0] = new KLed( frameVoltBCU1, "BCU1VolLed[0]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLed[0], 1, 2 );

    BCU1VolBut[0] = new QPushButton( frameVoltBCU1, "BCU1VolBut[0]" );
    BCU1VolBut[0]->setAutoDefault( FALSE );

    frameVoltBCU1Layout->addWidget( BCU1VolBut[0], 1, 3 );

    textLabelBCU1c_2 = new QLabel( frameVoltBCU1, "textLabelBCU1c_2" );

    frameVoltBCU1Layout->addWidget( textLabelBCU1c_2, 2, 0 );

    BCU1VolLab[1] = new QLabel( frameVoltBCU1, "BCU1VolLab[1]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLab[1], 2, 1 );

    BCU1VolLed[1] = new KLed( frameVoltBCU1, "BCU1VolLed[1]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLed[1], 2, 2 );

    BCU1VolBut[1] = new QPushButton( frameVoltBCU1, "BCU1VolBut[1]" );
    BCU1VolBut[1]->setAutoDefault( FALSE );

    frameVoltBCU1Layout->addWidget( BCU1VolBut[1], 2, 3 );

    textLabelBCU1c_3 = new QLabel( frameVoltBCU1, "textLabelBCU1c_3" );

    frameVoltBCU1Layout->addWidget( textLabelBCU1c_3, 3, 0 );

    BCU1VolLab[2] = new QLabel( frameVoltBCU1, "BCU1VolLab[2]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLab[2], 3, 1 );

    BCU1VolLed[2] = new KLed( frameVoltBCU1, "BCU1VolLed[2]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLed[2], 3, 2 );

    BCU1VolBut[2] = new QPushButton( frameVoltBCU1, "BCU1VolBut[2]" );
    BCU1VolBut[2]->setAutoDefault( FALSE );

    frameVoltBCU1Layout->addWidget( BCU1VolBut[2], 3, 3 );

    textLabelBCU1c_4 = new QLabel( frameVoltBCU1, "textLabelBCU1c_4" );

    frameVoltBCU1Layout->addWidget( textLabelBCU1c_4, 4, 0 );

    BCU1VolLab[3] = new QLabel( frameVoltBCU1, "BCU1VolLab[3]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLab[3], 4, 1 );

    BCU1VolLed[3] = new KLed( frameVoltBCU1, "BCU1VolLed[3]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLed[3], 4, 2 );

    BCU1VolBut[3] = new QPushButton( frameVoltBCU1, "BCU1VolBut[3]" );
    BCU1VolBut[3]->setAutoDefault( FALSE );

    frameVoltBCU1Layout->addWidget( BCU1VolBut[3], 4, 3 );

    textLabelBCU1c_5 = new QLabel( frameVoltBCU1, "textLabelBCU1c_5" );

    frameVoltBCU1Layout->addWidget( textLabelBCU1c_5, 5, 0 );

    BCU1VolLab[4] = new QLabel( frameVoltBCU1, "BCU1VolLab[4]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLab[4], 5, 1 );

    BCU1VolLed[4] = new KLed( frameVoltBCU1, "BCU1VolLed[4]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLed[4], 5, 2 );

    BCU1VolBut[4] = new QPushButton( frameVoltBCU1, "BCU1VolBut[4]" );
    BCU1VolBut[4]->setAutoDefault( FALSE );

    frameVoltBCU1Layout->addWidget( BCU1VolBut[4], 5, 3 );

    textLabelBCU1c_6 = new QLabel( frameVoltBCU1, "textLabelBCU1c_6" );

    frameVoltBCU1Layout->addWidget( textLabelBCU1c_6, 6, 0 );

    BCU1VolLab[5] = new QLabel( frameVoltBCU1, "BCU1VolLab[5]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLab[5], 6, 1 );

    BCU1VolLed[5] = new KLed( frameVoltBCU1, "BCU1VolLed[5]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLed[5], 6, 2 );

    BCU1VolBut[5] = new QPushButton( frameVoltBCU1, "BCU1VolBut[5]" );
    BCU1VolBut[5]->setAutoDefault( FALSE );

    frameVoltBCU1Layout->addWidget( BCU1VolBut[5], 6, 3 );

    textLabelBCU1c_7 = new QLabel( frameVoltBCU1, "textLabelBCU1c_7" );

    frameVoltBCU1Layout->addWidget( textLabelBCU1c_7, 7, 0 );

    BCU1VolLab[6] = new QLabel( frameVoltBCU1, "BCU1VolLab[6]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLab[6], 7, 1 );

    BCU1VolLed[6] = new KLed( frameVoltBCU1, "BCU1VolLed[6]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLed[6], 7, 2 );

    BCU1VolBut[6] = new QPushButton( frameVoltBCU1, "BCU1VolBut[6]" );
    BCU1VolBut[6]->setAutoDefault( FALSE );

    frameVoltBCU1Layout->addWidget( BCU1VolBut[6], 7, 3 );

    textLabelBCU1c_8 = new QLabel( frameVoltBCU1, "textLabelBCU1c_8" );

    frameVoltBCU1Layout->addWidget( textLabelBCU1c_8, 8, 0 );

    BCU1VolLab[7] = new QLabel( frameVoltBCU1, "BCU1VolLab[7]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLab[7], 8, 1 );

    BCU1VolLed[7] = new KLed( frameVoltBCU1, "BCU1VolLed[7]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLed[7], 8, 2 );

    BCU1VolBut[7] = new QPushButton( frameVoltBCU1, "BCU1VolBut[7]" );
    BCU1VolBut[7]->setAutoDefault( FALSE );

    frameVoltBCU1Layout->addWidget( BCU1VolBut[7], 8, 3 );

    textLabelBCU1c_9 = new QLabel( frameVoltBCU1, "textLabelBCU1c_9" );

    frameVoltBCU1Layout->addWidget( textLabelBCU1c_9, 9, 0 );

    BCU1VolLab[8] = new QLabel( frameVoltBCU1, "BCU1VolLab[8]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLab[8], 9, 1 );

    BCU1VolLed[8] = new KLed( frameVoltBCU1, "BCU1VolLed[8]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLed[8], 9, 2 );

    BCU1VolBut[8] = new QPushButton( frameVoltBCU1, "BCU1VolBut[8]" );
    BCU1VolBut[8]->setAutoDefault( FALSE );

    frameVoltBCU1Layout->addWidget( BCU1VolBut[8], 9, 3 );

    textLabelBCU1c_10 = new QLabel( frameVoltBCU1, "textLabelBCU1c_10" );

    frameVoltBCU1Layout->addWidget( textLabelBCU1c_10, 10, 0 );

    BCU1VolLab[9] = new QLabel( frameVoltBCU1, "BCU1VolLab[9]" );
    for (i=0; i<10; i++) {
        BCU1VolLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU1VolLab[9]->sizePolicy().hasHeightForWidth() ) );
        BCU1VolLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU1VolLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU1VolLab[i]->setFrameShadow( QLabel::Sunken );
        BCU1VolLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameVoltBCU1Layout->addWidget( BCU1VolLab[9], 10, 1 );

    BCU1VolLed[9] = new KLed( frameVoltBCU1, "BCU1VolLed[9]" );

    frameVoltBCU1Layout->addWidget( BCU1VolLed[9], 10, 2 );

    BCU1VolBut[9] = new QPushButton( frameVoltBCU1, "BCU1VolBut[9]" );
    BCU1VolBut[9]->setAutoDefault( FALSE );

    frameVoltBCU1Layout->addWidget( BCU1VolBut[9], 10, 3 );

    unnamedLayout->addWidget( frameVoltBCU1, 0, 2 );

    frameTempSIGGEN = new QFrame( unnamed, "frameTempSIGGEN" );
    frameTempSIGGEN->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameTempSIGGEN->sizePolicy().hasHeightForWidth() ) );
    frameTempSIGGEN->setFrameShape( QFrame::TabWidgetPanel );
    frameTempSIGGEN->setFrameShadow( QFrame::Raised );
    frameTempSIGGENLayout = new QGridLayout( frameTempSIGGEN, 1, 1, 11, 6, "frameTempSIGGENLayout"); 

    labelTempSIGGENd = new QLabel( frameTempSIGGEN, "labelTempSIGGENd" );
    labelTempSIGGENd->setPaletteForegroundColor( QColor( 0, 85, 0 ) );
    QFont labelTempSIGGENd_font(  labelTempSIGGENd->font() );
    labelTempSIGGENd_font.setFamily( "Bitstream Charter" );
    labelTempSIGGENd_font.setPointSize( 12 );
    labelTempSIGGENd_font.setBold( TRUE );
    labelTempSIGGENd_font.setItalic( TRUE );
    labelTempSIGGENd->setFont( labelTempSIGGENd_font ); 

    frameTempSIGGENLayout->addWidget( labelTempSIGGENd, 0, 0 );

    labelTempSIGGENd_2 = new QLabel( frameTempSIGGEN, "labelTempSIGGENd_2" );
    labelTempSIGGENd_2->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelTempSIGGENd_2_font(  labelTempSIGGENd_2->font() );
    labelTempSIGGENd_2_font.setFamily( "Bitstream Charter" );
    labelTempSIGGENd_2_font.setBold( TRUE );
    labelTempSIGGENd_2_font.setItalic( TRUE );
    labelTempSIGGENd_2->setFont( labelTempSIGGENd_2_font ); 

    frameTempSIGGENLayout->addWidget( labelTempSIGGENd_2, 0, 1 );

    textLabelSIGGENd_1 = new QLabel( frameTempSIGGEN, "textLabelSIGGENd_1" );

    frameTempSIGGENLayout->addWidget( textLabelSIGGENd_1, 1, 0 );

    BCU1SGNTempLab[0] = new QLabel( frameTempSIGGEN, "BCU1SGNTempLab[0]" );

    frameTempSIGGENLayout->addWidget( BCU1SGNTempLab[0], 1, 1 );

    BCU1SGNTempLed[0] = new KLed( frameTempSIGGEN, "BCU1SGNTempLed[0]" );

    frameTempSIGGENLayout->addWidget( BCU1SGNTempLed[0], 1, 2 );

    BCU1SGNTempBut[0] = new QPushButton( frameTempSIGGEN, "BCU1SGNTempBut[0]" );
    BCU1SGNTempBut[0]->setAutoDefault( FALSE );

    frameTempSIGGENLayout->addWidget( BCU1SGNTempBut[0], 1, 3 );

    textLabelSIGGENd_2 = new QLabel( frameTempSIGGEN, "textLabelSIGGENd_2" );

    frameTempSIGGENLayout->addWidget( textLabelSIGGENd_2, 2, 0 );

    BCU1SGNTempLab[1] = new QLabel( frameTempSIGGEN, "BCU1SGNTempLab[1]" );

    frameTempSIGGENLayout->addWidget( BCU1SGNTempLab[1], 2, 1 );

    BCU1SGNTempLed[1] = new KLed( frameTempSIGGEN, "BCU1SGNTempLed[1]" );

    frameTempSIGGENLayout->addWidget( BCU1SGNTempLed[1], 2, 2 );

    BCU1SGNTempBut[1] = new QPushButton( frameTempSIGGEN, "BCU1SGNTempBut[1]" );
    BCU1SGNTempBut[1]->setAutoDefault( FALSE );

    frameTempSIGGENLayout->addWidget( BCU1SGNTempBut[1], 2, 3 );

    textLabelSIGGENd_3 = new QLabel( frameTempSIGGEN, "textLabelSIGGENd_3" );

    frameTempSIGGENLayout->addWidget( textLabelSIGGENd_3, 3, 0 );

    BCU1SGNTempLab[2] = new QLabel( frameTempSIGGEN, "BCU1SGNTempLab[2]" );
    for (i=0; i<3; i++) {
        BCU1SGNTempLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU1SGNTempLab[2]->sizePolicy().hasHeightForWidth() ) );
        BCU1SGNTempLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU1SGNTempLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU1SGNTempLab[i]->setFrameShadow( QLabel::Sunken );
        BCU1SGNTempLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameTempSIGGENLayout->addWidget( BCU1SGNTempLab[2], 3, 1 );

    BCU1SGNTempLed[2] = new KLed( frameTempSIGGEN, "BCU1SGNTempLed[2]" );

    frameTempSIGGENLayout->addWidget( BCU1SGNTempLed[2], 3, 2 );

    BCU1SGNTempBut[2] = new QPushButton( frameTempSIGGEN, "BCU1SGNTempBut[2]" );
    BCU1SGNTempBut[2]->setAutoDefault( FALSE );

    frameTempSIGGENLayout->addWidget( BCU1SGNTempBut[2], 3, 3 );

    unnamedLayout->addWidget( frameTempSIGGEN, 1, 2 );
    tabWidget->insertTab( unnamed, QString::fromLatin1("") );

    TabPage = new QWidget( tabWidget, "TabPage" );
    TabPageLayout = new QGridLayout( TabPage, 1, 1, 11, 6, "TabPageLayout"); 

    frameResets = new QFrame( TabPage, "frameResets" );
    frameResets->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameResets->sizePolicy().hasHeightForWidth() ) );
    frameResets->setFrameShape( QFrame::TabWidgetPanel );
    frameResets->setFrameShadow( QFrame::Raised );
    frameResetsLayout = new QGridLayout( frameResets, 1, 1, 11, 6, "frameResetsLayout"); 

    labelResets = new QLabel( frameResets, "labelResets" );
    labelResets->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelResets_font(  labelResets->font() );
    labelResets_font.setFamily( "Bitstream Charter" );
    labelResets_font.setBold( TRUE );
    labelResets_font.setItalic( TRUE );
    labelResets->setFont( labelResets_font ); 

    frameResetsLayout->addWidget( labelResets, 0, 0 );

    textLabelBCU2_1 = new QLabel( frameResets, "textLabelBCU2_1" );

    frameResetsLayout->addWidget( textLabelBCU2_1, 1, 0 );

    BCU2ResetLed[0] = new KLed( frameResets, "BCU2ResetLed[0]" );
    BCU2ResetLed[0]->setColor( QColor( 255, 255, 255 ) );

    frameResetsLayout->addWidget( BCU2ResetLed[0], 1, 1 );

    textLabelBCU2_2 = new QLabel( frameResets, "textLabelBCU2_2" );

    frameResetsLayout->addWidget( textLabelBCU2_2, 2, 0 );

    BCU2ResetLed[1] = new KLed( frameResets, "BCU2ResetLed[1]" );
    BCU2ResetLed[1]->setColor( QColor( 255, 255, 255 ) );

    frameResetsLayout->addWidget( BCU2ResetLed[1], 2, 1 );

    textLabelBCU2_3 = new QLabel( frameResets, "textLabelBCU2_3" );

    frameResetsLayout->addWidget( textLabelBCU2_3, 3, 0 );

    BCU2ResetLed[2] = new KLed( frameResets, "BCU2ResetLed[2]" );
    BCU2ResetLed[2]->setColor( QColor( 255, 255, 255 ) );

    frameResetsLayout->addWidget( BCU2ResetLed[2], 3, 1 );

    textLabelBCU2_4 = new QLabel( frameResets, "textLabelBCU2_4" );

    frameResetsLayout->addWidget( textLabelBCU2_4, 4, 0 );

    BCU2ResetLed[3] = new KLed( frameResets, "BCU2ResetLed[3]" );
    BCU2ResetLed[3]->setColor( QColor( 255, 255, 255 ) );

    frameResetsLayout->addWidget( BCU2ResetLed[3], 4, 1 );

    textLabelBCU2_5 = new QLabel( frameResets, "textLabelBCU2_5" );

    frameResetsLayout->addWidget( textLabelBCU2_5, 5, 0 );

    BCU2ResetLed[4] = new KLed( frameResets, "BCU2ResetLed[4]" );
    BCU2ResetLed[4]->setColor( QColor( 255, 255, 255 ) );

    frameResetsLayout->addWidget( BCU2ResetLed[4], 5, 1 );

    textLabelBCU2_6 = new QLabel( frameResets, "textLabelBCU2_6" );

    frameResetsLayout->addWidget( textLabelBCU2_6, 6, 0 );

    BCU2ResetLed[5] = new KLed( frameResets, "BCU2ResetLed[5]" );
    BCU2ResetLed[5]->setColor( QColor( 255, 255, 255 ) );

    frameResetsLayout->addWidget( BCU2ResetLed[5], 6, 1 );

    textLabelBCU2_7 = new QLabel( frameResets, "textLabelBCU2_7" );

    frameResetsLayout->addWidget( textLabelBCU2_7, 7, 0 );

    BCU2ResetLed[6] = new KLed( frameResets, "BCU2ResetLed[6]" );
    BCU2ResetLed[6]->setColor( QColor( 255, 255, 255 ) );

    frameResetsLayout->addWidget( BCU2ResetLed[6], 7, 1 );

    textLabelBCU2_8 = new QLabel( frameResets, "textLabelBCU2_8" );

    frameResetsLayout->addWidget( textLabelBCU2_8, 8, 0 );

    BCU2ResetLed[7] = new KLed( frameResets, "BCU2ResetLed[7]" );
    BCU2ResetLed[7]->setColor( QColor( 255, 255, 255 ) );

    frameResetsLayout->addWidget( BCU2ResetLed[7], 8, 1 );

    textLabelBCU2_9 = new QLabel( frameResets, "textLabelBCU2_9" );

    frameResetsLayout->addWidget( textLabelBCU2_9, 9, 0 );

    BCU2ResetLed[8] = new KLed( frameResets, "BCU2ResetLed[8]" );
    BCU2ResetLed[8]->setColor( QColor( 255, 255, 255 ) );

    frameResetsLayout->addWidget( BCU2ResetLed[8], 9, 1 );

    textLabelBCU2_10 = new QLabel( frameResets, "textLabelBCU2_10" );

    frameResetsLayout->addWidget( textLabelBCU2_10, 10, 0 );

    BCU2ResetLed[9] = new KLed( frameResets, "BCU2ResetLed[9]" );
    BCU2ResetLed[9]->setColor( QColor( 255, 255, 255 ) );

    frameResetsLayout->addWidget( BCU2ResetLed[9], 10, 1 );

    TabPageLayout->addWidget( frameResets, 0, 0 );

    crateNumber2 = new QFrame( TabPage, "crateNumber2" );
    crateNumber2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, crateNumber2->sizePolicy().hasHeightForWidth() ) );
    crateNumber2->setFrameShape( QFrame::TabWidgetPanel );
    crateNumber2->setFrameShadow( QFrame::Raised );
    crateNumber2Layout = new QGridLayout( crateNumber2, 1, 1, 11, 6, "crateNumber2Layout"); 

    crateBCU2_gr = new QButtonGroup( crateNumber2, "crateBCU2_gr" );
    crateBCU2_gr->setPaletteForegroundColor( QColor( 85, 0, 127 ) );
    crateBCU2_gr->setPaletteBackgroundColor( QColor( 166, 192, 163 ) );
    QFont crateBCU2_gr_font(  crateBCU2_gr->font() );
    crateBCU2_gr_font.setFamily( "Bitstream Charter" );
    crateBCU2_gr_font.setBold( TRUE );
    crateBCU2_gr_font.setItalic( TRUE );
    crateBCU2_gr->setFont( crateBCU2_gr_font ); 
    crateBCU2_gr->setColumnLayout(0, Qt::Vertical );
    crateBCU2_gr->layout()->setSpacing( 6 );
    crateBCU2_gr->layout()->setMargin( 0 );
    crateBCU2_grLayout = new QHBoxLayout( crateBCU2_gr->layout() );
    crateBCU2_grLayout->setAlignment( Qt::AlignTop );

    BCU2RBut[0] = new QRadioButton( crateBCU2_gr, "BCU2RBut[0]" );
    BCU2RBut[0]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    BCU2RBut[0]->setChecked( TRUE );
    crateBCU2_grLayout->addWidget( BCU2RBut[0] );

    BCU2RBut[1] = new QRadioButton( crateBCU2_gr, "BCU2RBut[1]" );
    BCU2RBut[1]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateBCU2_grLayout->addWidget( BCU2RBut[1] );

    BCU2RBut[2] = new QRadioButton( crateBCU2_gr, "BCU2RBut[2]" );
    BCU2RBut[2]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateBCU2_grLayout->addWidget( BCU2RBut[2] );

    BCU2RBut[3] = new QRadioButton( crateBCU2_gr, "BCU2RBut[3]" );
    BCU2RBut[3]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateBCU2_grLayout->addWidget( BCU2RBut[3] );

    BCU2RBut[4] = new QRadioButton( crateBCU2_gr, "BCU2RBut[4]" );
    BCU2RBut[4]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateBCU2_grLayout->addWidget( BCU2RBut[4] );

    BCU2RBut[5] = new QRadioButton( crateBCU2_gr, "BCU2RBut[5]" );
    BCU2RBut[5]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateBCU2_grLayout->addWidget( BCU2RBut[5] );

    crateNumber2Layout->addWidget( crateBCU2_gr, 0, 0 );

    TabPageLayout->addWidget( crateNumber2, 1, 0 );

    frameDigitalIO = new QFrame( TabPage, "frameDigitalIO" );
    frameDigitalIO->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameDigitalIO->sizePolicy().hasHeightForWidth() ) );
    frameDigitalIO->setFrameShape( QFrame::TabWidgetPanel );
    frameDigitalIO->setFrameShadow( QFrame::Raised );
    frameDigitalIOLayout = new QGridLayout( frameDigitalIO, 1, 1, 11, 6, "frameDigitalIOLayout"); 

    labelDigital = new QLabel( frameDigitalIO, "labelDigital" );
    labelDigital->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelDigital_font(  labelDigital->font() );
    labelDigital_font.setFamily( "Bitstream Charter" );
    labelDigital_font.setBold( TRUE );
    labelDigital_font.setItalic( TRUE );
    labelDigital->setFont( labelDigital_font ); 

    frameDigitalIOLayout->addWidget( labelDigital, 0, 0 );

    textLabelBCU2a_1 = new QLabel( frameDigitalIO, "textLabelBCU2a_1" );

    frameDigitalIOLayout->addWidget( textLabelBCU2a_1, 1, 0 );

    BCU2DigLed[0] = new KLed( frameDigitalIO, "BCU2DigLed[0]" );
    BCU2DigLed[0]->setColor( QColor( 255, 255, 255 ) );

    frameDigitalIOLayout->addWidget( BCU2DigLed[0], 1, 1 );

    textLabelBCU2a_2 = new QLabel( frameDigitalIO, "textLabelBCU2a_2" );

    frameDigitalIOLayout->addWidget( textLabelBCU2a_2, 2, 0 );

    BCU2DigLed[1] = new KLed( frameDigitalIO, "BCU2DigLed[1]" );
    BCU2DigLed[1]->setColor( QColor( 255, 255, 255 ) );

    frameDigitalIOLayout->addWidget( BCU2DigLed[1], 2, 1 );

    textLabelBCU2a_3 = new QLabel( frameDigitalIO, "textLabelBCU2a_3" );

    frameDigitalIOLayout->addWidget( textLabelBCU2a_3, 3, 0 );

    BCU2DigLed[2] = new KLed( frameDigitalIO, "BCU2DigLed[2]" );
    BCU2DigLed[2]->setColor( QColor( 255, 255, 255 ) );

    frameDigitalIOLayout->addWidget( BCU2DigLed[2], 3, 1 );

    textLabelBCU2a_4 = new QLabel( frameDigitalIO, "textLabelBCU2a_4" );

    frameDigitalIOLayout->addWidget( textLabelBCU2a_4, 4, 0 );

    BCU2DigLed[3] = new KLed( frameDigitalIO, "BCU2DigLed[3]" );
    BCU2DigLed[3]->setColor( QColor( 255, 255, 255 ) );

    frameDigitalIOLayout->addWidget( BCU2DigLed[3], 4, 1 );

    textLabelBCU2a_5 = new QLabel( frameDigitalIO, "textLabelBCU2a_5" );

    frameDigitalIOLayout->addWidget( textLabelBCU2a_5, 5, 0 );

    BCU2DigLed[4] = new KLed( frameDigitalIO, "BCU2DigLed[4]" );
    BCU2DigLed[4]->setColor( QColor( 255, 255, 255 ) );

    frameDigitalIOLayout->addWidget( BCU2DigLed[4], 5, 1 );

    textLabelBCU2a_6 = new QLabel( frameDigitalIO, "textLabelBCU2a_6" );

    frameDigitalIOLayout->addWidget( textLabelBCU2a_6, 6, 0 );

    BCU2DigLed[5] = new KLed( frameDigitalIO, "BCU2DigLed[5]" );
    BCU2DigLed[5]->setColor( QColor( 255, 255, 255 ) );

    frameDigitalIOLayout->addWidget( BCU2DigLed[5], 6, 1 );

    textLabelBCU2a_7 = new QLabel( frameDigitalIO, "textLabelBCU2a_7" );

    frameDigitalIOLayout->addWidget( textLabelBCU2a_7, 7, 0 );

    BCU2DigLed[6] = new KLed( frameDigitalIO, "BCU2DigLed[6]" );
    BCU2DigLed[6]->setColor( QColor( 255, 255, 255 ) );

    frameDigitalIOLayout->addWidget( BCU2DigLed[6], 7, 1 );

    textLabelBCU2a_8 = new QLabel( frameDigitalIO, "textLabelBCU2a_8" );

    frameDigitalIOLayout->addWidget( textLabelBCU2a_8, 8, 0 );

    BCU2DigLed[7] = new KLed( frameDigitalIO, "BCU2DigLed[7]" );
    BCU2DigLed[7]->setColor( QColor( 255, 255, 255 ) );

    frameDigitalIOLayout->addWidget( BCU2DigLed[7], 8, 1 );

    textLabelBCU2a_9 = new QLabel( frameDigitalIO, "textLabelBCU2a_9" );

    frameDigitalIOLayout->addWidget( textLabelBCU2a_9, 9, 0 );

    BCU2DigLed[8] = new KLed( frameDigitalIO, "BCU2DigLed[8]" );
    BCU2DigLed[8]->setColor( QColor( 255, 255, 255 ) );

    frameDigitalIOLayout->addWidget( BCU2DigLed[8], 9, 1 );

    textLabelBCU2a_10 = new QLabel( frameDigitalIO, "textLabelBCU2a_10" );

    frameDigitalIOLayout->addWidget( textLabelBCU2a_10, 10, 0 );

    BCU2DigLed[9] = new KLed( frameDigitalIO, "BCU2DigLed[9]" );
    BCU2DigLed[9]->setColor( QColor( 255, 255, 255 ) );

    frameDigitalIOLayout->addWidget( BCU2DigLed[9], 10, 1 );

    TabPageLayout->addWidget( frameDigitalIO, 0, 1 );

    frameSIGGENa = new QFrame( TabPage, "frameSIGGENa" );
    frameSIGGENa->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameSIGGENa->sizePolicy().hasHeightForWidth() ) );
    frameSIGGENa->setFrameShape( QFrame::TabWidgetPanel );
    frameSIGGENa->setFrameShadow( QFrame::Raised );
    frameSIGGENaLayout = new QGridLayout( frameSIGGENa, 1, 1, 11, 6, "frameSIGGENaLayout"); 

    labelDiagn_2 = new QLabel( frameSIGGENa, "labelDiagn_2" );
    labelDiagn_2->setPaletteForegroundColor( QColor( 0, 85, 0 ) );
    QFont labelDiagn_2_font(  labelDiagn_2->font() );
    labelDiagn_2_font.setFamily( "Bitstream Charter" );
    labelDiagn_2_font.setPointSize( 12 );
    labelDiagn_2_font.setBold( TRUE );
    labelDiagn_2_font.setItalic( TRUE );
    labelDiagn_2->setFont( labelDiagn_2_font ); 

    frameSIGGENaLayout->addWidget( labelDiagn_2, 0, 0 );

    labelDiagn_3 = new QLabel( frameSIGGENa, "labelDiagn_3" );
    labelDiagn_3->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelDiagn_3_font(  labelDiagn_3->font() );
    labelDiagn_3_font.setFamily( "Bitstream Charter" );
    labelDiagn_3_font.setBold( TRUE );
    labelDiagn_3_font.setItalic( TRUE );
    labelDiagn_3->setFont( labelDiagn_3_font ); 

    frameSIGGENaLayout->addWidget( labelDiagn_3, 0, 1 );

    textLabelSIGGENa_1 = new QLabel( frameSIGGENa, "textLabelSIGGENa_1" );

    frameSIGGENaLayout->addWidget( textLabelSIGGENa_1, 1, 0 );

    BCU2SGNLab[0] = new QLabel( frameSIGGENa, "BCU2SGNLab[0]" );

    frameSIGGENaLayout->addWidget( BCU2SGNLab[0], 1, 1 );

    BCU2SGNLed[0] = new KLed( frameSIGGENa, "BCU2SGNLed[0]" );

    frameSIGGENaLayout->addWidget( BCU2SGNLed[0], 1, 2 );

    textLabelSIGGENa_2 = new QLabel( frameSIGGENa, "textLabelSIGGENa_2" );

    frameSIGGENaLayout->addWidget( textLabelSIGGENa_2, 2, 0 );

    BCU2SGNLab[1] = new QLabel( frameSIGGENa, "BCU2SGNLab[1]" );

    frameSIGGENaLayout->addWidget( BCU2SGNLab[1], 2, 1 );

    BCU2SGNLed[1] = new KLed( frameSIGGENa, "BCU2SGNLed[1]" );

    frameSIGGENaLayout->addWidget( BCU2SGNLed[1], 2, 2 );

    textLabelSIGGENa_3 = new QLabel( frameSIGGENa, "textLabelSIGGENa_3" );

    frameSIGGENaLayout->addWidget( textLabelSIGGENa_3, 3, 0 );

    BCU2SGNLab[2] = new QLabel( frameSIGGENa, "BCU2SGNLab[2]" );

    frameSIGGENaLayout->addWidget( BCU2SGNLab[2], 3, 1 );

    BCU2SGNLed[2] = new KLed( frameSIGGENa, "BCU2SGNLed[2]" );

    frameSIGGENaLayout->addWidget( BCU2SGNLed[2], 3, 2 );

    textLabelSIGGENa_4 = new QLabel( frameSIGGENa, "textLabelSIGGENa_4" );

    frameSIGGENaLayout->addWidget( textLabelSIGGENa_4, 4, 0 );

    BCU2SGNLab[3] = new QLabel( frameSIGGENa, "BCU2SGNLab[3]" );
    for (i=0; i<4; i++) {
        BCU2SGNLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU2SGNLab[3]->sizePolicy().hasHeightForWidth() ) );
        BCU2SGNLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU2SGNLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU2SGNLab[i]->setFrameShadow( QLabel::Sunken );
        BCU2SGNLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameSIGGENaLayout->addWidget( BCU2SGNLab[3], 4, 1 );

    BCU2SGNLed[3] = new KLed( frameSIGGENa, "BCU2SGNLed[3]" );

    frameSIGGENaLayout->addWidget( BCU2SGNLed[3], 4, 2 );

    TabPageLayout->addWidget( frameSIGGENa, 1, 1 );

    frameMasterBCU2 = new QFrame( TabPage, "frameMasterBCU2" );
    frameMasterBCU2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameMasterBCU2->sizePolicy().hasHeightForWidth() ) );
    frameMasterBCU2->setFrameShape( QFrame::TabWidgetPanel );
    frameMasterBCU2->setFrameShadow( QFrame::Raised );
    frameMasterBCU2Layout = new QGridLayout( frameMasterBCU2, 1, 1, 11, 6, "frameMasterBCU2Layout"); 

    labelEnvBCU2 = new QLabel( frameMasterBCU2, "labelEnvBCU2" );
    labelEnvBCU2->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelEnvBCU2_font(  labelEnvBCU2->font() );
    labelEnvBCU2_font.setFamily( "Bitstream Charter" );
    labelEnvBCU2_font.setBold( TRUE );
    labelEnvBCU2_font.setItalic( TRUE );
    labelEnvBCU2->setFont( labelEnvBCU2_font ); 

    frameMasterBCU2Layout->addWidget( labelEnvBCU2, 0, 1 );

    textLabelBCU2b_1 = new QLabel( frameMasterBCU2, "textLabelBCU2b_1" );

    frameMasterBCU2Layout->addWidget( textLabelBCU2b_1, 1, 0 );

    BCU2MastLab[0] = new QLabel( frameMasterBCU2, "BCU2MastLab[0]" );

    frameMasterBCU2Layout->addWidget( BCU2MastLab[0], 1, 1 );

    BCU2MastLed[0] = new KLed( frameMasterBCU2, "BCU2MastLed[0]" );

    frameMasterBCU2Layout->addWidget( BCU2MastLed[0], 1, 2 );

    BCU2MastBut[0] = new QPushButton( frameMasterBCU2, "BCU2MastBut[0]" );
    BCU2MastBut[0]->setAutoDefault( FALSE );

    frameMasterBCU2Layout->addWidget( BCU2MastBut[0], 1, 3 );

    textLabelBCU2b_2 = new QLabel( frameMasterBCU2, "textLabelBCU2b_2" );

    frameMasterBCU2Layout->addWidget( textLabelBCU2b_2, 2, 0 );

    BCU2MastLab[1] = new QLabel( frameMasterBCU2, "BCU2MastLab[1]" );

    frameMasterBCU2Layout->addWidget( BCU2MastLab[1], 2, 1 );

    BCU2MastLed[1] = new KLed( frameMasterBCU2, "BCU2MastLed[1]" );

    frameMasterBCU2Layout->addWidget( BCU2MastLed[1], 2, 2 );

    BCU2MastBut[1] = new QPushButton( frameMasterBCU2, "BCU2MastBut[1]" );
    BCU2MastBut[1]->setAutoDefault( FALSE );

    frameMasterBCU2Layout->addWidget( BCU2MastBut[1], 2, 3 );

    textLabelBCU2b_3 = new QLabel( frameMasterBCU2, "textLabelBCU2b_3" );

    frameMasterBCU2Layout->addWidget( textLabelBCU2b_3, 3, 0 );

    BCU2MastLab[2] = new QLabel( frameMasterBCU2, "BCU2MastLab[2]" );
    for (i=0; i<3; i++) {
        BCU2MastLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU2MastLab[2]->sizePolicy().hasHeightForWidth() ) );
        BCU2MastLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU2MastLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU2MastLab[i]->setFrameShadow( QLabel::Sunken );
        BCU2MastLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameMasterBCU2Layout->addWidget( BCU2MastLab[2], 3, 1 );

    BCU2MastLed[2] = new KLed( frameMasterBCU2, "BCU2MastLed[2]" );

    frameMasterBCU2Layout->addWidget( BCU2MastLed[2], 3, 2 );

    BCU2MastBut[2] = new QPushButton( frameMasterBCU2, "BCU2MastBut[2]" );
    BCU2MastBut[2]->setAutoDefault( FALSE );

    frameMasterBCU2Layout->addWidget( BCU2MastBut[2], 3, 3 );

    TabPageLayout->addWidget( frameMasterBCU2, 0, 2 );

    frameBCU2c = new QFrame( TabPage, "frameBCU2c" );
    frameBCU2c->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameBCU2c->sizePolicy().hasHeightForWidth() ) );
    frameBCU2c->setFrameShape( QFrame::TabWidgetPanel );
    frameBCU2c->setFrameShadow( QFrame::Raised );
    frameBCU2cLayout = new QGridLayout( frameBCU2c, 1, 1, 11, 6, "frameBCU2cLayout"); 

    textLabelBCU2c_0 = new QLabel( frameBCU2c, "textLabelBCU2c_0" );

    frameBCU2cLayout->addWidget( textLabelBCU2c_0, 0, 0 );

    BCU2TrLab[0] = new QLabel( frameBCU2c, "BCU2TrLab[0]" );

    frameBCU2cLayout->addWidget( BCU2TrLab[0], 0, 1 );

    BCU2TrLed[0] = new KLed( frameBCU2c, "BCU2TrLed[0]" );

    frameBCU2cLayout->addWidget( BCU2TrLed[0], 0, 2 );

    BCU2TrBut[0] = new QPushButton( frameBCU2c, "BCU2TrBut[0]" );
    BCU2TrBut[0]->setAutoDefault( FALSE );

    frameBCU2cLayout->addWidget( BCU2TrBut[0], 0, 3 );

    textLabelBCU2c_1 = new QLabel( frameBCU2c, "textLabelBCU2c_1" );

    frameBCU2cLayout->addWidget( textLabelBCU2c_1, 1, 0 );

    BCU2TrLab[1] = new QLabel( frameBCU2c, "BCU2TrLab[1]" );

    frameBCU2cLayout->addWidget( BCU2TrLab[1], 1, 1 );

    BCU2TrLed[1] = new KLed( frameBCU2c, "BCU2TrLed[1]" );

    frameBCU2cLayout->addWidget( BCU2TrLed[1], 1, 2 );

    BCU2TrBut[1] = new QPushButton( frameBCU2c, "BCU2TrBut[1]" );
    BCU2TrBut[1]->setAutoDefault( FALSE );

    frameBCU2cLayout->addWidget( BCU2TrBut[1], 1, 3 );

    textLabelBCU2c_2 = new QLabel( frameBCU2c, "textLabelBCU2c_2" );

    frameBCU2cLayout->addWidget( textLabelBCU2c_2, 2, 0 );

    BCU2TrLab[2] = new QLabel( frameBCU2c, "BCU2TrLab[2]" );

    frameBCU2cLayout->addWidget( BCU2TrLab[2], 2, 1 );

    BCU2TrLed[2] = new KLed( frameBCU2c, "BCU2TrLed[2]" );

    frameBCU2cLayout->addWidget( BCU2TrLed[2], 2, 2 );

    BCU2TrBut[2] = new QPushButton( frameBCU2c, "BCU2TrBut[2]" );
    BCU2TrBut[2]->setAutoDefault( FALSE );

    frameBCU2cLayout->addWidget( BCU2TrBut[2], 2, 3 );

    textLabelBCU2c_3 = new QLabel( frameBCU2c, "textLabelBCU2c_3" );

    frameBCU2cLayout->addWidget( textLabelBCU2c_3, 3, 0 );

    BCU2TrLab[3] = new QLabel( frameBCU2c, "BCU2TrLab[3]" );
    for (i=0; i<4; i++) {
        BCU2TrLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU2TrLab[3]->sizePolicy().hasHeightForWidth() ) );
        BCU2TrLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU2TrLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU2TrLab[i]->setFrameShadow( QLabel::Sunken );
        BCU2TrLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameBCU2cLayout->addWidget( BCU2TrLab[3], 3, 1 );

    BCU2TrLed[3] = new KLed( frameBCU2c, "BCU2TrLed[3]" );

    frameBCU2cLayout->addWidget( BCU2TrLed[3], 3, 2 );

    BCU2TrBut[3] = new QPushButton( frameBCU2c, "BCU2TrBut[3]" );
    BCU2TrBut[3]->setAutoDefault( FALSE );

    frameBCU2cLayout->addWidget( BCU2TrBut[3], 3, 3 );

    TabPageLayout->addWidget( frameBCU2c, 1, 2 );
    tabWidget->insertTab( TabPage, QString::fromLatin1("") );

    TabPage_2 = new QWidget( tabWidget, "TabPage_2" );
    TabPageLayout_2 = new QGridLayout( TabPage_2, 1, 1, 11, 6, "TabPageLayout_2"); 

    frameDSPa = new QFrame( TabPage_2, "frameDSPa" );
    frameDSPa->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameDSPa->sizePolicy().hasHeightForWidth() ) );
    frameDSPa->setFrameShape( QFrame::TabWidgetPanel );
    frameDSPa->setFrameShadow( QFrame::Raised );
    frameDSPaLayout = new QGridLayout( frameDSPa, 1, 1, 11, 6, "frameDSPaLayout"); 

    labelDiagn_4 = new QLabel( frameDSPa, "labelDiagn_4" );
    labelDiagn_4->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelDiagn_4_font(  labelDiagn_4->font() );
    labelDiagn_4_font.setFamily( "Bitstream Charter" );
    labelDiagn_4_font.setBold( TRUE );
    labelDiagn_4_font.setItalic( TRUE );
    labelDiagn_4->setFont( labelDiagn_4_font ); 

    frameDSPaLayout->addWidget( labelDiagn_4, 0, 1 );

    textLabelDSPa_1 = new QLabel( frameDSPa, "textLabelDSPa_1" );

    frameDSPaLayout->addWidget( textLabelDSPa_1, 1, 0 );

    DSPNiosLab[0] = new QLabel( frameDSPa, "DSPNiosLab[0]" );

    frameDSPaLayout->addWidget( DSPNiosLab[0], 1, 1 );

    DSPNiosLed[0] = new KLed( frameDSPa, "DSPNiosLed[0]" );

    frameDSPaLayout->addWidget( DSPNiosLed[0], 1, 2 );

    textLabelDSPa_2 = new QLabel( frameDSPa, "textLabelDSPa_2" );

    frameDSPaLayout->addWidget( textLabelDSPa_2, 2, 0 );

    DSPNiosLab[1] = new QLabel( frameDSPa, "DSPNiosLab[1]" );

    frameDSPaLayout->addWidget( DSPNiosLab[1], 2, 1 );

    DSPNiosLed[1] = new KLed( frameDSPa, "DSPNiosLed[1]" );

    frameDSPaLayout->addWidget( DSPNiosLed[1], 2, 2 );

    textLabelDSPa_3 = new QLabel( frameDSPa, "textLabelDSPa_3" );

    frameDSPaLayout->addWidget( textLabelDSPa_3, 3, 0 );

    DSPNiosLab[2] = new QLabel( frameDSPa, "DSPNiosLab[2]" );

    frameDSPaLayout->addWidget( DSPNiosLab[2], 3, 1 );

    DSPNiosLed[2] = new KLed( frameDSPa, "DSPNiosLed[2]" );

    frameDSPaLayout->addWidget( DSPNiosLed[2], 3, 2 );

    textLabelDSPa_4 = new QLabel( frameDSPa, "textLabelDSPa_4" );

    frameDSPaLayout->addWidget( textLabelDSPa_4, 4, 0 );

    DSPNiosLab[3] = new QLabel( frameDSPa, "DSPNiosLab[3]" );

    frameDSPaLayout->addWidget( DSPNiosLab[3], 4, 1 );

    DSPNiosLed[3] = new KLed( frameDSPa, "DSPNiosLed[3]" );

    frameDSPaLayout->addWidget( DSPNiosLed[3], 4, 2 );

    textLabelDSPa_5 = new QLabel( frameDSPa, "textLabelDSPa_5" );

    frameDSPaLayout->addWidget( textLabelDSPa_5, 5, 0 );

    DSPNiosLab[4] = new QLabel( frameDSPa, "DSPNiosLab[4]" );

    frameDSPaLayout->addWidget( DSPNiosLab[4], 5, 1 );

    DSPNiosLed[4] = new KLed( frameDSPa, "DSPNiosLed[4]" );

    frameDSPaLayout->addWidget( DSPNiosLed[4], 5, 2 );

    textLabelDSPa_6 = new QLabel( frameDSPa, "textLabelDSPa_6" );

    frameDSPaLayout->addWidget( textLabelDSPa_6, 6, 0 );

    DSPNiosLab[5] = new QLabel( frameDSPa, "DSPNiosLab[5]" );

    frameDSPaLayout->addWidget( DSPNiosLab[5], 6, 1 );

    DSPNiosLed[5] = new KLed( frameDSPa, "DSPNiosLed[5]" );

    frameDSPaLayout->addWidget( DSPNiosLed[5], 6, 2 );

    textLabelDSPa_7 = new QLabel( frameDSPa, "textLabelDSPa_7" );

    frameDSPaLayout->addWidget( textLabelDSPa_7, 7, 0 );

    DSPNiosLab[6] = new QLabel( frameDSPa, "DSPNiosLab[6]" );

    frameDSPaLayout->addWidget( DSPNiosLab[6], 7, 1 );

    DSPNiosLed[6] = new KLed( frameDSPa, "DSPNiosLed[6]" );

    frameDSPaLayout->addWidget( DSPNiosLed[6], 7, 2 );

    textLabelDSPa_8 = new QLabel( frameDSPa, "textLabelDSPa_8" );

    frameDSPaLayout->addWidget( textLabelDSPa_8, 8, 0 );

    DSPNiosLab[7] = new QLabel( frameDSPa, "DSPNiosLab[7]" );
    for (i=0; i<8; i++) {
        DSPNiosLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, DSPNiosLab[7]->sizePolicy().hasHeightForWidth() ) );
        DSPNiosLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        DSPNiosLab[i]->setFrameShape( QLabel::LineEditPanel );
        DSPNiosLab[i]->setFrameShadow( QLabel::Sunken );
        DSPNiosLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameDSPaLayout->addWidget( DSPNiosLab[7], 8, 1 );

    DSPNiosLed[7] = new KLed( frameDSPa, "DSPNiosLed[7]" );

    frameDSPaLayout->addWidget( DSPNiosLed[7], 8, 2 );

    TabPageLayout_2->addWidget( frameDSPa, 0, 0 );

    QWidget3 = new QWidget( TabPage_2, "QWidget3" );
    QWidget3Layout = new QVBoxLayout( QWidget3, 11, 6, "QWidget3Layout"); 

    frameTempDSP = new QFrame( QWidget3, "frameTempDSP" );
    frameTempDSP->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameTempDSP->sizePolicy().hasHeightForWidth() ) );
    frameTempDSP->setFrameShape( QFrame::TabWidgetPanel );
    frameTempDSP->setFrameShadow( QFrame::Raised );
    frameTempDSPLayout = new QGridLayout( frameTempDSP, 1, 1, 11, 6, "frameTempDSPLayout"); 

    labelTempDSPd = new QLabel( frameTempDSP, "labelTempDSPd" );
    labelTempDSPd->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelTempDSPd_font(  labelTempDSPd->font() );
    labelTempDSPd_font.setFamily( "Bitstream Charter" );
    labelTempDSPd_font.setBold( TRUE );
    labelTempDSPd_font.setItalic( TRUE );
    labelTempDSPd->setFont( labelTempDSPd_font ); 

    frameTempDSPLayout->addWidget( labelTempDSPd, 0, 1 );

    textLabelDSPd_1 = new QLabel( frameTempDSP, "textLabelDSPd_1" );

    frameTempDSPLayout->addWidget( textLabelDSPd_1, 1, 0 );

    DSPTempLab[0] = new QLabel( frameTempDSP, "DSPTempLab[0]" );

    frameTempDSPLayout->addWidget( DSPTempLab[0], 1, 1 );

    DSPTempLed[0] = new KLed( frameTempDSP, "DSPTempLed[0]" );

    frameTempDSPLayout->addWidget( DSPTempLed[0], 1, 2 );

    DSPTempBut[0] = new QPushButton( frameTempDSP, "DSPTempBut[0]" );
    DSPTempBut[0]->setAutoDefault( FALSE );

    frameTempDSPLayout->addWidget( DSPTempBut[0], 1, 3 );

    textLabelDSPd_2 = new QLabel( frameTempDSP, "textLabelDSPd_2" );

    frameTempDSPLayout->addWidget( textLabelDSPd_2, 2, 0 );

    DSPTempLab[1] = new QLabel( frameTempDSP, "DSPTempLab[1]" );

    frameTempDSPLayout->addWidget( DSPTempLab[1], 2, 1 );

    DSPTempLed[1] = new KLed( frameTempDSP, "DSPTempLed[1]" );

    frameTempDSPLayout->addWidget( DSPTempLed[1], 2, 2 );

    DSPTempBut[1] = new QPushButton( frameTempDSP, "DSPTempBut[1]" );
    DSPTempBut[1]->setAutoDefault( FALSE );

    frameTempDSPLayout->addWidget( DSPTempBut[1], 2, 3 );

    textLabelDSPd_3 = new QLabel( frameTempDSP, "textLabelDSPd_3" );

    frameTempDSPLayout->addWidget( textLabelDSPd_3, 3, 0 );

    DSPTempLab[2] = new QLabel( frameTempDSP, "DSPTempLab[2]" );

    frameTempDSPLayout->addWidget( DSPTempLab[2], 3, 1 );

    DSPTempLed[2] = new KLed( frameTempDSP, "DSPTempLed[2]" );

    frameTempDSPLayout->addWidget( DSPTempLed[2], 3, 2 );

    DSPTempBut[2] = new QPushButton( frameTempDSP, "DSPTempBut[2]" );
    DSPTempBut[2]->setAutoDefault( FALSE );

    frameTempDSPLayout->addWidget( DSPTempBut[2], 3, 3 );

    textLabelDSPd_4 = new QLabel( frameTempDSP, "textLabelDSPd_4" );

    frameTempDSPLayout->addWidget( textLabelDSPd_4, 4, 0 );

    DSPTempLab[3] = new QLabel( frameTempDSP, "DSPTempLab[3]" );
    for (i=0; i<4; i++) {
        DSPTempLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, DSPTempLab[3]->sizePolicy().hasHeightForWidth() ) );
        DSPTempLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        DSPTempLab[i]->setFrameShape( QLabel::LineEditPanel );
        DSPTempLab[i]->setFrameShadow( QLabel::Sunken );
        DSPTempLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameTempDSPLayout->addWidget( DSPTempLab[3], 4, 1 );

    DSPTempLed[3] = new KLed( frameTempDSP, "DSPTempLed[3]" );

    frameTempDSPLayout->addWidget( DSPTempLed[3], 4, 2 );

    DSPTempBut[3] = new QPushButton( frameTempDSP, "DSPTempBut[3]" );
    DSPTempBut[3]->setAutoDefault( FALSE );

    frameTempDSPLayout->addWidget( DSPTempBut[3], 4, 3 );
    QWidget3Layout->addWidget( frameTempDSP );

    frameSIGGENb = new QFrame( QWidget3, "frameSIGGENb" );
    frameSIGGENb->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameSIGGENb->sizePolicy().hasHeightForWidth() ) );
    frameSIGGENb->setFrameShape( QFrame::TabWidgetPanel );
    frameSIGGENb->setFrameShadow( QFrame::Raised );
    frameSIGGENbLayout = new QGridLayout( frameSIGGENb, 1, 1, 11, 6, "frameSIGGENbLayout"); 

    crateSIGGEN_gr = new QButtonGroup( frameSIGGENb, "crateSIGGEN_gr" );
    crateSIGGEN_gr->setPaletteForegroundColor( QColor( 85, 0, 127 ) );
    crateSIGGEN_gr->setPaletteBackgroundColor( QColor( 150, 192, 150 ) );
    QFont crateSIGGEN_gr_font(  crateSIGGEN_gr->font() );
    crateSIGGEN_gr_font.setFamily( "Bitstream Charter" );
    crateSIGGEN_gr_font.setBold( TRUE );
    crateSIGGEN_gr_font.setItalic( TRUE );
    crateSIGGEN_gr->setFont( crateSIGGEN_gr_font ); 
    crateSIGGEN_gr->setColumnLayout(0, Qt::Vertical );
    crateSIGGEN_gr->layout()->setSpacing( 6 );
    crateSIGGEN_gr->layout()->setMargin( 0 );
    crateSIGGEN_grLayout = new QHBoxLayout( crateSIGGEN_gr->layout() );
    crateSIGGEN_grLayout->setAlignment( Qt::AlignTop );

    DSP1RBut[0] = new QRadioButton( crateSIGGEN_gr, "DSP1RBut[0]" );
    DSP1RBut[0]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    DSP1RBut[0]->setChecked( TRUE );
    crateSIGGEN_grLayout->addWidget( DSP1RBut[0] );

    DSP1RBut[1] = new QRadioButton( crateSIGGEN_gr, "DSP1RBut[1]" );
    DSP1RBut[1]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateSIGGEN_grLayout->addWidget( DSP1RBut[1] );

    DSP1RBut[2] = new QRadioButton( crateSIGGEN_gr, "DSP1RBut[2]" );
    DSP1RBut[2]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateSIGGEN_grLayout->addWidget( DSP1RBut[2] );

    DSP1RBut[3] = new QRadioButton( crateSIGGEN_gr, "DSP1RBut[3]" );
    DSP1RBut[3]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateSIGGEN_grLayout->addWidget( DSP1RBut[3] );

    DSP1RBut[4] = new QRadioButton( crateSIGGEN_gr, "DSP1RBut[4]" );
    DSP1RBut[4]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateSIGGEN_grLayout->addWidget( DSP1RBut[4] );

    DSP1RBut[5] = new QRadioButton( crateSIGGEN_gr, "DSP1RBut[5]" );
    DSP1RBut[5]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
    crateSIGGEN_grLayout->addWidget( DSP1RBut[5] );

    frameSIGGENbLayout->addWidget( crateSIGGEN_gr, 0, 0 );
    QWidget3Layout->addWidget( frameSIGGENb );

    TabPageLayout_2->addWidget( QWidget3, 1, 0 );

    frameCoilCurrDSP = new QFrame( TabPage_2, "frameCoilCurrDSP" );
    frameCoilCurrDSP->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameCoilCurrDSP->sizePolicy().hasHeightForWidth() ) );
    frameCoilCurrDSP->setFrameShape( QFrame::TabWidgetPanel );
    frameCoilCurrDSP->setFrameShadow( QFrame::Raised );
    frameCoilCurrDSPLayout = new QGridLayout( frameCoilCurrDSP, 1, 1, 11, 6, "frameCoilCurrDSPLayout"); 

    labelTempDSPc_0 = new QLabel( frameCoilCurrDSP, "labelTempDSPc_0" );
    labelTempDSPc_0->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelTempDSPc_0_font(  labelTempDSPc_0->font() );
    labelTempDSPc_0_font.setFamily( "Bitstream Charter" );
    labelTempDSPc_0_font.setBold( TRUE );
    labelTempDSPc_0_font.setItalic( TRUE );
    labelTempDSPc_0->setFont( labelTempDSPc_0_font ); 

    frameCoilCurrDSPLayout->addWidget( labelTempDSPc_0, 0, 1 );

    textLabelDSPc_1 = new QLabel( frameCoilCurrDSP, "textLabelDSPc_1" );

    frameCoilCurrDSPLayout->addWidget( textLabelDSPc_1, 1, 0 );

    DSPCurLab[0] = new QLabel( frameCoilCurrDSP, "DSPCurLab[0]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLab[0], 1, 1 );

    DSPCurLed[0] = new KLed( frameCoilCurrDSP, "DSPCurLed[0]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLed[0], 1, 2 );

    DSPCurBut[0] = new QPushButton( frameCoilCurrDSP, "DSPCurBut[0]" );
    DSPCurBut[0]->setAutoDefault( FALSE );

    frameCoilCurrDSPLayout->addWidget( DSPCurBut[0], 1, 3 );

    textLabelDSPc_2 = new QLabel( frameCoilCurrDSP, "textLabelDSPc_2" );

    frameCoilCurrDSPLayout->addWidget( textLabelDSPc_2, 2, 0 );

    DSPCurLab[1] = new QLabel( frameCoilCurrDSP, "DSPCurLab[1]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLab[1], 2, 1 );

    DSPCurLed[1] = new KLed( frameCoilCurrDSP, "DSPCurLed[1]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLed[1], 2, 2 );

    DSPCurBut[1] = new QPushButton( frameCoilCurrDSP, "DSPCurBut[1]" );
    DSPCurBut[1]->setAutoDefault( FALSE );

    frameCoilCurrDSPLayout->addWidget( DSPCurBut[1], 2, 3 );

    textLabelDSPc_3 = new QLabel( frameCoilCurrDSP, "textLabelDSPc_3" );

    frameCoilCurrDSPLayout->addWidget( textLabelDSPc_3, 3, 0 );

    DSPCurLab[2] = new QLabel( frameCoilCurrDSP, "DSPCurLab[2]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLab[2], 3, 1 );

    DSPCurLed[2] = new KLed( frameCoilCurrDSP, "DSPCurLed[2]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLed[2], 3, 2 );

    DSPCurBut[2] = new QPushButton( frameCoilCurrDSP, "DSPCurBut[2]" );
    DSPCurBut[2]->setAutoDefault( FALSE );

    frameCoilCurrDSPLayout->addWidget( DSPCurBut[2], 3, 3 );

    textLabelDSPc_4 = new QLabel( frameCoilCurrDSP, "textLabelDSPc_4" );

    frameCoilCurrDSPLayout->addWidget( textLabelDSPc_4, 4, 0 );

    DSPCurLab[3] = new QLabel( frameCoilCurrDSP, "DSPCurLab[3]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLab[3], 4, 1 );

    DSPCurLed[3] = new KLed( frameCoilCurrDSP, "DSPCurLed[3]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLed[3], 4, 2 );

    DSPCurBut[3] = new QPushButton( frameCoilCurrDSP, "DSPCurBut[3]" );
    DSPCurBut[3]->setAutoDefault( FALSE );

    frameCoilCurrDSPLayout->addWidget( DSPCurBut[3], 4, 3 );

    textLabelDSPc_5 = new QLabel( frameCoilCurrDSP, "textLabelDSPc_5" );

    frameCoilCurrDSPLayout->addWidget( textLabelDSPc_5, 5, 0 );

    DSPCurLab[4] = new QLabel( frameCoilCurrDSP, "DSPCurLab[4]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLab[4], 5, 1 );

    DSPCurLed[4] = new KLed( frameCoilCurrDSP, "DSPCurLed[4]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLed[4], 5, 2 );

    DSPCurBut[4] = new QPushButton( frameCoilCurrDSP, "DSPCurBut[4]" );
    DSPCurBut[4]->setAutoDefault( FALSE );

    frameCoilCurrDSPLayout->addWidget( DSPCurBut[4], 5, 3 );

    textLabelDSPc_6 = new QLabel( frameCoilCurrDSP, "textLabelDSPc_6" );

    frameCoilCurrDSPLayout->addWidget( textLabelDSPc_6, 6, 0 );

    DSPCurLab[5] = new QLabel( frameCoilCurrDSP, "DSPCurLab[5]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLab[5], 6, 1 );

    DSPCurLed[5] = new KLed( frameCoilCurrDSP, "DSPCurLed[5]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLed[5], 6, 2 );

    DSPCurBut[5] = new QPushButton( frameCoilCurrDSP, "DSPCurBut[5]" );
    DSPCurBut[5]->setAutoDefault( FALSE );

    frameCoilCurrDSPLayout->addWidget( DSPCurBut[5], 6, 3 );

    textLabelDSPc_7 = new QLabel( frameCoilCurrDSP, "textLabelDSPc_7" );

    frameCoilCurrDSPLayout->addWidget( textLabelDSPc_7, 7, 0 );

    DSPCurLab[6] = new QLabel( frameCoilCurrDSP, "DSPCurLab[6]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLab[6], 7, 1 );

    DSPCurLed[6] = new KLed( frameCoilCurrDSP, "DSPCurLed[6]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLed[6], 7, 2 );

    DSPCurBut[6] = new QPushButton( frameCoilCurrDSP, "DSPCurBut[6]" );
    DSPCurBut[6]->setAutoDefault( FALSE );

    frameCoilCurrDSPLayout->addWidget( DSPCurBut[6], 7, 3 );

    textLabelDSPc_8 = new QLabel( frameCoilCurrDSP, "textLabelDSPc_8" );

    frameCoilCurrDSPLayout->addWidget( textLabelDSPc_8, 8, 0 );

    DSPCurLab[7] = new QLabel( frameCoilCurrDSP, "DSPCurLab[7]" );
    for (i=0; i<8; i++) {
        DSPCurLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, DSPCurLab[7]->sizePolicy().hasHeightForWidth() ) );
        DSPCurLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        DSPCurLab[i]->setFrameShape( QLabel::LineEditPanel );
        DSPCurLab[i]->setFrameShadow( QLabel::Sunken );
        DSPCurLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameCoilCurrDSPLayout->addWidget( DSPCurLab[7], 8, 1 );

    DSPCurLed[7] = new KLed( frameCoilCurrDSP, "DSPCurLed[7]" );

    frameCoilCurrDSPLayout->addWidget( DSPCurLed[7], 8, 2 );

    DSPCurBut[7] = new QPushButton( frameCoilCurrDSP, "DSPCurBut[7]" );
    DSPCurBut[7]->setAutoDefault( FALSE );

    frameCoilCurrDSPLayout->addWidget( DSPCurBut[7], 8, 3 );

    TabPageLayout_2->addWidget( frameCoilCurrDSP, 0, 1 );

    QWidget4 = new QWidget( TabPage_2, "QWidget4" );
    QWidget4Layout = new QHBoxLayout( QWidget4, 11, 6, "QWidget4Layout"); 

    frame1 = new QFrame( QWidget4, "frame1" );
    frame1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frame1->sizePolicy().hasHeightForWidth() ) );
    frame1->setFrameShape( QFrame::TabWidgetPanel );
    frame1->setFrameShadow( QFrame::Raised );
    frame1Layout = new QGridLayout( frame1, 1, 1, 11, 6, "frame1Layout"); 

    crateDSPb_6 = new QButtonGroup( frame1, "crateDSPb_6" );
    crateDSPb_6->setPaletteForegroundColor( QColor( 85, 0, 127 ) );
    crateDSPb_6->setPaletteBackgroundColor( QColor( 191, 192, 148 ) );
    QFont crateDSPb_6_font(  crateDSPb_6->font() );
    crateDSPb_6_font.setFamily( "Bitstream Charter" );
    crateDSPb_6_font.setBold( TRUE );
    crateDSPb_6_font.setItalic( TRUE );
    crateDSPb_6->setFont( crateDSPb_6_font ); 
    crateDSPb_6->setColumnLayout(0, Qt::Vertical );
    crateDSPb_6->layout()->setSpacing( 6 );
    crateDSPb_6->layout()->setMargin( 11 );
    crateDSPb_6Layout = new QGridLayout( crateDSPb_6->layout() );
    crateDSPb_6Layout->setAlignment( Qt::AlignTop );

    DSP2RBut[0] = new QRadioButton( crateDSPb_6, "DSP2RBut[0]" );
    DSP2RBut[0]->setChecked( TRUE );

    crateDSPb_6Layout->addWidget( DSP2RBut[0], 0, 0 );

    DSP2RBut[1] = new QRadioButton( crateDSPb_6, "DSP2RBut[1]" );

    crateDSPb_6Layout->addWidget( DSP2RBut[1], 1, 0 );

    DSP2RBut[2] = new QRadioButton( crateDSPb_6, "DSP2RBut[2]" );

    crateDSPb_6Layout->addWidget( DSP2RBut[2], 2, 0 );

    DSP2RBut[3] = new QRadioButton( crateDSPb_6, "DSP2RBut[3]" );

    crateDSPb_6Layout->addWidget( DSP2RBut[3], 3, 0 );

    DSP2RBut[4] = new QRadioButton( crateDSPb_6, "DSP2RBut[4]" );

    crateDSPb_6Layout->addWidget( DSP2RBut[4], 4, 0 );

    DSP2RBut[5] = new QRadioButton( crateDSPb_6, "DSP2RBut[5]" );

    crateDSPb_6Layout->addWidget( DSP2RBut[5], 5, 0 );

    DSP2RBut[6] = new QRadioButton( crateDSPb_6, "DSP2RBut[6]" );

    crateDSPb_6Layout->addWidget( DSP2RBut[6], 6, 0 );

    DSP2RBut[7] = new QRadioButton( crateDSPb_6, "DSP2RBut[7]" );

    crateDSPb_6Layout->addWidget( DSP2RBut[7], 0, 1 );

    DSP2RBut[8] = new QRadioButton( crateDSPb_6, "DSP2RBut[8]" );

    crateDSPb_6Layout->addWidget( DSP2RBut[8], 1, 1 );

    DSP2RBut[9] = new QRadioButton( crateDSPb_6, "DSP2RBut[9]" );

    crateDSPb_6Layout->addWidget( DSP2RBut[9], 2, 1 );

    DSP2RBut[10] = new QRadioButton( crateDSPb_6, "DSP2RBut[10]" );

    crateDSPb_6Layout->addWidget( DSP2RBut[10], 3, 1 );

    DSP2RBut[11] = new QRadioButton( crateDSPb_6, "DSP2RBut[11]" );

    crateDSPb_6Layout->addWidget( DSP2RBut[11], 4, 1 );

    DSP2RBut[12] = new QRadioButton( crateDSPb_6, "DSP2RBut[12]" );

    crateDSPb_6Layout->addWidget( DSP2RBut[12], 5, 1 );

    DSP2RBut[13] = new QRadioButton( crateDSPb_6, "DSP2RBut[13]" );

    crateDSPb_6Layout->addWidget( DSP2RBut[13], 6, 1 );

    frame1Layout->addWidget( crateDSPb_6, 0, 0 );
    QWidget4Layout->addWidget( frame1 );

    frameDriverEnabledDSP = new QFrame( QWidget4, "frameDriverEnabledDSP" );
    frameDriverEnabledDSP->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameDriverEnabledDSP->sizePolicy().hasHeightForWidth() ) );
    frameDriverEnabledDSP->setFrameShape( QFrame::TabWidgetPanel );
    frameDriverEnabledDSP->setFrameShadow( QFrame::Raised );
    frameDriverEnabledDSPLayout = new QGridLayout( frameDriverEnabledDSP, 1, 1, 11, 6, "frameDriverEnabledDSPLayout"); 

    labelDriverEnabled = new QLabel( frameDriverEnabledDSP, "labelDriverEnabled" );
    labelDriverEnabled->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelDriverEnabled_font(  labelDriverEnabled->font() );
    labelDriverEnabled_font.setFamily( "Bitstream Charter" );
    labelDriverEnabled_font.setBold( TRUE );
    labelDriverEnabled_font.setItalic( TRUE );
    labelDriverEnabled->setFont( labelDriverEnabled_font ); 

    frameDriverEnabledDSPLayout->addWidget( labelDriverEnabled, 0, 0 );

    unnamed_2 = new QWidget( frameDriverEnabledDSP, "unnamed_2" );
    unnamedLayout_2 = new QGridLayout( unnamed_2, 1, 1, 11, 6, "unnamedLayout_2"); 

    textLabelDSPf_1 = new QLabel( unnamed_2, "textLabelDSPf_1" );

    unnamedLayout_2->addWidget( textLabelDSPf_1, 0, 0 );

    DSPDrLed[0] = new KLed( unnamed_2, "DSPDrLed[0]" );
    DSPDrLed[0]->setColor( QColor( 255, 255, 255 ) );

    unnamedLayout_2->addWidget( DSPDrLed[0], 0, 1 );

    textLabelDSPf_2 = new QLabel( unnamed_2, "textLabelDSPf_2" );

    unnamedLayout_2->addWidget( textLabelDSPf_2, 1, 0 );

    DSPDrLed[1] = new KLed( unnamed_2, "DSPDrLed[1]" );
    DSPDrLed[1]->setColor( QColor( 255, 255, 255 ) );

    unnamedLayout_2->addWidget( DSPDrLed[1], 1, 1 );

    textLabelDSPf_3 = new QLabel( unnamed_2, "textLabelDSPf_3" );

    unnamedLayout_2->addWidget( textLabelDSPf_3, 2, 0 );

    DSPDrLed[2] = new KLed( unnamed_2, "DSPDrLed[2]" );
    DSPDrLed[2]->setColor( QColor( 255, 255, 255 ) );

    unnamedLayout_2->addWidget( DSPDrLed[2], 2, 1 );

    textLabelDSPf_4 = new QLabel( unnamed_2, "textLabelDSPf_4" );

    unnamedLayout_2->addWidget( textLabelDSPf_4, 3, 0 );

    DSPDrLed[3] = new KLed( unnamed_2, "DSPDrLed[3]" );
    DSPDrLed[3]->setColor( QColor( 255, 255, 255 ) );

    unnamedLayout_2->addWidget( DSPDrLed[3], 3, 1 );

    frameDriverEnabledDSPLayout->addWidget( unnamed_2, 1, 0 );

    unnamed_3 = new QWidget( frameDriverEnabledDSP, "unnamed_3" );
    unnamedLayout_3 = new QGridLayout( unnamed_3, 1, 1, 11, 6, "unnamedLayout_3"); 

    textLabelDSPf_5 = new QLabel( unnamed_3, "textLabelDSPf_5" );

    unnamedLayout_3->addWidget( textLabelDSPf_5, 0, 0 );

    DSPDrLed[4] = new KLed( unnamed_3, "DSPDrLed[4]" );
    DSPDrLed[4]->setColor( QColor( 255, 255, 255 ) );

    unnamedLayout_3->addWidget( DSPDrLed[4], 0, 1 );

    textLabelDSPf_6 = new QLabel( unnamed_3, "textLabelDSPf_6" );

    unnamedLayout_3->addWidget( textLabelDSPf_6, 1, 0 );

    DSPDrLed[5] = new KLed( unnamed_3, "DSPDrLed[5]" );
    DSPDrLed[5]->setColor( QColor( 255, 255, 255 ) );

    unnamedLayout_3->addWidget( DSPDrLed[5], 1, 1 );

    textLabelDSPf_7 = new QLabel( unnamed_3, "textLabelDSPf_7" );

    unnamedLayout_3->addWidget( textLabelDSPf_7, 2, 0 );

    DSPDrLed[6] = new KLed( unnamed_3, "DSPDrLed[6]" );
    DSPDrLed[6]->setColor( QColor( 255, 255, 255 ) );

    unnamedLayout_3->addWidget( DSPDrLed[6], 2, 1 );

    textLabelDSPf_8 = new QLabel( unnamed_3, "textLabelDSPf_8" );

    unnamedLayout_3->addWidget( textLabelDSPf_8, 3, 0 );

    DSPDrLed[7] = new KLed( unnamed_3, "DSPDrLed[7]" );
    DSPDrLed[7]->setColor( QColor( 255, 255, 255 ) );

    unnamedLayout_3->addWidget( DSPDrLed[7], 3, 1 );

    frameDriverEnabledDSPLayout->addWidget( unnamed_3, 1, 1 );
    QWidget4Layout->addWidget( frameDriverEnabledDSP );

    TabPageLayout_2->addWidget( QWidget4, 1, 1 );

    frameDriverStatusDSP = new QFrame( TabPage_2, "frameDriverStatusDSP" );
    frameDriverStatusDSP->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameDriverStatusDSP->sizePolicy().hasHeightForWidth() ) );
    frameDriverStatusDSP->setFrameShape( QFrame::TabWidgetPanel );
    frameDriverStatusDSP->setFrameShadow( QFrame::Raised );
    frameDriverStatusDSPLayout = new QGridLayout( frameDriverStatusDSP, 1, 1, 11, 6, "frameDriverStatusDSPLayout"); 

    labelDriverStatus = new QLabel( frameDriverStatusDSP, "labelDriverStatus" );
    labelDriverStatus->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelDriverStatus_font(  labelDriverStatus->font() );
    labelDriverStatus_font.setFamily( "Bitstream Charter" );
    labelDriverStatus_font.setBold( TRUE );
    labelDriverStatus_font.setItalic( TRUE );
    labelDriverStatus->setFont( labelDriverStatus_font ); 

    frameDriverStatusDSPLayout->addWidget( labelDriverStatus, 0, 0 );

    textLabelDSPe_1 = new QLabel( frameDriverStatusDSP, "textLabelDSPe_1" );

    frameDriverStatusDSPLayout->addWidget( textLabelDSPe_1, 1, 0 );

    DSPDrSLed[0] = new KLed( frameDriverStatusDSP, "DSPDrSLed[0]" );
    DSPDrSLed[0]->setColor( QColor( 255, 255, 255 ) );

    frameDriverStatusDSPLayout->addWidget( DSPDrSLed[0], 1, 1 );

    textLabelDSPe_2 = new QLabel( frameDriverStatusDSP, "textLabelDSPe_2" );

    frameDriverStatusDSPLayout->addWidget( textLabelDSPe_2, 2, 0 );

    DSPDrSLed[1] = new KLed( frameDriverStatusDSP, "DSPDrSLed[1]" );
    DSPDrSLed[1]->setColor( QColor( 255, 255, 255 ) );

    frameDriverStatusDSPLayout->addWidget( DSPDrSLed[1], 2, 1 );

    textLabelDSPe_3 = new QLabel( frameDriverStatusDSP, "textLabelDSPe_3" );

    frameDriverStatusDSPLayout->addWidget( textLabelDSPe_3, 3, 0 );

    DSPDrSLed[2] = new KLed( frameDriverStatusDSP, "DSPDrSLed[2]" );
    DSPDrSLed[2]->setColor( QColor( 255, 255, 255 ) );

    frameDriverStatusDSPLayout->addWidget( DSPDrSLed[2], 3, 1 );

    textLabelDSPe_4 = new QLabel( frameDriverStatusDSP, "textLabelDSPe_4" );

    frameDriverStatusDSPLayout->addWidget( textLabelDSPe_4, 4, 0 );

    DSPDrSLed[3] = new KLed( frameDriverStatusDSP, "DSPDrSLed[3]" );
    DSPDrSLed[3]->setColor( QColor( 255, 255, 255 ) );

    frameDriverStatusDSPLayout->addWidget( DSPDrSLed[3], 4, 1 );

    textLabelDSPe_5 = new QLabel( frameDriverStatusDSP, "textLabelDSPe_5" );

    frameDriverStatusDSPLayout->addWidget( textLabelDSPe_5, 5, 0 );

    DSPDrSLed[4] = new KLed( frameDriverStatusDSP, "DSPDrSLed[4]" );
    DSPDrSLed[4]->setColor( QColor( 255, 255, 255 ) );

    frameDriverStatusDSPLayout->addWidget( DSPDrSLed[4], 5, 1 );

    textLabelDSPe_6 = new QLabel( frameDriverStatusDSP, "textLabelDSPe_6" );

    frameDriverStatusDSPLayout->addWidget( textLabelDSPe_6, 6, 0 );

    DSPDrSLed[5] = new KLed( frameDriverStatusDSP, "DSPDrSLed[5]" );
    DSPDrSLed[5]->setColor( QColor( 255, 255, 255 ) );

    frameDriverStatusDSPLayout->addWidget( DSPDrSLed[5], 6, 1 );

    textLabelDSPe_7 = new QLabel( frameDriverStatusDSP, "textLabelDSPe_7" );

    frameDriverStatusDSPLayout->addWidget( textLabelDSPe_7, 7, 0 );

    DSPDrSLed[6] = new KLed( frameDriverStatusDSP, "DSPDrSLed[6]" );
    DSPDrSLed[6]->setColor( QColor( 255, 255, 255 ) );

    frameDriverStatusDSPLayout->addWidget( DSPDrSLed[6], 7, 1 );

    textLabelDSPe_8 = new QLabel( frameDriverStatusDSP, "textLabelDSPe_8" );

    frameDriverStatusDSPLayout->addWidget( textLabelDSPe_8, 8, 0 );

    DSPDrSLed[7] = new KLed( frameDriverStatusDSP, "DSPDrSLed[7]" );
    DSPDrSLed[7]->setColor( QColor( 255, 255, 255 ) );

    frameDriverStatusDSPLayout->addWidget( DSPDrSLed[7], 8, 1 );

    textLabelDSPe_9 = new QLabel( frameDriverStatusDSP, "textLabelDSPe_9" );

    frameDriverStatusDSPLayout->addWidget( textLabelDSPe_9, 9, 0 );

    DSPDrSLed[8] = new KLed( frameDriverStatusDSP, "DSPDrSLed[8]" );
    DSPDrSLed[8]->setColor( QColor( 255, 255, 255 ) );

    frameDriverStatusDSPLayout->addWidget( DSPDrSLed[8], 9, 1 );

    textLabelDSPe_10 = new QLabel( frameDriverStatusDSP, "textLabelDSPe_10" );

    frameDriverStatusDSPLayout->addWidget( textLabelDSPe_10, 10, 0 );

    DSPDrSLed[9] = new KLed( frameDriverStatusDSP, "DSPDrSLed[9]" );
    DSPDrSLed[9]->setColor( QColor( 255, 255, 255 ) );

    frameDriverStatusDSPLayout->addWidget( DSPDrSLed[9], 10, 1 );

    TabPageLayout_2->addWidget( frameDriverStatusDSP, 0, 2 );

    frameDSPb = new QFrame( TabPage_2, "frameDSPb" );
    frameDSPb->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameDSPb->sizePolicy().hasHeightForWidth() ) );
    frameDSPb->setFrameShape( QFrame::TabWidgetPanel );
    frameDSPb->setFrameShadow( QFrame::Raised );
    frameDSPbLayout = new QGridLayout( frameDSPb, 1, 1, 11, 6, "frameDSPbLayout"); 

    textLabelDSPb_0 = new QLabel( frameDSPb, "textLabelDSPb_0" );

    frameDSPbLayout->addWidget( textLabelDSPb_0, 0, 0 );

    DSPadLab[0] = new QLabel( frameDSPb, "DSPadLab[0]" );

    frameDSPbLayout->addWidget( DSPadLab[0], 0, 1 );

    DSPadLed[0] = new KLed( frameDSPb, "DSPadLed[0]" );

    frameDSPbLayout->addWidget( DSPadLed[0], 0, 2 );

    textLabelDSPb_1 = new QLabel( frameDSPb, "textLabelDSPb_1" );

    frameDSPbLayout->addWidget( textLabelDSPb_1, 1, 0 );

    DSPadLab[1] = new QLabel( frameDSPb, "DSPadLab[1]" );

    frameDSPbLayout->addWidget( DSPadLab[1], 1, 1 );

    DSPadLed[1] = new KLed( frameDSPb, "DSPadLed[1]" );

    frameDSPbLayout->addWidget( DSPadLed[1], 1, 2 );

    textLabelDSPb_2 = new QLabel( frameDSPb, "textLabelDSPb_2" );

    frameDSPbLayout->addWidget( textLabelDSPb_2, 2, 0 );

    DSPadLab[2] = new QLabel( frameDSPb, "DSPadLab[2]" );

    frameDSPbLayout->addWidget( DSPadLab[2], 2, 1 );

    DSPadLed[2] = new KLed( frameDSPb, "DSPadLed[2]" );

    frameDSPbLayout->addWidget( DSPadLed[2], 2, 2 );

    textLabelDSPb_3 = new QLabel( frameDSPb, "textLabelDSPb_3" );

    frameDSPbLayout->addWidget( textLabelDSPb_3, 3, 0 );

    DSPadLab[3] = new QLabel( frameDSPb, "DSPadLab[3]" );

    frameDSPbLayout->addWidget( DSPadLab[3], 3, 1 );

    DSPadLed[3] = new KLed( frameDSPb, "DSPadLed[3]" );

    frameDSPbLayout->addWidget( DSPadLed[3], 3, 2 );

    textLabelDSPb_4 = new QLabel( frameDSPb, "textLabelDSPb_4" );

    frameDSPbLayout->addWidget( textLabelDSPb_4, 4, 0 );

    DSPadLab[4] = new QLabel( frameDSPb, "DSPadLab[4]" );

    frameDSPbLayout->addWidget( DSPadLab[4], 4, 1 );

    DSPadLed[4] = new KLed( frameDSPb, "DSPadLed[4]" );

    frameDSPbLayout->addWidget( DSPadLed[4], 4, 2 );

    textLabelDSPb_5 = new QLabel( frameDSPb, "textLabelDSPb_5" );

    frameDSPbLayout->addWidget( textLabelDSPb_5, 5, 0 );

    DSPadLab[5] = new QLabel( frameDSPb, "DSPadLab[5]" );
    for (i=0; i<6; i++) {
        DSPadLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, DSPadLab[5]->sizePolicy().hasHeightForWidth() ) );
        DSPadLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        DSPadLab[i]->setFrameShape( QLabel::LineEditPanel );
        DSPadLab[i]->setFrameShadow( QLabel::Sunken );
        DSPadLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }

    frameDSPbLayout->addWidget( DSPadLab[5], 5, 1 );

    DSPadLed[5] = new KLed( frameDSPb, "DSPadLed[5]" );

    frameDSPbLayout->addWidget( DSPadLed[5], 5, 2 );

    crateDSPb_6_2 = new QButtonGroup( frameDSPb, "crateDSPb_6_2" );
    crateDSPb_6_2->setPaletteForegroundColor( QColor( 85, 0, 127 ) );
    crateDSPb_6_2->setPaletteBackgroundColor( QColor( 189, 179, 192 ) );
    QFont crateDSPb_6_2_font(  crateDSPb_6_2->font() );
    crateDSPb_6_2_font.setFamily( "Bitstream Charter" );
    crateDSPb_6_2_font.setBold( TRUE );
    crateDSPb_6_2_font.setItalic( TRUE );
    crateDSPb_6_2->setFont( crateDSPb_6_2_font ); 
    crateDSPb_6_2->setColumnLayout(0, Qt::Vertical );
    crateDSPb_6_2->layout()->setSpacing( 6 );
    crateDSPb_6_2->layout()->setMargin( 11 );
    crateDSPb_6Layout_2 = new QGridLayout( crateDSPb_6_2->layout() );
    crateDSPb_6Layout_2->setAlignment( Qt::AlignTop );

    DSP3RBut[0] = new QRadioButton( crateDSPb_6_2, "DSP3RBut[0]" );
    DSP3RBut[0]->setChecked( TRUE );

    crateDSPb_6Layout_2->addWidget( DSP3RBut[0], 0, 0 );

    DSP3RBut[1] = new QRadioButton( crateDSPb_6_2, "DSP3RBut[1]" );

    crateDSPb_6Layout_2->addWidget( DSP3RBut[1], 0, 1 );

    DSP3RBut[2] = new QRadioButton( crateDSPb_6_2, "DSP3RBut[2]" );

    crateDSPb_6Layout_2->addWidget( DSP3RBut[2], 0, 2 );

    DSP3RBut[3] = new QRadioButton( crateDSPb_6_2, "DSP3RBut[3]" );

    crateDSPb_6Layout_2->addWidget( DSP3RBut[3], 0, 3 );

    DSP3RBut[4] = new QRadioButton( crateDSPb_6_2, "DSP3RBut[4]" );

    crateDSPb_6Layout_2->addWidget( DSP3RBut[4], 1, 0 );

    DSP3RBut[5] = new QRadioButton( crateDSPb_6_2, "DSP3RBut[5]" );

    crateDSPb_6Layout_2->addWidget( DSP3RBut[5], 1, 1 );

    DSP3RBut[6] = new QRadioButton( crateDSPb_6_2, "DSP3RBut[6]" );

    crateDSPb_6Layout_2->addWidget( DSP3RBut[6], 1, 2 );

    DSP3RBut[7] = new QRadioButton( crateDSPb_6_2, "DSP3RBut[7]" );

    crateDSPb_6Layout_2->addWidget( DSP3RBut[7], 1, 3 );

    frameDSPbLayout->addWidget( crateDSPb_6_2, 6, 1 );

    TabPageLayout_2->addWidget( frameDSPb, 1, 2 );
    tabWidget->insertTab( TabPage_2, QString::fromLatin1("") );

    Widget18 = new QWidget( tabWidget, "Widget18" );

    listBox1 = new QListBox( Widget18, "listBox1" );
    listBox1->setGeometry( QRect( -3, 2, 1030, 600 ) );
    listBox1->setSelectionMode( QListBox::NoSelection );
    listBox1->setColumnMode( QListBox::FitToWidth );
    tabWidget->insertTab( Widget18, QString::fromLatin1("") );

    Widget19 = new QWidget( tabWidget, "Widget19" );

    listBox1_2 = new QListBox( Widget19, "listBox1_2" );
    listBox1_2->setGeometry( QRect( 0, 1, 1030, 600 ) );
    listBox1_2->setSelectionMode( QListBox::NoSelection );
    listBox1_2->setColumnMode( QListBox::FitToWidth );
    tabWidget->insertTab( Widget19, QString::fromLatin1("") );
    topLBT672Layout->addWidget( tabWidget );

    Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1"); 

    buttonStart = new QPushButton( this, "buttonStart" );
    buttonStart->setPaletteBackgroundColor( QColor( 255, 255, 127 ) );
    QFont buttonStart_font(  buttonStart->font() );
    buttonStart_font.setFamily( "URW Gothic L" );
    buttonStart_font.setPointSize( 11 );
    buttonStart_font.setBold( TRUE );
    buttonStart->setFont( buttonStart_font ); 
    buttonStart->setAutoDefault( TRUE );
    Layout1->addWidget( buttonStart );
    Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( Horizontal_Spacing2 );

    warnLabel = new QLabel( this, "             " );
    Layout1->addWidget( warnLabel );
    alarmLabel = new QLabel( this, "             " );
    Layout1->addWidget( alarmLabel );

    Horizontal_Spacing3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( Horizontal_Spacing3 );

    buttonStart_2 = new QPushButton( this, "buttonStart_2" );
    buttonStart_2->setPaletteBackgroundColor( QColor( 171, 255, 148 ) );
    QFont buttonStart_2_font(  buttonStart_2->font() );
    buttonStart_2_font.setFamily( "URW Gothic L" );
    buttonStart_2_font.setPointSize( 11 );
    buttonStart_2_font.setBold( TRUE );
    buttonStart_2->setFont( buttonStart_2_font ); 
    buttonStart_2->setAutoDefault( TRUE );
    Layout1->addWidget( buttonStart_2 );

    buttonExit = new QPushButton( this, "buttonExit" );
    buttonExit->setPaletteBackgroundColor( QColor( 255, 170, 255 ) );
    QFont buttonExit_font(  buttonExit->font() );
    buttonExit_font.setFamily( "URW Gothic L" );
    buttonExit_font.setPointSize( 11 );
    buttonExit_font.setBold( TRUE );
    buttonExit->setFont( buttonExit_font ); 
    buttonExit->setAutoDefault( TRUE );
    Layout1->addWidget( buttonExit );
    topLBT672Layout->addLayout( Layout1 );
    languageChange();
    resize( QSize(1049, 689).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    timer = new QTimer (this);


/* initialisation ***************************************************/
   timeout=2000;

   ICr1=0;
   ICr2=0;
   ICr3=0;
   IBo=0;
   ICh=0;
   for (i=0; i<200; i++) FamilyNames[i]=FamilyName[i];

   int i1,i2,i3;
   for (i1=0; i1<14; i1++) {
       if (i1<7) DiagAppDataStat[i1] = 0;
       if (i1<9) ExternalStat[i1] = 0;
       if (i1<3) BCU2MastStat[i1] = 0;
       if (i1<4) BCU2SGNStat[i1] = 0;
       if (i1<4) BCU2TrStat[i1] = 0;
       if (i1<8) DSPNiosStat[i1] = 0;
       if (i1<11) BCU1NiosStat[7][i1] = 0;
       if (i1<2)  BCU1TempStat[7][i1] = 0;
       for (i2=0; i2<6; i2++) {
           if (i1<11) BCU1NiosStat[i2][i1] = 0;
           if (i1<6)  BCU1EnvStat[i2][i1] = 0;
           if (i1<10) BCU1VolStat[i2][i1] = 0;
           if (i1<2)  BCU1TempStat[i2][i1] = 0;
           if (i1<3)  BCU1SGNTempStat[i2][i1] = 0;
           if (i1<10) BCU2ResetStat[i2][i1] = 0;
           for (i=0; i<8; i++) {
               DSPCurStat[i2][i1][i] = 0;
               DSPDrStat[i2][i1][i] = 0;
               if (i<2) DSPDrStat[i2][i1][i+8] = 0;
               for (i3=0; i3<8; i3++) {
                   DSPadStat[i2][i1][i3][i] = 0;
               }
           }
       }
   }

   DiagAppDataInd[0] = 0;
   DiagAppDataInd[1] = 7;

   ExternalInd[0] = 112;
   ExternalInd[1] =  9;

   BCU1NiosInd[0][0] = 11;
   BCU1NiosInd[0][1] = 11;
   BCU1NiosInd[1][0] = 22;
   BCU1NiosInd[1][1] = 11;

   BCU1EnvInd[0] = 73;
   BCU1EnvInd[1] = 6;

   BCU1VolInd[0] = 63;
   BCU1VolInd[1] = 10;

   BCU1TempInd[0][0] = 61;
   BCU1TempInd[0][1] = 2;
   BCU1TempInd[1][0] = 59;
   BCU1TempInd[1][1] = 2;

   BCU1SGNTempInd[0] = 51;
   BCU1SGNTempInd[1] = 3;

   BCU2ResetInd[0] = 92;
   BCU2ResetInd[1] = 10;

   BCU2DigInd[0] = 82;
   BCU2DigInd[1] = 10;

   BCU2MastInd[0] = 78;
   BCU2MastInd[1] = 3;

   BCU2SGNInd[0] = 47;
   BCU2SGNInd[1] = 4;

   BCU2TrInd[0] = 7;
   BCU2TrInd[1] = 4;

   DSPNiosInd[0] = 33;
   DSPNiosInd[1] = 8;

   DSPCurInd[0] = 58;
   DSPCurInd[1] = 1;

   DSPDrSInd[0] = 102;
   DSPDrSInd[1] = 10;

   DSPTempInd[0] = 54;
   DSPTempInd[1] = 4;

   DSPDrInd[0] = 112;
   DSPDrInd[1] = 1;

   DSPadInd[0] = 41;
   DSPadInd[1] = 6;
/* initialisation **************************end**********************/



    // signals and slots connections
    for (i=0; i<9;  i++) connect( ExternalBut[i], SIGNAL( clicked() ), this, SLOT( newPlot() ) );
    for (i=0; i<6;  i++) connect( BCU1EnvBut[i], SIGNAL( clicked() ), this, SLOT( newPlot() ) );
    for (i=0; i<10; i++) connect( BCU1VolBut[i], SIGNAL( clicked() ), this, SLOT( newPlot() ) );
    for (i=0; i<2;  i++) connect( BCU1TempBut[i], SIGNAL( clicked() ), this, SLOT( newPlot() ) );
    for (i=0; i<3;  i++) connect( BCU1SGNTempBut[i], SIGNAL( clicked() ), this, SLOT( newPlot() ) );
    for (i=0; i<3;  i++) connect( BCU2MastBut[i], SIGNAL( clicked() ), this, SLOT( newPlot() ) );
    for (i=0; i<4;  i++) connect( BCU2TrBut[i], SIGNAL( clicked() ), this, SLOT( newPlot() ) );
    for (i=0; i<4;  i++) connect( DSPTempBut[i], SIGNAL( clicked() ), this, SLOT( newPlot() ) );
    for (i=0; i<8;  i++) connect( DSPCurBut[i], SIGNAL( clicked() ), this, SLOT( newPlot() ) );
    for (i=0; i<7;  i++) connect( BCU1RBut[i], SIGNAL( clicked() ), this, SLOT( SetICr1() ) );
    for (i=0; i<6;  i++) connect( BCU2RBut[i], SIGNAL( clicked() ), this, SLOT( SetICr2() ) );
    for (i=0; i<6;  i++) connect( DSP1RBut[i], SIGNAL( clicked() ), this, SLOT( SetICr3() ) );
    for (i=0; i<14; i++) connect( DSP2RBut[i], SIGNAL( clicked() ), this, SLOT( SetIBo() ) );
    for (i=0; i<8;  i++) connect( DSP3RBut[i], SIGNAL( clicked() ), this, SLOT( SetICh() ) );
    connect( buttonStart, SIGNAL( clicked() ), this, SLOT( StopStartTimer() ) );
    connect( buttonExit, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( timer, SIGNAL( timeout() ), this, SLOT( Refresh() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
topLBT672::~topLBT672()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void topLBT672::languageChange()
{
    char temp[300];

    setCaption( tr( "LBT672 Housekeeping" ) );
    labelDiagn->setText( tr( "DiagnApp Data" ) );
    textLabel1->setText( tr( "DiagnAppFrameTimestamp" ) );
    textLabel2->setText( tr( "DiagnAppFrameCounter" ) );
    textLabel3->setText( tr( "DiagnAppFastVarsCounter" ) );
    textLabel4->setText( tr( "DiagnAppSlowVarsCounter" ) );
    textLabel5->setText( tr( "DiagnAppFrameRate" ) );
    textLabel6->setText( tr( "DiagnAppFastVarsRate" ) );
    textLabel7->setText( tr( "DiagnAppSlowVarsRate" ) );
    labelTemper->setText( tr( "External Temperatures and Cooling Data" ) );
    textLabel11->setText( tr( "ExternalTemperature,  C" ) );
    ExternalBut[0]->setText( tr( "Plot" ) );
    textLabel12->setText( tr( "FluxRateIn,          L/min" ) );
    ExternalBut[1]->setText( tr( "Plot" ) );
    textLabel13->setText( tr( "WaterMainInlet,          C" ) );
    ExternalBut[2]->setText( tr( "Plot" ) );
    textLabel14->setText( tr( "WaterMainOutlet,        C" ) );
    ExternalBut[3]->setText( tr( "Plot" ) );
    textLabel15->setText( tr( "WaterColdPlateInlet,   C" ) );
    ExternalBut[4]->setText( tr( "Plot" ) );
    textLabel16->setText( tr( "WaterColdPlateOutlet, C" ) );
    ExternalBut[5]->setText( tr( "Plot" ) );
    textLabel17->setText( tr( "ExternalHumidity,       %" ) );
    ExternalBut[6]->setText( tr( "Plot" ) );
    textLabel18->setText( tr( "DewPoint,                  C" ) );
    ExternalBut[7]->setText( tr( "Plot" ) );
    textLabel19->setText( tr( "CheckDewPoint,         C" ) );
    ExternalBut[8]->setText( tr( "Plot" ) );
    tabWidget->changeTab( tabSYS, tr( "System Summary" ) );
    labelNIOS->setText( tr( "NIOS Fixed Area" ) );
    textLabelBCU1_1->setText( tr( "CrateID" ) );
    textLabelBCU1_2->setText( tr( "WhoAmI" ) );
    textLabelBCU1_3->setText( tr( "SoftwareRelease" ) );
    textLabelBCU1_4->setText( tr( "LogicRelease" ) );
    textLabelBCU1_5->setText( tr( "IPaddress" ) );
    textLabelBCU1_6->setText( tr( "FramesCounter" ) );
    textLabelBCU1_7->setText( tr( "SerialNumber" ) );
    textLabelBCU1_8->setText( tr( "PowerBackplaneSerial" ) );
    textLabelBCU1_9->setText( tr( "EnableMasterDiagnostic" ) );
    textLabelBCU1_10->setText( tr( "DecimationFactor" ) );
    textLabelBCU1_11->setText( tr( "RemoteIPaddress" ) );
    if (ICr1 < 6) sprintf (temp,"Crate %1i",ICr1);
    else strcpy(temp,"Switch BCU");
    crateBCU1_gr->setTitle( tr( temp ) );
    BCU1RBut[0]->setText( tr( "0" ) );
    BCU1RBut[1]->setText( tr( "1" ) );
    BCU1RBut[2]->setText( tr( "2" ) );
    BCU1RBut[3]->setText( tr( "3" ) );
    BCU1RBut[4]->setText( tr( "4" ) );
    BCU1RBut[5]->setText( tr( "5" ) );
    BCU1RBut[6]->setText( tr( "Switch BCU" ) );
    labelEnvBCU1->setText( tr( "Environment" ) );
    textLabelBCU1b_1->setText( tr( "Cooler in0 temp.,   C" ) );
    BCU1EnvBut[0]->setText( tr( "Plot" ) );
    textLabelBCU1b_2->setText( tr( "Cooler in1 temp.,   C" ) );
    BCU1EnvBut[1]->setText( tr( "Plot" ) );
    textLabelBCU1b_3->setText( tr( "Cooler out0 temp., C" ) );
    BCU1EnvBut[2]->setText( tr( "Plot" ) );
    textLabelBCU1b_4->setText( tr( "Cooler out1 temp., C" ) );
    BCU1EnvBut[3]->setText( tr( "Plot" ) );
    textLabelBCU1b_5->setText( tr( "Cooler Pressure,   P" ) );
    BCU1EnvBut[4]->setText( tr( "Plot" ) );
    textLabelBCU1b_6->setText( tr( "Humidity,             %" ) );
    BCU1EnvBut[5]->setText( tr( "Plot" ) );
    labelTempBCU1->setText( tr( "Temperatures" ) );
    textLabelBCU1a_1->setText( tr( "StratixTemp,  C" ) );
    BCU1TempBut[0]->setText( tr( "Plot" ) );
    textLabelBCU1a_2->setText( tr( "PowerTemp,  C" ) );
    BCU1TempBut[1]->setText( tr( "Plot" ) );
    labelVoltBCU1->setText( tr( "Voltage and Currents" ) );
    textLabelBCU1c_1->setText( tr( "VoltageVCCL,  V" ) );
    BCU1VolBut[0]->setText( tr( "Plot" ) );
    textLabelBCU1c_2->setText( tr( "VoltageVCCA,  V" ) );
    BCU1VolBut[1]->setText( tr( "Plot" ) );
    textLabelBCU1c_3->setText( tr( "VoltageVSSA,  V" ) );
    BCU1VolBut[2]->setText( tr( "Plot" ) );
    textLabelBCU1c_4->setText( tr( "VoltageVCCP,  V" ) );
    BCU1VolBut[3]->setText( tr( "Plot" ) );
    textLabelBCU1c_5->setText( tr( "VoltageVSSP,  V" ) );
    BCU1VolBut[4]->setText( tr( "Plot" ) );
    textLabelBCU1c_6->setText( tr( "CurrentVCCL,  A" ) );
    BCU1VolBut[5]->setText( tr( "Plot" ) );
    textLabelBCU1c_7->setText( tr( "CurrentVCCA,  A" ) );
    BCU1VolBut[6]->setText( tr( "Plot" ) );
    textLabelBCU1c_8->setText( tr( "CurrentVSSA,  A" ) );
    BCU1VolBut[7]->setText( tr( "Plot" ) );
    textLabelBCU1c_9->setText( tr( "CurrentVCCP,  A" ) );
    BCU1VolBut[8]->setText( tr( "Plot" ) );
    textLabelBCU1c_10->setText( tr( "CurrentVSSP,  A" ) );
    BCU1VolBut[9]->setText( tr( "Plot" ) );
    labelTempSIGGENd->setText( tr( "SIGGEN" ) );
    labelTempSIGGENd_2->setText( tr( "Temperatures" ) );
    textLabelSIGGENd_1->setText( tr( "Stratix,  C" ) );
    BCU1SGNTempBut[0]->setText( tr( "Plot" ) );
    textLabelSIGGENd_2->setText( tr( "Power,  C" ) );
    BCU1SGNTempBut[1]->setText( tr( "Plot" ) );
    textLabelSIGGENd_3->setText( tr( "DSPs,   C" ) );
    BCU1SGNTempBut[2]->setText( tr( "Plot" ) );
    tabWidget->changeTab( unnamed, tr( "Crate BCU 1" ) );
    labelResets->setText( tr( "Reset status" ) );
    textLabelBCU2_1->setText( tr( "FPGA Reset" ) );
    textLabelBCU2_2->setText( tr( "BUS Reset" ) );
    textLabelBCU2_3->setText( tr( "DSP Reset" ) );
    textLabelBCU2_4->setText( tr( "Flash Reset" ) );
    textLabelBCU2_5->setText( tr( "PCI Reset" ) );
    textLabelBCU2_6->setText( tr( "BUS Driver Enable" ) );
    textLabelBCU2_7->setText( tr( "BUS Driver Enable Status" ) );
    textLabelBCU2_8->setText( tr( "BUS Power Fault Ctrl" ) );
    textLabelBCU2_9->setText( tr( "BUS Power Fault" ) );
    textLabelBCU2_10->setText( tr( "System watchdog" ) );
    sprintf (temp,"Crate %1i",ICr2);
    crateBCU2_gr->setTitle( tr( temp ) );
    BCU2RBut[0]->setText( tr( "0" ) );
    BCU2RBut[1]->setText( tr( "1" ) );
    BCU2RBut[2]->setText( tr( "2" ) );
    BCU2RBut[3]->setText( tr( "3" ) );
    BCU2RBut[4]->setText( tr( "4" ) );
    BCU2RBut[5]->setText( tr( "5" ) );
    labelDigital->setText( tr( "Digital I/O" ) );
    textLabelBCU2a_1->setText( tr( "Driver Enabled" ) );
    textLabelBCU2a_2->setText( tr( "Crate is master" ) );
    textLabelBCU2a_3->setText( tr( "BUS system fault" ) );
    textLabelBCU2a_4->setText( tr( "VCC fault" ) );
    textLabelBCU2a_5->setText( tr( "AC power fault 0" ) );
    textLabelBCU2a_6->setText( tr( "AC power fault 1" ) );
    textLabelBCU2a_7->setText( tr( "AC power fault 2" ) );
    textLabelBCU2a_8->setText( tr( "IC disconnected" ) );
    textLabelBCU2a_9->setText( tr( "Overcurrent" ) );
    textLabelBCU2a_10->setText( tr( "Crate ID" ) );
    labelDiagn_2->setText( tr( "SIGGEN" ) );
    labelDiagn_3->setText( tr( "NIOS Fixed Area" ) );
    textLabelSIGGENa_1->setText( tr( "WhoAmI" ) );
    textLabelSIGGENa_2->setText( tr( "SoftwareRelease" ) );
    textLabelSIGGENa_3->setText( tr( "LogicRelease" ) );
    textLabelSIGGENa_4->setText( tr( "SerialNumber" ) );
    labelEnvBCU2->setText( tr( "Master crate global current levels" ) );
    textLabelBCU2b_1->setText( tr( "CurrentVCCP,  A" ) );
    BCU2MastBut[0]->setText( tr( "Plot" ) );
    textLabelBCU2b_2->setText( tr( "CurrentVSSP,  A" ) );
    BCU2MastBut[1]->setText( tr( "Plot" ) );
    textLabelBCU2b_3->setText( tr( "CurrentVP,      A" ) );
    BCU2MastBut[2]->setText( tr( "Plot" ) );
    textLabelBCU2c_0->setText( tr( "LocalCurrentThreshold,      A" ) );
    BCU2TrBut[0]->setText( tr( "Plot" ) );
    textLabelBCU2c_1->setText( tr( "VPSet,                             V" ) );
    BCU2TrBut[1]->setText( tr( "Plot" ) );
    textLabelBCU2c_2->setText( tr( "TotalCurrentThresholdPos,  A" ) );
    BCU2TrBut[2]->setText( tr( "Plot" ) );
    textLabelBCU2c_3->setText( tr( "TotalCurrentThresholdNeg,  A" ) );
    BCU2TrBut[3]->setText( tr( "Plot" ) );
    tabWidget->changeTab( TabPage, tr( "Crate BCU 2" ) );
    labelDiagn_4->setText( tr( "NIOS Fixed Area" ) );
    textLabelDSPa_1->setText( tr( "WhoAmI" ) );
    textLabelDSPa_2->setText( tr( "SoftwareRelease" ) );
    textLabelDSPa_3->setText( tr( "LogicRelease" ) );
    textLabelDSPa_4->setText( tr( "SerialNumber" ) );
    textLabelDSPa_5->setText( tr( "DiagnosticRecordPtr" ) );
    textLabelDSPa_6->setText( tr( "DiagnosticRecordLen" ) );
    textLabelDSPa_7->setText( tr( "RdDiagnosticRecordPtr" ) );
    textLabelDSPa_8->setText( tr( "WrDiagnosticRecordPtr" ) );
    labelTempDSPd->setText( tr( "Temperatures" ) );
    textLabelDSPd_1->setText( tr( "Stratix,   C" ) );
    DSPTempBut[0]->setText( tr( "Plot" ) );
    textLabelDSPd_2->setText( tr( "Power,   C" ) );
    DSPTempBut[1]->setText( tr( "Plot" ) );
    textLabelDSPd_3->setText( tr( "DSPs,    C" ) );
    DSPTempBut[2]->setText( tr( "Plot" ) );
    textLabelDSPd_4->setText( tr( "Drivers,  C" ) );
    DSPTempBut[3]->setText( tr( "Plot" ) );
    sprintf (temp,"Crate %1i",ICr3);
    crateSIGGEN_gr->setTitle( tr( temp ) );
    DSP1RBut[0]->setText( tr( "0" ) );
    DSP1RBut[1]->setText( tr( "1" ) );
    DSP1RBut[2]->setText( tr( "2" ) );
    DSP1RBut[3]->setText( tr( "3" ) );
    DSP1RBut[4]->setText( tr( "4" ) );
    DSP1RBut[5]->setText( tr( "5" ) );
    labelTempDSPc_0->setText( tr( "Coil currents (SPI),   A" ) );
    textLabelDSPc_1->setText( tr( "ch0" ) );
    DSPCurBut[0]->setText( tr( "Plot" ) );
    textLabelDSPc_2->setText( tr( "ch1" ) );
    DSPCurBut[1]->setText( tr( "Plot" ) );
    textLabelDSPc_3->setText( tr( "ch2" ) );
    DSPCurBut[2]->setText( tr( "Plot" ) );
    textLabelDSPc_4->setText( tr( "ch3" ) );
    DSPCurBut[3]->setText( tr( "Plot" ) );
    textLabelDSPc_5->setText( tr( "ch4" ) );
    DSPCurBut[4]->setText( tr( "Plot" ) );
    textLabelDSPc_6->setText( tr( "ch5" ) );
    DSPCurBut[5]->setText( tr( "Plot" ) );
    textLabelDSPc_7->setText( tr( "ch6" ) );
    DSPCurBut[6]->setText( tr( "Plot" ) );
    textLabelDSPc_8->setText( tr( "ch7" ) );
    DSPCurBut[7]->setText( tr( "Plot" ) );
    sprintf (temp,"Board %2i",IBo);
    crateDSPb_6->setTitle( tr( temp ) );
    DSP2RBut[0]->setText( tr( "0" ) );
    DSP2RBut[1]->setText( tr( "1" ) );
    DSP2RBut[2]->setText( tr( "2" ) );
    DSP2RBut[3]->setText( tr( "3" ) );
    DSP2RBut[4]->setText( tr( "4" ) );
    DSP2RBut[5]->setText( tr( "5" ) );
    DSP2RBut[6]->setText( tr( "6" ) );
    DSP2RBut[7]->setText( tr( "7" ) );
    DSP2RBut[8]->setText( tr( "8" ) );
    DSP2RBut[9]->setText( tr( "9" ) );
    DSP2RBut[10]->setText( tr( "10" ) );
    DSP2RBut[11]->setText( tr( "11" ) );
    DSP2RBut[12]->setText( tr( "12" ) );
    DSP2RBut[13]->setText( tr( "13" ) );
    labelDriverEnabled->setText( tr( "Driver Enabled" ) );
    textLabelDSPf_1->setText( tr( "ch0" ) );
    textLabelDSPf_2->setText( tr( "ch1" ) );
    textLabelDSPf_3->setText( tr( "ch2" ) );
    textLabelDSPf_4->setText( tr( "ch3" ) );
    textLabelDSPf_5->setText( tr( "ch4" ) );
    textLabelDSPf_6->setText( tr( "ch5" ) );
    textLabelDSPf_7->setText( tr( "ch6" ) );
    textLabelDSPf_8->setText( tr( "ch7" ) );
    labelDriverStatus->setText( tr( "Driver Status" ) );
    textLabelDSPe_1->setText( tr( "FPGA Reset" ) );
    textLabelDSPe_2->setText( tr( "DSP0 Reset" ) );
    textLabelDSPe_3->setText( tr( "DSP1 Reset" ) );
    textLabelDSPe_4->setText( tr( "Flash Reset" ) );
    textLabelDSPe_5->setText( tr( "BUS Power Fault Ctrl" ) );
    textLabelDSPe_6->setText( tr( "BUS Power Fault" ) );
    textLabelDSPe_7->setText( tr( "BUS Driver Enable" ) );
    textLabelDSPe_8->setText( tr( "Enable DSP Watchdog" ) );
    textLabelDSPe_9->setText( tr( "DSP0 watchdog expired" ) );
    textLabelDSPe_10->setText( tr( "DSP1 watchdog expired" ) );
    textLabelDSPb_0->setText( tr( "ADCspiOffset" ) );
    textLabelDSPb_1->setText( tr( "ADCspiGain" ) );
    textLabelDSPb_2->setText( tr( "ADCOffset" ) );
    textLabelDSPb_3->setText( tr( "ADCGain" ) );
    textLabelDSPb_4->setText( tr( "DACOffset" ) );
    textLabelDSPb_5->setText( tr( "DACGain" ) );
    sprintf (temp,"Board Channel %1i",ICh);
    crateDSPb_6_2->setTitle( tr( temp ) );
    DSP3RBut[0]->setText( tr( "0" ) );
    DSP3RBut[1]->setText( tr( "1" ) );
    DSP3RBut[2]->setText( tr( "2" ) );
    DSP3RBut[3]->setText( tr( "3" ) );
    DSP3RBut[4]->setText( tr( "4" ) );
    DSP3RBut[5]->setText( tr( "5" ) );
    DSP3RBut[6]->setText( tr( "6" ) );
    DSP3RBut[7]->setText( tr( "7" ) );
    tabWidget->changeTab( TabPage_2, tr( "Crate DSP" ) );
    listBox1->clear();
    listBox1->insertItem( tr( "NO WARNINGS" ) );
    tabWidget->changeTab( Widget18, tr( "Warnings" ) );
    listBox1_2->clear();
    listBox1_2->insertItem( tr( "NO ALARMS" ) );
    tabWidget->changeTab( Widget19, tr( "Alarms" ) );
    buttonStart->setText( tr( "&Stop" ) );
    buttonStart->setAccel( QKeySequence( tr( "Alt+S" ) ) );
    buttonStart_2->setText( tr( "&Help" ) );
    buttonStart_2->setAccel( QKeySequence( tr( "Alt+H" ) ) );
    buttonExit->setText( tr( "&Exit" ) );
    buttonExit->setAccel( QKeySequence( tr( "Alt+E" ) ) );
}

void topLBT672::newPlot()
{
    qWarning( "topLBT672::newPlot(): Not implemented yet" );
}

void topLBT672::Refresh()
{
     GetAll();
     SetAll();
     if ( !timer->isActive() ) timer->start (timeout,FALSE);
}





/*  Get All Housekeeper variables *************************************/
void topLBT672::GetAll()
{

    char *diagnapp = "HOUSEKPR00";
    char *family = "*";
    int   from = -1;
    int   to   = -1;
    int   timeout = 1000; // timeout in ms;
    ValueDict dict;

    std::string Family;
    int   Index;
    int i,stat;
    float mean,std;

    try{
        DiagnWhich which(family, from, to);
        dict = GetDiagnValue( diagnapp,which, timeout);
    } catch (AOException &e) {
            qWarning( "topLBT672::GetAll(): error" );
//        _logger->log(Logger::LOG_LEV_ERROR, e.what());
//        return e._errcode;
    }

   //Iterate over the dictionary of the retrieved DiagnVars.
   for (ValueDict::iterator it = dict.begin(); it != dict.end();  it++){
        DiagnWhich wh = (*it).first;
        DiagnValue va = (*it).second;

        //DiagnVar name & index
        Family = wh.Family();   // DiagnVar name.
        for (i=0; i<nfamily; i++) {
            if (Family == (std::string) FamilyNames[i]) {
               Index  = wh.From();     // DiagnVar index
               // Values
               mean = (float) va.Mean();     // DiagnVar value (average of a running mean)
               std  = (float) va.Stddev();   // Stddev of the running mean
               //va.Last();     // Instantaneuos value (last value inserted in the running mean)
               //va.Time();     // Time (average of the time stamp of the values in the running mean)
               stat = va.Status();   // Status: 0=OK, 1=Warning, 2=Error
               if (i<7) {
                  DiagAppDataVal[i]  = mean;
                  DiagAppDataStd[i]  = std;
                  DiagAppDataStat[i] = stat;
               }
               if (i>6 && i<11) {
                  BCU2TrVal[i]  = mean;
                  BCU2TrStd[i]  = std;
                  BCU2TrStat[i] = stat;
               }
               if (i>10 && i<22) {
                  BCU1NiosVal[Index][i]  = mean;
                  BCU1NiosStd[Index][i]  = std;
                  BCU1NiosStat[Index][i] = stat;
               }
            }
        }
//        _logger->log(Logger::LOG_LEV_DEBUG, "[GetDiagnValue] %s  %s --- %s [%s:%d]",
//                diagnapp, wh.str().c_str(), va.str().c_str(), __FILE__,__LINE__);
    }
}


/*
void topLBT672::GetAll()           
{
    qWarning( "topLBT672::GetAll(): called" );

   DiagAppDataVal[0] = 15674.036478;
   DiagAppDataStd[0] = 21.036478;
   DiagAppDataVal[3] = 2674.036478;
   DiagAppDataStd[3] = 1.036478;
   DiagAppDataStat[3] = 0;
   DiagAppDataVal[4] = 35674.036478;
   DiagAppDataStd[4] = 2.036478;
   DiagAppDataStat[4] = 2;
   DiagAppDataVal[5] = 23674.036478;
   DiagAppDataStd[5] = 1.036478;
   DiagAppDataStat[5] = 0;
   DiagAppDataVal[6] = 2.;
   DiagAppDataStd[6] = 0.;

   ExternalVal[0] = 113.234;
   ExternalStd[0] =  0.9;
   ExternalVal[1] = 1132.234;
   ExternalStd[1] =  1.9;
   ExternalStat[1] =  2;
   ExternalVal[2] = 11132.234;
   ExternalStd[2] =  2.9;
   ExternalStat[2] =  0;

   BCU1NiosVal[0][0] = 113.234;
   BCU1NiosStd[0][0] =  0.9;
   BCU1NiosVal[0][1] = 1132.234;
   BCU1NiosStd[0][1] =  1.9;
   BCU1NiosStat[0][1] =  0;
   BCU1NiosVal[0][2] = 11132.234;
   BCU1NiosStd[0][2] =  2.9;
   BCU1NiosStat[0][2] =  0;

   BCU1NiosVal[1][3] = 113.234;
   BCU1NiosStd[1][3] =  0.9;
   BCU1NiosStat[1][3] =  0;
   BCU1NiosVal[1][4] = 1132.234;
   BCU1NiosStd[1][4] =  1.9;
   BCU1NiosStat[1][4] =  0;
   BCU1NiosVal[1][2] = 11132.234;
   BCU1NiosStd[1][2] =  2.9;
   BCU1NiosStat[1][2] =  2;

   DiagAppDataStat[0] =2;
   DiagAppDataStat[6] =2;
   ExternalStat[0] = 2;
   ExternalStat[8] = 2;
   BCU1NiosStat[0][0] = 2;
   BCU1NiosStat[0][10] = 2;
   BCU1NiosStat[5][0] = 2;
   BCU1NiosStat[5][10] = 2;
   BCU1NiosStat[6][0] = 2;
   BCU1NiosStat[6][10] = 2;
   BCU1EnvStat[0][0] = 2;
   BCU1EnvStat[0][5] = 2;
   BCU1EnvStat[5][0] = 2;
   BCU1EnvStat[5][5] = 2;
   BCU1VolStat[0][0] = 1;
   BCU1VolStat[0][9] = 1;
   BCU1VolStat[5][0] = 1;
   BCU1VolStat[5][9] = 1;
   BCU1TempStat[0][0] = 2;
   BCU1TempStat[0][1] = 2;
   BCU1TempStat[5][0] = 2;
   BCU1TempStat[5][1] = 2;
   BCU1TempStat[6][0] = 2;
   BCU1TempStat[6][1] = 2;
   BCU1SGNTempStat[0][0] = 2;
   BCU1SGNTempStat[0][2] = 2;
   BCU1SGNTempStat[5][0] = 2;
   BCU1SGNTempStat[5][2] = 2;
   BCU2ResetStat[0][0] = 1;
   BCU2ResetStat[0][9] = 1;
   BCU2ResetStat[5][0] = 1;
   BCU2ResetStat[5][9] = 1;
   BCU2DigStat[0][0] = 1;
   BCU2DigStat[0][9] = 1;
   BCU2DigStat[5][0] = 1;
   BCU2DigStat[5][9] = 1;
   BCU2MastStat[0] = 1;
   BCU2MastStat[1] = 0;
   BCU2MastStat[2] = 1;
   BCU2SGNStat[0] = 1;
   BCU2SGNStat[1] = 0;
   BCU2SGNStat[3] = 1;
   BCU2TrStat[0] = 1;
   BCU2TrStat[1] = 2;
   BCU2TrStat[3] = 1;
   DSPNiosStat[0] = 1;
   DSPNiosStat[7] = 1;
   DSPCurStat[0][0][0] = 1;
   DSPCurStat[0][0][7] = 1;
   DSPDrSStat[0][0][0] = 1;
   DSPDrSStat[0][0][9] = 1;
   DSPDrStat[0][0][0] = 1;
   DSPDrStat[0][0][7] = 1;
   DSPTempStat[0][0][0] = 1;
   DSPTempStat[0][0][3] = 1;
   DSPadStat[0][0][0][0] = 2;
   DSPadStat[0][0][0][5] = 2;

}
*/

void topLBT672::SetICr1()
{
     int i;
     for (i=0; i<7; i++) {
         if ( BCU1RBut[i]->isChecked() ) {
            if (ICr1 != i) {
               ICr1=i;
               SetCrateBCU1();
            }
         }
     }
}

void topLBT672::SetICr2()
{
     int i;
     for (i=0; i<6; i++) {
         if ( BCU2RBut[i]->isChecked() ) {
            if (ICr2 != i) {
               ICr2=i;
               SetCrateBCU2();
            }
         }
     }
}

void topLBT672::SetICr3()
{
     int i;
     for (i=0; i<6; i++) {
         if ( DSP1RBut[i]->isChecked() ) {
            if (ICr3 != i) {
               ICr3=i;
               SetCrateDSP();
            }
         }
     }
}

void topLBT672::SetIBo()
{
     int i;
     for (i=0; i<14; i++) {
         if ( DSP2RBut[i]->isChecked() ) {
            if (IBo != i) {
               IBo=i;
               SetCrateDSP();
            }
         }
     }
}

void topLBT672::SetICh()
{
     int i;
     for (i=0; i<8; i++) {
         if ( DSP3RBut[i]->isChecked() ) {
            if (ICh != i) {
               ICh=i;
               SetCrateDSP();
            }
         }
     }
}

void topLBT672::SetAll()
{
   SetSystemSummary();
   SetCrateBCU1();
   SetCrateBCU2();
   SetCrateDSP();
   CheckWarnAlarms();
   SetWarnings();
   SetAlarms();
}

void topLBT672::SetSystemSummary()
{
   char temp[300];
   int i;

   // DiagnAppData panel
   for (i=0; i<7; i++) {
       sprintf (temp,"%10.3f +- %8.3f",DiagAppDataVal[i],DiagAppDataStd[i]);
       DiagnAppDataLab[i]->setText ( QString(temp));
       if (DiagAppDataStat[i] == 0) DiagnAppDataLed[i]->setColor( QColor (0,255,0));
       if (DiagAppDataStat[i] == 1) DiagnAppDataLed[i]->setColor( QColor (255,255,0));
       if (DiagAppDataStat[i] == 2) DiagnAppDataLed[i]->setColor( QColor (255,0,0));
   }
   // External.... panel
   for (i=0; i<9; i++) {
       sprintf (temp,"%10.3f +- %8.3f",ExternalVal[i],DiagAppDataStd[i]);
       ExternalLab[i]->setText (QString(temp));
       if (ExternalStat[i] == 0) ExternalLed[i]->setColor( QColor (0,255,0));
       if (ExternalStat[i] == 1) ExternalLed[i]->setColor( QColor (255,255,0));
       if (ExternalStat[i] == 2) ExternalLed[i]->setColor( QColor (255,0,0));
   }
}

void topLBT672::SetCrateBCU1()
{
   char temp[300];
   int i,j;

   if (ICr1 < 6) sprintf (temp,"Crate %1i",ICr1);
   else strcpy(temp,"Switch BCU");
   crateBCU1_gr->setTitle( tr( temp ) );

   if (ICr1==6) j=1; else j=0;
   // NIOS Fixed Area  panel
   for (i=0; i<11; i++) {
       sprintf (temp,"%10.3f +- %8.3f",BCU1NiosVal[ICr1][i],BCU1NiosStd[ICr1][i]);
       if (BCU1NiosStat[ICr1][i] == 0) BCU1NiosLed[i]->setColor( QColor (0,255,0));
       if (BCU1NiosStat[ICr1][i] == 1) BCU1NiosLed[i]->setColor( QColor (255,255,0));
       if (BCU1NiosStat[ICr1][i] == 2) BCU1NiosLed[i]->setColor( QColor (255,0,0));
       if (j==1 && i==7) {
          strcpy(temp,"  ");
          BCU1NiosLed[i]->setColor( QColor (192,192,192));
       }  
       BCU1NiosLab[i]->setText ( QString(temp));
   }
   // Environment panel
   if (ICr1<6) {
      for (i=0; i<6; i++) {
          sprintf (temp,"%10.3f +- %8.3f",BCU1EnvVal[ICr1][i],BCU1EnvStd[ICr1][i]);
          BCU1EnvLab[i]->setText (QString(temp));
          if (BCU1EnvStat[ICr1][i] == 0) BCU1EnvLed[i]->setColor( QColor (0,255,0));
          if (BCU1EnvStat[ICr1][i] == 1) BCU1EnvLed[i]->setColor( QColor (255,255,0));
          if (BCU1EnvStat[ICr1][i] == 2) BCU1EnvLed[i]->setColor( QColor (255,0,0));
      }
   } else {
      for (i=0; i<6; i++) {
          BCU1EnvLab[i]->setText (QString("    "));
          BCU1EnvLed[i]->setColor( QColor (192,192,192));
      }
   }
   // Voltage and Currents panel
   if (ICr1<6) {
      for (i=0; i<10; i++) {
          sprintf (temp,"%10.3f +- %8.3f",BCU1VolVal[ICr1][i],BCU1VolStd[ICr1][i]);
          BCU1VolLab[i]->setText (QString(temp));
          if (BCU1VolStat[ICr1][i] == 0) BCU1VolLed[i]->setColor( QColor (0,255,0));
          if (BCU1VolStat[ICr1][i] == 1) BCU1VolLed[i]->setColor( QColor (255,255,0));
          if (BCU1VolStat[ICr1][i] == 2) BCU1VolLed[i]->setColor( QColor (255,0,0));
      }
   } else {
      for (i=0; i<10; i++) {
          BCU1VolLab[i]->setText (QString("    "));
          BCU1VolLed[i]->setColor( QColor (192,192,192));
      }
   }
   // Temperatures  panel
   for (i=0; i<2; i++) {
       sprintf (temp,"%10.3f +- %8.3f",BCU1TempVal[ICr1][i],BCU1TempStd[ICr1][i]);
       BCU1TempLab[i]->setText ( QString(temp));
       if (BCU1TempStat[ICr1][i] == 0) BCU1TempLed[i]->setColor( QColor (0,255,0));
       if (BCU1TempStat[ICr1][i] == 1) BCU1TempLed[i]->setColor( QColor (255,255,0));
       if (BCU1TempStat[ICr1][i] == 2) BCU1TempLed[i]->setColor( QColor (255,0,0));
   }
   // SIGGEN Temperatures panel
   if (ICr1<6) {
      for (i=0; i<3; i++) {
          sprintf (temp,"%10.3f +- %8.3f",BCU1SGNTempVal[ICr1][i],BCU1SGNTempStd[ICr1][i]);
          BCU1SGNTempLab[i]->setText (QString(temp));
          if (BCU1SGNTempStat[ICr1][i] == 0) BCU1SGNTempLed[i]->setColor( QColor (0,255,0));
          if (BCU1SGNTempStat[ICr1][i] == 1) BCU1SGNTempLed[i]->setColor( QColor (255,255,0));
          if (BCU1SGNTempStat[ICr1][i] == 2) BCU1SGNTempLed[i]->setColor( QColor (255,0,0));
      }
   } else {
      for (i=0; i<3; i++) {
          BCU1SGNTempLab[i]->setText (QString("    "));
          BCU1SGNTempLed[i]->setColor( QColor (192,192,192));
      }
   }
}

void topLBT672::SetCrateBCU2()
{
   char temp[300];
   int i;

   sprintf (temp,"Crate %1i",ICr2);
   crateBCU2_gr->setTitle( tr( temp ) );

   // Reset status  panel
   for (i=0; i<10; i++) {
       if (BCU2ResetStat[ICr2][i] == 0) BCU2ResetLed[i]->setState( KLed::Off );
       if (BCU2ResetStat[ICr2][i] == 1) BCU2ResetLed[i]->setState( KLed::On );
   }
   // Digital I/O  panel
   for (i=0; i<10; i++) {
       if (BCU2DigStat[ICr2][i] == 0) BCU2DigLed[i]->setState( KLed::Off );
       if (BCU2DigStat[ICr2][i] == 1) BCU2DigLed[i]->setState( KLed::On );
   }
   // Master crate  panel
   for (i=0; i<3; i++) {
       sprintf (temp,"%10.3f +- %8.3f",BCU2MastVal[i],BCU2MastStd[i]);
       BCU2MastLab[i]->setText ( QString(temp));
       if (BCU2MastStat[i] == 0) BCU2MastLed[i]->setColor( QColor (0,255,0));
       if (BCU2MastStat[i] == 1) BCU2MastLed[i]->setColor( QColor (255,255,0));
       if (BCU2MastStat[i] == 2) BCU2MastLed[i]->setColor( QColor (255,0,0));
   }
   // SIGGEN NIOS Fixed Area panel
   for (i=0; i<4; i++) {
       sprintf (temp,"%10.3f +- %8.3f",BCU2SGNVal[i],BCU2SGNStd[i]);
       BCU2SGNLab[i]->setText ( QString(temp));
       if (BCU2SGNStat[i] == 0) BCU2SGNLed[i]->setColor( QColor (0,255,0));
       if (BCU2SGNStat[i] == 1) BCU2SGNLed[i]->setColor( QColor (255,255,0));
       if (BCU2SGNStat[i] == 2) BCU2SGNLed[i]->setColor( QColor (255,0,0));
   }
   // Thresholds panel
   for (i=0; i<4; i++) {
       sprintf (temp,"%10.3f +- %8.3f",BCU2TrVal[i],BCU2TrStd[i]);
       BCU2TrLab[i]->setText ( QString(temp));
       if (BCU2TrStat[i] == 0) BCU2TrLed[i]->setColor( QColor (0,255,0));
       if (BCU2TrStat[i] == 1) BCU2TrLed[i]->setColor( QColor (255,255,0));
       if (BCU2TrStat[i] == 2) BCU2TrLed[i]->setColor( QColor (255,0,0));
   }
}

void topLBT672::SetCrateDSP()
{
   char temp[300];
   int i;


   sprintf (temp,"Crate %1i",ICr3);
   crateSIGGEN_gr->setTitle( tr( temp ) );
   sprintf (temp,"Board %2i",IBo);
   crateDSPb_6->setTitle( tr( temp ) );
   sprintf (temp,"Board Channel %1i",ICh);
   crateDSPb_6_2->setTitle( tr( temp ) );

//    sprintf (temp,"cr %i %i %i bo %i ch %i",ICr1,ICr2,ICr3,IBo,ICh);
//    qWarning( temp );
   //  NIOS Fixed Area panel
   for (i=0; i<8; i++) {
       sprintf (temp,"%10.3f +- %8.3f",DSPNiosVal[i],DSPNiosStd[i]);
       DSPNiosLab[i]->setText ( QString(temp));
       if (DSPNiosStat[i] == 0) DSPNiosLed[i]->setColor( QColor (0,255,0));
       if (DSPNiosStat[i] == 1) DSPNiosLed[i]->setColor( QColor (255,255,0));
       if (DSPNiosStat[i] == 2) DSPNiosLed[i]->setColor( QColor (255,0,0));
   }
   //  Coil currents (SPI) panel
   for (i=0; i<8; i++) {
       sprintf (temp,"%10.3f +- %8.3f",DSPCurVal[ICr3][IBo][i],DSPCurStd[ICr3][IBo][i]);
       DSPCurLab[i]->setText ( QString(temp));
       if (DSPCurStat[ICr3][IBo][i] == 0) DSPCurLed[i]->setColor( QColor (0,255,0));
       if (DSPCurStat[ICr3][IBo][i] == 1) DSPCurLed[i]->setColor( QColor (255,255,0));
       if (DSPCurStat[ICr3][IBo][i] == 2) DSPCurLed[i]->setColor( QColor (255,0,0));
   }
   //  Driver Status panel
   for (i=0; i<10; i++) {
       if (DSPDrSStat[ICr3][IBo][i] == 0) DSPDrSLed[i]->setState( KLed::Off );
       if (DSPDrSStat[ICr3][IBo][i] == 1) DSPDrSLed[i]->setState( KLed::On );
   }
   //  Temperatures panel
   for (i=0; i<4; i++) {
       sprintf (temp,"%10.3f +- %8.3f",DSPTempVal[ICr3][IBo][i],DSPTempStd[ICr3][IBo][i]);
       DSPTempLab[i]->setText ( QString(temp));
       if (DSPTempStat[ICr3][IBo][i] == 0) DSPTempLed[i]->setColor( QColor (0,255,0));
       if (DSPTempStat[ICr3][IBo][i] == 1) DSPTempLed[i]->setColor( QColor (255,255,0));
       if (DSPTempStat[ICr3][IBo][i] == 2) DSPTempLed[i]->setColor( QColor (255,0,0));
   }
   //  Driver Enabled panel
   for (i=0; i<8; i++) {
       if (DSPDrStat[ICr3][IBo][i] == 0) DSPDrLed[i]->setState( KLed::Off );
       if (DSPDrStat[ICr3][IBo][i] == 1) DSPDrLed[i]->setState( KLed::On );
   }
   //  ADC panel
   for (i=0; i<6; i++) {
       sprintf (temp,"%10.3f +- %8.3f",DSPadVal[ICr3][IBo][ICh][i],DSPadStd[ICr3][IBo][ICh][i]);
       DSPadLab[i]->setText ( QString(temp));
//    sprintf (temp,"cr3  %i bo %i ch %i i %i Stat %i",ICr3,IBo,ICh,i,DSPadStat[ICr3][IBo][ICh][i]);
//    qWarning( temp );
       if (DSPadStat[ICr3][IBo][ICh][i] == 0) DSPadLed[i]->setColor( QColor (0,255,0));
       if (DSPadStat[ICr3][IBo][ICh][i] == 1) DSPadLed[i]->setColor( QColor (255,255,0));
       if (DSPadStat[ICr3][IBo][ICh][i] == 2) DSPadLed[i]->setColor( QColor (255,0,0));
   }
}

void topLBT672::CheckWarnAlarms()
{
 int i,ic,ib,ic1,j;
 char temp[300];

 //System Summary tab -------------------------------------------------
   // DiagnAppData panel 
   for (i=0; i<7; i++) {
       if (DiagAppDataStat[i] == 1) {
           sprintf (temp," %s(Summary) ",FamilyNames[DiagAppDataInd[0]+i]);
           Warnings.push_back(QString(temp));
       }
       if (DiagAppDataStat[i] == 2) {
           sprintf (temp," %s(Summary) ",FamilyNames[DiagAppDataInd[0]+i]);
           Alarms.push_back(QString(temp));
       }
   }
   // External.... panel
   for (i=0; i<9; i++) {
       if (ExternalStat[i] == 1) {
           sprintf (temp," %s(Summary) ",FamilyNames[ExternalInd[0]+i]);
           Warnings.push_back(QString(temp));
       }
       if (ExternalStat[i] == 2) {
           sprintf (temp," %s(Summary) ",FamilyNames[ExternalInd[0]+i]);
           Alarms.push_back(QString(temp));
       }
   }
 //Crate BCU1 tab --------------------------------------------------
   // NIOS Fixed Area  panel
   for (ic=0; ic<7; ic++) {
       if (ic==6) j=1; else j=0;
       for (i=0; i<11; i++) {
           if (BCU1NiosStat[ic][i] == 1) {
               if (j==0) sprintf (temp," %s(BCU1,Crate=%1i) ",FamilyNames[BCU1NiosInd[j][0]+i],ic);
               if (j==1) sprintf (temp," %s(BCU1,SwitchBCU) ",FamilyNames[BCU1NiosInd[j][0]+i]);
               Warnings.push_back(QString(temp));
           }
           if (BCU1NiosStat[ic][i] == 2) {
               if (j==0) sprintf (temp," %s(BCU1,Crate=%1i) ",FamilyNames[BCU1NiosInd[j][0]+i],ic);
               if (j==1) sprintf (temp," %s(BCU1,SwitchBCU) ",FamilyNames[BCU1NiosInd[j][0]+i]);
               Alarms.push_back(QString(temp));
           }
       }
   }
   // Environment panel
   for (ic=0; ic<6; ic++) {
       for (i=0; i<6; i++) {
           if (BCU1EnvStat[ic][i] == 1) {
               sprintf (temp," %s(BCU1,Crate=%1i) ",FamilyNames[BCU1EnvInd[0]+i],ic);
               Warnings.push_back(QString(temp));
           }
           if (BCU1EnvStat[ic][i] == 2) {
               sprintf (temp," %s(BCU1,Crate=%1i) ",FamilyNames[BCU1EnvInd[0]+i],ic);
               Alarms.push_back(QString(temp));
           }
       }
   } 
   // Voltage and Currents panel
   for (ic=0; ic<6; ic++) {
       for (i=0; i<10; i++) {
           if (BCU1VolStat[ic][i] == 1) {
               sprintf (temp," %s(BCU1,Crate=%1i) ",FamilyNames[BCU1VolInd[0]+i],ic);
               Warnings.push_back(QString(temp));
           }
           if (BCU1VolStat[ic][i] == 2) {
               sprintf (temp," %s(BCU1,Crate=%1i) ",FamilyNames[BCU1VolInd[0]+i],ic);
               Alarms.push_back(QString(temp));
           }
       }
   } 
   // Temperatures  panel
   for (ic=0; ic<7; ic++) {
       if (ic==6) j=1; else j=0;
       for (i=0; i<2; i++) {
           if (BCU1TempStat[ic][i] == 1) {
               if (j==0) sprintf (temp," %s(BCU1,Crate=%1i) ",FamilyNames[BCU1TempInd[j][0]+i],ic);
               if (j==1) sprintf (temp," %s(BCU1,SwitchBCU) ",FamilyNames[BCU1TempInd[j][0]+i]);
               Warnings.push_back(QString(temp));
           }
           if (BCU1TempStat[ic][i] == 2) {
               if (j==0) sprintf (temp," %s(BCU1,Crate=%1i) ",FamilyNames[BCU1TempInd[j][0]+i],ic);
               if (j==1) sprintf (temp," %s(BCU1,SwitchBCU) ",FamilyNames[BCU1TempInd[j][0]+i]);
               Alarms.push_back(QString(temp));
           }
       }
   }
   // SIGGEN Temperatures panel
   for (ic=0; ic<6; ic++) {
       for (i=0; i<3; i++) {
           if (BCU1SGNTempStat[ic][i] == 1) {
               sprintf (temp," %s(BCU1,Crate=%1i) ",FamilyNames[BCU1SGNTempInd[0]+i],ic);
               Warnings.push_back(QString(temp));
           }
           if (BCU1SGNTempStat[ic][i] == 2) {
               sprintf (temp," %s(BCU1,Crate=%1i) ",FamilyNames[BCU1SGNTempInd[0]+i],ic);
               Alarms.push_back(QString(temp));
           }
       }
   } 
 //Crate BCU2 tab --------------------------------------------------
   // Master  panel
   for (i=0; i<3; i++) {
       if (BCU2MastStat[i] == 1) {
           sprintf (temp," %s(BCU2) ",FamilyNames[BCU2MastInd[0]+i]);
           Warnings.push_back(QString(temp));
       }
       if (BCU2MastStat[i] == 2) {
           sprintf (temp," %s(BCU2) ",FamilyNames[BCU2MastInd[0]+i]);
           Alarms.push_back(QString(temp));
       }
    }
   // SIGGEN NIOS Fixed Area panel
   for (i=0; i<4; i++) {
       if (BCU2SGNStat[i] == 1) {
           sprintf (temp," %s(BCU2) ",FamilyNames[BCU2SGNInd[0]+i]);
           Warnings.push_back(QString(temp));
       }
       if (BCU2SGNStat[i] == 2) {
           sprintf (temp," %s(BCU2) ",FamilyNames[BCU2SGNInd[0]+i]);
           Alarms.push_back(QString(temp));
       }
   }
   // Thresholds panel
   for (i=0; i<4; i++) {
       if (BCU2TrStat[i] == 1) {
           sprintf (temp," %s(BCU2) ",FamilyNames[BCU2TrInd[0]+i]);
           Warnings.push_back(QString(temp));
       }
       if (BCU2TrStat[i] == 2) {
           sprintf (temp," %s(BCU2) ",FamilyNames[BCU2TrInd[0]+i]);
           Alarms.push_back(QString(temp));
       }
   } 
 //Crate DSP tab --------------------------------------------------
   // NIOS Fixed Area panel
   for (i=0; i<8; i++) {
       if (DSPNiosStat[i] == 1) {
           sprintf (temp," %s(DSP) ",FamilyNames[DSPNiosInd[0]+i]);
           Warnings.push_back(QString(temp));
       }
       if (DSPNiosStat[i] == 2) {
           sprintf (temp," %s(DSP) ",FamilyNames[DSPNiosInd[0]+i]);
           Alarms.push_back(QString(temp));
       }
   } 
   // Coil currents (SPI) panel
   for (ic=0; ic<6; ic++) {
       for (i=0; i<8; i++) {
           for (ib=0; ib<14; ib++) {
               if (DSPCurStat[ic][ib][i] == 1) {
                   sprintf (temp," %s(DSP,Cr=%1i,Bo=%1i,Ch=%1i) ",FamilyNames[DSPCurInd[0]],ic,ib,i);
                   Warnings.push_back(QString(temp));
               }
               if (DSPCurStat[ic][ib][i] == 2) {
                   sprintf (temp," %s(DSP,Cr=%1i,Bo=%1i,Ch=%1i) ",FamilyNames[DSPCurInd[0]],ic,ib,i);
                   Alarms.push_back(QString(temp));
               }
           }
       }
   } 
   // Temperatures panel
   for (ic=0; ic<6; ic++) {
       for (i=0; i<4; i++) {
           for (ib=0; ib<14; ib++) {
               if (DSPTempStat[ic][ib][i] == 1) {
                   sprintf (temp," %s(DSP,Cr=%1i,Board=%1i) ",FamilyNames[DSPTempInd[0]+i],ic,ib);
                   Warnings.push_back(QString(temp));
               }
               if (DSPTempStat[ic][ib][i] == 2) {
                   sprintf (temp," %s(DSP,Cr=%1i,Board=%1i) ",FamilyNames[DSPTempInd[0]+i],ic,ib);
                   Alarms.push_back(QString(temp));
               }
           }
       }
   } 
   // ADC panel
   for (ic=0; ic<6; ic++) {
       for (i=0; i<6; i++) {
           for (ib=0; ib<14; ib++) {
               for (ic1=0; ic1<8; ic1++) {
                   if (DSPadStat[ic][ib][ic1][i] == 1) {
                       sprintf (temp," %s(DSP,Cr=%1i,Bo=%1i,Ch=%1i) ",FamilyNames[DSPadInd[0]+i],ic,ib,ic1);
                       Warnings.push_back(QString(temp));
                   }
                   if (DSPadStat[ic][ib][ic1][i] == 2) {
                       sprintf (temp," %s(DSP,Cr=%1i,Bo=%1i,Ch=%1i) ",FamilyNames[DSPadInd[0]+i],ic,ib,ic1);
                       Alarms.push_back(QString(temp));
                   }
               }
           }
       }
   } 
}

void topLBT672::SetWarnings()
{
    int n=0;
    char temp[300];

    listBox1->clear();
    if (Warnings.size() == 0) {
       listBox1->insertItem( tr( "NO WARNINGS" ) );
       sprintf (temp,"               ");
       warnLabel->setText( temp );
       warnLabel->setPaletteBackgroundColor( QColor( 192,192,192 ) );
    } else {
        vector<string>::iterator p=Warnings.begin();
        while (p!=Warnings.end()) {
              listBox1->insertItem( *p);
              p++;
              n++;
        }
        sprintf (temp," %4i Warnings ",n);
        warnLabel->setText( temp );
        warnLabel->setPaletteBackgroundColor( QColor( 255,255,0 ) );
        Warnings.clear();
    }
}

void topLBT672::SetAlarms()
{
    int n=0;
    char temp[300];

    listBox1_2->clear();
    if (Alarms.size() == 0) {
       listBox1_2->insertItem( tr( "NO ALARMS" ) );
       sprintf (temp,"             ");
       alarmLabel->setText( temp );
       alarmLabel->setPaletteBackgroundColor( QColor( 192,192,192 ) );
    } else {
        vector<string>::iterator p=Alarms.begin();
        while (p!=Alarms.end()) {
              listBox1_2->insertItem( *p);
              p++;
              n++;
        }
        sprintf (temp," %4i Alarms ",n);
        alarmLabel->setText( temp );
        alarmLabel->setPaletteBackgroundColor( QColor( 255,0,0 ) );
        Alarms.clear();
    }
}

void topLBT672::StopStartTimer()
{
    if ( QString::compare(buttonStart->text(), "&Stop") == 0 ) {
       timer->stop();
       buttonStart->setText( tr( "&Start" ) );
       buttonStart->setPaletteBackgroundColor( QColor( 200, 0, 0 ) );
    } else {
       Refresh();
       buttonStart->setText( tr( "&Stop" ) );
       buttonStart->setPaletteBackgroundColor( QColor( 255, 255, 127 ) );
    }
}
