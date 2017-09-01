#ifndef INTMAT_ACQUIRE_GUI_H_INCLUDED
#define INTMAT_ACQUIRE_GUI_H_INCLUDED

#include "QtDesigner/intmatAcquire_gui.h"
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



class intmatAcquireGui: public AOAppGui, public intmatAcquire_gui {

public:
   intmatAcquireGui( string name, string configFile, KApplication &kApp);
   intmatAcquireGui( int argc, char *argv[], KApplication &kApp);

   virtual ~intmatAcquireGui();

	// Variable handler
      static int varHandler(void* thisPtr, Variable* var);

      RTDBvar & varCcdFreq() { return _varCcdFreq; }
      RTDBvar & varDriftEnabledCur() { return _varDriftEnabledCur; }


	double _freq;

protected:
      void PostInit();

      void init();

      void customEvent( QCustomEvent *e);
      int recalcLen(void);


private:
    void buttonAcquire_clicked();
    void comboM2C_activated( const QString & );
    void comboDisturb_activated( const QString & );
    void spinBoxIterations_valueChanged( int );
    void checkCloseloop_clicked();
    void kURLGain_openFileDialog( KURLRequester *);
    void kURLRec_openFileDialog( KURLRequester *);

    ArbitratorInterface* _wfsIntf;
    ArbitratorInterface* _adsecIntf;

    int sendCommandWithReply( ArbitratorInterface *intf, Command* cmd, Command **reply=NULL);

    void acquireThread();

    RTDBvar _varCcdFreq;
    RTDBvar _varDriftEnabledCur;


};

#endif // INTMAT_ACQUIRE_GUI_H_INCLUDED




