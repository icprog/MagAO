//+File: AbstractRelayCtrl.cpp
//
// Control program for the relay and temperature board
//-

#include "AbstractRelayCtrl.h"
#include "Logger.h"
#include "AOStates.h"

#ifdef _WIN32
#include "winsock2.h"
typedef unsigned int socklen_t;
#undef NO_ERROR
#endif

extern "C" {
#include <stdio.h>
#include <string.h>		// strncmp()
#include <stdlib.h>		// atoi(), atof()
#include <stdarg.h>
#include <time.h>		// time(), localtime()
//#include <unistd.h>		// sleep()
#include <pthread.h>
#include <errno.h>

#include "hwlib/netseriallib.h"


// General Supervisor libs
#include "base/msglib.h"
}

#include "RTDBvar.h"
#include "stdconfig.h"
#include "AOStates.h"

#define BUFSIZE (1024)

AbstractRelayCtrl::AbstractRelayCtrl( int argc, char **argv) throw (AOException): AOApp( argc, argv)
{
   Create();
}

void AbstractRelayCtrl::Create() throw (AOException)
{
   _lastCmdOk = false;

    try {
        _boardNetAddr = (std::string)ConfigDictionary()["boardNetAddr"];
        _boardNetPort = ConfigDictionary()["boardNetPort"];
        _boardName = (std::string) ConfigDictionary()["boardName"];
    } catch (Config_File_Exception &e) {
       _logger->log( Logger::LOG_LEV_FATAL, e.what());
       throw AOException("Fatal: missing configuration data");
    }

}



void AbstractRelayCtrl::PostInit() throw (AOException)
{
    pthread_mutex_init( &serial_mutex, NULL);

    try {
        // Create the individual power items
        int num_items = ConfigDictionary()["num_items"];

        for (int i=0; i<num_items; i++)
            {
            char item_name[32];
            sprintf( item_name, "item%d", i);
            Config_File *cfg = ConfigDictionary().extract( item_name);
            PowerBoardItem *item = new PowerBoardItem( this, *cfg);
            poweritems.push_back(item);
            }

    } catch (Config_File_Exception &e) {
       _logger->log( Logger::LOG_LEV_FATAL, e.what());
       throw AOException("Fatal: missing configuration data");
    }

    _logger->log( Logger::LOG_LEV_DEBUG, "AbstractRelayCtrl created");
    printf("created\n");
}

void AbstractRelayCtrl::Run()
{
   while (TimeToDie() == false)
   {
      try {
         DoFSM();
         
         EmptySerial();
      } catch (AOException &e) {
         _logger->log( Logger::LOG_LEV_DEBUG, "Run(): caught exception %s", e.what().c_str());

         // When the exception was thrown, the mutex may be held!
         pthread_mutex_unlock( &serial_mutex);
      }
   }
}



//@Function
//
// DoFSM()               Manages controller states 
//
// Switches the controller from one state to another. States can be changed asynchronously
// from this thread or the listening thread, and this function will properly react to a new state.
//
// Many states include a msleep() function to slow down the thread when immediate action is not necessary
//@

int AbstractRelayCtrl::DoFSM(void)
{
    int stat = NO_ERROR;
    int status = getCurState();

    // Drop into no connection state for any connection problem
    if (_lastCmdOk == false)
       setCurState(STATE_NOCONNECTION);

	 switch(status)
      {
      // At first, try to start network
      case STATE_NOCONNECTION:
      stat = SetupNetwork();
      if (stat == NO_ERROR)
          {
          // Clean up boot message
          for (int i=0; i<3; i++)
             EmptySerial(0, 200, false);
          if (TestLink() == NO_ERROR)
            setCurState(STATE_OPERATING);
          }

      break;

		// When ready to go, read temperatures every now and then
		case STATE_OPERATING:
      updateLoop();
		msleep(1000);
		break;

      // Unknown states should not exist
      default:
      msleep(3000);
      break;
      }

     // Always set the current state (for watchdogs)
     setCurState(getCurState());

    // Return the generated error code, if any
     return stat;
}


int AbstractRelayCtrl::VariableHandler( Variable *var)
{
   unsigned int i;

   int status = getCurState();
   if (status != STATE_OPERATING)
      {
      _logger->log( Logger::LOG_LEV_ERROR, "Command refused: no network connection");
      throw AOException("Can't send command: state is not OPERATING", NETWORK_ERROR, __FILE__, __LINE__);
      }
   
   for (i=0; i< poweritems.size(); i++)
      poweritems[i]->HandleRequest(var);

   return NO_ERROR;
}

