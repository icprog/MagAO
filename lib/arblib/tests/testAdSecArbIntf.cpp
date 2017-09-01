
#include "arblib/base/ArbitratorInterface.h"
#include "arblib/base/AlertsExport.h"
#include "arblib/adSecArb/AdSecCommandsExport.h"

using namespace Arcetri::Arbitrator;

int LOG_LEVEL = Logger::LOG_LEV_INFO;

ArbitratorInterface* _arbIntf;


/*
 * If timeout_ms is ZERO wait forever
 */
void requestSingleCommand(OpCode cmdCode, bool wantReply, int timeout_ms) {

	Command* cmd;
	Command* cmdReply;

	// Init parameters for some commands (temporary solution!!!)
	MirrorParams setMirrorParams;
  	setMirrorParams._recMatA = 	"recMatA.txt";
  	setMirrorParams._recMatB = 	"recMatB.txt";
  	setMirrorParams._aDelay = 	"aDelay.txt";
  	setMirrorParams._bDelay_A = "bDelay_A.txt";
  	setMirrorParams._bDelay_B = "bDelay_B.txt";
  	setMirrorParams._gGain_A = 	"gGain_A.txt";
  	setMirrorParams._gGain_B = 	"gGain_B.txt";
  	setMirrorParams._m2c = 		"m2c.txt";
  	setMirrorParams._nSlopes =  50;

  	modePType mode = ZERNIKE;

	// Define command
	switch(cmdCode) {

		case AdSecOpCodes::ON_CMD:
			cmd = new On(timeout_ms);
			break;

		case AdSecOpCodes::OFF_CMD:
			cmd = new Off(timeout_ms);
			break;

		case AdSecOpCodes::LOAD_PROGRAM_CMD:
			cmd = new LoadProgram(timeout_ms);
			break;

		case AdSecOpCodes::CALIBRATE_CMD:
			cmd = new Calibrate(timeout_ms);
			break;

		case AdSecOpCodes::RESET_CMD:
			cmd = new Reset(timeout_ms);
			break;

		case AdSecOpCodes::SET_FLAT_CHOP_CMD:
			cmd = new SetFlatChop(timeout_ms, setMirrorParams);
			break;

		case AdSecOpCodes::SET_FLAT_SL_CMD:
			cmd = new SetFlatSl(timeout_ms, setMirrorParams);
			break;

		case AdSecOpCodes::SET_FLAT_AO_CMD:
		cmd = new SetFlatAo(timeout_ms, setMirrorParams);
			break;

		case AdSecOpCodes::REST_CMD:
			cmd = new Rest(timeout_ms);
			break;

		case AdSecOpCodes::APPLY_MODES_CMD:
			cmd = new ApplyModes(timeout_ms, mode);
			break;

		case AdSecOpCodes::CHOP_TO_SL_CMD:
			cmd = new ChopToSl(timeout_ms, setMirrorParams);
			break;

		case AdSecOpCodes::AO_TO_SL_CMD:
			cmd = new ChopToSl(timeout_ms, setMirrorParams);
			break;

		case AdSecOpCodes::SL_TO_AO_CMD:
			cmd = new SlToAo(timeout_ms, setMirrorParams);
			break;

		case AdSecOpCodes::CHOP_TO_AO_CMD:
			cmd = new ChopToAo(timeout_ms, setMirrorParams);
			break;

		case AdSecOpCodes::AO_TO_CHOP_CMD:
			cmd = new AoToChop(timeout_ms, setMirrorParams);
			break;

		case AdSecOpCodes::SL_TO_CHOP_CMD:
			cmd = new SlToChop(timeout_ms, setMirrorParams);
			break;

		case AdSecOpCodes::RUN_AO_CMD:
			cmd = new RunAo(timeout_ms);
			break;

		case AdSecOpCodes::STOP_AO_CMD:
			cmd = new StopAo(timeout_ms);
			break;

		case AdSecOpCodes::RUN_CHOP_CMD:
			cmd = new RunChop(timeout_ms);
			break;

		case AdSecOpCodes::STOP_CHOP_CMD:
			cmd = new StopChop(timeout_ms);
			break;

		case AdSecOpCodes::SET_GAIN_CMD:
			cmd = new SetGain(timeout_ms);
			break;
	}

	// Set wantReply
	cmd->setWantReply(wantReply);

	// Request command
	cmdReply = _arbIntf->requestCommand(cmd);

	// Log reply
	if (cmdReply!= NULL) {

		cmdReply->log();
	}

	// Clean up
	delete cmd;
	delete cmdReply;

}

/*
 * Send e sequence of predefined commands
 */
