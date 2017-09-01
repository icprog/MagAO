#include "QtDesigner/msgtool_gui.h"
#include "RTDBvar.h"

#include <qevent.h>
#include <qobject.h>
#include <qtimer.h>

#include "AOApp.h"

using namespace Arcetri;

#define MSGTOOL_NAME              "msgtool"
#define MSGTOOL_IP                "127.0.0.1"
#define MSGTOOL_PING_TIMEOUT      1000
#define MSGTOOL_MSG_TIMEOUT       1000
#define MSGTOOL_MSGD_STR          "MsgD"
#define MSGTOOL_LOCAL_STR         "local"
#define MSGTOOL_STRMAX            128

class msgtool : public msgtool_gui, public AOApp {

    Q_OBJECT

  public:
    msgtool( int argc, char *argv[]);
    ~msgtool();

    void logtext(const char* fmt, ...);
    void Init();

  public slots:
    void buttonPing_clicked();
    void buttonSend_clicked();
    void buttonRefresh_clicked();

  private slots:
    void Periodic();

  private:
    QTimer *timer;
    void PrintReply(const char *dest, int stat);

  protected:
    void PostInit();
    void Run() {};

};
