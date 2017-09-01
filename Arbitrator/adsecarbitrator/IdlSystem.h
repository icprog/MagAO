#ifndef IDLSYSTEM_H_INCLUDE
#define IDLSYSTEM_H_INCLUDE

#include "framework/AbstractSystem.h"
#include "idllib/IdlCtrlInterface.h"
#include "arblib/adSecArb/AdSecPTypes.h"

using namespace Arcetri::Arbitrator;

namespace Arcetri{
namespace AdSec_Arbitrator {

class IdlSystem: public AbstractSystem {

    public:

        IdlSystem(bool simulation=false);

        virtual ~IdlSystem();

        void powerOn(int timeout_ms);
        void powerOff(int timeout_ms);
        void loadProgram(int timeout_ms);
        void setFlatFF(int timeout_ms);
        void ifAcquire(int timeout_ms);
        void ifReduce(int timeout_ms);
        void dustTest(int timeout_ms);
        void coilTest(int timeout_ms);
        void capsensTest(int timeout_ms);
        void ffAcquire(int timeout_ms);
        void ffReduce(int timeout_ms);
        void wdTest(int timeout_ms);
        void flTest(int timeout_ms);
        void psTest(int timeout_ms);
        void reset(int timeout_ms);
        void setFlatChop(int timeout_ms, flatParams params);
        void setFlatAo( float elevation, int timeout_ms);
//      void setFlatSl(int timeout_ms, flatParams params);
        void rest(int timeout_ms);
        void applyCommands(int timeout_ms, applyCommandParams mode);
        void HOOffload(int timeout_ms, applyCommandParams mode);
//      void chopToSl(int timeout_ms);
//      void aoToSl(int timeout_ms);
//      void slToAo(int timeout_ms);
        void chopToAo(int timeout_ms);
        void aoToChop(int timeout_ms);
//      void slToChop(int timeout_ms);
        void runChop(int timeout_ms);
        void stopChop(int timeout_ms);
        void runAo(int timeout_ms, runAoParams params);
        void setZernikes(int timeout_ms, setZernikesParams params);
        void stopAo(int timeout_ms, bool restoreShape);
        void pauseAo(int timeout_ms);
        void resumeAo( int timeout_ms);
        void setGain(int timeout_ms, gainParams params);
        void setDisturb(int timeout_ms, disturbParams params);
        void setRecMat(int timeout_ms, recMatParams params);
        void loadShape(int timeout_ms, shapeParams params);
        void saveShape(int timeout_ms, shapeParams params);
        void recoverFailure(int timeout_ms);

        void saveSlopes( int timeout_ms, saveSlopesParams params);
        void saveStatus( int timeout_ms, saveStatusParams params);

        void recoverSkipFrame( int timeout_ms);

        bool isGainZero();
        bool isLoopClosed();

        bool isDemoMode();

        void selectWfsPort( int timeout_ms, int input);
        void setPistonCurrent(double elevation, int timeout_ms);
        //void setTSS(bool on, int timeout_ms); NOT NEEDED. Done through AdamLib

        void processDump( int timeout_ms, string filename);

        AbstractSystemCommandResult simpleCommand( string cmdName, int timeout_ms, string idlFunc);

   private:

                /*
                 * Send a message to the IDL controller using an IdlCtrlInterface,
                 * and return true only if command succesfully executed by IDL
                 */
        AbstractSystemCommandResult sendIdlCommand(string idlCmd, int timeout_ms = 1000, bool checkStderr=false);


   private:

        AbstractArbitrator* _arbitrator;
        bool _gainZero;
        bool _loopClosed;
        bool _simulation;

};

}
}

#endif /*IDLSYSTEM_H_INCLUDE*/
