#ifndef PSF_OPTIMIZATION_GUI_INCLUDED
#define PSF_OPTIMIZATION_GUI_INCLUDED

#include "QtDesigner/psfOptimization_gui.h"
#include "arblib/base/ArbitratorInterface.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <qevent.h>
#include <qobject.h>

#include "AOAppGui.h"
using namespace Arcetri;

#define LOG_EVENT_ID         65434
#define SAVEFILE_EVENT_ID         65435
#define WARNING_EVENT_ID         65436


class LogEvent : public QCustomEvent {

  public:
    LogEvent(string text): QCustomEvent(LOG_EVENT_ID), _text(text) {}
    string getText() const { return _text; }

  private:
    string _text;
};

class SaveFileEvent : public QCustomEvent {

  public:
    SaveFileEvent(string text): QCustomEvent(SAVEFILE_EVENT_ID), _text(text) {}
    string getText() const { return _text; }

  private:
    string _text;
};

class WarningEvent : public QCustomEvent {

  public:
    WarningEvent(string text): QCustomEvent(WARNING_EVENT_ID), _text(text) {}
    string getText() const { return _text; }

  private:
    string _text;
};



class psfOptimizationGui: public AOAppGui, public psfOptimization_gui {

public:
   psfOptimizationGui( string name, string configFile, KApplication &kApp);
   psfOptimizationGui( int argc, char *argv[], KApplication &kApp);

   virtual ~psfOptimizationGui();

protected:
      void PostInit();

      void init();

      void customEvent( QCustomEvent *e);


private:
    void buttonStart_clicked();
    void comboM2C_activated( const QString & );
    void kURLGain_openFileDialog( KURLRequester *kurl);


    ArbitratorInterface* _wfsIntf;
    ArbitratorInterface* _adsecIntf;

    int sendCommandWithReply( ArbitratorInterface *intf, Command* cmd, Command **reply=NULL);

    void acquireThread();



};

#endif // PSF_OPTIMIZATION_GUI_INCLUDED




