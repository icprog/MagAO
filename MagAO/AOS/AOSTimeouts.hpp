
// AOSTimeouts.hpp
//
// Defines command timeouts for infividual AO Arbitrator commands
// in milliseconds
//

#ifndef AOSTimeouts_H

#define AOSTimeouts_H

// Coeeficient to compute offset timeout
#define  _OffsetSpeedFactorOL  1000.0  // Open loop/Pause value (millisec per mm)
#define  _OffsetSpeedFactorCL 30000.0  // Loop closed value (millisec per mm)

//        Predefined maximum timeouts (in millisec)
#define  _AOPresetFlatTmo     60000
#define  _AOPresetAOTmo      120000
#define  _AOCheckRefTmo      120000
#define  _AOAcquireRefTmo    300000
#define  _AORefineAOTmo      120000
#define  _AOModifyAOTmo      120000
#define  _AOStartAOTmo       600000
#define  _AOOffsetXYTmo        8000
#define  _AOOffsetZTmo         8000
#define  _AOCorrectModesTmo    2000
#define  _AOSetZernikesTmo     2000
#define  _AOStopTmo           25000
#define  _AOPauseTmo           2000
#define  _AOResumeTmo          5000
#define  _AOAdsecOnTmo       180000
#define  _AOAdsecOffTmo       60000
#define  _AOAdsecSetTmo      120000
#define  _AOAdsecRestTmo      60000
#define  _AOWfsOnTmo         240000
#define  _AOWfsOffTmo         60000


#endif
