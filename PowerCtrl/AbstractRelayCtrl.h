// @File: AbstractRelayCtrl.h
//
//@

#ifndef ABSTRACT_RELAY_CTRL_H_INCLUDED
#define ABSTRACT_RELAY_CTRL_H_INCLUDED

#include "AOApp.h"
#include "PowerBoardItem.h"
#include <pthread.h>

//@Class: AbstractRelayCtrl
//
// Extends an AOApp to control the relay board
//@

class AbstractRelayCtrl: public AOApp {

public:
   AbstractRelayCtrl( int argc, char **argv) throw (AOException);

public:
   // Board operation
   int SendCommand(const char *cmd, int avoidLock = 0);
   int EmptySerial( int avoidLock = 0, int timeout = 200, bool network_check = true);

//   std::string SendCommandWaitAnswer( std::string cmd, std::string answer, int timeout = 2000, int len=-1, char terminator=0x0A);
   std::string SendCommandWaitAnswer( std::string cmd, std::string answer, int timeout = 2000, int len=-1, std::string terminator="\n");

protected:
   virtual void Create(void) throw (AOException);

   virtual void updateLoop(void) {};
   virtual int parseTemps( std::string s) { s=""; return 0; }; // avoid warning
   virtual void parseOthers( std::string s) { s=""; };  // avoid warning

   virtual void StateChange( int oldstate, int state);

   // Wrapper around SerialInString() to avoid issues with newlines
   // The arguments are the same
//   int SerialInStr( char *buf, int len, int timeout, char terminator);
   int SerialInStr( char *buf, int len, int timeout, std::string terminator);




   // VIRTUAL post init
   virtual void PostInit(void) throw (AOException);

   // local FSM
   int DoFSM(void);

   // VIRTUAL RTDB handler
   int VariableHandler( Variable *var);

protected:
   vector<PowerBoardItem *> poweritems;

  // VIRTUAL - Run
  // switches the stage from one state to the other
  virtual void Run();

  // Connects to the board
  int SetupNetwork(void);

  // Checks network connection
   virtual int TestLink(void);

protected:

   // Mutex to lock communication
   pthread_mutex_t serial_mutex; 

   // Powerboard network address
   std::string _boardNetAddr;
   int         _boardNetPort;
   std::string _boardName;

   // Keeps track of how many temperatures have been read
   int _readTemps;

public:
   bool _lastCmdOk;
};

#endif // RELAY_CTRL_H_INCLUDED


