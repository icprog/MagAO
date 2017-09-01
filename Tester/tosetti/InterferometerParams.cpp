#include "IntfLib/Commands.h"

using namespace Arcetri;

int main() {
	
	MeasurementParams mp = ParamsFactory::CreateMeasurementParams("MY_VERYVERYLONG_VERYVERYLONG_VERYVERYLONG_VERYVERYLONG_VERYVERYLONG_MEAS");
	printf("'%s'\n", mp.measPrefix);
	
	MeasurementBurstParams mbp = ParamsFactory::CreateMeasurementBurstParams(100, "MY_BURST");
	printf("%d '%s'\n", mbp.measNum, mbp.burstName);
	
	PostProcessParams ppp = ParamsFactory::CreatePostProcessParams("SINGLE_MEAS/PIPPO", "MY_BURST", 50);
	printf("'%s' '%s' %d \n", ppp.remotePath, ppp.measPrefix, ppp.measNum);
	
	return 0;
}