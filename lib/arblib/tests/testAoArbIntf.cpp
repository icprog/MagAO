
#include "arblib/base/ArbitratorInterface.h"
#include "arblib/aoArb/AOCommandsExport.h"

namespace arb = Arbitrator;


int main() {

	Logger::setParentName("AOARBITRATORINTF");
	//Logger::setLogFile(Logger::LOG_NEW, "AOARBITRATORINTF-TEST", getenv("ADOPT_LOG"));
	
	// Init and start the thrdLib
	thInit("AOARBITRATORINTF");
	thStart("127.0.0.1", 5);
	thSendMsg(0, "", SETREADY, 0, NULL);
	
	// Create the interface...
	arb::ArbitratorInterface* arbIntf = new arb::ArbitratorInterface("AOARBITRATOR00", Logger::LOG_LEV_TRACE);
	
	// ...and use it!
	unsigned int loops = 1;
	for(unsigned int i=0; i<loops; i++) {
		
		arb::Command* cmd;
		arb::Command* cmdReply;
		
		// START OBS
		//cmd = new arb::StartObs(2000); 
		//cmd->setWantReply(false); // Don't wait the reply!
		//cmdReply = arbIntf->requestCommand(cmd);
		// cmdReply->log(); 
		//delete cmd;
		//delete cmdReply;
		
		sleep(2);
		
		
		// CANCEL
		// arbIntf->cancelCommand();

		
		// PRESET AO
//		SOCoordsPType soCoord; 
//		ROCoordsPType roCoord; 
//		AOModePType aoMode = "AOMODE";
//		WFSPType wfs ="FWS";
//		soCoord.push_back(1.5); soCoord.push_back(2.7);
//		roCoord.push_back(1.0); roCoord.push_back(0.7);
//		ElevationPType elevation = 34.7;
//		RotAnglePType rotAngle = 15;
//		GravAnglePType gravAngle = 12.4;
//		MagPType mag = 17;
//		ColorPType color = 3;
//		R0PType r0 = 1.1;
//		cmd = new PresetAO(200, aoMode, wfs, soCoord, roCoord, elevation, rotAngle, gravAngle, mag, color, r0); 
//		cmdReply = arbIntf->requestCommand(cmd);
//		delete cmd;
//		delete cmdReply;
//		
//		sleep(2);
//		
//		
//		// PRESET FLAT
//		cmd = new PresetFlat(100, "FLAT_1"); 
//		cmdReply = arbIntf->requestCommand(cmd);
//		delete cmd;
//		delete cmdReply;
//
//
//		// ACQUIRE REF AO
//		int result = Command::CMD_NOT_PROCESSED;
//		do {
//			NModesPType nModes = 10;
//			ITimePType iTime = 0.1;
//			NBinsPType nBins; nBins.push_back(4); nBins.push_back(4);
//			TTModPType ttMode = 1.5;	
//			FSpec1PType fSpec1 = "2";
//			FSpec2PType fSpec2 = "4";
//			sthrelPType sthrel; sthrel.push_back(0.56); sthrel.push_back(0.6); sthrel.push_back(0.7);
//			r0 = 2.3;
//			MSNratioPType mSNratio; 
//			for (unsigned int i=0; i<672; i++) {
//				mSNratio.push_back(i);
//			}
//			TVframePType tvFrame; //TBD		
//			cmd = new AcquireRefAO(500, nModes, iTime, nBins, ttMode, fSpec1, fSpec2, sthrel, r0, mSNratio, tvFrame);
//			cmdReply = arbIntf->requestCommand(cmd);
//			if(cmdReply!= NULL) {
//				result = cmdReply->getStatus();
//			}
//			delete cmd;
//			delete cmdReply;
//		}
//		while(result != Command::CMD_EXECUTION_SUCCESS);
		
		
		// RECOVER FAILURE
//		cmd = new RecoverFailure(100); 
//		cmdReply = arbIntf->requestCommand(cmd);
//		delete cmd;
//		delete cmdReply;
		
		
//		// TERMINATE OBS
//		cmd = new TerminateObs(100); 
//		cmdReply = arbIntf->requestCommand(cmd);
//		delete cmd;
//		delete cmdReply;
//		
//		
//		arbIntf->notifyWarning("Sta per piovere!");
//		usleep(100000);
//		arbIntf->notifyPanic("Alluvione in corso!");

		//arbIntf->undoCommand(100);
	}
	
	// Remember to clean up
	thClose();
	
	return 0;
}
