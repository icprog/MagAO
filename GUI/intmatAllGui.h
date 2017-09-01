#ifndef INTMAT_ALL_H_INCLUDED
#define INTMAT_ALL_H_INCLUDED

#include "QtDesigner/intmatAll_gui.h"
#include "arblib/base/ArbitratorInterface.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <qthread.h>
#include <qevent.h>
#include <qobject.h>

#include "AOApp.h"
using namespace Arcetri;

#define LOG_EVENT_ID         65434
#define SAVEFILE_EVENT_ID         65435
#define WARNING_EVENT_ID         65436
#define GENERATE_EVENT_ID         65437
#define ACQUISITION_EVENT_ID         65438


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

class GenerateEvent : public QCustomEvent {

  public:
    GenerateEvent(string text): QCustomEvent(GENERATE_EVENT_ID), _text(text) {}
    string getText() const { return _text; }

  private:
    string _text;
};

class AcquisitionEvent : public QCustomEvent {

  public:
    AcquisitionEvent(string text): QCustomEvent(ACQUISITION_EVENT_ID), _text(text) {}
    string getText() const { return _text; }

  private:
    string _text;
};



class intmatAllGui: public AOApp, public QThread, public intmatAll {

public:
   intmatAllGui( string name, string configFile);
   intmatAllGui( int argc, char *argv[]);

   virtual ~intmatAllGui();

protected:
      void PostInit();
      void Run();

      void run();


      void init();

      void customEvent( QCustomEvent *e);
      int recalcLen(void);

      void acceptModulation( string filename);
      void acceptAcquisition( string filename);
      void generateThread();
      void acquireThread();
      void analyseThread();


private:
    void buttonAcquire_clicked();
    void radioPP_clicked();
    void radioSin_clicked();

    void buttonStartModulation_clicked();
    void buttonChangeDisturbance_clicked();
    void buttonChangeAcquisition_clicked();
    void buttonStartAcquire_clicked();
    void buttonStartAnalyse_clicked();

    void comboM2C_activated( const QString & );
    void spinBoxIterations_valueChanged( int );

    ArbitratorInterface* _wfsIntf;
    ArbitratorInterface* _adsecIntf;


};

#endif // INTMAT_ALL_H_INCLUDED




