
#include "QtDesigner/AdSecConf_gui.h"

#include <qthread.h>
#include <qevent.h>
#include <qobject.h>
#include <qtimer.h>
#include <AOApp.h>

#define NUM_ACTUATORS 672
#define NUM_ACTPERCOL 32

class AdSecConfGui : public AdSecConfWizard, public QThread {

   Q_OBJECT

  public:
    
    QPushButton * actPos[NUM_ACTUATORS];
    QPushButton * actCurr[NUM_ACTUATORS];
    QPushButton * actIcc[NUM_ACTUATORS];

    QHBoxLayout* actPosLayout[NUM_ACTUATORS/NUM_ACTPERCOL];
    QHBoxLayout* actCurrLayout[NUM_ACTUATORS/NUM_ACTPERCOL];
    QHBoxLayout* actIccLayout[NUM_ACTUATORS/NUM_ACTPERCOL];

    AdSecConfGui( int argc, char *argv[]);
    ~AdSecConfGui();
    pthread_mutex_t _displayMutex;



protected:
   void run();
   void redisplay();
   std::vector<std::string>  parseDir(string dir2parse, string pattern="");


private slots:
   void unitList_activated(const QString&);
   void shellList_activated(const QString&);
   void updateFitsTN();
   void updateTxtTN();


private:
    //void parkButton_clicked();


private:
    std::string _shell;
    std::string _unit;
    std::string _dspver;
    std::string _cbcuver;
    std::string _sbcuver;
    std::string _accver;
    Config_File * _cfgElec;
    std::string _currElecTN;
    std::string _currFFmatrixTN;
    std::string _currOptDataTN;
    std::string _currFlatTN;
    std::string _zernTN;
    std::string _hoTN;
    std::string _loZernTN;
    std::string _hoZernTN;
    std::string _fastParamTN;
    std::string _hkprParamTN;
    //bool _aosInit;

};




