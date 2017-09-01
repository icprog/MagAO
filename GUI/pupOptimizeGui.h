#ifndef PUP_OPTIMIZE_GUI_H_INCLUDED
#define PUP_OPTIMIZE_GUI_H_INCLUDED

#include "QtDesigner/pupOptimize_gui.h"
#include "arblib/base/ArbitratorInterface.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <qevent.h>
#include <qobject.h>

#include "AOAppGui.h"
using namespace Arcetri;

class pupOptimizeGui: public AOAppGui, public pupOptimize_gui {

public:
   pupOptimizeGui( string name, string configFile, KApplication &kApp);
   pupOptimizeGui( int argc, char *argv[], KApplication &kApp);

   virtual ~pupOptimizeGui();

   // Variable handler
   static int ccdBinHandler(void* thisPtr, Variable* var);
    
   RTDBvar & varCcdBin() { return _varCcdBin; }


protected:
      void PostInit();


private:
    void buttonAcquire_clicked();
    void buttonTest_clicked();
    void buttonAccept_clicked();

    ArbitratorInterface* _wfsIntf;

    int sendCommandWithReply( ArbitratorInterface *intf, Command* cmd, Command **reply=NULL);

    RTDBvar _varCcdBin;
    string  _tracknum;

};

#endif // PUP_OPTIMIZE_GUI_H_INCLUDED




