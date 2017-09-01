#ifndef ADSECCALIBRATION_H
#define ADSECCALIBRATION_H

#include <qtimer.h>
#include "AbstractArbControl.h"	 
#include "QtDesigner/AdSecCalibration_gui.h"


#define ADSEC_CALIB_TIMEOUT 60000

class AdSecCalibration : public AdSecCalibration_gui {

   Q_OBJECT

public:
   AdSecCalibration(AbstractArbControl *arbcontrol);
   virtual ~AdSecCalibration() {};

public slots:
    virtual void buttonAccelerometer_clicked();
    virtual void buttonTurbulenceTrack_clicked();
    virtual void buttonDynamicResponse_clicked();
    virtual void buttonFFMatrix_clicked();
    virtual void buttonTestNoise_clicked();
    virtual void buttonStepResponse_clicked();
    virtual void buttonFindTouch_clicked();
    virtual void buttonAcq4DIntMat_clicked();
    virtual void buttonFlat4DMirror_clicked();

private slots:
    void Periodic();

protected:
    QTimer *timer;
    AbstractArbControl *_arbctl;

    void enableButtons();

};

#endif


