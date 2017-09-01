#ifndef WFSPTYPES_H_INCLUDE
#define WFSPTYPES_H_INCLUDE

#include "WfsInterfaceDefines.h"

using namespace Arcetri;

// **************************************** SERIALIZATION ******************************************** //

namespace boost {
namespace serialization {

// prepareAcquireRefParams
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::powerOnParams& params, const unsigned int /* version */) {
    ar & params.config;
	ar & params.boardSetup;
	ar & params.sourceSetup;
	ar & params.opticalSetup;
}

// prepareAcquireRefParams
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::prepareAcquireRefParams& params, const unsigned int /* version */) {
   ar & params.Instr;
   ar & params.AOMode;
	ar & params.SOCoords[0];
	ar & params.SOCoords[1];
	ar & params.ROCoords[0];
	ar & params.ROCoords[1];
	ar & params.Elevation;
	ar & params.RotAngle;
	ar & params.GravAngle;
	ar & params.Mag;
	ar & params.Color;
	ar & params.r0;
}





// modifyAOparams
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::modifyAOparams& params, const unsigned int /* version */) {
    ar & params.freq;
    ar & params.Binning;
    ar & params.TTmod;
    ar & params.Fw1Pos;
    ar & params.Fw2Pos;
    ar & params.checkCameralens;
}

// acquireRefResult
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::acquireRefResult& params, const unsigned int /* version */) {

   ar & params.deltaXY[0];
   ar & params.deltaXY[1];
   ar & params.nModes;
   ar & params.freq;
   ar & params.bin;
   ar & params.TTmod;
   ar & params.Fw1Pos;
   ar & params.Fw2Pos;
   ar & params.TVframe;
   ar & params.pupils;
   ar & params.base;
   ar & params.rec;
   ar & params.filtering;
   ar & params.gain;
   ar & params.decimation;
   ar & params.starMag;
   ar & params.CLbase;
   ar & params.CLrec;
   ar & params.CLfreq;
}

// getTVSnap
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::snapParams& params, const unsigned int /* version */) {

   ar & params.frameValid;
   ar & params.TVframe;
}

// prepareAcquireRefResult
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::prepareAcquireRefResult& params, const unsigned int /* version */) {
        
   ar & params.freq;
   ar & params.binning;
   ar & params.TTmod;
   ar & params.Fw1Pos;
   ar & params.Fw2Pos;     
   ar & params.gain;
   ar & params.pupils;
   ar & params.base;
   ar & params.rec;
   ar & params.filtering;
   ar & params.decimation;
   ar & params.nModes;
}

// getTVSnapResult
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::getTVSnapResult& params, const unsigned int /* version */) {

   ar & params.TVframe;
}

//  saveStatusParams
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::saveStatusParams& params, const unsigned int /* version */) {

   ar & params._outputFile;
}


//  saveOptLoopDataParams
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::saveOptLoopDataParams& params, const unsigned int /* version */) {

   ar & params._nFrames;
   ar & params._saveFrames;
   ar & params._saveSlopes;
   ar & params._saveModes;
   ar & params._saveCommands;
   ar & params._savePositions;
   ar & params._trackNum;
   ar & params._savePsf;
   ar & params._nFramesPsf;
   ar & params._saveIrtc;
   ar & params._nFramesIrtc;
   ar & params._savePisces;
   ar & params._nFramesPisces;
}


// wfsParams
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::wfsLoopParams& wfsloopPar, const unsigned int /*version*/) {
    ar & wfsloopPar.zmodes;
}

// enableDisturbParams
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::enableDisturbParams& params, const unsigned int /*version*/) {
    ar & params._enableWFS;
    ar & params._enableOVS;
}

// offsetXYparams
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::offsetXYparams& params, const unsigned int /*version*/) {
    ar & params.offsetX;
    ar & params.offsetY;
    ar & params.brake;
}

// offsetZparams
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::offsetZparams& params, const unsigned int /*version*/) {
    ar & params.offsetZ;
}

// correctModes
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::correctModesParams& params, const unsigned int /*version*/) {
    ar & params.modes;
}

// checkRef
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::checkRefResult& params, const unsigned int /*version*/) {
    ar & params.deltaXY;
    ar & params.starMag;
}

// setSource
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::setSourceParams& params, const unsigned int /*version*/) {
    ar & params.source;
    ar & params.magnitude;
}

// antiDrift
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::antiDriftParams& params, const unsigned int /*version*/) {
    ar & params.enable;
}

// autoTrack
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::autoTrackParams& params, const unsigned int /*version*/) {
    ar & params.rerotTrack;
    ar & params.lensTrack;
    ar & params.adcTrack;
}

// stopLoopAdsec
template<class Archive>
void serialize(Archive & ar, Wfs_Arbitrator::stopLoopAdsecParams& params, const unsigned int /*version*/) {
    ar & params.hold;
}

} // namespace serialization
} // namespace boost

// *************************************************************************************************** //


#endif /*WFSPTYPES_H_INCLUDE*/
