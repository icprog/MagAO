/* FSM declarations
 * Automatically generated for FSM: WfsFsmCore by FSMC vers. 1.1.1 */

#ifndef WfsFsmCore1521134217
#define WfsFsmCore1521134217
#include <queue> // for event queue
#include <string>

class WfsFsmCore {
public:
/** Events*/
   enum Event { AntiDrift, CalibratePISCESDark, ResumeLoop, OffsetXY, OptimizeGain, SaveStatus, RecoverPanic, CalibrateIRTCDark, OffsetZ, RecoverFailure, CalibrateSlopenull, PrepareAdsec, StopLoopAdsec, Off, SetSource, AutoTrack, SaveOptLoopData, CalibrateTVDark, ForceFailure, ModifyAO, Operate, CheckRef, PauseLoop, CorrectModes, StopLoop, CloseLoop, GetTVSnap, PrepareAcquireRef, EmergencyOff, EnableDisturb, CalibrateHODark, AcquireRef, ForcePanic, SetLoopFreq, RefineLoop   };
protected:
   virtual bool processCommand() const = 0; 



public:
  /** States */
   enum States { _Unknown, Operating, /*!< All WFS devices are operating correctly.*/
               Failure, LoopClosed, /*!< Adaptive optics loop is closed*/
               AOPrepared, LoopPaused, /*!< Closed loop is paused*/
               PowerOff, /*!< Main power switch is turned off. Only ethernet switch, ethernet/serial converters and relay board are powered by the 110V mains.*/
               AOSet/*!< AO light source (reference fiber or star) is acquired and the system is correctly configured for the closed loop.*/
   }; // states

private:
   States __Y;
   std::queue<Event> __events; 
   void __processEvent( Event e );
public:
   /// Constructor
   WfsFsmCore() { __Y = PowerOff;}
   /// Destructor
   virtual ~WfsFsmCore() {}
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

       Use this function to send event to WfsFsmCore
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
