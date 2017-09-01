/****************************************************************************
** Form interface generated from reading ui file '.ui/lbt672_housekeeping.ui'
**
** Created by User Interface Compiler
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef TOPLBT672_H
#define TOPLBT672_H

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

class topLBT672 : public QDialog, public AOApp
{
    Q_OBJECT

public:
    topLBT672( string configFile, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~topLBT672();

    // ---------------------------------------- //

    void Run() { }

    // ---------------------------------------- //

    QTimer* timer;

//internal widgets
    QTabWidget* tabWidget;
    QWidget* tabSYS;
    QFrame* frameDiagn;
    QLabel* labelDiagn;
    QLabel* textLabel1;
    QLabel* textLabel2;
    QLabel* textLabel3;
    QLabel* textLabel4;
    QLabel* textLabel5;
    QLabel* textLabel6;
    QLabel* textLabel7;
    QLabel* DiagnAppDataLab[7];
    KLed* DiagnAppDataLed[7];
    QFrame* frameTemper;
    QLabel* labelTemper;
    QLabel* textLabel11;
    QLabel* textLabel12;
    QLabel* textLabel13;
    QLabel* textLabel14;
    QLabel* textLabel15;
    QLabel* textLabel16;
    QLabel* textLabel17;
    QLabel* textLabel18;
    QLabel* textLabel19;
    QLabel* ExternalLab[9];
    KLed* ExternalLed[9];
    QPushButton* ExternalBut[9];
    QWidget* unnamed;
    QFrame* frameNIOS;
    QLabel* labelNIOS;
    QLabel* textLabelBCU1_1;
    QLabel* textLabelBCU1_2;
    QLabel* textLabelBCU1_3;
    QLabel* textLabelBCU1_4;
    QLabel* textLabelBCU1_5;
    QLabel* textLabelBCU1_6;
    QLabel* textLabelBCU1_7;
    QLabel* textLabelBCU1_8;
    QLabel* textLabelBCU1_9;
    QLabel* textLabelBCU1_10;
    QLabel* textLabelBCU1_11;
    QLabel* BCU1NiosLab[11];
    KLed* BCU1NiosLed[11];
    QFrame* crateNumber;
    QButtonGroup* crateBCU1_gr;
    QRadioButton* BCU1RBut[7];
    QFrame* frameEnvirBCU1;
    QLabel* labelEnvBCU1;
    QLabel* textLabelBCU1b_1;
    QLabel* textLabelBCU1b_2;
    QLabel* textLabelBCU1b_3;
    QLabel* textLabelBCU1b_4;
    QLabel* textLabelBCU1b_5;
    QLabel* textLabelBCU1b_6;
    QLabel* BCU1EnvLab[6];
    KLed* BCU1EnvLed[6];
    QPushButton* BCU1EnvBut[6];
    QFrame* frameTempBCU1;
    QLabel* labelTempBCU1;
    QLabel* textLabelBCU1a_1;
    QLabel* textLabelBCU1a_2;
    QLabel* BCU1TempLab[2];
    KLed* BCU1TempLed[2];
    QPushButton* BCU1TempBut[2];
    QFrame* frameVoltBCU1;
    QLabel* labelVoltBCU1;
    QLabel* textLabelBCU1c_1;
    QLabel* textLabelBCU1c_2;
    QLabel* textLabelBCU1c_3;
    QLabel* textLabelBCU1c_4;
    QLabel* textLabelBCU1c_5;
    QLabel* textLabelBCU1c_6;
    QLabel* textLabelBCU1c_7;
    QLabel* textLabelBCU1c_8;
    QLabel* textLabelBCU1c_9;
    QLabel* textLabelBCU1c_10;
    QLabel* BCU1VolLab[10];
    KLed* BCU1VolLed[10];
    QPushButton* BCU1VolBut[10];
    QFrame* frameTempSIGGEN;
    QLabel* labelTempSIGGENd;
    QLabel* labelTempSIGGENd_2;
    QLabel* textLabelSIGGENd_1;
    QLabel* textLabelSIGGENd_2;
    QLabel* textLabelSIGGENd_3;
    QLabel* BCU1SGNTempLab[3];
    KLed* BCU1SGNTempLed[3];
    QPushButton* BCU1SGNTempBut[3];
    QWidget* TabPage;
    QFrame* frameResets;
    QLabel* labelResets;
    QLabel* textLabelBCU2_1;
    QLabel* textLabelBCU2_2;
    QLabel* textLabelBCU2_3;
    QLabel* textLabelBCU2_4;
    QLabel* textLabelBCU2_5;
    QLabel* textLabelBCU2_6;
    QLabel* textLabelBCU2_7;
    QLabel* textLabelBCU2_8;
    QLabel* textLabelBCU2_9;
    QLabel* textLabelBCU2_10;
    KLed* BCU2ResetLed[10];
    QFrame* crateNumber2;
    QButtonGroup* crateBCU2_gr;
    QRadioButton* BCU2RBut[6];
    QFrame* frameDigitalIO;
    QLabel* labelDigital;
    QLabel* textLabelBCU2a_1;
    QLabel* textLabelBCU2a_2;
    QLabel* textLabelBCU2a_3;
    QLabel* textLabelBCU2a_4;
    QLabel* textLabelBCU2a_5;
    QLabel* textLabelBCU2a_6;
    QLabel* textLabelBCU2a_7;
    QLabel* textLabelBCU2a_8;
    QLabel* textLabelBCU2a_9;
    QLabel* textLabelBCU2a_10;
    KLed* BCU2DigLed[10];
    QFrame* frameSIGGENa;
    QLabel* labelDiagn_2;
    QLabel* labelDiagn_3;
    QLabel* textLabelSIGGENa_1;
    QLabel* textLabelSIGGENa_2;
    QLabel* textLabelSIGGENa_3;
    QLabel* textLabelSIGGENa_4;
    QLabel* BCU2SGNLab[4];
    KLed* BCU2SGNLed[4];
    QFrame* frameMasterBCU2;
    QLabel* labelEnvBCU2;
    QLabel* textLabelBCU2b_1;
    QLabel* textLabelBCU2b_2;
    QLabel* textLabelBCU2b_3;
    QLabel* BCU2MastLab[3];
    KLed* BCU2MastLed[3];
    QPushButton* BCU2MastBut[3];
    QFrame* frameBCU2c;
    QLabel* textLabelBCU2c_0;
    QLabel* textLabelBCU2c_1;
    QLabel* textLabelBCU2c_2;
    QLabel* textLabelBCU2c_3;
    QLabel* BCU2TrLab[4];
    KLed* BCU2TrLed[4];
    QPushButton* BCU2TrBut[4];
    QWidget* TabPage_2;
    QFrame* frameDSPa;
    QLabel* labelDiagn_4;
    QLabel* textLabelDSPa_1;
    QLabel* textLabelDSPa_2;
    QLabel* textLabelDSPa_3;
    QLabel* textLabelDSPa_4;
    QLabel* textLabelDSPa_5;
    QLabel* textLabelDSPa_6;
    QLabel* textLabelDSPa_7;
    QLabel* textLabelDSPa_8;
    QLabel* DSPNiosLab[8];
    KLed* DSPNiosLed[8];
    QWidget* QWidget3;
    QFrame* frameTempDSP;
    QLabel* labelTempDSPd;
    QLabel* textLabelDSPd_1;
    QLabel* textLabelDSPd_2;
    QLabel* textLabelDSPd_3;
    QLabel* textLabelDSPd_4;
    QLabel* DSPTempLab[4];
    KLed* DSPTempLed[4];
    QPushButton* DSPTempBut[4];
    QFrame* frameSIGGENb;
    QButtonGroup* crateSIGGEN_gr;
    QRadioButton* DSP1RBut[6];
    QFrame* frameCoilCurrDSP;
    QLabel* labelTempDSPc_0;
    QLabel* textLabelDSPc_1;
    QLabel* textLabelDSPc_2;
    QLabel* textLabelDSPc_3;
    QLabel* textLabelDSPc_4;
    QLabel* textLabelDSPc_5;
    QLabel* textLabelDSPc_6;
    QLabel* textLabelDSPc_7;
    QLabel* textLabelDSPc_8;
    QLabel* DSPCurLab[8];
    KLed* DSPCurLed[8];
    QPushButton* DSPCurBut[8];
    QWidget* QWidget4;
    QFrame* frame1;
    QButtonGroup* crateDSPb_6;
    QRadioButton* DSP2RBut[14];
    QFrame* frameDriverEnabledDSP;
    QLabel* labelDriverEnabled;
    QWidget* unnamed_2;
    QLabel* textLabelDSPf_1;
    QLabel* textLabelDSPf_2;
    QLabel* textLabelDSPf_3;
    QLabel* textLabelDSPf_4;
    QWidget* unnamed_3;
    QLabel* textLabelDSPf_5;
    QLabel* textLabelDSPf_6;
    QLabel* textLabelDSPf_7;
    QLabel* textLabelDSPf_8;
    KLed* DSPDrLed[8];
    QFrame* frameDriverStatusDSP;
    QLabel* labelDriverStatus;
    QLabel* textLabelDSPe_1;
    QLabel* textLabelDSPe_2;
    QLabel* textLabelDSPe_3;
    QLabel* textLabelDSPe_4;
    QLabel* textLabelDSPe_5;
    QLabel* textLabelDSPe_6;
    QLabel* textLabelDSPe_7;
    QLabel* textLabelDSPe_8;
    QLabel* textLabelDSPe_9;
    QLabel* textLabelDSPe_10;
    KLed* DSPDrSLed[10];
    QFrame* frameDSPb;
    QLabel* textLabelDSPb_0;
    QLabel* textLabelDSPb_1;
    QLabel* textLabelDSPb_2;
    QLabel* textLabelDSPb_3;
    QLabel* textLabelDSPb_4;
    QLabel* textLabelDSPb_5;
    QLabel* DSPadLab[6];
    KLed* DSPadLed[6];
    QButtonGroup* crateDSPb_6_2;
    QRadioButton* DSP3RBut[8];
    QWidget* Widget18;
    QListBox* listBox1;
    QWidget* Widget19;
    QListBox* listBox1_2;
    QPushButton* buttonStart;
    QLabel* warnLabel;
    QLabel* alarmLabel;
    QPushButton* buttonStart_2;
    QPushButton* buttonExit;

//variables
    /****  Housekeeper Family Names,  from housekeeper.param ***********************/
    char *FamilyNames[200];
    int nfamily;
    int timeout;  //refresh rate

    //internal indexes
    int ICr1, ICr2, ICr3; //chosen crate number for "Crate BCU1", "Crate BCU2", "Crate DSP" panels
    int IBo, ICh;  //chosen board and channel numbers, for "Crate DSP" panel


    /****  arrays for all panels *************************************************************************/

    //****  System Summary tab   -----------------------------------------------------------------------
    // to show in "DiagApp Data" panel (left)
    int    DiagAppDataInd[2];  //indexes of corresponding FamilyNames in array-> {first, number}
    float  DiagAppDataVal[7];  // values  (running mean)
    float  DiagAppDataStd[7];  // standard deviations
    int    DiagAppDataStat[7]; // status :  0=normal, 1=warning, 2=alarm

    // to show in "External..." panel (right)
    int    ExternalInd[2];  //indexes of corresponding FamilyNames in array-> {first, number}
    float  ExternalVal[9];  // values
    float  ExternalStd[9];  // standard deviations
    int    ExternalStat[9]; // status :  0=normal, 1=warning, 2=alarm



    //****  Crate BCU1 tab      -----------------------------------------------------------------------
    // first index is a chosen Crate (0 to 5) or Switch BCU(=6), if applied
    // to show in "NIOS Fixed Area" panel (left/upper)
    int    BCU1NiosInd[2][2];  //indexes of corresponding FamilyNames in array-> {first, number}
                               //Crates   Switch <- ...Ind[1][...]
    float  BCU1NiosVal[7][11];  // values  (running mean)
    float  BCU1NiosStd[7][11];  // standard deviations
    int    BCU1NiosStat[7][11]; // status :  0=normal, 1=warning, 2=alarm

    // to show in "Environment" panel (middle/upper)
    int    BCU1EnvInd[2];     //indexes of corresponding FamilyNames in array-> {first, number}
    float  BCU1EnvVal[6][6];  // values  (running mean)
    float  BCU1EnvStd[6][6];  // standard deviations
    int    BCU1EnvStat[6][6]; // status :  0=normal, 1=warning, 2=alarm

    // to show in "Voltage and Currents" panel (right/upper)
    int    BCU1VolInd[2];      //indexes of corresponding FamilyNames in array-> {first, number}
    float  BCU1VolVal[6][10];  // values  (running mean)
    float  BCU1VolStd[6][10];  // standard deviations
    int    BCU1VolStat[6][10]; // status :  0=normal, 1=warning, 2=alarm

    // to show in "Temperatures" panel (middle/down)
    int    BCU1TempInd[2][2];  //indexes of corresponding FamilyNames in array-> {first, number}
                               //Crates   Switch <- ...Ind[1][...]
    float  BCU1TempVal[7][2];  // values  (running mean)
    float  BCU1TempStd[7][2];  // standard deviations
    int    BCU1TempStat[7][2]; // status :  0=normal, 1=warning, 2=alarm

    // to show in "SIGGEN Temperatures" panel (right/down)
    int    BCU1SGNTempInd[2];     //indexes of corresponding FamilyNames in array-> {first, number}
    float  BCU1SGNTempVal[6][3];  // values  (running mean)
    float  BCU1SGNTempStd[6][3];  // standard deviations
    int    BCU1SGNTempStat[6][3]; // status :  0=normal, 1=warning, 2=alarm



    //****  Crate BCU2 tab      -----------------------------------------------------------------------
    // first index is a chosen Crate (0 to 5) , if applied
    // to show in "Reset Status" panel (left/upper)
    int    BCU2ResetInd[2];      //indexes of corresponding FamilyNames in array-> {first, number}
    int    BCU2ResetStat[6][10]; // status :  0=notset, 1=set

    // to show in "Digital I/O" panel (middle/upper)
    int    BCU2DigInd[2];      //indexes of corresponding FamilyNames in array-> {first, number}
    int    BCU2DigStat[6][10]; // status :  0=notset, 1=set

    // to show in "Master crate global..." panel (right/upper)
    int    BCU2MastInd[2];  //indexes of corresponding FamilyNames in array-> {first, number}
    float  BCU2MastVal[3];  // values  (running mean)
    float  BCU2MastStd[3];  // standard deviations
    int    BCU2MastStat[3]; // status :  0=normal, 1=warning, 2=alarm

    // to show in "SIGGEN NIOS Fixed Area" panel (middle/down)
    int    BCU2SGNInd[2];  //indexes of corresponding FamilyNames in array-> {first, number}
    float  BCU2SGNVal[4];  // values  (running mean)
    float  BCU2SGNStd[4];  // standard deviations
    int    BCU2SGNStat[4]; // status :  0=normal, 1=warning, 2=alarm

    // to show in  right/down  panel
    int    BCU2TrInd[2];  //indexes of corresponding FamilyNames in array-> {first, number}
    float  BCU2TrVal[4];  // values  (running mean)
    float  BCU2TrStd[4];  // standard deviations
    int    BCU2TrStat[4]; // status :  0=normal, 1=warning, 2=alarm



    //****  Crate DSP tab      -----------------------------------------------------------------------
    // first  index is a chosen Crate (0 to 5) , if applied
    // second index is a chosen Board (0 to 13) , if applied
    // third  index is a chosen Channel (0 to 7) , if applied
    // to show in "NIOS Fixed Area" panel (left/upper)
    int    DSPNiosInd[2];  //indexes of corresponding FamilyNames in array-> {first, number}
    float  DSPNiosVal[8];  // values  (running mean)
    float  DSPNiosStd[8];  // standard deviations
    int    DSPNiosStat[8]; // status :  0=normal, 1=warning, 2=alarm

    // to show in "Coil currents (SPI)" panel (middle/upper)
    int    DSPCurInd[2];         //indexes of corresponding FamilyNames in array-> {first, number}
    float  DSPCurVal[6][14][8];  // values  (running mean)
    float  DSPCurStd[6][14][8];  // standard deviations
    int    DSPCurStat[6][14][8]; // status :  0=normal, 1=warning, 2=alarm

    // to show in "Driver Status" panel (right/upper)
    int    DSPDrSInd[2];          //indexes of corresponding FamilyNames in array-> {first, number}
    int    DSPDrSStat[6][14][10]; // status :  0=notset, 1=set

    // to show in "Temperatures" panel (left/middle)
    int    DSPTempInd[2];         //indexes of corresponding FamilyNames in array-> {first, number}
    float  DSPTempVal[6][14][4];  // values  (running mean)
    float  DSPTempStd[6][14][4];  // standard deviations
    int    DSPTempStat[6][14][4]; // status :  0=normal, 1=warning, 2=alarm

    // to show in "Driver Enabled" panel (middle/down)
    int    DSPDrInd[2];         //indexes of corresponding FamilyNames in array-> {first, number}
    int    DSPDrStat[6][14][8]; // status :  0=notset, 1=set

    // to show in right/down panel
    int    DSPadInd[2];            //indexes of corresponding FamilyNames in array-> {first, number}
    float  DSPadVal[6][14][8][6];  // values  (running mean)
    float  DSPadStd[6][14][8][6];  // standard deviations
    int    DSPadStat[6][14][8][6]; // status :  0=normal, 1=warning, 2=alarm

    // to show in Warnings & Alarms
    vector<string> Warnings;
    vector<string> Alarms;


public slots:
    virtual void newPlot();

    virtual void StopStartTimer();

//  Get&Set All Housekeeper variables
    virtual void Refresh ();
    virtual void GetAll ();
    virtual void SetAll ();

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
    QVBoxLayout* topLBT672Layout;
    QHBoxLayout* tabSYSLayout;
    QGridLayout* frameDiagnLayout;
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

protected slots:
    virtual void languageChange();

};

#endif // TOPLBT672_H
