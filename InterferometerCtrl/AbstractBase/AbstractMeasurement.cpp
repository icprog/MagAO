#include <string>

#include "AbstractFactory.h"
#include "AbstractMeasurement.h"

using namespace std;
using namespace Arcetri;

AbstractMeasurement::AbstractMeasurement(string remoteFilename) {
	_logger = AbstractFactory::getLogger();
	_remoteFilename = remoteFilename;
}

AbstractMeasurement::~AbstractMeasurement() {
	
}
