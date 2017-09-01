/* FSM declarations
 * Automatically generated for FSM: AOArbFsmCore by FSMC vers. 1.1.1 */

#ifndef AOArbFsmCore872473175
#define AOArbFsmCore872473175
#include <queue> // for event queue
#include <string>

#include <stack> // for state stack

class AOArbFsmCore {
public:
/** Events*/
   enum Event { OffsetXY, SetZernikes, PowerOnWfs, Pause, OffsetZ, StartAO, PowerOffWfs, AcquireRefAO, PowerOnAdSec, SetSafeMode, ModifyAO, CheckRefAO, PowerOffAdSec, PresetAO, CorrectModes, RefineAO, Resume, PresetFlat, Stop, AdjustGain, SetSeeingLmtd   };
protected:
   virtual bool processCommand() const = 0; 



public:
  /** States */
   enum States { _Unknown, Failure, Operational, RefAcquired, LoopClosed, LoopSuspended, ReadyToAcquire   }; // states

private:
   States __Y;
   std::queue<Event> __events; 
   std::stack<States> __stateStack; 
   void __processEvent( Event e );
public:
   /// Constructor
   AOArbFsmCore() { __Y = Operational;}
   /// Destructor
   virtual ~AOArbFsmCore() {}
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

       Use this function to send event to AOArbFsmCore
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
