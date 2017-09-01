
#include "adsecarbitrator/AdSecArbitrator.h"
#include "adsecarbitrator/AdSecFsm.h"
#include "adsecarbitrator/AdSecCommandImplFactory.h"

// ************************* SERIALIZED OBJECTS ********************* //
#include "arblib/base/CommandsExport.h"
#include "arblib/base/AlertsExport.h"
#include "arblib/adSecArb/AdSecCommandsExport.h"
// ******************************************************************* //

#define VERS_MAJOR 2
#define VERS_MINOR 3

using namespace Arcetri::Arbitrator;
using namespace Arcetri::AdSec_Arbitrator;

// ------------------------ MAIN ---------------------------- //

int main(int argc, char* argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

	printf("AdSecArbitrator starting (main)...\n");

	AdSecArbitrator* arb = NULL;
	try {
		arb = new AdSecArbitrator(argc, argv);
		arb->Exec();
	}
	catch (LoggerFatalException &e) {
		// In this case the logger can't log!!!
		printf("%s\n", e.what().c_str());
	}
	catch (AOException &e) {
		printf("%s\n", e.what().c_str());
		Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error in AdSecArbitrator: %s", e.what().c_str());
	}
   catch (...){
		Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error in AdSecArbitrator:UNKNOWN EXCEPTION");
   }

	delete arb;
	return 0;
}
