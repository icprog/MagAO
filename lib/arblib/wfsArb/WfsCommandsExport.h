/*
 * 						*** ATTENTION ***
 * This file contains boost intializations that must be done only once!
 * PLEASE include it in the object containing the main.
 */

#ifndef WFSCOMMANDSEXPORT_H_INCLUDE
#define WFSCOMMANDSEXPORT_H_INCLUDE

#include "arblib/wfsArb/WfsCommands.h"

#include <boost/serialization/export.hpp>

// Commands
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::Operate, 		   "Wfs_Arbitrator::Operate")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::Off, 	    	   "Wfs_Arbitrator::Off")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::PrepareAcquireRef, "Wfs_Arbitrator::PrepareAcquireRef")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::AcquireRef, 	   "Wfs_Arbitrator::AcquireRef")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::ModifyAO,   	   "Wfs_Arbitrator::ModifyAO")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::CloseLoop,  	   "Wfs_Arbitrator::CloseLoop")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::PauseLoop,  	   "Wfs_Arbitrator::PauseLoop")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::ResumeLoop, 	   "Wfs_Arbitrator::ResumeLoop")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::RefineLoop, 	   "Wfs_Arbitrator::RefineLoop")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::StopLoop,   	   "Wfs_Arbitrator::StopLoop")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::CalibrateHODark,    "Wfs_Arbitrator::CalibrateHODark")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::CalibrateMovements, "Wfs_Arbitrator::CalibrateMovements")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::CalibrateSlopenull, "Wfs_Arbitrator::CalibrateSlopenull")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::CalibrateTVDark,    "Wfs_Arbitrator::CalibrateTVDark")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::GetTVSnap,          "Wfs_Arbitrator::GetTVSnap")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::SaveStatus,          "Wfs_Arbitrator::SaveStatus")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::SaveOptLoopData,     "Wfs_Arbitrator::SaveOptLoopData")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::EnableDisturb,       "Wfs_Arbitrator::EnableDisturb")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::OffsetXY,       "Wfs_Arbitrator::OffsetXY")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::OffsetZ,       "Wfs_Arbitrator::OffsetZ")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::CorrectModes,       "Wfs_Arbitrator::CorrectModes")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::EmergencyOff,       "Wfs_Arbitrator::EmergencyOff")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::CheckRef,       "Wfs_Arbitrator::CheckRef")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::SetSource,       "Wfs_Arbitrator::SetSource")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::RequestWfsStatus,       "Wfs_Arbitrator::RequestWfsStatus")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::CalibrateIRTCDark,       "Wfs_Arbitrator::CalibrateIRTCDark")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::CalibratePISCESDark,       "Wfs_Arbitrator::CalibratePISCESDark")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::PrepareAdsec,       "Wfs_Arbitrator::PrepareAdsec")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::StopLoopAdsec,       "Wfs_Arbitrator::StopLoopAdsec")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::AntiDrift,       "Wfs_Arbitrator::AntiDrift")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::AutoTrack,       "Wfs_Arbitrator::AutoTrack")
BOOST_CLASS_EXPORT_GUID(Wfs_Arbitrator::OptimizeGain,       "Wfs_Arbitrator::OptimizeGain")

#endif /*WFSCOMMANDSEXPORT_H_INCLUDE*/
