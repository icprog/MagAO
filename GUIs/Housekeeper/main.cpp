
#define VERS_MAJOR   1
#define VERS_MINOR   0

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


#include <qapplication.h>
#include "Housekeeper_gui.h"




int main( int argc, char ** argv )
{

    QApplication a( argc, argv );
    string id ="housekeepergui";
    ostringstream oss;
    oss << "conf/adsec/current/processConf/" << id << ".conf";

    SetVersion(VERS_MAJOR,VERS_MINOR);

    GUILBT672 w(id, oss.str());

    w.Exec(true);       //start AOApp
    w.Refresh();    //start polling of Hausekeeper information
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

    int res = a.exec();
    a.quit();
    Logger::destroy();
    return res;
}
