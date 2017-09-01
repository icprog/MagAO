#include "stdconfig.h"
#include <vector>

using namespace std;
using namespace Arcetri;

int main() {

	Config_File myConfig("conf/left/arb-wfs.conf");

	vector<string> modes = myConfig["OP_MODES"];
	vector<string>::iterator iter;
	for(iter = modes.begin(); iter != modes.end(); iter++) {
		printf("'%s'\n", iter->c_str());
	}
}
