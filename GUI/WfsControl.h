#include "QtDesigner/WfsControl_gui.h"	// the GUI
#include "AbstractArbControl.h"			// the BEHAVIOUR

#include "arblib/wfsArb/WfsInterfaceDefines.h"

using namespace Arcetri;
using namespace Arcetri::Arbitrator;

#define LOOPSTATUS_EVENT_ID             (FIRST_EVENT_ID)
#define CALIB_UNIT_EVENT_ID             (FIRST_EVENT_ID+1)

class CalibUnitEvent : public QCustomEvent {

  public:
    CalibUnitEvent( string sourceName): QCustomEvent(CALIB_UNIT_EVENT_ID) { _sourceName = sourceName; }
    string sourceName() const { return _sourceName; }

  private:
    string _sourceName;
};




class WfsControl: public WfsControlGui, public AbstractArbControl {

	Q_OBJECT

	public:

		WfsControl(string name, string configFile, KApplication &kApp);
		WfsControl(int argc, char* argv[], KApplication &kApp);
		~WfsControl();

      bool _driftEnabled;
      float _driftAmount;
      bool _rerotTrackEnabled;
      bool _adcTrackEnabled;
      bool _lensTrackEnabled;
      bool _lensTrackTarget;
      bool _lensTrackOutOfRange;
      bool _lensTrackAlert;
      int  _fastlinkEnabled;
      void redisplay();
      void displayEnabled( QLabel *label, bool enabled, string enabledText="Enabled", string disabledText="Disabled",const QColor *enabledColor=NULL, const QColor *disabledColor=NULL);


	protected:


		void setSupervisorStatus(string status);
		void setArbitratorStatus(string arbStatus, string lastCmdDescr, string lastCmdResult);
		void logAlert(string log);
		void logText(string log);
		void enableGui(bool enable, bool enableStop);
		void setMessage(string text);
		void setAsynchMessage(string text);
      
      void updateArbStatus();
      void SetupVarsChild();

      virtual void customEvent(QCustomEvent* e);





 private:
		void init();

	private:


		// --- All the commands ---//
		void operateButton_clicked();
		void offButton_clicked();
		void closeLoopButton_clicked();
		void pauseLoopButton_clicked();
		void resumeLoopButton_clicked();
		void stopLoopButton_clicked();
		void calibrateHODarkButton_clicked();
		void calibrateTVDarkButton_clicked();
		void calibrateIRTCDarkButton_clicked();
		void calibratePupilsButton_clicked();
		void calibrateMovementsButton_clicked();
		void calibrateSlopenullButton_clicked();
      void showStatusButton_clicked();
      void checkBoardSetup_clicked();
      void buttonCCD39_clicked();
      void buttonCCD47_clicked();
      void buttonFreqApply_clicked(); 
      void radioDisturbanceDisabled_clicked();
      void radioDisturbanceEnabledWFS_clicked();
      void radioDisturbanceEnabledAlways_clicked();
      void buttonXYoffset_clicked();
      void buttonZoffset_clicked();
      void buttonStopAutogain_clicked();
      void buttonRecoverFailure_clicked();
      void buttonAntiDriftOn_clicked();
      void buttonAntiDriftOff_clicked();

      void buttonADCTrackOn_clicked();
      void buttonADCTrackOff_clicked();
      void buttonRerotTrackOn_clicked();
      void buttonRerotTrackOff_clicked();
      void buttonLensTrackOn_clicked();
      void buttonLensTrackOff_clicked();
      void buttonGainOptimize_clicked();

      void processReply( Command *reply);

      string filename2display( string filename, string display_filename);
      string display2filename( string display);

protected:
      static RTDBvar _VarDriftEnabledCur;
      static RTDBvar _VarDriftEnabledReq;
      static RTDBvar _VarDriftAmount;
      static RTDBvar _VarLensTrack;
      static RTDBvar _VarFastlinkEnabled;
      static RTDBvar _VarAutogainStop;;

      // Handlers for variable notifications
      static int updateDrift(void* thisPtr, Variable* var);
      static int updateFastlink(void* thisPtr, Variable* var);



      // -- Log file following
public slots:
      void slotDataAvail();

};
