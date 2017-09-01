
#ifndef WFSARB_INTERFACE_H
#define WFSARB_INTERFACE_H

#include <string>
#include <map>
using namespace std;

#include "Python.h"

// Functions exported by the Wfs Arbitrator for use by the upper level
// arbitrator or other interested parties.
//
// Definitions:
//
// HO arm:  High-Order arm of the optical bench - with pyramid and ccd39
// TV arm:  Technical viewer arm of the optical bench - with the ccd47


// Definitions for enums, etc.

#include "framework/AbstractSystem.h"
#include "arblib/wfsArb/WfsInterfaceDefines.h"
#include "Logger.h"

using namespace Arcetri::Arbitrator;

namespace Arcetri{
namespace Wfs_Arbitrator {

// Results of functions:
//
// OK    - all actions completed.
// ERROR - internal WFS error occurred, action cannot be completed.
// RETRY - actions not completed because of external circumstances (e.g. star not found)
//         Device status rollbacked to the previous one. Command may be issued again if needed.


class WfsInterface {

   private:
      string canbeNaN(float value);
      string canbeNaN(double value);

   public:

      WfsInterface();
      virtual ~WfsInterface();

   map<string, AbstractSystemCommandResult> _returnCodes;


   string getErrorString();

   AbstractSystemCommandResult test();


   // -------------------------------------------
   // Init function
   //
   // Can select between starting an embedded interpreter
   // or using an external one.

   void init( Logger *logger = NULL, int useEmbeddedPython = 1, string externalPython = "");



   // --------------------
   // Set the sensor and instrument mode
   void setSensorInstr( string sensor, string instrument);


   // ---------------- STATE CHANGE FUNCTIONS ------------------


   // -------------- setOperating() --------------------
   //
   // Goes into Operating state, with optional state description string.
   // If nothing specified (e.g. an empty string), goes into default operating state, otherwise
   // the string can describe some special status (e.g. only leftbox
   // active) selected from an available list.
   //
   // VALID STATES: PowerOff, SetAO, Operating, Stop, RecoverableError
   // END STATE   : Operating
   //
   // Returns     : error code.

   AbstractSystemCommandResult setOperating( powerOnParams params);

   // ------------- setPowerOff --------------------
   //
   // Turns off the system from whatever configuration was set.
   //
   // VALID STATES: Operating
   // END STATE   : PowerOff
   //
   // Returns:  error code

