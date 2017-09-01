#include "AOAppGui.h"

#include <cstdlib>
#include <iostream>
#include <dirent.h>


AOAppGui::AOAppGui(string name, string configFile, KApplication &kApp): QThread(), AOApp(name, configFile), _kApp(kApp) {
}


AOAppGui::AOAppGui(int argc, char* argv[], KApplication &kApp): QThread(), AOApp(argc, argv), _kApp(kApp) {
}


AOAppGui::~AOAppGui() {

}


// Called by AOApp::Exec()
void AOAppGui::InstallHandlers() {

    int stat;
    _logger->log(Logger::LOG_LEV_INFO, "AOAppGui: installing custom terminate handlers ...");
    if((stat=thHandler(TERMINATE,(char *)"*",0,terminate_handler,(char *)"terminate", this))<0) {
	_logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
	SetTimeToDie(true);
    }

}


int AOAppGui::terminate_handler(MsgBuf *msgb, void *argp, int /*hndlrQueueSize*/)
// @R: NO_ERROR
{
    AOAppGui* pt = (AOAppGui*) argp;
    Logger::get()->log(Logger::LOG_LEV_FATAL, "%s terminated by %s", pt->MyFullName().c_str(), HDR_FROM(msgb));
    thRelease(msgb);
    printf("Terminating kApp\n");
    pt->gui()->close();
    return NO_ERROR;
}

void AOAppGui::doGui( QWidget *gui, bool rename, bool run) { 

    _gui = gui;
    _kApp.setMainWidget(gui);
    this->Exec(rename);

    gui->show();

    if (run) {
	QThread *ptr = dynamic_cast<QThread*>(gui);
	ptr->start();
	}	

    _kApp.exec();
}




