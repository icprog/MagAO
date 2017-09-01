
#include "AbstractFactory.h"
#include "AbstractMeasurementBurst.h"

using namespace Arcetri;

AbstractMeasurementBurst::AbstractMeasurementBurst(int measNum, string remotePath) {
	_logger = AbstractFactory::getLogger();
	_size = measNum;
	_remotePath = remotePath;
}

AbstractMeasurementBurst::~AbstractMeasurementBurst() {
	
}
