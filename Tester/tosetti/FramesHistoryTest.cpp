
#include "mastdiagnlib.h"

using namespace Arcetri;

int main() {
	
	Logger::setParentName("FRAMESHISTORY-TEST");
	Logger::setLogFile("FRAMESHISTORY-TEST", Utils::getAdoptLog());
	Logger::get("MAIN", Logger::LOG_LEV_DEBUG);
	
	int framesNumber = 1000;
	int frameSize = 100000;
	
	// DUMP
	OFramesHistory ofh("TEST-HISTORY", framesNumber, frameSize);
	for(int i=0; i<framesNumber+200; i++) {
		BYTE frame[frameSize];
		memset(frame, i, frameSize);
		ofh.pushFrame(frame);
	}
	string outFile = ofh.dumpToFile();
	
	// LOAD
	IFramesHistory ifh(outFile);
	BYTE* frame = NULL;
	frame = ifh.getFrame(0);
	frame= ifh.getFrame(1);
	frame = ifh.getFrame(2);
	frame = ifh.getFrame(5);
	frame = ifh.getFrame(14);
	frame = ifh.getFrame(10);
	frame = ifh.getFrame(0);
	frame = ifh.getFrame(5);
	frame = ifh.getFrame(8);
	frame = ifh.getFrame(19);
	frame = ifh.getFrame(9);
}
