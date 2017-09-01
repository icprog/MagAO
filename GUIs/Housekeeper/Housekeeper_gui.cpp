 /****************************************************************************
** 
**
** 
**
** 
****************************************************************************/

extern "C"{
//	#include "msglib.h"
	#include <stdlib.h>
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
#include "Utils.h"

using namespace Arcetri;
using namespace Arcetri::Diagnostic;

/*
#include "Logger.h"

Logger*		    _logger = Logger::get("MAIN", Logger::LOG_LEV_DEBUG);	// Use the same name of the AOApp main logger
*/

#include "Housekeeper_gui.h"
#include "PlotForm.h"

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
#include <qmessagebox.h>
#include <qwhatsthis.h>
#include <time.h>
#include <qdatetime.h> 
#include "kled.h"
#include "qwt_plot.h"
#include "qwt_plot_zoomer.h"
#include "qwt_text.h"
#include "qwt_text_label.h"
#include "qwt_legend.h"
#include "qwt_legend_item.h"
#include "qwt_plot_curve.h"


/* INITIALIZATION */
/****  Housekeeper Family Names,  from housekeeper.param ***********************/
char *FamilyName[] = {                                                           //1st-last indexes in array
 (char *)"DIAGNAPPFRAMERATE",
 (char *)"DIAGNAPPFASTVARSRATE",
 (char *)"DIAGNAPPSLOWVARSRATE",                  // ----------------------------------------------1END------
 (char *)"DIAGNAPPFRAMETIMESTAMP",                // DIAGNAPP FIXED GENERAL DATA: 1 VALUE/LINE  0-6  1BEG------
 (char *)"DIAGNAPPFRAMECOUNTER",                  // NEXT 7 LINES ARE
 (char *)"DIAGNAPPFASTVARSCOUNTER",               // SHOWN ON "SYSTEM SUMMARY" LEFT PANEL
 (char *)"DIAGNAPPSLOWVARSCOUNTER",
 (char *)"BCULOCALCURRENTTHRESHOLD",              // FIXED FOR BCU DATA: 1 VALUE/LINE         7-10   2BEG------ 
 (char *)"BCUVPSET",                              // NEXT 4 LINES ARE
 (char *)"BCUTOTALCURRENTTHRESHOLDPOS",           // SHOWN ON "CRATE BCU2" RIGHT/DOWN PANEL
 (char *)"BCUTOTALCURRENTTHRESHOLDNEG",           // ----------------------------------------------2END------
 (char *)"BCUCRATEID",                            // FIXED FOR EACH BCU CRATE DATA: 1 VALUE/CRATE, 11-21 2BEG------
 (char *)"BCUWHOAMI",                             //  6 CRATES    NEXT 11 LINES ARE
 (char *)"BCUSOFTWARERELEASE",                    // SHOWN ON "CRATE BCU1" LEFT/UPPER PANEL
 (char *)"BCULOGICRELEASE",                       // IN CORRESPONDANCE TO CRATE NUMBER
 (char *)"BCUIPADDRESS",                          //
 (char *)"BCUFRAMESCOUNTER",                      // FOR EACH LINE THERE ARE 6 VALUES
 (char *)"BCUSERIALNUMBER",
 (char *)"BCUPOWERBACKPLANESERIAL",
 (char *)"BCUENABLEMASTERDIAGNOSTIC",
 (char *)"BCUDECIMATIONFACTOR",
 (char *)"BCUREMOTEIPADDRESS",                    // ----------------------------------------------3END------
 (char *)"SWITCHCRATEID",                         // FIXED FOR BCU SWITCH DATA: 1 VALUE/SWITCH,  22-32  4BEG------
 (char *)"SWITCHWHOAMI",                          //  1 SWITCH     NEXT 11 LINES ARE
 (char *)"SWITCHSOFTWARERELEASE",                 // SHOWN ON "CRATE BCU1" LEFT/UPPER PANEL
 (char *)"SWITCHLOGICRELEASE",                    // WHEN THE "SWITCH BCU" IS CHOSEN INSTEAD
 (char *)"SWITCHIPADDRESS",                       //  OF "CRATE NUMBER"
 (char *)"SWITCHFRAMESCOUNTER",
 (char *)"SWITCHSERIALNUMBER",                    
 (char *)"SWITCHPOWERBACKPLANESERIAL",            // IS NOT APPLIED FOR SWITCH, SHOULD BE SHOWN DEEM
 (char *)"SWITCHENABLEMASTERDIAGNOSTIC",
 (char *)"SWITCHDECIMATIONFACTOR",
 (char *)"SWITCHREMOTEIPADDRESS",                 // ----------------------------------------------4END------
 (char *)"DSPWHOAMI",                             // FIXED FOR DSP DATA: 1 VALUE/LINE            33-40  5BEG------
 (char *)"DSPSOFTWARERELEASE",                    // NEXT 8 LINES ARE
 (char *)"DSPLOGICRELEASE",                       // SHOWN ON "CRATE DSP" LEFT/UPPER PANEL
 (char *)"DSPSERIALNUMBER",                       
 (char *)"DSPDIAGNOSTICRECORDPTR",                
 (char *)"DSPDIAGNOSTICRECORDLEN",                
 (char *)"DSPRDDIAGNOSTICRECORDPTR",
 (char *)"DSPWRDIAGNOSTICRECORDPTR",              // ----------------------------------------------5END------
 (char *)"DSPADCSPIOFFSET",                       // 1 VALUE/CHANNEL(8)/BOARD(14)/CRATE(6)       41-46  6BEG------
 (char *)"DSPADCSPIGAIN",                         // NEXT 6  LINES ARE
 (char *)"DSPADCOFFSET",                          // SHOWN ON "CRATE DSP" RIGHT/DOWN PANEL
 (char *)"DSPADCGAIN",                            // FOR EACH CHANNEL, BOARD AND CRATE
 (char *)"DSPDACOFFSET",
 (char *)"DSPDACGAIN",                            // ----------------------------------------------6END------
 (char *)"SGNWHOAMI",                             // FIXED FOR SIGGEN DATA: 1 VALUE/LINE          47-50 7BEG------
 (char *)"SGNSOFTWARERELEASE",                    // NEXT 4 LINES ARE
 (char *)"SGNLOGICRELEASE",                       // SHOWN ON "CRATE BCU2" MIDDLE/DOWN PANEL
 (char *)"SGNSERIALNUMBER",                       // ----------------------------------------------7END------
 (char *)"SGNSTRATIXTEMP",                        // SIGGEN DATA: 1 VALUE/CRATE = 6               51-53 8BEG------
 (char *)"SGNPOWERTEMP",                          // NEXT 3 LINES ARE SHOWN  FOR EACH CRATE
 (char *)"SGNDSPSTEMP",                           // ON "CRATE BCU1" RIGHT/DOWN PANEL -------------8END------
 (char *)"DSPSTRATIXTEMP",                        // DSP DATA: 1 VAL/BOARD(14)/CRATE(6) = 84      54-57 9BEG------
 (char *)"DSPPOWERTEMP",                          // NEXT 4 LINES ARE SHOWN  FOR EACH CRATE AND BOARD
 (char *)"DSPDSPSTEMP",                           // ON "CRATE DSP" LEFT/MIDDLE PANEL
 (char *)"DSPDRIVERTEMP",                         // ----------------------------------------------9END------
 (char *)"DSPCOILCURRENT",                        // 8 VAL FOR EACH CRATE & BOARD ARE SHOWN ON "CRATE DSP" MIDLE/UPPER PANEL   58
 (char *)"SWITCHSTRATIXTEMP",                     //FIXED 1 VAL/LINE                             59-60  11BEG-----
 (char *)"SWITCHPOWERTEMP",                       // 2 LINES  ARE SHOWN ON "CRATE BCU1"  MIDDLE/DOWN PANEL WHEN "SWITCH BCU" IS CHOSEN
 (char *)"BCUSTRATIXTEMP",                        //FIXED 1 VAL/CRATE                            61-62  12BEG-----
 (char *)"BCUPOWERTEMP",                          // 2 LINES  ARE SHOWN ON "CRATE BCU1"  MIDDLE/DOWN PANEL  WHEN ANY "CRATE BCU" IS CHOSEN
 (char *)"BCUVOLTAGEVCCL",                        // BCU DATA: 1 VALUE/CRATE = 6                 63-72  13BEG------
 (char *)"BCUVOLTAGEVCCA",                        // NEXT 10 LINES ARE SHOWN  FOR EACH CRATE
 (char *)"BCUVOLTAGEVSSA",                        // ON "CRATE BCU1" RIGHT/UPPER PANEL
 (char *)"BCUVOLTAGEVCCP",
 (char *)"BCUVOLTAGEVSSP",
 (char *)"BCUCURRENTVCCL",
 (char *)"BCUCURRENTVCCA",
 (char *)"BCUCURRENTVSSA",
 (char *)"BCUCURRENTVCCP",
 (char *)"BCUCURRENTVSSP",                        // ----------------------------------------------13END-----
 (char *)"BCUCOOLERIN0TEMP",                      // BCU DATA: 1 VALUE/CRATE = 6                73-78   14BEG-----
 (char *)"BCUCOOLERIN1TEMP",                      // NEXT 6 LINES ARE SHOWN  FOR EACH CRATE
 (char *)"BCUCOOLEROUT0TEMP",                     // ON "CRATE BCU1" MIDDLE/UPPER PANEL
 (char *)"BCUCOOLEROUT1TEMP",
 (char *)"BCUCOOLERPRESSURE",
 (char *)"BCUHUMIDITY",                           // ----------------------------------------------14END-----
 (char *)"TOTALCURRENTVCCP",                      // BCU DATA: 1 VALUE                          79-81   15BEG-----
 (char *)"TOTALCURRENTVSSP",                      // 3 LINES ARE SHOWN 
 (char *)"TOTALCURRENTVP",                        // ON "CRATE BCU2" RIGHT/UPPER PANEL             15END-----
 (char *)"BCUDIGITALIODRIVERENABLED",             // BCU DATA: 1 VALUE/CRATE                    82-91   16BEG-----
 (char *)"BCUDIGITALIOCRATEISMASTER",             // NEXT 10 LINES ARE SHOWN  FOR EACH CRATE
 (char *)"BCUDIGITALIOBUSSYSFAULT",               // ON "CRATE BCU2" MIDDLE/UPPER PANEL    
 (char *)"BCUDIGITALIOVCCFAULT",
 (char *)"BCUDIGITALIOACPOWERFAULT0",
 (char *)"BCUDIGITALIOACPOWERFAULT1",
 (char *)"BCUDIGITALIOACPOWERFAULT2",
 (char *)"BCUDIGITALIOICDISCONNECTED",
 (char *)"BCUDIGITALIOOVERCURRENT",
 (char *)"BCUDIGITALIOCRATEID",                   // ----------------------------------------------16END-----
 (char *)"BCURESETSTATUSFPGARESET",               // BCU DATA: 1 VALUE/CRATE                    92-101   17BEG-----
 (char *)"BCURESETSTATUSBUSRESET",                // NEXT 10 LINES ARE SHOWN  FOR EACH CRATE
 (char *)"BCURESETSTATUSDSPRESET",                // ON "CRATE BCU2" LEFT/UPPER PANEL
 (char *)"BCURESETSTATUSFLASHRESET",
 (char *)"BCURESETSTATUSPCIRESET",
 (char *)"BCURESETSTATUSBUSDRIVERENABLE",
 (char *)"BCURESETSTATUSBUSDRIVERENABLESTATUS",
 (char *)"BCURESETSTATUSBUSPOWERFAULTCTRL",
 (char *)"BCURESETSTATUSBUSPOWERFAULT",
 (char *)"BCURESETSTATUSSYSTEMWATCHDOG",          // ----------------------------------------------17END-----
 (char *)"DSPDRIVERSTATUSFPGARESET",              // DSP DATA: 1 VALUE/BOARD(14)/CRATE(6) =84    102-111  18BEG-----
 (char *)"DSPDRIVERSTATUSDSP0RESET",              // NEXT 10 LINES ARE SHOWN  FOR EACH CRATE & BOARD
 (char *)"DSPDRIVERSTATUSDSP1RESET",              // ON "CRATE DSP" RIGHT/UPPER PANEL
 (char *)"DSPDRIVERSTATUSFLASHRESET",
 (char *)"DSPDRIVERSTATUSBUSPOWERFAULTCTRL",
 (char *)"DSPDRIVERSTATUSBUSPOWERFAULT",
 (char *)"DSPDRIVERSTATUSBUSDRIVERENABLE",
 (char *)"DSPDRIVERSTATUSENABLEDSPWATCHDOG",
 (char *)"DSPDRIVERSTATUSDSP0WATCHDOGEXPIRED",
 (char *)"DSPDRIVERSTATUSDSP1WATCHDOGEXPIRED",    // ----------------------------------------------18END-----
 (char *)"DSPDRIVERSTATUSDRIVERENABLE",           // 8 CHANNELS FOR EACH CRATE & BOARD ARE SHOWN ON "CRATE DSP" MIDDLE/DOWN PANEL 112
 (char *)"EXTERNALTEMPERATURE",                   // FIXED DATA: 1 VALUE/LINE                113-121      20BEG-----
 (char *)"FLUXRATEIN",                            // NEXT 9 LINES ARE SHOWN 
 (char *)"WATERMAININLET",                        // ON "SYSTEM SUMMARY" RIGHT PANEL
 (char *)"WATERMAINOUTLET",
 (char *)"WATERCOLDPLATEINLET",
 (char *)"WATERCOLDPLATEOUTLET",
 (char *)"EXTERNALHUMIDITY",
 (char *)"DEWPOINT",
 (char *)"CHECKDEWPOINT",
 (char *)"HUBTEMP",
 (char *)"CPTEMP",
 (char *)"RBODYTEMP",
 (char *)"INNERSTRTEMP" 
// "PBPLANETEMP",
};
char *AdamFamilyName[]={
 (char *)"ADAMMAINPOWER",
 (char *)"ADAMWATCHDOGEXP",
 (char *)"ADAMTSSDISABLE",
 (char *)"ADAMDRIVERENABLE",
 (char *)"ADAMFPGACLEARN",
 (char *)"ADAMBOOTSELECTN",
 (char *)"ADAMSYSRESETN",
 (char *)"ADAMTSSPOWERFAULTN0",
 (char *)"ADAMTSSPOWERFAULTN1",
 (char *)"ADAMTSSFAULTN",
 (char *)"ADAMTCSSYSFAULT",
 (char *)"ADAMTCSPOWERFAULTN0",
 (char *)"ADAMTCSPOWERFAULTN1",
 (char *)"ADAMTCSPOWERFAULTN2"     
     };
char *AdamExpl[5]={(char *)"AdSec MainPower",(char *)"ADAM COMM Watchdog expired",(char *)"TSS disabled", (char *)"Coils drivers enabled", (char *)"FPGA..."};


/*
 *  Constructs a GUILBT672 as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
GUILBT672::GUILBT672( string id, string configFile, QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ), AOApp(id, configFile)
{
    int i;
    char temp[300];

    if ( !name ) 	setName( "GUILBT672" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setSizeGripEnabled( TRUE );
    GUILBT672Layout = new QVBoxLayout( this, 11, 6, "GUILBT672Layout"); 

    tabWidget = new QTabWidget( this, "tabWidget" );
   
    //System Summary
    tabSYS = new QWidget( tabWidget, "tabSYS" );
    tabSYSLayout = new QHBoxLayout( tabSYS, 11, 6, "tabSYSLayout"); 

    //  left panel
    wLeft = new QWidget( tabSYS, "wLeft" );
    vBLeftLayout = new QVBoxLayout( wLeft, 11, 6, "vBLeftLayout");
    qwtPlotSysLeft = new QwtPlot (wLeft,"qwtPlotSysLeft");
    qwtPlotSysLeft->setGeometry(QRect(0,10,300,150));
    qwtPlotSysLeft->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    vBLeftLayout->addWidget(qwtPlotSysLeft);

    frameDiagn = new QFrame( wLeft, "frameDiagn" );
    frameDiagn->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameDiagn->sizePolicy().hasHeightForWidth() ) );
    frameDiagn->setFrameShape( QFrame::TabWidgetPanel );
    frameDiagn->setFrameShadow( QFrame::Raised );
    frameDiagnLayout = new QGridLayout( frameDiagn, 1, 1, 11, 6, "frameDiagnLayout"); 
    //label for panel Header
    labelDiagn = new QLabel( frameDiagn, "labelDiagn" );
    labelDiagn->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelDiagn_font(  labelDiagn->font() );
    labelDiagn_font.setFamily( "Bitstream Charter" );
    labelDiagn_font.setBold( TRUE );
    labelDiagn_font.setItalic( TRUE );
    labelDiagn->setFont( labelDiagn_font ); 
    frameDiagnLayout->addWidget( labelDiagn, 0, 1 );

   int start_pos1=0;
   
    for (i=0; i<2; i++) {     //7
        sprintf (temp,"textLabel%1i",i);
        textLabel[i] = new QLabel( frameDiagn, temp );
        frameDiagnLayout->addWidget( textLabel[i], start_pos1+i+1, 0 );
        sprintf (temp,"DiagnAppDataLab%1i",i);
        DiagnAppDataLab[i] = new QLabel( frameDiagn, temp );
        frameDiagnLayout->addWidget( DiagnAppDataLab[i], i+1, 1 );
        sprintf (temp,"DiagnAppDataLed%1i",i);
        DiagnAppDataLed[i] = new KLed( frameDiagn, temp );
        frameDiagnLayout->addWidget( DiagnAppDataLed[i], i+1, 2 );
        DiagnAppDataLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, DiagnAppDataLed[i]->sizePolicy().hasHeightForWidth() ) );
        DiagnAppDataLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, DiagnAppDataLab[i]->sizePolicy().hasHeightForWidth() ) );
        DiagnAppDataLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        DiagnAppDataLab[i]->setFrameShape( QLabel::LineEditPanel );
        DiagnAppDataLab[i]->setFrameShadow( QLabel::Sunken );
        DiagnAppDataLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    start_pos1=i;

    /*modRB*/ //aggiungere il titolo
    labelDiagnTemp = new QLabel( frameDiagn, "labelDiagnTemp" );
    labelDiagnTemp->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelDiagnTemp_font(  labelDiagnTemp->font() );
    labelDiagnTemp_font.setFamily( "Bitstream Charter" );
    labelDiagnTemp_font.setBold( TRUE );
    labelDiagnTemp_font.setItalic( TRUE );
    labelDiagnTemp->setFont( labelDiagnTemp_font ); 
    frameDiagnLayout->addWidget( labelDiagnTemp, start_pos1+1, 1 );
    start_pos1=start_pos1+1;
    for (i=0; i<5; i++) {     //7
        sprintf (temp,"textLabelTemp%1i",i);
        textLabelTemp[i] = new QLabel( frameDiagn, temp );
        frameDiagnLayout->addWidget( textLabelTemp[i], start_pos1+i+1, 0 );
        sprintf (temp,"GeneralValTemp%1i",i);
        GeneralValTemp[i] = new QLabel( frameDiagn, temp );
        frameDiagnLayout->addWidget( GeneralValTemp[i], start_pos1+i+1, 1 );
        sprintf (temp,"GeneralValTempLed%1i",i);
        GeneralValTempLed[i] = new KLed( frameDiagn, temp );
        frameDiagnLayout->addWidget( GeneralValTempLed[i], start_pos1+i+1, 2 );
        GeneralValTempLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, GeneralValTempLed[i]->sizePolicy().hasHeightForWidth() ) );
        GeneralValTemp[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, GeneralValTemp[i]->sizePolicy().hasHeightForWidth() ) );
        GeneralValTemp[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        GeneralValTemp[i]->setFrameShape( QLabel::LineEditPanel );
        GeneralValTemp[i]->setFrameShadow( QLabel::Sunken );
        GeneralValTemp[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    /*moRB*/
    
    vBLeftLayout->addWidget(frameDiagn);
    tabSYSLayout->addWidget( wLeft );

    //   right panel
    wRight = new QWidget( tabSYS, "wRight" );
    vBRightLayout = new QVBoxLayout( wRight, 11, 6, "vBRightLayout");

    qwtPlotSysRight = new QwtPlot (wRight,"qwtPlotSysRight");
    qwtPlotSysRight->setGeometry(QRect(0,10,300,150));
    qwtPlotSysRight->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    vBRightLayout->addWidget(qwtPlotSysRight);

    frameTemper = new QFrame( wRight, "frameTemper" );
    frameTemper->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameTemper->sizePolicy().hasHeightForWidth() ) );
    frameTemper->setFrameShape( QFrame::TabWidgetPanel );
    frameTemper->setFrameShadow( QFrame::Raised );
    frameTemperLayout = new QGridLayout( frameTemper, 1, 1, 11, 6, "frameTemperLayout"); 
    // label for panel Header
    labelTemper = new QLabel( frameTemper, "labelTemper" );
    labelTemper->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelTemper_font(  labelTemper->font() );
    labelTemper_font.setFamily( "Bitstream Charter" );
    labelTemper_font.setBold( TRUE );
    labelTemper_font.setItalic( TRUE );
    labelTemper->setFont( labelTemper_font ); 
    frameTemperLayout->addWidget( labelTemper, 0, 1 );
    for (i=0; i<9; i++) {
        sprintf (temp,"textLabelE%1i",i);
        textLabelE[i] = new QLabel( frameTemper, temp );
        frameTemperLayout->addWidget( textLabelE[i], i+1, 0 );
        sprintf (temp,"ExternalLab%1i",i);
        ExternalLab[i] = new QLabel( frameTemper, temp );
        ExternalLab[i]-> installEventFilter ( this );
        frameTemperLayout->addWidget( ExternalLab[i], i+1, 1 );
        sprintf (temp,"ExternalLed%1i",i);
        ExternalLed[i] = new KLed( frameTemper, temp );
        frameTemperLayout->addWidget( ExternalLed[i], i+1, 2 );
        sprintf (temp,"ExternalBut%1i",i);
        ExternalBut[i] = new QPushButton( frameTemper, temp );
        ExternalBut[i]->setMaximumWidth(35);
        ExternalBut[i]->setAutoDefault( FALSE );
        ExternalBut[i]-> installEventFilter ( this );
        frameTemperLayout->addWidget( ExternalBut[i], i+1, 3 );
        ExternalLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, ExternalLed[i]->sizePolicy().hasHeightForWidth() ) );
        ExternalLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, ExternalLab[i]->sizePolicy().hasHeightForWidth() ) );
        ExternalLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        ExternalLab[i]->setFrameShape( QLabel::LineEditPanel );
        ExternalLab[i]->setFrameShadow( QLabel::Sunken );
        ExternalLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    vBRightLayout->addWidget(frameTemper);
    tabSYSLayout->addWidget(wRight );
    tabWidget->insertTab( tabSYS, QString::fromLatin1("") );

    //Crate BCU1
    unnamed = new QWidget( tabWidget, "unnamed" );
    unnamedLayout = new QGridLayout( unnamed, 1, 1, 11, 6, "unnamedLayout"); 
    
    //  left/upper panel
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
    for (i=0; i<11; i++) {
        sprintf (temp,"textLabelBCU1%2i",i);
        textLabelBCU1[i] = new QLabel( frameNIOS, temp );
        frameNIOSLayout->addWidget( textLabelBCU1[i], i+1, 0 );
        sprintf (temp,"BCU1NiosLab%2i",i);
        BCU1NiosLab[i] = new QLabel( frameNIOS, temp );
        frameNIOSLayout->addWidget( BCU1NiosLab[i], i+1, 1 );
        sprintf (temp,"BCU1NiosLedb%2i",i);
        BCU1NiosLed[i] = new KLed( frameNIOS, temp );
        frameNIOSLayout->addWidget( BCU1NiosLed[i], i+1, 2 );
        BCU1NiosLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, BCU1NiosLed[i]->sizePolicy().hasHeightForWidth() ) );
        BCU1NiosLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU1NiosLab[i]->sizePolicy().hasHeightForWidth() ) );
        BCU1NiosLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU1NiosLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU1NiosLab[i]->setFrameShadow( QLabel::Sunken );
        BCU1NiosLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    unnamedLayout->addWidget( frameNIOS, 0, 0 );
    // left/down panel
    wBCU1ld = new QWidget( unnamed, "wBCU1ld" );
    vBCU1ldLayout = new QVBoxLayout( wBCU1ld, 11, 6, "vBCU1ldLayout");

    qwtBCU1ld = new QwtPlot (wBCU1ld,"qwtBCU1ld");
    qwtBCU1ld->setGeometry(QRect(0,10,300,200));
    qwtBCU1ld->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    vBCU1ldLayout->addWidget(qwtBCU1ld);
    
    crateNumber = new QFrame( wBCU1ld, "crateNumber" );
    crateNumber->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, QSizePolicy::Minimum, 0, 0, crateNumber->sizePolicy().hasHeightForWidth() ) );
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

    for (i=0; i<CR1NUM; i++) {
        sprintf (temp,"BCU1RBut%1i",i);
        BCU1RBut[i] = new QRadioButton( crateBCU1_gr, temp );
        BCU1RBut[i]->setMaximumWidth(35);
        BCU1RBut[i]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
        BCU1RBut[i]->setChecked( TRUE );
        crateBCU1_grLayout->addWidget( BCU1RBut[i] );
    }
    crateNumberLayout->addWidget( crateBCU1_gr, 0, 0 );
    BCU1RBut[0]->setChecked( TRUE );
    vBCU1ldLayout->addWidget(crateNumber);
    unnamedLayout->addWidget( wBCU1ld, 1, 0 );
    //  middle/upper panel
    wBCU1mu = new QWidget( unnamed, "wBCU1mu" );
    vBCU1muLayout = new QVBoxLayout( wBCU1mu, 11, 6, "vBCU1muLayout");

    //qwtBCU1mu = new QwtPlot (wBCU1mu,"qwtBCU1mu");
    //qwtBCU1mu->setGeometry(QRect(0,10,300,100));
    //qwtBCU1mu->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    //vBCU1muLayout->addWidget(qwtBCU1mu);

    frameEnvirBCU1 = new QFrame( wBCU1mu, "frameEnvirBCU1" );
    frameEnvirBCU1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, QSizePolicy::Minimum, 0, 0, frameEnvirBCU1->sizePolicy().hasHeightForWidth() ) );
    frameEnvirBCU1->setFrameShape( QFrame::TabWidgetPanel );
    frameEnvirBCU1->setFrameShadow( QFrame::Raised );
    frameEnvirBCU1Layout = new QGridLayout( frameEnvirBCU1, 1, 1, 11, 6, "frameEnvirBCU1Layout"); 
    // label for panel Header
    labelEnvBCU1 = new QLabel( frameEnvirBCU1, "labelEnvBCU1" );
    labelEnvBCU1->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelEnvBCU1_font(  labelEnvBCU1->font() );
    labelEnvBCU1_font.setFamily( "Bitstream Charter" );
    labelEnvBCU1_font.setBold( TRUE );
    labelEnvBCU1_font.setItalic( TRUE );
    labelEnvBCU1->setFont( labelEnvBCU1_font ); 
    frameEnvirBCU1Layout->addWidget( labelEnvBCU1, 0, 1 );
    for (i=0; i<6; i++) {
        sprintf (temp,"BCU1EnvText%1i",i);
        BCU1EnvText[i] = new QLabel( frameEnvirBCU1, temp );
        frameEnvirBCU1Layout->addWidget( BCU1EnvText[i], i+1, 0 );
        sprintf (temp,"BCU1EnvLab%1i",i);
        BCU1EnvLab[i] = new QLabel( frameEnvirBCU1, temp );
        BCU1EnvLab[i]-> installEventFilter ( this );
        frameEnvirBCU1Layout->addWidget( BCU1EnvLab[i], i+1, 1 );
        sprintf (temp,"BCU1EnvLed%1i",i);
        BCU1EnvLed[i] = new KLed( frameEnvirBCU1, temp );
        frameEnvirBCU1Layout->addWidget( BCU1EnvLed[i], i+1, 2 );
        sprintf (temp,"BCU1EnvBut%1i",i);
        BCU1EnvBut[i] = new QPushButton( frameEnvirBCU1, temp );
        BCU1EnvBut[i]->setMaximumWidth(35);
        BCU1EnvBut[i]->setAutoDefault( FALSE );
        BCU1EnvBut[i]-> installEventFilter ( this );
        frameEnvirBCU1Layout->addWidget( BCU1EnvBut[i], i+1, 3 );
        BCU1EnvLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, BCU1EnvLed[i]->sizePolicy().hasHeightForWidth() ) );
        BCU1EnvLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU1EnvLab[i]->sizePolicy().hasHeightForWidth() ) );
        BCU1EnvLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU1EnvLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU1EnvLab[i]->setFrameShadow( QLabel::Sunken );
        BCU1EnvLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    vBCU1muLayout->addWidget(frameEnvirBCU1);
    unnamedLayout->addWidget( wBCU1mu, 0, 1 );
    //  middle/down panel
    wBCU1md = new QWidget( unnamed, "wBCU1md" );
    vBCU1mdLayout = new QVBoxLayout( wBCU1md, 11, 6, "vBCU1mdLayout");

    qwtBCU1md = new QwtPlot (wBCU1md,"qwtBCU1md");
    qwtBCU1md->setGeometry(QRect(0,10,300,200));
    qwtBCU1md->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    vBCU1mdLayout->addWidget(qwtBCU1md);
    frameTempBCU1 = new QFrame( wBCU1md, "frameTempBCU1" );
    frameTempBCU1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, QSizePolicy::Minimum, 0, 0, frameTempBCU1->sizePolicy().hasHeightForWidth() ) );
    frameTempBCU1->setFrameShape( QFrame::TabWidgetPanel );
    frameTempBCU1->setFrameShadow( QFrame::Raised );
    frameTempBCU1Layout = new QGridLayout( frameTempBCU1, 1, 1, 11, 6, "frameTempBCU1Layout"); 
    // label for panel Header
    labelTempBCU1 = new QLabel( frameTempBCU1, "labelTempBCU1" );
    labelTempBCU1->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelTempBCU1_font(  labelTempBCU1->font() );
    labelTempBCU1_font.setFamily( "Bitstream Charter" );
    labelTempBCU1_font.setBold( TRUE );
    labelTempBCU1_font.setItalic( TRUE );
    labelTempBCU1->setFont( labelTempBCU1_font ); 
    frameTempBCU1Layout->addWidget( labelTempBCU1, 0, 1 );
    for (i=0; i<2; i++) {
        sprintf (temp,"BCU1TempText%1i",i);
        BCU1TempText[i] = new QLabel( frameTempBCU1, temp );
        frameTempBCU1Layout->addWidget( BCU1TempText[i], i+1, 0 );
        sprintf (temp,"BCU1TempLab%1i",i);
        BCU1TempLab[i] = new QLabel( frameTempBCU1, temp );
        BCU1TempLab[i]-> installEventFilter ( this );
        frameTempBCU1Layout->addWidget( BCU1TempLab[i], i+1, 1 );
        sprintf (temp,"BCU1TempLed%1i",i);
        BCU1TempLed[i] = new KLed( frameTempBCU1, temp );
        frameTempBCU1Layout->addWidget( BCU1TempLed[i], i+1, 2 );
        sprintf (temp,"BCU1TempBut%1i",i);
        BCU1TempBut[i] = new QPushButton( frameTempBCU1, temp );
        BCU1TempBut[i]->setMaximumWidth(35);
        BCU1TempBut[i]->setAutoDefault( FALSE );
        BCU1TempBut[i]-> installEventFilter ( this );
        frameTempBCU1Layout->addWidget( BCU1TempBut[i], i+1, 3 );
        BCU1TempLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, BCU1TempLed[i]->sizePolicy().hasHeightForWidth() ) );
        BCU1TempLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU1TempLab[i]->sizePolicy().hasHeightForWidth() ) );
        BCU1TempLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU1TempLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU1TempLab[i]->setFrameShadow( QLabel::Sunken );
        BCU1TempLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    vBCU1mdLayout->addWidget(frameTempBCU1);
    unnamedLayout->addWidget( wBCU1md, 1, 1 );
    //  right/upper panel
    frameVoltBCU1 = new QFrame( unnamed, "frameVoltBCU1" );
    frameVoltBCU1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameVoltBCU1->sizePolicy().hasHeightForWidth() ) );
    frameVoltBCU1->setFrameShape( QFrame::TabWidgetPanel );
    frameVoltBCU1->setFrameShadow( QFrame::Raised );
    frameVoltBCU1Layout = new QGridLayout( frameVoltBCU1, 1, 1, 11, 6, "frameVoltBCU1Layout"); 
    // label for panel Header
    labelVoltBCU1 = new QLabel( frameVoltBCU1, "labelVoltBCU1" );
    labelVoltBCU1->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelVoltBCU1_font(  labelVoltBCU1->font() );
    labelVoltBCU1_font.setFamily( "Bitstream Charter" );
    labelVoltBCU1_font.setBold( TRUE );
    labelVoltBCU1_font.setItalic( TRUE );
    labelVoltBCU1->setFont( labelVoltBCU1_font ); 
    frameVoltBCU1Layout->addWidget( labelVoltBCU1, 0, 1 );
    for (i=0; i<10; i++) {
        sprintf (temp,"BCU1VolText%1i",i);
        BCU1VolText[i] = new QLabel( frameVoltBCU1, temp );
        frameVoltBCU1Layout->addWidget( BCU1VolText[i], i+1, 0 );
        sprintf (temp,"BCU1VolLab%1i",i);
        BCU1VolLab[i] = new QLabel( frameVoltBCU1, temp );
        BCU1VolLab[i]-> installEventFilter ( this );
        frameVoltBCU1Layout->addWidget( BCU1VolLab[i], i+1, 1 );
        sprintf (temp,"BCU1VolLed%1i",i);
        BCU1VolLed[i] = new KLed( frameVoltBCU1, temp );
        frameVoltBCU1Layout->addWidget( BCU1VolLed[i], i+1, 2 );
        sprintf (temp,"BCU1VolBut%1i",i);
        BCU1VolBut[i] = new QPushButton( frameVoltBCU1, temp );
        BCU1VolBut[i]->setMaximumWidth(35);
        BCU1VolBut[i]->setAutoDefault( FALSE );
        BCU1VolBut[i]-> installEventFilter ( this );
        frameVoltBCU1Layout->addWidget( BCU1VolBut[i], i+1, 3 );
        BCU1VolLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, BCU1VolLed[i]->sizePolicy().hasHeightForWidth() ) );
        BCU1VolLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU1VolLab[i]->sizePolicy().hasHeightForWidth() ) );
        BCU1VolLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU1VolLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU1VolLab[i]->setFrameShadow( QLabel::Sunken );
        BCU1VolLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    unnamedLayout->addWidget( frameVoltBCU1, 0, 2 );
    //  right/down panel
    wBCU1rd = new QWidget( unnamed, "wBCU1rd" );
    vBCU1rdLayout = new QVBoxLayout( wBCU1rd, 11, 6, "vBCU1rdLayout");

    qwtBCU1rd = new QwtPlot (wBCU1rd,"qwtBCU1rd");
    qwtBCU1rd->setGeometry(QRect(0,10,300,100));
    qwtBCU1rd->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    vBCU1rdLayout->addWidget(qwtBCU1rd);
    frameTempSIGGEN = new QFrame( wBCU1rd, "frameTempSIGGEN" );
    frameTempSIGGEN->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, QSizePolicy::Minimum, 0, 0, frameTempSIGGEN->sizePolicy().hasHeightForWidth() ) );
    frameTempSIGGEN->setFrameShape( QFrame::TabWidgetPanel );
    frameTempSIGGEN->setFrameShadow( QFrame::Raised );
    frameTempSIGGENLayout = new QGridLayout( frameTempSIGGEN, 1, 1, 11, 6, "frameTempSIGGENLayout"); 
    // label for panel Header
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
    for (i=0; i<3; i++) {
        sprintf (temp,"BCU1SGNTempText%1i",i);
        BCU1SGNTempText[i] = new QLabel( frameTempSIGGEN, temp );
        frameTempSIGGENLayout->addWidget( BCU1SGNTempText[i], i+1, 0 );
        sprintf (temp,"BCU1SGNTempLab%1i",i);
        BCU1SGNTempLab[i] = new QLabel( frameTempSIGGEN, temp );
        BCU1SGNTempLab[i]-> installEventFilter ( this );
        frameTempSIGGENLayout->addWidget( BCU1SGNTempLab[i], i+1, 1 );
        sprintf (temp,"BCU1SGNTempLed%1i",i);
        BCU1SGNTempLed[i] = new KLed( frameTempSIGGEN, temp );
        frameTempSIGGENLayout->addWidget( BCU1SGNTempLed[i], i+1, 2 );
        sprintf (temp,"BCU1SGNTempBut%1i",i);
        BCU1SGNTempBut[i] = new QPushButton( frameTempSIGGEN, temp );
        BCU1SGNTempBut[i]->setMaximumWidth(35);
        BCU1SGNTempBut[i]->setAutoDefault( FALSE );
        BCU1SGNTempBut[i]-> installEventFilter ( this );
        frameTempSIGGENLayout->addWidget( BCU1SGNTempBut[i], i+1, 3 );
        BCU1SGNTempLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, BCU1SGNTempLed[i]->sizePolicy().hasHeightForWidth() ) );
        BCU1SGNTempLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU1SGNTempLab[i]->sizePolicy().hasHeightForWidth() ) );
        BCU1SGNTempLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU1SGNTempLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU1SGNTempLab[i]->setFrameShadow( QLabel::Sunken );
        BCU1SGNTempLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    vBCU1rdLayout->addWidget(frameTempSIGGEN);
    unnamedLayout->addWidget( wBCU1rd, 1, 2 );
    tabWidget->insertTab( unnamed, QString::fromLatin1("") );

    //Crate BCU2
    TabPage = new QWidget( tabWidget, "TabPage" );
    TabPageLayout = new QGridLayout( TabPage, 1, 1, 11, 6, "TabPageLayout"); 

    //  left/upper panel
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
    for (i=0; i<10; i++) {
        sprintf (temp,"BCU2ResetText%1i",i);
        BCU2ResetText[i] = new QLabel( frameResets, temp );
        frameResetsLayout->addWidget( BCU2ResetText[i], i+1, 0 );
        sprintf (temp,"BCU2ResetLed%1i",i);
        BCU2ResetLed[i] = new KLed( frameResets, temp );
        BCU2ResetLed[i]->setColor( QColor( 255, 255, 255 ) );
        BCU2ResetLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, BCU2ResetLed[i]->sizePolicy().hasHeightForWidth() ) );
        frameResetsLayout->addWidget( BCU2ResetLed[i], i+1, 1 );
    }
    TabPageLayout->addWidget( frameResets, 0, 0 );

    //  left/down panel
    wBCU2ld = new QWidget( TabPage, "wBCU2ld" );
    vBCU2ldLayout = new QVBoxLayout( wBCU2ld, 11, 6, "vBCU2ldLayout");

    qwtBCU2ld = new QwtPlot (wBCU2ld,"qwtBCU2ld");
    qwtBCU2ld->setGeometry(QRect(0,10,300,200));
    qwtBCU2ld->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    vBCU2ldLayout->addWidget(qwtBCU2ld);
    crateNumber2 = new QFrame( wBCU2ld, "crateNumber2" );
    crateNumber2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, QSizePolicy::Minimum, 0, 0, crateNumber2->sizePolicy().hasHeightForWidth() ) );
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
    for (i=0; i<CRNUM; i++) {
        sprintf (temp,"BCU2RBut%1i",i);
        BCU2RBut[i] = new QRadioButton( crateBCU2_gr, temp );
        BCU2RBut[i]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
        BCU2RBut[i]->setChecked( TRUE );
        crateBCU2_grLayout->addWidget( BCU2RBut[i] );
    }
    crateNumber2Layout->addWidget( crateBCU2_gr, 0, 0 );
    vBCU2ldLayout->addWidget(crateNumber2);
    TabPageLayout->addWidget( wBCU2ld, 1, 0 );
    //TabPageLayout->addWidget( crateNumber2, 1, 0 );
    BCU2RBut[0]->setChecked( TRUE );

    // middle/upper panel
    frameDigitalIO = new QFrame( TabPage, "frameDigitalIO" );
    frameDigitalIO->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameDigitalIO->sizePolicy().hasHeightForWidth() ) );
    frameDigitalIO->setFrameShape( QFrame::TabWidgetPanel );
    frameDigitalIO->setFrameShadow( QFrame::Raised );
    frameDigitalIOLayout = new QGridLayout( frameDigitalIO, 1, 1, 11, 6, "frameDigitalIOLayout"); 
    // label for panel Header
    labelDigital = new QLabel( frameDigitalIO, "labelDigital" );
    labelDigital->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelDigital_font(  labelDigital->font() );
    labelDigital_font.setFamily( "Bitstream Charter" );
    labelDigital_font.setBold( TRUE );
    labelDigital_font.setItalic( TRUE );
    labelDigital->setFont( labelDigital_font ); 
    frameDigitalIOLayout->addWidget( labelDigital, 0, 0 );
    for (i=0; i<10; i++) {
        sprintf (temp,"BCU2DigText%1i",i);
        BCU2DigText[i] = new QLabel( frameDigitalIO, temp );
        frameDigitalIOLayout->addWidget( BCU2DigText[i], i+1, 0 );
        sprintf (temp,"BCU2DigLed%1i",i);
        BCU2DigLed[i] = new KLed( frameDigitalIO, temp );
        BCU2DigLed[i]->setColor( QColor( 255, 255, 255 ) );
        BCU2DigLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, BCU2DigLed[i]->sizePolicy().hasHeightForWidth() ) );
        frameDigitalIOLayout->addWidget( BCU2DigLed[i], i+1, 1 );
    }
    TabPageLayout->addWidget( frameDigitalIO, 0, 1 );

    // middle/down panel
    frameSIGGENa = new QFrame( TabPage, "frameSIGGENa" );
    frameSIGGENa->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameSIGGENa->sizePolicy().hasHeightForWidth() ) );
    frameSIGGENa->setFrameShape( QFrame::TabWidgetPanel );
    frameSIGGENa->setFrameShadow( QFrame::Raised );
    frameSIGGENaLayout = new QGridLayout( frameSIGGENa, 1, 1, 11, 6, "frameSIGGENaLayout"); 
    // label for panel Header
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
    for (i=0; i<4; i++) {
        sprintf (temp,"BCU2SGNText%1i",i);
        BCU2SGNText[i] = new QLabel( frameSIGGENa, temp );
        frameSIGGENaLayout->addWidget( BCU2SGNText[i], i+1, 0 );
        sprintf (temp,"BCU2SGNLab%1i",i);
        BCU2SGNLab[i] = new QLabel( frameSIGGENa, temp );
        frameSIGGENaLayout->addWidget( BCU2SGNLab[i], i+1, 1 );
        sprintf (temp,"BCU2SGNLed%1i",i);
        BCU2SGNLed[i] = new KLed( frameSIGGENa, temp );
        frameSIGGENaLayout->addWidget( BCU2SGNLed[i], i+1, 2 );
        BCU2SGNLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, BCU2SGNLed[i]->sizePolicy().hasHeightForWidth() ) );
        BCU2SGNLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU2SGNLab[i]->sizePolicy().hasHeightForWidth() ) );
        BCU2SGNLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU2SGNLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU2SGNLab[i]->setFrameShadow( QLabel::Sunken );
        BCU2SGNLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    TabPageLayout->addWidget( frameSIGGENa, 1, 1 );

    // right/upper panel
    wBCU2ru = new QWidget( TabPage, "wBCU2ru" );
    vBCU2ruLayout = new QVBoxLayout( wBCU2ru, 11, 6, "vBCU2ruLayout");

    qwtBCU2ru = new QwtPlot (wBCU2ru,"qwtBCU2ru");
    qwtBCU2ru->setGeometry(QRect(0,10,300,200));
    qwtBCU2ru->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    vBCU2ruLayout->addWidget(qwtBCU2ru);
    frameMasterBCU2 = new QFrame( wBCU2ru, "frameMasterBCU2" );
    frameMasterBCU2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, QSizePolicy::Minimum, 0, 0, frameMasterBCU2->sizePolicy().hasHeightForWidth() ) );
    frameMasterBCU2->setFrameShape( QFrame::TabWidgetPanel );
    frameMasterBCU2->setFrameShadow( QFrame::Raised );
    frameMasterBCU2Layout = new QGridLayout( frameMasterBCU2, 1, 1, 11, 6, "frameMasterBCU2Layout"); 
    // label for panel Header
    labelEnvBCU2 = new QLabel( frameMasterBCU2, "labelEnvBCU2" );
    labelEnvBCU2->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelEnvBCU2_font(  labelEnvBCU2->font() );
    labelEnvBCU2_font.setFamily( "Bitstream Charter" );
    labelEnvBCU2_font.setBold( TRUE );
    labelEnvBCU2_font.setItalic( TRUE );
    labelEnvBCU2->setFont( labelEnvBCU2_font ); 
    frameMasterBCU2Layout->addWidget( labelEnvBCU2, 0, 1 );
    for (i=0; i<3; i++) {
        sprintf (temp,"BCU2MastText%1i",i);
        BCU2MastText[i] = new QLabel( frameMasterBCU2, temp );
        frameMasterBCU2Layout->addWidget( BCU2MastText[i], i+1, 0 );
        sprintf (temp,"BCU2MastLab%1i",i);
        BCU2MastLab[i] = new QLabel( frameMasterBCU2, temp );
        BCU2MastLab[i]-> installEventFilter ( this );
        frameMasterBCU2Layout->addWidget( BCU2MastLab[i], i+1, 1 );
        sprintf (temp,"BCU2MastLed%1i",i);
        BCU2MastLed[i] = new KLed( frameMasterBCU2, temp );
        frameMasterBCU2Layout->addWidget( BCU2MastLed[i], i+1, 2 );
        sprintf (temp,"BCU2MastBut%1i",i);
        BCU2MastBut[i] = new QPushButton( frameMasterBCU2, temp );
        BCU2MastBut[i]->setMaximumWidth(35);
        BCU2MastBut[i]->setAutoDefault( FALSE );
        BCU2MastBut[i]-> installEventFilter ( this );
        frameMasterBCU2Layout->addWidget( BCU2MastBut[i], i+1, 3 );
        BCU2MastLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, BCU2MastLed[i]->sizePolicy().hasHeightForWidth() ) );
        BCU2MastLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU2MastLab[i]->sizePolicy().hasHeightForWidth() ) );
        BCU2MastLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU2MastLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU2MastLab[i]->setFrameShadow( QLabel::Sunken );
        BCU2MastLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    vBCU2ruLayout->addWidget(frameMasterBCU2);
    TabPageLayout->addWidget( wBCU2ru, 0, 2 );
    //TabPageLayout->addWidget( frameMasterBCU2, 0, 2 );

    //  right/down panel
    frameBCU2c = new QFrame( TabPage, "frameBCU2c" );
    frameBCU2c->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameBCU2c->sizePolicy().hasHeightForWidth() ) );
    frameBCU2c->setFrameShape( QFrame::TabWidgetPanel );
    frameBCU2c->setFrameShadow( QFrame::Raised );
    frameBCU2cLayout = new QGridLayout( frameBCU2c, 1, 1, 11, 6, "frameBCU2cLayout"); 
    for (i=0; i<4; i++) {
        sprintf (temp,"BCU2TrText%1i",i);
        BCU2TrText[i] = new QLabel( frameBCU2c, temp );
        frameBCU2cLayout->addWidget( BCU2TrText[i], i, 0 );
        sprintf (temp,"BCU2TrLab%1i",i);
        BCU2TrLab[i] = new QLabel( frameBCU2c, temp );
        BCU2TrLab[i]-> installEventFilter ( this );
        frameBCU2cLayout->addWidget( BCU2TrLab[i], i, 1 );
        sprintf (temp,"BCU2TrLed%1i",i);
        BCU2TrLed[i] = new KLed( frameBCU2c, temp );
        frameBCU2cLayout->addWidget( BCU2TrLed[i], i, 2 );
        sprintf (temp,"BCU2TrBut%1i",i);
        BCU2TrBut[i] = new QPushButton( frameBCU2c, temp );
        BCU2TrBut[i]->setMaximumWidth(35);
        BCU2TrBut[i]->setAutoDefault( FALSE );
        BCU2TrBut[i]-> installEventFilter ( this );
        frameBCU2cLayout->addWidget( BCU2TrBut[i], i, 3 );
        BCU2TrLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, BCU2TrLed[i]->sizePolicy().hasHeightForWidth() ) );
        BCU2TrLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, BCU2TrLab[i]->sizePolicy().hasHeightForWidth() ) );
        BCU2TrLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        BCU2TrLab[i]->setFrameShape( QLabel::LineEditPanel );
        BCU2TrLab[i]->setFrameShadow( QLabel::Sunken );
        BCU2TrLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    TabPageLayout->addWidget( frameBCU2c, 1, 2 );
    tabWidget->insertTab( TabPage, QString::fromLatin1("") );

    //Crate DSP
    TabPage_2 = new QWidget( tabWidget, "TabPage_2" );
    TabPageLayout_2 = new QGridLayout( TabPage_2, 1, 1, 11, 6, "TabPageLayout_2"); 
    //left/upper panel
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
    for (i=0; i<8; i++) {
        sprintf (temp,"DSPNiosText%1i",i);
        DSPNiosText[i] = new QLabel( frameDSPa, temp );
        frameDSPaLayout->addWidget( DSPNiosText[i], i+1, 0 );
        sprintf (temp,"DSPNiosLab%1i",i);
        DSPNiosLab[i] = new QLabel( frameDSPa, temp );
        frameDSPaLayout->addWidget( DSPNiosLab[i], i+1, 1 );
        sprintf (temp,"DSPNiosLed%1i",i);
        DSPNiosLed[i] = new KLed( frameDSPa, temp );
        frameDSPaLayout->addWidget( DSPNiosLed[i], i+1, 2 );
        DSPNiosLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, DSPNiosLed[i]->sizePolicy().hasHeightForWidth() ) );
        DSPNiosLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, DSPNiosLab[i]->sizePolicy().hasHeightForWidth() ) );
        DSPNiosLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        DSPNiosLab[i]->setFrameShape( QLabel::LineEditPanel );
        DSPNiosLab[i]->setFrameShadow( QLabel::Sunken );
        DSPNiosLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    TabPageLayout_2->addWidget( frameDSPa, 0, 0 );
    //left/down panel
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
    for (i=0; i<4; i++) {
        sprintf (temp,"DSPTempText%1i",i);
        DSPTempText[i] = new QLabel( frameTempDSP, temp );
        frameTempDSPLayout->addWidget(DSPTempText[i] , i+1, 0 );
        sprintf (temp,"DSPTempLab%1i",i);
        DSPTempLab[i] = new QLabel( frameTempDSP, temp );
        DSPTempLab[i]-> installEventFilter ( this );
        frameTempDSPLayout->addWidget( DSPTempLab[i], i+1, 1 );
        sprintf (temp,"DSPTempLed%1i",i);
        DSPTempLed[i] = new KLed( frameTempDSP, temp );
        frameTempDSPLayout->addWidget( DSPTempLed[i], i+1, 2 );
        sprintf (temp,"DSPTempBut%1i",i);
        DSPTempBut[i] = new QPushButton( frameTempDSP, temp );
        DSPTempBut[i]->setMaximumWidth(35);
        DSPTempBut[i]->setAutoDefault( FALSE );
        DSPTempBut[i]-> installEventFilter ( this );
        frameTempDSPLayout->addWidget( DSPTempBut[i], i+1, 3 );
        DSPTempLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, DSPTempLed[i]->sizePolicy().hasHeightForWidth() ) );
        DSPTempLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, DSPTempLab[i]->sizePolicy().hasHeightForWidth() ) );
        DSPTempLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        DSPTempLab[i]->setFrameShape( QLabel::LineEditPanel );
        DSPTempLab[i]->setFrameShadow( QLabel::Sunken );
        DSPTempLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    QWidget3Layout->addWidget( frameTempDSP );
    //Crate button group
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

    for (i=0; i<CRNUM; i++) {
        sprintf (temp,"DSP1RBut%1i",i);
        DSP1RBut[i] = new QRadioButton( crateSIGGEN_gr, temp );
        DSP1RBut[i]->setPaletteForegroundColor( QColor( 0, 0, 127 ) );
        crateSIGGEN_grLayout->addWidget( DSP1RBut[i] );
    }
    DSP1RBut[0]->setChecked( TRUE );
    frameSIGGENbLayout->addWidget( crateSIGGEN_gr, 0, 0 );
    QWidget3Layout->addWidget( frameSIGGENb );
    TabPageLayout_2->addWidget( QWidget3, 1, 0 );
    //middle/upper panel
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
    for (i=0; i<8; i++) {
        sprintf (temp,"DSPCurText%1i",i);
        DSPCurText[i] = new QLabel( frameCoilCurrDSP, temp );
        frameCoilCurrDSPLayout->addWidget( DSPCurText[i], i+1, 0 );
        sprintf (temp,"DSPCurLab%1i",i);
        DSPCurLab[i] = new QLabel( frameCoilCurrDSP, temp );
        DSPCurLab[i]-> installEventFilter ( this );
        frameCoilCurrDSPLayout->addWidget( DSPCurLab[i], i+1, 1 );
        sprintf (temp,"DSPCurLed%1i",i);
        DSPCurLed[i] = new KLed( frameCoilCurrDSP, temp );
        frameCoilCurrDSPLayout->addWidget( DSPCurLed[i], i+1, 2 );
        sprintf (temp,"DSPCurBut%1i",i);
        DSPCurBut[i] = new QPushButton( frameCoilCurrDSP, temp );
        DSPCurBut[i]->setMaximumWidth(35);
        DSPCurBut[i]->setAutoDefault( FALSE );
        DSPCurBut[i]-> installEventFilter ( this );
        frameCoilCurrDSPLayout->addWidget( DSPCurBut[i], i+1, 3 );
        DSPCurLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, DSPCurLed[i]->sizePolicy().hasHeightForWidth() ) );
        DSPCurLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, DSPCurLab[i]->sizePolicy().hasHeightForWidth() ) );
        DSPCurLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        DSPCurLab[i]->setFrameShape( QLabel::LineEditPanel );
        DSPCurLab[i]->setFrameShadow( QLabel::Sunken );
        DSPCurLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    TabPageLayout_2->addWidget( frameCoilCurrDSP, 0, 1 );
    //middle/down panel
    QWidget4 = new QWidget( TabPage_2, "QWidget4" );
    QWidget4Layout = new QHBoxLayout( QWidget4, 11, 6, "QWidget4Layout"); 
    //board group
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

    for (i=0; i<BONUM; i++) {
        sprintf (temp,"DSP2RBut%2i",i);
        DSP2RBut[i] = new QRadioButton( crateDSPb_6, temp );
        if (i<7) crateDSPb_6Layout->addWidget( DSP2RBut[i], i, 0 );
        if (i>6) crateDSPb_6Layout->addWidget( DSP2RBut[i], i-7, 1 );
    }
    DSP2RBut[0]->setChecked( TRUE );
    frame1Layout->addWidget( crateDSPb_6, 0, 0 );
    QWidget4Layout->addWidget( frame1 );
    //Driver Enabled panel
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
    for (i=0; i<4; i++) {
        sprintf (temp,"DSPDrText%1i",i);
        DSPDrText[i] = new QLabel( unnamed_2, temp );
        unnamedLayout_2->addWidget( DSPDrText[i], i, 0 );
        sprintf (temp,"DSPDrLed%1i",i);
        DSPDrLed[i] = new KLed( unnamed_2, temp );
        DSPDrLed[i]->setColor( QColor( 255, 255, 255 ) );
        DSPDrLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, DSPDrLed[i]->sizePolicy().hasHeightForWidth() ) );
        unnamedLayout_2->addWidget( DSPDrLed[i], i, 1 );
    }
    frameDriverEnabledDSPLayout->addWidget( unnamed_2, 1, 0 );

    unnamed_3 = new QWidget( frameDriverEnabledDSP, "unnamed_3" );
    unnamedLayout_3 = new QGridLayout( unnamed_3, 1, 1, 11, 6, "unnamedLayout_3"); 
    for (i=4; i<CHNUM; i++) {
        sprintf (temp,"DSPDrText%1i",i);
        DSPDrText[i] = new QLabel( unnamed_3, temp );
        unnamedLayout_3->addWidget( DSPDrText[i], i-4, 0 );
        sprintf (temp,"DSPDrLed%1i",i);
        DSPDrLed[i] = new KLed( unnamed_3, temp );
        DSPDrLed[i]->setColor( QColor( 255, 255, 255 ) );
        DSPDrLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, DSPDrLed[i]->sizePolicy().hasHeightForWidth() ) );
        unnamedLayout_3->addWidget( DSPDrLed[i], i-4, 1 );
    }
    frameDriverEnabledDSPLayout->addWidget( unnamed_3, 1, 1 );
    QWidget4Layout->addWidget( frameDriverEnabledDSP );

    TabPageLayout_2->addWidget( QWidget4, 1, 1 );
    //right/upper panel
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
    for (i=0; i<10; i++) {
        sprintf (temp,"DSPDrSText%1i",i);
        DSPDrSText[i] = new QLabel( frameDriverStatusDSP, temp );
        frameDriverStatusDSPLayout->addWidget( DSPDrSText[i], i+1, 0 );
        sprintf (temp,"DSPDrSLed%1i",i);
        DSPDrSLed[i] = new KLed( frameDriverStatusDSP, temp );
        DSPDrSLed[i]->setColor( QColor( 255, 255, 255 ) );
        DSPDrSLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, DSPDrLed[i]->sizePolicy().hasHeightForWidth() ) );
        frameDriverStatusDSPLayout->addWidget( DSPDrSLed[i], i+1, 1 );
    }
    TabPageLayout_2->addWidget( frameDriverStatusDSP, 0, 2 );
    //right/down panel
    frameDSPb = new QFrame( TabPage_2, "frameDSPb" );
    frameDSPb->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameDSPb->sizePolicy().hasHeightForWidth() ) );
    frameDSPb->setFrameShape( QFrame::TabWidgetPanel );
    frameDSPb->setFrameShadow( QFrame::Raised );
    frameDSPbLayout = new QGridLayout( frameDSPb, 1, 1, 11, 6, "frameDSPbLayout"); 
    for (i=0; i<6; i++) {
        sprintf (temp,"DSPadText%1i",i);
        DSPadText[i] = new QLabel( frameDSPb, temp );
        frameDSPbLayout->addWidget( DSPadText[i], i, 0 );
        sprintf (temp,"DSPadLab%1i",i);
        DSPadLab[i] = new QLabel( frameDSPb, temp );
        DSPadLab[i]-> installEventFilter ( this );
        frameDSPbLayout->addWidget( DSPadLab[i], i, 1 );
        sprintf (temp,"DSPadLed%1i",i);
        DSPadLed[i] = new KLed( frameDSPb, temp );
        frameDSPbLayout->addWidget( DSPadLed[i], i, 2 );
        DSPadLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, DSPadLed[i]->sizePolicy().hasHeightForWidth() ) );
        DSPadLab[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, DSPadLab[i]->sizePolicy().hasHeightForWidth() ) );
        DSPadLab[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        DSPadLab[i]->setFrameShape( QLabel::LineEditPanel );
        DSPadLab[i]->setFrameShadow( QLabel::Sunken );
        DSPadLab[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    }
    // channel button group
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

    for (i=0; i<CHNUM; i++) {
        sprintf (temp,"DSP3RBut%1i",i);
        DSP3RBut[i] = new QRadioButton( crateDSPb_6_2, temp );
        if (i<4) crateDSPb_6Layout_2->addWidget( DSP3RBut[i], 0, i );
        if (i>3) crateDSPb_6Layout_2->addWidget( DSP3RBut[i], 1, i-4 );
    }
    DSP3RBut[0]->setChecked( TRUE );
    frameDSPbLayout->addWidget( crateDSPb_6_2, 6, 1 );
    TabPageLayout_2->addWidget( frameDSPb, 1, 2 );

    tabWidget->insertTab( TabPage_2, QString::fromLatin1("") );

    //here starts RB mod
    //Adam tab
    tabAdam = new QWidget( tabWidget, "Widget18" );
    AdamTabLayout = new QHBoxLayout( tabAdam, 11, 6, "AdamTab");
    aLeft = new QWidget( tabAdam, "leftPanel" );
    aBLeftLayout = new QVBoxLayout( aLeft, 11, 6, "vBLeftLayout");
    
    int start_pos=0;
    //int start_pos=i;
    frameAdam = new QFrame( aLeft, "frameAdam" );
    frameAdam->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, frameAdam->sizePolicy().hasHeightForWidth() ) );
    frameAdam->setFrameShape( QFrame::TabWidgetPanel );
    frameAdam->setFrameShadow( QFrame::Raised );
    frameAdamLayout = new QGridLayout( frameAdam, 1, 1, 11, 6, "frameAdamLayout"); 
