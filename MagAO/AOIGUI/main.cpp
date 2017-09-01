#include <qapplication.h>
#include "aoigui.uic.h"

using namespace std;

int main( int argc, char ** argv )
{
   char cSide = 'l';

   QApplication appAOIGUI( argc, argv );
   AOIGUIForm guiAOIGUI;
   guiAOIGUI.setSide( cSide );
   guiAOIGUI.show();
   appAOIGUI.connect( &appAOIGUI, SIGNAL( lastWindowClosed() ), &appAOIGUI, SLOT( quit() ) );
   return appAOIGUI.exec();
 
}
