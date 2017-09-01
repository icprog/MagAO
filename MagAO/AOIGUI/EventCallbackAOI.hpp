#include <cstring>
#include <qtextedit.h>
#include <tcs/eventhandler/EventHandler.hpp>

#define AOSGUImsgType 54321

using namespace std;

class EventCallbackAOS
{
  ///  construction/destruction
public:
    EventCallbackAOS(AOSGUIForm *mainWindow);
    virtual ~EventCallbackAOS();

  //  general methods.
public:
    ///  this function contains the code to handle an event.
    string  eventHandle( string szData );
    string  sideFilter;
private:
    AOSGUIForm *mw;
};

class AOSAppendEvent : public QCustomEvent
{
public:
    AOSAppendEvent(QString line) : QCustomEvent(AOSGUImsgType), l(line) {}
    QString line() {return l;}
private:
    QString l;
};