//mod RB
    labelDiagnAdam = new QLabel( frameAdam, "labelDiagnAdam" );
    labelDiagnAdam->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont labelDiagnAdam_font(  labelDiagnAdam->font() );
    labelDiagnAdam_font.setFamily( "Bitstream Charter" );
    labelDiagnAdam_font.setBold( TRUE );
    labelDiagnAdam_font.setItalic( TRUE );
    labelDiagnAdam->setFont( labelDiagnAdam_font ); 
    frameAdamLayout->addWidget( labelDiagnAdam, start_pos+1, 1 );
    start_pos=start_pos+1;

    for (i=0; i<14;i++){
        sprintf (temp,"textLabelAdam%1i",i);
        textLabelAdam[i] = new QLabel( frameAdam, temp);
        frameAdamLayout->addWidget( textLabelAdam[i], start_pos+i+1, 0 );
        sprintf (temp,"DiagnAdam%1i",i);
        DiagnAdam[i] = new QLabel( frameAdam, temp );//++
        frameAdamLayout->addWidget( DiagnAdam[i], start_pos+i+1, 1 );//++
        sprintf (temp,"DiagnAdam%1i",i);//++
        DiagnAdamLed[i] = new KLed( frameAdam, "Ciao" );
        frameAdamLayout->addWidget( DiagnAdamLed[i], start_pos+i+1, 2 );
        DiagnAdamLed[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)2, (QSizePolicy::SizeType)0, 0, 0, DiagnAdamLed[i]->sizePolicy().hasHeightForWidth() ) );
        DiagnAdam[i]->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)9, (QSizePolicy::SizeType)0, 0, 0, DiagnAdam[i]->sizePolicy().hasHeightForWidth() ) );
        DiagnAdam[i]->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        DiagnAdam[i]->setFrameShape( QLabel::LineEditPanel );
        DiagnAdam[i]->setFrameShadow( QLabel::Sunken );
        DiagnAdam[i]->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignLeft ) );
        DiagnAdam[i]-> installEventFilter ( this );
              
        }
    //here ends RB mod

    aBLeftLayout-> addWidget(frameAdam);
    AdamTabLayout->addWidget( aLeft );
    tabWidget->insertTab( tabAdam, QString::fromLatin1("") );

    //Warnings Tab
    Widget18 = new QWidget( tabWidget, "Widget18" );

    listBox1 = new QListBox( Widget18, "listBox1" );
    listBox1->setGeometry( QRect( -3, 2, 1030, 600 ) );
    listBox1->setSelectionMode( QListBox::NoSelection );
    listBox1->setColumnMode( QListBox::FitToWidth );

    tabWidget->insertTab( Widget18, QString::fromLatin1("") );

    //Alarms Tab
    Widget19 = new QWidget( tabWidget, "Widget19" );

    listBox1_2 = new QListBox( Widget19, "listBox1_2" );
    listBox1_2->setGeometry( QRect( 0, 1, 1030, 600 ) );
    listBox1_2->setSelectionMode( QListBox::NoSelection );
    listBox1_2->setColumnMode( QListBox::FitToWidth );
    tabWidget->insertTab( Widget19, QString::fromLatin1("") );
    GUILBT672Layout->addWidget( tabWidget );

    //Down command buttons panel
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
    Horizontal_Spacing2 = new QSpacerItem( 20, 10, QSizePolicy::Expanding, QSizePolicy::Minimum );//was 20
    Layout1->addItem( Horizontal_Spacing2 );
    //mod RB to  implement a clock
    clockLabel = new QLabel( this, "clockLabel");
    Layout1->addWidget( clockLabel );
    clockLabel->setPaletteForegroundColor( QColor( 170, 0, 0 ) );
    QFont clockLabel_font(  clockLabel->font() );
    clockLabel_font.setFamily( "Bitstream Charter" );
    clockLabel_font.setBold( TRUE );
    clockLabel_font.setItalic( TRUE );
    clockLabel->setFont( clockLabel_font ); 
    Clock = new QLabel( this,  "Clock");
    Layout1->addWidget(Clock);
    Clock->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, Clock->sizePolicy().hasHeightForWidth() ) );
    Clock->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    //Clock->setFrameShape( QLabel::LineEditPanel );
    //Clock->setFrameShadow( QLabel::Sunken );
    //rizontal_Spacing2 = new QSpacerItem( 5,20, QSizePolicy::Expanding, QSizePolicy::Minimum );//was 20
    Layout1->addItem( Horizontal_Spacing2 );    
    
    wrangeLabel = new QLabel( this, "                                      " );
    Layout1->addWidget( wrangeLabel );
    warnLabel = new QLabel( this, "             " );
    Layout1->addWidget( warnLabel );
    alarmLabel = new QLabel( this, "             " );
    Layout1->addWidget( alarmLabel );
    arangeLabel = new QLabel( this, "                                      " );
    Layout1->addWidget( arangeLabel );

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
    GUILBT672Layout->addLayout( Layout1 );

