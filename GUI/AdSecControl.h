#include "QtDesigner/AdSecControl_gui.h"	// the GUI
#include "AbstractArbControl.h"				// the BEHAVIOUR
#include "AdSecCalibration.h"				// the BEHAVIOUR
#include "AdamLib.h"

#define EXPIRE_TIME         10     // Lifetime for a variable


#include <vector>
#include <string>
using namespace std;

using namespace Arcetri;
using namespace Arcetri::Arbitrator;

#define ADSECCONTROL_NONE_STR          "NONE"
#define ADSECCONTROL_FS_STR            "FocalStation_"
#define ADSECCONTROL_OFFLOAD_ACTDIFF   5 // in order to tell if the HO offloads are active
#define SKIPSTATUS_EVENT_ID            (FIRST_EVENT_ID)

class SkipStatusEvent : public QCustomEvent {

  public:
    SkipStatusEvent( bool on, int count, double percent): QCustomEvent(SKIPSTATUS_EVENT_ID) { _on = on; _count = count; _percent = percent; }
    bool on() const { return _on; }
    int count() const { return _count; }
    double percent() const { return _percent; }

  private:
    bool _on;
    int _count;
    double _percent;
};



class AdSecControl:public AdSecControlGui, public AbstractArbControl {

    Q_OBJECT

  public:

    AdSecControl(string name, string configFile, KApplication &a);

    AdSecControl(int argc, char* argv[], KApplication &a);

    ~AdSecControl();

    pthread_mutex_t _displayMutex;

  protected:


    void setArbitratorStatus(string arbStatus, string lastCmdDescr, string lastCmdResult);
    void logAlert(string log);
    void logText(string log);
    void enableGui(bool enable);
    void setMessage(string text);
    void setAsynchMessage(string text);

    virtual void customEvent(QCustomEvent* e);    //thread separato per mandare il comando all'Arbitrator. La risposta viene impacchettata in un custom event. (non usato da alfio perche' la risposta dell'arbitrator e' una finestra di avvertimento)

    //Reads the autogain.conf file to determine how to set the gain ranges.
    void readAutogainConf();

  private:

    void init();

    // --- All the commands ---//
    void onButton_clicked();
    void offButton_clicked();
    void loadProgramButton_clicked();
    void calibrateButton_clicked();
    void resetButton_clicked();
    void setFlatChopButton_clicked();
    void setFlatSlButton_clicked();
    void setFlatAoButton_clicked();
    void restButton_clicked();
    void resetChopButton_clicked();
    void resetSlButton_clicked();
    void resetAoButton_clicked();
    void chopToSlButton_clicked();
    void slToChopButton_clicked();
    void aoToSl_clicked();
    void slToAoButton_clicked();
    void chopToAoButton_clicked();
    void aoToChopButton_clicked();
    void runChopButton_clicked();
    void stopChopButton_clicked();
    void runAoButton_clicked();
    void stopAoButton_clicked();
    void pauseAoButton_clicked();
    void resumeAoButton_clicked();
    void setGainButton_clicked();
    void buttonOffloadEnable_clicked();
    void buttonOffloadDisable_clicked();
    void buttonGainZero_clicked();
    void buttonGain10modes_clicked();

    void buttonGainTTUp_clicked();
    void buttonGainTTDown_clicked();
    void buttonGainHO1Up_clicked();
    void buttonGainHO2Up_clicked();
    void buttonGainHO1Down_clicked();
    void buttonGainHO2Down_clicked();

    void z9Input_returnPressed();
    void z8Input_returnPressed();
    void z7Input_returnPressed();
    void z6Input_returnPressed();
    void z5Input_returnPressed();
    void z3Input_returnPressed();
    void z2Input_returnPressed();

    void shapeLoad_clicked();
    void shapeSaveButton_clicked();
    void commandApplyVector_clicked();
    void buttonApply_clicked();
    void zernApply_clicked();
    void buttonSetFocalStation_clicked();

