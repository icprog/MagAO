
#ifndef OPTLOOPGUI_H_INCLUDED
#define OPTLOOPGUI_H_INCLUDED

#include "QtDesigner/optloopdiagn_gui.h"

#include <qevent.h>
#include <qobject.h>

#include "AOAppGui.h"

#include "../OptLoopDiagnostic/OptLoopDiagnosticStructs.h"
#include "arblib/base/ArbitratorInterface.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>


using namespace Arcetri;

#define REPLY_EVENT_ID         65435
#define WARNING_EVENT_ID         65436
#define SAVEFILE_EVENT_ID         65437


class ReplyEvent : public QCustomEvent {

  public:
      ReplyEvent(optsave_reply reply): QCustomEvent(REPLY_EVENT_ID), _reply(reply) {}
      optsave_reply getReply() const { return _reply; }
                  
     private: 
     optsave_reply _reply;
};

class WarningEvent : public QCustomEvent {

  public:
     WarningEvent(string text): QCustomEvent(WARNING_EVENT_ID), _text(text) {}
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






class OptLoopDiagnGui : public AOAppGui, public OptLoopDiagn {

public:
   OptLoopDiagnGui( string name, string configFile, KApplication &kApp);
   OptLoopDiagnGui( int argc, char *argv[], KApplication &kApp);

   virtual ~OptLoopDiagnGui();

protected:
      void InstallHandlers();
      void PostInit();

      static int optsavereply_handler(MsgBuf *msgb, void *argp, int hndlrQueueSize);

      void customEvent( QCustomEvent *e); 


private:
    void buttonSave_clicked();
    void checkSaveCcd47_clicked();
    void checkSaveIrtc_clicked();
    void checkSavePisces_clicked();
    void saveThread();

    ArbitratorInterface* _wfsIntf;
    bool _saving;



private:
};

#endif // OPTLOOPGUI_H_INCLUDED