/* initialisation ***************************************************/

   _logger->log(Logger::LOG_LEV_INFO, "Initialization started");
   Init=0;
   Dummy=FALSE;
   timeoutR=2000;
   NBuf = 200;  //max buffer length
   if (NBuf>NBUF) NBuf=NBUF;
   NBufL = 200;  //max LONG buffer length
   if (NBufL>NBUF) NBufL=NBUF;
   NLong=10;
   iLong=0;
   timeB[0]=0.;
   Nb=0;
   Nbl=0;

   ICr1=0;       //Crate indixes
   ICr2=0;
   ICr3=0;
   IBo=0;        //Board inex
   ICh=0;        //Channel index
   //nfamily=135;  //if there is a need in InitTimestamp, set this to 124
   nfamily=sizeof(FamilyName)/4;  //if there is a need in InitTimestamp, set this to 124
    _logger->log(Logger::LOG_LEV_INFO, "num of families= %d",nfamily);
   nadamfamily=sizeof(AdamFamilyName)/4;  
   for (i=0; i<nfamily; i++) FamilyNames[i]=FamilyName[i];
   for (i=0; i<nadamfamily; i++) AdamFamilyNames[i]=AdamFamilyName[i];
   int i1,i2,i3;
   for (i1=0; i1<14; i1++) {        //initial status
       if (i1<7) DiagAppDataStat[i1] = 0;
       if (i1<9) {
          ExternalStat[i1] = 0;
       }
       if (i1<3) BCU2MastStat[i1] = 0;
       if (i1<4) BCU2SGNStat[i1] = 0;
       if (i1<4) BCU2TrStat[i1] = 0;
//       if (i1<8) DSPNiosStat[i1] = 0;
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

   ExternalInd[0] = 113;
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

   BCU2MastInd[0] = 79;
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
   _logger->log(Logger::LOG_LEV_INFO, "Initialization end");
/* initialisation **************************end**********************/

    languageChange();
    resize( QSize(1049, 689).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    timer = new QTimer (this);



    // signals and slots connections
    for (i=0; i<7;  i++) connect( BCU1RBut[i], SIGNAL( clicked() ), this, SLOT( SetICr1() ) );
    for (i=0; i<6;  i++) connect( BCU2RBut[i], SIGNAL( clicked() ), this, SLOT( SetICr2() ) );
    for (i=0; i<6;  i++) connect( DSP1RBut[i], SIGNAL( clicked() ), this, SLOT( SetICr3() ) );
    for (i=0; i<14; i++) connect( DSP2RBut[i], SIGNAL( clicked() ), this, SLOT( SetIBo() ) );
    for (i=0; i<8;  i++) connect( DSP3RBut[i], SIGNAL( clicked() ), this, SLOT( SetICh() ) );
    connect( buttonStart, SIGNAL( clicked() ), this, SLOT( StopStartTimer() ) );
    connect( buttonExit, SIGNAL( clicked() ), this, SLOT( quit() ) );

    connect( timer, SIGNAL( timeout() ), this, SLOT( Refresh() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
GUILBT672::~GUILBT672()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void GUILBT672::languageChange()
{
    char temp[300];
    string side = Utils::uppercase(Utils::getAdoptSide());
    if (side == "R") setCaption( tr( "LBT672 Housekeeping RIGHT" ) );
    else setCaption( tr( "LBT672 Housekeeping LEFT" ) );
    
    tabWidget->changeTab( tabSYS, tr( "System Summary" ) );
    //System Summary left panel
    labelDiagn->setText( tr( "DiagnApp Data" ) );
    /*textLabel[0]->setText( tr( "DiagnAppFrameTimestamp" ) );
    textLabel[1]->setText( tr( "DiagnAppFrameCounter" ) );
    textLabel[2]->setText( tr( "DiagnAppFastVarsCounter" ) );
    textLabel[3]->setText( tr( "DiagnAppSlowVarsCounter" ) );
    textLabel[4]->setText( tr( "DiagnAppFrameRate" ) );*/
//    textLabel[0]->setText( tr( "DiagnAppFastVariable [s]" ) );
    textLabel[0]->setText( tr( "DiagnAppFastRate [Hz]" ) );
    textLabel[1]->setText( tr( "DiagnAppSlowVariable [s]" ) );

    labelDiagnTemp->setText( tr( "AdSec672 Temperatures" ) );
    textLabelTemp[0]->setText(tr("Hub Temp [C]"));
    textLabelTemp[1]->setText(tr("ColdPlate Temp [C]"));
    textLabelTemp[2]->setText(tr("RefBody Temp [C]"));
    textLabelTemp[3]->setText(tr("InnerStruct Temp [C]"));
    textLabelTemp[4]->setText(tr("PowBackPlane Temp [C]"));

    //System Summary right panel
    labelTemper->setText( tr( "External Temperatures and Cooling Data" ) );
    textLabelE[0]->setText( tr( "ExternalTemperature [C]" ) );
    ExternalBut[0]->setText( tr( "Plot" ) );
    textLabelE[1]->setText( tr( "FluxRateIn [L/min]" ) );
    ExternalBut[1]->setText( tr( "Plot" ) );
    textLabelE[2]->setText( tr( "WaterMainInlet [C]" ) );
    ExternalBut[2]->setText( tr( "Plot" ) );
    textLabelE[3]->setText( tr( "WaterMainOutlet [C]" ) );
    ExternalBut[3]->setText( tr( "Plot" ) );
    textLabelE[4]->setText( tr( "WaterColdPlateInlet [C]" ) );
    ExternalBut[4]->setText( tr( "Plot" ) );
    textLabelE[5]->setText( tr( "WaterColdPlateOutlet [C]" ) );
    ExternalBut[5]->setText( tr( "Plot" ) );
    textLabelE[6]->setText( tr( "ExternalHumidity [%]" ) );
    ExternalBut[6]->setText( tr( "Plot" ) );
    textLabelE[7]->setText( tr( "DewPoint [C]" ) );
    ExternalBut[7]->setText( tr( "Plot" ) );
    textLabelE[8]->setText( tr( "Dewpoint Distance [C]" ) );
    ExternalBut[8]->setText( tr( "Plot" ) );

    tabWidget->changeTab( unnamed, tr( "Crate BCU 1" ) );
    //BCU1 left/upper panel
    labelNIOS->setText( tr( "NIOS Fixed Area" ) );
    textLabelBCU1[0]->setText( tr( "CrateID" ) );
    textLabelBCU1[1]->setText( tr( "WhoAmI" ) );
    textLabelBCU1[2]->setText( tr( "SoftwareRelease" ) );
    textLabelBCU1[3]->setText( tr( "LogicRelease" ) );
    textLabelBCU1[4]->setText( tr( "IPaddress" ) );
    textLabelBCU1[5]->setText( tr( "FramesCounter" ) );
    textLabelBCU1[6]->setText( tr( "SerialNumber" ) );
    textLabelBCU1[7]->setText( tr( "PowerBackplaneSerial" ) );
    textLabelBCU1[8]->setText( tr( "EnableMasterDiagnostic" ) );
    textLabelBCU1[9]->setText( tr( "DecimationFactor" ) );
    textLabelBCU1[10]->setText( tr( "RemoteIPaddress" ) );
    //BCU1 left/down panel
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
    //BCU1 middle/upper panel
    labelEnvBCU1->setText( tr( "Environment" ) );
    BCU1EnvText[0]->setText( tr( "Cooler in0" ) );
    BCU1EnvBut[0]->setText( tr( "Plot" ) );
    BCU1EnvText[1]->setText( tr( "Cooler in1" ) );
    BCU1EnvBut[1]->setText( tr( "Plot" ) );
    BCU1EnvText[2]->setText( tr( "Cooler out0" ) );
    BCU1EnvBut[2]->setText( tr( "Plot" ) );
    BCU1EnvText[3]->setText( tr( "Cooler out1" ) );
    BCU1EnvBut[3]->setText( tr( "Plot" ) );
    BCU1EnvText[4]->setText( tr( "Cooler Pressure" ) );
    BCU1EnvBut[4]->setText( tr( "Plot" ) );
    BCU1EnvText[5]->setText( tr( "Humidity" ) );
    BCU1EnvBut[5]->setText( tr( "Plot" ) );
    //BCU1 middle/down panel
    labelTempBCU1->setText( tr( "Temperatures" ) );
    BCU1TempText[0]->setText( tr( "StratixTemp [C]" ) );
    BCU1TempBut[0]->setText( tr( "Plot" ) );
    BCU1TempText[1]->setText( tr( "PowerTemp [C]" ) );
    BCU1TempBut[1]->setText( tr( "Plot" ) );
    //BCU1 right/upper panel
    labelVoltBCU1->setText( tr( "Voltage and Currents" ) );
    BCU1VolText[0]->setText( tr( "VoltageVCCL [V]" ) );
    BCU1VolBut[0]->setText( tr( "Plot" ) );
    BCU1VolText[1]->setText( tr( "VoltageVCCA [V]" ) );
    BCU1VolBut[1]->setText( tr( "Plot" ) );
    BCU1VolText[2]->setText( tr( "VoltageVSSA [V]" ) );
    BCU1VolBut[2]->setText( tr( "Plot" ) );
    BCU1VolText[3]->setText( tr( "VoltageVCCP [V]" ) );
    BCU1VolBut[3]->setText( tr( "Plot" ) );
    BCU1VolText[4]->setText( tr( "VoltageVSSP [V]" ) );
    BCU1VolBut[4]->setText( tr( "Plot" ) );
    BCU1VolText[5]->setText( tr( "CurrentVCCL [A]" ) );
    BCU1VolBut[5]->setText( tr( "Plot" ) );
    BCU1VolText[6]->setText( tr( "CurrentVCCA [A]" ) );
    BCU1VolBut[6]->setText( tr( "Plot" ) );
    BCU1VolText[7]->setText( tr( "CurrentVSSA [A]" ) );
    BCU1VolBut[7]->setText( tr( "Plot" ) );
    BCU1VolText[8]->setText( tr( "CurrentVCCP [A]" ) );
    BCU1VolBut[8]->setText( tr( "Plot" ) );
    BCU1VolText[9]->setText( tr( "CurrentVSSP [A]" ) );
    BCU1VolBut[9]->setText( tr( "Plot" ) );
    //BCU1 right/down panel
    labelTempSIGGENd->setText( tr( "SIGGEN" ) );
    labelTempSIGGENd_2->setText( tr( "Temperatures" ) );
    BCU1SGNTempText[0]->setText( tr( "Stratix [C]" ) );
    BCU1SGNTempBut[0]->setText( tr( "Plot" ) );
    BCU1SGNTempText[1]->setText( tr( "Power [C]" ) );
    BCU1SGNTempBut[1]->setText( tr( "Plot" ) );
    BCU1SGNTempText[2]->setText( tr( "DSPs [C]" ) );
    BCU1SGNTempBut[2]->setText( tr( "Plot" ) );

    tabWidget->changeTab( TabPage, tr( "Crate BCU 2" ) );
    //BCU2 left/upper panel
    labelResets->setText( tr( "Reset status" ) );
    BCU2ResetText[0]->setText( tr( "FPGA Reset" ) );
    BCU2ResetText[1]->setText( tr( "BUS Reset" ) );
    BCU2ResetText[2]->setText( tr( "DSP Reset" ) );
    BCU2ResetText[3]->setText( tr( "Flash Reset" ) );
    BCU2ResetText[4]->setText( tr( "PCI Reset" ) );
    BCU2ResetText[5]->setText( tr( "BUS Driver Enable" ) );
    BCU2ResetText[6]->setText( tr( "BUS Driver Enable Status" ) );
    BCU2ResetText[7]->setText( tr( "BUS Power Fault Ctrl" ) );
    BCU2ResetText[8]->setText( tr( "BUS Power Fault" ) );
    BCU2ResetText[9]->setText( tr( "System watchdog" ) );
    //BCU2 left/down panel
    sprintf (temp,"Crate %1i",ICr2);
    crateBCU2_gr->setTitle( tr( temp ) );
    BCU2RBut[0]->setText( tr( "0" ) );
    BCU2RBut[1]->setText( tr( "1" ) );
    BCU2RBut[2]->setText( tr( "2" ) );
    BCU2RBut[3]->setText( tr( "3" ) );
    BCU2RBut[4]->setText( tr( "4" ) );
    BCU2RBut[5]->setText( tr( "5" ) );
    //BCU2 middle/upper panel
    labelDigital->setText( tr( "Digital I/O" ) );
    BCU2DigText[0]->setText( tr( "Driver Enabled" ) );
    BCU2DigText[1]->setText( tr( "Crate is master" ) );
    BCU2DigText[2]->setText( tr( "BUS system fault" ) );
    BCU2DigText[3]->setText( tr( "VCC fault" ) );
    BCU2DigText[4]->setText( tr( "AC power fault 0" ) );
    BCU2DigText[5]->setText( tr( "AC power fault 1" ) );
    BCU2DigText[6]->setText( tr( "AC power fault 2" ) );
    BCU2DigText[7]->setText( tr( "IC disconnected" ) );
    BCU2DigText[8]->setText( tr( "Overcurrent" ) );
    BCU2DigText[9]->setText( tr( "Crate ID" ) );
    //BCU2 middle/down panel
    labelDiagn_2->setText( tr( "SIGGEN" ) );
    labelDiagn_3->setText( tr( "NIOS Fixed Area" ) );
    BCU2SGNText[0]->setText( tr( "WhoAmI" ) );
    BCU2SGNText[1]->setText( tr( "SoftwareRelease" ) );
    BCU2SGNText[2]->setText( tr( "LogicRelease" ) );
    BCU2SGNText[3]->setText( tr( "SerialNumber" ) );
    //BCU2 right/upper panel
    labelEnvBCU2->setText( tr( "Master crate global current levels" ) );
    BCU2MastText[0]->setText( tr( "CurrentVCCP [A]" ) );
    BCU2MastBut[0]->setText( tr( "Plot" ) );
    BCU2MastText[1]->setText( tr( "CurrentVSSP [A]" ) );
    BCU2MastBut[1]->setText( tr( "Plot" ) );
    BCU2MastText[2]->setText( tr( "CurrentVP [A]" ) );
    BCU2MastBut[2]->setText( tr( "Plot" ) );
    //BCU2 right/down panel
    BCU2TrText[0]->setText( tr( "LocalCurrentThreshold [A]" ) );
    BCU2TrBut[0]->setText( tr( "Plot" ) );
    BCU2TrText[1]->setText( tr( "VPSet [V]" ) );
    BCU2TrBut[1]->setText( tr( "Plot" ) );
    BCU2TrText[2]->setText( tr( "TotalCurrentThresholdPos [A]" ) );
    BCU2TrBut[2]->setText( tr( "Plot" ) );
    BCU2TrText[3]->setText( tr( "TotalCurrentThresholdNeg [A]" ) );
    BCU2TrBut[3]->setText( tr( "Plot" ) );

    tabWidget->changeTab( TabPage_2, tr( "Crate DSP" ) );
    //DSP left/upper panel
    labelDiagn_4->setText( tr( "NIOS Fixed Area" ) );
    DSPNiosText[0]->setText( tr( "WhoAmI" ) );
    DSPNiosText[1]->setText( tr( "SoftwareRelease" ) );
    DSPNiosText[2]->setText( tr( "LogicRelease" ) );
    DSPNiosText[3]->setText( tr( "SerialNumber" ) );
    DSPNiosText[4]->setText( tr( "DiagnosticRecordPtr" ) );
    DSPNiosText[5]->setText( tr( "DiagnosticRecordLen" ) );
    DSPNiosText[6]->setText( tr( "RdDiagnosticRecordPtr" ) );
    DSPNiosText[7]->setText( tr( "WrDiagnosticRecordPtr" ) );
    //DSP left/down panel
    labelTempDSPd->setText( tr( "Temperatures" ) );
    DSPTempText[0]->setText( tr( "Stratix [C]" ) );
    DSPTempBut[0]->setText( tr( "Plot" ) );
    DSPTempText[1]->setText( tr( "Power [C]" ) );
    DSPTempBut[1]->setText( tr( "Plot" ) );
    DSPTempText[2]->setText( tr( "DSPs [C]" ) );
    DSPTempBut[2]->setText( tr( "Plot" ) );
    DSPTempText[3]->setText( tr( "Drivers [C]" ) );
    DSPTempBut[3]->setText( tr( "Plot" ) );
    sprintf (temp,"Crate %1i",ICr3);
    crateSIGGEN_gr->setTitle( tr( temp ) );
    DSP1RBut[0]->setText( tr( "0" ) );
    DSP1RBut[1]->setText( tr( "1" ) );
    DSP1RBut[2]->setText( tr( "2" ) );
    DSP1RBut[3]->setText( tr( "3" ) );
    DSP1RBut[4]->setText( tr( "4" ) );
    DSP1RBut[5]->setText( tr( "5" ) );
    //DSP middle/upper panel
    labelTempDSPc_0->setText( tr( "Coil currents (SPI) [A]" ) );
    DSPCurText[0]->setText( tr( "ch0" ) );
    DSPCurBut[0]->setText( tr( "Plot" ) );
    DSPCurText[1]->setText( tr( "ch1" ) );
    DSPCurBut[1]->setText( tr( "Plot" ) );
    DSPCurText[2]->setText( tr( "ch2" ) );
    DSPCurBut[2]->setText( tr( "Plot" ) );
    DSPCurText[3]->setText( tr( "ch3" ) );
    DSPCurBut[3]->setText( tr( "Plot" ) );
    DSPCurText[4]->setText( tr( "ch4" ) );
    DSPCurBut[4]->setText( tr( "Plot" ) );
    DSPCurText[5]->setText( tr( "ch5" ) );
    DSPCurBut[5]->setText( tr( "Plot" ) );
    DSPCurText[6]->setText( tr( "ch6" ) );
    DSPCurBut[6]->setText( tr( "Plot" ) );
    DSPCurText[7]->setText( tr( "ch7" ) );
    DSPCurBut[7]->setText( tr( "Plot" ) );
    //DSP middle/down panel
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
    DSPDrText[0]->setText( tr( "ch0" ) );
    DSPDrText[1]->setText( tr( "ch1" ) );
    DSPDrText[2]->setText( tr( "ch2" ) );
    DSPDrText[3]->setText( tr( "ch3" ) );
    DSPDrText[4]->setText( tr( "ch4" ) );
    DSPDrText[5]->setText( tr( "ch5" ) );
    DSPDrText[6]->setText( tr( "ch6" ) );
    DSPDrText[7]->setText( tr( "ch7" ) );
    //DSP right/upper panel
    labelDriverStatus->setText( tr( "Driver Status" ) );
    DSPDrSText[0]->setText( tr( "FPGA Reset" ) );
    DSPDrSText[1]->setText( tr( "DSP0 Reset" ) );
    DSPDrSText[2]->setText( tr( "DSP1 Reset" ) );
    DSPDrSText[3]->setText( tr( "Flash Reset" ) );
    DSPDrSText[4]->setText( tr( "BUS Power Fault Ctrl" ) );
    DSPDrSText[5]->setText( tr( "BUS Power Fault" ) );
    DSPDrSText[6]->setText( tr( "BUS Driver Enable" ) );
    DSPDrSText[7]->setText( tr( "Enable DSP Watchdog" ) );
    DSPDrSText[8]->setText( tr( "DSP0 watchdog expired" ) );
    DSPDrSText[9]->setText( tr( "DSP1 watchdog expired" ) );
    //DSP right/down panel
    DSPadText[0]->setText( tr( "ADCspiOffset" ) );
    DSPadText[1]->setText( tr( "ADCspiGain" ) );
    DSPadText[2]->setText( tr( "ADCOffset" ) );
    DSPadText[3]->setText( tr( "ADCGain" ) );
    DSPadText[4]->setText( tr( "DACOffset" ) );
    DSPadText[5]->setText( tr( "DACGain" ) );
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
    //mod RB
    //Adam Tab
    tabWidget->changeTab( tabAdam, tr( "Adam" ) );
    labelDiagnAdam->setText( tr( "Adam Data" ) );
    textLabelAdam[0]->setText( tr( "AdamMainPower" ) );
    textLabelAdam[1]->setText( tr( "AdamWatchdogExp" ) );
    textLabelAdam[2]->setText( tr( "AdamTSSDisabl" ) );
    textLabelAdam[3]->setText( tr( "AdamDriverEnable" ) );
    textLabelAdam[4]->setText( tr( "AdamFPGAClearN" ) );
    textLabelAdam[5]->setText( tr( "AdamBootSelectN" ) );
    textLabelAdam[6]->setText( tr( "AdamSysResetN" ) );
    textLabelAdam[7]->setText( tr( "AdamTSSPowerFaultN0" ) );
    textLabelAdam[8]->setText( tr( "AdamTSSPowerFaultN1" ) );
    textLabelAdam[9]->setText( tr( "AdamTSSFaultN" ) );
    textLabelAdam[10]->setText( tr( "AdamTCSSysFault" ) );
    textLabelAdam[11]->setText( tr( "AdamTCSPowerFaultN0" ) );
    textLabelAdam[12]->setText( tr( "AdamTCSPowerFaultN1" ) );
    textLabelAdam[13]->setText( tr( "AdamTCSPowerFaultN2" ) );

    clockLabel->setText(tr("System Time"));
    
   //end mod RB
    //Warnings Tab
    tabWidget->changeTab( Widget18, tr( "Warnings" ) );
    listBox1->clear();
    listBox1->insertItem( tr( "NO WARNINGS" ) );
    //Alarms Tab
    tabWidget->changeTab( Widget19, tr( "Alarms" ) );
    listBox1_2->clear();
    listBox1_2->insertItem( tr( "NO ALARMS" ) );
    //Down command buttons panel
    buttonStart->setText( tr( "&Stop" ) );
    buttonStart->setAccel( QKeySequence( tr( "Alt+S" ) ) );
    buttonStart_2->setText( tr( "&Help" ) );
    buttonStart_2->setAccel( QKeySequence( tr( "Alt+H" ) ) );
    buttonExit->setText( tr( "&Exit" ) );
    buttonExit->setAccel( QKeySequence( tr( "Alt+E" ) ) );
}


void GUILBT672::PlotSysLeft()
{
  double x[NBUF],y[NBUF];
  int i;
  double mean;
  double xmin,xmax;
  double ymin,ymax,ymin1,ymax1;
  char temp[100];

    //printf("Nbl=%d\n",Nbl);
    if (Nbl<2) return;

    qwtPlotSysLeft->clear();
    QwtLegend* Legend = new QwtLegend ();
    Legend->setDisplayPolicy(QwtLegend::AutoIdentifier,QwtLegendItem::ShowLine||QwtLegendItem::ShowText);
    qwtPlotSysLeft->insertLegend(Legend);

    QwtText Title = QwtText();
    Title.setText( (QString) "Temperatures");
    Title.setColor( QColor (200,0,0));
    qwtPlotSysLeft->setTitle(Title);
    qwtPlotSysLeft->setCanvasBackground( QColor (220,220,255));
    QwtText ytitle = QwtText();
    ytitle.setText( (QString) "temperature, C");

    qwtPlotSysLeft->setAxisTitle(QwtPlot::yLeft,ytitle);
    qwtPlotSysLeft->enableAxis(QwtPlot::xTop);
    //xmin = (double) (timeL[0]-2.);
    //xmax = (double) (timeL[Nbl-1]+2.);
    xmin = (double) timeL[0];
    xmax = (double) timeL[Nbl-1];
    qwtPlotSysLeft->setAxisScale(QwtPlot::xTop,xmin,xmax);
    //xmin = (double) (timeL[0]-timeL[Nbl-1]-2.);
    //xmax = 2.;
    xmin = (double) (timeL[0]-timeL[Nbl-1]);
    xmax = 0.;
    qwtPlotSysLeft->setAxisScale(QwtPlot::xBottom,xmin,xmax);
    QwtText xtitle = QwtText();
    xtitle.setText( (QString) "time, sec");
    //xtitle.setColor( QColor (200,0,0));
    qwtPlotSysLeft->setAxisTitle(QwtPlot::xBottom,xtitle);

    QwtPlotCurve* c1 = new QwtPlotCurve ((QString) "External");
    for (i=0; i<Nbl; i++) {
        x[i] = (double) (timeL[i]-timeL[Nbl-1]);
        y[i] = (double)ExternalBufVL[0][i];
    }

    c1->setData (x,y, Nbl);
    c1->setPen(QPen(QColor(255,0,0)));
    c1->attach (qwtPlotSysLeft);

    ymin= (double)ExternalAMin[0];
    ymax= (double)ExternalAMax[0];
    mean  = (double)ExternalBufVL[0][0];
    sprintf (temp,"%f",ymin);
    if (strcmp(temp,"-inf") ==0) ymin= mean*0.9;
    sprintf (temp,"%f",ymax);
    if (strcmp(temp,"inf") ==0) ymax= mean*1.1;
    for (i=2; i<9; i++) {
      if (i!=6) {
         ymin1 = (double)ExternalAMin[i];
         ymax1 = (double)ExternalAMax[i];
         mean  = (double)ExternalBufVL[i][0];
         sprintf (temp,"%f",ymin1);
         if (strcmp(temp,"-inf") ==0) ymin1= mean*0.9;
         sprintf (temp,"%f",ymax1);
         if (strcmp(temp,"inf") ==0) ymax1= mean*1.1;
         if (ymin> ymin1) ymin= ymin1;
         if (ymax< ymax1) ymax= ymax1;
      }
    }
    ymin = 0.9*ymin;
    ymax = 1.1*ymax;
    qwtPlotSysLeft->setAxisScale(QwtPlot::yLeft,ymin,ymax);

    QwtPlotCurve* c2 = new QwtPlotCurve ((QString) "MainInlet");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)ExternalBufVL[2][i];
    }
    c2->setData (x,y, Nbl);
    c2->setPen(QPen(QColor(150,150,0)));
    c2->attach (qwtPlotSysLeft);

    QwtPlotCurve* c2a = new QwtPlotCurve ((QString) "MainOutlet");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)ExternalBufVL[3][i];
    }
    c2a->setData (x,y, Nbl);
    c2a->setPen(QPen(QColor(0,255,255)));
    c2a->attach (qwtPlotSysLeft);

    QwtPlotCurve* c2b = new QwtPlotCurve ((QString) "ColdPlateInlet");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)ExternalBufVL[4][i];
    }
    c2b->setData (x,y, Nbl);
    c2b->setPen(QPen(QColor(0,255,0)));
    c2b->attach (qwtPlotSysLeft);

    QwtPlotCurve* c2c = new QwtPlotCurve ((QString) "ColdPlateOutlet");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)ExternalBufVL[5][i];
    }
    c2c->setData (x,y, Nbl);
    c2c->setPen(QPen(QColor(0,0,255)));
    c2c->attach (qwtPlotSysLeft);

    QwtPlotCurve* c3 = new QwtPlotCurve ((QString) "DewPoint");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)ExternalBufVL[7][i];
    }
    c3->setData (x,y, Nbl);
    c3->setPen(QPen(QColor(255,0,255)));
    c3->attach (qwtPlotSysLeft);

    QwtPlotCurve* c4 = new QwtPlotCurve ((QString) "CheckDewPoint");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)ExternalBufVL[8][i];
    }
    c4->setData (x,y, Nbl);
    c4->setPen(QPen(QColor(0,0,0)));
    c4->attach (qwtPlotSysLeft);

    qwtPlotSysLeft->replot();
    qwtPlotSysLeftZoom = new QwtPlotZoomer ( QwtPlot::xBottom,QwtPlot::yLeft, qwtPlotSysLeft->canvas() );
}

