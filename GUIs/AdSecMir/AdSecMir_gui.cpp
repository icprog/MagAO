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
	#include <string.h>
	#include <time.h>
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
#include <iostream>

/*
#include "Logger.h"

Logger*		    _logger = Logger::get("MAIN", Logger::LOG_LEV_DEBUG);	// Use the same name of the AOApp main logger
*/

#include "Mirror.h"
#include "AdSecMir_gui.h"

#include "PlotModes.h"


#include <qpainter.h>
#include <qvariant.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <kled.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qmessagebox.h>
#include <qwhatsthis.h>
//#include "kled.h"
//#include "qwt_plot.h"
//#include "qwt_text.h"
//#include "qwt_text_label.h"
//#include "qwt_legend.h"
//#include "qwt_legend_item.h"
//#include "qwt_plot_curve.h"

long FastdiagnN=0;
long HousekeeperN=0;
double HframeRate;     //frame rate fromf Housekeeper application
double HtimePassed;    //time passed from last change of Houskeeper frame counter
char *FrameCounter= (char *)"DiagnAppFrameCounter";
char *FrameRate   = (char *)"DiagnAppFrameRate"; //frame rate in HZ

char *FamilyNames[NVARS] = {
(char *)"CHDISTAVERAGE",
(char *)"CHDISTRMS",
(char *)"CHCURRAVERAGE",
(char *)"CHCURRRMS",
(char *)"CHINTCONTROLCURRENT",
(char *)"CHFFPURECURRENT",
(char *)"CHNEWDELTACOMMAND",
(char *)"CHFFCOMMAND",
(char *)"MODES"
};
int Ival[NVARS]={
0,1,       //positions
2,3,4,5,   //currents
8,9,       //commands
12         //modes
};

char *FamilyNames1[NVARS1] = {
(char *)"DSPBIASCURRENT",
(char *)"DSPFMOL",
(char *)"DSPBIASCOMMAND",
(char *)"DSPCMOL",
};
char *Variant[2] = {(char *)"A",(char *)"B"};
int Ival1[NVARS1]={
6,7,    //currents
10,11   //commands
};

char *FamilyNames2[NVARS2] = {
(char *)"SwitchOffloadSelector",
(char *)"SwitchWFSFrameCounter",
(char *)"SwitchMirrFramesCounter",
(char *)"SwitchSafeSkipFrameCnt",
(char *)"SwitchPendingSkipFrameCnt",
(char *)"SwitchWFSGlobalTimeout",
(char *)"SwitchNumFLTTimeout",
(char *)"SwitchNumCrcErr",
(char *)"SwitchParamSelector"
};
char *FamilyNames2Help[NVARS2] = {
(char *)"not available\n",
(char *)"not available\n",
(char *)"not available\n",
(char *)"not available\n",
(char *)"not available\n",
(char *)"not available\n",
(char *)"not available\n",
(char *)"not available\n",
(char *)"not available\n"
};


int convert_att[672]={
504,505,506,507,585,
586,587,588,360,361,
362,363,429,430,431,
432,509,510,511,590,
591,592,593,594,364,
433,434,435,436,508,
512,589,513,514,515,
516,595,596,597,598,
367,368,369,370,437,
438,439,440,246,247,
248,249,304,305,306,
307,298,299,300,301,
302,303,365,366,189,
240,241,242,243,244,
245,297,144,145,146,
147,190,191,192,193,
148,149,150,151,194,
195,196,197, 72,105,
106,107,108,109,110,
111, 47, 48, 49, 73,
 74, 75, 76, 77,  0,
  9, 10, 24, 25, 26,
 45, 46,527,528,529,
530,610,611,612,613,
379,380,381,382,450,
451,452,453,523,524,
525,604,605,606,607,
608,378,446,447,448,
449,522,526,609,518,
519,520,599,600,601,
602,603,371,441,442,
443,444,445,517,521,
250,308,309,310,372,
373,374,375,311,312,
313,314,315,316,376,
377,205,253,254,255,
256,257,258,317,155,
156,157,158,201,202,
203,204,152,153,154,
198,199,200,251,252,
 78, 79,112,113,114,
115,116,117, 30, 50,
 51, 52, 53, 80, 81,
 82,  1,  2, 11, 12,
 13, 27, 28, 29,531,
532,533,534,614,615,
616,617,383,384,385,
386,454,455,456,457,
536,537,538,619,620,
621,622,623,387,458,
459,460,461,535,539,
618,540,541,542,543,
624,625,626,627,390,
391,392,393,462,463,
464,465,265,266,267,
268,325,326,327,328,
319,320,321,322,323,
324,388,389,206,259,
260,261,262,263,264,
318,159,160,161,162,
207,208,209,210,163,
164,165,166,211,212,
213,214, 83,118,119,
120,121,122,123,124,
 56, 57, 58, 84, 85,
 86, 87, 88,  3, 14,
 15, 31, 32, 33, 54,
 55,554,555,556,557,
639,640,641,642,402,
403,404,405,475,476,
477,478,550,551,552,
633,634,635,636,637,
401,471,472,473,474,
549,553,638,545,546,
547,628,629,630,631,
632,394,466,467,468,
469,470,544,548,269,
329,330,331,395,396,
397,398,332,333,334,
335,336,337,399,400,
222,272,273,274,275,
276,277,338,170,171,
172,173,218,219,220,
221,167,168,169,215,
216,217,270,271, 89,
 90,125,126,127,128,
129,130, 37, 59, 60,
 61, 62, 91, 92, 93,
  4,  5, 16, 17, 18,
 34, 35, 36,558,559,
560,561,643,644,645,
646,406,407,408,409,
479,480,481,482,563,
564,565,648,649,650,
651,652,410,483,484,
485,486,562,566,647,
567,568,569,570,653,
654,655,656,413,414,
415,416,487,488,489,
490,284,285,286,287,
346,347,348,349,340,
341,342,343,344,345,
411,412,223,278,279,
280,281,282,283,339,
174,175,176,177,224,
225,226,227,178,179,
180,181,228,229,230,
231, 94,131,132,133,
134,135,136,137, 65,
 66, 67, 95, 96, 97,
 98, 99,  6, 19, 20,
 38, 39, 40, 63, 64,
581,582,583,584,668,
669,670,671,425,426,
427,428,500,501,502,
503,577,578,579,662,
663,664,665,666,424,
496,497,498,499,576,
580,667,572,573,574,
657,658,659,660,661,
417,491,492,493,494,
495,571,575,288,350,
351,352,418,419,420,
421,353,354,355,356,
357,358,422,423,239,
291,292,293,294,295,
296,359,185,186,187,
188,235,236,237,238,
182,183,184,232,233,
234,289,290,100,101,
138,139,140,141,142,
143, 44, 68, 69, 70,
 71,102,103,104,  7,
  8, 21, 22, 23, 41,
 42, 43
};

