#include <math.h>

#include "Logger.h"

using namespace Arcetri;

int main() {
	
	printf("LogLine format: %s\n", LogLine::LOG_FMT.c_str());

	char line[] = "CAMERALENSCTRL       [2007-09-20 16:25:03.944557] TRACE   >> CommandSender constructed [CommandSender.cpp:38]";
	printf(">> LINE: %s\n", line);
	LogLine aLogLine(line);
	printf("<< LINE: ");
	aLogLine.print();
}