void GUILBT672::PlotSysRight()
{
  double x[NBUF],y[NBUF];
  int i;
  double xmin,xmax;
  double ymin,ymax;

    //printf("Nbl=%d\n",Nbl);
    if (Nbl<2) return;

    qwtPlotSysRight->clear();
    QwtLegend* Legend = new QwtLegend ();
    Legend->setDisplayPolicy(QwtLegend::AutoIdentifier,QwtLegendItem::ShowLine||QwtLegendItem::ShowText);
    qwtPlotSysRight->insertLegend(Legend);

    QwtText Title = QwtText();
    Title.setText( (QString) "FluxRateIn & ExternalHumidity");
    Title.setColor( QColor (0,0,220));
    qwtPlotSysRight->setTitle(Title);
    qwtPlotSysRight->setCanvasBackground( QColor (220,220,255));
         
    ymin= 0.;
    ymax= 16.;
    qwtPlotSysRight->setAxisScale(QwtPlot::yLeft,ymin,ymax);
    QwtText ytitle = QwtText();
    ytitle.setText( (QString) "flux, L/min");
    ytitle.setColor( QColor (0,0,200));
    qwtPlotSysRight->setAxisTitle(QwtPlot::yLeft,ytitle);
    
    ymin= 0.;
    ymax= 100.;
    qwtPlotSysRight->enableAxis(QwtPlot::yRight);
    qwtPlotSysRight->setAxisScale(QwtPlot::yRight,ymin,ymax);
    QwtText ytitle1 = QwtText();
    ytitle1.setText( (QString) "humidity, %");
    ytitle1.setColor( QColor (0,200,0));
    qwtPlotSysRight->setAxisTitle(QwtPlot::yRight,ytitle1);
      
    qwtPlotSysRight->enableAxis(QwtPlot::xTop);
    //xmin = (double) (timeL[0]-2.);
    //xmax = (double) (timeL[Nbl-1]+2.);
    xmin = (double) (timeL[0]);
    xmax = (double) (timeL[Nbl-1]);
    qwtPlotSysRight->setAxisScale(QwtPlot::xTop,xmin,xmax);
    //xmin = (double) (timeL[0]-timeL[Nbl-1]-2.);
    //xmax = 2.;
    xmin = (double) (timeL[0]-timeL[Nbl-1]);
    xmax = 0.;
    qwtPlotSysRight->setAxisScale(QwtPlot::xBottom,xmin,xmax);
    QwtText xtitle = QwtText();
    xtitle.setText( (QString) "time, sec");
    //xtitle.setColor( QColor (200,0,0));
    qwtPlotSysRight->setAxisTitle(QwtPlot::xBottom,xtitle);
        

    QwtPlotCurve* c1 = new QwtPlotCurve ((QString) "FluxRateIn");
    for (i=0; i<Nbl; i++) {
        x[i] = (double) (timeL[i]-timeL[Nbl-1]);
        y[i] = (double)ExternalBufVL[1][i];
    }
    c1->setData (x,y, Nbl);
    c1->setPen(QPen(QColor(0,0,200)));
    c1->setYAxis(QwtPlot::yLeft);
    c1->attach (qwtPlotSysRight);

    QwtPlotCurve* c2 = new QwtPlotCurve ((QString) "Humidity");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)ExternalBufVL[6][i];
    }
    c2->setData (x,y, Nbl);
    c2->setPen(QPen(QColor(0,200,0)));
    c2->setYAxis(QwtPlot::yRight);
    c2->attach (qwtPlotSysRight);

    qwtPlotSysRight->replot();
    qwtPlotSysRightZoom = new QwtPlotZoomer ( QwtPlot::xBottom,QwtPlot::yLeft, qwtPlotSysRight->canvas() );
}

void GUILBT672::PlotBCU1mu()                        //middle-upper panel
{
  double x[NBUF],y[NBUF];
  int i;
  double xmin,xmax;
  double ymin,ymax;

    //printf("Nbl=%d\n",Nbl);
    if (Nbl<2) return;
    if (ICr1==6) return;
    
    qwtBCU1mu->clear();
    QwtLegend* Legend = new QwtLegend ();
    Legend->setDisplayPolicy(QwtLegend::AutoIdentifier,QwtLegendItem::ShowLine||QwtLegendItem::ShowText);
    qwtBCU1mu->insertLegend(Legend);

    QwtText Title = QwtText();
    Title.setText( (QString) "Cooler Pressure & Humidity");
    Title.setColor( QColor (0,0,220));
    qwtBCU1mu->setTitle(Title);
    qwtBCU1mu->setCanvasBackground( QColor (220,220,255));
         
    ymin= 0.9*(double)BCU1EnvAMin[ICr1][4];
    ymax= 1.1*(double)BCU1EnvAMax[ICr1][4];
    qwtBCU1mu->setAxisScale(QwtPlot::yLeft,ymin,ymax);
    QwtText ytitle = QwtText();
    ytitle.setText( (QString) " P");
    ytitle.setColor( QColor (0,0,200));
    qwtBCU1mu->setAxisTitle(QwtPlot::yLeft,ytitle);
    
    ymin= 0.9*(double)BCU1EnvAMin[ICr1][5];
    ymax= 1.1*(double)BCU1EnvAMax[ICr1][5];
    qwtBCU1mu->enableAxis(QwtPlot::yRight);
    qwtBCU1mu->setAxisScale(QwtPlot::yRight,ymin,ymax);
    QwtText ytitle1 = QwtText();
    ytitle1.setText( (QString) " %");
    ytitle1.setColor( QColor (0,200,0));
    qwtBCU1mu->setAxisTitle(QwtPlot::yRight,ytitle1);
      
    qwtBCU1mu->enableAxis(QwtPlot::xTop);
    //xmin = (double) (timeL[0]-2.);
    //xmax = (double) (timeL[Nbl-1]+2.);
    xmin = (double) (timeL[0]);
    xmax = (double) (timeL[Nbl-1]);
    qwtBCU1mu->setAxisScale(QwtPlot::xTop,xmin,xmax);
    //xmin = (double) (timeL[0]-timeL[Nbl-1]-2.);
    //xmax = 2.;
    xmin = (double) (timeL[0]-timeL[Nbl-1]);
    xmax = 0.;
    qwtBCU1mu->setAxisScale(QwtPlot::xBottom,xmin,xmax);
    QwtText xtitle = QwtText();
    xtitle.setText( (QString) "time, sec");
    //xtitle.setColor( QColor (200,0,0));
    qwtBCU1mu->setAxisTitle(QwtPlot::xBottom,xtitle);
        

    QwtPlotCurve* c1 = new QwtPlotCurve ((QString) "Pressure");
    for (i=0; i<Nbl; i++) {
        x[i] = (double) (timeL[i]-timeL[Nbl-1]);
        y[i] = (double)BCU1EnvBufVL[ICr1][4][i];
    }
    c1->setData (x,y, Nbl);
    c1->setPen(QPen(QColor(0,0,200)));
    c1->setYAxis(QwtPlot::yLeft);
    c1->attach (qwtBCU1mu);

    QwtPlotCurve* c2 = new QwtPlotCurve ((QString) "Humidity");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)BCU1EnvBufVL[ICr1][5][i];
    }
    c2->setData (x,y, Nbl);
    c2->setPen(QPen(QColor(0,200,0)));
    c2->setYAxis(QwtPlot::yRight);
    c2->attach (qwtBCU1mu);
    
    qwtBCU1mu->replot();
    qwtBCU1muZoom = new QwtPlotZoomer ( QwtPlot::xBottom,QwtPlot::yLeft, qwtBCU1mu->canvas() );
}

void GUILBT672::PlotBCU1ld()                         //left-down panel
{
  double x[NBUF],y[NBUF];
  int i;
  double xmin,xmax;
  double ymin,ymax,ymin1,ymax1;
  double mean;
  char temp[100];

    //printf("Nbl=%d\n",Nbl);
    if (Nbl<2) return;
    
    qwtBCU1ld->clear();
    if (ICr1==6) return;
    QwtLegend* Legend = new QwtLegend ();
    Legend->setDisplayPolicy(QwtLegend::AutoIdentifier,QwtLegendItem::ShowLine||QwtLegendItem::ShowText);
    qwtBCU1ld->insertLegend(Legend);

    QwtText Title = QwtText();
    Title.setText( (QString) "Cooler Temperatures");
    Title.setColor( QColor (200,0,0));
    qwtBCU1ld->setTitle(Title);
    qwtBCU1ld->setCanvasBackground( QColor (220,220,255));

    for (i=0; i<4; i++) {
        ymin1 = (double)BCU1EnvAMin[ICr1][i];
        ymax1 = (double)BCU1EnvAMax[ICr1][i];
        mean = (double)BCU1EnvBufVL[ICr1][i][0];
        sprintf (temp,"%f",ymin1);
        if (strcmp(temp,"-inf") ==0) ymin1= mean*0.9;
        sprintf (temp,"%f",ymax1);
        if (strcmp(temp,"inf") ==0) ymax1= mean*1.1;
        if (i==0) {
          ymin= ymin1;
          ymax= ymax1;
        } else {
           if (ymin > ymin1) ymin= ymin1;
           if (ymax < ymax1) ymax= ymax1;
        }
    }
    ymin = 0.9*ymin;
    ymax = 1.1*ymax;
    qwtBCU1ld->setAxisScale(QwtPlot::yLeft,ymin,ymax);
    QwtText ytitle = QwtText();
    ytitle.setText( (QString) "temperature, C");

    qwtBCU1ld->setAxisTitle(QwtPlot::yLeft,ytitle);
    qwtBCU1ld->enableAxis(QwtPlot::xTop);
    //xmin = (double) (timeL[0]-2.);
    //xmax = (double) (timeL[Nbl-1]+2.);
    xmin = (double) (timeL[0]);
    xmax = (double) (timeL[Nbl-1]);
    qwtBCU1ld->setAxisScale(QwtPlot::xTop,xmin,xmax);
    //xmin = (double) (timeL[0]-timeL[Nbl-1]-2.);
    //xmax = 2.;
    xmin = (double) (timeL[0]-timeL[Nbl-1]);
    xmax = 0.;
    qwtBCU1ld->setAxisScale(QwtPlot::xBottom,xmin,xmax);
    QwtText xtitle = QwtText();
    xtitle.setText( (QString) "time, sec");
    //xtitle.setColor( QColor (200,0,0));
    qwtBCU1ld->setAxisTitle(QwtPlot::xBottom,xtitle);


    QwtPlotCurve* c1 = new QwtPlotCurve ((QString) "in0");
    for (i=0; i<Nbl; i++) {
        x[i] = (double) (timeL[i]-timeL[Nbl-1]);
        y[i] = (double)BCU1EnvBufVL[ICr1][0][i];
    }
    c1->setData (x,y, Nbl);
    c1->setPen(QPen(QColor(255,0,0)));
    c1->attach (qwtBCU1ld);

    QwtPlotCurve* c2 = new QwtPlotCurve ((QString) "in1");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)BCU1EnvBufVL[ICr1][1][i];
    }
    c2->setData (x,y, Nbl);
    c2->setPen(QPen(QColor(0,0,0)));
    c2->attach (qwtBCU1ld);

    QwtPlotCurve* c2a = new QwtPlotCurve ((QString) "out0");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)BCU1EnvBufVL[ICr1][2][i];
    }
    c2a->setData (x,y, Nbl);
    c2a->setPen(QPen(QColor(0,255,255)));
    c2a->attach (qwtBCU1ld);

    QwtPlotCurve* c2b = new QwtPlotCurve ((QString) "out1");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)BCU1EnvBufVL[ICr1][3][i];
    }
    c2b->setData (x,y, Nbl);
    c2b->setPen(QPen(QColor(0,255,0)));
    c2b->attach (qwtBCU1ld);

    qwtBCU1ld->replot();
    qwtBCU1ldZoom = new QwtPlotZoomer ( QwtPlot::xBottom,QwtPlot::yLeft, qwtBCU1ld->canvas() );
}

void GUILBT672::PlotBCU1md()                       //middle-down panel
{
  double x[NBUF],y[NBUF];
  int i;
  double xmin,xmax;
  double ymin,ymax;

    //printf("Nbl=%d  icr=%d\n",Nbl,ICr1);
    if (Nbl<2) return;
    
    qwtBCU1md->clear();
    QwtLegend* Legend = new QwtLegend ();
    Legend->setDisplayPolicy(QwtLegend::AutoIdentifier,QwtLegendItem::ShowLine||QwtLegendItem::ShowText);
    qwtBCU1md->insertLegend(Legend);

    QwtText Title = QwtText();
    Title.setText( (QString) "Temperatures");
    Title.setColor( QColor (200,0,0));
    qwtBCU1md->setTitle(Title);
    qwtBCU1md->setCanvasBackground( QColor (220,220,255));

    ymin= (double)BCU1TempAMin[ICr1][0];
    ymax= (double)BCU1TempAMax[ICr1][0];
    for (i=1; i<2; i++) {
        if (ymin> (double)BCU1TempAMin[ICr1][i]) ymin= (double)BCU1TempAMin[ICr1][i];
        if (ymax< (double)BCU1TempAMax[ICr1][i]) ymin= (double)BCU1TempAMax[ICr1][i];
    }
    ymin = 0.9*ymin;
    ymax = 1.1*ymax;
    qwtBCU1md->setAxisScale(QwtPlot::yLeft,ymin,ymax);
    QwtText ytitle = QwtText();
    ytitle.setText( (QString) "temperature, C");

    qwtBCU1md->setAxisTitle(QwtPlot::yLeft,ytitle);
    qwtBCU1md->enableAxis(QwtPlot::xTop);
    //xmin = (double) (timeL[0]-2.);
    //xmax = (double) (timeL[Nbl-1]+2.);
    xmin = (double) (timeL[0]);
    xmax = (double) (timeL[Nbl-1]);
    qwtBCU1md->setAxisScale(QwtPlot::xTop,xmin,xmax);
    //xmin = (double) (timeL[0]-timeL[Nbl-1]-2.);
    //xmax = 2.;
    xmin = (double) (timeL[0]-timeL[Nbl-1]);
    xmax = 0.;
    qwtBCU1md->setAxisScale(QwtPlot::xBottom,xmin,xmax);
    QwtText xtitle = QwtText();
    xtitle.setText( (QString) "time, sec");
    //xtitle.setColor( QColor (200,0,0));
    qwtBCU1md->setAxisTitle(QwtPlot::xBottom,xtitle);


    QwtPlotCurve* c1 = new QwtPlotCurve ((QString) "Stratix");
    for (i=0; i<Nbl; i++) {
        x[i] = (double) (timeL[i]-timeL[Nbl-1]);
        y[i] = (double)BCU1TempBufVL[ICr1][0][i];
    }
    c1->setData (x,y, Nbl);
    c1->setPen(QPen(QColor(255,0,0)));
    c1->attach (qwtBCU1md);

    QwtPlotCurve* c2 = new QwtPlotCurve ((QString) "Power");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)BCU1TempBufVL[ICr1][1][i];
    }
    c2->setData (x,y, Nbl);
    c2->setPen(QPen(QColor(0,0,0)));
    c2->attach (qwtBCU1md);


    qwtBCU1md->replot();
    qwtBCU1mdZoom = new QwtPlotZoomer ( QwtPlot::xBottom,QwtPlot::yLeft, qwtBCU1md->canvas() );
    
}

void GUILBT672::PlotBCU1rd()                         //right-down panel
{
  double x[NBUF],y[NBUF];
  int i;
  double xmin,xmax;
  double ymin,ymax;

    //printf("Nbl=%d\n",Nbl);
    if (Nbl<2) return;

    qwtBCU1rd->clear();
    if (ICr1==6) return;
    QwtLegend* Legend = new QwtLegend ();
    Legend->setDisplayPolicy(QwtLegend::AutoIdentifier,QwtLegendItem::ShowLine||QwtLegendItem::ShowText);
    qwtBCU1rd->insertLegend(Legend);

    QwtText Title = QwtText();
    Title.setText( (QString) "SIGGEN Temperatures");
    Title.setColor( QColor (200,0,0));
    qwtBCU1rd->setTitle(Title);
    qwtBCU1rd->setCanvasBackground( QColor (220,220,255));

    ymin= (double)BCU1SGNTempAMin[ICr1][0];
    ymax= (double)BCU1SGNTempAMax[ICr1][0];
    for (i=1; i<3; i++) {
        if (ymin> (double)BCU1SGNTempAMin[ICr1][i]) ymin= (double)BCU1SGNTempAMin[ICr1][i];
        if (ymax< (double)BCU1SGNTempAMax[ICr1][i]) ymin= (double)BCU1SGNTempAMax[ICr1][i];
    }
    ymin = 0.9*ymin;
    ymax = 1.1*ymax;
    qwtBCU1rd->setAxisScale(QwtPlot::yLeft,ymin,ymax);
    QwtText ytitle = QwtText();
    ytitle.setText( (QString) "temperature, C");

    qwtBCU1rd->setAxisTitle(QwtPlot::yLeft,ytitle);
    qwtBCU1rd->enableAxis(QwtPlot::xTop);
    //xmin = (double) (timeL[0]-2.);
    //xmax = (double) (timeL[Nbl-1]+2.);
    xmin = (double) (timeL[0]);
    xmax = (double) (timeL[Nbl-1]);
    qwtBCU1rd->setAxisScale(QwtPlot::xTop,xmin,xmax);
    //xmin = (double) (timeL[0]-timeL[Nbl-1]-2.);
    //xmax = 2.;
    xmin = (double) (timeL[0]-timeL[Nbl-1]);
    xmax = 0.;
    qwtBCU1rd->setAxisScale(QwtPlot::xBottom,xmin,xmax);
    QwtText xtitle = QwtText();
    xtitle.setText( (QString) "time, sec");
    //xtitle.setColor( QColor (200,0,0));
    qwtBCU1rd->setAxisTitle(QwtPlot::xBottom,xtitle);


    QwtPlotCurve* c1 = new QwtPlotCurve ((QString) "Stratix");
    for (i=0; i<Nbl; i++) {
        x[i] = (double) (timeL[i]-timeL[Nbl-1]);
        y[i] = (double)BCU1SGNTempBufVL[ICr1][0][i];
    }
    c1->setData (x,y, Nbl);
    c1->setPen(QPen(QColor(255,0,0)));
    c1->attach (qwtBCU1rd);

    QwtPlotCurve* c2 = new QwtPlotCurve ((QString) "Power");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)BCU1SGNTempBufVL[ICr1][1][i];
    }
    c2->setData (x,y, Nbl);
    c2->setPen(QPen(QColor(0,0,0)));
    c2->attach (qwtBCU1rd);

    QwtPlotCurve* c3 = new QwtPlotCurve ((QString) "DSPs");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)BCU1SGNTempBufVL[ICr1][2][i];
    }
    c3->setData (x,y, Nbl);
    c3->setPen(QPen(QColor(0,0,255)));
    c3->attach (qwtBCU1rd);


    qwtBCU1rd->replot();
    qwtBCU1rdZoom = new QwtPlotZoomer ( QwtPlot::xBottom,QwtPlot::yLeft, qwtBCU1rd->canvas() );
}

