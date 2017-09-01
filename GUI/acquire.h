#ifndef ACQUIRE_H_INCLUDED
#define ACQUIRE_H_INCLUDED

#include "QtDesigner/acquire_gui.h"
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



class acquire: public AOApp, public QThread, public acquire_gui {

public:
   acquire( string name, string configFile);
   acquire( int argc, char *argv[]);

   virtual ~acquire();

protected:
      void PostInit();
      void Run();

      void run();


      void init();

      void customEvent( QCustomEvent *e);

private:
    void buttonAcquire_clicked();
    void kURLinputFile_URLselected(const QString&);

    ArbitratorInterface* _wfsIntf;
    ArbitratorInterface* _adsecIntf;

    int sendCommandWithReply( ArbitratorInterface *intf, Command* cmd, Command **reply=NULL);

    void acquireThread();

    string doIntmat( float loopfreq, string m2c, string cmddisturb, int iterations);
    string doCloop( int nFrames);
    int calcLen( string disturbFile, int iterations);




};

#endif // ACQUIRE_H_INCLUDED




