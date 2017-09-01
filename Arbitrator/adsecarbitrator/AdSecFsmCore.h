/* FSM declarations
 * Automatically generated for FSM: AdSecFsmCore by FSMC vers. 1.1.1 */

#ifndef AdSecFsmCore2029893588
#define AdSecFsmCore2029893588
#include <queue> // for event queue
#include <string>

class AdSecFsmCore {
public:
/** Events*/
   enum Event { SetFlatAo, SelectFocalStation, ResumeAo, Reset, SetZernikes, LoadShape, SaveStatus, LoadProgram, RecoverPanic, TTOffLoad, SetRecMat, AoToChop, RecoverFailure, RecoverSkipFrame, ApplyCommands, SetGain, Off, SetFlatChop, ForceFailure, RunAo, PauseAo, On, Calibrate, SaveSlopes, SetDisturb, SaveShape, Rest, SetDataDecimation, ForcePanic, ChopToAo, RunChop, StopAo, StopChop   };
protected:
   virtual bool processCommand() const = 0; 



public:
  /** States */
   enum States { _Unknown, Failure, AOPause, /*!< AO loop is paused*/
               Ready, /*!< In this state the DSP program is uploaded and running. The configuration for the default shell-to-refplate gap is loaded. Coils and current driver have been tested. Capacitive sensors have been tested. No significant dust contamination has been detected. ASM fast diagnostic (FST) and HSK are running and the related GUIs are available. The shell is pushed against the reference plate by the bias magnets. Coils are disabled. The LBT672 unit is ready for running calibration and engineering test procedures on AO-Supervisor request and it is ready for loading the configuration related to the operative mode that will be selected by the TCS through the AOS. This is the end-state of a successful default start-up procedure.*/
               ChopRunning, /*!< The shell is chopping between two flat shapes on external trigger*/
               AORunning, /*!< AO loop is closed*/
               PowerOff, /*!< In this status the Manual Circuit Breaker is on. The Auxiliary Power Supply (APS) is switched on. The Ethernet switch and the Ethernet controlled digital I/O are powered by APS and are correctly working. Adaptive secondary Main Power Supplies (MPSs) are switched off.*/
               ChopSet, /*!< The shell is flat and redy to set chopping mode*/
               PowerOn, /*!< In this status the MPSs is switched on. The LBT672 crates have power. The BCU, SIGGEN, DSP boards and Power Backplane (PBP) firmware has correctly boostrapped. Diagnostic communication is properly working. DSPs are in idle waiting for program uploading. Housekeeping diagnostic (HSK) is running  and the related GUI is available. The shell is pushed against the reference plate by the bias magnets. Coils are disabled.*/
               Panic, AOSet/*!< The shell is set to a "flat" shape*/
   }; // states

private:
   States __Y;
   std::queue<Event> __events; 
   void __processEvent( Event e );
public:
   /// Constructor
   AdSecFsmCore() { __Y = PowerOff;}
   /// Destructor
   virtual ~AdSecFsmCore() {}
   /** Get current FSM state
       @returns current FSM state
    */
   States currentState() { return __Y; }


   /** Get symbolic name of an event

      @param e - numeric event
      @returns symbolic event name
    */
   std::string eventName( Event e );


   /** Get symbolic name of a state

      @param e - numeric state
      @returns symbolic state name
    */
   
   std::string stateName( States s );


   /** Get state code given symbolic state name

      @param e - state name
      @returns state code (_Unknown if no state matches)
    */
   
   States stateCode( std::string stateName );


   /** Force state 

       Use this function to force a state.
       This may be useful when the FSM represents
       the state of an external process and you want
       your machine to start from that state

       @param s - numeric state
    */

   void forceState( States s );


   /** Send event to FSM

       Use this function to send event to AdSecFsmCore
       After you call it given event will be handled,
       and, if some of transition conditions match, 
       appropriate transition will be triggered,
       and currentState() will be changed. 

       If this function is called during existing event
       handling process, given event will be added to 
       pending event queue, and will be handled after current
       transition. See examples for details.
   */
   void A( Event e );
};

#endif