void GUILBT672::PlotBCU2ld()              //left-down panel
{
  double x[NBUF],y[NBUF];
  int i;
  double xmin,xmax;
  double ymin,ymax,ymin1,ymax1;
  double mean;
  char temp[100];

    //printf("Nbl=%d\n",Nbl);
    if (Nbl<2) return;
    
    qwtBCU2ld->clear();
    QwtLegend* Legend = new QwtLegend ();
    Legend->setDisplayPolicy(QwtLegend::AutoIdentifier,QwtLegendItem::ShowLine||QwtLegendItem::ShowText);
    qwtBCU2ld->insertLegend(Legend);

    QwtText Title = QwtText();
    Title.setText( (QString) "Thresholds");
    Title.setColor( QColor (200,0,0));
    qwtBCU2ld->setTitle(Title);
    qwtBCU2ld->setCanvasBackground( QColor (220,220,255));

    ymin= (double)BCU2TrAMin[0];
    ymax= (double)BCU2TrAMax[0];
    mean = (double)BCU2TrBufVL[0][0];
    sprintf (temp,"%f",ymin);
    if (strcmp(temp,"-inf") ==0) ymin= mean*0.9;
    sprintf (temp,"%f",ymax);
    if (strcmp(temp,"inf") ==0) ymax= mean*1.1;
    //printf("BU2ld min,max=%f %f\n",ymin,ymax);
    for (i=2; i<4; i++) {
        ymin1 = (double)BCU2TrAMin[i];
        ymax1 = (double)BCU2TrAMax[i];
        mean = (double)BCU2TrBufVL[i][0];
        sprintf (temp,"%f",ymin1);
        if (strcmp(temp,"-inf") ==0) ymin1= mean*0.9;
        sprintf (temp,"%f",ymax1);
        if (strcmp(temp,"inf") ==0) ymax1= mean*1.1;
        if (ymin > ymin1) ymin= ymin1;
        if (ymax < ymax1) ymax= ymax1;
        //printf("BU2ld min,max=%f %f min1,max1=%f %f\n",ymin,ymax,ymin1,ymax1);
    }
    ymin = 0.9*ymin;
    ymax = 1.1*ymax;
    qwtBCU2ld->setAxisScale(QwtPlot::yLeft,ymin,ymax);
    QwtText ytitle = QwtText();
    ytitle.setText( (QString) "current, A");
    qwtBCU2ld->setAxisTitle(QwtPlot::yLeft,ytitle);

    ymin= 0.9*(double)BCU2TrAMin[1];
    ymax= 1.1*(double)BCU2TrAMax[1];
        mean = (double)BCU2TrBufVL[1][0];
        sprintf (temp,"%f",ymin);
        if (strcmp(temp,"-inf") ==0) ymin= mean*0.9;
        sprintf (temp,"%f",ymax);
        if (strcmp(temp,"inf") ==0) ymax= mean*1.1;
    //printf("BU2ld VPSet min,max=%f %f\n",ymin,ymax);
    qwtBCU2ld->enableAxis(QwtPlot::yRight);
    qwtBCU2ld->setAxisScale(QwtPlot::yRight,ymin,ymax);
    QwtText ytitle1 = QwtText();
    ytitle1.setText( (QString) "VPSet, V");
    ytitle1.setColor( QColor (0,0,255));
    qwtBCU2ld->setAxisTitle(QwtPlot::yRight,ytitle1);

    qwtBCU2ld->enableAxis(QwtPlot::xTop);
    //xmin = (double) (timeL[0]-2.);
    //xmax = (double) (timeL[Nbl-1]+2.);
    xmin = (double) (timeL[0]);
    xmax = (double) (timeL[Nbl-1]);
    qwtBCU2ld->setAxisScale(QwtPlot::xTop,xmin,xmax);
    //xmin = (double) (timeL[0]-timeL[Nbl-1]-2.);
    //xmax = 2.;
    xmin = (double) (timeL[0]-timeL[Nbl-1]);
    xmax = 0.;
    qwtBCU2ld->setAxisScale(QwtPlot::xBottom,xmin,xmax);
    QwtText xtitle = QwtText();
    xtitle.setText( (QString) "time, sec");
    //xtitle.setColor( QColor (200,0,0));
    qwtBCU2ld->setAxisTitle(QwtPlot::xBottom,xtitle);


    QwtPlotCurve* c1 = new QwtPlotCurve ((QString) "Local");
    for (i=0; i<Nbl; i++) {
        x[i] = (double) (timeL[i]-timeL[Nbl-1]);
        y[i] = (double)BCU2TrBufVL[0][i];
    }
    c1->setData (x,y, Nbl);
    c1->setPen(QPen(QColor(255,0,0)));
    c1->setYAxis(QwtPlot::yLeft);
    c1->attach (qwtBCU2ld);

    QwtPlotCurve* c2 = new QwtPlotCurve ((QString) "Total+");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)BCU2TrBufVL[2][i];
    }
    c2->setData (x,y, Nbl);
    c2->setPen(QPen(QColor(0,0,0)));
    c2->setYAxis(QwtPlot::yLeft);
    c2->attach (qwtBCU2ld);

    QwtPlotCurve* c2a = new QwtPlotCurve ((QString) "Total-");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)BCU2TrBufVL[3][i];
    }
    c2a->setData (x,y, Nbl);
    c2a->setPen(QPen(QColor(0,255,255)));
    c2a->setYAxis(QwtPlot::yLeft);
    c2a->attach (qwtBCU2ld);

    QwtPlotCurve* c2b = new QwtPlotCurve ((QString) "VPSet");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)BCU2TrBufVL[1][i];
    }
    c2b->setData (x,y, Nbl);
    c2b->setPen(QPen(QColor(0,0,255)));
    c2b->setYAxis(QwtPlot::yRight);
    c2b->attach (qwtBCU2ld);

    qwtBCU2ld->replot();
    qwtBCU2ldZoom = new QwtPlotZoomer ( QwtPlot::xBottom,QwtPlot::yLeft, qwtBCU2ld->canvas() );
}

void GUILBT672::PlotBCU2ru()          //right-upper panel
{
  double x[NBUF],y[NBUF];
  int i;
  double xmin,xmax;
  double ymin,ymax;

    //printf("Nbl=%d\n",Nbl);
    if (Nbl<2) return;
    
    qwtBCU2ru->clear();
    QwtLegend* Legend = new QwtLegend ();
    Legend->setDisplayPolicy(QwtLegend::AutoIdentifier,QwtLegendItem::ShowLine||QwtLegendItem::ShowText);
    qwtBCU2ru->insertLegend(Legend);

    QwtText Title = QwtText();
    Title.setText( (QString) "Master crate Currents");
    Title.setColor( QColor (200,0,0));
    qwtBCU2ru->setTitle(Title);
    qwtBCU2ru->setCanvasBackground( QColor (220,220,255));

    ymin= (double)BCU2MastAMin[0];
    ymax= (double)BCU2MastAMax[0];
    for (i=1; i<3; i++) {
        if (ymin> (double)BCU2MastAMin[i]) ymin= (double)BCU2MastAMin[i];
        if (ymax< (double)BCU2MastAMax[i]) ymax= (double)BCU2MastAMax[i];
    }
    ymin = 0.9*ymin;
    ymax = 1.1*ymax;
    qwtBCU2ru->setAxisScale(QwtPlot::yLeft,ymin,ymax);
    QwtText ytitle = QwtText();
    ytitle.setText( (QString) "current, A");
    qwtBCU2ru->setAxisTitle(QwtPlot::yLeft,ytitle);

    qwtBCU2ru->enableAxis(QwtPlot::xTop);
    //xmin = (double) (timeL[0]-2.);
    //xmax = (double) (timeL[Nbl-1]+2.);
    xmin = (double) (timeL[0]);
    xmax = (double) (timeL[Nbl-1]);
    qwtBCU2ru->setAxisScale(QwtPlot::xTop,xmin,xmax);
    //xmin = (double) (timeL[0]-timeL[Nbl-1]-2.);
    //xmax = 2.;
    xmin = (double) (timeL[0]-timeL[Nbl-1]);
    xmax = 0.;
    qwtBCU2ru->setAxisScale(QwtPlot::xBottom,xmin,xmax);
    QwtText xtitle = QwtText();
    xtitle.setText( (QString) "time, sec");
    //xtitle.setColor( QColor (200,0,0));
    qwtBCU2ru->setAxisTitle(QwtPlot::xBottom,xtitle);


    QwtPlotCurve* c1 = new QwtPlotCurve ((QString) "VCCP");
    for (i=0; i<Nbl; i++) {
        x[i] = (double) (timeL[i]-timeL[Nbl-1]);
        y[i] = (double)BCU2MastBufVL[0][i];
    }
    c1->setData (x,y, Nbl);
    c1->setPen(QPen(QColor(255,0,0)));
    c1->attach (qwtBCU2ru);

    QwtPlotCurve* c2 = new QwtPlotCurve ((QString) "VSSP");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)BCU2MastBufVL[1][i];
    }
    c2->setData (x,y, Nbl);
    c2->setPen(QPen(QColor(0,0,0)));
    c2->attach (qwtBCU2ru);

    QwtPlotCurve* c2a = new QwtPlotCurve ((QString) "VP");
    for (i=0; i<Nbl; i++) {
        y[i] = (double)BCU2MastBufVL[2][i];
    }
    c2a->setData (x,y, Nbl);
    c2a->setPen(QPen(QColor(0,255,255)));
    c2a->attach (qwtBCU2ru);

    qwtBCU2ru->replot();
    qwtBCU2ruZoom = new QwtPlotZoomer ( QwtPlot::xBottom,QwtPlot::yLeft, qwtBCU2ru->canvas() );
}

void GUILBT672::newPlot( string s, int i)                //new window called from  PLOT buttons
{
   int iy;
   PlotForm* plot;

         iy=0;
    if (Nb <= 1 ) return;
    //some exeptions
    if ( s == "BCU1Env" && ICr1==6 )  return;
    if ( s == "BCU1Vol" && ICr1==6 )  return;
    if ( s == "BCU1SGNTemp" && ICr1==6 )  return;

    //plot = new PlotForm (0,"PLOT");  //if we like to have the possibility to hide from the taskbar
    plot = new PlotForm (this,"PLOT");
    plot->xtitle = (QwtText) "time, sec";
    plot->iy = iy;
    plot->Nx = Nb;
    plot->Refresh = TRUE;
    plot->Panel = s;
    plot->IndexP = i;
    fillPlot (plot);

    pplot.push_back(plot);
    plot->inVector = pplot.size()-1;
    connect (plot, SIGNAL (removeMyPlot (int) ), this, SLOT (remove_plot (int) ) );
}

void GUILBT672::fillPlot(PlotForm* plot)
{
   char temp[100];
   QPen pen;
   int it,i,iy,Nx;
   double mean,rms;
   double ymin,ymax;

   i=plot->IndexP;
   iy=plot->iy;
   Nx= plot->Nx;
   mean=0.;
   rms =0.;
       //printf("----Nx=%d NBuf=%d\n",Nx,NBuf);
   if ( plot->Panel == "External" ) {
      for (it=0; it<Nx;  it++) {
          plot->x[it] = (double) timeB[it];
          plot->y[iy][it] = (double)ExternalBufV[i][it];
          mean += (double)ExternalBufV[i][it];
      }
      mean = mean/Nx;
      for (it=0; it<Nx;  it++)  rms += pow( (double)ExternalBufV[i][it]-mean, 2);
      rms = sqrt (rms/Nx);

      plot->mean[iy] = mean;
      plot->rms[iy]  = rms;
      plot->amin[iy] = (double)ExternalAMin[i];
      plot->wmin[iy] = (double)ExternalWMin[i];
      plot->wmax[iy] = (double)ExternalWMax[i];
      plot->amax[iy] = (double)ExternalAMax[i];
      ymin = (double)ExternalAMin[i];
      ymax = (double)ExternalAMax[i];
      sprintf (temp,"%f",ymin);
      if (strcmp(temp,"-inf") ==0) ymin= mean- 2.*rms;
      sprintf (temp,"%f",ymax);
      if (strcmp(temp,"inf") ==0) ymax= mean+ 2.*rms;
      if (ymin > 0.) ymin=0.9*ymin;
      if (ymin < 0.) ymin=1.1*ymin;
      if (ymax > 0.) ymax=1.1*ymax;
      if (ymax < 0.) ymax=0.9*ymax;
      plot->ymin[iy] = ymin;
      plot->ymax[iy] = ymax;
      temp[0]='\0';
      //strcpy (temp, FamilyNames[ExternalInd[0]+i]);
      strcpy (temp, "Running mean");
      if (i==1) strcat (temp, ", L/min");
      else if (i==6) strcat (temp, ", %");
      else strcat (temp, ", C");
      plot->ytitle[iy] = (QwtText) temp;

      //plot->backgr = QColor (255,210,210);
      //plot->backgr = QColor (255,240,240);
      plot->backgr = QColor (255,255,255);
      plot->setCaption ((QString) "External Temperatures & Cooling Data");
      //plot->Title = (QwtText) FamilyNames[ExternalInd[0]+i];
      sprintf(temp,"%s     Mean= %6.2f",FamilyNames[ExternalInd[0]+i], mean);
      //sprintf(temp,"%s     Mean= %6.2f +- %6.2f",FamilyNames[ExternalInd[0]+i], (double)mean,(double)rms);
      plot->Title = (QwtText) temp;
      pen.setColor( QColor (0,0,0) );
      plot->pen[iy]=pen;
      plot->plotxy();
      plot->show();
      return;
   }
   if ( plot->Panel == "BCU1Env" ) {
      if (ICr1==6) return;
      for (it=0; it<Nx;  it++) {
          plot->x[it] = (double) timeB[it];
          plot->y[iy][it] = (double)BCU1EnvBufV[ICr1][i][it];
          mean += (double)BCU1EnvBufV[ICr1][i][it];
      }
      mean = mean/Nx;
      for (it=0; it<Nx;  it++)  rms += pow( (double)BCU1EnvBufV[ICr1][i][it]-mean, 2);
      rms = sqrt (rms/Nx);

      plot->mean[iy] = mean;
      plot->rms[iy]  = rms;
      plot->amin[iy] = (double)BCU1EnvAMin[ICr1][i];
      plot->wmin[iy] = (double)BCU1EnvWMin[ICr1][i];
      plot->wmax[iy] = (double)BCU1EnvWMax[ICr1][i];
      plot->amax[iy] = (double)BCU1EnvAMax[ICr1][i];
      ymin = (double)BCU1EnvAMin[ICr1][i];
      ymax = (double)BCU1EnvAMax[ICr1][i];
      sprintf (temp,"%f",ymin);
      if (strcmp(temp,"-inf") ==0) ymin= mean- 2.*rms;
      sprintf (temp,"%f",ymax);
      if (strcmp(temp,"inf") ==0) ymax= mean+ 2.*rms;
      if (ymin > 0.) ymin=0.9*ymin;
      if (ymin < 0.) ymin=1.1*ymin;
      if (ymax > 0.) ymax=1.1*ymax;
      if (ymax < 0.) ymax=0.9*ymax;
      if (ymin==ymax) {
         ymin=-1.;
         ymax=+1.;
      }
      plot->ymin[iy] = ymin;
      plot->ymax[iy] = ymax;
      temp[0]='\0';
      //strcpy (temp, FamilyNames[BCU1EnvInd[0]+i]);
      strcpy (temp, "Running mean");
      if (i==4) strcat (temp, ", P");
      else if (i==5) strcat (temp, ", %");
      else strcat (temp, ", C");
      plot->ytitle[iy] = (QwtText) temp;

      //plot->backgr = QColor (255,210,210);
      //plot->backgr = QColor (255,240,240);
      plot->backgr = QColor (255,255,255);
      //sprintf(temp,"BCU1 Environment:    Crate=%d",ICr1);
      //plot->setCaption ((QString) temp);
      plot->setCaption ((QString) "BCU1 Environment");
      //plot->Title = (QwtText) FamilyNames[BCU1EnvInd[0]+i];
      sprintf(temp,"Crate=%d   %s   Mean= %6.2f",ICr1,FamilyNames[BCU1EnvInd[0]+i],mean);
      //sprintf(temp,"Crate=%d   %s   Mean= %6.2f +- %6.2f",ICr1,FamilyNames[BCU1EnvInd[0]+i], (double)mean,(double)rms);
      plot->Title = (QwtText) temp;
      pen.setColor( QColor (0,0,0) );
      plot->pen[iy]=pen;
      plot->plotxy();
      plot->show();
      return;
   }
   if ( plot->Panel == "BCU1Vol" ) {
      if (ICr1==6) return;
      for (it=0; it<Nx;  it++) {
          plot->x[it] = (double) timeB[it];
          plot->y[iy][it] = (double)BCU1VolBufV[ICr1][i][it];
          mean += (double)BCU1VolBufV[ICr1][i][it];
      }
      mean = mean/Nx;
      for (it=0; it<Nx;  it++)  rms += pow( (double)BCU1VolBufV[ICr1][i][it]-mean, 2);
      rms = sqrt (rms/Nx);

      plot->mean[iy] = mean;
      plot->rms[iy]  = rms;
      plot->amin[iy] = (double)BCU1VolAMin[ICr1][i];
      plot->wmin[iy] = (double)BCU1VolWMin[ICr1][i];
      plot->wmax[iy] = (double)BCU1VolWMax[ICr1][i];
      plot->amax[iy] = (double)BCU1VolAMax[ICr1][i];
      ymin = (double)BCU1VolAMin[ICr1][i];
      ymax = (double)BCU1VolAMax[ICr1][i];
      sprintf (temp,"%f",ymin);
      if (strcmp(temp,"-inf") ==0) ymin= mean- 2.*rms;
      sprintf (temp,"%f",ymax);
      if (strcmp(temp,"inf") ==0) ymax= mean+ 2.*rms;
      if (ymin > 0.) ymin=0.9*ymin;
      if (ymin < 0.) ymin=1.1*ymin;
      if (ymax > 0.) ymax=1.1*ymax;
      if (ymax < 0.) ymax=0.9*ymax;
      plot->ymin[iy] = ymin;
      plot->ymax[iy] = ymax;
      temp[0]='\0';
      //strcpy (temp, FamilyNames[BCU1VolInd[0]+i]);
      strcpy (temp, "Running mean");
      if (i<5) strcat (temp, ", V");
      else strcat (temp, ", A");
      plot->ytitle[iy] = (QwtText) temp;

      //plot->backgr = QColor (255,210,210);
      //plot->backgr = QColor (255,240,240);
      plot->backgr = QColor (255,255,255);
      //sprintf(temp,"BCU1 Environment:    Crate=%d",ICr1);
      //plot->setCaption ((QString) temp);
      plot->setCaption ((QString) "BCU1 Voltage and Currents");
      //plot->Title = (QwtText) FamilyNames[BCU1VolInd[0]+i];
      sprintf(temp,"Crate=%d   %s   Mean= %6.2f",ICr1,FamilyNames[BCU1VolInd[0]+i],mean);
      //sprintf(temp,"Crate=%d   %s   Mean= %6.2f +- %6.2f",ICr1,FamilyNames[BCU1VolInd[0]+i], (double)mean,(double)rms);
      plot->Title = (QwtText) temp;
      pen.setColor( QColor (0,0,0) );
      plot->pen[iy]=pen;
      plot->plotxy();
      plot->show();
      return;
   }
   if ( plot->Panel == "BCU1Temp" ) {
      for (it=0; it<Nx;  it++) {
          plot->x[it] = (double) timeB[it];
          plot->y[iy][it] = (double)BCU1TempBufV[ICr1][i][it];
          mean += (double)BCU1TempBufV[ICr1][i][it];
      }
      mean = mean/Nx;
      for (it=0; it<Nx;  it++)  rms += pow( (double)BCU1TempBufV[ICr1][i][it]-mean, 2);
      rms = sqrt (rms/Nx);

      plot->mean[iy] = mean;
      plot->rms[iy]  = rms;
      plot->amin[iy] = (double)BCU1TempAMin[ICr1][i];
      plot->wmin[iy] = (double)BCU1TempWMin[ICr1][i];
      plot->wmax[iy] = (double)BCU1TempWMax[ICr1][i];
      plot->amax[iy] = (double)BCU1TempAMax[ICr1][i];
      ymin = (double)BCU1TempAMin[ICr1][i];
      ymax = (double)BCU1TempAMax[ICr1][i];
      sprintf (temp,"%f",ymin);
      if (strcmp(temp,"-inf") ==0) ymin= mean- 2.*rms;
      sprintf (temp,"%f",ymax);
      if (strcmp(temp,"inf") ==0) ymax= mean+ 2.*rms;
      if (ymin > 0.) ymin=0.9*ymin;
      if (ymin < 0.) ymin=1.1*ymin;
      if (ymax > 0.) ymax=1.1*ymax;
      if (ymax < 0.) ymax=0.9*ymax;
      plot->ymin[iy] = ymin;
      plot->ymax[iy] = ymax;
      temp[0]='\0';
      //strcpy (temp, FamilyNames[BCU1TempInd[0]+i]);
      strcpy (temp, "Running mean, C");
      plot->ytitle[iy] = (QwtText) temp;

      //plot->backgr = QColor (255,210,210);
      //plot->backgr = QColor (255,240,240);
      plot->backgr = QColor (255,255,255);
      //sprintf(temp,"BCU1 Temperatures:    Crate=%d",ICr1);
      //plot->setCaption ((QString) temp);
      plot->setCaption ((QString) "BCU1 Temperatures");
      //plot->Title = (QwtText) FamilyNames[BCU1TempInd[0][0]+i];
      if (ICr1==6)
         sprintf(temp,"Crate=%d   %s   Mean= %6.2f",ICr1,FamilyNames[BCU1TempInd[1][0]+i],mean);
         //sprintf(temp,"Crate=%d   %s   Mean= %6.2f +- %6.2f",ICr1,FamilyNames[BCU1TempInd[1][0]+i], (double)mean,(double)rms);
      else
         sprintf(temp,"Crate=%d   %s   Mean= %6.2f",ICr1,FamilyNames[BCU1TempInd[0][0]+i],mean);
         //sprintf(temp,"Crate=%d   %s   Mean= %6.2f +- %6.2f",ICr1,FamilyNames[BCU1TempInd[0][0]+i], (double)mean,(double)rms);
      plot->Title = (QwtText) temp;
      pen.setColor( QColor (0,0,0) );
      plot->pen[iy]=pen;
      plot->plotxy();
      plot->show();
      return;
   }
   if ( plot->Panel == "BCU1SGNTemp" ) {
      if (ICr1==6) return;
      for (it=0; it<Nx;  it++) {
          plot->x[it] = (double) timeB[it];
          plot->y[iy][it] = (double)BCU1SGNTempBufV[ICr1][i][it];
          mean += (double)BCU1SGNTempBufV[ICr1][i][it];
      }
      mean = mean/Nx;
      for (it=0; it<Nx;  it++)  rms += pow( (double)BCU1SGNTempBufV[ICr1][i][it]-mean, 2);
      rms = sqrt (rms/Nx);

      plot->mean[iy] = mean;
      plot->rms[iy]  = rms;
      plot->amin[iy] = (double)BCU1SGNTempAMin[ICr1][i];
      plot->wmin[iy] = (double)BCU1SGNTempWMin[ICr1][i];
      plot->wmax[iy] = (double)BCU1SGNTempWMax[ICr1][i];
      plot->amax[iy] = (double)BCU1SGNTempAMax[ICr1][i];
      ymin = (double)BCU1SGNTempAMin[ICr1][i];
      ymax = (double)BCU1SGNTempAMax[ICr1][i];
      sprintf (temp,"%f",ymin);
      if (strcmp(temp,"-inf") ==0) ymin= mean- 2.*rms;
      sprintf (temp,"%f",ymax);
      if (strcmp(temp,"inf") ==0) ymax= mean+ 2.*rms;
      if (ymin > 0.) ymin=0.9*ymin;
      if (ymin < 0.) ymin=1.1*ymin;
      if (ymax > 0.) ymax=1.1*ymax;
      if (ymax < 0.) ymax=0.9*ymax;
      plot->ymin[iy] = ymin;
      plot->ymax[iy] = ymax;
      temp[0]='\0';
      //strcpy (temp, FamilyNames[BCU1SGNTempInd[0]+i]);
      strcpy (temp, "Running mean, C");
      plot->ytitle[iy] = (QwtText) temp;

      //plot->backgr = QColor (255,210,210);
      //plot->backgr = QColor (255,240,240);
      plot->backgr = QColor (255,255,255);
      //sprintf(temp,"BCU1 SIGGEN Temperatures:    Crate=%d",ICr1);
      //plot->setCaption ((QString) temp);
      plot->setCaption ((QString) "BCU1  SIGGEN Temperatures");
      //plot->Title = (QwtText) FamilyNames[BCU1SGNTempInd[0]+i];
      sprintf(temp,"Crate=%d   %s   Mean= %6.2f",ICr1,FamilyNames[BCU1SGNTempInd[0]+i],mean);
      //sprintf(temp,"Crate=%d   %s   Mean= %6.2f +- %6.2f",ICr1,FamilyNames[BCU1SGNTempInd[0]+i], (double)mean,(double)rms);
      plot->Title = (QwtText) temp;
      pen.setColor( QColor (0,0,0) );
      plot->pen[iy]=pen;
      plot->plotxy();
      plot->show();
      return;
   }
   if ( plot->Panel == "BCU2Mast" ) {
      for (it=0; it<Nx;  it++) {
          plot->x[it] = (double) timeB[it];
          plot->y[iy][it] = (double)BCU2MastBufV[i][it];
          mean += (double)BCU2MastBufV[i][it];
      }
      mean = mean/Nx;
      for (it=0; it<Nx;  it++)  rms += pow( (double)BCU2MastBufV[i][it]-mean, 2);
      rms = sqrt (rms/Nx);

      plot->mean[iy] = mean;
      plot->rms[iy]  = rms;
      plot->amin[iy] = (double)BCU2MastAMin[i];
      plot->wmin[iy] = (double)BCU2MastWMin[i];
      plot->wmax[iy] = (double)BCU2MastWMax[i];
      plot->amax[iy] = (double)BCU2MastAMax[i];
      ymin = (double)BCU2MastAMin[i];
      ymax = (double)BCU2MastAMax[i];
      sprintf (temp,"%f",ymin);
      if (strcmp(temp,"-inf") ==0) ymin= mean- 2.*rms;
      sprintf (temp,"%f",ymax);
      if (strcmp(temp,"inf") ==0) ymax= mean+ 2.*rms;
      if (ymin > 0.) ymin=0.9*ymin;
      if (ymin < 0.) ymin=1.1*ymin;
      if (ymax > 0.) ymax=1.1*ymax;
      if (ymax < 0.) ymax=0.9*ymax;
      plot->ymin[iy] = ymin;
      plot->ymax[iy] = ymax;
      temp[0]='\0';
      //strcpy (temp, FamilyNames[BCU2MastInd[0]+i]);
      strcpy (temp, "Running mean, A");
      plot->ytitle[iy] = (QwtText) temp;

      //plot->backgr = QColor (255,210,210);
      //plot->backgr = QColor (255,240,240);
      plot->backgr = QColor (255,255,255);
      plot->setCaption ((QString) "BCU2  Master crate global currents");
      //plot->Title = (QwtText) FamilyNames[BCU2MastInd[0]+i];
      sprintf(temp,"%s     Mean= %6.2f",FamilyNames[BCU2MastInd[0]+i], mean);
      //sprintf(temp,"%s     Mean= %6.2f +- %6.2f",FamilyNames[BCU2MastInd[0]+i], (double)mean,(double)rms);
      plot->Title = (QwtText) temp;
      pen.setColor( QColor (0,0,0) );
      plot->pen[iy]=pen;
      plot->plotxy();
      plot->show();
      return;
   }
   if ( plot->Panel == "BCU2Tr" ) {
      for (it=0; it<Nx;  it++) {
          plot->x[it] = (double) timeB[it];
          plot->y[iy][it] = (double)BCU2TrBufV[i][it];
          mean += (double)BCU2TrBufV[i][it];
      }
      mean = mean/Nx;
      for (it=0; it<Nx;  it++)  rms += pow( (double)BCU2TrBufV[i][it]-mean, 2);
      rms = sqrt (rms/Nx);

      plot->mean[iy] = mean;
      plot->rms[iy]  = rms;
      plot->amin[iy] = (double)BCU2TrAMin[i];
      plot->wmin[iy] = (double)BCU2TrWMin[i];
      plot->wmax[iy] = (double)BCU2TrWMax[i];
      plot->amax[iy] = (double)BCU2TrAMax[i];
      ymin = (double)BCU2TrAMin[i];
      ymax = (double)BCU2TrAMax[i];
      sprintf (temp,"%f",ymin);
      if (strcmp(temp,"-inf") ==0) ymin= mean- 2.*rms;
      sprintf (temp,"%f",ymax);
      if (strcmp(temp,"inf") ==0) ymax= mean+ 2.*rms;
      if (ymin > 0.) ymin=0.9*ymin;
      if (ymin < 0.) ymin=1.1*ymin;
      if (ymax > 0.) ymax=1.1*ymax;
      if (ymax < 0.) ymax=0.9*ymax;
      plot->ymin[iy] = ymin;
      plot->ymax[iy] = ymax;
      temp[0]='\0';
      //strcpy (temp, FamilyNames[BCU2TrInd[0]+i]);
      if (i==1) strcat (temp, ", V");
      else strcat (temp, ", A");
      plot->ytitle[iy] = (QwtText) temp;

      //plot->backgr = QColor (255,210,210);
      //plot->backgr = QColor (255,240,240);
      plot->backgr = QColor (255,255,255);
      plot->setCaption ((QString) "BCU2  Thresholds");
      //plot->Title = (QwtText) FamilyNames[BCU2TrInd[0]+i];
      sprintf(temp,"%s     Mean= %6.2f",FamilyNames[BCU2TrInd[0]+i],mean);
      //sprintf(temp,"%s     Mean= %6.2f +- %6.2f",FamilyNames[BCU2TrInd[0]+i], (double)mean,(double)rms);
      plot->Title = (QwtText) temp;
      pen.setColor( QColor (0,0,0) );
      plot->pen[iy]=pen;
      plot->plotxy();
      plot->show();
      return;
   }
   if ( plot->Panel == "DSPTemp" ) {
      for (it=0; it<Nx;  it++) {
          plot->x[it] = (double) timeB[it];
          plot->y[iy][it] = (double)DSPTempBufV[ICr3][IBo][i][it];
          mean += (double)DSPTempBufV[ICr3][IBo][i][it];
      }
      mean = mean/Nx;
      for (it=0; it<Nx;  it++)  rms += pow( (double)DSPTempBufV[ICr3][IBo][i][it]-mean, 2);
      rms = sqrt (rms/Nx);

      plot->mean[iy] = mean;
      plot->rms[iy]  = rms;
      plot->amin[iy] = (double)DSPTempAMin[ICr3][IBo][i];
      plot->wmin[iy] = (double)DSPTempWMin[ICr3][IBo][i];
      plot->wmax[iy] = (double)DSPTempWMax[ICr3][IBo][i];
      plot->amax[iy] = (double)DSPTempAMax[ICr3][IBo][i];
      ymin = (double)DSPTempAMin[ICr3][IBo][i];
      ymax = (double)DSPTempAMax[ICr3][IBo][i];
      sprintf (temp,"%f",ymin);
      if (strcmp(temp,"-inf") ==0) ymin= mean- 2.*rms;
      sprintf (temp,"%f",ymax);
      if (strcmp(temp,"inf") ==0) ymax= mean+ 2.*rms;
      if (ymin > 0.) ymin=0.9*ymin;
      if (ymin < 0.) ymin=1.1*ymin;
      if (ymax > 0.) ymax=1.1*ymax;
      if (ymax < 0.) ymax=0.9*ymax;
      plot->ymin[iy] = ymin;
      plot->ymax[iy] = ymax;
      temp[0]='\0';
      //strcpy (temp, FamilyNames[DSPTempInd[0]+i]);
      strcpy (temp, "Running mean, C");
      plot->ytitle[iy] = (QwtText) temp;

      //plot->backgr = QColor (255,210,210);
      //plot->backgr = QColor (255,240,240);
      plot->backgr = QColor (255,255,255);
      //sprintf(temp,"DSP Temperatures:    Crate=%d",ICr3);
      //plot->setCaption ((QString) temp);
      plot->setCaption ((QString) "DSP  Temperatures");
      //plot->Title = (QwtText) FamilyNames[DSPTempInd[0]+i];
      sprintf(temp,"Crate=%d Board=%d  %s   Mean= %6.2f",ICr3,IBo,FamilyNames[DSPTempInd[0]+i],mean);
      //sprintf(temp,"Crate=%d Board=%d  %s   Mean= %6.2f +- %6.2f",ICr3,IBo,FamilyNames[DSPTempInd[0]+i], (double)mean,(double)rms);
      plot->Title = (QwtText) temp;
      pen.setColor( QColor (0,0,0) );
      plot->pen[iy]=pen;
      plot->plotxy();
      plot->show();
      return;
   }
   if ( plot->Panel == "DSPCur" ) {
      for (it=0; it<Nx;  it++) {
          plot->x[it] = (double) timeB[it];
          plot->y[iy][it] = (double)DSPCurBufV[ICr3][IBo][i][it];
          mean += (double)DSPCurBufV[ICr3][IBo][i][it];
      }
      mean = mean/Nx;
      for (it=0; it<Nx;  it++)  rms += pow( (double)DSPCurBufV[ICr3][IBo][i][it]-mean, 2);
      rms = sqrt (rms/Nx);

      plot->mean[iy] = mean;
      plot->rms[iy]  = rms;
      plot->amin[iy] = (double)DSPCurAMin[ICr3][IBo][i];
      plot->wmin[iy] = (double)DSPCurWMin[ICr3][IBo][i];
      plot->wmax[iy] = (double)DSPCurWMax[ICr3][IBo][i];
      plot->amax[iy] = (double)DSPCurAMax[ICr3][IBo][i];
      ymin = (double)DSPCurAMin[ICr3][IBo][i];
      ymax = (double)DSPCurAMax[ICr3][IBo][i];
      sprintf (temp,"%f",ymin);
      if (strcmp(temp,"-inf") ==0) ymin= mean- 2.*rms;
      sprintf (temp,"%f",ymax);
      if (strcmp(temp,"inf") ==0) ymax= mean+ 2.*rms;
      if (ymin > 0.) ymin=0.9*ymin;
      if (ymin < 0.) ymin=1.1*ymin;
      if (ymax > 0.) ymax=1.1*ymax;
      if (ymax < 0.) ymax=0.9*ymax;
      plot->ymin[iy] = ymin;
      plot->ymax[iy] = ymax;
      temp[0]='\0';
      //strcpy (temp, FamilyNames[DSPCurInd[0]+i]);
      strcpy (temp, "Running mean, A");
      plot->ytitle[iy] = (QwtText) temp;

      //plot->backgr = QColor (255,210,210);
      //plot->backgr = QColor (255,240,240);
      plot->backgr = QColor (255,255,255);
      //sprintf(temp,"DSP Temperatures:    Crate=%d",ICr3);
      //plot->setCaption ((QString) temp);
      plot->setCaption ((QString) "DSP  Coil Currents (SPI)");
      //plot->Title = (QwtText) FamilyNames[DSPCurInd[0]+i];
      sprintf(temp,"Crate=%d Board=%d Ch=%d %s   Mean= %6.2f",ICr3,IBo,i,FamilyNames[DSPCurInd[0]],mean);
      //sprintf(temp,"Crate=%d Board=%d Ch=%d %s   Mean= %6.2f +- %6.2f",ICr3,IBo,i,FamilyNames[DSPCurInd[0]], (double)mean,(double)rms);
      plot->Title = (QwtText) temp;
      pen.setColor( QColor (0,0,0) );
      plot->pen[iy]=pen;
      plot->plotxy();
      plot->show();
      return;
   }
}

