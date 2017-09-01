/****************************************************************************
** Form implementation generated from reading ui file 'AdSecControl_gui.ui'
**
** Created by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "AdSecControl_gui.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qgroupbox.h>
#include <qprogressbar.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <kurlrequester.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qtextedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include "kurlrequester.h"
#include "klineedit.h"
#include "kpushbutton.h"

/*
 *  Constructs a AdSecControlGui as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
AdSecControlGui::AdSecControlGui( QWidget* parent, const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    (void)statusBar();
    if ( !name )
	setName( "AdSecControlGui" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setMinimumSize( QSize( 861, 949 ) );
    setMaximumSize( QSize( 32767, 32767 ) );
    setCentralWidget( new QWidget( this, "qt_central_widget" ) );
    AdSecControlGuiLayout = new QHBoxLayout( centralWidget(), 5, 5, "AdSecControlGuiLayout"); 

    commandButtonsGroup = new QButtonGroup( centralWidget(), "commandButtonsGroup" );
    commandButtonsGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)5, 0, 0, commandButtonsGroup->sizePolicy().hasHeightForWidth() ) );
    commandButtonsGroup->setMaximumSize( QSize( 120, 32767 ) );
    commandButtonsGroup->setFrameShape( QButtonGroup::GroupBoxPanel );
    commandButtonsGroup->setFrameShadow( QButtonGroup::Sunken );

    onButton = new QPushButton( commandButtonsGroup, "onButton" );
    onButton->setGeometry( QRect( 5, 22, 110, 26 ) );

    offButton = new QPushButton( commandButtonsGroup, "offButton" );
    offButton->setGeometry( QRect( 5, 53, 110, 26 ) );

    loadProgramButton = new QPushButton( commandButtonsGroup, "loadProgramButton" );
    loadProgramButton->setGeometry( QRect( 5, 84, 110, 26 ) );

    calibrateButton = new QPushButton( commandButtonsGroup, "calibrateButton" );
    calibrateButton->setGeometry( QRect( 5, 115, 110, 26 ) );

    setFlatAoButton = new QPushButton( commandButtonsGroup, "setFlatAoButton" );
    setFlatAoButton->setGeometry( QRect( 5, 177, 110, 26 ) );

    restButton = new QPushButton( commandButtonsGroup, "restButton" );
    restButton->setGeometry( QRect( 5, 208, 110, 26 ) );

    aoToSl = new QPushButton( commandButtonsGroup, "aoToSl" );
    aoToSl->setGeometry( QRect( 5, 270, 110, 26 ) );

    slToAoButton = new QPushButton( commandButtonsGroup, "slToAoButton" );
    slToAoButton->setGeometry( QRect( 5, 301, 110, 26 ) );

    runAoButton = new QPushButton( commandButtonsGroup, "runAoButton" );
    runAoButton->setGeometry( QRect( 5, 332, 110, 26 ) );

    pauseAoButton = new QPushButton( commandButtonsGroup, "pauseAoButton" );
    pauseAoButton->setGeometry( QRect( 5, 363, 110, 26 ) );

    resumeAoButton = new QPushButton( commandButtonsGroup, "resumeAoButton" );
    resumeAoButton->setGeometry( QRect( 5, 394, 110, 26 ) );

    stopAoButton = new QPushButton( commandButtonsGroup, "stopAoButton" );
    stopAoButton->setGeometry( QRect( 5, 425, 110, 26 ) );

    recoverFailButton = new QPushButton( commandButtonsGroup, "recoverFailButton" );
    recoverFailButton->setGeometry( QRect( 5, 456, 110, 26 ) );

    textLabel1_2 = new QLabel( commandButtonsGroup, "textLabel1_2" );
    textLabel1_2->setGeometry( QRect( 5, 513, 110, 21 ) );

    setFlatChopButton = new QPushButton( commandButtonsGroup, "setFlatChopButton" );
    setFlatChopButton->setGeometry( QRect( 5, 539, 110, 26 ) );

    setFlatSlButton = new QPushButton( commandButtonsGroup, "setFlatSlButton" );
    setFlatSlButton->setGeometry( QRect( 5, 570, 110, 26 ) );

    resetChopButton = new QPushButton( commandButtonsGroup, "resetChopButton" );
    resetChopButton->setGeometry( QRect( 5, 601, 110, 26 ) );

    resetSlButton = new QPushButton( commandButtonsGroup, "resetSlButton" );
    resetSlButton->setGeometry( QRect( 5, 632, 110, 26 ) );

    chopToSlButton = new QPushButton( commandButtonsGroup, "chopToSlButton" );
    chopToSlButton->setGeometry( QRect( 5, 663, 110, 26 ) );

    slToChopButton = new QPushButton( commandButtonsGroup, "slToChopButton" );
    slToChopButton->setGeometry( QRect( 5, 694, 110, 26 ) );

    chopToAoButton = new QPushButton( commandButtonsGroup, "chopToAoButton" );
    chopToAoButton->setGeometry( QRect( 5, 725, 110, 26 ) );

    aoToChopButton = new QPushButton( commandButtonsGroup, "aoToChopButton" );
    aoToChopButton->setGeometry( QRect( 5, 756, 110, 26 ) );

    runChopButton = new QPushButton( commandButtonsGroup, "runChopButton" );
    runChopButton->setGeometry( QRect( 5, 787, 110, 26 ) );

    stopChopButton = new QPushButton( commandButtonsGroup, "stopChopButton" );
    stopChopButton->setGeometry( QRect( 5, 818, 110, 26 ) );

    resetButton = new QPushButton( commandButtonsGroup, "resetButton" );
    resetButton->setEnabled( FALSE );
    resetButton->setGeometry( QRect( 5, 146, 110, 26 ) );

    resetAoButton = new QPushButton( commandButtonsGroup, "resetAoButton" );
    resetAoButton->setEnabled( FALSE );
    resetAoButton->setGeometry( QRect( 5, 239, 110, 26 ) );
    AdSecControlGuiLayout->addWidget( commandButtonsGroup );

    layout56 = new QVBoxLayout( 0, 0, 5, "layout56"); 

    layout55 = new QVBoxLayout( 0, 0, 5, "layout55"); 

    layout54 = new QHBoxLayout( 0, 0, 5, "layout54"); 

    layout17 = new QVBoxLayout( 0, 0, 5, "layout17"); 

    textLabel2_2 = new QLabel( centralWidget(), "textLabel2_2" );
    textLabel2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    layout17->addWidget( textLabel2_2 );

    textLabel2_2_2 = new QLabel( centralWidget(), "textLabel2_2_2" );
    layout17->addWidget( textLabel2_2_2 );

    textLabel2_2_2_2 = new QLabel( centralWidget(), "textLabel2_2_2_2" );
    textLabel2_2_2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    layout17->addWidget( textLabel2_2_2_2 );
    layout54->addLayout( layout17 );

    layout53 = new QVBoxLayout( 0, 0, 5, "layout53"); 

    layout52 = new QHBoxLayout( 0, 0, 5, "layout52"); 

    labModeLabel = new QLabel( centralWidget(), "labModeLabel" );
    labModeLabel->setPaletteBackgroundColor( QColor( 255, 0, 0 ) );
    QFont labModeLabel_font(  labModeLabel->font() );
    labModeLabel_font.setPointSize( 12 );
    labModeLabel_font.setBold( TRUE );
    labModeLabel->setFont( labModeLabel_font ); 
    labModeLabel->setAlignment( int( QLabel::AlignCenter ) );
    layout52->addWidget( labModeLabel );
    layout53->addLayout( layout52 );

    layout48 = new QHBoxLayout( 0, 0, 5, "layout48"); 

    adSecArbitratorStatusLabel = new QLabel( centralWidget(), "adSecArbitratorStatusLabel" );
    QFont adSecArbitratorStatusLabel_font(  adSecArbitratorStatusLabel->font() );
    adSecArbitratorStatusLabel_font.setPointSize( 16 );
    adSecArbitratorStatusLabel->setFont( adSecArbitratorStatusLabel_font ); 
    layout48->addWidget( adSecArbitratorStatusLabel );

    focalStationLabel = new QLabel( centralWidget(), "focalStationLabel" );
    QFont focalStationLabel_font(  focalStationLabel->font() );
    focalStationLabel_font.setPointSize( 16 );
    focalStationLabel->setFont( focalStationLabel_font ); 
    layout48->addWidget( focalStationLabel );
    layout53->addLayout( layout48 );

    layout16 = new QHBoxLayout( 0, 0, 5, "layout16"); 

    lastCommandLabel = new QLabel( centralWidget(), "lastCommandLabel" );
    lastCommandLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, lastCommandLabel->sizePolicy().hasHeightForWidth() ) );
    QFont lastCommandLabel_font(  lastCommandLabel->font() );
    lastCommandLabel_font.setPointSize( 16 );
    lastCommandLabel->setFont( lastCommandLabel_font ); 
    layout16->addWidget( lastCommandLabel );

    lastCommandResultLabel = new QLabel( centralWidget(), "lastCommandResultLabel" );
    lastCommandResultLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 0, 0, lastCommandResultLabel->sizePolicy().hasHeightForWidth() ) );
    QFont lastCommandResultLabel_font(  lastCommandResultLabel->font() );
    lastCommandResultLabel_font.setPointSize( 16 );
    lastCommandResultLabel->setFont( lastCommandResultLabel_font ); 
    layout16->addWidget( lastCommandResultLabel );
    layout53->addLayout( layout16 );
    layout54->addLayout( layout53 );
    layout55->addLayout( layout54 );

    layout36 = new QHBoxLayout( 0, 0, 5, "layout36"); 

    layout64 = new QHBoxLayout( 0, 0, 5, "layout64"); 

    textLabel1_3 = new QLabel( centralWidget(), "textLabel1_3" );
    layout64->addWidget( textLabel1_3 );

    WindSpeedLCD = new QLCDNumber( centralWidget(), "WindSpeedLCD" );
    WindSpeedLCD->setPaletteForegroundColor( QColor( 0, 0, 0 ) );
    QPalette pal;
    QColorGroup cg;
    cg.setColor( QColorGroup::Foreground, black );
    cg.setColor( QColorGroup::Button, black );
    cg.setColor( QColorGroup::Light, black );
    cg.setColor( QColorGroup::Midlight, black );
    cg.setColor( QColorGroup::Dark, black );
    cg.setColor( QColorGroup::Mid, black );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, black );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, QColor( 230, 230, 230) );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, black );
    cg.setColor( QColorGroup::LinkVisited, black );
    pal.setActive( cg );
    cg.setColor( QColorGroup::Foreground, black );
    cg.setColor( QColorGroup::Button, black );
    cg.setColor( QColorGroup::Light, black );
    cg.setColor( QColorGroup::Midlight, black );
    cg.setColor( QColorGroup::Dark, black );
    cg.setColor( QColorGroup::Mid, black );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, black );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, QColor( 230, 230, 230) );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, QColor( 0, 0, 255) );
    cg.setColor( QColorGroup::LinkVisited, QColor( 255, 0, 255) );
    pal.setInactive( cg );
    cg.setColor( QColorGroup::Foreground, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::Button, black );
    cg.setColor( QColorGroup::Light, black );
    cg.setColor( QColorGroup::Midlight, black );
    cg.setColor( QColorGroup::Dark, black );
    cg.setColor( QColorGroup::Mid, black );
    cg.setColor( QColorGroup::Text, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, QColor( 230, 230, 230) );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, QColor( 0, 0, 255) );
    cg.setColor( QColorGroup::LinkVisited, QColor( 255, 0, 255) );
    pal.setDisabled( cg );
    WindSpeedLCD->setPalette( pal );
    QFont WindSpeedLCD_font(  WindSpeedLCD->font() );
    WindSpeedLCD_font.setBold( TRUE );
    WindSpeedLCD->setFont( WindSpeedLCD_font ); 
    WindSpeedLCD->setLineWidth( 0 );
    WindSpeedLCD->setNumDigits( 3 );
    WindSpeedLCD->setProperty( "value", -1 );
    layout64->addWidget( WindSpeedLCD );
    layout36->addLayout( layout64 );

    layout62 = new QHBoxLayout( 0, 0, 5, "layout62"); 

    textLabel1_3_2 = new QLabel( centralWidget(), "textLabel1_3_2" );
    layout62->addWidget( textLabel1_3_2 );

    ElevationLCD = new QLCDNumber( centralWidget(), "ElevationLCD" );
    cg.setColor( QColorGroup::Foreground, black );
    cg.setColor( QColorGroup::Button, black );
    cg.setColor( QColorGroup::Light, black );
    cg.setColor( QColorGroup::Midlight, black );
    cg.setColor( QColorGroup::Dark, black );
    cg.setColor( QColorGroup::Mid, black );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, black );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, QColor( 230, 230, 230) );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, black );
    cg.setColor( QColorGroup::LinkVisited, black );
    pal.setActive( cg );
    cg.setColor( QColorGroup::Foreground, black );
    cg.setColor( QColorGroup::Button, black );
    cg.setColor( QColorGroup::Light, black );
    cg.setColor( QColorGroup::Midlight, black );
    cg.setColor( QColorGroup::Dark, black );
    cg.setColor( QColorGroup::Mid, black );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, black );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, QColor( 230, 230, 230) );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, QColor( 0, 0, 255) );
    cg.setColor( QColorGroup::LinkVisited, QColor( 255, 0, 255) );
    pal.setInactive( cg );
    cg.setColor( QColorGroup::Foreground, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::Button, black );
    cg.setColor( QColorGroup::Light, black );
    cg.setColor( QColorGroup::Midlight, black );
    cg.setColor( QColorGroup::Dark, black );
    cg.setColor( QColorGroup::Mid, black );
    cg.setColor( QColorGroup::Text, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, QColor( 230, 230, 230) );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, QColor( 0, 0, 255) );
    cg.setColor( QColorGroup::LinkVisited, QColor( 255, 0, 255) );
    pal.setDisabled( cg );
    ElevationLCD->setPalette( pal );
    QFont ElevationLCD_font(  ElevationLCD->font() );
    ElevationLCD_font.setBold( TRUE );
    ElevationLCD->setFont( ElevationLCD_font ); 
    ElevationLCD->setLineWidth( 0 );
    ElevationLCD->setNumDigits( 3 );
    ElevationLCD->setProperty( "value", -1 );
    layout62->addWidget( ElevationLCD );
    layout36->addLayout( layout62 );

    layout62_2 = new QHBoxLayout( 0, 0, 5, "layout62_2"); 

    labelCheckDew = new QLabel( centralWidget(), "labelCheckDew" );
    layout62_2->addWidget( labelCheckDew );

    SwaLCD = new QLCDNumber( centralWidget(), "SwaLCD" );
    cg.setColor( QColorGroup::Foreground, black );
    cg.setColor( QColorGroup::Button, black );
    cg.setColor( QColorGroup::Light, black );
    cg.setColor( QColorGroup::Midlight, black );
    cg.setColor( QColorGroup::Dark, black );
    cg.setColor( QColorGroup::Mid, black );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, black );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, QColor( 230, 230, 230) );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, black );
    cg.setColor( QColorGroup::LinkVisited, black );
    pal.setActive( cg );
    cg.setColor( QColorGroup::Foreground, black );
    cg.setColor( QColorGroup::Button, black );
    cg.setColor( QColorGroup::Light, black );
    cg.setColor( QColorGroup::Midlight, black );
    cg.setColor( QColorGroup::Dark, black );
    cg.setColor( QColorGroup::Mid, black );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, black );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, QColor( 230, 230, 230) );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, QColor( 0, 0, 255) );
    cg.setColor( QColorGroup::LinkVisited, QColor( 255, 0, 255) );
    pal.setInactive( cg );
    cg.setColor( QColorGroup::Foreground, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::Button, black );
    cg.setColor( QColorGroup::Light, black );
    cg.setColor( QColorGroup::Midlight, black );
    cg.setColor( QColorGroup::Dark, black );
    cg.setColor( QColorGroup::Mid, black );
    cg.setColor( QColorGroup::Text, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, QColor( 230, 230, 230) );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 0, 0, 128) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, QColor( 0, 0, 255) );
    cg.setColor( QColorGroup::LinkVisited, QColor( 255, 0, 255) );
    pal.setDisabled( cg );
    SwaLCD->setPalette( pal );
    QFont SwaLCD_font(  SwaLCD->font() );
    SwaLCD_font.setBold( TRUE );
    SwaLCD->setFont( SwaLCD_font ); 
    SwaLCD->setLineWidth( 0 );
    SwaLCD->setNumDigits( 3 );
    SwaLCD->setProperty( "value", -1 );
    layout62_2->addWidget( SwaLCD );
    layout36->addLayout( layout62_2 );
    layout55->addLayout( layout36 );
    layout56->addLayout( layout55 );

    groupBox4 = new QGroupBox( centralWidget(), "groupBox4" );
    groupBox4->setColumnLayout(0, Qt::Vertical );
    groupBox4->layout()->setSpacing( 5 );
    groupBox4->layout()->setMargin( 5 );
    groupBox4Layout = new QHBoxLayout( groupBox4->layout() );
    groupBox4Layout->setAlignment( Qt::AlignTop );

    layout54_2 = new QVBoxLayout( 0, 0, 5, "layout54_2"); 

    ShellRipStatus = new QLabel( groupBox4, "ShellRipStatus" );
    ShellRipStatus->setPaletteBackgroundColor( QColor( 192, 192, 192 ) );
    layout54_2->addWidget( ShellRipStatus );

    forceRip = new QPushButton( groupBox4, "forceRip" );
    forceRip->setEnabled( FALSE );
    layout54_2->addWidget( forceRip );
    groupBox4Layout->addLayout( layout54_2 );

    layout55_2 = new QVBoxLayout( 0, 0, 5, "layout55_2"); 

    TssStatus = new QLabel( groupBox4, "TssStatus" );
    TssStatus->setPaletteBackgroundColor( QColor( 192, 192, 192 ) );
    layout55_2->addWidget( TssStatus );

    ForceTssOn = new QPushButton( groupBox4, "ForceTssOn" );
    ForceTssOn->setPaletteBackgroundColor( QColor( 255, 0, 0 ) );
    layout55_2->addWidget( ForceTssOn );
    groupBox4Layout->addLayout( layout55_2 );

    layout56_2 = new QVBoxLayout( 0, 0, 5, "layout56_2"); 

    PowerStatus = new QLabel( groupBox4, "PowerStatus" );
    PowerStatus->setPaletteBackgroundColor( QColor( 192, 192, 192 ) );
    layout56_2->addWidget( PowerStatus );

    ForcePowerOff = new QPushButton( groupBox4, "ForcePowerOff" );
    ForcePowerOff->setEnabled( FALSE );
    layout56_2->addWidget( ForcePowerOff );
    groupBox4Layout->addLayout( layout56_2 );

    layout53_2 = new QVBoxLayout( 0, 0, 5, "layout53_2"); 

    labelSkipStatus = new QLabel( groupBox4, "labelSkipStatus" );
    labelSkipStatus->setMaximumSize( QSize( 32767, 32767 ) );
    labelSkipStatus->setPaletteBackgroundColor( QColor( 192, 192, 192 ) );
    labelSkipStatus->setFrameShape( QLabel::Box );
    labelSkipStatus->setMargin( 1 );
    layout53_2->addWidget( labelSkipStatus );

    progressBarSkip = new QProgressBar( groupBox4, "progressBarSkip" );
    progressBarSkip->setProgress( 0 );
    layout53_2->addWidget( progressBarSkip );
    groupBox4Layout->addLayout( layout53_2 );
    layout56->addWidget( groupBox4 );

    shapeTab = new QTabWidget( centralWidget(), "shapeTab" );

    tab = new QWidget( shapeTab, "tab" );
    tabLayout = new QVBoxLayout( tab, 5, 5, "tabLayout"); 

    buttonGroup9 = new QButtonGroup( tab, "buttonGroup9" );
    buttonGroup9->setColumnLayout(0, Qt::Vertical );
    buttonGroup9->layout()->setSpacing( 5 );
    buttonGroup9->layout()->setMargin( 5 );
    buttonGroup9Layout = new QHBoxLayout( buttonGroup9->layout() );
    buttonGroup9Layout->setAlignment( Qt::AlignTop );

    layout41 = new QVBoxLayout( 0, 0, 5, "layout41"); 

    textLabel10 = new QLabel( buttonGroup9, "textLabel10" );
    layout41->addWidget( textLabel10 );

    textLabel5 = new QLabel( buttonGroup9, "textLabel5" );
    layout41->addWidget( textLabel5 );

    textLabel6 = new QLabel( buttonGroup9, "textLabel6" );
    layout41->addWidget( textLabel6 );

    textLabel4_2_2 = new QLabel( buttonGroup9, "textLabel4_2_2" );
    layout41->addWidget( textLabel4_2_2 );
    buttonGroup9Layout->addLayout( layout41 );

    layout59 = new QVBoxLayout( 0, 0, 5, "layout59"); 

    curShape = new QLabel( buttonGroup9, "curShape" );
    curShape->setMinimumSize( QSize( 250, 0 ) );
    QFont curShape_font(  curShape->font() );
    curShape_font.setBold( TRUE );
    curShape->setFont( curShape_font ); 
    layout59->addWidget( curShape );

    layout58 = new QHBoxLayout( 0, 0, 5, "layout58"); 

    kURLshapeLoad = new KURLRequester( buttonGroup9, "kURLshapeLoad" );
    layout58->addWidget( kURLshapeLoad );

    shapeLoad = new QPushButton( buttonGroup9, "shapeLoad" );
    shapeLoad->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, shapeLoad->sizePolicy().hasHeightForWidth() ) );
    shapeLoad->setMaximumSize( QSize( 90, 30 ) );
    layout58->addWidget( shapeLoad );
    layout59->addLayout( layout58 );

    layout57 = new QHBoxLayout( 0, 0, 5, "layout57"); 

    kURLshapeSave = new KURLRequester( buttonGroup9, "kURLshapeSave" );
    layout57->addWidget( kURLshapeSave );

    shapeSaveButton = new QPushButton( buttonGroup9, "shapeSaveButton" );
    shapeSaveButton->setMaximumSize( QSize( 90, 30 ) );
    layout57->addWidget( shapeSaveButton );
    layout59->addLayout( layout57 );

    layout56_3 = new QHBoxLayout( 0, 0, 5, "layout56_3"); 

    kURLcommandVector = new KURLRequester( buttonGroup9, "kURLcommandVector" );
    layout56_3->addWidget( kURLcommandVector );

    commandApplyVector = new QPushButton( buttonGroup9, "commandApplyVector" );
    commandApplyVector->setMaximumSize( QSize( 90, 30 ) );
    layout56_3->addWidget( commandApplyVector );
    layout59->addLayout( layout56_3 );
    buttonGroup9Layout->addLayout( layout59 );

    layout47 = new QVBoxLayout( 0, 0, 5, "layout47"); 
    spacer3 = new QSpacerItem( 117, 21, QSizePolicy::Minimum, QSizePolicy::Minimum );
    layout47->addItem( spacer3 );
    spacer2 = new QSpacerItem( 117, 21, QSizePolicy::Minimum, QSizePolicy::Minimum );
    layout47->addItem( spacer2 );

    checkBoxDefault = new QCheckBox( buttonGroup9, "checkBoxDefault" );
    layout47->addWidget( checkBoxDefault );
    spacer5 = new QSpacerItem( 21, 21, QSizePolicy::Minimum, QSizePolicy::Minimum );
    layout47->addItem( spacer5 );
    buttonGroup9Layout->addLayout( layout47 );
    tabLayout->addWidget( buttonGroup9 );

    buttonGroup7 = new QButtonGroup( tab, "buttonGroup7" );

    QWidget* privateLayoutWidget = new QWidget( buttonGroup7, "layout46" );
    privateLayoutWidget->setGeometry( QRect( 5, 22, 684, 112 ) );
    layout46 = new QHBoxLayout( privateLayoutWidget, 5, 5, "layout46"); 

    layout44 = new QVBoxLayout( 0, 0, 5, "layout44"); 

    textLabel1_4_2 = new QLabel( privateLayoutWidget, "textLabel1_4_2" );
    layout44->addWidget( textLabel1_4_2 );

    textLabel1_4 = new QLabel( privateLayoutWidget, "textLabel1_4" );
    layout44->addWidget( textLabel1_4 );

    textLabel1_4_3 = new QLabel( privateLayoutWidget, "textLabel1_4_3" );
    layout44->addWidget( textLabel1_4_3 );
    layout46->addLayout( layout44 );

    layout40 = new QVBoxLayout( 0, 0, 5, "layout40"); 

    labelOffloadStatus = new QLabel( privateLayoutWidget, "labelOffloadStatus" );
    labelOffloadStatus->setMaximumSize( QSize( 32767, 20 ) );
    labelOffloadStatus->setPaletteBackgroundColor( QColor( 192, 192, 192 ) );
    labelOffloadStatus->setFrameShape( QLabel::Box );
    labelOffloadStatus->setMargin( 1 );
    layout40->addWidget( labelOffloadStatus );

    labelHOOffloadStatus = new QLabel( privateLayoutWidget, "labelHOOffloadStatus" );
    labelHOOffloadStatus->setMaximumSize( QSize( 32767, 20 ) );
    labelHOOffloadStatus->setPaletteBackgroundColor( QColor( 192, 192, 192 ) );
    labelHOOffloadStatus->setFrameShape( QLabel::Box );
    labelHOOffloadStatus->setMargin( 1 );
    layout40->addWidget( labelHOOffloadStatus );

    labelAccStatus = new QLabel( privateLayoutWidget, "labelAccStatus" );
    labelAccStatus->setMaximumSize( QSize( 32767, 20 ) );
    labelAccStatus->setPaletteBackgroundColor( QColor( 192, 192, 192 ) );
    labelAccStatus->setFrameShape( QLabel::Box );
    labelAccStatus->setMargin( 1 );
    layout40->addWidget( labelAccStatus );
    layout46->addLayout( layout40 );

    layout45 = new QVBoxLayout( 0, 0, 5, "layout45"); 

    layout41_2 = new QHBoxLayout( 0, 0, 5, "layout41_2"); 

    buttonOffloadEnable = new QPushButton( privateLayoutWidget, "buttonOffloadEnable" );
    buttonOffloadEnable->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, buttonOffloadEnable->sizePolicy().hasHeightForWidth() ) );
    buttonOffloadEnable->setMaximumSize( QSize( 90, 30 ) );
    layout41_2->addWidget( buttonOffloadEnable );

    buttonOffloadDisable = new QPushButton( privateLayoutWidget, "buttonOffloadDisable" );
    buttonOffloadDisable->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, buttonOffloadDisable->sizePolicy().hasHeightForWidth() ) );
    buttonOffloadDisable->setMaximumSize( QSize( 90, 30 ) );
    layout41_2->addWidget( buttonOffloadDisable );
    layout45->addLayout( layout41_2 );

    layout42 = new QHBoxLayout( 0, 0, 5, "layout42"); 

    buttonHOOOffloadEnable = new QPushButton( privateLayoutWidget, "buttonHOOOffloadEnable" );
    buttonHOOOffloadEnable->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, buttonHOOOffloadEnable->sizePolicy().hasHeightForWidth() ) );
    buttonHOOOffloadEnable->setMaximumSize( QSize( 90, 30 ) );
    layout42->addWidget( buttonHOOOffloadEnable );

    buttonHOOffloadDisable = new QPushButton( privateLayoutWidget, "buttonHOOffloadDisable" );
    buttonHOOffloadDisable->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, buttonHOOffloadDisable->sizePolicy().hasHeightForWidth() ) );
    buttonHOOffloadDisable->setMaximumSize( QSize( 90, 30 ) );
    layout42->addWidget( buttonHOOffloadDisable );
    layout45->addLayout( layout42 );

    layout43 = new QHBoxLayout( 0, 0, 5, "layout43"); 

    buttonAccEnable = new QPushButton( privateLayoutWidget, "buttonAccEnable" );
    buttonAccEnable->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, buttonAccEnable->sizePolicy().hasHeightForWidth() ) );
    buttonAccEnable->setMaximumSize( QSize( 90, 30 ) );
    layout43->addWidget( buttonAccEnable );

    buttonAccDisable = new QPushButton( privateLayoutWidget, "buttonAccDisable" );
    buttonAccDisable->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, buttonAccDisable->sizePolicy().hasHeightForWidth() ) );
    buttonAccDisable->setMaximumSize( QSize( 90, 30 ) );
    layout43->addWidget( buttonAccDisable );
    layout45->addLayout( layout43 );
    layout46->addLayout( layout45 );
    tabLayout->addWidget( buttonGroup7 );

    buttonGroup7_2 = new QButtonGroup( tab, "buttonGroup7_2" );
    buttonGroup7_2->setColumnLayout(0, Qt::Vertical );
    buttonGroup7_2->layout()->setSpacing( 5 );
    buttonGroup7_2->layout()->setMargin( 5 );
    buttonGroup7_2Layout = new QHBoxLayout( buttonGroup7_2->layout() );
    buttonGroup7_2Layout->setAlignment( Qt::AlignTop );

    layout53_3 = new QVBoxLayout( 0, 0, 5, "layout53_3"); 

    textLabel1_5 = new QLabel( buttonGroup7_2, "textLabel1_5" );
    layout53_3->addWidget( textLabel1_5 );

    textLabel1_6 = new QLabel( buttonGroup7_2, "textLabel1_6" );
    textLabel1_6->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    layout53_3->addWidget( textLabel1_6 );

    textLabel1_6_2 = new QLabel( buttonGroup7_2, "textLabel1_6_2" );
    textLabel1_6_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    layout53_3->addWidget( textLabel1_6_2 );
    spacer7 = new QSpacerItem( 121, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout53_3->addItem( spacer7 );
    buttonGroup7_2Layout->addLayout( layout53_3 );

    layout54_3 = new QVBoxLayout( 0, 0, 5, "layout54_3"); 

    z2label = new QLabel( buttonGroup7_2, "z2label" );
    z2label->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z2label->sizePolicy().hasHeightForWidth() ) );
    z2label->setMinimumSize( QSize( 50, 20 ) );
    z2label->setMaximumSize( QSize( 50, 20 ) );
    layout54_3->addWidget( z2label );

    z2Total = new QLabel( buttonGroup7_2, "z2Total" );
    z2Total->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z2Total->sizePolicy().hasHeightForWidth() ) );
    z2Total->setMinimumSize( QSize( 50, 20 ) );
    z2Total->setMaximumSize( QSize( 50, 20 ) );
    layout54_3->addWidget( z2Total );

    z2Applied = new QLabel( buttonGroup7_2, "z2Applied" );
    z2Applied->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z2Applied->sizePolicy().hasHeightForWidth() ) );
    z2Applied->setMinimumSize( QSize( 50, 20 ) );
    z2Applied->setMaximumSize( QSize( 50, 20 ) );
    layout54_3->addWidget( z2Applied );

    z2Input = new QLineEdit( buttonGroup7_2, "z2Input" );
    z2Input->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, z2Input->sizePolicy().hasHeightForWidth() ) );
    z2Input->setAlignment( int( QLineEdit::AlignHCenter ) );
    layout54_3->addWidget( z2Input );
    buttonGroup7_2Layout->addLayout( layout54_3 );

    layout55_3 = new QVBoxLayout( 0, 0, 5, "layout55_3"); 

    z3label = new QLabel( buttonGroup7_2, "z3label" );
    z3label->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z3label->sizePolicy().hasHeightForWidth() ) );
    z3label->setMinimumSize( QSize( 50, 20 ) );
    z3label->setMaximumSize( QSize( 50, 20 ) );
    layout55_3->addWidget( z3label );

    z3Total = new QLabel( buttonGroup7_2, "z3Total" );
    z3Total->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z3Total->sizePolicy().hasHeightForWidth() ) );
    z3Total->setMinimumSize( QSize( 50, 20 ) );
    z3Total->setMaximumSize( QSize( 50, 20 ) );
    layout55_3->addWidget( z3Total );

    z3Applied = new QLabel( buttonGroup7_2, "z3Applied" );
    z3Applied->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z3Applied->sizePolicy().hasHeightForWidth() ) );
    z3Applied->setMinimumSize( QSize( 50, 20 ) );
    z3Applied->setMaximumSize( QSize( 50, 20 ) );
    layout55_3->addWidget( z3Applied );

    z3Input = new QLineEdit( buttonGroup7_2, "z3Input" );
    z3Input->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, z3Input->sizePolicy().hasHeightForWidth() ) );
    z3Input->setAlignment( int( QLineEdit::AlignHCenter ) );
    layout55_3->addWidget( z3Input );
    buttonGroup7_2Layout->addLayout( layout55_3 );

    layout56_4 = new QVBoxLayout( 0, 0, 5, "layout56_4"); 

    z5label = new QLabel( buttonGroup7_2, "z5label" );
    z5label->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z5label->sizePolicy().hasHeightForWidth() ) );
    z5label->setMinimumSize( QSize( 50, 20 ) );
    z5label->setMaximumSize( QSize( 50, 20 ) );
    layout56_4->addWidget( z5label );

    z5Total = new QLabel( buttonGroup7_2, "z5Total" );
    z5Total->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z5Total->sizePolicy().hasHeightForWidth() ) );
    z5Total->setMinimumSize( QSize( 50, 20 ) );
    z5Total->setMaximumSize( QSize( 50, 20 ) );
    layout56_4->addWidget( z5Total );

    z5Applied = new QLabel( buttonGroup7_2, "z5Applied" );
    z5Applied->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z5Applied->sizePolicy().hasHeightForWidth() ) );
    z5Applied->setMinimumSize( QSize( 50, 20 ) );
    z5Applied->setMaximumSize( QSize( 50, 20 ) );
    layout56_4->addWidget( z5Applied );

    z5Input = new QLineEdit( buttonGroup7_2, "z5Input" );
    z5Input->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, z5Input->sizePolicy().hasHeightForWidth() ) );
    z5Input->setAlignment( int( QLineEdit::AlignHCenter ) );
    layout56_4->addWidget( z5Input );
    buttonGroup7_2Layout->addLayout( layout56_4 );

    layout57_2 = new QVBoxLayout( 0, 0, 5, "layout57_2"); 

    z6label = new QLabel( buttonGroup7_2, "z6label" );
    z6label->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z6label->sizePolicy().hasHeightForWidth() ) );
    z6label->setMinimumSize( QSize( 50, 20 ) );
    z6label->setMaximumSize( QSize( 50, 20 ) );
    layout57_2->addWidget( z6label );

    z6Total = new QLabel( buttonGroup7_2, "z6Total" );
    z6Total->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z6Total->sizePolicy().hasHeightForWidth() ) );
    z6Total->setMinimumSize( QSize( 50, 20 ) );
    z6Total->setMaximumSize( QSize( 50, 20 ) );
    layout57_2->addWidget( z6Total );

    z6Applied = new QLabel( buttonGroup7_2, "z6Applied" );
    z6Applied->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z6Applied->sizePolicy().hasHeightForWidth() ) );
    z6Applied->setMinimumSize( QSize( 50, 20 ) );
    z6Applied->setMaximumSize( QSize( 50, 20 ) );
    layout57_2->addWidget( z6Applied );

    z6Input = new QLineEdit( buttonGroup7_2, "z6Input" );
    z6Input->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, z6Input->sizePolicy().hasHeightForWidth() ) );
    z6Input->setAlignment( int( QLineEdit::AlignHCenter ) );
    layout57_2->addWidget( z6Input );
    buttonGroup7_2Layout->addLayout( layout57_2 );

    layout58_2 = new QVBoxLayout( 0, 0, 5, "layout58_2"); 

    z7label = new QLabel( buttonGroup7_2, "z7label" );
    z7label->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z7label->sizePolicy().hasHeightForWidth() ) );
    z7label->setMinimumSize( QSize( 50, 20 ) );
    z7label->setMaximumSize( QSize( 50, 20 ) );
    layout58_2->addWidget( z7label );

    z7Total = new QLabel( buttonGroup7_2, "z7Total" );
    z7Total->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z7Total->sizePolicy().hasHeightForWidth() ) );
    z7Total->setMinimumSize( QSize( 50, 20 ) );
    z7Total->setMaximumSize( QSize( 50, 20 ) );
    layout58_2->addWidget( z7Total );

    z7Applied = new QLabel( buttonGroup7_2, "z7Applied" );
    z7Applied->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z7Applied->sizePolicy().hasHeightForWidth() ) );
    z7Applied->setMinimumSize( QSize( 50, 20 ) );
    z7Applied->setMaximumSize( QSize( 50, 20 ) );
    layout58_2->addWidget( z7Applied );

    z7Input = new QLineEdit( buttonGroup7_2, "z7Input" );
    z7Input->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, z7Input->sizePolicy().hasHeightForWidth() ) );
    z7Input->setAlignment( int( QLineEdit::AlignHCenter ) );
    layout58_2->addWidget( z7Input );
    buttonGroup7_2Layout->addLayout( layout58_2 );

    layout59_2 = new QVBoxLayout( 0, 0, 5, "layout59_2"); 

    z8label = new QLabel( buttonGroup7_2, "z8label" );
    z8label->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z8label->sizePolicy().hasHeightForWidth() ) );
    z8label->setMinimumSize( QSize( 50, 20 ) );
    z8label->setMaximumSize( QSize( 50, 20 ) );
    layout59_2->addWidget( z8label );

    z8Total = new QLabel( buttonGroup7_2, "z8Total" );
    z8Total->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z8Total->sizePolicy().hasHeightForWidth() ) );
    z8Total->setMinimumSize( QSize( 50, 20 ) );
    z8Total->setMaximumSize( QSize( 50, 20 ) );
    layout59_2->addWidget( z8Total );

    z8Applied = new QLabel( buttonGroup7_2, "z8Applied" );
    z8Applied->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z8Applied->sizePolicy().hasHeightForWidth() ) );
    z8Applied->setMinimumSize( QSize( 50, 20 ) );
    z8Applied->setMaximumSize( QSize( 50, 20 ) );
    layout59_2->addWidget( z8Applied );

    z8Input = new QLineEdit( buttonGroup7_2, "z8Input" );
    z8Input->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, z8Input->sizePolicy().hasHeightForWidth() ) );
    z8Input->setAlignment( int( QLineEdit::AlignHCenter ) );
    layout59_2->addWidget( z8Input );
    buttonGroup7_2Layout->addLayout( layout59_2 );

    layout60 = new QVBoxLayout( 0, 0, 5, "layout60"); 

    z9label = new QLabel( buttonGroup7_2, "z9label" );
    z9label->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z9label->sizePolicy().hasHeightForWidth() ) );
    z9label->setMinimumSize( QSize( 50, 20 ) );
    z9label->setMaximumSize( QSize( 50, 20 ) );
    layout60->addWidget( z9label );

    z9Total = new QLabel( buttonGroup7_2, "z9Total" );
    z9Total->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z9Total->sizePolicy().hasHeightForWidth() ) );
    z9Total->setMinimumSize( QSize( 50, 20 ) );
    z9Total->setMaximumSize( QSize( 50, 20 ) );
    layout60->addWidget( z9Total );

    z9Applied = new QLabel( buttonGroup7_2, "z9Applied" );
    z9Applied->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, z9Applied->sizePolicy().hasHeightForWidth() ) );
    z9Applied->setMinimumSize( QSize( 50, 20 ) );
    z9Applied->setMaximumSize( QSize( 50, 20 ) );
    layout60->addWidget( z9Applied );

    z9Input = new QLineEdit( buttonGroup7_2, "z9Input" );
    z9Input->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, z9Input->sizePolicy().hasHeightForWidth() ) );
    z9Input->setAlignment( int( QLineEdit::AlignHCenter ) );
    layout60->addWidget( z9Input );
    buttonGroup7_2Layout->addLayout( layout60 );

    zernApply = new QPushButton( buttonGroup7_2, "zernApply" );
    zernApply->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, zernApply->sizePolicy().hasHeightForWidth() ) );
    zernApply->setMaximumSize( QSize( 90, 130 ) );
    buttonGroup7_2Layout->addWidget( zernApply );
    tabLayout->addWidget( buttonGroup7_2 );
    shapeTab->insertTab( tab, QString::fromLatin1("") );

    tab_2 = new QWidget( shapeTab, "tab_2" );
    tabLayout_2 = new QVBoxLayout( tab_2, 5, 5, "tabLayout_2"); 

    groupBox5 = new QGroupBox( tab_2, "groupBox5" );
    groupBox5->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, groupBox5->sizePolicy().hasHeightForWidth() ) );
    groupBox5->setColumnLayout(0, Qt::Vertical );
    groupBox5->layout()->setSpacing( 5 );
    groupBox5->layout()->setMargin( 5 );
    groupBox5Layout = new QVBoxLayout( groupBox5->layout() );
    groupBox5Layout->setAlignment( Qt::AlignTop );

    layout84 = new QHBoxLayout( 0, 0, 5, "layout84"); 

    layout22_2 = new QVBoxLayout( 0, 0, 5, "layout22_2"); 

    textLabel1 = new QLabel( groupBox5, "textLabel1" );
    layout22_2->addWidget( textLabel1 );

    textLabel2 = new QLabel( groupBox5, "textLabel2" );
    layout22_2->addWidget( textLabel2 );

    textLabel3 = new QLabel( groupBox5, "textLabel3" );
    layout22_2->addWidget( textLabel3 );
    layout84->addLayout( layout22_2 );

    layout23_2 = new QVBoxLayout( 0, 0, 5, "layout23_2"); 

    curModalBasis = new QLabel( groupBox5, "curModalBasis" );
    QFont curModalBasis_font(  curModalBasis->font() );
    curModalBasis_font.setBold( TRUE );
    curModalBasis->setFont( curModalBasis_font ); 
    layout23_2->addWidget( curModalBasis );

    curRecMatrix = new QLabel( groupBox5, "curRecMatrix" );
    QFont curRecMatrix_font(  curRecMatrix->font() );
    curRecMatrix_font.setBold( TRUE );
    curRecMatrix->setFont( curRecMatrix_font ); 
    layout23_2->addWidget( curRecMatrix );

    curFiltering = new QLabel( groupBox5, "curFiltering" );
    QFont curFiltering_font(  curFiltering->font() );
    curFiltering_font.setBold( TRUE );
    curFiltering->setFont( curFiltering_font ); 
    layout23_2->addWidget( curFiltering );
    layout84->addLayout( layout23_2 );

    layout23_3 = new QVBoxLayout( 0, 0, 5, "layout23_3"); 

    comboModalBasis = new QComboBox( FALSE, groupBox5, "comboModalBasis" );
    layout23_3->addWidget( comboModalBasis );

    kURLrecMatrix = new KURLRequester( groupBox5, "kURLrecMatrix" );
    layout23_3->addWidget( kURLrecMatrix );

    comboFiltering = new QComboBox( FALSE, groupBox5, "comboFiltering" );
    layout23_3->addWidget( comboFiltering );
    layout84->addLayout( layout23_3 );

    buttonApply = new QPushButton( groupBox5, "buttonApply" );
    buttonApply->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, buttonApply->sizePolicy().hasHeightForWidth() ) );
    buttonApply->setMaximumSize( QSize( 90, 130 ) );
    layout84->addWidget( buttonApply );
    groupBox5Layout->addLayout( layout84 );
    tabLayout_2->addWidget( groupBox5 );

    buttonGroup6 = new QButtonGroup( tab_2, "buttonGroup6" );
    buttonGroup6->setColumnLayout(0, Qt::Vertical );
    buttonGroup6->layout()->setSpacing( 5 );
    buttonGroup6->layout()->setMargin( 5 );
    buttonGroup6Layout = new QVBoxLayout( buttonGroup6->layout() );
    buttonGroup6Layout->setAlignment( Qt::AlignTop );

    layout62_3 = new QHBoxLayout( 0, 0, 5, "layout62_3"); 

    comboKalman = new QComboBox( FALSE, buttonGroup6, "comboKalman" );
    layout62_3->addWidget( comboKalman );

    buttonKalmanApply = new QPushButton( buttonGroup6, "buttonKalmanApply" );
    buttonKalmanApply->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, buttonKalmanApply->sizePolicy().hasHeightForWidth() ) );
    buttonKalmanApply->setMaximumSize( QSize( 90, 30 ) );
    layout62_3->addWidget( buttonKalmanApply );
    buttonGroup6Layout->addLayout( layout62_3 );
    tabLayout_2->addWidget( buttonGroup6 );

    buttonGroup4 = new QButtonGroup( tab_2, "buttonGroup4" );
    buttonGroup4->setColumnLayout(0, Qt::Vertical );
    buttonGroup4->layout()->setSpacing( 5 );
    buttonGroup4->layout()->setMargin( 5 );
    buttonGroup4Layout = new QVBoxLayout( buttonGroup4->layout() );
    buttonGroup4Layout->setAlignment( Qt::AlignTop );

    layout48_2 = new QHBoxLayout( 0, 0, 5, "layout48_2"); 

    layout30 = new QVBoxLayout( 0, 0, 5, "layout30"); 

    textLabel4_2 = new QLabel( buttonGroup4, "textLabel4_2" );
    layout30->addWidget( textLabel4_2 );

    textLabel5_2 = new QLabel( buttonGroup4, "textLabel5_2" );
    layout30->addWidget( textLabel5_2 );
    layout48_2->addLayout( layout30 );

    layout25 = new QVBoxLayout( 0, 0, 5, "layout25"); 

    curGainVector = new QLabel( buttonGroup4, "curGainVector" );
    QFont curGainVector_font(  curGainVector->font() );
    curGainVector_font.setBold( TRUE );
    curGainVector->setFont( curGainVector_font ); 
    layout25->addWidget( curGainVector );

    layout24 = new QHBoxLayout( 0, 0, 5, "layout24"); 

    kURLgainVector = new KURLRequester( buttonGroup4, "kURLgainVector" );
    layout24->addWidget( kURLgainVector );

    gainApplyButton = new QPushButton( buttonGroup4, "gainApplyButton" );
    gainApplyButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, gainApplyButton->sizePolicy().hasHeightForWidth() ) );
    gainApplyButton->setMaximumSize( QSize( 90, 30 ) );
    layout24->addWidget( gainApplyButton );
    layout25->addLayout( layout24 );
    layout48_2->addLayout( layout25 );
    buttonGroup4Layout->addLayout( layout48_2 );

    layout54_4 = new QHBoxLayout( 0, 0, 5, "layout54_4"); 

    buttonGainZero = new QPushButton( buttonGroup4, "buttonGainZero" );
    layout54_4->addWidget( buttonGainZero );
    spacer8 = new QSpacerItem( 16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout54_4->addItem( spacer8 );

    buttonGain10modes = new QPushButton( buttonGroup4, "buttonGain10modes" );
    layout54_4->addWidget( buttonGain10modes );
    spacer8_2 = new QSpacerItem( 16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout54_4->addItem( spacer8_2 );

    textLabel1_7 = new QLabel( buttonGroup4, "textLabel1_7" );
    layout54_4->addWidget( textLabel1_7 );

    buttonGainTTUp = new QPushButton( buttonGroup4, "buttonGainTTUp" );
    layout54_4->addWidget( buttonGainTTUp );

    buttonGainTTDown = new QPushButton( buttonGroup4, "buttonGainTTDown" );
    layout54_4->addWidget( buttonGainTTDown );
    spacer10 = new QSpacerItem( 20, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout54_4->addItem( spacer10 );

    labelHO1 = new QLabel( buttonGroup4, "labelHO1" );
    layout54_4->addWidget( labelHO1 );

    buttonGainHO1Up = new QPushButton( buttonGroup4, "buttonGainHO1Up" );
    layout54_4->addWidget( buttonGainHO1Up );

    buttonGainHO1Down = new QPushButton( buttonGroup4, "buttonGainHO1Down" );
    layout54_4->addWidget( buttonGainHO1Down );
    spacer9 = new QSpacerItem( 16, 21, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout54_4->addItem( spacer9 );

    labelHO2 = new QLabel( buttonGroup4, "labelHO2" );
    layout54_4->addWidget( labelHO2 );

    buttonGainHO2Up = new QPushButton( buttonGroup4, "buttonGainHO2Up" );
    layout54_4->addWidget( buttonGainHO2Up );

    buttonGainHO2Down = new QPushButton( buttonGroup4, "buttonGainHO2Down" );
    layout54_4->addWidget( buttonGainHO2Down );
    buttonGroup4Layout->addLayout( layout54_4 );
    tabLayout_2->addWidget( buttonGroup4 );

    buttonGroup3_2 = new QButtonGroup( tab_2, "buttonGroup3_2" );
    buttonGroup3_2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, buttonGroup3_2->sizePolicy().hasHeightForWidth() ) );
    buttonGroup3_2->setColumnLayout(0, Qt::Vertical );
    buttonGroup3_2->layout()->setSpacing( 5 );
    buttonGroup3_2->layout()->setMargin( 5 );
    buttonGroup3_2Layout = new QHBoxLayout( buttonGroup3_2->layout() );
    buttonGroup3_2Layout->setAlignment( Qt::AlignTop );

    layout33 = new QVBoxLayout( 0, 0, 5, "layout33"); 

    textLabel7_2 = new QLabel( buttonGroup3_2, "textLabel7_2" );
    layout33->addWidget( textLabel7_2 );

    textLabel7_2_2 = new QLabel( buttonGroup3_2, "textLabel7_2_2" );
    layout33->addWidget( textLabel7_2_2 );
    buttonGroup3_2Layout->addLayout( layout33 );

    layout24_3 = new QVBoxLayout( 0, 0, 5, "layout24_3"); 

    layout23 = new QHBoxLayout( 0, 0, 5, "layout23"); 

    curDisturb = new QLabel( buttonGroup3_2, "curDisturb" );
    QFont curDisturb_font(  curDisturb->font() );
    curDisturb_font.setBold( TRUE );
    curDisturb->setFont( curDisturb_font ); 
    layout23->addWidget( curDisturb );
    layout24_3->addLayout( layout23 );

    layout22 = new QHBoxLayout( 0, 0, 5, "layout22"); 

    kURLdisturbSelect = new KURLRequester( buttonGroup3_2, "kURLdisturbSelect" );
    layout22->addWidget( kURLdisturbSelect );

    buttonDisturbSend = new QPushButton( buttonGroup3_2, "buttonDisturbSend" );
    buttonDisturbSend->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)4, (QSizePolicy::SizeType)4, 0, 0, buttonDisturbSend->sizePolicy().hasHeightForWidth() ) );
    buttonDisturbSend->setMaximumSize( QSize( 90, 30 ) );
    layout22->addWidget( buttonDisturbSend );
    layout24_3->addLayout( layout22 );
    buttonGroup3_2Layout->addLayout( layout24_3 );
    tabLayout_2->addWidget( buttonGroup3_2 );
    shapeTab->insertTab( tab_2, QString::fromLatin1("") );

    TabPage = new QWidget( shapeTab, "TabPage" );

    buttonGroupFS = new QButtonGroup( TabPage, "buttonGroupFS" );
    buttonGroupFS->setGeometry( QRect( 10, 10, 710, 100 ) );
    buttonGroupFS->setExclusive( TRUE );

    comboFocalStations = new QComboBox( FALSE, buttonGroupFS, "comboFocalStations" );
    comboFocalStations->setGeometry( QRect( 10, 30, 400, 30 ) );

    buttonSetFocalStation = new QPushButton( buttonGroupFS, "buttonSetFocalStation" );
    buttonSetFocalStation->setGeometry( QRect( 430, 30, 104, 30 ) );
    shapeTab->insertTab( TabPage, QString::fromLatin1("") );

    TabPage_2 = new QWidget( shapeTab, "TabPage_2" );
    TabPageLayout = new QHBoxLayout( TabPage_2, 5, 5, "TabPageLayout"); 

    processDumpText = new QTextEdit( TabPage_2, "processDumpText" );
    TabPageLayout->addWidget( processDumpText );
    shapeTab->insertTab( TabPage_2, QString::fromLatin1("") );
    layout56->addWidget( shapeTab );

    alertsBox = new QGroupBox( centralWidget(), "alertsBox" );
    alertsBox->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, alertsBox->sizePolicy().hasHeightForWidth() ) );
    alertsBox->setMaximumSize( QSize( 32767, 32767 ) );
    alertsBox->setColumnLayout(0, Qt::Vertical );
    alertsBox->layout()->setSpacing( 5 );
    alertsBox->layout()->setMargin( 5 );
    alertsBoxLayout = new QVBoxLayout( alertsBox->layout() );
    alertsBoxLayout->setAlignment( Qt::AlignTop );

    layout52_2 = new QVBoxLayout( 0, 0, 5, "layout52_2"); 

    alertsText = new QTextEdit( alertsBox, "alertsText" );
    alertsText->setEnabled( TRUE );
    alertsText->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)7, 0, 0, alertsText->sizePolicy().hasHeightForWidth() ) );
    alertsText->setMinimumSize( QSize( 700, 130 ) );
    alertsText->setMaximumSize( QSize( 32767, 32767 ) );
    alertsText->setTextFormat( QTextEdit::LogText );
    alertsText->setOverwriteMode( FALSE );
    alertsText->setReadOnly( TRUE );
    alertsText->setUndoRedoEnabled( FALSE );
    alertsText->setAutoFormatting( int( QTextEdit::AutoAll ) );
    layout52_2->addWidget( alertsText );

    layout51 = new QHBoxLayout( 0, 0, 5, "layout51"); 

    messageLabel = new QLabel( alertsBox, "messageLabel" );
    messageLabel->setMinimumSize( QSize( 500, 0 ) );
    messageLabel->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
    layout51->addWidget( messageLabel );

    commandProgressBar = new QProgressBar( alertsBox, "commandProgressBar" );
    layout51->addWidget( commandProgressBar );
    layout52_2->addLayout( layout51 );
    alertsBoxLayout->addLayout( layout52_2 );
    layout56->addWidget( alertsBox );
    AdSecControlGuiLayout->addLayout( layout56 );

    // toolbars

    languageChange();
    resize( QSize(874, 1021).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( ForceTssOn, SIGNAL( toggled(bool) ), this, SLOT( ForceTssOn_toggled(bool) ) );
    connect( ForcePowerOff, SIGNAL( clicked() ), this, SLOT( ForcePowerOff_clicked() ) );
    connect( commandApplyVector, SIGNAL( clicked() ), this, SLOT( commandApplyVector_clicked() ) );
    connect( shapeSaveButton, SIGNAL( clicked() ), this, SLOT( shapeSaveButton_clicked() ) );
    connect( shapeLoad, SIGNAL( clicked() ), this, SLOT( shapeLoad_clicked() ) );
    connect( onButton, SIGNAL( clicked() ), this, SLOT( onButton_clicked() ) );
    connect( offButton, SIGNAL( clicked() ), this, SLOT( offButton_clicked() ) );
    connect( loadProgramButton, SIGNAL( clicked() ), this, SLOT( loadProgramButton_clicked() ) );
    connect( calibrateButton, SIGNAL( clicked() ), this, SLOT( calibrateButton_clicked() ) );
    connect( resetButton, SIGNAL( clicked() ), this, SLOT( resetButton_clicked() ) );
    connect( setFlatAoButton, SIGNAL( clicked() ), this, SLOT( setFlatAoButton_clicked() ) );
    connect( restButton, SIGNAL( clicked() ), this, SLOT( restButton_clicked() ) );
    connect( resetAoButton, SIGNAL( clicked() ), this, SLOT( resetAoButton_clicked() ) );
    connect( aoToSl, SIGNAL( clicked() ), this, SLOT( aoToSl_clicked() ) );
    connect( slToAoButton, SIGNAL( clicked() ), this, SLOT( slToAoButton_clicked() ) );
    connect( runAoButton, SIGNAL( clicked() ), this, SLOT( runAoButton_clicked() ) );
    connect( pauseAoButton, SIGNAL( clicked() ), this, SLOT( pauseAoButton_clicked() ) );
    connect( resumeAoButton, SIGNAL( clicked() ), this, SLOT( resumeAoButton_clicked() ) );
    connect( stopAoButton, SIGNAL( clicked() ), this, SLOT( stopAoButton_clicked() ) );
    connect( recoverFailButton, SIGNAL( clicked() ), this, SLOT( recoverFailButton_clicked() ) );
    connect( setFlatChopButton, SIGNAL( clicked() ), this, SLOT( setFlatChopButton_clicked() ) );
    connect( setFlatSlButton, SIGNAL( clicked() ), this, SLOT( setFlatSlButton_clicked() ) );
    connect( resetChopButton, SIGNAL( clicked() ), this, SLOT( resetChopButton_clicked() ) );
    connect( resetSlButton, SIGNAL( clicked() ), this, SLOT( resetSlButton_clicked() ) );
    connect( aoToChopButton, SIGNAL( clicked() ), this, SLOT( aoToChopButton_clicked() ) );
    connect( runChopButton, SIGNAL( clicked() ), this, SLOT( runChopButton_clicked() ) );
    connect( stopChopButton, SIGNAL( clicked() ), this, SLOT( stopChopButton_clicked() ) );
    connect( forceRip, SIGNAL( clicked() ), this, SLOT( forceRip_clicked() ) );
    connect( buttonApply, SIGNAL( clicked() ), this, SLOT( buttonApply_clicked() ) );
    connect( buttonKalmanApply, SIGNAL( clicked() ), this, SLOT( buttonKalmanApply_clicked() ) );
    connect( gainApplyButton, SIGNAL( clicked() ), this, SLOT( gainApplyButton_clicked() ) );
    connect( buttonDisturbSend, SIGNAL( clicked() ), this, SLOT( buttonDisturbSend_clicked() ) );
    connect( chopToAoButton, SIGNAL( clicked() ), this, SLOT( chopToAoButton_clicked() ) );
    connect( slToChopButton, SIGNAL( clicked() ), this, SLOT( slToChopButton_clicked() ) );
    connect( chopToSlButton, SIGNAL( clicked() ), this, SLOT( chopToSlButton_clicked() ) );
    connect( zernApply, SIGNAL( clicked() ), this, SLOT( zernApply_clicked() ) );
    connect( z2Input, SIGNAL( returnPressed() ), this, SLOT( z2Input_returnPressed() ) );
    connect( z3Input, SIGNAL( returnPressed() ), this, SLOT( z3Input_returnPressed() ) );
    connect( z5Input, SIGNAL( returnPressed() ), this, SLOT( z5Input_returnPressed() ) );
    connect( z6Input, SIGNAL( returnPressed() ), this, SLOT( z6Input_returnPressed() ) );
    connect( z8Input, SIGNAL( returnPressed() ), this, SLOT( z8Input_returnPressed() ) );
    connect( z9Input, SIGNAL( returnPressed() ), this, SLOT( z9Input_returnPressed() ) );
    connect( z7Input, SIGNAL( returnPressed() ), this, SLOT( z7Input_returnPressed() ) );
    connect( kURLshapeSave, SIGNAL( openFileDialog(KURLRequester*) ), this, SLOT( kURLshapeSave_openFileDialog(KURLRequester*) ) );
    connect( kURLshapeLoad, SIGNAL( openFileDialog(KURLRequester*) ), this, SLOT( kURLshapeLoad_openFileDialog(KURLRequester*) ) );
    connect( kURLcommandVector, SIGNAL( openFileDialog(KURLRequester*) ), this, SLOT( kURLcommandVector_openFileDialog(KURLRequester*) ) );
    connect( kURLrecMatrix, SIGNAL( openFileDialog(KURLRequester*) ), this, SLOT( kURLrecMatrix_openFileDialog(KURLRequester*) ) );
    connect( kURLgainVector, SIGNAL( openFileDialog(KURLRequester*) ), this, SLOT( kURLgainVector_openFileDialog(KURLRequester*) ) );
    connect( kURLdisturbSelect, SIGNAL( openFileDialog(KURLRequester*) ), this, SLOT( kURLdisturbSelect_openFileDialog(KURLRequester*) ) );
    connect( buttonGainZero, SIGNAL( clicked() ), this, SLOT( buttonGainZero_clicked() ) );
    connect( buttonGainTTDown, SIGNAL( clicked() ), this, SLOT( buttonGainTTDown_clicked() ) );
    connect( buttonGainTTUp, SIGNAL( clicked() ), this, SLOT( buttonGainTTUp_clicked() ) );
    connect( buttonSetFocalStation, SIGNAL( clicked() ), this, SLOT( buttonSetFocalStation_clicked() ) );
    connect( buttonOffloadEnable, SIGNAL( clicked() ), this, SLOT( buttonOffloadEnable_clicked() ) );
    connect( buttonOffloadDisable, SIGNAL( clicked() ), this, SLOT( buttonOffloadDisable_clicked() ) );
    connect( buttonHOOOffloadEnable, SIGNAL( clicked() ), this, SLOT( buttonHOOOffloadEnable_clicked() ) );
    connect( buttonHOOffloadDisable, SIGNAL( clicked() ), this, SLOT( buttonHOOffloadDisable_clicked() ) );
    connect( buttonAccEnable, SIGNAL( clicked() ), this, SLOT( buttonAccEnable_clicked() ) );
    connect( buttonAccDisable, SIGNAL( clicked() ), this, SLOT( buttonAccDisable_clicked() ) );
    connect( comboModalBasis, SIGNAL( activated(const QString&) ), this, SLOT( comboModalBasis_activated(const QString&) ) );
    connect( buttonGainHO1Down, SIGNAL( clicked() ), this, SLOT( buttonGainHO1Down_clicked() ) );
    connect( buttonGainHO1Up, SIGNAL( clicked() ), this, SLOT( buttonGainHO1Up_clicked() ) );
    connect( buttonGainHO2Down, SIGNAL( clicked() ), this, SLOT( buttonGainHO2Down_clicked() ) );
    connect( buttonGainHO2Up, SIGNAL( clicked() ), this, SLOT( buttonGainHO2Up_clicked() ) );
    connect( buttonGain10modes, SIGNAL( clicked() ), this, SLOT( buttonGain10modes_clicked() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AdSecControlGui::~AdSecControlGui()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AdSecControlGui::languageChange()
{
    setCaption( tr( "AdSec Control GUI" ) );
    commandButtonsGroup->setTitle( tr( "Commands" ) );
    onButton->setText( tr( "On" ) );
    offButton->setText( tr( "Off" ) );
    loadProgramButton->setText( tr( "LoadProgram" ) );
    calibrateButton->setText( tr( "Calibrate" ) );
    setFlatAoButton->setText( tr( "SetFlatAo" ) );
    restButton->setText( tr( "Rest" ) );
    aoToSl->setText( tr( "AoToSl" ) );
    slToAoButton->setText( tr( "SlToAo" ) );
    runAoButton->setText( tr( "RunAo" ) );
    pauseAoButton->setText( tr( "PauseAo" ) );
    resumeAoButton->setText( tr( "ResumeAo" ) );
    stopAoButton->setText( tr( "StopAo" ) );
    recoverFailButton->setText( tr( "Recover Fail" ) );
    textLabel1_2->setText( tr( "<p align=\"center\">Unused</p>" ) );
    setFlatChopButton->setText( tr( "SetFlatChop" ) );
    setFlatSlButton->setText( tr( "SetFlatSl" ) );
    resetChopButton->setText( tr( "ResetChop" ) );
    resetSlButton->setText( tr( "ResetSl" ) );
    chopToSlButton->setText( tr( "ChopToSl" ) );
    slToChopButton->setText( tr( "SlToChop" ) );
    chopToAoButton->setText( tr( "ChopToAo" ) );
    aoToChopButton->setText( tr( "AoToChop" ) );
    runChopButton->setText( tr( "RunChop" ) );
    stopChopButton->setText( tr( "StopChop" ) );
    resetButton->setText( tr( "Reset" ) );
    resetAoButton->setText( tr( "ResetAo" ) );
    textLabel2_2->setText( QString::null );
    textLabel2_2_2->setText( tr( "AdSec Arbitrator Status:" ) );
    textLabel2_2_2_2->setText( tr( "Last command:" ) );
    labModeLabel->setText( tr( "LAB MODE ENABLED" ) );
    adSecArbitratorStatusLabel->setText( tr( "UNDEFINED" ) );
    focalStationLabel->setText( tr( "UNDEFINED" ) );
    lastCommandLabel->setText( tr( "UNDEFINED" ) );
    lastCommandResultLabel->setText( tr( "UNDEFINED" ) );
    textLabel1_3->setText( tr( "Wind [m/s]" ) );
    textLabel1_3_2->setText( tr( "Elevation [deg]" ) );
    labelCheckDew->setText( tr( "Swing arm status:" ) );
    groupBox4->setTitle( tr( "General Mirror Status" ) );
    ShellRipStatus->setText( tr( "<p align=\"center\">SHELL<br>RIP</p>" ) );
    forceRip->setText( tr( "Force RIP SHELL" ) );
    TssStatus->setText( tr( "<p align=\"center\">TSS<br>\n"
"OFF</p>" ) );
    ForceTssOn->setText( tr( "Force enable TSS" ) );
    PowerStatus->setText( tr( "<p align=\"center\">POWER<br>\n"
"OFF</p>" ) );
    ForcePowerOff->setText( tr( "Force POWER OFF" ) );
    labelSkipStatus->setText( tr( "Safe skip" ) );
    buttonGroup9->setTitle( tr( "Shape control" ) );
    textLabel10->setText( tr( "Last loaded shape:" ) );
    textLabel5->setText( tr( "Load shape:" ) );
    textLabel6->setText( tr( "Save shape:" ) );
    textLabel4_2_2->setText( tr( "Commands vector:" ) );
    curShape->setText( tr( "curShape" ) );
    shapeLoad->setText( tr( "Load" ) );
    shapeSaveButton->setText( tr( "Save" ) );
    commandApplyVector->setText( tr( "Apply" ) );
    checkBoxDefault->setText( tr( "Set as default" ) );
    buttonGroup7->setTitle( tr( "Automatic Shape Correction Controls" ) );
    textLabel1_4_2->setText( tr( "LO Offload to TCS" ) );
    textLabel1_4->setText( tr( "HO Auto-Offload" ) );
    textLabel1_4_3->setText( tr( "Accelerometer correction" ) );
    labelOffloadStatus->setText( QString::null );
    labelHOOffloadStatus->setText( QString::null );
    labelAccStatus->setText( QString::null );
    buttonOffloadEnable->setText( tr( "Enable" ) );
    buttonOffloadDisable->setText( tr( "Disable" ) );
    buttonHOOOffloadEnable->setText( tr( "Enable" ) );
    buttonHOOffloadDisable->setText( tr( "Disable" ) );
    buttonAccEnable->setText( tr( "Enable" ) );
    buttonAccDisable->setText( tr( "Disable" ) );
    buttonGroup7_2->setTitle( tr( "Zernike Application" ) );
    textLabel1_5->setText( tr( "Correct MODES [um] WF RMS" ) );
    textLabel1_6->setText( tr( "Total correction:" ) );
    textLabel1_6_2->setText( tr( "Delta correction to apply:" ) );
    z2label->setText( tr( "<p align=\"center\">Z2</p>" ) );
    z2Total->setText( tr( "<p align=\"center\">-</p>" ) );
    z2Applied->setText( tr( "<p align=\"center\">-</p>" ) );
    z2Input->setText( tr( "0.0" ) );
    z3label->setText( tr( "<p align=\"center\">Z3</p>" ) );
    z3Total->setText( tr( "<p align=\"center\">-</p>" ) );
    z3Applied->setText( tr( "<p align=\"center\">-</p>" ) );
    z3Input->setText( tr( "0.0" ) );
    z5label->setText( tr( "<p align=\"center\">Z5</p>" ) );
    z5Total->setText( tr( "<p align=\"center\">-</p>" ) );
    z5Applied->setText( tr( "<p align=\"center\">-</p>" ) );
    z5Input->setText( tr( "0.0" ) );
    z6label->setText( tr( "<p align=\"center\">Z6</p>" ) );
    z6Total->setText( tr( "<p align=\"center\">-</p>" ) );
    z6Applied->setText( tr( "<p align=\"center\">-</p>" ) );
    z6Input->setText( tr( "0.0" ) );
    z7label->setText( tr( "<p align=\"center\">Z7</p>" ) );
    z7Total->setText( tr( "<p align=\"center\">-</p>" ) );
    z7Applied->setText( tr( "<p align=\"center\">-</p>" ) );
    z7Input->setText( tr( "0.0" ) );
    z8label->setText( tr( "<p align=\"center\">Z8</p>" ) );
    z8Total->setText( tr( "<p align=\"center\">-</p>" ) );
    z8Applied->setText( tr( "<p align=\"center\">-</p>" ) );
    z8Input->setText( tr( "0.0" ) );
    z9label->setText( tr( "<p align=\"center\">Z9</p>" ) );
    z9Total->setText( tr( "<p align=\"center\">-</p>" ) );
    z9Applied->setText( tr( "<p align=\"center\">-</p>" ) );
    z9Input->setText( tr( "0.0" ) );
    zernApply->setText( tr( "Apply" ) );
    shapeTab->changeTab( tab, tr( "Shape Control" ) );
    groupBox5->setTitle( tr( "Standard Reconstructor" ) );
    textLabel1->setText( tr( "Modal basis:" ) );
    textLabel2->setText( tr( "Rec matrix:" ) );
    textLabel3->setText( tr( "Filtering:" ) );
    curModalBasis->setText( tr( "curModalBasis" ) );
    curRecMatrix->setText( tr( "curRecMatrix" ) );
    curFiltering->setText( tr( "curFiltering" ) );
    comboModalBasis->clear();
    comboModalBasis->insertItem( tr( "Mirror standard" ) );
    comboModalBasis->insertItem( tr( "Fernando KL" ) );
    comboModalBasis->insertItem( tr( "Guido Kalman" ) );
    comboFiltering->clear();
    comboFiltering->insertItem( tr( "Pure integrator" ) );
    comboFiltering->insertItem( tr( "Integrator + TT kalman filtering" ) );
    buttonApply->setText( tr( "Apply" ) );
    buttonGroup6->setTitle( tr( "Kalman Reconstructor" ) );
    buttonKalmanApply->setText( tr( "Apply" ) );
    buttonGroup4->setTitle( tr( "Gain vector" ) );
    textLabel4_2->setText( tr( "Current:" ) );
    textLabel5_2->setText( tr( "Change:" ) );
    curGainVector->setText( tr( "curGainVector" ) );
    gainApplyButton->setText( tr( "Apply" ) );
    buttonGainZero->setText( tr( "Zero gain" ) );
    buttonGain10modes->setText( tr( "10 modes" ) );
    textLabel1_7->setText( tr( "TT:" ) );
    buttonGainTTUp->setText( tr( "+" ) );
    buttonGainTTDown->setText( tr( "-" ) );
    labelHO1->setText( tr( "HO1:" ) );
    buttonGainHO1Up->setText( tr( "+" ) );
    buttonGainHO1Down->setText( tr( "-" ) );
    labelHO2->setText( tr( "HO2:" ) );
    buttonGainHO2Up->setText( tr( "+" ) );
    buttonGainHO2Down->setText( tr( "-" ) );
    buttonGroup3_2->setTitle( tr( "Disturbance" ) );
    textLabel7_2->setText( tr( "Current:" ) );
    textLabel7_2_2->setText( tr( "Change:" ) );
    curDisturb->setText( tr( "curDisturb" ) );
    buttonDisturbSend->setText( tr( "Send" ) );
    shapeTab->changeTab( tab_2, tr( "Reconstructor Control" ) );
    buttonGroupFS->setTitle( tr( "Switch input port and data bouncer settings" ) );
    buttonSetFocalStation->setText( tr( "Set" ) );
    shapeTab->changeTab( TabPage, tr( "Focal station" ) );
    shapeTab->changeTab( TabPage_2, tr( "Diagnostic Info" ) );
    alertsBox->setTitle( tr( "AdSecArbitrator Log" ) );
    messageLabel->setText( tr( "MSGD appears to be offline..." ) );
}

void AdSecControlGui::forceDisable_clicked()
{
    qWarning( "AdSecControlGui::forceDisable_clicked(): Not implemented yet" );
}

void AdSecControlGui::ForceTssOn_toggled(bool)
{
    qWarning( "AdSecControlGui::ForceTssOn_toggled(bool): Not implemented yet" );
}

void AdSecControlGui::ForcePowerOff_clicked()
{
    qWarning( "AdSecControlGui::ForcePowerOff_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonOffloadOff_clicked()
{
    qWarning( "AdSecControlGui::buttonOffloadOff_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonOffloadOn_clicked()
{
    qWarning( "AdSecControlGui::buttonOffloadOn_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonRecoverSkipFrame_clicked()
{
    qWarning( "AdSecControlGui::buttonRecoverSkipFrame_clicked(): Not implemented yet" );
}

void AdSecControlGui::gainApplyButton_clicked()
{
    qWarning( "AdSecControlGui::gainApplyButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonDisturbSend_clicked()
{
    qWarning( "AdSecControlGui::buttonDisturbSend_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonKalmanApply_clicked()
{
    qWarning( "AdSecControlGui::buttonKalmanApply_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonApply_clicked()
{
    qWarning( "AdSecControlGui::buttonApply_clicked(): Not implemented yet" );
}

void AdSecControlGui::commandApplyVector_clicked()
{
    qWarning( "AdSecControlGui::commandApplyVector_clicked(): Not implemented yet" );
}

void AdSecControlGui::shapeSaveButton_clicked()
{
    qWarning( "AdSecControlGui::shapeSaveButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::shapeLoad_clicked()
{
    qWarning( "AdSecControlGui::shapeLoad_clicked(): Not implemented yet" );
}

void AdSecControlGui::onButton_clicked()
{
    qWarning( "AdSecControlGui::onButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::offButton_clicked()
{
    qWarning( "AdSecControlGui::offButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::loadProgramButton_clicked()
{
    qWarning( "AdSecControlGui::loadProgramButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::calibrateButton_clicked()
{
    qWarning( "AdSecControlGui::calibrateButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::resetButton_clicked()
{
    qWarning( "AdSecControlGui::resetButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::setFlatAoButton_clicked()
{
    qWarning( "AdSecControlGui::setFlatAoButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::restButton_clicked()
{
    qWarning( "AdSecControlGui::restButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::resetAoButton_clicked()
{
    qWarning( "AdSecControlGui::resetAoButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::aoToSl_clicked()
{
    qWarning( "AdSecControlGui::aoToSl_clicked(): Not implemented yet" );
}

void AdSecControlGui::slToAoButton_clicked()
{
    qWarning( "AdSecControlGui::slToAoButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::runAoButton_clicked()
{
    qWarning( "AdSecControlGui::runAoButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::pauseAoButton_clicked()
{
    qWarning( "AdSecControlGui::pauseAoButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::resumeAoButton_clicked()
{
    qWarning( "AdSecControlGui::resumeAoButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::stopAoButton_clicked()
{
    qWarning( "AdSecControlGui::stopAoButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::recoverFailButton_clicked()
{
    qWarning( "AdSecControlGui::recoverFailButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::setFlatChopButton_clicked()
{
    qWarning( "AdSecControlGui::setFlatChopButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::setFlatSlButton_clicked()
{
    qWarning( "AdSecControlGui::setFlatSlButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::resetChopButton_clicked()
{
    qWarning( "AdSecControlGui::resetChopButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::resetSlButton_clicked()
{
    qWarning( "AdSecControlGui::resetSlButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::chopToSlButton_released()
{
    qWarning( "AdSecControlGui::chopToSlButton_released(): Not implemented yet" );
}

void AdSecControlGui::slToChopButton_released()
{
    qWarning( "AdSecControlGui::slToChopButton_released(): Not implemented yet" );
}

void AdSecControlGui::chopToAoButton_released()
{
    qWarning( "AdSecControlGui::chopToAoButton_released(): Not implemented yet" );
}

void AdSecControlGui::aoToChopButton_clicked()
{
    qWarning( "AdSecControlGui::aoToChopButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::runChopButton_clicked()
{
    qWarning( "AdSecControlGui::runChopButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::stopChopButton_clicked()
{
    qWarning( "AdSecControlGui::stopChopButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::forceRip_clicked()
{
    qWarning( "AdSecControlGui::forceRip_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonHOOOffloadOn_clicked()
{
    qWarning( "AdSecControlGui::buttonHOOOffloadOn_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonHOOffloadOff_clicked()
{
    qWarning( "AdSecControlGui::buttonHOOffloadOff_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonAccOn_clicked()
{
    qWarning( "AdSecControlGui::buttonAccOn_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonAccOff_clicked()
{
    qWarning( "AdSecControlGui::buttonAccOff_clicked(): Not implemented yet" );
}

void AdSecControlGui::chopToAoButton_clicked()
{
    qWarning( "AdSecControlGui::chopToAoButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::slToChopButton_clicked()
{
    qWarning( "AdSecControlGui::slToChopButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::chopToSlButton_clicked()
{
    qWarning( "AdSecControlGui::chopToSlButton_clicked(): Not implemented yet" );
}

void AdSecControlGui::zernApply_clicked()
{
    qWarning( "AdSecControlGui::zernApply_clicked(): Not implemented yet" );
}

void AdSecControlGui::z2_returnPressed()
{
    qWarning( "AdSecControlGui::z2_returnPressed(): Not implemented yet" );
}

void AdSecControlGui::z3_returnPressed()
{
    qWarning( "AdSecControlGui::z3_returnPressed(): Not implemented yet" );
}

void AdSecControlGui::z5_returnPressed()
{
    qWarning( "AdSecControlGui::z5_returnPressed(): Not implemented yet" );
}

void AdSecControlGui::z6_returnPressed()
{
    qWarning( "AdSecControlGui::z6_returnPressed(): Not implemented yet" );
}

void AdSecControlGui::z7_returnPressed()
{
    qWarning( "AdSecControlGui::z7_returnPressed(): Not implemented yet" );
}

void AdSecControlGui::lineEdit1_5_textChanged(const QString&)
{
    qWarning( "AdSecControlGui::lineEdit1_5_textChanged(const QString&): Not implemented yet" );
}

void AdSecControlGui::z8_textChanged(const QString&)
{
    qWarning( "AdSecControlGui::z8_textChanged(const QString&): Not implemented yet" );
}

void AdSecControlGui::z9_returnPressed()
{
    qWarning( "AdSecControlGui::z9_returnPressed(): Not implemented yet" );
}

void AdSecControlGui::z2Input_returnPressed()
{
    qWarning( "AdSecControlGui::z2Input_returnPressed(): Not implemented yet" );
}

void AdSecControlGui::z3Input_returnPressed()
{
    qWarning( "AdSecControlGui::z3Input_returnPressed(): Not implemented yet" );
}

void AdSecControlGui::z5Input_returnPressed()
{
    qWarning( "AdSecControlGui::z5Input_returnPressed(): Not implemented yet" );
}

void AdSecControlGui::z6Input_returnPressed()
{
    qWarning( "AdSecControlGui::z6Input_returnPressed(): Not implemented yet" );
}

void AdSecControlGui::z8Input_returnPressed()
{
    qWarning( "AdSecControlGui::z8Input_returnPressed(): Not implemented yet" );
}

void AdSecControlGui::z9Input_returnPressed()
{
    qWarning( "AdSecControlGui::z9Input_returnPressed(): Not implemented yet" );
}

void AdSecControlGui::z7Input_returnPressed()
{
    qWarning( "AdSecControlGui::z7Input_returnPressed(): Not implemented yet" );
}

void AdSecControlGui::kURLshapeSave_openFileDialog(KURLRequester*)
{
    qWarning( "AdSecControlGui::kURLshapeSave_openFileDialog(KURLRequester*): Not implemented yet" );
}

void AdSecControlGui::kURLshapeLoad_openFileDialog(KURLRequester*)
{
    qWarning( "AdSecControlGui::kURLshapeLoad_openFileDialog(KURLRequester*): Not implemented yet" );
}

void AdSecControlGui::kURLcommandVector_openFileDialog(KURLRequester*)
{
    qWarning( "AdSecControlGui::kURLcommandVector_openFileDialog(KURLRequester*): Not implemented yet" );
}

void AdSecControlGui::radiofsNONE_clicked()
{
    qWarning( "AdSecControlGui::radiofsNONE_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonGroupFS_clicked(int)
{
    qWarning( "AdSecControlGui::buttonGroupFS_clicked(int): Not implemented yet" );
}

void AdSecControlGui::kURLrecMatrix_openFileDialog(KURLRequester*)
{
    qWarning( "AdSecControlGui::kURLrecMatrix_openFileDialog(KURLRequester*): Not implemented yet" );
}

void AdSecControlGui::kURLgainVector_openFileDialog(KURLRequester*)
{
    qWarning( "AdSecControlGui::kURLgainVector_openFileDialog(KURLRequester*): Not implemented yet" );
}

void AdSecControlGui::kURLdisturbSelect_openFileDialog(KURLRequester*)
{
    qWarning( "AdSecControlGui::kURLdisturbSelect_openFileDialog(KURLRequester*): Not implemented yet" );
}

void AdSecControlGui::buttonGainZero_clicked()
{
    qWarning( "AdSecControlGui::buttonGainZero_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonGainHODown_clicked()
{
    qWarning( "AdSecControlGui::buttonGainHODown_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonGainHOUp_clicked()
{
    qWarning( "AdSecControlGui::buttonGainHOUp_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonGainTTDown_clicked()
{
    qWarning( "AdSecControlGui::buttonGainTTDown_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonGainTTUp_clicked()
{
    qWarning( "AdSecControlGui::buttonGainTTUp_clicked(): Not implemented yet" );
}

void AdSecControlGui::pushSetFocalStation_clicked()
{
    qWarning( "AdSecControlGui::pushSetFocalStation_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonSetFocalStation_clicked()
{
    qWarning( "AdSecControlGui::buttonSetFocalStation_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonOffloadEnable_clicked()
{
    qWarning( "AdSecControlGui::buttonOffloadEnable_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonOffloadDisable_clicked()
{
    qWarning( "AdSecControlGui::buttonOffloadDisable_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonHOOOffloadEnable_clicked()
{
    qWarning( "AdSecControlGui::buttonHOOOffloadEnable_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonHOOffloadDisable_clicked()
{
    qWarning( "AdSecControlGui::buttonHOOffloadDisable_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonAccEnable_clicked()
{
    qWarning( "AdSecControlGui::buttonAccEnable_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonAccDisable_clicked()
{
    qWarning( "AdSecControlGui::buttonAccDisable_clicked(): Not implemented yet" );
}

void AdSecControlGui::comboModalBasis_activated(const QString&)
{
    qWarning( "AdSecControlGui::comboModalBasis_activated(const QString&): Not implemented yet" );
}

void AdSecControlGui::buttonGainHO1Down_clicked()
{
    qWarning( "AdSecControlGui::buttonGainHO1Down_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonGainHO1Up_clicked()
{
    qWarning( "AdSecControlGui::buttonGainHO1Up_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonGainHO2Down_clicked()
{
    qWarning( "AdSecControlGui::buttonGainHO2Down_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonGainHO2Up_clicked()
{
    qWarning( "AdSecControlGui::buttonGainHO2Up_clicked(): Not implemented yet" );
}

void AdSecControlGui::buttonGain10modes_clicked()
{
    qWarning( "AdSecControlGui::buttonGain10modes_clicked(): Not implemented yet" );
}

