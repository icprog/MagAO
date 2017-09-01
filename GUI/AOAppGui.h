#ifndef AOAPP_GUI_H
#define AOAPP_GUI_H

#include "AOApp.h"

#include <qthread.h>
#include <qevent.h>
#include <qobject.h>
#include <qprocess.h>
#include <qcolor.h>
#include <kapplication.h>


#include <vector>
#include <string>
using namespace std;

/*
 * This class implements all the common features of a generic AOApp used with QT
 * arbitrator control GUI.
 */


class AOAppGui: public QThread, public AOApp {


	public:

	 AOAppGui(string name, string configFile, KApplication &kApp);
	 AOAppGui(int argc, char* argv[], KApplication &kApp);
	virtual ~AOAppGui();


      virtual void Periodic() {}


	void doGui( QWidget *gui, bool rename = true, bool run= false);

        KApplication &kApp() { return _kApp; }
        QWidget *gui() { return _gui; }

	protected:	// AOApp methods

	KApplication &_kApp;
	QWidget *_gui;

	void InstallHandlers();
	static int terminate_handler(MsgBuf *msgb, void *argp, int hndlrQueueSize);

	virtual void SetupVars() {}

	virtual void PostInit() {}

	virtual void Run() {}  // AOApp Exec thread. Must not block

	virtual void run() {}  // QT thread


	protected:	// Virtual methods

	virtual void customEvent(QCustomEvent *) {}


};

#endif  // AOAPP_GUI_H
