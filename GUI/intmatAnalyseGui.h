#ifndef INTMAT_ANALYSE_GUI_H_INCLUDED
#define INTMAT_ANALYSE_GUI_H_INCLUDED

#include "QtDesigner/intmatAnalyse_gui.h"

#include <qevent.h>
#include <qobject.h>

#include "AOAppGui.h"
using namespace Arcetri;

class intmatAnalyseGui : public AOAppGui, public intmatAnalyse_gui {

public:
   intmatAnalyseGui( string name, string configFile, KApplication &kApp);
   intmatAnalyseGui( int argc, char *argv[], KApplication &kApp);

   virtual ~intmatAnalyseGui() {};

protected:
      void PostInit();

      void init();

private:
    void buttonAnalyse_clicked();
    void comboM2C_activated( const QString & );


};

#endif // INTMAT_ANALYSE_GUI_H_INCLUDED




