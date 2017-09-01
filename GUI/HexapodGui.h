
#include "QtDesigner/hexapod_gui.h"
#include "RTDBvar.h"

#include <qthread.h>
#include <qevent.h>
#include <qobject.h>
#include <qtimer.h>

#include "AOApp.h"
extern "C"{
   #include "aoslib/aoslib.h"
}

using namespace Arcetri;


class HexapodGui : public HexapodGUI, public QThread, public AOApp {

   Q_OBJECT

public:
   HexapodGui( string name, string configFile);
   HexapodGui( int argc, char *argv[]);

   virtual ~HexapodGui() {};

protected:
//    void InstallHandlers();
      void PostInit();
      void Run();

      void run();


      void init();

      static int curpos_changed( void *thisPtr, Variable *var);

      void setCurPos( double *pos);


      int moveBy( float x, float y, float z, float a1, float a2, float a3);

      void thSetup();
      void thPark();

      QTimer *timer;

      double _curPos[6];

private slots:
      void Periodic();



private:
    void moveX_clicked();
    void moveY_clicked() {}
    void moveZ_clicked() {}
    void moveTip_clicked() {}
    void moveTilt_clicked() {}
    void setupButton_clicked();
    void stopButton_clicked();
    void parkButton_clicked();


private:
    string v_curPos_name;
    RTDBvar *v_curPos;
    double _init_x;
    double _init_y;
    double _init_z;
    double _init_a;
    double _init_b;
    double _init_c;

    bool _aosInit;

};




