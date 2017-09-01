
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


#include <qapplication.h>
#include "FastAdSecMir_gui.h"


#define VERS_MAJOR   1
#define VERS_MINOR   0


int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    string id = "adsecmirrorgui";
    ostringstream oss;
    oss << "conf/adsec/current/processConf/" << id << ".conf";

    cout << oss.str();

    SetVersion(VERS_MAJOR,VERS_MINOR);
    GUIadSecM w( id, oss.str());

    w.Exec(true);       //start AOApp
    w.Refresh();    //start polling of FastDiagn information
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

    int res = a.exec();
    Logger::destroy();
    return res;
}
