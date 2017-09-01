//@File: AOArbitratorMain.cpp
//
// Main function of the AO Arbitrator
//@

#include "aoarbitrator/AOArbitrator.h"

// Version 1.1  -- Original
// Version 2.1  -- Support for multiple WFS added. L.Fini 2/2011

#define VERS_MAJOR 2
#define VERS_MINOR 1


// ************************* SERIALIZED OBJECTS ********************* //
#include "arblib/base/CommandsExport.h"				// To receive and send basic commands (Ping, ...)
#include "arblib/base/AlertsExport.h"				// To report alerts to AOS
#include "arblib/aoArb/AOCommandsExport.h"			// To receive command from AOS
#include "arblib/wfsArb/WfsCommandsExport.h"		// To send commands to WfsArbitrator
#include "arblib/adSecArb/AdSecCommandsExport.h"	// To send commands to AdSecArbitrator
// Need WfsArbitrator interface to send commands to WfsArbitrator
// ******************************************************************* //


using namespace Arcetri::Arbitrator;


int main( int argc, char **argv) {

   SetVersion(VERS_MAJOR,VERS_MINOR);
   AOArbitrator* arb = NULL;

	printf("AOArbitrator starting (main)...\n");

   try {
      arb = new AOArbitrator(argc, argv);
      arb->Exec();
   }
	catch (AOException& e) {
      Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error in AOArbitrator: %s", e.what().c_str());
   }
  
   catch (const std::exception& e) {
    	Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Generic exception: %s", e.what());
   }

}