void GUILBT672::remove_plot (int it)
{
   int i,n;
   PlotForm* plot;

   vector<PlotForm*>::iterator p = pplot.begin();
   p +=it;
   pplot.erase(p);

   n = pplot.size();
   if (n<=0) return;

   for (i=0; i<n; i++) {
       plot = pplot[i];
       plot->inVector = i;
   } 
}

void GUILBT672::refreshPlots()
{
   int i,n;
   PlotForm* plot;

   n = pplot.size();
   if (n<=0) return;

   for (i=0; i<n; i++) {
       plot = pplot[i];
       //printf("----Nb=%d NBuf=%d\n",Nb,NBuf);
       plot->Nx = Nb;
       plot->inVector = i;
       if (plot->Refresh) fillPlot (plot);
   } 
}

void GUILBT672::Refresh()
{
 //qWarning( "GUILBT672::Refresh: entered" );
     if ( TimeToDie() ) quit(); //to support AOApp signals
     if (Init==0) GetLimits();
     GetAll();
     GetAllAdam();
     SetAll();
     refreshPlots();
     if (iLong == 0) {
        PlotSysLeft();
        PlotSysRight();
        PlotBCU1ld();
        //PlotBCU1mu();
        PlotBCU1md();
        PlotBCU1rd();
        PlotBCU2ld();
        PlotBCU2ru();
     }
     //start timer if the Housekeeper interrogation was NOT Stopped due to an error
     //remember that FIRST interrogation was made out of the timer loop
     if ( QString::compare(buttonStart->text(), "&Stop") == 0 &&  !timer->isActive() ) timer->start (timeoutR,FALSE);
}



// filter for specially treated events
// now only ENTER & LEAVE events for certain labels are used to show the limits set for 
// the corresponding values
bool GUILBT672::eventFilter( QObject *obj, QEvent *ev )
{
   int i;

   //Description of ADAM values
  /* for (i=0; i<14; i++) {
       if ( obj == DiagnAdam[i] ) {
          if (ev->type() == QEvent::Enter ) {
             wrangeLabel->setPaletteBackgroundColor( QColor( 0,150,255 ) );
             wrangeLabel->setText(tr(AdamExpl[i]));
             return TRUE;
          }
          if (ev->type() == QEvent::Leave ) {
             SetRangeLabel ((string)"dummy",0);
             return TRUE;
          }
          return FALSE;
       }
   }*/
   //limits in System Summary Environment panel
   for (i=0; i<9; i++) {
       if ( obj == ExternalLab[i] ) {
          if (ev->type() == QEvent::Enter ) {
             SetRangeLabel ((string)"SumEnv",i);
             return TRUE;
          }
          if (ev->type() == QEvent::Leave ) {
             SetRangeLabel ((string)"dummy",0);
             return TRUE;
          }
          return FALSE;
       }
   }

   //limits in BCU1  panels
   for (i=0; i<6; i++) {
       if ( obj == BCU1EnvLab[i] ) {
          if (ev->type() == QEvent::Enter ) {
             SetRangeLabel ((string)"BCU1Env",i);
             return TRUE;
          }
          if (ev->type() == QEvent::Leave ) {
             SetRangeLabel ((string)"dummy",0);
             return TRUE;
          }
          return FALSE;
       }
   }
   for (i=0; i<2; i++) {
       if ( obj == BCU1TempLab[i] ) {
          if (ev->type() == QEvent::Enter ) {
             SetRangeLabel ((string)"BCU1Temp",i);
             return TRUE;
          }
          if (ev->type() == QEvent::Leave ) {
             SetRangeLabel ((string)"dummy",0);
             return TRUE;
          }
          return FALSE;
       }
   }
   for (i=0; i<10; i++) {
       if ( obj == BCU1VolLab[i] ) {
          if (ev->type() == QEvent::Enter ) {
             SetRangeLabel ((string)"BCU1Vol",i);
             return TRUE;
          }
          if (ev->type() == QEvent::Leave ) {
             SetRangeLabel ((string)"dummy",0);
             return TRUE;
          }
          return FALSE;
       }
   }
   for (i=0; i<3; i++) {
       if ( obj == BCU1SGNTempLab[i] ) {
          if (ev->type() == QEvent::Enter ) {
             SetRangeLabel ((string)"BCU1SGNTemp",i);
             return TRUE;
          }
          if (ev->type() == QEvent::Leave ) {
             SetRangeLabel ((string)"dummy",0);
             return TRUE;
          }
          return FALSE;
       }
   }

   //limits in BCU2  panels
   for (i=0; i<3; i++) {
       if ( obj == BCU2MastLab[i] ) {
          if (ev->type() == QEvent::Enter ) {
             SetRangeLabel ((string)"BCU2Mast",i);
             return TRUE;
          }
          if (ev->type() == QEvent::Leave ) {
             SetRangeLabel ((string)"dummy",0);
             return TRUE;
          }
          return FALSE;
       }
   }
   for (i=0; i<4; i++) {
       if ( obj == BCU2TrLab[i] ) {
          if (ev->type() == QEvent::Enter ) {
             SetRangeLabel ((string)"BCU2Tr",i);
             return TRUE;
          }
          if (ev->type() == QEvent::Leave ) {
             SetRangeLabel ((string)"dummy",0);
             return TRUE;
          }
          return FALSE;
       }
   }

   //limits in DSP  panels
   for (i=0; i<8; i++) {
       if ( obj == DSPCurLab[i] ) {
          if (ev->type() == QEvent::Enter ) {
             SetRangeLabel ((string)"DSPCur",i);
             return TRUE;
          }
          if (ev->type() == QEvent::Leave ) {
             SetRangeLabel ((string)"dummy",0);
             return TRUE;
          }
          return FALSE;
       }
   }
   for (i=0; i<4; i++) {
       if ( obj == DSPTempLab[i] ) {
          if (ev->type() == QEvent::Enter ) {
             SetRangeLabel ((string)"DSPTemp",i);
             return TRUE;
          }
          if (ev->type() == QEvent::Leave ) {
             SetRangeLabel ((string)"dummy",0);
             return TRUE;
          }
          return FALSE;
       }
   }
   for (i=0; i<6; i++) {
       if ( obj == DSPadLab[i] ) {
          if (ev->type() == QEvent::Enter ) {
             SetRangeLabel ((string)"DSPad",i);
             return TRUE;
          }
          if (ev->type() == QEvent::Leave ) {
             SetRangeLabel ((string)"dummy",0);
             return TRUE;
          }
          return FALSE;
       }
   }

   //support for PLOTs
   if (ev->type() == QEvent::MouseButtonPress ) {
      for (i=0; i<9; i++) {        //System summary
          if ( obj == ExternalBut[i] ) {
             if (ExternalEn[i] != 1) return FALSE;
             newPlot ((string)"External",i);
             return TRUE;
          }
      }
      for (i=0; i<6; i++) {       //BCU1
          if ( obj == BCU1EnvBut[i] ) {
             if (BCU1EnvEn[ICr1][i] != 1) return FALSE;
             newPlot ((string)"BCU1Env",i);
             return TRUE;
          }
      }
      for (i=0; i<2; i++) {
          if ( obj == BCU1TempBut[i] ) {
             if (BCU1TempEn[ICr1][i] != 1) return FALSE;
             newPlot ((string)"BCU1Temp",i);
             return TRUE;
          }
      }
      for (i=0; i<10; i++) {
          if ( obj == BCU1VolBut[i] ) {
             if (BCU1VolEn[ICr1][i] != 1) return FALSE;
             newPlot ((string)"BCU1Vol",i);
             return TRUE;
          }
      }
      for (i=0; i<3; i++) {
          if ( obj == BCU1SGNTempBut[i] ) {
             if (BCU1SGNTempEn[ICr1][i] != 1) return FALSE;
             newPlot ((string)"BCU1SGNTemp",i);
             return TRUE;
          }
      }
      for (i=0; i<3; i++) {       //BCU2
             if (BCU2MastEn[i] != 1) return FALSE;
          if ( obj == BCU2MastBut[i] ) {
             newPlot ((string)"BCU2Mast",i);
             return TRUE;
          }
      }
      for (i=0; i<4; i++) {
          if ( obj == BCU2TrBut[i] ) {
             if (BCU2TrEn[i] != 1) return FALSE;
             newPlot ((string)"BCU2Tr",i);
             return TRUE;
          }
      }
      for (i=0; i<8; i++) {      //DSP
          if ( obj == DSPCurBut[i] ) {
             if (DSPCurEn[ICr3][IBo][i] != 1) return FALSE;
             newPlot ((string)"DSPCur",i);
             return TRUE;
          }
      }
      for (i=0; i<4; i++) {
          if ( obj == DSPTempBut[i] ) {
             if (DSPTempEn[ICr3][IBo][i] != 1) return FALSE;
             newPlot ((string)"DSPTemp",i);
             return TRUE;
          }
      }
   }
   return FALSE;
}


void GUILBT672::SetRangeLabel( string s, int j)  //show limits
{
  char temp[300],temp1[300];

  if ( s == "dummy" )  {
     sprintf (temp, "                                      ");
     sprintf (temp1,"                                      ");
     wrangeLabel->setPaletteBackgroundColor( QColor( 192,192,192 ) );
     arangeLabel->setPaletteBackgroundColor( QColor( 192,192,192 ) );
  } else {
     wrangeLabel->setPaletteBackgroundColor( QColor( 200,200,0 ) );
     arangeLabel->setPaletteBackgroundColor( QColor( 200,0,0 ) );
  }

  if ( s == "SumEnv" ) {
     sprintf (temp, "Warning Min,Max: %10.3f, %10.3f",ExternalWMin[j],ExternalWMax[j]);
     sprintf (temp1,"Alarm   Min,Max: %10.3f, %10.3f",ExternalAMin[j],ExternalAMax[j]);
  }
  if ( s == "BCU1Env" && ICr1 <6 ) {
     sprintf (temp, "Warning Min,Max: %10.3f, %10.3f",BCU1EnvWMin[ICr1][j],BCU1EnvWMax[ICr1][j]);
     sprintf (temp1,"Alarm   Min,Max: %10.3f, %10.3f",BCU1EnvAMin[ICr1][j],BCU1EnvAMax[ICr1][j]);
  }
  if ( s == "BCU1Env" && ICr1 ==6 ) {
     sprintf (temp, "                                      ");
     sprintf (temp1,"                                      ");
     wrangeLabel->setPaletteBackgroundColor( QColor( 192,192,192 ) );
     arangeLabel->setPaletteBackgroundColor( QColor( 192,192,192 ) );
  }
  if ( s == "BCU1Vol" && ICr1 <6 ) {
     sprintf (temp, "Warning Min,Max: %10.3f, %10.3f",BCU1VolWMin[ICr1][j],BCU1VolWMax[ICr1][j]);
     sprintf (temp1,"Alarm   Min,Max: %10.3f, %10.3f",BCU1VolAMin[ICr1][j],BCU1VolAMax[ICr1][j]);
  }
  if ( s == "BCU1Vol" && ICr1 ==6 ) {
     sprintf (temp, "                                      ");
     sprintf (temp1,"                                      ");
     wrangeLabel->setPaletteBackgroundColor( QColor( 192,192,192 ) );
     arangeLabel->setPaletteBackgroundColor( QColor( 192,192,192 ) );
  }
  if ( s == "BCU1Temp" ) {
     sprintf (temp, "Warning Min,Max: %10.3f, %10.3f",BCU1TempWMin[ICr1][j],BCU1TempWMax[ICr1][j]);
     sprintf (temp1,"Alarm   Min,Max: %10.3f, %10.3f",BCU1TempAMin[ICr1][j],BCU1TempAMax[ICr1][j]);
  }
  if ( s == "BCU1SGNTemp" && ICr1 <6 ) {
     sprintf (temp, "Warning Min,Max: %10.3f, %10.3f",BCU1SGNTempWMin[ICr1][j],BCU1SGNTempWMax[ICr1][j]);
     sprintf (temp1,"Alarm   Min,Max: %10.3f, %10.3f",BCU1SGNTempAMin[ICr1][j],BCU1SGNTempAMax[ICr1][j]);
  }
  if ( s == "BCU1SGNTemp" && ICr1 ==6 ) {
     sprintf (temp, "                                      ");
     sprintf (temp1,"                                      ");
     wrangeLabel->setPaletteBackgroundColor( QColor( 192,192,192 ) );
     arangeLabel->setPaletteBackgroundColor( QColor( 192,192,192 ) );
  }
  if ( s == "BCU2Mast" ) {
     sprintf (temp, "Warning Min,Max: %10.3f, %10.3f",BCU2MastWMin[j],BCU2MastWMax[j]);
     sprintf (temp1,"Alarm   Min,Max: %10.3f, %10.3f",BCU2MastAMin[j],BCU2MastAMax[j]);
  }
  if ( s == "BCU2Tr" ) {
     sprintf (temp, "Warning Min,Max: %10.3f, %10.3f",BCU2TrWMin[j],BCU2TrWMax[j]);
     sprintf (temp1,"Alarm   Min,Max: %10.3f, %10.3f",BCU2TrAMin[j],BCU2TrAMax[j]);
  }

  if ( s == "DSPCur" ) {
     sprintf (temp, "Warning Min,Max: %10.3f, %10.3f",DSPCurWMin[ICr3][IBo][j],DSPCurWMax[ICr3][IBo][j]);
     sprintf (temp1,"Alarm   Min,Max: %10.3f, %10.3f",DSPCurAMin[ICr3][IBo][j],DSPCurAMax[ICr3][IBo][j]);
  }
  if ( s == "DSPTemp" ) {
     sprintf (temp, "Warning Min,Max: %10.3f, %10.3f",DSPTempWMin[ICr3][IBo][j],DSPTempWMax[ICr3][IBo][j]);
     sprintf (temp1,"Alarm   Min,Max: %10.3f, %10.3f",DSPTempAMin[ICr3][IBo][j],DSPTempAMax[ICr3][IBo][j]);
  }
  if ( s == "DSPad" )  {
     sprintf (temp, "Warning Min,Max: %10.3f, %10.3f",DSPadWMin[ICr3][IBo][ICh][j],DSPadWMax[ICr3][IBo][ICh][j]);
     sprintf (temp1,"Alarm   Min,Max: %10.3f, %10.3f",DSPadAMin[ICr3][IBo][ICh][j],DSPadAMax[ICr3][IBo][ICh][j]);
  }
  wrangeLabel->setText( temp );
  arangeLabel->setText( temp1);
}


/*  Get Limits of Housekeeper variables *************************************/
void GUILBT672::GetLimits()
{

    //char *diagnapp = (char *)"HOUSEKPR00";
    string _diagnapp = std::string("housekeeper.") + Side();
    char *diagnapp = (char *)_diagnapp.c_str();

    char *family = (char *)"*";
    int   from = -1;
    int   to   = -1;
    int   timeout = 1000; // timeout in ms;
    int   enabled;
    ParamDict dict;

    string Family;
    const char *familyName;
    int   Index;
    int i,j, icr,ibo,ich;

    int jj=0;
    try{
        DiagnWhich which(family, from, to);
        dict = GetDiagnVarParam( diagnapp,which, timeout);
    } catch (AOException &e) {
            if (Init==0) QMessageBox::information(this,"GUILBT672::GetLimits()",
            "Unable to get the Warning&Alarm Range\nContinue without this information");
            return;
            _logger->log(Logger::LOG_LEV_ERROR, "Error occurred %s [%s:%d]", e.what().c_str(), __FILE__, __LINE__);

//        return e._errcode;
    }
//qWarning( "GUILBT672::GetLimits(): GetDiagnParam " );

   //Iterate over the dictionary of the retrieved DiagnVars.
   for (ParamDict::iterator it = dict.begin(); it != dict.end();  it++){
        jj++;
//printf("-----it= %i\n",jj);
        DiagnWhich wh = (*it).first;
        DiagnParam pa = (*it).second;

        //DiagnVar name & index
        Family = wh.Family();   // DiagnVar name.
        familyName = Family.c_str();
//printf("-----%s-\n",familyName);
        Index  = wh.From();     // DiagnVar index
        enabled = pa.isEnabled();
        DiagnRange<float> va = pa.Range();
        for (i=0; i<nfamily; i++) {
//printf(".....-----i= %i %s=%s\n",i,FamilyNames[i],familyName);

            if (strcmp(familyName,FamilyNames[i]) == 0) {  //was FamilyNames
//if (i>112) printf("found-----i= %i %s=%s\n",i,FamilyNames[i],familyName);

               float WMin= va.WarningMin();
               float WMax= va.WarningMax();
               float AMin= va.AlarmMin();
               float AMax= va.AlarmMax();
//if (i>112) printf("wmin==%f wmax=%f amin=%f amax=%f\n",WMin,WMax,AMin,AMax);
               //Index = icrate 
               //Index = icrate * BONUM + iboard  , if applied
               //Index = icrate * BONUM*CHNUM + iboard * CHNUM + ichannel , if applied
               if (i<7) {
                  DiagAppDataWMin[i]  =WMin;
                  DiagAppDataWMax[i]  =WMax;
                  DiagAppDataAMin[i]  =AMin;
                  DiagAppDataAMax[i]  =AMax;
                  DiagAppDataEn[i]  = enabled;
                  break;
               }
               if (i>6 && i<11) {
                  j=i-7;
                  BCU2TrWMin[j]  =WMin; 
                  BCU2TrWMax[j]  =WMax; 
                  BCU2TrAMin[j]  =AMin; 
                  BCU2TrAMax[j]  =AMax; 
                  BCU2TrEn[j]  =enabled;
                  break;
               }
               if (i>10 && i<22) {
                  icr=Index;
                  j=i-11;
                  BCU1NiosWMin[icr][j]  =WMin; 
                  BCU1NiosWMax[icr][j]  =WMax; 
                  BCU1NiosAMin[icr][j]  =AMin; 
                  BCU1NiosAMax[icr][j]  =AMax; 
                  BCU1NiosEn[icr][j]  =enabled;
                  break;
               }
               if (i>21 && i<33) {
                  icr=6;  //switch BCU
                  j=i-22;
                  BCU1NiosWMin[icr][j]  =WMin;
                  BCU1NiosWMax[icr][j]  =WMax;
                  BCU1NiosAMin[icr][j]  =AMin;
                  BCU1NiosAMax[icr][j]  =AMax;
                  BCU1NiosEn[icr][j]  =enabled;
                  break;
               }
               if (i>32 && i<41) {
                  j=i-33;
                  ich=Index % CHNUM;     //channel number , complete channel blocks ignored
                  j=Index / CHNUM;       // = icr *BONUM + ibo
                  ibo=j % BONUM;         //board number, complete board blocks ignored
                  icr=j / BONUM;         //number of complete board blocks == icr
                  DSPNiosWMin[icr][ibo][j]  =WMin; 
                  DSPNiosWMax[icr][ibo][j]  =WMax; 
                  DSPNiosAMin[icr][ibo][j]  =AMin; 
                  DSPNiosAMax[icr][ibo][j]  =AMax; 
                  DSPNiosEn[icr][ibo][j]  =enabled;
                  break;
               }
               if (i>40 && i<47) {
                  ich=Index % CHNUM;     //channel number , complete channel blocks ignored
                  j=Index / CHNUM;       // = icr *BONUM + ibo
                  ibo=j % BONUM;         //board number, complete board blocks ignored
                  icr=j / BONUM;         //number of complete board blocks == icr
                  j=i-41;
//printf("-----j= %i icr=%i ibo=%i ich=%i mean=%f std=%f stat=%i\n",j,icr,ibo,ich,mean,std,stat);
                  DSPadWMin[icr][ibo][ich][j]  =WMin; 
                  DSPadWMax[icr][ibo][ich][j]  =WMax; 
                  DSPadAMin[icr][ibo][ich][j]  =AMin; 
                  DSPadAMax[icr][ibo][ich][j]  =AMax; 
                  DSPadEn[icr][ibo][ich][j]  =enabled;
                  break;
               }
         /*      if (i>46 && i<51) {
                  j=i-47;
                  BCU2SGNVal[j]  = mean;
                  BCU2SGNStd[j]  = std;
                  BCU2SGNStat[j] = stat;
                  break;
               }     */
               if (i>50 && i<54) {
                  icr=Index;
                  j=i-51;
                  BCU1SGNTempWMin[icr][j]  =WMin; 
                  BCU1SGNTempWMax[icr][j]  =WMax; 
                  BCU1SGNTempAMin[icr][j]  =AMin; 
                  BCU1SGNTempAMax[icr][j]  =AMax; 
                  BCU1SGNTempEn[icr][j]  =enabled;
                  break;
               }
               if (i>53 && i<58) {
                  ibo=Index % BONUM;        //board number, complete board blocks ignored
                  icr=Index / BONUM;         //number of complete board blocks == icr
                  j=i-54;
                  DSPTempWMin[icr][ibo][j]  =WMin;
                  DSPTempWMax[icr][ibo][j]  =WMax;
                  DSPTempAMin[icr][ibo][j]  =AMin;
                  DSPTempAMax[icr][ibo][j]  =AMax;
                  DSPTempEn[icr][ibo][j]  =enabled;
                  break;
               }
               if (i==58) {
                  ich=Index % CHNUM;     //channel number , complete channel blocks ignored
                  j=Index / CHNUM;       // = icr *BONUM + ibo
                  ibo=j % BONUM;         //board number, complete board blocks ignored
                  icr=j / BONUM;         //number of complete board blocks == icr
                  DSPCurWMin[icr][ibo][ich]  =WMin;
                  DSPCurWMax[icr][ibo][ich]  =WMax;
                  DSPCurAMin[icr][ibo][ich]  =AMin;
                  DSPCurAMax[icr][ibo][ich]  =AMax;
                  DSPCurEn[icr][ibo][ich]  =enabled;
                  break;
               }
               if (i>58 && i<61) {
                  icr=6;  //switch BCU
                  j=i-59;
                  //BCU1TempVal[icr][j]  = mean;
                  //BCU1TempStd[icr][j]  = std;
                  //BCU1TempStat[icr][j] = stat; 
                  BCU1TempWMin[icr][j]  =WMin;
                  BCU1TempWMax[icr][j]  =WMax;
                  BCU1TempAMin[icr][j]  =AMin;
                  BCU1TempAMax[icr][j]  =AMax;
                  BCU1TempEn[icr][j] =enabled;
                  break;
               }   
               if (i>60 && i<63) {
                  icr=Index;
                  j=i-61;
                  BCU1TempWMin[icr][j]  =WMin;
                  BCU1TempWMax[icr][j]  =WMax;
                  BCU1TempAMin[icr][j]  =AMin;
                  BCU1TempAMax[icr][j]  =AMax;
                  BCU1TempEn[icr][j]  =enabled;
                  break;
               }
               if (i>62 && i<73) {
                  icr=Index;
                  j=i-63;
                  BCU1VolWMin[icr][j]  =WMin;
                  BCU1VolWMax[icr][j]  =WMax;
                  BCU1VolAMin[icr][j]  =AMin;
                  BCU1VolAMax[icr][j]  =AMax;
                  BCU1VolEn[icr][j]  =enabled;
                  break;
               }
               if (i>72 && i<79) {
                  icr=Index;
                  j=i-73;
//printf("-----j= %i icr=%i mean=%f std=%f stat=%i\n",j,icr,mean,std,stat);
                  BCU1EnvWMin[icr][j]  =WMin;
                  BCU1EnvWMax[icr][j]  =WMax;
                  BCU1EnvAMin[icr][j]  =AMin;
                  BCU1EnvAMax[icr][j]  =AMax;
                  BCU1EnvEn[icr][j]  =enabled;
                  break;
               }
               if (i>78 && i<82) {
                  j=i-79;
                  BCU2MastWMin[j]  =WMin;
                  BCU2MastWMax[j]  =WMax;
                  BCU2MastAMin[j]  =AMin;
                  BCU2MastAMax[j]  =AMax;
                  BCU2MastEn[j]  =enabled;
                  break;
               }
               if (i>81 && i<92) {
                  icr=Index;
                  j=i-82;
                  //BCU2DigStat[icr][j] = stat;
                  BCU2DigEn[icr][j] =enabled;
                  break;
               }
               if (i>91 && i<102) {
                  icr=Index;
                  j=i-92;
                  //BCU2ResetStat[icr][j] = stat;
                  BCU2ResetEn[icr][j] = enabled;
                  break;
               }
               if (i>101 && i<112) {
                  ibo=Index % BONUM;         //board number, complete board blocks ignored
                  icr=Index / BONUM;         //number of complete board blocks == icr
                  j=i-102;
                  //DSPDrSStat[icr][ibo][j] = stat;
                  DSPDrSEn[icr][ibo][j] =enabled;
                  break;
               }     
               if (i>112 && i<122) {
                  j=i-113;
                  ExternalWMin[j]  =WMin;
                  ExternalWMax[j]  =WMax;
                  ExternalAMin[j]  =AMin;
                  ExternalAMax[j]  =AMax;
                  ExternalEn[j]  =enabled;
//printf("j=%i wmin==%f wmax=%f amin=%f amax=%f\n",j,WMin,WMax,AMin,AMax);
                  break;
               }
               if (i>121 && i<127) {
                  j=i-122;
                  GeneralWMin[j]  =WMin;
                  GeneralWMax[j]  =WMax;
                  GeneralAMin[j]  =AMin;
                  GeneralAMax[j]  =AMax;
                  GeneralEn[j]  =enabled;
//printf("j=%i wmin==%f wmax=%f amin=%f amax=%f\n",j,WMin,WMax,AMin,AMax);
                  break;
               }
 
            }
        }
//        _logger->log(Logger::LOG_LEV_DEBUG, "[GetDiagnVarParam] %s  %s --- %s [%s:%d]",
//                diagnapp, wh.str().c_str(), va.str().c_str(), __FILE__,__LINE__);
    }
}

