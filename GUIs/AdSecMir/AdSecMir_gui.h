/****************************************************************************
****************************************************************************/

#ifndef GUIadSecM_H
#define GUIadSecM_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;


#include <qwidget.h>
#include <qvariant.h>
#include <qdialog.h>
#include <qthread.h>
#include <qtimer.h>



#include "AOApp.h"
#include "Mirror.h"

using namespace Arcetri;

//#define NBUF 500
//#define CRNUM 6
//#define CR1NUM (CRNUM + 1)
//#define BONUM 14
//#define CHNUM 8

#define DUMMY TRUE
#define NRMAX 100      //max number of the Nsamples for Running Mean
#define NVARS  9
#define NVARS1 4
#define NVARS2 9
#define NVARST NVARS+NVARS1
#define NMIR 672

class QPainter;
class QPaintEvent;

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QTabWidget;
class QWidget;
class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;
//class KLed;
//class QButtonGroup;
//class QRadioButton;
//class QListBox;
//class QListBoxItem;
class QMessageBox;
//class QwtPlot;
//class QwtText;
//class QwtTextLabel;

class Mirror;
class PlotModes;




class GUIadSecM : public QDialog, public AOApp
{
    Q_OBJECT

public:
    GUIadSecM( string id, string configFile, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~GUIadSecM();

    QTimer* timer;

//internal widgets
    QTabWidget* tabWidget;
    QWidget* tab1;
    QWidget* tab2;
    //QWidget* tab3;
    //QWidget* tab4;
    //QWidget* tab5;
    //Mirror* mir[16];
    Mirror* mir[6];

    QFrame* frameInfo;
    QLabel* infoLab [NVARS2];
    QLabel* infoText[NVARS2];
    QPushButton* buttonModes;
    QPushButton* buttonModesMean;
    QPushButton* buttonModesRMS;

    QPushButton* buttonStart;
    QLineEdit* lab1line;
    QLineEdit* lab2line;
    QPushButton* buttonOffset;
    QLabel* TimeF;
    QLabel* TimeH;
    QPushButton* buttonHelp;
    QPushButton* buttonExit;


//variables
    int timeoutR; //refresh rate
    int Nint;     //total number of interogations
    int Nsamples; //number of samples to evarage
    int Nr;       //current number of samples to evarage, at begining =0, max =Nsamples-1.

    int    NMirShown;
    int    Init[NVARST];
    int    ibuf[NVARST][NMIR];
    double buf[NVARST][NMIR];
    double bufRmean[NVARST][NMIR];
    double bufr[NVARST][NMIR][NRMAX];
    int    iModes;
    int    OffloadSel;
    double Vswitch [NVARS2];
    int    Vcolor  [NVARS2];
    double timec;
    char   time_F[9]; //timestamp for fastdiagn
    char   time_H[9]; //timestamp for housekeeper
    QColor Red;
    QColor Green;
    QColor TimeFCol;
    QColor TimeHCol;

/*
    // to show in Warnings & Alarms
    vector<string> Warnings;
    vector<string> Alarms;
*/
    vector<PlotModes*> pplot;

public slots:


    void Run();  //to overload the Run method of AOApp

    virtual void quit();  //quit this and related  widgets
    virtual void StopStartTimer(); 
    virtual void TakeOffset();
    virtual void RefreshRate();
    virtual void Refresh();
    virtual void GetAll();
    virtual void SetAll();
    virtual void RunningMean();
    virtual void changeNsamples();
    virtual void GetAllDummy();

    virtual void CreatePlotModes();
    virtual void CreatePlotModesMean();
    virtual void CreatePlotModesRMS();
    virtual void newPlot(char*,int);
    virtual void fillPlot(PlotModes *);
    virtual void remove_plot(int);
    virtual void refreshPlots();


//    virtual void StopStartTimer();



protected:
    void paintEvent ( QPaintEvent * );

    QVBoxLayout* GUIadSecMLayout;
    QHBoxLayout* tab1Layout;
    QHBoxLayout* tab2Layout;
    //QHBoxLayout* tab3Layout;
    //QHBoxLayout* tab4Layout;
    //QHBoxLayout* tab5Layout;
    QHBoxLayout* Layout1;
    QGridLayout* frameInfoLayout;
    QSpacerItem* Horizontal_Spacing2;
    QSpacerItem* Horizontal_Spacing3;

//    void shiftBuf (float *, int);

//    bool eventFilter ( QObject *obj, QEvent *ev );
    //int _fastdiagnIsUnprotected;

protected slots:
    virtual void languageChange();
    //void SetupVars();
    //static int fastdgnUnprotectedHdlr(void *thisPtr, Variable *var);

};



#endif // GUIadSecM_H
