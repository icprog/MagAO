#ifndef INTMAT_DISTURB_GUI_H_INCLUDED
#define INTMAT_DISTURB_GUI_H_INCLUDED

#include "QtDesigner/intmatDisturb_gui.h"

#include <qthread.h>
#include <qevent.h>
#include <qobject.h>

#include "AOAppGui.h"
using namespace Arcetri;

class intmatDisturbGui : public AOAppGui, public intmatDisturb_gui {

public:
   intmatDisturbGui( string name, string configFile, KApplication &kApp);
   intmatDisturbGui( int argc, char *argv[], KApplication &kApp);

   virtual ~intmatDisturbGui() {};

protected:
      void PostInit();

      void init();

      int recalcDisturbLength();
      int setDisturbLengthLabel( int len);


private:
    void radioPP_clicked();
    void radioSin_clicked();
    void buttonGenerate_clicked();
    void comboM2C_activated( const QString & );

    void editModesNum_textChanged( const QString & );
    void editFramesNum_textChanged( const QString & );
    void editCyclesNum_textChanged( const QString & );

    void kURLfileLgs_openFileDialog( KURLRequester *kurl );




};

#endif // INTMAT_DISTURB_GUI_H_INCLUDED