void GUILBT672::shiftBuf(double *buf, int n)
{
   if ( n<2 ) return;
   for ( int i=0; i<n-1; i++) buf[i]=buf[i+1];
   buf[n-1]=0.;
}

double GUILBT672::meanBuf(double *buf, int n)
{
   double mean=0.;
   for ( int i=n; i>n-NLong; i--) mean += buf[i];
   mean = mean/NLong;
   return mean;
}

/*  Get All Housekeeper variables *************************************/
void GUILBT672::GetAll()
{
 
    QTime qtime = QTime::currentTime();
    QString stime = qtime.toString(Qt::LocalDate);
    Clock->setText(tr(stime));

    //char *diagnapp = (char *)"HOUSEKPR00";
    string _diagnapp = std::string("housekeeper.") + Side();
    char *diagnapp = (char *)_diagnapp.c_str();

    char *family = (char *)"*";
    int   from = -1;
    int   to   = -1;
    int   timeout = 1000; // timeout in ms;
    ValueDict dict;

    string Family;
    const char *familyName;
    int   Index;
    int i,j,stat, icr,ibo,ich;
    double mean,std;
    char temp[100];

//qWarning( "GUILBT672::GetAll(): entered" );
    int jj=0;


    if (Init!=0) {
       if (Nb >=NBuf) {
          shiftBuf ( timeB, NBuf );
          for (i=0; i<10; i++) {
               if (i<9) shiftBuf ( ExternalBufV[i], NBuf );
               if (i<4) shiftBuf ( BCU2TrBufV[i], NBuf );
               if (i<3) shiftBuf ( BCU2MastBufV[i], NBuf );
               for (int icr=0; icr<CRNUM; icr++) {
                   shiftBuf ( BCU1VolBufV[icr][i], NBuf );
                   if (i<6) shiftBuf ( BCU1EnvBufV[icr][i], NBuf );
                   if (i<2) shiftBuf ( BCU1TempBufV[icr][i], NBuf );
                   if (i<3) shiftBuf ( BCU1SGNTempBufV[icr][i], NBuf );
                   for (int ibo=0; ibo<BONUM; ibo++) {
                       if (i<8) shiftBuf ( DSPCurBufV[icr][ibo][i], NBuf );
                       if (i<4) shiftBuf ( DSPTempBufV[icr][ibo][i], NBuf );
                   }
               }
          }
          Nb=NBuf-1;
       }
       if (Nbl >=NBufL) {
          shiftBuf ( timeL, NBufL );
          for (i=0; i<10; i++) {
               if (i<9) shiftBuf ( ExternalBufVL[i], NBufL );
               if (i<4) shiftBuf ( BCU2TrBufVL[i], NBufL );
               if (i<3) shiftBuf ( BCU2MastBufVL[i], NBufL );
               for (int icr=0; icr<CRNUM; icr++) {
                   shiftBuf ( BCU1VolBufVL[icr][i], NBufL );
                   if (i<6) shiftBuf ( BCU1EnvBufVL[icr][i], NBufL );
                   if (i<2) shiftBuf ( BCU1TempBufVL[icr][i], NBufL );
                   if (i<3) shiftBuf ( BCU1SGNTempBufVL[icr][i], NBufL );
                   for (int ibo=0; ibo<BONUM; ibo++) {
                       if (i<8) shiftBuf ( DSPCurBufVL[icr][ibo][i], NBufL );
                       if (i<4) shiftBuf ( DSPTempBufVL[icr][ibo][i], NBufL );
                   }
               }
          }
          Nbl=NBufL-1;
       }
    } else {
      srand(1);
      Init=1;
      Nb=0;
      Nbl=0;
   }

    try{
        DiagnWhich which(family, from, to);
        dict = GetDiagnValue( diagnapp,which, timeout);
    } catch (AOException &e) {
            if (!Dummy) {
               timer->stop();
               buttonStart->setText( tr( "&Start" ) );
               buttonStart->setPaletteBackgroundColor( QColor( 200, 0, 0 ) );
               QMessageBox::information(this,"GUILBT672::GetAll()",
                "Unable to get the DiagnValues\nThe Housekeeper interrogation is STOPPED");
               //Dummy = DUMMY;
            }
//qWarning( "GUILBT672::GetAll(): error" );
            if (Dummy) GetAllDummy();
            return;
//        _logger->log(Logger::LOG_LEV_ERROR, e.what());
//        return e._errcode;
    } 

   //Iterate over the dictionary of the retrieved DiagnVars.
   for (ValueDict::iterator it = dict.begin(); it != dict.end();  it++){
        jj++;
//printf("GETALL-----it= %i\n",jj);
        DiagnWhich wh = (*it).first;
        DiagnValue va = (*it).second;

        //DiagnVar name & index
        Family = wh.Family();   // DiagnVar name.
        familyName = Family.c_str();
        //_logger->log(Logger::LOG_LEV_INFO, "Family %s",familyName);
        timeB[Nb] = va.Time();   //accumulate current value time  in sec?

//printf("-----%s-\n",familyName);
        for (i=0; i<nfamily; i++) {
//printf(".....-----i= %i %s=%s\n",i,FamilyNames[i],familyName);
//_logger->log(Logger::LOG_LEV_INFO, "Family is %s",FamilyNames[i]);//mod RB
            if (strcmp(familyName,FamilyNames[i]) == 0) {
//printf("found-----i= %i %s=%s\n",i,FamilyNames[i],familyName);
               Index  = wh.From();     // DiagnVar index
               //Index = icrate 
               //Index = icrate * BONUM + iboard  , if applied
               //Index = icrate * BONUM*CHNUM + iboard * CHNUM + ichannel , if applied
               // Values
               stat = va.Status();   // Status: 0=OK, 1=Warning, 2=Error
               mean =  va.Mean();     // DiagnVar value (average of a running mean)
               sprintf (temp,"%f",mean);
               if (strcmp(temp,"nan")==0) mean = 0.;
               std  =  va.Stddev();   // Stddev of the running mean
               sprintf(temp,"%f",std);
               if (strcmp(temp,"nan")==0) std = 0.;

               //va.Last();     // Instantaneuos value (last value inserted in the running mean)
               //va.Time();     // Time (average of the time stamp of the values in the running mean)
//_logger->log(Logger::LOG_LEV_INFO, "Family is %s",familyName);//mod RB
               //if (i<7) {  original
               //if (i>4 && i<7) { //mod RB
               if (i<2){
                  DiagAppDataVal[i]  = mean;
                  DiagAppDataStd[i]  = std;
                  DiagAppDataStat[i] = stat;
                  break;
               }
               if (i>6 && i<11) {
                  j=i-7;
                  BCU2TrVal[j]  = mean;
                  BCU2TrStd[j]  = std;
                  BCU2TrStat[j] = stat;
                  BCU2TrBufV[j][Nb] = mean;
                  break;
               }
               if (i>10 && i<22) {
                  icr=Index;
                  j=i-11;
                  BCU1NiosVal[icr][j]  = mean;
                  BCU1NiosStd[icr][j]  = std;
                  BCU1NiosStat[icr][j] = stat;
                  break;
               }
               if (i>21 && i<33) {
                  icr=6;  //switch BCU
                  j=i-22;
                  BCU1NiosVal[icr][j]  = mean;
                  BCU1NiosStd[icr][j]  = std;
                  BCU1NiosStat[icr][j] = stat;
                  break;
               }
               if (i>32 && i<41) {
                  ibo=Index % BONUM;         //board number, complete board blocks ignored
                  icr=Index / BONUM;         //number of complete board blocks == icr
                  j=i-33;
                  DSPNiosVal[icr][ibo][j]  = mean;
                  DSPNiosStd[icr][ibo][j]  = std;
                  DSPNiosStat[icr][ibo][j] = stat;
                  break;
               }
               if (i>40 && i<47) {
                  ich=Index % CHNUM;     //channel number , complete channel blocks ignored
                  j=Index / CHNUM;       // = icr *BONUM + ibo
                  ibo=j % BONUM;         //board number, complete board blocks ignored
                  icr=j / BONUM;         //number of complete board blocks == icr
                  j=i-41;
//printf("-----j= %i icr=%i ibo=%i ich=%i mean=%f std=%f stat=%i\n",j,icr,ibo,ich,mean,std,stat);
                  DSPadVal[icr][ibo][ich][j]  = mean;
                  DSPadStd[icr][ibo][ich][j]  = std;
                  DSPadStat[icr][ibo][ich][j] = stat;
                  break;
               }
               if (i>46 && i<51) {
                  j=i-47;
                  BCU2SGNVal[j]  = mean;
                  BCU2SGNStd[j]  = std;
                  BCU2SGNStat[j] = stat;
                  break;
               }
               if (i>50 && i<54) {
                  icr=Index;
                  j=i-51;
                  BCU1SGNTempVal[icr][j]  = mean;
                  BCU1SGNTempStd[icr][j]  = std;
                  BCU1SGNTempStat[icr][j] = stat;
                  BCU1SGNTempBufV[icr][j][Nb] = mean;
                  break;
               }
               if (i>53 && i<58) {
                  ibo=Index % BONUM;        //board number, complete board blocks ignored
                  icr=Index / BONUM;         //number of complete board blocks == icr
                  j=i-54;
                  DSPTempVal[icr][ibo][j]  = mean;
                  DSPTempStd[icr][ibo][j]  = std;
                  DSPTempStat[icr][ibo][j] = stat;
                  DSPTempBufV[icr][ibo][j][Nb] = mean;
                  break;
               }
               if (i==58) {
                  ich=Index % CHNUM;     //channel number , complete channel blocks ignored
                  j=Index / CHNUM;       // = icr *BONUM + ibo
                  ibo=j % BONUM;         //board number, complete board blocks ignored
                  icr=j / BONUM;         //number of complete board blocks == icr
                  DSPCurVal[icr][ibo][ich]  = mean;
                  DSPCurStd[icr][ibo][ich]  = std;
                  DSPCurStat[icr][ibo][ich] = stat;
                  DSPCurBufV[icr][ibo][ich][Nb] = mean;
                  break;
               }
               if (i>58 && i<61) {
                  icr=6;  //switch BCU
                  j=i-59;
                  BCU1TempVal[icr][j]  = mean;
                  BCU1TempStd[icr][j]  = std;
                  BCU1TempStat[icr][j] = stat;
                  BCU1TempBufV[icr][j][Nb] = mean;
                  break;
               }
               if (i>60 && i<63) {
                  icr=Index;
                  j=i-61;
                  BCU1TempVal[icr][j]  = mean;
                  BCU1TempStd[icr][j]  = std;
                  BCU1TempStat[icr][j] = stat;
                  BCU1TempBufV[icr][j][Nb] = mean;
                  break;
               }
               if (i>62 && i<73) {
                  icr=Index;
                  j=i-63;
                  BCU1VolVal[icr][j]  = mean;
                  BCU1VolStd[icr][j]  = std;
                  BCU1VolStat[icr][j] = stat;
                  BCU1VolBufV[icr][j][Nb] = mean;
                  break;
               }
               if (i>72 && i<79) {
                  icr=Index;
                  j=i-73;
//printf("-----j= %i icr=%i mean=%f std=%f stat=%i\n",j,icr,mean,std,stat);
                  BCU1EnvVal[icr][j]  = mean;
                  BCU1EnvStd[icr][j]  = std;
                  BCU1EnvStat[icr][j] = stat;
                  BCU1EnvBufV[icr][j][Nb] = mean;
                  break;
               }
               if (i>78 && i<82) {
                  j=i-79;
                  BCU2MastVal[j]  = mean;
                  BCU2MastStd[j]  = std;
                  BCU2MastStat[j] = stat;
                  BCU2MastBufV[j][Nb] = mean;
                  break;
               }
               if (i>81 && i<92) {
                  icr=Index;
                  j=i-82;
                  BCU2DigStat[icr][j] = (long long) mean;
                  //BCU2DigStat[icr][j] = stat;
//if (icr==0 && j==0 )
//printf("DigStat-----i= %i %s=%s stat=%i mean=%f std=%f\n",i,FamilyNames[i],familyName,stat,mean,std);
                  break;
               }
               if (i>91 && i<102) {
                  icr=Index;
                  j=i-92;
                  BCU2ResetStat[icr][j] = (long long) mean;
                  //BCU2ResetStat[icr][j] = stat;
//if (icr==0 && j==0 )
//printf("ResetStat-----i= %i %s=%s stat=%i mean=%f std=%f\n",i,FamilyNames[i],familyName,stat,mean,std);
                  break;
               }
               if (i>101 && i<112) {
                  ibo=Index % BONUM;         //board number, complete board blocks ignored
                  icr=Index / BONUM;         //number of complete board blocks == icr
                  j=i-102;
//printf("....-----j= %i icr=%i ibo=%i stat=%i\n",j,icr,ibo,stat);
                  DSPDrSStat[icr][ibo][j] = (long long) mean;
                  //DSPDrSStat[icr][ibo][j] = stat;
//if (icr==0 && ibo==0 && j==0 )
//printf("DrSStat-----i= %i %s=%s stat=%i mean=%f std=%f\n",i,FamilyNames[i],familyName,stat,mean,std);
                  break;
               }
               if (i==112) {
                  ich=Index % CHNUM;     //channel number , complete channel blocks ignored
                  j=Index / CHNUM;       // = icr *BONUM + ibo
                  ibo=j % BONUM;         //board number, complete board blocks ignored
                  icr=j / BONUM;         //number of complete board blocks == icr
                  DSPDrStat[icr][ibo][ich] = (long long) mean;
                  //DSPDrStat[icr][ibo][ich] = stat;
//if (icr==0 && ibo==0 && ich==0 )
//printf("DrStat-----i= %i %s=%s stat=%i mean=%f std=%f\n",i,FamilyNames[i],familyName,stat,mean,std);
//printf("....-----j= %i icr=%i ibo=%i stat=%i\n",j,icr,ibo,stat);
                  break;
               }
               if (i>112 && i<122) {
                  j=i-113;
                  ExternalVal[j]  = mean;
                  ExternalStd[j]  = std;
                  ExternalStat[j] = stat;
                  ExternalBufV[j][Nb] = mean;
//printf("-----j= %i  mean=%f std=%f stat=%i\n",j,mean,std,stat);
                  break;
               }
               
                if (i>121 && i<127) {
                  j=i-122;
                  GeneralVal[j]  = mean;
                  GeneralStd[j]  = std;
                  GeneralStat[j] = stat;
                     _logger->log(Logger::LOG_LEV_INFO, "Family is %s",familyName);
                    _logger->log(Logger::LOG_LEV_INFO, "Temp val= %f",GeneralVal[j]);
                  break;
               }
            }
        }
//        _logger->log(Logger::LOG_LEV_DEBUG, "[GetDiagnValue] %s  %s --- %s [%s:%d]",
//                diagnapp, wh.str().c_str(), va.str().c_str(), __FILE__,__LINE__);
    }

    iLong++;
    if (iLong>=NLong) {
       //accumlate LONG buffer time  in sec?
       if (Nbl==0) timeL[Nbl] = .5 *(timeB[Nb-1]-timeB[0])/60.;
       else timeL[Nbl] = .5 *(timeB[Nb-1]-timeB[Nb-NLong-1])/60. +timeB[Nb-NLong-1]/60.;
       for (i=0; i<10; i++) {
            if (i<9) ExternalBufVL[i][Nbl] = meanBuf ( ExternalBufV[i], Nb );
            if (i<4) BCU2TrBufVL[i][Nbl] = meanBuf ( BCU2TrBufV[i], Nb );
            if (i<3) BCU2MastBufVL[i][Nbl] = meanBuf ( BCU2MastBufV[i], Nb );
            for (int icr=0; icr<CRNUM; icr++) {
                BCU1VolBufVL[icr][i][Nbl] = meanBuf ( BCU1VolBufV[icr][i], Nb );
                if (i<6) BCU1EnvBufVL[icr][i][Nbl] = meanBuf ( BCU1EnvBufV[icr][i], Nb );
                if (i<2) BCU1TempBufVL[icr][i][Nbl] = meanBuf ( BCU1TempBufV[icr][i], Nb );
                if (i<3) BCU1SGNTempBufVL[icr][i][Nbl] = meanBuf ( BCU1SGNTempBufV[icr][i], Nb );
                for (int ibo=0; ibo<BONUM; ibo++) {
                    if (i<8) DSPCurBufVL[icr][ibo][i][Nbl] = meanBuf ( DSPCurBufV[icr][ibo][i], Nb );
                    if (i<4) DSPTempBufVL[icr][ibo][i][Nbl] = meanBuf ( DSPTempBufV[icr][ibo][i], Nb );
                }
            }
       }
       Nbl++;
       iLong=0;
    }
    Nb++;
}


void GUILBT672::GetAllAdam()
{
 
    //char *diagnapp = (char *)"ADAMHOUSEKPR00";
    string _diagnapp = std::string("adamhousekeeper.") + Side();
    char *diagnapp = (char *)_diagnapp.c_str();

    char *family = (char *)"*";
    int   from = -1;
    int   to   = -1;
    int   timeout = 1000; // timeout in ms;
    ValueDict dict;

    string Family;
    const char *familyName;
    int   Index;
    int i,stat;
    double mean,std;
    char temp[100];

    int jj=0;

/*    if (Init!=0) {
       if (Nb >=NBuf) {
          shiftBuf ( timeB, NBuf );
          for (i=0; i<10; i++) {
               if (i<9) shiftBuf ( ExternalBufV[i], NBuf );
               if (i<4) shiftBuf ( BCU2TrBufV[i], NBuf );
               if (i<3) shiftBuf ( BCU2MastBufV[i], NBuf );
               for (int icr=0; icr<CRNUM; icr++) {
                   shiftBuf ( BCU1VolBufV[icr][i], NBuf );
                   if (i<6) shiftBuf ( BCU1EnvBufV[icr][i], NBuf );
                   if (i<2) shiftBuf ( BCU1TempBufV[icr][i], NBuf );
                   if (i<3) shiftBuf ( BCU1SGNTempBufV[icr][i], NBuf );
                   for (int ibo=0; ibo<BONUM; ibo++) {
                       if (i<8) shiftBuf ( DSPCurBufV[icr][ibo][i], NBuf );
                       if (i<4) shiftBuf ( DSPTempBufV[icr][ibo][i], NBuf );
                   }
               }
          }
          Nb=NBuf-1;
       }
       if (Nbl >=NBufL) {
          shiftBuf ( timeL, NBufL );
          for (i=0; i<10; i++) {
               if (i<9) shiftBuf ( ExternalBufVL[i], NBufL );
               if (i<4) shiftBuf ( BCU2TrBufVL[i], NBufL );
               if (i<3) shiftBuf ( BCU2MastBufVL[i], NBufL );
               for (int icr=0; icr<CRNUM; icr++) {
                   shiftBuf ( BCU1VolBufVL[icr][i], NBufL );
                   if (i<6) shiftBuf ( BCU1EnvBufVL[icr][i], NBufL );
                   if (i<2) shiftBuf ( BCU1TempBufVL[icr][i], NBufL );
                   if (i<3) shiftBuf ( BCU1SGNTempBufVL[icr][i], NBufL );
                   for (int ibo=0; ibo<BONUM; ibo++) {
                       if (i<8) shiftBuf ( DSPCurBufVL[icr][ibo][i], NBufL );
                       if (i<4) shiftBuf ( DSPTempBufVL[icr][ibo][i], NBufL );
                   }
               }
          }
          Nbl=NBufL-1;
       }
    } else {
      srand(1);
      Init=1;
      Nb=0;
      Nbl=0;
   }*/

    try{
        DiagnWhich which(family, from, to);
        dict = GetDiagnValue( diagnapp,which, timeout);
    } catch (AOException &e) {
            if (!Dummy) {
               timer->stop();
               buttonStart->setText( tr( "&Start" ) );
               buttonStart->setPaletteBackgroundColor( QColor( 200, 0, 0 ) );
               QMessageBox::information(this,"GUILBT672::GetAll()",
                "Unable to get the AdamValues\nThe AdamHousekeeper interrogation is STOPPED\nor maybe broken");
               }

            if (Dummy) GetAllDummy();
            return;

    } 

   //Iterate over the dictionary of the retrieved DiagnVars.
   for (ValueDict::iterator it = dict.begin(); it != dict.end();  it++){
        jj++;

        DiagnWhich wh = (*it).first;
        DiagnValue va = (*it).second;

        //DiagnVar name & index
        Family = wh.Family();   // DiagnVar name.
        familyName = Family.c_str();
        //_logger->log(Logger::LOG_LEV_INFO, "Family %s",familyName);
        //timeB[Nb] = va.Time();   //accumulate current value time  in sec?

//printf("-----%s-\n",familyName);
        for (i=0; i<nadamfamily; i++) {
//printf(".....-----i= %i %s=%s\n",i,FamilyNames[i],familyName);
//_logger->log(Logger::LOG_LEV_INFO, "Family is %s",FamilyNames[i]);//mod RB
            if (strcmp(familyName,AdamFamilyNames[i]) == 0) {
//printf("found-----i= %i %s=%s\n",i,FamilyNames[i],familyName);
               Index  = wh.From();     // DiagnVar index
               
               stat = va.Status();   // Status: 0=OK, 1=Warning, 2=Error
               mean = (double) va.Mean();     // DiagnVar value (average of a running mean)
               sprintf (temp,"%f",mean);
               if (strcmp(temp,"nan")==0) mean = 0.;
               std  = (double) va.Stddev();   // Stddev of the running mean
               sprintf(temp,"%f",std);
               if (strcmp(temp,"nan")==0) std = 0.;

               //va.Last();     // Instantaneuos value (last value inserted in the running mean)
               //va.Time();     // Time (average of the time stamp of the values in the running mean)
//_logger->log(Logger::LOG_LEV_INFO, "Family is %s",familyName);//mod RB
               
                if (i<14) {

                  _logger->log(Logger::LOG_LEV_INFO, "Family is %s",familyName);
                  AdamVal[i]  = mean;
                  _logger->log(Logger::LOG_LEV_INFO, "Adam val= %f",AdamVal[i]);
                  AdamStd[i]  = std;
                  AdamStat[i] = stat;
                  _logger->log(Logger::LOG_LEV_INFO, "Adam stat= %f",AdamStat[i]);

                  break;
               }
            }
        }
//        _logger->log(Logger::LOG_LEV_DEBUG, "[GetDiagnValue] %s  %s --- %s [%s:%d]",
//                diagnapp, wh.str().c_str(), va.str().c_str(), __FILE__,__LINE__);
    }

/*    iLong++;
    if (iLong>=NLong) {
       //accumlate LONG buffer time  in sec?
       if (Nbl==0) timeL[Nbl] = .5 *(timeB[Nb-1]-timeB[0])/60.;
       else timeL[Nbl] = .5 *(timeB[Nb-1]-timeB[Nb-NLong-1])/60. +timeB[Nb-NLong-1]/60.;
       for (i=0; i<10; i++) {
            if (i<9) ExternalBufVL[i][Nbl] = meanBuf ( ExternalBufV[i], Nb );
            if (i<4) BCU2TrBufVL[i][Nbl] = meanBuf ( BCU2TrBufV[i], Nb );
            if (i<3) BCU2MastBufVL[i][Nbl] = meanBuf ( BCU2MastBufV[i], Nb );
            for (int icr=0; icr<CRNUM; icr++) {
                BCU1VolBufVL[icr][i][Nbl] = meanBuf ( BCU1VolBufV[icr][i], Nb );
                if (i<6) BCU1EnvBufVL[icr][i][Nbl] = meanBuf ( BCU1EnvBufV[icr][i], Nb );
                if (i<2) BCU1TempBufVL[icr][i][Nbl] = meanBuf ( BCU1TempBufV[icr][i], Nb );
                if (i<3) BCU1SGNTempBufVL[icr][i][Nbl] = meanBuf ( BCU1SGNTempBufV[icr][i], Nb );
                for (int ibo=0; ibo<BONUM; ibo++) {
                    if (i<8) DSPCurBufVL[icr][ibo][i][Nbl] = meanBuf ( DSPCurBufV[icr][ibo][i], Nb );
                    if (i<4) DSPTempBufVL[icr][ibo][i][Nbl] = meanBuf ( DSPTempBufV[icr][ibo][i], Nb );
                }
            }
       }
       Nbl++;
       iLong=0;
    }
    Nb++;  */
}




