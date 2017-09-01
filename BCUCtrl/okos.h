//+File: okos.h
//
// Header file with OKO mirror configuration information
//

#define OKO_CHANNELS 40

struct oko_config
{
	int null;
	int saturation;
	int bias;
	int flat[ OKO_CHANNELS];
};
	 

	  
