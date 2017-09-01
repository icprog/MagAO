/****************************************************************************
** Form interface generated from reading ui file '.ui/lbt672_housekeeping.ui'
**
** Created by User Interface Compiler
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GUILBT672_H
#define GUILBT672_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;


#include <qvariant.h>
#include <qdialog.h>
#include <qthread.h>
#include <qtimer.h>



#include "AOApp.h"

using namespace Arcetri;

#define NBUF 500
#define CRNUM 6
#define CR1NUM (CRNUM + 1)
#define BONUM 14
#define CHNUM 8

#define DUMMY TRUE

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QTabWidget;
class QWidget;
class QFrame;
class QLabel;
class KLed;
class QPushButton;
class QButtonGroup;
class QRadioButton;
class QListBox;
class QListBoxItem;
class QMessageBox;
class PlotForm;
class QwtPlot;
class QwtPlotZoomer;
class QwtText;
class QwtTextLabel;

class GUILBT672 : public QDialog, public AOApp
{
    Q_OBJECT

public:
    GUILBT672( string id, string configFile, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~GUILBT672();

    QTimer* timer;

//internal widgets
    QTabWidget* tabWidget;
    QWidget* tabSYS;
    QWidget* tabAdam;
    QWidget* aLeft;
    QWidget* wLeft;
    QWidget* wRight;
    QwtPlot* qwtPlotSysLeft;
    QwtPlotZoomer* qwtPlotSysLeftZoom;
    QwtPlot* qwtPlotSysRight;
    QwtPlotZoomer* qwtPlotSysRightZoom;
    QFrame* frameDiagn;
    QFrame* frameAdam;//mod RB
    QLabel* labelDiagn;
    QLabel* labelDiagnAdam;
    QLabel* textLabel[2];//7
    QLabel* DiagnAppDataLab[2];//7
    KLed* DiagnAppDataLed[2];//7
    QLabel* labelDiagnTemp;
    QLabel* textLabelTemp[5];
    QLabel* DiagnAppDataTemp[5];//7
    KLed* DiagnAppDataTempLed[5];//
    QLabel* GeneralValTemp[5];//7
    KLed*   GeneralValTempLed[5];//
    QLabel* textLabelAdam[14];//mod RB
    QLabel* DiagnAdam[14];//mod RB
    KLed* DiagnAdamLed[14];//mod RB
    QLabel* Clock; //mod RB
    QLabel* clockLabel; //mod RB
    QFrame* frameTemper;
    QLabel* labelTemper;
    QLabel* textLabelE[9];
    QLabel* ExternalLab[9];
    KLed* ExternalLed[9];
    QPushButton* ExternalBut[9];
    QWidget* unnamed;
    QWidget* wBCU1ld;
    QWidget* wBCU1mu;
    QWidget* wBCU1md;
    QWidget* wBCU1rd;
    QWidget* wBCU2ru;
    QWidget* wBCU2ld;
    QwtPlot* qwtBCU1ld;
    QwtPlotZoomer* qwtBCU1ldZoom;
    QwtPlot* qwtBCU1mu;
    QwtPlotZoomer* qwtBCU1muZoom;
    QwtPlot* qwtBCU1md;
    QwtPlotZoomer* qwtBCU1mdZoom;
    QwtPlot* qwtBCU1rd;
    QwtPlotZoomer* qwtBCU1rdZoom;
    QwtPlot* qwtBCU2ld;
    QwtPlotZoomer* qwtBCU2ldZoom;
    QwtPlot* qwtBCU2ru;
    QwtPlotZoomer* qwtBCU2ruZoom;
    QFrame* frameNIOS;
    QLabel* labelNIOS;
    QLabel* textLabelBCU1[11];
    QLabel* BCU1NiosLab[11];
    KLed* BCU1NiosLed[11];
    QFrame* crateNumber;
    QButtonGroup* crateBCU1_gr;
    QRadioButton* BCU1RBut[CR1NUM];
    QFrame* frameEnvirBCU1;
    QLabel* labelEnvBCU1;
    QLabel* BCU1EnvText[6];
    QLabel* BCU1EnvLab[6];
    KLed* BCU1EnvLed[6];
    QPushButton* BCU1EnvBut[6];
    QFrame* frameTempBCU1;
    QLabel* labelTempBCU1;
    QLabel* BCU1TempText[2];
    QLabel* BCU1TempLab[2];
    KLed* BCU1TempLed[2];
    QPushButton* BCU1TempBut[2];
    QFrame* frameVoltBCU1;
    QLabel* labelVoltBCU1;
    QLabel* BCU1VolText[10];
    QLabel* BCU1VolLab[10];
    KLed* BCU1VolLed[10];
    QPushButton* BCU1VolBut[10];
    QFrame* frameTempSIGGEN;
    QLabel* labelTempSIGGENd;
    QLabel* labelTempSIGGENd_2;
    QLabel* BCU1SGNTempText[3];
    QLabel* BCU1SGNTempLab[3];
    KLed* BCU1SGNTempLed[3];
    QPushButton* BCU1SGNTempBut[3];
    QWidget* TabPage;
    QFrame* frameResets;
    QLabel* labelResets;
    QLabel* BCU2ResetText[10];
    KLed* BCU2ResetLed[10];
    QFrame* crateNumber2;
    QButtonGroup* crateBCU2_gr;
    QRadioButton* BCU2RBut[CRNUM];
    QFrame* frameDigitalIO;
    QLabel* labelDigital;
    QLabel* BCU2DigText[10];
    KLed* BCU2DigLed[10];
    QFrame* frameSIGGENa;
    QLabel* labelDiagn_2;
    QLabel* labelDiagn_3;
    QLabel* BCU2SGNText[4];
    QLabel* BCU2SGNLab[4];
    KLed* BCU2SGNLed[4];
    QFrame* frameMasterBCU2;
    QLabel* labelEnvBCU2;
    QLabel* BCU2MastText[3];
    QLabel* BCU2MastLab[3];
    KLed* BCU2MastLed[3];
    QPushButton* BCU2MastBut[3];
    QFrame* frameBCU2c;
    QLabel* BCU2TrText[4];
    QLabel* BCU2TrLab[4];
    KLed* BCU2TrLed[4];
    QPushButton* BCU2TrBut[4];
    QWidget* TabPage_2;
    QFrame* frameDSPa;
    QLabel* labelDiagn_4;
    QLabel* DSPNiosText[8];
    QLabel* DSPNiosLab[8];
    KLed* DSPNiosLed[8];
    QWidget* QWidget3;
    QFrame* frameTempDSP;
    QLabel* labelTempDSPd;
    QLabel* DSPTempText[4];
    QLabel* DSPTempLab[4];
    KLed* DSPTempLed[4];
    QPushButton* DSPTempBut[4];
    QFrame* frameSIGGENb;
    QButtonGroup* crateSIGGEN_gr;
    QRadioButton* DSP1RBut[CRNUM];
    QFrame* frameCoilCurrDSP;
    QLabel* labelTempDSPc_0;
    QLabel* DSPCurText[8];
    QLabel* DSPCurLab[8];
    KLed* DSPCurLed[8];
    QPushButton* DSPCurBut[8];
    QWidget* QWidget4;
    QFrame* frame1;
    QButtonGroup* crateDSPb_6;
    QRadioButton* DSP2RBut[BONUM];
    QFrame* frameDriverEnabledDSP;
    QLabel* labelDriverEnabled;
    QWidget* unnamed_2;
    QWidget* unnamed_3;
    QLabel* DSPDrText[8];
    KLed* DSPDrLed[8];
    QFrame* frameDriverStatusDSP;
    QLabel* labelDriverStatus;
    QLabel* DSPDrSText[10];
    KLed* DSPDrSLed[10];
    QFrame* frameDSPb;
    QLabel* DSPadText[6];
    QLabel* DSPadLab[6];
    KLed* DSPadLed[6];
    QButtonGroup* crateDSPb_6_2;
    QRadioButton* DSP3RBut[CHNUM];
    QWidget* Widget18;
    QListBox* listBox1;
    QWidget* Widget19;
    QListBox* listBox1_2;
    QPushButton* buttonStart;
    QLabel* warnLabel;
    QLabel* alarmLabel;
    QLabel* wrangeLabel;
    QLabel* arangeLabel;
    QPushButton* buttonStart_2;
    QPushButton* buttonExit;


//variables
    /****  Housekeeper Family Names,  from housekeeper.param ***********************/
    char *FamilyNames[200];
    char *AdamFamilyNames[14];
    int nfamily;
    int nadamfamily;
    int timeoutR;  //refresh rate

    int Init;     //=0 to init,  =1 is done already
    bool Dummy;   //whether we are in dummy mode

    double  timeB[NBUF];     // times for accumulated value buffers
    double  timeL[NBUF];     // times for LONG accumulated value buffers
    int NBuf;               //max size of arrays with temporal buffers to accumulate  <=NBUF
    int Nb;                 // current size of the accumulated buffers
    int NBufL;              //max size of arrays with LONG buffers  <=NBUF
    int Nbl;                // current size of the LONG accumulated buffers
    int NLong;              // numder of basic points to evarage for one point of LONG  buffers
    int iLong;              // current size of not yet evaraged basic points

    //internal indexes
    int ICr1, ICr2, ICr3; //chosen crate number for "Crate BCU1", "Crate BCU2", "Crate DSP" panels
    int IBo, ICh;  //chosen board and channel numbers, for "Crate DSP" panel


    /****  arrays for all panels *************************************************************************/

    //****  System Summary tab   -----------------------------------------------------------------------
    // to show in "DiagApp Data" panel (left)
    int    DiagAppDataInd[7];  //indexes of corresponding FamilyNames in array-> {first, number}
    double  DiagAppDataVal[7];  // values  (running mean)//was 7
    double  DiagAppDataStd[7];  // standard deviations//was 7
    double  DiagAppDataTempVal[7];  // values  (running mean)//was 7
    double  DiagAppDataTempStd[7];  // standard deviations//was 7
    double  DiagAppDataWMin[7];  // min limit//was 7
    double  DiagAppDataWMax[7];  // max limit//was 7
    double  DiagAppDataAMin[7];  // min limit//was 7
    double  DiagAppDataAMax[7];  // max limit//was 7
    int    DiagAppDataStat[7]; // status :  0=normal, 1=warning, 2=alarm//was 7
    int    DiagAppDataEn[7];   // enabled :  1= is anabled
    int    DiagAppDataTempStat[7];
    double  DiagAppDataTemp[7];

   
    double GeneralVal[5];   //general temp values(hub, cp, rb...)
    double GeneralStd[5];
    int   GeneralStat[5];
    int   GeneralEn[5];
    double  GeneralWMin[5];  // limits
    double  GeneralWMax[5];  //
    double  GeneralAMin[5];  // limits
    double  GeneralAMax[5];  //


    
    double AdamVal[14];
    double AdamStd[14];
    int   AdamStat[14];
    int   AdamEn[14];
    //char  AdamExpl[3];
    char  fam;

    // to show in "External..." panel (right)
    int    ExternalInd[2];  //indexes of corresponding FamilyNames in array-> {first, number}
    double  ExternalVal[9];  // values
    double  ExternalStd[9];  // standard deviations
    double  ExternalWMin[9];  // limits
    double  ExternalWMax[9];  //
    double  ExternalAMin[9];  // limits
    double  ExternalAMax[9];  //
    int    ExternalStat[9]; // status :  0=normal, 1=warning, 2=alarm
    int    ExternalEn[9];
    double  ExternalBufV[9][NBUF]; //buffer for mean values
    double  ExternalBufVL[9][NBUF]; //buffer for mean values


    //****  Crate BCU1 tab      -----------------------------------------------------------------------
    // first index is a chosen Crate (0 to 5) or Switch BCU(=6), if applied
    // to show in "NIOS Fixed Area" panel (left/upper)
    int    BCU1NiosInd[2][2];  //indexes of corresponding FamilyNames in array-> {first, number}
                               //Crates   Switch <- ...Ind[1][...]
    double  BCU1NiosVal[CR1NUM][11];  // values  (running mean)
    double  BCU1NiosStd[CR1NUM][11];  // standard deviations
    double  BCU1NiosWMin[CR1NUM][11];  // limits
    double  BCU1NiosWMax[CR1NUM][11];  // 
    double  BCU1NiosAMin[CR1NUM][11];  // limits
    double  BCU1NiosAMax[CR1NUM][11];  //
    int    BCU1NiosStat[CR1NUM][11]; // status :  0=normal, 1=warning, 2=alarm
    int    BCU1NiosEn[CR1NUM][11]; 

    // to show in "Environment" panel (middle/upper)
    int    BCU1EnvInd[2];     //indexes of corresponding FamilyNames in array-> {first, number}
    double  BCU1EnvVal[CRNUM][6];  // values  (running mean)
    double  BCU1EnvStd[CRNUM][6];  // standard deviations
    double  BCU1EnvWMin[CRNUM][6];  // limits
    double  BCU1EnvWMax[CRNUM][6];  // 
    double  BCU1EnvAMin[CRNUM][6];  // limits
    double  BCU1EnvAMax[CRNUM][6];  //
    int    BCU1EnvStat[CRNUM][6]; // status :  0=normal, 1=warning, 2=alarm
    int    BCU1EnvEn[CRNUM][6];
    double  BCU1EnvBufV[CRNUM][6][NBUF]; //buffer for mean values
    double  BCU1EnvBufVL[CRNUM][6][NBUF]; //buffer for mean values

    // to show in "Voltage and Currents" panel (right/upper)
    int    BCU1VolInd[2];      //indexes of corresponding FamilyNames in array-> {first, number}
    double  BCU1VolVal[CRNUM][10];  // values  (running mean)
    double  BCU1VolStd[CRNUM][10];  // standard deviations
    double  BCU1VolWMin[CRNUM][10];  // limits
    double  BCU1VolWMax[CRNUM][10];  // 
    double  BCU1VolAMin[CRNUM][10];  // limits
    double  BCU1VolAMax[CRNUM][10];  //
    int    BCU1VolStat[CRNUM][10]; // status :  0=normal, 1=warning, 2=alarm
    int    BCU1VolEn[CRNUM][10];
    double  BCU1VolBufV[CRNUM][10][NBUF]; //buffer for mean values
    double  BCU1VolBufVL[CRNUM][10][NBUF]; //buffer for mean values

    // to show in "Temperatures" panel (middle/down)
    int    BCU1TempInd[2][2];  //indexes of corresponding FamilyNames in array-> {first, number}
                               //Crates   Switch <- ...Ind[1][...]
    double  BCU1TempVal[CR1NUM][2];  // values  (running mean)
    double  BCU1TempStd[CR1NUM][2];  // standard deviations
    double  BCU1TempWMin[CR1NUM][2];  // limits
    double  BCU1TempWMax[CR1NUM][2];  // 
    double  BCU1TempAMin[CR1NUM][2];  // limits
    double  BCU1TempAMax[CR1NUM][2];  //
    int    BCU1TempStat[CR1NUM][2]; // status :  0=normal, 1=warning, 2=alarm
    int    BCU1TempEn[CR1NUM][2];
    double  BCU1TempBufV[CR1NUM][2][NBUF]; //buffer for mean values
    double  BCU1TempBufVL[CR1NUM][2][NBUF]; //buffer for mean values

    // to show in "SIGGEN Temperatures" panel (right/down)
    int    BCU1SGNTempInd[2];     //indexes of corresponding FamilyNames in array-> {first, number}
    double  BCU1SGNTempVal[CRNUM][3];  // values  (running mean)
    double  BCU1SGNTempStd[CRNUM][3];  // standard deviations
    double  BCU1SGNTempWMin[CRNUM][3];  // limits
    double  BCU1SGNTempWMax[CRNUM][3];  // 
    double  BCU1SGNTempAMin[CRNUM][3];  // limits
    double  BCU1SGNTempAMax[CRNUM][3];  //
    int    BCU1SGNTempStat[CRNUM][3]; // status :  0=normal, 1=warning, 2=alarm
    int    BCU1SGNTempEn[CRNUM][3];
    double  BCU1SGNTempBufV[CRNUM][3][NBUF]; //buffer for mean values
    double  BCU1SGNTempBufVL[CRNUM][3][NBUF]; //buffer for mean values



    //****  Crate BCU2 tab      -----------------------------------------------------------------------
    // first index is a chosen Crate (0 to 5) , if applied
    // to show in "Reset Status" panel (left/upper)
    int    BCU2ResetInd[2];      //indexes of corresponding FamilyNames in array-> {first, number}
    int    BCU2ResetStat[CRNUM][10]; // status :  0=notset, 1=set
    int    BCU2ResetEn[CRNUM][10];

    // to show in "Digital I/O" panel (middle/upper)
    int    BCU2DigInd[2];      //indexes of corresponding FamilyNames in array-> {first, number}
    int    BCU2DigStat[CRNUM][10]; // status :  0=notset, 1=set
    int    BCU2DigEn[CRNUM][10]; 

    // to show in "Master crate global..." panel (right/upper)
    int    BCU2MastInd[2];  //indexes of corresponding FamilyNames in array-> {first, number}
    double  BCU2MastVal[3];  // values  (running mean)
    double  BCU2MastStd[3];  // standard deviations
    double  BCU2MastWMin[3];  // limits
    double  BCU2MastWMax[3];  //
    double  BCU2MastAMin[3];  // limits
    double  BCU2MastAMax[3];  //
    int    BCU2MastStat[3]; // status :  0=normal, 1=warning, 2=alarm
    int    BCU2MastEn[3];
    double  BCU2MastBufV[3][NBUF]; //buffer for mean values
    double  BCU2MastBufVL[3][NBUF]; //buffer for mean values

    // to show in "SIGGEN NIOS Fixed Area" panel (middle/down)
    int    BCU2SGNInd[2];  //indexes of corresponding FamilyNames in array-> {first, number}
    double  BCU2SGNVal[4];  // values  (running mean)
    double  BCU2SGNStd[4];  // standard deviations
    int    BCU2SGNStat[4]; // status :  0=normal, 1=warning, 2=alarm
    int    BCU2SGNEn[4]; 

    // to show in  right/down  panel
    int    BCU2TrInd[2];  //indexes of corresponding FamilyNames in array-> {first, number}
    double  BCU2TrVal[4];  // values  (running mean)
    double  BCU2TrStd[4];  // standard deviations
    double  BCU2TrWMin[4];  // limits
    double  BCU2TrWMax[4];  //
    double  BCU2TrAMin[4];  // limits
    double  BCU2TrAMax[4];  //
    int    BCU2TrStat[4]; // status :  0=normal, 1=warning, 2=alarm
    int    BCU2TrEn[4];
    double  BCU2TrBufV[4][NBUF]; //buffer for mean values
    double  BCU2TrBufVL[4][NBUF]; //buffer for mean values



    //****  Crate DSP tab      -----------------------------------------------------------------------
    // first  index is a chosen Crate (0 to 5) , if applied
    // second index is a chosen Board (0 to 13) , if applied
    // third  index is a chosen Channel (0 to 7) , if applied
    // to show in "NIOS Fixed Area" panel (left/upper)
    int    DSPNiosInd[2];  //indexes of corresponding FamilyNames in array-> {first, number}
    double  DSPNiosVal[CRNUM][BONUM][8];  // values  (running mean)
    double  DSPNiosStd[CRNUM][BONUM][8];  // standard deviations
    double  DSPNiosWMin[CRNUM][BONUM][8];  // limits
    double  DSPNiosWMax[CRNUM][BONUM][8];  // 
    double  DSPNiosAMin[CRNUM][BONUM][8];  // limits
    double  DSPNiosAMax[CRNUM][BONUM][8];  //
    int    DSPNiosStat[CRNUM][BONUM][8]; // status :  0=normal, 1=warning, 2=alarm
    int    DSPNiosEn[CRNUM][BONUM][8];

    // to show in "Coil currents (SPI)" panel (middle/upper)
    int    DSPCurInd[2];         //indexes of corresponding FamilyNames in array-> {first, number}
    double  DSPCurVal[CRNUM][BONUM][8];  // values  (running mean)
    double  DSPCurStd[CRNUM][BONUM][8];  // standard deviations
    double  DSPCurWMin[CRNUM][BONUM][8];  // limits
    double  DSPCurWMax[CRNUM][BONUM][8];  // 
    double  DSPCurAMin[CRNUM][BONUM][8];  // limits
    double  DSPCurAMax[CRNUM][BONUM][8];  //
    int    DSPCurStat[CRNUM][BONUM][8]; // status :  0=normal, 1=warning, 2=alarm
    int    DSPCurEn[CRNUM][BONUM][8];
    double  DSPCurBufV[CRNUM][BONUM][8][NBUF]; //buffer for mean values
    double  DSPCurBufVL[CRNUM][BONUM][8][NBUF]; //buffer for mean values

    // to show in "Driver Status" panel (right/upper)
    int    DSPDrSInd[2];          //indexes of corresponding FamilyNames in array-> {first, number}
    int    DSPDrSStat[CRNUM][BONUM][10]; // status :  0=notset, 1=set
    int    DSPDrSEn[CRNUM][BONUM][10];

    // to show in "Temperatures" panel (left/middle)
    int    DSPTempInd[2];         //indexes of corresponding FamilyNames in array-> {first, number}
    double  DSPTempVal[CRNUM][BONUM][4];  // values  (running mean)
    double  DSPTempStd[CRNUM][BONUM][4];  // standard deviations
    double  DSPTempWMin[CRNUM][BONUM][4];  // limits
    double  DSPTempWMax[CRNUM][BONUM][4];  //
    double  DSPTempAMin[CRNUM][BONUM][4];  // limits
    double  DSPTempAMax[CRNUM][BONUM][4];  //
    int    DSPTempStat[CRNUM][BONUM][4]; // status :  0=normal, 1=warning, 2=alarm
    int    DSPTempEn[CRNUM][BONUM][4];
    double  DSPTempBufV[CRNUM][BONUM][4][NBUF]; //buffer for mean values
    double  DSPTempBufVL[CRNUM][BONUM][4][NBUF]; //buffer for mean values

    // to show in "Driver Enabled" panel (middle/down)
    int    DSPDrInd[2];         //indexes of corresponding FamilyNames in array-> {first, number}
    int    DSPDrStat[CRNUM][BONUM][8]; // status :  0=notset, 1=set
    int    DSPDrEn[CRNUM][BONUM][8];

    // to show in right/down panel
    int    DSPadInd[2];            //indexes of corresponding FamilyNames in array-> {first, number}
    double  DSPadVal[CRNUM][BONUM][CHNUM][6];  // values  (running mean)
    double  DSPadStd[CRNUM][BONUM][CHNUM][6];  // standard deviations
    double  DSPadWMin[CRNUM][BONUM][CHNUM][6];  // limits
    double  DSPadWMax[CRNUM][BONUM][CHNUM][6];  // 
    double  DSPadAMin[CRNUM][BONUM][CHNUM][6];  // limits
    double  DSPadAMax[CRNUM][BONUM][CHNUM][6];  //
    int    DSPadStat[CRNUM][BONUM][CHNUM][6]; // status :  0=normal, 1=warning, 2=alarm
    int    DSPadEn[CRNUM][BONUM][CHNUM][6]; 

    // to show in Warnings & Alarms
    vector<string> Warnings;
    vector<string> Alarms;
    vector<PlotForm*> pplot;


