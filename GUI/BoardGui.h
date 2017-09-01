
#include "QtDesigner/board_gui.h"
#include "RTDBvar.h"

#include <vector>

#include <qevent.h>
#include <qobject.h>
#include <qtimer.h>

#include "AOAppGui.h"

using namespace Arcetri;

#define STAGES_EVENT_ID         65434
#define CCD39_EVENT_ID          65435
#define CCD47_EVENT_ID          65436
#define REROT_EVENT_ID          65437
#define ADC_EVENT_ID            65438
#define CAMERALENS_EVENT_ID     65439
#define FW1_EVENT_ID            65440
#define FW2_EVENT_ID            65441
#define TT_EVENT_ID             65442
#define CLEAN_EVENT_ID          65443
#define TEL_EVENT_ID            65444
#define CUBE_EVENT_ID           65445
#define DEACTIVATE_EVENT_ID     65446

class StagesEvent : public QCustomEvent {
  
  public:
    StagesEvent(float x, float y, float z, float curx, float cury, float tgtx, float tgty): QCustomEvent(STAGES_EVENT_ID),
                _x(x), _y(y), _z(z), _curx(curx), _cury(cury), _tgtx(tgtx), _tgty(tgty) {}

    float curx() const { return _curx; }
    float cury() const { return _cury; }
    float tgtx() const { return _tgtx; }
    float tgty() const { return _tgty; }
    float x() const { return _x; }
    float y() const { return _y; }
    float z() const { return _z; }
  
  private: 
    float _x, _y, _z;
    float _curx, _cury;
    float _tgtx, _tgty;
};

class Ccd39Event : public QCustomEvent {
  
  public:
    Ccd39Event(int bin, float freq, int temp1, int temp2, int temp3): QCustomEvent(CCD39_EVENT_ID), _bin(bin), _freq(freq), _temp1(temp1), _temp2(temp2), _temp3(temp3) {}
    float bin() const { return _bin; }
    float freq() const { return _freq; }
    int temp1() const { return _temp1; }
    int temp2() const { return _temp2; }
    int temp3() const { return _temp3; }
  
  private: 
    float _bin, _freq;
    int _temp1, _temp2, _temp3;
};

class Ccd47Event : public QCustomEvent {
  
  public:
    Ccd47Event(int bin, float freq, int temp1, int temp2, int temp3): QCustomEvent(CCD47_EVENT_ID), _bin(bin), _freq(freq), _temp1(temp1), _temp2(temp2), _temp3(temp3) {}
    float bin() const { return _bin; }
    float freq() const { return _freq; }
    int temp1() const { return _temp1; }
    int temp2() const { return _temp2; }
    int temp3() const { return _temp3; }
  
  private: 
    float _bin, _freq;
    int _temp1, _temp2, _temp3;
};

class RerotEvent : public QCustomEvent {
  
  public:
    RerotEvent(float pos, bool sync): QCustomEvent(REROT_EVENT_ID), _pos(pos), _sync(sync) {}
    float pos() const { return _pos; }
    bool sync() const { return _sync; }
  
  private: 
    float _pos;
    bool _sync;
};

class CubeEvent : public QCustomEvent {
  
  public:
    CubeEvent(float pos): QCustomEvent(CUBE_EVENT_ID), _pos(pos) {}
    float pos() const { return _pos; }
    
  private: 
    float _pos;
};  

class DeactivateEvent : public QCustomEvent {
  
  public:
    DeactivateEvent(QWidget *widget): QCustomEvent(DEACTIVATE_EVENT_ID), _widget(widget) {}
    QWidget *widget() const { return _widget; }
    
  private: 
    QWidget *_widget;
};  


class CameraLensEvent : public QCustomEvent {
  
  public:
    CameraLensEvent(float xpos, float ypos): QCustomEvent(CAMERALENS_EVENT_ID), _xpos(xpos), _ypos(ypos) {}
    float xpos() const { return _xpos; }
    float ypos() const { return _ypos; }
  
  private: 
    float _xpos, _ypos;
};

class AdcEvent : public QCustomEvent {
  
