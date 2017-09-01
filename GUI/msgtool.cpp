#define VERS_MAJOR   1
#define VERS_MINOR   0

#include <qapplication.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include "msgtool.h"

extern "C"{
#include "aoslib/aoslib.h"
#include "base/msgtxt.h"
}


//
//
msgtool::msgtool( int argc, char *argv[]) : AOApp(MSGTOOL_NAME, Utils::getAdoptSide(), MSGTOOL_IP,
                                                  Logger::LOG_LEV_DEBUG, false, "NONE") {

    
}

//
//
msgtool::~msgtool() {
    
}

//
//
void msgtool::logtext(const char* fmt, ...) {

    char message[MAXLOGLINELEN];   

    va_list argp;
    va_start(argp, fmt);
    vsnprintf(message, MAXLOGLINELEN-1, fmt, argp);
    va_end(argp);

    textLog->append(message);
}

//
//
void msgtool::PostInit() {

    logtext("Initialized ...");
}

//
//
void msgtool::Init() {

    char str[MSGTOOL_STRMAX];

    // fill clients
    comboProcId->insertItem(MSGTOOL_MSGD_STR);
    // fill domains
    comboDomain->insertItem(MSGTOOL_LOCAL_STR);
    // fill in codes for simple message
    DbTable * msg;
    int i = 0;

    while ((msg = msgtxt_i(i)) != NULL) {
        snprintf(str, MSGTOOL_STRMAX-1, "%s (0x%x)", msg->name, msg->code);
        comboMsgId->insertItem(str);
        i++;
    }
    
    buttonSend->setFocus();

    Periodic();

    timer = new QTimer( this);
    connect( timer, SIGNAL(timeout()), SLOT(Periodic()));
    timer->start(1000);
}    

//
//
void msgtool::Periodic() {

    qInfo *stat = thInfo();

    if (stat->initialized) {
        labelHandlers->setNum(stat->nHandlers);
        labelTotBufs->setNum(stat->nTot);
        labelWtngMsgs->setNum(stat->nWaiting);
        labelFreeBufs->setNum(stat->nFree);
        labelRmvdMsgs->setNum(stat->nFlushd);
    }
    else {
        labelHandlers->setNum(-1);
        labelTotBufs->setNum(-1);
        labelWtngMsgs->setNum(-1);
        labelFreeBufs->setNum(-1);
        labelRmvdMsgs->setNum(-1);
    }
    if (stat->connected) {
        labelStatus->setText("Connected");
    }
    else {
        labelStatus->setText("Not connected");
        // try to reconnect 
        // ConnectBase() is private
        thStart((char*)Server().c_str(),1);
    }

    if (TimeToDie())
        close();
}

//
//
void msgtool::PrintReply(const char *dest, int stat) {

    MsgBuf *ret;
    Variable * var;
    int errcod;

    if ((ret = thWaitMsg(ANY_MSG, dest, stat, MSGTOOL_MSG_TIMEOUT, &errcod)) == NULL) {
        logtext("error waiting for reply (%d) %s",errcod, lao_strerror(errcod));
    }
    else {
        switch HDR_CODE(ret) 
            {
            case ACK:
                if (HDR_LEN(ret))
                    logtext("ACK reply: PLOAD %d BODY (%d) %s", HDR_PLOAD(ret), HDR_LEN(ret), (char *)MSG_BODY(ret));
                else
                    logtext("ACK reply: PLOAD %d", HDR_PLOAD(ret));
                break;

            case NAK:
                logtext("NAK reply");
                break;
                
            case INFOREPLY:
                logtext("INFOREPLY reply: (%d) %s", HDR_LEN(ret), (char *)MSG_BODY(ret));
                break;

            case VERSREPLY:
                logtext("VERSREPLY reply: %d", HDR_PLOAD(ret));
                break;

            case VARREPLY:
                var = thValue(ret);
                logtext("VARREPLY reply: type %d items %d", var->H.Type, var->H.NItems);
                break;

            default:
                if (HDR_LEN(ret))
                    logtext("reply (0x%x): PLOAD %d BODY (%d) %s", HDR_CODE(ret), HDR_PLOAD(ret), 
                            HDR_LEN(ret), (char *)MSG_BODY(ret));
                else
                    logtext("reply (0x%x): PLOAD %d", HDR_CODE(ret), HDR_PLOAD(ret));
                break;
            }
        thRelease(ret);
    }
}