// +Entry
//
// SetupNetwork       setup network connection with LittleJoe
//
// This function sets up the network connection with the LittleJoe CCD Camera.
// After the network is up, the communication is tested with the TestJoe() function.
//
// Return value: zero or a negative error code.

int AbstractRelayCtrl::SetupNetwork()
{
	int result;

	// Close the previous connection, if any
	SerialClose();
	msleep(1000);

	// Setup serial/network interface
	_logger->log( Logger::LOG_LEV_INFO, "Connecting to %s, %d", (char *) _boardNetAddr.c_str(), _boardNetPort);

	if (( result = SerialInit( (char *)_boardNetAddr.c_str(), _boardNetPort)) != NO_ERROR)
		{
		_logger->log( Logger::LOG_LEV_ERROR, "Error configuring network, errno=%s", strerror(errno));
		return NETWORK_ERROR;
		}

	_logger->log( Logger::LOG_LEV_INFO, "Network reconfigured OK");	

//	stat = TestLink();
//	if (stat)
//		{
//		_logger->log( Logger::LOG_LEV_ERROR, "Error during network testing testing: (%d) %s", stat, (char *)lao_strerror( stat));
//		return stat;
//		}

	return NO_ERROR;
}

//+Function
//
// TestLink         test serial communication with CPE board

int AbstractRelayCtrl::TestLink(void)
{
   try {
      SendCommandWaitAnswer("z", "Retry", 1000, 18, "!"); // A command not recognized that triggers an error reply
      _logger->log( Logger::LOG_LEV_TRACE, "TestLink() successful");
   } catch (AOException &e) { 
      _logger->log( Logger::LOG_LEV_TRACE, "TestLink() failed");
    }

   if (_lastCmdOk)
      return NO_ERROR;
   else
      return NETWORK_ERROR;
}

// Sends a command and waits for the corresponding answer.
// The answer string is returned as a result.
// Throws an exception if a timeout occurs
// Terminator defaults to 0x0A
/*
std::string AbstractRelayCtrl::SendCommandWaitAnswer( std::string cmd, std::string answer, int timeout, int len, char terminator)
{
    std::string s;
    s = terminator;

    return SendCommandWaitAnswer( cmd, answer, timeout, len, s);
}
*/

std::string AbstractRelayCtrl::SendCommandWaitAnswer( std::string cmd, std::string answer, int timeout, int len, std::string terminator)
{
    static int faults=0;	// Consecutive comm. faults.

    char buffer[BUFSIZE];

   _logger->log( Logger::LOG_LEV_DEBUG, "SendCommand: %s   Answer: %s", cmd.c_str(), answer.c_str());

   pthread_mutex_lock( &serial_mutex);
   SendCommand( (char *)cmd.c_str(), 1);

   _logger->log( Logger::LOG_LEV_DEBUG, "Waiting for answer: %s", answer.c_str());

   int n;
   int ss = BUFSIZE-1;
   if (len>=1)
      ss = len;
	while( (n= SerialInStr( buffer, ss, timeout, terminator)) > 0)
      {
      _logger->log( Logger::LOG_LEV_DEBUG, "Received string (len %d): %s", n, buffer);

      std::string s = std::string(buffer, strlen(buffer));
      if (s.find(answer) != std::string::npos)
         {
         //_logger->log( Logger::LOG_LEV_TRACE, "Answer received, returning");
         pthread_mutex_unlock( &serial_mutex);
         _lastCmdOk = true;
	 faults=0;
         return s;
         }
      _logger->log( Logger::LOG_LEV_DEBUG, "Looping");
      }

   //_logger->log( Logger::LOG_LEV_TRACE, "No answer received");

   pthread_mutex_unlock( &serial_mutex);
   if (++faults >1)
	{
   	_lastCmdOk = false;
   	throw AOException( "Timeout waiting for answer from PIC", TIMEOUT_ERROR, __FILE__, __LINE__);
	}
   else
	{
	// Retry command 
	_logger->log( Logger::LOG_LEV_WARNING, "Command timeout, retrying...");
	return SendCommandWaitAnswer(cmd, answer, timeout, len, terminator);
	}
}

//+Function
//
// SendCommand     sends a command to PIC board
//-