    void buttonHOOOffloadEnable_clicked();
    void buttonHOOffloadDisable_clicked();


    void gainApplyButton_clicked();
    void buttonDisturbSend_clicked();
    void recoverFailButton_clicked();

    void comboModalBasis_activated( const QString &str );
    void kURLrecMatrix_openFileDialog( KURLRequester *kurl);
    void kURLgainVector_openFileDialog( KURLRequester *kurl);
    void kURLdisturbSelect_openFileDialog( KURLRequester *kurl);
    void kURLshapeLoad_openFileDialog( KURLRequester *kurl);
    void kURLshapeSave_openFileDialog( KURLRequester *kurl);
    void kURLcommandVector_openFileDialog( KURLRequester *kurl);
    void kURLrecMatrix_urlSelected(const QString&);
    void forceDisable_clicked();
    void ForceTssOn_toggled( bool on);
    void ForcePowerOff_clicked();

    void saveSlopesButton_clicked();

    void processReply( Command *reply);

    void buttonKalmanApply_clicked();

    void updateArbStatus();
    void SetupVarsChild();

    // Handler for variable notifications
    static int updateDisplay(void* thisPtr, Variable* var);
    static int updatePupils(void* thisPtr, Variable* var);


    void applyGain( string filename, bool updateEditBox=false);
    string writeGain( double tt, double ho1, double ho2);
    void getCurGain( double& tt, double& ho1, double &ho2);

    void updateGainRanges( string recpath);



    Adam* adam;
    // AlertNotifier* adSecArbNotifier;



  protected:


    static RTDBvar _adsecM2C;
    static RTDBvar _adsecB0_A;
    static RTDBvar _adsecA_DELAY;
    static RTDBvar _adsecG_GAIN_A;
    static RTDBvar _adsecDISTURB;
    static RTDBvar _adsecSHAPE;
    static RTDBvar _ttOffloadENABLE;
    static RTDBvar _ttOffloadACTIVE;
    static RTDBvar _hoOffloadENABLE;
    static RTDBvar _hoOffloadTIME;
    static RTDBvar _windVALUE;
    static RTDBvar _elevVALUE;
    static RTDBvar _swaDEPLOYED;
    static RTDBvar _coilSTATUS;
    static RTDBvar _tssSTATUS;
    static RTDBvar _mainpowerSTATUS;
    static RTDBvar _labModeStatus;
    static RTDBvar _focalStation;
    static RTDBvar _zernikeTotal;
    static RTDBvar _processDumpText;
    static RTDBvar _globalProgress;
    static RTDBvar _forceTSS;

    RTDBvar _ccd39bin; //Added by Jared for synch with autogain

    bool _elevOK;
    bool _windOK;

    Config_File *arbcfg;
    double _minElevationSet;
    int    _anemometerFromAOS;

    vector <string> focalStationAddr;

  public:
    string _m2c;
    string _b0_a;
    string _a_delay;
    string _g_gain_a;
    string _disturb;
    string _shape;
    AdSecCalibration *calibDialog;
    bool _ttOffload;
    bool _ttOffloadActive;
    bool _hoOffload;
    time_t  _hoOffloadTime;
    double _windValue;
    double _elevValue;
    int _swaDeployed;
    bool _mainpowerStatus;
    bool _coilStatus;
    int  _tssStatus;
    bool _labMode;
    string _focalStationString;
    string _processDumpTextString;
    double _z2;
    double _z3;
    double _z5;
    double _z6;
    double _z7;
    double _z8;
    double _z9;
    string wfsPupils;
    void redisplay();
    void Periodic();
    static RTDBvar _wfsPupils;
    int _progress;
    int _ho_middle;

    //Added by Jared to match autogain:
    int bin1_ho_middle;
    int bin2_ho_middle;
    int bin3_ho_middle;
    int bin4_ho_middle;

    // -- Log file following
            public slots:
            void slotDataAvail();
    void slotDataAvail2();

    bool isCurrentPupil( string recPup);




};