/*
 *  Constructs a GUIadSecM as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
GUIadSecM::GUIadSecM(string id, string configFile, QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ), AOApp(id, configFile)
{
    int i;
    char temp[8];
    char tmp1[24];
    char tmp2[18];


    if ( !name ) 	setName( "GUIadSecM" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setSizeGripEnabled( TRUE );
    GUIadSecMLayout = new QVBoxLayout( this, 11, 6, "GUIadSecMLayout"); 

    tabWidget = new QTabWidget( this, "tabWidget" );

    // current values
    tab1 = new QWidget( tabWidget, "tab1" );
    tab1Layout = new QHBoxLayout( tab1, 11, 6, "tab1Layout"); 

    mir[0] = new Mirror(tab1,"test");
    mir[0]->setFrameShape (QFrame::TabWidgetPanel);
    mir[0]->setFrameShadow (QFrame::Raised);
    tab1Layout->addWidget( mir[0] );
    mir[0]->init(1,0,14,140,215,10,160);
    mir[0]->init(0,0,14,140,215,10,160);

    mir[1] = new Mirror(tab1,"test");
    tab1Layout->addWidget( mir[1] );
    mir[1]->init(7,0,14,140,215,10,160);
    mir[1]->init(6,0,14,140,215,10,160);
    mir[1]->init(5,0,14,140,215,10,160);
    mir[1]->init(4,0,14,140,215,10,160);
    mir[1]->init(3,0,14,140,215,10,160);
    mir[1]->init(2,0,14,140,215,10,160);

    mir[2] = new Mirror(tab1,"test");
    tab1Layout->addWidget( mir[2] );
    mir[2]->init(11,0,14,140,215,10,160);
    mir[2]->init(10,0,14,140,215,10,160);
    mir[2]->init(9,0,14,140,215,10,160);
    mir[2]->init(8,0,14,140,215,10,160);

    NMirShown=3;

    tabWidget->insertTab( tab1, QString::fromLatin1("") );

    //   Running Mean
    tab2 = new QWidget( tabWidget, "tab2" );
    tab2Layout = new QHBoxLayout( tab2, 11, 6, "tab2Layout");

    mir[3] = new Mirror(tab2,"test");
    tab2Layout->addWidget( mir[3] );
    mir[3]->init(1,1,14,140,215,10,160);
    mir[3]->init(0,1,14,140,215,10,160);

    mir[4] = new Mirror(tab2,"test");
    tab2Layout->addWidget( mir[4] );
    mir[4]->init(7,1,14,140,215,10,160);
    mir[4]->init(6,1,14,140,215,10,160);
    mir[4]->init(5,1,14,140,215,10,160);
    mir[4]->init(4,1,14,140,215,10,160);
    mir[4]->init(3,1,14,140,215,10,160);
    mir[4]->init(2,1,14,140,215,10,160);
    //mir[4]->updateRings(0,14);

    mir[5] = new Mirror(tab2,"test");
    tab2Layout->addWidget( mir[5] );
    mir[5]->init(11,1,14,140,215,10,160);
    mir[5]->init(10,1,14,140,215,10,160);
    mir[5]->init(9,1,14,140,215,10,160);
    mir[5]->init(8,1,14,140,215,10,160);


    tabWidget->insertTab( tab2, QString::fromLatin1("") );


    GUIadSecMLayout->addWidget( tabWidget );

    frameInfo = new QFrame (this,"FrameInfo");
    frameInfo->setSizePolicy(QSizePolicy( (QSizePolicy::SizeType)7,(QSizePolicy::SizeType)0,0,0,frameInfo->sizePolicy().hasHeightForWidth() ) );
    frameInfo->setFrameShape(QFrame::TabWidgetPanel);
    frameInfo->setFrameShadow(QFrame::Raised);
    frameInfoLayout = new QGridLayout(frameInfo,1,1,11,6,"frameInfoLayout");
    strcpy(tmp1,FamilyNames2[0]);
    strcpy(tmp2,&tmp1[6]);
    strcat(tmp2,"=");
    infoLab[0] = new QLabel(frameInfo,tmp1);
    QWhatsThis::add (infoLab[0],FamilyNames2Help[0]);
    frameInfoLayout->addWidget(infoLab[0],0,0);
    infoLab[0]->setText( tmp2 );
    infoText[0] = new QLabel(frameInfo,tmp2);
    frameInfoLayout->addWidget(infoText[0],0,1);
    buttonModes = new QPushButton( frameInfo, "buttonModes" );
    buttonModes->setPaletteBackgroundColor( QColor( 255, 171, 148 ) );
    frameInfoLayout->addWidget(buttonModes,0,2);
    buttonModes->setText("Modes" );
    QWhatsThis::add (buttonModes,"pop-ups the plot with current MODES values\n");
    buttonModesMean = new QPushButton( frameInfo, "buttonModesMean" );
    QWhatsThis::add (buttonModesMean,"pop-ups the plot with MODES values,\n averaged by currently available Nsamples\nif the Nsamples-to-mean is not achieved yet\n");
    frameInfoLayout->addWidget(buttonModesMean,0,4);
    buttonModesMean->setText("Modes Mean" );
    buttonModesMean->setPaletteBackgroundColor( QColor( 255, 200, 160 ) );
    buttonModesRMS = new QPushButton( frameInfo, "buttonModesRMS" );
    QWhatsThis::add (buttonModesRMS,"pop-ups the plot with RootMeanSquare of current MODES values\n");
    frameInfoLayout->addWidget(buttonModesRMS,0,6);
    buttonModesRMS->setText("Modes RMS" );
    buttonModesRMS->setPaletteBackgroundColor( QColor( 255, 230, 190 ) );
    int i1=0;
    int i2=0;
    for (i=1;i<NVARS2;i++) {
        strcpy(tmp1,FamilyNames2[i]);
        strcpy(tmp2,&tmp1[6]);
        strcat(tmp2,"=");
        infoLab[i]  = new QLabel(frameInfo,tmp1);
        infoText[i] = new QLabel(frameInfo,tmp2);
        if (i<5) {
           frameInfoLayout->addWidget(infoLab[i] ,1,i1);
           frameInfoLayout->addWidget(infoText[i],1,i1+1);
           i1+=2;
        }
        if (i>4 ) {
           frameInfoLayout->addWidget(infoLab[i], 2,i2);
           frameInfoLayout->addWidget(infoText[i],2,i2+1);
           i2+=2;
        }
        infoLab[i]->setText( tmp2 );
        QWhatsThis::add (infoLab[i],FamilyNames2Help[i]);
    }
    GUIadSecMLayout->addWidget( frameInfo );
    // command buttons panel
    Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1"); 

    buttonStart = new QPushButton( this, "buttonStart" );
    QWhatsThis::add (buttonStart,"stop&start data refreshing\n");
    buttonStart->setPaletteBackgroundColor( QColor( 255, 255, 127 ) );
    QFont buttonStart_font(  buttonStart->font() );
    buttonStart_font.setFamily( "URW Gothic L" );
    buttonStart_font.setPointSize( 11 );
    buttonStart_font.setBold( TRUE );
    buttonStart->setFont( buttonStart_font ); 
    buttonStart->setAutoDefault( TRUE );
    Layout1->addWidget( buttonStart );
    QLabel* lab1 = new QLabel (this,"rrLab");
    QWhatsThis::add (lab1,"chose in the right window the Refresh Rate\n(rate of the interrogation of Fastdiagn&Housekeeper applications)\n & press RETURN\n");
    QFont lab1_font(  lab1->font() );
    //lab1_font.setFamily( "URW Gothic L" );
    lab1_font.setPointSize( 11 );
    lab1_font.setItalic( TRUE );
    lab1->setFont( lab1_font);
    lab1->setText( "at refresh rate(msec):");
    Layout1->addWidget( lab1 );
    lab1line = new QLineEdit (this,"rrEdit");
    QFont lab1line_font(  lab1line->font() );
    //lab1line_font.setFamily( "URW Gothic L" );
    lab1line_font.setPointSize( 11 );
    //lab1line_font.setItalic( TRUE );
    lab1line->setFont( lab1line_font);
    lab1line->setMaximumWidth(60);
    lab1line->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred);
    Layout1->addWidget( lab1line );
    QLabel* lab2 = new QLabel (this,"meanLab");
    QWhatsThis::add (lab2,"chose in the right window the Number of samples to mean & press RETURN\n");
    QFont lab2_font(  lab2->font() );
    //lab2_font.setFamily( "URW Gothic L" );
    lab2_font.setPointSize( 11 );
    lab2_font.setItalic( TRUE );
    lab2->setFont( lab2_font);
    lab2->setText( "Nsamples to mean =");
    Layout1->addWidget( lab2 );
    lab2line = new QLineEdit (this,"meanEdit");
    QFont lab2line_font(  lab2line->font() );
    //lab2line_font.setFamily( "URW Gothic L" );
    lab2line_font.setPointSize( 11 );
    //lab2line_font.setItalic( TRUE );
    lab2line->setFont( lab2line_font);
    lab2line->setMaximumWidth(60);
    lab2line->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred);
    Layout1->addWidget( lab2line );

    buttonOffset = new QPushButton( this, "buttonOffset" );
    QWhatsThis::add (buttonOffset,"take GLOBAL Offset, for ALL panels, ALL values\n");
    buttonOffset->setPaletteBackgroundColor( QColor( 255, 160,0 ) );
    QFont buttonOffset_font(  buttonOffset->font() );
    buttonOffset_font.setFamily( "URW Gothic L" );
    buttonOffset_font.setPointSize( 11 );
    buttonOffset_font.setBold( TRUE );
    buttonOffset->setFont( buttonOffset_font ); 
    buttonOffset->setAutoDefault( TRUE );
    Layout1->addWidget( buttonOffset );

    TimeF = new QLabel (this,"TimeLabelF");
    QWhatsThis::add (TimeF,"time when the Fastdiagn application has been interrogated.\nGreen if FrameCounter is changed, otherwise Red\n");
    QFont TimeFFont(TimeF->font());
    TimeFFont.setPointSize(8);
    TimeF->setFont( TimeFFont );
    Layout1->addWidget( TimeF );

    TimeH = new QLabel (this,"TimeLabelH");
    QWhatsThis::add (TimeH,"time when the Housekeeper application has been interrogated.\nGreen if FrameCounter is changed, otherwise Red\n");
    QFont TimeHFont(TimeH->font());
    TimeHFont.setPointSize(8);
    TimeH->setFont( TimeHFont );
    Layout1->addWidget( TimeH );

    Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( Horizontal_Spacing2 );

//    Horizontal_Spacing3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
//    Layout1->addItem( Horizontal_Spacing3 );

    buttonHelp = new QPushButton( this, "buttonHelp" );
    buttonHelp->setPaletteBackgroundColor( QColor( 171, 255, 148 ) );
    QFont buttonHelp_font(  buttonHelp->font() );
    buttonHelp_font.setFamily( "URW Gothic L" );
    buttonHelp_font.setPointSize( 11 );
    buttonHelp_font.setBold( TRUE );
    buttonHelp->setFont( buttonHelp_font ); 
    buttonHelp->setAutoDefault( TRUE );
    Layout1->addWidget( buttonHelp );

    buttonExit = new QPushButton( this, "buttonExit" );
    buttonExit->setPaletteBackgroundColor( QColor( 255, 170, 255 ) );
    QFont buttonExit_font(  buttonExit->font() );
    buttonExit_font.setFamily( "URW Gothic L" );
    buttonExit_font.setPointSize( 11 );
    buttonExit_font.setBold( TRUE );
    buttonExit->setFont( buttonExit_font ); 
    buttonExit->setAutoDefault( TRUE );
    Layout1->addWidget( buttonExit );
    GUIadSecMLayout->addLayout( Layout1 );

/* initialisation ***************************************************/
    timeoutR = 100; //   10/sec
    sprintf(temp,"%7d",timeoutR);
    lab1line->setText( temp );
    Nint=0;
    Nr=0;
    Nsamples=100;
    if (Nsamples>NRMAX) Nsamples=NRMAX;
    sprintf(temp,"%7d",Nsamples);
    lab2line->setText( temp );
    for (int i=0; i<NVARS; i++) Init[i]=0;
    for (int i=0; i<NVARS2; i++) {
        Vswitch[i]=0.;
        Vcolor [i]=0;
    }
    OffloadSel=0;
    iModes=8;  //Modes family
    Red   =  QColor( 200, 0, 0 );
    Green =  QColor( 0, 80, 0 );
    TimeFCol =  Red;
    TimeHCol =  Red;
    strcpy(time_F,"offline");
    strcpy(time_H,"offline");

