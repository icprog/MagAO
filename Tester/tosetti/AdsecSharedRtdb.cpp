#include "AdSecConstants.h"

using namespace Arcetri::AdSecConstants;

int main() {
	for(int i=0; i<100000; i++) {
		SharedVarsMap* vars = new SharedVarsMap();
		string varName = (*vars)["OVS_P"];
		printf("%s\n", varName.c_str());
		delete vars;
		usleep(10000);
	}
	
	return 0;
}
