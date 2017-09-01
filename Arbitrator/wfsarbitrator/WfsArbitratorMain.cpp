//@File: WfsArbitratorMain.cpp
//
// Main function of the WfsArbitrator
//
//@

#define VERS_MAJOR 1
#define VERS_MINOR 2

#include "wfsarbitrator/WfsArbitrator.h"
#include "wfsarbitrator/WfsFsm.h"
#include "wfsarbitrator/WfsCommandImplFactory.h"

// ************************* SERIALIZED OBJECTS ********************* //
#include "arblib/base/CommandsExport.h"
#include "arblib/base/AlertsExport.h"
#include "arblib/wfsArb/WfsCommandsExport.h"
// ******************************************************************* //

using namespace Arcetri::Arbitrator;
using namespace Arcetri::Wfs_Arbitrator;

// ------------------------ MAIN ---------------------------- //

int main(int argc, char* argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

	printf("WfsArbitrator starting (main)...\n");
	WfsArbitrator* arb = NULL;

	try {
		arb = new WfsArbitrator(argc, argv);
		arb->Exec();
	}
	catch (LoggerFatalException &e) {
		// In this case the logger can't log!!!
		printf("%s\n", e.what().c_str());
	}
	catch (AOException &e) {
		printf("%s\n", e.what().c_str());
		Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error in WfsArbitrator: %s", e.what().c_str());
	}

	delete arb;
	return 0;
}