/* initialisation **************************end**********************/

    languageChange();
    resize( QSize(1000, 590).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    timer = new QTimer (this);



    // signals and slots connections
    connect( buttonStart, SIGNAL( clicked() ), this, SLOT( StopStartTimer() ) );
    connect( lab1line, SIGNAL( returnPressed() ), this, SLOT( RefreshRate() ) );
    connect( lab2line, SIGNAL( returnPressed() ), this, SLOT( changeNsamples() ) );
    connect( buttonOffset, SIGNAL( clicked() ), this, SLOT(TakeOffset() ) );
    connect( buttonModes, SIGNAL( clicked() ), this, SLOT(CreatePlotModes() ) );
    connect( buttonModesMean, SIGNAL( clicked() ), this, SLOT(CreatePlotModesMean() ) );
    connect( buttonModesRMS, SIGNAL( clicked() ), this, SLOT(CreatePlotModesRMS() ) );
    connect( buttonExit, SIGNAL( clicked() ), this, SLOT( quit() ) );
    connect( timer, SIGNAL( timeout() ), this, SLOT( Refresh() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
GUIadSecM::~GUIadSecM()
{
    // no need to delete child widgets, Qt does it all for us
}


void GUIadSecM::paintEvent(QPaintEvent *)
{
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void GUIadSecM::languageChange()
{
    char temp[300];
    string side = Utils::uppercase(Utils::getAdoptSide());
    if (side == "R") setCaption( tr( "Adaptive Secondary Mirror Interface RIGHT" ) );
    else setCaption( tr( "Adaptive Secondary Mirror Interface LEFT" ) );


    tabWidget->changeTab( tab1, tr( "Current Values" ) );
    tabWidget->changeTab( tab2, tr( "Running Mean of Nsamples" ) );
    buttonStart->setText( tr( "&Stop Refresh" ) );
    buttonStart->setAccel( QKeySequence( tr( "Alt+S" ) ) );
    buttonOffset->setText( tr( "Take Offset" ) );
    buttonHelp->setText( tr( "&Help" ) );
    buttonHelp->setAccel( QKeySequence( tr( "Alt+H" ) ) );
    buttonExit->setText( tr( "&Exit" ) );
    buttonExit->setAccel( QKeySequence( tr( "Alt+E" ) ) );
    sprintf(temp,"Fastdiagn\n%s",time_F);
    TimeF->setText(tr(temp));
    sprintf(temp,"Housekeeper\n%s",time_H);
    TimeH->setText(tr(temp));
   TimeF->setPaletteForegroundColor( TimeFCol );
   TimeH->setPaletteForegroundColor( TimeHCol );
}


void GUIadSecM::StopStartTimer()
{
    if ( QString::compare(buttonStart->text(), "&Stop Refresh") == 0 ) {
       timer->stop();
       buttonStart->setText( tr( "&Start Refresh" ) );
       buttonStart->setPaletteBackgroundColor( QColor( 200, 0, 0 ) );
    } else {
       buttonStart->setText( tr( "&Stop Refresh" ) );
       buttonStart->setPaletteBackgroundColor( QColor( 255, 255, 127 ) );
       //re-read the params from fastdiagn
       for (int i=0; i<NVARS; i++) Init[i]=0;
       Nr=0; //re-start Running Mean too
       Refresh();
    }
}


void GUIadSecM::RefreshRate()
{
    bool OK;
    char temp[10];

    QString t = lab1line->text();
    int ti = t.toInt(&OK);
    if (OK) timeoutR = ti;
    if (timeoutR<1) timeoutR = 1;
    if (timeoutR>10000) timeoutR = 10000;
    sprintf(temp,"%7d",timeoutR);
    lab1line->setText( temp );
    if (timer->isActive() ) {
       timer->stop();
       timer->start (timeoutR,FALSE);
    }
}

void GUIadSecM::changeNsamples()
{
    bool OK;
    char temp[10];

    QString t = lab2line->text();
    int ti = t.toInt(&OK);
    if (OK) Nsamples = ti;
    if (Nsamples<2) Nsamples = 2;
    if (Nsamples>NRMAX) Nsamples = NRMAX;
    sprintf(temp,"%7d",Nsamples);
    lab2line->setText( temp );
    Nr=0; //re-start Running Mean too
}


void GUIadSecM::quit() //close everything
{
 //    SetTimeToDie(true);
 //    timer->stop();
     close();
}

void GUIadSecM::Run()
{
   // qWarning( "GUIadSecM::AOApp::Run(): skeep Run method of AOApp" );
}

void GUIadSecM::Refresh()
{
 //qWarning( "GUILBT672::Refresh: entered" );
// static int num=0;
// num++;
// printf( "Refresh num=%d\n",num );
     if ( TimeToDie()==true) quit(); //to support AOApp signals
     GetAll();
     SetAll();
     //start timer if the FastDiagn interrogation was NOT Stopped due to an error
     //remember that FIRST interrogation was made out of the timer loop
     if ( QString::compare(buttonStart->text(), "&Stop Refresh") == 0 &&  !timer->isActive() ) timer->start (timeoutR,FALSE);
}




/*  Get all necessary FastDiagn variables *************************************/
void GUIadSecM::GetAll()
{
    long time_d;
    char time_arr[27];
    char *time_str;

    char *family;
    char sfamily[200];
    int   from = -1;
    int   to   = -1;
    int   timeout  = 3000; // timeout in ms for Fastdiagn;
    int   timeout1 = 3000; // timeout in ms for Housekeeper;
    ParamDict pdict;
    ValueDict dict;

    string Family;
    const char *familyName;
    //int   Index;
    int ifam,jj;
    int i;
    double mean;
    //double std;
    char temp[100];

//printf ("GetAll\n");
//qWarning( "GUIadSecM::GetAll(): entered" );

    //char *diagnapp = (char *)"FASTDGN00";
    //char *diagnapp = (char *)"fastdiagn.L";
    string diagnapp = std::string("fastdiagn.") + Side();
    for  (ifam=0; ifam<NVARS+1; ifam++) {
         if (ifam<NVARS) family = FamilyNames[ifam];
         else family = FrameCounter;

         //first we get patameters  to check if some of actuators are disabled
         //we do this only ones at the begining
         if (Init[ifam]==0 && ifam<NVARS) {
            try{      //to define bad actuators from  param file
                DiagnWhich which(family, from, to);
                pdict = GetDiagnVarParam( (char *)diagnapp.c_str(),which, timeout);
            } catch (AOException &e) {
                    if ( timer->isActive() ) timer->stop();
                    buttonStart->setText( tr( "&Start Refresh" ) );
                    buttonStart->setPaletteBackgroundColor( QColor( 200, 0, 0 ) );
                    QMessageBox::information(this,"AdSecMir_gui::GetAll()",
                     "Unable to get the DiagnVarParam\nCheck whether the  fastdiagn  is running.");
                 GetAllDummy();
             return;
            }
            jj=-1;
            for (ParamDict::iterator it = pdict.begin(); it != pdict.end();  it++){
                 jj++;
                 DiagnParam pa = (*it).second;
                 int enabled = pa.isEnabled();
                 if (enabled==1) ibuf[ifam][convert_att[jj]] = 0; //enabled(=1) means the actuatuator is OK
                 else ibuf[ifam][convert_att[jj]] = 1;
/*
if (jj==0) {
printf("en/dis: ************ ifam=%d\n",ifam);
printf("jj=%d--%s--%s- bad=%d\n",jj,family,FamilyNames[ifam],bad);
}
*/
            }
            Init[ifam]=1;
         }
         //now we get values
         try{
             DiagnWhich which(family, from, to);
             dict = GetDiagnValue( (char *)diagnapp.c_str(),which, timeout);
         } catch (AOException &e) {
                 cerr <<  e.what() <<" fastdiagn\n";
                 if ( timer->isActive() ) timer->stop();
                 buttonStart->setText( tr( "&Start Refresh" ) );
                 buttonStart->setPaletteBackgroundColor( QColor( 200, 0, 0 ) );
                 QMessageBox::information(this,"AdSecMir_gui::GetAll()",
                  "Error in GetDiagnValue\nThe FastDiagn interrogation is INTERRUPTED");
           return;
         }

        jj=-1;
        //Iterate over the dictionary of the retrieved DiagnVars.
        for (ValueDict::iterator it = dict.begin(); it != dict.end();  it++){
             jj++;
///////printf("GETALL-----it= %i\n",jj);
             DiagnWhich wh = (*it).first;
             DiagnValue va = (*it).second;

             //DiagnVar name & index
             Family = wh.Family();   // DiagnVar name.
             familyName = Family.c_str();

             //va.Time();     // Time (average of the time stamp of the values in the running mean)
             //timec = va.Time();   //accumulate current value time  in sec?

             mean = va.Mean();     // DiagnVar value (average of a running mean)
//if (mean!=0)
//printf("-%s-jj=%d-timec=%f-mean=%f ",familyName,jj,timec,mean);
             if (ifam==NVARS) {
                if ((long)mean != FastdiagnN ) {
                   TimeFCol =  Green;
                   FastdiagnN=(long)mean;
                } else {
                   TimeFCol =  Red;
                }
                TimeF->setPaletteForegroundColor( TimeFCol );
             } else {
                 sprintf (temp,"%f",mean);
                 if (strcmp(temp,"nan")==0) mean = 0.;
                 if (ifam==0 )  mean = mean*1.e6;
                 if (ifam==1 )  mean = mean*1.e9;
                 if (ifam<=7 && ifam >5 )  mean = mean*1.e6;
                 //printf("jj=%d att=%d\n",jj,convert_att[jj]);
                 if (ifam!=8) buf[ifam][convert_att[jj]] = mean;
                 else buf[ifam][jj] = mean;  //modes are excluded
                 //std  = (float) va.Stddev();   // Stddev of the running mean
                 //sprintf(temp,"%f",std);
                 //if (strcmp(temp,"nan")==0) std = 0.;

                 //va.Last();     // Instantaneuos value (last value inserted in the running mean)
             }
        }
        time(&time_d);
        time_str = ctime(&time_d);
        sprintf(time_arr,"%s",time_str);
        for (i=0;i<8;i++) time_F[i]=time_arr[i+11];
        time_F[8]='\0';
        sprintf(temp,"Fastdiagn\n%s",time_F);
        TimeF->setText(temp);
        TimeFCol =  Green;
        TimeF->setPaletteForegroundColor( TimeFCol );
    }

//printf ("GetAll 1\n");
    OffloadSel=0;
    //char *diagnapp1 = (char *)"HOUSEKPR00";
    //char *diagnapp1 = (char *)"housekeeper.L";
    string diagnapp1 = std::string("housekeeper.") + Side();

    for  (ifam=0; ifam<NVARS2+2; ifam++) {
         if (ifam<NVARS2) family = FamilyNames2[ifam];
         if (ifam==NVARS2)   family = FrameRate;
         if (ifam==NVARS2+1) family = FrameCounter;
         try{
             DiagnWhich which(family, from, to);
             dict = GetDiagnValue( (char *)diagnapp1.c_str(),which, timeout1);
//printf ("GetAll 1a ifam=%d family=%s\n",ifam,family);
         } catch (AOException &e) {
                 cerr <<  e.what() <<" housekeeper\n";
                 if ( timer->isActive() ) timer->stop();
                 buttonStart->setText( tr( "&Start Refresh" ) );
                 buttonStart->setPaletteBackgroundColor( QColor( 200, 0, 0 ) );
                 QMessageBox::information(this,"AdSecMir_gui::GetAll()",
                  "Error in GetDiagnValue\nThe Housekeeper interrogation is INTERRUPTED");
           return;
         }
//printf ("GetAll 10a size=%d\n",dict.size());
        jj=-1;
        //Iterate over the dictionary of the retrieved DiagnVars.
        for (ValueDict::iterator it = dict.begin(); it != dict.end();  it++){
             jj++;
             DiagnWhich wh = (*it).first;
             DiagnValue va = (*it).second;

             //DiagnVar name & index
             Family = wh.Family();   // DiagnVar name.
             familyName = Family.c_str();

             //va.Time();     // Time (average of the time stamp of the values in the running mean)
             timec = va.Time();   //accumulate current value time  in sec?

             // Values
             mean = va.Mean();     // DiagnVar value (average of a running mean)
//printf ("GetAll 1b ifam=%d mean=%e\n",ifam,mean);
             if (ifam==NVARS2) {
                HframeRate = mean;
             }
             if (ifam==NVARS2+1) {
                if ((long)mean != HousekeeperN ) {
                   TimeHCol =  Green;
                   HousekeeperN=(long)mean;
                   HtimePassed = timec;
                } else {
                   if ( (timec-HtimePassed) > 1.2/HframeRate ) TimeHCol =  Red;
                }
                TimeH->setPaletteForegroundColor( TimeHCol );
             } else {
               if (ifam==0) {
                  if (mean==0.) OffloadSel=0; else OffloadSel=1;
               } else {
                 sprintf (temp,"%f",mean);
                 if (strcmp(temp,"nan")==0) mean = 0.;
                 //check the direction of the change for ifam=3,4
                 if (ifam>2 && ifam<5) {
                    if (Vswitch[ifam]<mean) Vcolor[ifam]=1; else Vcolor[ifam]=0;
                 }
                 Vswitch[ifam] = mean;
               }
             }
        }
        time(&time_d);
        time_str = ctime(&time_d);
        sprintf(time_arr,"%s",time_str);
        for (i=0;i<8;i++) time_H[i]=time_arr[i+11];
        time_H[8]='\0';
        sprintf(temp,"Housekeeper\n%s",time_H);
        TimeH->setText(temp);
    }

//printf ("GetAll 2\n");
    for  (ifam=0; ifam<NVARS1; ifam++) {
         family = FamilyNames1[ifam];
         strcpy(sfamily,family);
//printf ("GetAll 2a ifam=%d family=%s sel=%d\n",ifam,sfamily,OffloadSel);
         if (ifam==1 || ifam==3) strcat (sfamily,Variant[OffloadSel]);
         //first we get patameters  to check if some of actuators are disabled
         //we do this only ones at the begining
         if (Init[ifam+NVARS]==0) {
            try{      //to define bad actuators from  param file
                DiagnWhich which(sfamily, from, to);
                pdict = GetDiagnVarParam( (char *)diagnapp1.c_str(),which, timeout1);
            } catch (AOException &e) {
                    if ( timer->isActive() ) timer->stop();
                    buttonStart->setText( tr( "&Start Refresh" ) );
                    buttonStart->setPaletteBackgroundColor( QColor( 200, 0, 0 ) );
                    QMessageBox::information(this,"AdSecMir_gui::GetAll()",
                     "Unable to get the DiagnVarParam\nCheck whether the  housekeeper  is running.");
                 GetAllDummy();
             return;
            }
            jj=-1;
            for (ParamDict::iterator it = pdict.begin(); it != pdict.end();  it++){
                 jj++;
                 DiagnParam pa = (*it).second;
                 int enabled = pa.isEnabled();
                 if (enabled==1) ibuf[ifam+NVARS][convert_att[jj]] = 0; //enabled(=1) means the actuatuator is OK
                 else ibuf[ifam+NVARS][convert_att[jj]] = 1;
            }
            Init[ifam+NVARS]=1;
         }
         //now we get values
         try{
             DiagnWhich which(family, from, to);
             dict = GetDiagnValue( (char *)diagnapp1.c_str(),which, timeout);
         } catch (AOException &e) {
                 cerr <<  e.what() <<" housekeeper\n";
                 if ( timer->isActive() ) timer->stop();
                 buttonStart->setText( tr( "&Start Refresh" ) );
                 buttonStart->setPaletteBackgroundColor( QColor( 200, 0, 0 ) );
                 QMessageBox::information(this,"AdSecMir_gui::GetAll()",
                  "Error in GetDiagnValue\nThe Housekeeper interrogation is INTERRUPTED");
           return;
         }

        jj=-1;
        //Iterate over the dictionary of the retrieved DiagnVars.
        for (ValueDict::iterator it = dict.begin(); it != dict.end();  it++){
             jj++;
             DiagnWhich wh = (*it).first;
             DiagnValue va = (*it).second;

             //DiagnVar name & index
             Family = wh.Family();   // DiagnVar name.
             familyName = Family.c_str();

             //va.Time();     // Time (average of the time stamp of the values in the running mean)
             //timec = va.Time();   //accumulate current value time  in sec?

             // Values
             mean = va.Mean();     // DiagnVar value (average of a running mean)
             sprintf (temp,"%f",mean);
             if (strcmp(temp,"nan")==0) mean = 0.;
             if (ifam >2 )  mean = mean*1.e6;
             buf[ifam+NVARS][convert_att[jj]] = mean;
        }
    }
//printf ("GetAll 3\n");
}

void GUIadSecM::SetAll()
{
   int i;
   char tmp[100];
//printf ("SetAll\n");
   //info panel
   sprintf (tmp,"%s\n",Variant[OffloadSel]);
   infoText[0]->setText(tmp);
   for (i=1; i<NVARS2-1; i++) {
       sprintf(tmp,"%u\n",(int)Vswitch[i]);
       infoText[i]->setText(tmp);
       if (Vcolor[i]==0) infoText[i]->setPaletteForegroundColor(QColor(0,0,0));
       if (Vcolor[i]==1) infoText[i]->setPaletteForegroundColor(QColor(255,0,0));
   }
   i=NVARS2-1;
       sprintf(tmp,"%X\n",(int)Vswitch[i]);
       infoText[i]->setText(tmp);
       if (Vcolor[i]==0) infoText[i]->setPaletteForegroundColor(QColor(0,0,0));
       if (Vcolor[i]==1) infoText[i]->setPaletteForegroundColor(QColor(255,0,0));
   //mirror panels
   for (int iv=0; iv<NVARST; iv++) {
       if (iv<=1 )          mir[0]->fillM(&ibuf[iv][0],&buf[iv][0],Ival[iv]);
       if (iv<=5 && iv>1 )  mir[1]->fillM(&ibuf[iv][0],&buf[iv][0],Ival[iv]);
       if (iv<=7 && iv>5 )  mir[2]->fillM(&ibuf[iv][0],&buf[iv][0],Ival[iv]);
       //iv=8 -> modes; here skipped
       if (iv<=10 && iv>8 ) mir[1]->fillM(&ibuf[iv][0],&buf[iv][0],Ival1[iv-9]);
       if (iv>10 )          mir[2]->fillM(&ibuf[iv][0],&buf[iv][0],Ival1[iv-9]);
   }
   for (int i=0; i<NMirShown; i++)     mir[i]->updateM();
   RunningMean();
   refreshPlots();
}

void GUIadSecM::RunningMean()
{
    int irun;
    static float  fNint=0.;

//printf ("RunningMean\n");
    Nint++;
    fNint += 1.;
    if (Nint>100000) Nint=0;

    for  (int ifam=0; ifam<NVARST; ifam++) {
         for (int i=0; i<NMIR; i++) {

            //we save <=Nsamples data patterns for Running Mean
            if (Nr == Nsamples-1) {
              for (irun=1; irun<Nr; irun++) {
                  bufr[ifam][i][irun-1]= bufr[ifam][i][irun];
              }
            } 
            bufr[ifam][i][Nr]= buf[ifam][i];

            //make current Mean
            bufRmean[ifam][i] = 0.;
            for (irun=0; irun<Nr+1; irun++) {
               bufRmean[ifam][i]+= bufr[ifam][i][irun];
            }
            bufRmean[ifam][i]= bufRmean[ifam][i]/(Nr+1.);
         }

         //and show it
         int iv=ifam;
         if (iv<=1 )          mir[3]->fillM(&ibuf[iv][0],&buf[iv][0],Ival[iv]);
         if (iv<=5 && iv>1 )  mir[4]->fillM(&ibuf[iv][0],&buf[iv][0],Ival[iv]);
         if (iv<=7 && iv>5 )  mir[5]->fillM(&ibuf[iv][0],&buf[iv][0],Ival[iv]);
       //iv=8 -> modes; here skipped
         if (iv<=10 && iv>8 ) mir[4]->fillM(&ibuf[iv][0],&buf[iv][0],Ival1[iv-9]);
         if (iv>10 )          mir[5]->fillM(&ibuf[iv][0],&buf[iv][0],Ival1[iv-9]);
         //if (ifam<=1 )            mir[3]->fillM(&ibuf[ifam][0],&bufRmean[ifam][0],ifam);
         //if (ifam<=5 && ifam>1 )  mir[4]->fillM(&ibuf[ifam][0],&bufRmean[ifam][0],ifam);
         //if (ifam<=7 && ifam>5 )  mir[5]->fillM(&ibuf[ifam][0],&bufRmean[ifam][0],ifam);
    }
    Nr++;
    if (Nr>=Nsamples) Nr=Nsamples-1;
    for (int i=3; i<6; i++)     mir[i]->updateM();
}

void GUIadSecM::GetAllDummy()
{
    for  (int ifam=0; ifam<NVARST; ifam++) {
         for (int i=0; i<NMIR; i++) {
             ibuf[ifam][i]=0;
             buf[ifam][i]=1.*ifam + .0001*i*(rand()/32767.);
             if (i==17) ibuf[ifam][i]=1;
             if (i==76) ibuf[ifam][i]=1;
         }
    }
}

// take global offset in all current Mirror widgets
void GUIadSecM::TakeOffset()
{
   for (int i=0; i<6; i++)     mir[i]->takeOffset();
}


void GUIadSecM::newPlot( char* s, int i)
{
   PlotModes* plot;


    //plot = new PlotModes (0,"PLOT");  //if we like to have the possibility to hide from the taskbar
    plot = new PlotModes (this,"PLOT");
    plot->type = i;
    sprintf(plot->Title,"%s",s);
    plot->Refresh = TRUE;
    fillPlot (plot);

    pplot.push_back(plot);
    plot->inVector = pplot.size()-1;
    plot->show();
    connect (plot, SIGNAL (removeMyPlot (int) ), this, SLOT (remove_plot (int) ) );
}

void GUIadSecM::fillPlot(PlotModes* plot)
{
    double rms,m,t;

    int type=plot->type;

    for (int i=0; i<NMIR; i++) {    //number of points in modes =NMIR
        if (type==0) plot->y[i]=buf[iModes][i];
        if (type==1) plot->y[i]=bufRmean[iModes][i];
        if (type==2) {
            m=bufRmean[iModes][i];
            rms=0.;
            for (int irun=0; irun<Nr; irun++) {
               t= bufr[iModes][i][irun]-m;
               rms+= t*t;
            }
            if (Nr>0) rms = sqrt(rms/(double)Nr);
            plot->y[i]=rms;
        }
        plot->en[i]=ibuf[iModes][i];
    }
    strcpy(plot->TimeRefr,time_F);
    plot->timeCol = TimeFCol;
    plot->plotxy();
}

void GUIadSecM::remove_plot (int it)
{
   int i,n;
   PlotModes* plot;

   vector<PlotModes*>::iterator p = pplot.begin();
   p +=it;
   pplot.erase(p);

   n = pplot.size();
   if (n<=0) return;

   for (i=0; i<n; i++) {
       plot = pplot[i];
       plot->inVector = i;
   } 
}


void GUIadSecM::CreatePlotModes()
{
 newPlot((char *)"Modes",0);
}

void GUIadSecM::CreatePlotModesMean()
{
 newPlot((char *)"Modes Mean",1);
}

void GUIadSecM::CreatePlotModesRMS()
{
 newPlot((char *)"Modes RMS",2);
}
void GUIadSecM::refreshPlots()
{
   int i,n;
   PlotModes* plot;

   n = pplot.size();
   if (n<=0) return;

   for (i=0; i<n; i++) {
       plot = pplot[i];
       //plot->inVector = i;
       if (plot->Refresh) fillPlot (plot);
   } 
}
/*
void GUIadSecM::SetupVars() {
   try {
               RTDBvar fastdgnUnprotected = RTDBvar(Side(), "FASTDGN", 0,"UNPROTECTED", NO_DIR);
               Notify(fastdgnUnprotected, fastdgnUnprotectedHdlr);
               _logger->log(Logger::LOG_LEV_INFO, " -> SUCCESFULLY installed support to check if fastdgn isUnprotected!");
       }
       catch (AOVarException& e) {
               _logger->log(Logger::LOG_LEV_INFO, " -> CANNOT check if fastdgn isUnprotected !");
       }
}

int GUIadSecM::fastdgnUnprotectedHdlr(void *thisPtr, Variable *var) {
       GUIadSecM* mc = (GUIadSecM*)thisPtr;
       _fastdiagnIsUnprotected = var->Value.Lv[0];
       return NO_ERROR;
}
*/
