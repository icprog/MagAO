#include "QtDesigner/AdOptControl_gui.h"	// the GUI
#include "AbstractArbControl.h"				// the BEHAVIOUR


using namespace Arcetri;
using namespace Arcetri::Arbitrator;


class AdOptControl:public AdOptControlGui, public AbstractArbControl {

    Q_OBJECT
    
  public:
    
    AdOptControl(string name, string configFile, KApplication &kApp);

    AdOptControl(int argc, char* argv[], KApplication &kApp);

    ~AdOptControl();



  protected:

    void setArbitratorStatus(string arbStatus, string lastCmdDescr, string lastCmdResult);
    void enableGui(bool enable);
    void setMessage(string text);
    virtual void customEvent(QCustomEvent* e);


    bool _wfsOn;
    bool _adSecOn;
    bool _adSecSet;
    string _adsecStatus;
    string _arbStatus;
    string _aoMode;
    string _wfsStatus;
    bool _wfsSoftware;
    bool _adsecSoftware;


    pthread_mutex_t _updateStatusMutex;

  private:

    void ex(string cmd);

    void init();

    // --- All the commands ---//
    void buttonPowerOnAdSec_clicked();
    void buttonPowerOffAdSec_clicked();
    void buttonAdsecMirrorSet_clicked();
    void buttonAdsecMirrorRest_clicked();
    void buttonRecoverAdSecFailure_clicked();
    void buttonAdSecSwStart_clicked();
    void buttonAdSecSwStop_clicked();

    void buttonPowerOnWfs_clicked();
    void buttonPowerOffWfs_clicked();
    void buttonRecoverWfsFailure_clicked();

    void buttonWfsSwStart_clicked();
    void buttonWfsSwStop_clicked();

    void processReply( Command *cmd);
    void updateArbStatus();
    void SetupVarsChild();

            public slots:
            void slotDataAvail();
    void slotDataAvail2();

  protected:
    static RTDBvar _aoSubstate;
    static RTDBvar _wfsSource;
    static RTDBvar _globalProgress;
    static int OnNotify(void *, Variable *);


};