  public:
    AdcEvent(float angle, float disp, bool sync): QCustomEvent(ADC_EVENT_ID), _angle(angle), _disp(disp), _sync(sync) {}
    float angle() const { return _angle; }
    float disp() const { return _disp; }
    bool sync() const { return _sync; }
  
  private: 
    float _angle, _disp;
    bool _sync;
};

class Fw1Event : public QCustomEvent {
  
  public:
    Fw1Event(string posname): QCustomEvent(FW1_EVENT_ID), _posname(posname) {}
    string posname() const { return _posname; }
  
  private: 
    string _posname;
};

class Fw2Event : public QCustomEvent {
  
  public:
    Fw2Event(string posname): QCustomEvent(FW2_EVENT_ID), _posname(posname) {}
    string posname() const { return _posname; }
  
  private: 
    string _posname;
};

class TelEvent : public QCustomEvent {
  
  public:
    TelEvent(double derotPos, double telAz, double telEl): QCustomEvent(TEL_EVENT_ID), _derotPos(derotPos), _telAz(telAz), _telEl(telEl) {}
    double derotPos() const { return _derotPos; }
    double telAz() const { return _telAz; }
    double telEl() const { return _telEl; }
  
  private: 
    double _derotPos;
    double _telAz;
    double _telEl;
};


class TTEvent : public QCustomEvent {
  
  public:
    TTEvent(float amp, float freq): QCustomEvent(TT_EVENT_ID), _amp(amp), _freq(freq) {}
    float amp() const { return _amp; }
    float freq() const { return _freq; }
  
  private: 
    float _amp, _freq;
};

class CleanEvent : public QCustomEvent {
  public:
    CleanEvent(): QCustomEvent(CLEAN_EVENT_ID) {}
};



class BoardGui : public board_gui, public AOAppGui {


   Q_OBJECT

public:
   BoardGui( string name, string configFile, KApplication &kApp);
   BoardGui( int argc, char *argv[], KApplication &kApp);

   void init();

   vector<QWidget *> active;
   void activate( QWidget *widget);

   virtual ~BoardGui() {};

   QSize stagesFrameSize;

    RTDBvar *v_curPosX;
    RTDBvar *v_curPosY;
    RTDBvar *v_curPosZ;
    RTDBvar *v_tgtPosX;
    RTDBvar *v_tgtPosY;
    RTDBvar *v_tgtPosZ;

    RTDBvar *v_ccd39Bin;
    RTDBvar *v_ccd39Freq;
    RTDBvar *v_ccd39Temp;

    RTDBvar *v_ccd47Bin;
    RTDBvar *v_ccd47Freq;
    RTDBvar *v_ccd47Temp;

    RTDBvar *v_fw1Pos;
    RTDBvar *v_fw2Pos;

    RTDBvar *v_rerotPos;
    RTDBvar *v_rerotSync;

    RTDBvar *v_clPosX;
    RTDBvar *v_clPosY;

    RTDBvar *v_adcPos1;
    RTDBvar *v_adcPos2;
    RTDBvar *v_adcSync;

    RTDBvar *v_ttAmp;
    RTDBvar *v_ttFreq;

    RTDBvar *v_derotPos;
    RTDBvar *v_telAz;
    RTDBvar *v_telEl;

    RTDBvar *v_cubePos;

protected:
      void SetupVars();

      void run();

      void customEvent( QCustomEvent *e);

      static int stages_notify( void *thisPtr, Variable *var);
      static int ccd39_notify( void *thisPtr, Variable *var);
      static int ccd47_notify( void *thisPtr, Variable *var);
      static int tt_notify( void *thisPtr, Variable *var);
      static int adc_notify( void *thisPtr, Variable *var);
      static int fw1_notify( void *thisPtr, Variable *var);
      static int fw2_notify( void *thisPtr, Variable *var);
      static int rerot_notify( void *thisPtr, Variable *var);
      static int cube_notify( void *thisPtr, Variable *var);
      static int cl_notify( void *thisPtr, Variable *var);
      static int tel_notify( void *thisPtr, Variable *var);


private slots:
      void Periodic();

private:
    string v_curPos_name;


};