public slots:

    void Run();  //to overload the Run method of AOApp

    virtual void quit();  //quit this and related  widgets

    virtual void refreshPlots();
    virtual void newPlot( string, int );
    virtual void fillPlot( PlotForm* );
    virtual void remove_plot( int );
    virtual void PlotSysLeft();
    virtual void PlotSysRight();
    virtual void PlotBCU1ld();
    virtual void PlotBCU1mu();
    virtual void PlotBCU1md();
    virtual void PlotBCU1rd();
    virtual void PlotBCU2ld();
    virtual void PlotBCU2ru();

//  show range support
    virtual void SetRangeLabel( string, int );

    virtual void StopStartTimer();

//  Get&Set All Housekeeper variables
    virtual void Refresh ();
    virtual void GetAll ();
    virtual void GetAllAdam();
    virtual void GetAllDummy ();
    virtual void SetAll ();
    virtual void GetLimits ();

//  System Summary setting group
    virtual void SetSystemSummary ();

//  Crate BCU1 setting group
    virtual void SetCrateBCU1 ();

//  Crate BCU2 setting group
    virtual void SetCrateBCU2 ();

//  Crate DSP setting group
    virtual void SetCrateDSP ();

//  Warnings &  Alarms setting group
    virtual void CheckWarnAlarms ();
    virtual void SetWarnings ();
    virtual void SetAlarms ();

