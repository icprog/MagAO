
#include "MirrorCtrlTest.h"
#include "Logger.h"
using namespace Arcetri;

int main() {

    try {
        MirrorCtrl mirrorCtrl("MirrorCtrl/Test/mirrorctrl", Logger::LOG_LEV_TRACE);
        mirrorCtrl.Exec();
    }
    catch(AOException e) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
    }
    
    return 0;
}