/*      dummy routine to check the functionality *********************************/
void GUILBT672::GetAllDummy()
{
   int i,stat;
   double mean,std;
//   qWarning( "GUILBT672::GetAllDummy(): called" );

//   DiagAppDataStat[3] = 0;   mod RB to implement ADAMDIAGN readings
//   DiagAppDataStat[4] = 2;
//   DiagAppDataStat[5] = 0;
//   DiagAppDataStd[6] = 0.;
   DiagAppDataStat[0] = 0;
   DiagAppDataStd[1] = 0.;

   ExternalStat[1] =  2;
   ExternalStat[2] =  0;

   BCU1NiosStat[0][1] =  0;
   BCU1NiosStat[0][2] =  0;

   BCU1NiosStat[1][3] =  0;
   BCU1NiosStat[1][4] =  0;
   BCU1NiosStat[1][2] =  2;
   

//   DiagAppDataStat[0] =2;
//   DiagAppDataStat[6] =2;
   DiagAppDataStat[1] =2;

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
   //DSPNiosStat[0] = 1;
   //DSPNiosStat[7] = 1;
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

   mean=10.;
   stat =0;
   for (i=0; i<9; i++) {
       mean =mean+ 0.00001*rand()/33000.;
       std =0.03*mean;
       stat =rand()%2+rand()%2;
       ExternalVal[i]  = mean;
       ExternalStd[i]  = std;
       ExternalStat[i] = stat;
       ExternalBufV[i][Nb] = mean;

       ExternalWMin[i]  = 9.;
       ExternalWMax[i]  = 11.;
       ExternalAMin[i]  = 8.;
       ExternalAMax[i]  = 12.;
  }

   stat =0;
  for (int icr=0; icr<6; icr++) {
   mean=14.7;
   for (i=0; i<3; i++) {
       mean =mean+ 0.00003*rand()/33000.;
       std =0.03*mean;
       stat =rand()%2+rand()%2;
       BCU1SGNTempVal[icr][i]  = mean;
       BCU1SGNTempStd[icr][i]  = std;
       BCU1SGNTempStat[icr][i] = stat;
       BCU1SGNTempBufV[icr][i][Nb] = mean;

       BCU1SGNTempWMin[icr][i]  = 11.;
       BCU1SGNTempWMax[icr][i]  = 21.;
       BCU1SGNTempAMin[icr][i]  = 10.;
       BCU1SGNTempAMax[icr][i]  = 22.5;
  }
 }

   stat =0;
  for (int icr=0; icr<6; icr++) {
   mean=17.;
   for (i=0; i<6; i++) {
       mean =mean+ 0.00002*rand()/33000.;
       std =0.03*mean;
       stat =rand()%2+rand()%2;
       BCU1EnvVal[icr][i]  = mean;
       BCU1EnvStd[icr][i]  = std;
       BCU1EnvStat[icr][i] = stat;
       BCU1EnvBufV[icr][i][Nb] = mean;

       BCU1EnvWMin[icr][i]  = 11.;
       BCU1EnvWMax[icr][i]  = 21.;
       BCU1EnvAMin[icr][i]  = 10.;
       BCU1EnvAMax[icr][i]  = 22.;
  }
 }

   stat =0;
  for (int icr=0; icr<7; icr++) {
   mean=14.;
   for (i=0; i<2; i++) {
       mean =mean+ 0.00002*rand()/33000.;
       std =0.03*mean;
       stat =rand()%2+rand()%2;
       BCU1TempVal[icr][i]  = mean;
       BCU1TempStd[icr][i]  = std;
       BCU1TempStat[icr][i] = stat;
       BCU1TempBufV[icr][i][Nb] = mean;

       BCU1TempWMin[icr][i]  = 11.;
       BCU1TempWMax[icr][i]  = 20.;
       BCU1TempAMin[icr][i]  = 10.;
       BCU1TempAMax[icr][i]  = 21.;
  }
 }

   mean=13.;
   stat =0;
   for (i=0; i<3; i++) {
       mean =mean+ 0.00003*rand()/33000.;
       std =0.03*mean;
       stat =rand()%2+rand()%2;
       BCU2MastVal[i]  = mean;
       BCU2MastStd[i]  = std;
       BCU2MastStat[i] = stat;
       BCU2MastBufV[i][Nb] = mean;

       BCU2MastWMin[i]  = 9.;
       BCU2MastWMax[i]  = 15.;
       BCU2MastAMin[i]  = 8.;
       BCU2MastAMax[i]  = 17.;
  }

   mean=21.;
   stat =0;
   for (i=0; i<4; i++) {
       mean =mean+ 0.00002*rand()/33000.;
       std =0.03*mean;
       stat =rand()%2+rand()%2;
       BCU2TrVal[i]  = mean;
       BCU2TrStd[i]  = std;
       BCU2TrStat[i] = stat;
       BCU2TrBufV[i][Nb] = mean;

       BCU2TrWMin[i]  = 9.;
       BCU2TrWMax[i]  = 11.;
       BCU2TrAMin[i]  = 8.;
       BCU2TrAMax[i]  = 32.;
  }

    if ( Nb==0 ) timeB[0] =0.;
    else timeB[Nb] = timeB[Nb-1] + 0.001*timeoutR;   //accumulate time from the session begining in sec
    //printf("Nb=%d time=%f\n",Nb,timeB[Nb]);
    
    iLong++;
    if (iLong>=NLong) {
       //accumlate LONG buffer time  in sec?
       if (Nbl==0) timeL[Nbl] = .5 *(timeB[Nb]-timeB[0])/60.;
       else timeL[Nbl] = .5 *(timeB[Nb]-timeB[Nb-NLong])/60. +timeB[Nb-NLong]/60.;
//       printf("Nbl=%d Nb=%d time=%f %f %f\n",Nbl,Nb,timeL[Nbl],timeB[Nb-1],timeB[Nb-NLong]);
       for (i=0; i<10; i++) {
            if (i<9) ExternalBufVL[i][Nbl] = meanBuf ( ExternalBufV[i], Nb );
            if (i<4) BCU2TrBufVL[i][Nbl] = meanBuf ( BCU2TrBufV[i], Nb );
            if (i<3) BCU2MastBufVL[i][Nbl] = meanBuf ( BCU2MastBufV[i], Nb );
            for (int icr=0; icr<CRNUM; icr++) {
                BCU1VolBufVL[icr][i][Nbl] = meanBuf ( BCU1VolBufV[icr][i], Nb );
                if (i<6) BCU1EnvBufVL[icr][i][Nbl] = meanBuf ( BCU1EnvBufV[icr][i], Nb );
                if (i<2) BCU1TempBufVL[icr][i][Nbl] = meanBuf ( BCU1TempBufV[icr][i], Nb );
                if (i<2) BCU1TempBufVL[6][i][Nbl] = meanBuf ( BCU1TempBufV[6][i], Nb );
                if (i<3) BCU1SGNTempBufVL[icr][i][Nbl] = meanBuf ( BCU1SGNTempBufV[icr][i], Nb );
                for (int ibo=0; ibo<BONUM; ibo++) {
                    if (i<8) DSPCurBufVL[icr][ibo][i][Nbl] = meanBuf ( DSPCurBufV[icr][ibo][i], Nb );
                    if (i<4) DSPTempBufVL[icr][ibo][i][Nbl] = meanBuf ( DSPTempBufV[icr][ibo][i], Nb );
                }
            }
       }
       Nbl++;
       iLong=0;
    }
    Nb++;
//   qWarning( "GUILBT672::GetAllDummy(): finished" );
}

void GUILBT672::SetICr1()
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

void GUILBT672::SetICr2()
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

void GUILBT672::SetICr3()
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

void GUILBT672::SetIBo()
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

void GUILBT672::SetICh()
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

void GUILBT672::SetAll()
{
   SetSystemSummary();
   SetCrateBCU1();
   SetCrateBCU2();
   SetCrateDSP();
   CheckWarnAlarms();
   SetWarnings();
   SetAlarms();
}

void GUILBT672::SetSystemSummary()
{
   char temp[300];
   int i;
   double refTime=-1;
   // DiagnAppData panel
  /*  mod RB
   for (i=0; i<7; i++) {
       switch (i) {
              case 1:
              case 2:
              case 3:
                sprintf (temp,"%10i                ",(int)DiagAppDataVal[i]);
                break;
              default:
                sprintf (temp,"%10.3f +- %8.3f",DiagAppDataVal[i],DiagAppDataStd[i]);
       }
       DiagnAppDataLab[i]->setText ( QString(temp));
       if (DiagAppDataStat[i] == 0) DiagnAppDataLed[i]->setColor( QColor (0,255,0));
       if (DiagAppDataStat[i] == 1) DiagnAppDataLed[i]->setColor( QColor (255,255,0));
       if (DiagAppDataStat[i] == 2) DiagnAppDataLed[i]->setColor( QColor (255,0,0));
   }*/
   for (i=0; i<1; i++){
             //sprintf (temp,"%10.3f +- %8.3f",DiagAppDataVal[i],DiagAppDataStd[i]);
             sprintf (temp,"%10.3f",DiagAppDataVal[i]);
             DiagnAppDataLab[i]->setText ( QString(temp));
             if (i==0) {
                  if (DiagAppDataVal[i] == refTime) {
                     DiagAppDataStat[i]=2;
                } else {
                    refTime = DiagAppDataVal[i];
                }
             } else {
                if (DiagAppDataStat[i] == 0) DiagnAppDataLed[i]->setColor( QColor (0,255,0));//Green
                if (DiagAppDataStat[i] == 1) DiagnAppDataLed[i]->setColor( QColor (255,255,0));//Yellow
                if (DiagAppDataStat[i] == 2) DiagnAppDataLed[i]->setColor( QColor (255,0,0));//Red
             }
   }
   for (i=0; i<5; i++){
             //sprintf (temp,"%10.3f +- %8.3f",GeneralVal[i],GeneralStd[i]);
             sprintf (temp,"%10.3f",GeneralVal[i]);
             GeneralValTemp[i]->setText ( QString(temp));
             if (GeneralStat[i] == 0) GeneralValTempLed[i]->setColor( QColor (0,255,0));//Green
             if (GeneralStat[i] == 1) GeneralValTempLed[i]->setColor( QColor (255,255,0));//Yellow
             if (GeneralStat[i] == 2) GeneralValTempLed[i]->setColor( QColor (255,0,0));//Red
       }
   for (i=0; i<14; i++){
       sprintf (temp,"%10.5f",AdamVal[i]);//++
       DiagnAdam[i]->setText ( QString(temp));//++
       if (i == 1 || i == 9 || i == 10) {    //was i=1,5,10; elements that
          AdamVal[i]=int(! bool(AdamVal[i]));
          }
       _logger->log(Logger::LOG_LEV_INFO, "Neg Family is %s ",AdamFamilyName[i]);//mod RB
       if (AdamVal[i] == 0) DiagnAdamLed[i]->setColor( QColor (255,0,0));
       else if (AdamVal[i] == 1) {DiagnAdamLed[i]->setColor( QColor (0,255,0));}
           /*else { DiagnAdamLed[i]->setColor( QColor (255,255,0));}
       }
       else {
           if (AdamVal[i] == 1) DiagnAdamLed[i]->setColor( QColor (0,255,0));
           else if (AdamVal[i] == 0) {DiagnAdamLed[i]->setColor( QColor (255,0,0));}
           else {DiagnAdamLed[i]->setColor( QColor (255,255,0));}
            }*/
       }
   // External.... panel
   for (i=0; i<9; i++) {
       //sprintf (temp,"%10.3f +- %8.3f",ExternalVal[i],ExternalStd[i]);
       sprintf (temp,"%10.3f",ExternalVal[i]);
       ExternalLab[i]->setText (QString(temp));
       if (ExternalStat[i] == 0) ExternalLed[i]->setColor( QColor (0,255,0));
       if (ExternalStat[i] == 1) ExternalLed[i]->setColor( QColor (255,255,0));
       if (ExternalStat[i] == 2) ExternalLed[i]->setColor( QColor (255,0,0));
       if (ExternalEn[i] != 1) ExternalLed[i]->setColor( QColor (100,100,100));
       
   }
}

void GUILBT672::SetCrateBCU1()
{
   char temp[300];
   char ip[300];
   char ip0[4];
   char ip1[4];
   char ip2[4];
   char ip3[4];
   int i,j,k;

   if (ICr1 < 6) sprintf (temp,"Crate %1i",ICr1);
   else strcpy(temp,"Switch BCU");
   crateBCU1_gr->setTitle( tr( temp ) );

   if (ICr1==6) j=1; else j=0;
   // NIOS Fixed Area  panel
   for (i=0; i<11; i++) {
       switch (i) {
              case 1:
              case 2:
              case 3:
                sprintf (temp,"%8llX", (long long) BCU1NiosVal[ICr1][i]);
                break;
              case 4:
              case 10:
                sprintf (ip,"%12lld",(long long)BCU1NiosVal[ICr1][i]);
                strncpy(ip0, &ip[0], 3);
                ip0[3] = '\0';
                strncpy(ip1, &ip[3], 3);
                ip1[3] = '\0';
                strncpy(ip2, &ip[6], 3);
                ip2[3] = '\0';
                strncpy(ip3, &ip[9], 3);
                ip3[3] = '\0';
                sprintf(temp, "%s.%s.%s.%s", ip0, ip1, ip2, ip3);
                break;
              case 9:
                sprintf (temp,"%lld",(long long)BCU1NiosVal[ICr1][i]);


                break;
              default:
                sprintf (temp,"%lld", (long long) BCU1NiosVal[ICr1][i]);
       }
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
          //sprintf (temp,"%10.3f +- %8.3f",BCU1EnvVal[ICr1][i],BCU1EnvStd[ICr1][i]);
          sprintf (temp,"%10.3f",BCU1EnvVal[ICr1][i]);
          BCU1EnvLab[i]->setText (QString(temp));
          if (BCU1EnvStat[ICr1][i] == 0) BCU1EnvLed[i]->setColor( QColor (0,255,0));
          if (BCU1EnvStat[ICr1][i] == 1) BCU1EnvLed[i]->setColor( QColor (255,255,0));
          if (BCU1EnvStat[ICr1][i] == 2) BCU1EnvLed[i]->setColor( QColor (255,0,0));
          if (BCU1EnvEn[ICr1][i] != 1) BCU1EnvLed[i]->setColor( QColor (100,100,100));
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
          //sprintf (temp,"%10.3f +- %8.3f",BCU1VolVal[ICr1][i],BCU1VolStd[ICr1][i]);
          sprintf (temp,"%10.3f",BCU1VolVal[ICr1][i]);
          BCU1VolLab[i]->setText (QString(temp));
          if (BCU1VolStat[ICr1][i] == 0) BCU1VolLed[i]->setColor( QColor (0,255,0));
          if (BCU1VolStat[ICr1][i] == 1) BCU1VolLed[i]->setColor( QColor (255,255,0));
          if (BCU1VolStat[ICr1][i] == 2) BCU1VolLed[i]->setColor( QColor (255,0,0));
          if (BCU1VolEn[ICr1][i] != 1) BCU1VolLed[i]->setColor( QColor (100,100,100));
      }
   } else {
      for (i=0; i<10; i++) {
          BCU1VolLab[i]->setText (QString("    "));
          BCU1VolLed[i]->setColor( QColor (192,192,192));
      }
   }
   // Temperatures  panel
   for (i=0; i<2; i++) {
       sprintf (temp,"%10.3f",BCU1TempVal[ICr1][i]);
       //sprintf (temp,"%10.3f +- %8.3f",BCU1TempVal[ICr1][i],BCU1TempStd[ICr1][i]);
       BCU1TempLab[i]->setText ( QString(temp));
       if (BCU1TempStat[ICr1][i] == 0) BCU1TempLed[i]->setColor( QColor (0,255,0));
       if (BCU1TempStat[ICr1][i] == 1) BCU1TempLed[i]->setColor( QColor (255,255,0));
       if (BCU1TempStat[ICr1][i] == 2) BCU1TempLed[i]->setColor( QColor (255,0,0));
       if (BCU1TempEn[ICr1][i] != 1) BCU1TempLed[i]->setColor( QColor (100,100,100));
   }
   // SIGGEN Temperatures panel
   if (ICr1<6) {
      for (i=0; i<3; i++) {
          sprintf (temp,"%10.3f",BCU1SGNTempVal[ICr1][i]);
          //sprintf (temp,"%10.3f +- %8.3f",BCU1SGNTempVal[ICr1][i],BCU1SGNTempStd[ICr1][i]);
          BCU1SGNTempLab[i]->setText (QString(temp));
          if (BCU1SGNTempStat[ICr1][i] == 0) BCU1SGNTempLed[i]->setColor( QColor (0,255,0));
          if (BCU1SGNTempStat[ICr1][i] == 1) BCU1SGNTempLed[i]->setColor( QColor (255,255,0));
          if (BCU1SGNTempStat[ICr1][i] == 2) BCU1SGNTempLed[i]->setColor( QColor (255,0,0));
          if (BCU1SGNTempEn[ICr1][i] != 1) BCU1SGNTempLed[i]->setColor( QColor (100,100,100));
      }
   } else {
      for (i=0; i<3; i++) {
          BCU1SGNTempLab[i]->setText (QString("    "));
          BCU1SGNTempLed[i]->setColor( QColor (192,192,192));
      }
   }
}

void GUILBT672::SetCrateBCU2()
{
   char temp[300];
   int i;

   sprintf (temp,"Crate %1i",ICr2);
   crateBCU2_gr->setTitle( tr( temp ) );

   // Reset status  panel
   for (i=0; i<10; i++) {
       if (BCU2ResetStat[ICr2][i] == 0) BCU2ResetLed[i]->setColor( QColor (50,50,50));
       if (BCU2ResetStat[ICr2][i] == 1) BCU2ResetLed[i]->setColor( QColor (255,255,255));
      // if (BCU2ResetStat[ICr2][i] == 0) BCU2ResetLed[i]->setState( KLed::Off );
      // if (BCU2ResetStat[ICr2][i] == 1) BCU2ResetLed[i]->setState( KLed::On );
   }
   // Digital I/O  panel
   for (i=0; i<10; i++) {
       if (BCU2DigStat[ICr2][i] == 0) BCU2DigLed[i]->setColor( QColor (50,50,50));
       if (BCU2DigStat[ICr2][i] == 1) BCU2DigLed[i]->setColor( QColor (255,255,255));
       //if (BCU2DigStat[ICr2][i] == 0) BCU2DigLed[i]->setState( KLed::Off );
       //if (BCU2DigStat[ICr2][i] == 1) BCU2DigLed[i]->setState( KLed::On );
   }
   // Master crate  panel
   for (i=0; i<3; i++) {
       sprintf (temp,"%10.3f",BCU2MastVal[i]);
       //sprintf (temp,"%10.3f +- %8.3f",BCU2MastVal[i],BCU2MastStd[i]);
       BCU2MastLab[i]->setText ( QString(temp));
       if (BCU2MastStat[i] == 0) BCU2MastLed[i]->setColor( QColor (0,255,0));
       if (BCU2MastStat[i] == 1) BCU2MastLed[i]->setColor( QColor (255,255,0));
       if (BCU2MastStat[i] == 2) BCU2MastLed[i]->setColor( QColor (255,0,0));
       if (BCU2MastEn[i] != 1) BCU2MastLed[i]->setColor( QColor (100,100,100));
   }
   // SIGGEN NIOS Fixed Area panel
   for (i=0; i<4; i++) {
       //sprintf (temp,"%10.3f +- %8.3f",BCU2SGNVal[i],BCU2SGNStd[i]);
       sprintf (temp,"%10i",(int)BCU2SGNVal[i]);
       BCU2SGNLab[i]->setText ( QString(temp));
       if (BCU2SGNStat[i] == 0) BCU2SGNLed[i]->setColor( QColor (0,255,0));
       if (BCU2SGNStat[i] == 1) BCU2SGNLed[i]->setColor( QColor (255,255,0));
       if (BCU2SGNStat[i] == 2) BCU2SGNLed[i]->setColor( QColor (255,0,0));
   }
   // Thresholds panel
   for (i=0; i<4; i++) {
       sprintf (temp,"%10.3f",BCU2TrVal[i]);
       //sprintf (temp,"%10.3f +- %8.3f",BCU2TrVal[i],BCU2TrStd[i]);
       BCU2TrLab[i]->setText ( QString(temp));
       if (BCU2TrStat[i] == 0) BCU2TrLed[i]->setColor( QColor (0,255,0));
       if (BCU2TrStat[i] == 1) BCU2TrLed[i]->setColor( QColor (255,255,0));
       if (BCU2TrStat[i] == 2) BCU2TrLed[i]->setColor( QColor (255,0,0));
       if (BCU2TrEn[i] != 1) BCU2TrLed[i]->setColor( QColor (100,100,100));
   }
}

void GUILBT672::SetCrateDSP()
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
       //sprintf (temp,"%10.3f +- %8.3f",DSPNiosVal[i],DSPNiosStd[i]);
       switch(i) {
         case 0:
         case 3: {
             sprintf (temp,"%lld",(long long)DSPNiosVal[ICr3][IBo][i]);
             break;
          }
          default: 
             sprintf (temp,"%8llx",(long long)DSPNiosVal[ICr3][IBo][i]);

       }
       DSPNiosLab[i]->setText ( QString(temp));
       if (DSPNiosStat[ICr3][IBo][i] == 0) DSPNiosLed[i]->setColor( QColor (0,255,0));
       if (DSPNiosStat[ICr3][IBo][i] == 1) DSPNiosLed[i]->setColor( QColor (255,255,0));
       if (DSPNiosStat[ICr3][IBo][i] == 2) DSPNiosLed[i]->setColor( QColor (255,0,0));
   }
   //  Coil currents (SPI) panel
   for (i=0; i<8; i++) {
       sprintf (temp,"%10.3f",DSPCurVal[ICr3][IBo][i]);
       //sprintf (temp,"%10.3f +- %8.3f",DSPCurVal[ICr3][IBo][i],DSPCurStd[ICr3][IBo][i]);
       DSPCurLab[i]->setText ( QString(temp));
       if (DSPCurStat[ICr3][IBo][i] == 0) DSPCurLed[i]->setColor( QColor (0,255,0));
       if (DSPCurStat[ICr3][IBo][i] == 1) DSPCurLed[i]->setColor( QColor (255,255,0));
       if (DSPCurStat[ICr3][IBo][i] == 2) DSPCurLed[i]->setColor( QColor (255,0,0));
       if (DSPCurEn[ICr3][IBo][i] != 1) DSPCurLed[i]->setColor( QColor (100,100,100));
   }
   //  Driver Status panel
   for (i=0; i<10; i++) {
       if (DSPDrSStat[ICr3][IBo][i] == 0) DSPDrSLed[i]->setColor( QColor (50,50,50));
       if (DSPDrSStat[ICr3][IBo][i] == 1) DSPDrSLed[i]->setColor( QColor (255,255,255));
       //if (DSPDrSStat[ICr3][IBo][i] == 0) DSPDrSLed[i]->setState( KLed::Off );
       //if (DSPDrSStat[ICr3][IBo][i] == 1) DSPDrSLed[i]->setState( KLed::On );
   }
   //  Temperatures panel
   for (i=0; i<4; i++) {
       sprintf (temp,"%10.3f",DSPTempVal[ICr3][IBo][i]);
       //sprintf (temp,"%10.3f +- %8.3f",DSPTempVal[ICr3][IBo][i],DSPTempStd[ICr3][IBo][i]);
       DSPTempLab[i]->setText ( QString(temp));
       if (DSPTempStat[ICr3][IBo][i] == 0) DSPTempLed[i]->setColor( QColor (0,255,0));
       if (DSPTempStat[ICr3][IBo][i] == 1) DSPTempLed[i]->setColor( QColor (255,255,0));
       if (DSPTempStat[ICr3][IBo][i] == 2) DSPTempLed[i]->setColor( QColor (255,0,0));
       if (DSPTempEn[ICr3][IBo][i] != 1) DSPTempLed[i]->setColor( QColor (100,100,100));
   }
   //  Driver Enabled panel
   for (i=0; i<8; i++) {
       if (DSPDrStat[ICr3][IBo][i] == 0) DSPDrLed[i]->setColor( QColor (50,50,50));
       if (DSPDrStat[ICr3][IBo][i] == 1) DSPDrLed[i]->setColor( QColor (255,255,255));
       //if (DSPDrStat[ICr3][IBo][i] == 0) DSPDrLed[i]->setState( KLed::Off );
       //if (DSPDrStat[ICr3][IBo][i] == 1) DSPDrLed[i]->setState( KLed::On );
   }
   //  ADC panel
   for (i=0; i<6; i++) {
       sprintf (temp,"%10.3f",DSPadVal[ICr3][IBo][ICh][i]);
       //sprintf (temp,"%10.3f +- %8.3f",DSPadVal[ICr3][IBo][ICh][i],DSPadStd[ICr3][IBo][ICh][i]);
       DSPadLab[i]->setText ( QString(temp));
//    sprintf (temp,"cr3  %i bo %i ch %i i %i Stat %i",ICr3,IBo,ICh,i,DSPadStat[ICr3][IBo][ICh][i]);
//    qWarning( temp );
       if (DSPadStat[ICr3][IBo][ICh][i] == 0) DSPadLed[i]->setColor( QColor (0,255,0));
       if (DSPadStat[ICr3][IBo][ICh][i] == 1) DSPadLed[i]->setColor( QColor (255,255,0));
       if (DSPadStat[ICr3][IBo][ICh][i] == 2) DSPadLed[i]->setColor( QColor (255,0,0));
   }
}

void GUILBT672::CheckWarnAlarms()
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
/*   for (i=0; i<8; i++) {
       if (DSPNiosStat[ic][ib][i] == 1) {
           sprintf (temp," %s(DSP) ",FamilyNames[DSPNiosInd[0]+i]);
           Warnings.push_back(QString(temp));
       }
       if (DSPNiosStat[ic][ib][i] == 2) {
           sprintf (temp," %s(DSP) ",FamilyNames[DSPNiosInd[0]+i]);
           Alarms.push_back(QString(temp));
       }*/
  for (ic=0; ic<6; ic++) {
       for (i=0; i<8; i++) {
           for (ib=0; ib<14; ib++) {
               if (DSPNiosStat[ic][ib][i] == 1) {
                   sprintf (temp," %s(DSP,Cr=%1i,Bo=%1i,Ch=%1i) ",FamilyNames[DSPNiosInd[0]],ic,ib,i);
                   Warnings.push_back(QString(temp));
               }
               if (DSPNiosStat[ic][ib][i] == 2) {
                   sprintf (temp," %s(DSP,Cr=%1i,Bo=%1i,Ch=%1i) ",FamilyNames[DSPNiosInd[0]],ic,ib,i);
                   Alarms.push_back(QString(temp));
               }
           }
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

void GUILBT672::SetWarnings()
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

void GUILBT672::SetAlarms()
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

void GUILBT672::StopStartTimer()
{
    if ( QString::compare(buttonStart->text(), "&Stop") == 0 ) {
       timer->stop();
       buttonStart->setText( tr( "&Start" ) );
       buttonStart->setPaletteBackgroundColor( QColor( 200, 0, 0 ) );
    } else {
       buttonStart->setText( tr( "&Stop" ) );
       buttonStart->setPaletteBackgroundColor( QColor( 255, 255, 127 ) );
       Refresh();
    }
}


void GUILBT672::quit() //close everything
{
     SetTimeToDie(true);
     timer->stop();
     close();
}

void GUILBT672::Run()
{
   // qWarning( "GUILBT672::AOApp::Run(): skeep Run method of AOApp" );
//   for (int j=0; j<200; j++) {
//      string tmpFam = string(FamilyName[j]);
//      std::transform(tmpFam.begin(), tmpFam.end(), tmpFam.begin(),(int(*)(int)) std::toupper);
//      FamilyName[j] = (char*)tmpFam.c_str();
//    }
}