int AbstractRelayCtrl::SendCommand(const char *cmd, int avoidLock)
{
	int stat;
	char buf_0A[1];

	buf_0A[0] = 0x0A;	// Cmd terminator

	// Be sure that nothing is inside the serial line
	//EmptySerial(avoidLock);
   _logger->log( Logger::LOG_LEV_TRACE, "Sending command \"%s\"", cmd);

   // Lock serial communication
   if (!avoidLock)
      pthread_mutex_lock( &serial_mutex);

	if ((stat = SerialOut( cmd, strlen(cmd))) == NO_ERROR)
		if (cmd[ strlen(cmd)-1] != 0x0A)
         {
         if ((std::string) ConfigDictionary()["add_0A"] == "on")
            {
            _logger->log( Logger::LOG_LEV_TRACE, "Adding 0x0A terminator");
            stat = SerialOut( buf_0A, 1);
			   stat = SerialOut( buf_0A, 1);
            }
         }

   // unlock serial communication
   if (!avoidLock)
        pthread_mutex_unlock( &serial_mutex);

	return stat;
}

//+Function
//
// EmptySerial
//
// Empties the input serial port buffer.
// If output data about the temperature sensors
// is found, update that data too


int AbstractRelayCtrl::EmptySerial( int avoidLock, int timeout, bool network_check)
{
	char buffer[BUFSIZE];
   int status;

   if (network_check)
      {
      status = getCurState();
	   if (status != STATE_OPERATING)
         return NO_ERROR;
      }

   // Lock serial communication
   if (!avoidLock)
      pthread_mutex_lock( &serial_mutex);

	int n;
	while( (n= SerialInStr( buffer, BUFSIZE-1, timeout, "\n")) > 0)
      {
      _logger->log( Logger::LOG_LEV_TRACE, "EmptySerial(): received string: %s", buffer);

      _readTemps += parseTemps((std::string)buffer);
      }

   // unlock serial communication
   if (!avoidLock)
      pthread_mutex_unlock( &serial_mutex);

	return NO_ERROR;
}
/*
int AbstractRelayCtrl::SerialInStr( char *buf, int len, int timeout, char terminator)
{
    std::string s;
    s = terminator;

    return SerialInStr( buf, len, timeout, terminator);
}
*/

int AbstractRelayCtrl::SerialInStr( char *buf, int len, int timeout, std::string terminator)
{
   static std::string s = "";
   unsigned int pos;

   // Call the underlying function if no data was ready
   if ((s.size()==0) || (s.find(terminator) == std::string::npos))
      {
      int n = SerialInString2( buf, len-s.size(), timeout, (char *)terminator.c_str());
      _logger->log( Logger::LOG_LEV_TRACE, "SerialInString received (len %d) %s", n, buf);

      if (n>0)
         s = s + std::string( buf, n);
      }

   _logger->log( Logger::LOG_LEV_TRACE, "SerialInString buffer (len %d) %s", s.size(), s.c_str());

   // If still no data, return
   pos = s.find(terminator);
   if ((pos == std::string::npos) && (s.size() < (unsigned int)len))
      return 0;

   // Copy including the terminator character
   if (pos == std::string::npos)
      pos = s.size();

   if (pos > (unsigned int)len)
       pos = len;
   memcpy( buf, s.c_str(), pos+1);
   buf[pos+1] = 0;

   if (pos < s.size()-1)
      s = s.substr(pos+1);
   else
      s = "";

   return pos+1;

}

//@Function: StateChange
//
// Automatically called when our status change.
// Updates power status to unreacheable or not depending on state transition.
//@

void AbstractRelayCtrl::StateChange( int oldstate, int state)
{
   unsigned int i;
   static int first=1;

   if ((state == STATE_NOCONNECTION) && (oldstate != STATE_NOCONNECTION)) {
      try {
         for (i=0; i< poweritems.size(); i++)
           poweritems[i]->forceStatus( PWRSTATUS_NOTREACHABLE);
      } catch (AOException &e) {
          Logger::get()->log( Logger::LOG_LEV_ERROR, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
      }
   }


   if ((state != STATE_NOCONNECTION) && (oldstate == STATE_NOCONNECTION)) {
      if (!first) {
        // Re-enable items like they were, but not the first time!
        first = 0;
        try {
         for (i=0; i< poweritems.size(); i++)
           poweritems[i]->Retry();
        } catch (AOException &e) {
          Logger::get()->log( Logger::LOG_LEV_ERROR, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
        }
      } else {
       // The first time, set everything to OFF
        try {
         for (i=0; i< poweritems.size(); i++)
           poweritems[i]->forceStatus( PWRSTATUS_OFF);
        } catch (AOException &e) {
          Logger::get()->log( Logger::LOG_LEV_ERROR, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
        }
     }

  }
}

