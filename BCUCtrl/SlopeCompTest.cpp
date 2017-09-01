//+File: SlopeCompTest.c
//
// Slope computer controller program
//

#include <string>

extern "C" {
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>    // NAN

#include <pthread.h>
 
#include "base/thrdlib.h"

#include "commlib.h"
#include "iolib.h"
}


#include "RTDBvar.h"
#include "BcuLib/BCUmsg.h"
#include "BCUfile.h"
#include "BCUlbt.h"
#include "SlopeComp.h"
#include "SlopeCompTest.h"
#include "AOStates.h"
#include "BcuLib/bcucommand.h"
#include "Paths.h"


extern "C" {
#define NIOS_WFS
#include "../Housekeeper/HouseKeeperRawStruct.h"
}
using namespace Arcetri::HouseKeeperRaw;


// **************************************
// Client name and versioning information
// **************************************

int VersMajor = 1;
int VersMinor = 0;
const char *Date = "Jun 2011";
const char *Author = "A. Puglisi";


int CHAR_VAR_LEN = 128;			// Length of a "filename variable
int ERRMSG_LEN = 128;			// Length of the error message variable

int seqnum = 0;

// Define this to use the command history on the slope computer
// instead of the switch BCU
//#define USE_COMMANDHISTORY

//+Function: help
//
// Prints an usage message
//-
// Rimosso perche' non usato (L.F.)
//static void help()
//{
//    printf("SlopeCompTest  - Vers. %d.%d.  %s, %s\n\n", VersMajor,VersMinor,Author,Date);
//    printf("Usage: SlopeCompTest [-v] [-s <Server Address>]\n\n");
//    printf("   -s    specify server address (defaults to localhost)\n");
//    printf("   -v    verbose mode\n");
//    printf("\n");
//}


// +Main: 
//
// Main function
//-

SlopeCompTest::SlopeCompTest( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
}

SlopeCompTest::~SlopeCompTest() {
}

void SlopeCompTest::Create() throw (AOException)
{
   _bcuNum = ConfigDictionary()["bcuNum"];
    wfs = new SlopeComp( _bcuNum, (char*)MyFullName().c_str());

}

void SlopeCompTest::Run()
{
   int period = 50;   // Interval between fastlink on/off in ms

   printf("Starting fastlink test at interval of %d ms\n", period);
   printf("Press Ctrl-C to stop\n");

   for (;;) {

         wfs->ParamBlockSelector_enable( PBS_FASTLINK);
         msleep(period);

         wfs->ParamBlockSelector_disable( PBS_FASTLINK);
         msleep(period);
   }


}



void SlopeCompTest::SetupVars()
{ }


//@Function: TestComm
//
// Tests the communication with the BCU hardware. Since this
// process is not intended to directly communicate with
// the BCU, a successful test requires also the appropriate
// BCU controller to be running.
//
// Returns NO_ERROR if ok or an error code.
//@

int SlopeCompTest::TestComm(void)
{
	int buffer;
   try {
      return wfs->GetBuffer( _bcuNum, 0xFF, 0xFF, (unsigned char *)&buffer, 4, 0, MGP_OP_RDSEQ_SDRAM, 0, 1000);
   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error in TestComm(): %s", e.what().c_str());
      return COMMUNICATION_ERROR;
   }
}	




//@Function: safeStop and safeStart
//
// These two function can stop and re-start the DSP loop
// always keeping up-to-date status information in the MsgD-RTDB.
//
// The loop is re-started only if it was already running before,
// otherwise the safeStart() function does nothing. It needs the
// <previous state> parameter, returned by the safeStop() function.
//@

int SlopeCompTest::safeStop( int *previous_state)
{
	int status = getCurState();

	// Always force stop, for now
	wfs->stopDSP();


	if (previous_state)
	 *previous_state = status;

	if (status == STATE_OPERATING)
		setCurState(STATE_READY);

	// Wait until the longest possible frame has been processed. Min speed = 100 hz
	usleep( 100*1000);

	return NO_ERROR;
}	

int SlopeCompTest::safeStart( int previous_state)
{

	int status = getCurState();

	if ((status == STATE_READY) && (previous_state == STATE_OPERATING)) {
		usleep(50*1000);
		wfs->startDSP();
		setCurState(STATE_OPERATING);
	}

	return NO_ERROR;
}

int main( int argc, char **argv) {

   SetVersion(VersMajor,VersMinor);

   try {
      SlopeCompTest *c;

      c = new SlopeCompTest( argc, argv);

      c->Exec();

      delete c;
   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}

