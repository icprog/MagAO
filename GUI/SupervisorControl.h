#include "QtDesigner/SupervisorControl_gui.h"	// the GUI
#include "AbstractArbControl.h"			// the BEHAVIOUR

#include "arblib/aoArb/AOPTypes.h"

#include <boost/thread/mutex.hpp>

using namespace Arcetri;
using namespace Arcetri::Arbitrator;


class SupervisorControl: public SupervisorControlGui, public AbstractArbControl {

    Q_OBJECT

  public:

    SupervisorControl(string name, string configFile, KApplication &kApp);
    SupervisorControl(int argc, char* argv[], KApplication &kApp);
    ~SupervisorControl();


  protected:

    void setArbitratorStatus(string arbStatus, string lastCmdDescr, string lastCmdResult);
    void enableGui(bool enable);
    void logText(const char* fmt, ...);

  private:

    void init();
    void processReply( Command *reply);
    void displayAcquireRefResult( acquireRefAOResult result, string source);

    // --- All the commands ---//
  public slots:
    void presetFlatButton_clicked();
    void presetAOButton_clicked();
    void buttonCheckRefAO_clicked();
    void buttonAcquireRef_clicked();
    void buttonModifyAO_clicked();
    void buttonClose_clicked();
    void buttonPause_clicked();
    void buttonResume_clicked();
    void buttonOpen_clicked();

    void buttonStopFlat_clicked();
    void buttonStopPreset_clicked();


  protected:
    void PostInit();

  public slots:
    void slotDataAvail();
    void slotDataAvail2();
};