//  toggled Crate in BUC1
    virtual void SetICr1 ();

//  toggled Crate in BUC2
    virtual void SetICr2 ();

//  toggled Crate in DSP
    virtual void SetICr3 ();

//  toggled Board in DSP
    virtual void SetIBo ();

//  toggled Channel in DSP
    virtual void SetICh ();


protected:
    QVBoxLayout* GUILBT672Layout;
    QVBoxLayout* vBLeftLayout;
    QVBoxLayout* vBRightLayout;
    QVBoxLayout* vBCU1ldLayout;
    QVBoxLayout* vBCU1muLayout;
    QVBoxLayout* vBCU1mdLayout;
    QVBoxLayout* vBCU1rdLayout;
    QVBoxLayout* vBCU2ldLayout;
    QVBoxLayout* vBCU2ruLayout;
    QHBoxLayout* tabSYSLayout;
    QGridLayout* frameDiagnLayout;
    QHBoxLayout* AdamTabLayout;//mod RB
    QVBoxLayout* aBLeftLayout;
    QGridLayout* frameAdamLayout;//mod RB
    QGridLayout* frameTemperLayout;
    QGridLayout* unnamedLayout;
    QGridLayout* frameNIOSLayout;
    QGridLayout* crateNumberLayout;
    QHBoxLayout* crateBCU1_grLayout;
    QGridLayout* frameEnvirBCU1Layout;
    QGridLayout* frameTempBCU1Layout;
    QGridLayout* frameVoltBCU1Layout;
    QGridLayout* frameTempSIGGENLayout;
    QGridLayout* TabPageLayout;
    QGridLayout* frameResetsLayout;
    QGridLayout* crateNumber2Layout;
    QHBoxLayout* crateBCU2_grLayout;
    QGridLayout* frameDigitalIOLayout;
    QGridLayout* frameSIGGENaLayout;
    QGridLayout* frameMasterBCU2Layout;
    QGridLayout* frameBCU2cLayout;
    QGridLayout* TabPageLayout_2;
    QGridLayout* frameDSPaLayout;
    QVBoxLayout* QWidget3Layout;
    QGridLayout* frameTempDSPLayout;
    QGridLayout* frameSIGGENbLayout;
    QHBoxLayout* crateSIGGEN_grLayout;
    QGridLayout* frameCoilCurrDSPLayout;
    QHBoxLayout* QWidget4Layout;
    QGridLayout* frame1Layout;
    QGridLayout* crateDSPb_6Layout;
    QGridLayout* frameDriverEnabledDSPLayout;
    QGridLayout* unnamedLayout_2;
    QGridLayout* unnamedLayout_3;
    QGridLayout* frameDriverStatusDSPLayout;
    QGridLayout* frameDSPbLayout;
    QGridLayout* crateDSPb_6Layout_2;
    QHBoxLayout* Layout1;
    QSpacerItem* Horizontal_Spacing2;
    QSpacerItem* Horizontal_Spacing3;

    void shiftBuf (double *, int);
    double meanBuf (double *, int);

    bool eventFilter ( QObject *obj, QEvent *ev );

protected slots:
    virtual void languageChange();

};

#endif // GUILBT672_H