void RequestMultiCommands(unsigned int loops) {

	for(unsigned int i=0; i<loops; i++) {

		requestSingleCommand(AdSecOpCodes::ON_CMD, true, 500);
		requestSingleCommand(AdSecOpCodes::OFF_CMD, true, 500);

		requestSingleCommand(AdSecOpCodes::ON_CMD, true, 1000);
		requestSingleCommand(AdSecOpCodes::LOAD_PROGRAM_CMD, true, 500);
		requestSingleCommand(AdSecOpCodes::CALIBRATE_CMD, true, 200);

		//OPERATING

		requestSingleCommand(AdSecOpCodes::SET_FLAT_CHOP_CMD, true, 500);
		requestSingleCommand(AdSecOpCodes::REST_CMD, true, 500);
//
//		requestSingleCommand(AdSecOpCodes::SET_FLAT_AO_CMD, true, 500);
//		requestSingleCommand(AdSecOpCodes::REST_CMD, true, 200);
//
//		requestSingleCommand(AdSecOpCodes::SET_FLAT_SL_CMD, true, 500);
//		requestSingleCommand(AdSecOpCodes::REST_CMD, true, 200);
//
//		//OPERATING
//
//		requestSingleCommand(AdSecOpCodes::SET_FLAT_AO_CMD, true, 500);
//		requestSingleCommand(AdSecOpCodes::RUN_AO_CMD, true, 1000);
//		requestSingleCommand(AdSecOpCodes::SET_GAIN_CMD, true, 200);
//
//		//AO RUNNING
//
//		requestSingleCommand(AdSecOpCodes::STOP_AO_CMD, true, 1000);
//
//		//---> Go to OFF
//		requestSingleCommand(AdSecOpCodes::REST_CMD, true, 200);
		requestSingleCommand(AdSecOpCodes::RESET_CMD, true, 500);
		requestSingleCommand(AdSecOpCodes::OFF_CMD, true, 500);
	}
}

int main(int argc, char** argv) {

	Logger::setParentName("ADSECARBITRATORINTF-TEST");
	Logger::setLogFile(Logger::LOG_NEW, "ADSECARBITRATORINTF-TEST", getenv("ADOPT_LOG"));

	Logger* logger = Logger::get(LOG_LEVEL);

	// Check that the arguments number is 2:
	// 	-single [command]
	//  -multi [loops]
	if (argc != 2) {
		printf("Command syntax: \n");
		printf("   testAdSecArbInterface -single [command]\n");
		printf("   testAdSecArbInterface -multi [loops]\n");
		printf("   testAdSecArbInterface -cancel\n");
		printf("   testAdSecArbInterface -panic\n");
		printf("   testAdSecArbInterface -error\n");
		printf("   testAdSecArbInterface -warning\n");
		return 0;
	}

	// Init and start the thrdLib
	thInit("ADSECARBITRATORINTF");
	thStart("127.0.0.1", 5);
	thSendMsg(0, "", SETREADY, 0, NULL);

	_arbIntf = new ArbitratorInterface("ADSECARBITRATOR00", LOG_LEVEL);

	// Check the test mode:
	// 	1. Predefined loop
	// 	2. Single command
	printf("Command: %s %s\n", argv[1], argv[2]);
	if(strcmp(argv[1],"-single") == 0) {
		int cmdCode = atoi(argv[2]);
		logger->log(Logger::LOG_LEV_INFO, "Single command code: %d", cmdCode);
		requestSingleCommand(cmdCode, true, 500); // [to change]
	}
	else if(strcmp(argv[1],"-cancel") == 0) {
		logger->log(Logger::LOG_LEV_INFO, "Cancel command...");
		_arbIntf->cancelCommand();
	}
	else if(strcmp(argv[1],"-multi") == 0) {
		unsigned int loops = atoi(argv[2]);
		logger->log(Logger::LOG_LEV_INFO, "Multi command (%d loops)", loops);
		RequestMultiCommands(loops);
	}
	else if(strcmp(argv[1],"-panic") == 0) {
		Panic panic("I'm a panic!");
        _arbIntf->notifyAlert((Alert*)(&panic));
	}
	else if(strcmp(argv[1],"-error") == 0) {
		Error error("I'm an error!");
        _arbIntf->notifyAlert((Alert*)(&error));
	}
	else if(strcmp(argv[1],"-warning") == 0) {
		Warning warning("I'm a warning!");
        _arbIntf->notifyAlert((Alert*)(&warning));
	}


	// Remember to clean up
	thClose();
	delete _arbIntf;
	Logger::destroy();


	return 0;
}