   AbstractSystemCommandResult setPowerOff();


// -------------- prepareAcquireRef --------------
//
// Prepare for reference source acquisition:
//
// 1. configure HO and TV arms for the expected star magnitude etc.
// 2. in the meantime, slew the stages to the expected position
//
// Only devices in "auto" policy will be configured (see setHOarmPolicy and setTVarmPolicy)
//
// Returns: error code

AbstractSystemCommandResult prepareAcquireRef( prepareAcquireRefParams *params, prepareAcquireRefResult *outputParams); 


// -------------- acquireRef --------------
//
// Acquire reference source and prepare WFS for closed loop.
//
// 3. once everything is in place, center the star on the TV
// 4. verify there is light on the HO arm and complete.
//
// Only devices in "auto" policy will be configured (see setHOarmPolicy and setTVarmPolicy)
//
// VALID STATES:  Operating, SetAO
// END STATE   :  SetAO
//
// Returns: error code

AbstractSystemCommandResult acquireRef(  acquireRefResult *outputParams );



// ------------- closeLoop() -----------------
//
// Quickly closes the AO loop.
// Everything is assumed ready from the SetAO previous state, so
// that this operation is very fast. It only has to activate
// the fastlink fiber.
//
// VALID STATES: SetAO
// END STATE   : LoopClosed

AbstractSystemCommandResult closeLoop();

// ------------ pauseLoop() ------------

// Quickly pauses the AO loop.
// Just opens the fastlink fiber.
//
// VALID STATES:  LoopClosed
// END STATE   :  LoopPaused
//
// Returns: error code

AbstractSystemCommandResult pauseLoop();

// ----------- resumeLoop() ------------
//
// Restarts a previously paused AO loop
//
// - checks whether there is light on the ccd
// - recloses the loop
//
// VALID STATES:  LoopPaused
// END STATE   :  LoopClosed
//
// Returns: error code

AbstractSystemCommandResult resumeLoop();


// ----------------- stopLoop ----------------
//
// Stops the AO loop. It is assumed that another acquireRef
// will be needed, therefore it will drop to Operating state.
//
// VALID STATES:  LoopClosed
// END STATE   :  Operating
//
// Returns: error code

AbstractSystemCommandResult stopLoop();



// ----------------- recoverError ------------
//
// Recover from the current Recoverable Error, if possible.
// Goes to the last Operating setting specified.
//
// VALID STATES:  RecoverableError
// END STATE   :  Operating
//
// Returns: error code

AbstractSystemCommandResult recoverError();



// --------- NON STATE-CHANGE FUNCTIONS ---------------

// -------------- setSource -------------------

// Select whether to use the internal ref. source or the telescope
// source. Both types can have several configurations available,
// all described by an unique string.
// In case of the internal source, the required equivalent magnitude
// must be specified.
//
// VALID STATES: Operating
//
// Returns: error code

AbstractSystemCommandResult setSource( string config, float magnitude);

// ----------- setHOarmPolicy -----------------
//
// Sets whether the HO devices are to be driven in automatic
// or in manual control:
//
// ccd: auto will select the best ccd freq. and binning given the
// incoming light level (either specified with magnitude etc. or
// measured at the start or at regular intervals - it's not a continous
// change)
// adc: auto will slave the ADC position to the telescope elevation and
// derotator position.
// tt policy: auto will select the best tt freq & ampl. given the ccd
// freq and known observation conditions
// fw1 policy: auto will select the best filter for observation
// rerotPolicy: auto will slave the rerot policy to the derotator
// position
//
// All these arguments have a "don't care" setting which doesn't touch
// the current setting.
// Devices not in automatic can be changed with the Engineering GUI.
// Devices not present in the current configuration (e.g. some devices are
// turned off) have a default "ignore" policy.
//
// VALID STATES: Operating, SetAO
//
// Returns: error code

AbstractSystemCommandResult setHOarmPolicy( SetHoArmPolicy hoArmPolicy);

// ------------- setTVarmPolicy ---------------
//
// Sets whether the TV elements must be configured in automatic or manul
// control. Argument meaning similar to the setHOarmPolicy.
//
// VALID STATES: all except PowerOff
//
// Returns: error code

AbstractSystemCommandResult setTVarmPolicy( SetTvArmPolicy tvArmPolicy);

// ------------- setPointAndSourcePolicy ---------------
//
// Sets whether the pointing and source elements must be configured in automatic or manul
// control. Argument meaning similar to the setHOarmPolicy.
//
// VALID STATES: all except PowerOff
//
// Returns: error code

AbstractSystemCommandResult setPointAndSourcePolicy( PointAndSourcePolicy pointAndSourcePolicy);

// ------------- modifyAO -------------
//
// Manually changes AO loop parameters.
//
// VALID STATES:  Operating, AOPrepared, AOSet
//
// Returns: error code

AbstractSystemCommandResult modifyAO( modifyAOparams params, modifyAOparams *result);


// ---------------- refineLoop ------------
//
// Changes loop parameters while the loop is closed
//
// VALID STATES:  LoopClosed
//
// Returns: error code

AbstractSystemCommandResult refineLoop( wfsLoopParams /* params */) { /* TODO */ return SysCmdSuccess; }


// ----------- ASYNCHRONOUS FUNCTIONS ---------------------
//
// Functions that can be called at any time. If another function
// is executing at the same time (e.g. in another thread) that
// function will not complete and return an error.

// -------------- setTimeout ------------
//
// Set the command timeout. This value will be used by state-change functions.

void setTimeout( int ms);

// -------------- emergencyStop -------------
//
// Call to interrupt whatever operations was in progress.
// Re-initialization may be quite long depending on the
// exact circumstances. A setOperating() command is required.
//
// VALID STATES: Any
// END STATE   : Stop
//
// Returns: error code

AbstractSystemCommandResult emergencyStop();

// -------------- emergencyOff -------------
//
// Immediately power off the WFS system. All devices will be
// powered off. Complete restart is required afterwards.
//
// VALID STATES: Any
// END STATE   : PowerOff
//
// Returns: error code

AbstractSystemCommandResult emergencyOff();



// -------------- CALIBRATION FUNCTIONS ------------

// ------------ calibAcquireRef ---------
//
// Calibrates the acquireRef function using the internal reference source.
//
// VALID STATES: Operating
//
// Returns: error code

AbstractSystemCommandResult calibAcquire();


AbstractSystemCommandResult calibrateHODark( int nframes);
AbstractSystemCommandResult calibrateIRTCDark( int nframes);
AbstractSystemCommandResult calibratePISCESDark( int nframes);
AbstractSystemCommandResult calibrateTVDark( int nframes);
AbstractSystemCommandResult calibrateMovements();
AbstractSystemCommandResult calibrateSlopenull( int nframes);
AbstractSystemCommandResult saveStatus( saveStatusParams params);
AbstractSystemCommandResult getTVSnap( snapParams *params);
AbstractSystemCommandResult savePsf( string filename, int nFrames);
AbstractSystemCommandResult saveIrtc( string filename, int nFrames);
AbstractSystemCommandResult savePisces( string filename, int nFrames);
AbstractSystemCommandResult enableDisturb( enableDisturbParams params);
AbstractSystemCommandResult offsetXY( offsetXYparams params);
AbstractSystemCommandResult offsetZ( offsetZparams params);
AbstractSystemCommandResult correctModes( correctModesParams params);
AbstractSystemCommandResult optimizeGain();
AbstractSystemCommandResult checkRef( checkRefResult *params);

int getDecimation();
float getLoopFreq();
int getBinning();
float getModulation();
string getSourceName();
float getSourceMag();
float getRangeMin();
float getRangeMax();





   // --------------- Internal stuff


protected:

   AbstractSystemCommandResult sendCommand( string command, int asyinc=0);

   string addPolicy( string device, wfsPolicy policy);
   string param( string name, string value); 
   string stringParam( string name, string value); 
   string intParam( string name, int value); 
   string commandStart( string command);
   string commandEnd();

   Logger *_logger;

   // ---- Memorize last operation outcome

   AbstractSystemCommandResult _lastCode;
   string    _lastStr;

   int _useEmbeddedPython;
   string _externalPython;
   int _externalPythonTimeout;

   ///////////////////////////
   // Embedded python data

   // ---- Main module references
   PyObject *_mainModule;
   PyObject *_mainDict;


   //////////////////////////////
   // External python data

   typedef struct
   {
      char code[32];
      char str[1024];
   } externalPythonAnswer;

   int commTest() { return 0; };

   ////////////////////////
   // Loop parameters

   void computeDecimation( float freq);
   int _decimation;
   float _loopFreq;
   int _loopBinning;
   float _loopModulation;
   float _rangemin;
   float _rangemax;

   string _sourceName;
   float _sourceMag;

   // Sensor / instrument
   string _sensor;
   string _instrument;
};


}
}

#endif // WFSARB_INTERFACE_H