//
//
void msgtool::buttonPing_clicked() {

    string dest;
    int stat;
    double delay;

    if ((comboProcId->currentText().isEmpty()) || (strcmp(comboProcId->currentText(), MSGTOOL_MSGD_STR) == 0))
        dest = "";
    else
        dest = comboProcId->currentText().ascii();
    
    if ((!comboDomain->currentText().isEmpty()) && (strcmp(comboDomain->currentText(), MSGTOOL_LOCAL_STR) != 0)) 
        dest += '@' + string(comboDomain->currentText().ascii());
    
    stat = thPing(dest.c_str(), MSGTOOL_PING_TIMEOUT, &delay);
        
    if (stat<0)
        logtext("ping to %s error %d: %s", dest.c_str(), stat, lao_strerror(stat));
    else
        logtext("reply from %s received in %f ms", dest.c_str(), delay);

    Periodic();
}


//
//
void msgtool::buttonSend_clicked() {

    const char * msg;
    int len, flag;
    int stat;
    string dest;
    DbTable * msgcode;
    int pload = editPLoad->text().toInt();

    switch (tabParameters->currentPageIndex()) 
        {
        case 0: // simple command

            pload = editPLoad->text().toInt();
            if ((comboProcId->currentText().isEmpty()) || (strcmp(comboProcId->currentText(), MSGTOOL_MSGD_STR) == 0))
                dest = "";
            else
                dest = comboProcId->currentText().ascii();
            if ((!comboDomain->currentText().isEmpty()) && (strcmp(comboDomain->currentText(), MSGTOOL_LOCAL_STR) != 0)) 
                dest += '@' + string(comboDomain->currentText().ascii());

            switch (comboFlag->currentItem()) 
                {
                case 1:  flag = NOHANDLE_FLAG; break;
                case 2:  flag = DISCARD_FLAG; break;
                default: flag = 0; break;
                }
            
            if (editMsgText->text().isEmpty()) {
                msg = NULL;
                len = 0;
            }
            else {
                msg = editMsgText->text();
                len = strlen(msg) + 1;
            }

            msgcode = msgtxt_i(comboMsgId->currentItem());
            
            logtext("\nsending %s ...", msgcode->name);
            stat = thSendMsgPl(len, dest.c_str(),  msgcode->code, pload, flag, msg);

            if (stat < 0) {
                logtext("error sending command (%d) %s",stat,lao_strerror(stat));
            }
            else if (checkReply->isOn()) {
                PrintReply(dest.c_str(), stat);
            }
            break;
            
        case 1:
            logtext("msgtool::buttonSend_clicked() serialized command");
            
            break;
        }

    Periodic();
}

//
//
void msgtool::buttonRefresh_clicked()
{
    char proc[256], type[256];
    
    string domain("");
    int stat;

    if ((!comboDomain->currentText().isEmpty()) && (strcmp(comboDomain->currentText(), MSGTOOL_LOCAL_STR) != 0)) 
        domain = "@" + string(comboDomain->currentText().ascii());

    MsgBuf *clients = thClList(200,domain.c_str(),&stat);

    if (clients) {
        Variable * var;

        comboProcId->clear();
        comboProcId->insertItem(MSGTOOL_MSGD_STR);
        
        if (domain == "") {
            comboDomain->clear();
            comboDomain->insertItem(MSGTOOL_LOCAL_STR);
        }

        while (clients != NULL) {
            var = thValue(clients);
            sscanf(var->Value.Sv, "%s %s", proc, type);

            if (strcmp(type, "(Clnt)") == 0) {
                comboProcId->insertItem(proc);                
            } else if ((strcmp(type, "(Peer)") == 0) && (domain == "")) {
                comboDomain->insertItem(proc);                
            }
            
            clients = thNextVar(clients);
        }
        logtext("Registered clients and peers refresh done");
    }
}

//
//
int main( int argc, char ** argv )
{
    QApplication app( argc, argv );
    msgtool * gui = NULL;

    try {
        gui = new msgtool( argc, argv);

        app.setMainWidget(gui);
        gui->Exec(true); // true for trying rename
        gui->Init();
        gui->show();
        app.exec();

        delete gui;
    }
    catch (LoggerFatalException &e) {
        printf("%s\n", e.what().c_str());
    }
    catch (AOException &e) {
        Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error: %s", e.what().c_str());
    }
    return 0;
}

