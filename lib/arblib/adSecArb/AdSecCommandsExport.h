/*
 * 						*** ATTENTION ***
 * This file contains boost intializations that must be done only once!
 * PLEASE include it in the object containing the main.
 */

#ifndef ADSECCOMMANDSEXPORT_H_INCLUDE
#define ADSECCOMMANDSEXPORT_H_INCLUDE

#include "arblib/adSecArb/AdSecPTypes.h"
#include "arblib/adSecArb/AdSecCommands.h"

#include <boost/serialization/export.hpp>


//Parameters
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::MirrorParams, "AdSec_Arbitrator::SetMirrorParams")

// Commands
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::On, 		   "AdSec_Arbitrator::On")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::Off, 		   "AdSec_Arbitrator::Off")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::LoadProgram, "AdSec_Arbitrator::LoadProgram")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::Reset, 	   "AdSec_Arbitrator::Reset")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SetFlatChop, "AdSec_Arbitrator::SetFlatChop")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SetFlatAo,   "AdSec_Arbitrator::SetFlatAo")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SetFlatSl,   "AdSec_Arbitrator::SetFlatSl")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::Rest,        "AdSec_Arbitrator::Rest")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::ApplyCommands,  "AdSec_Arbitrator::ApplyCommands")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::ChopToSl,    "AdSec_Arbitrator::ChopToSl")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::AoToSl, 	   "AdSec_Arbitrator::AoToSl")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::AoToChop,    "AdSec_Arbitrator::AoToChop")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SlToChop,    "AdSec_Arbitrator::SlToChop")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SlToAo, 	   "AdSec_Arbitrator::SlToAo")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::ChopToAo,    "AdSec_Arbitrator::ChopToAo")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::RunChop, 	   "AdSec_Arbitrator::RunChop")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::StopChop,    "AdSec_Arbitrator::StopChop")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::RunAo, 	   "AdSec_Arbitrator::RunAo")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SetZernikes, 	"AdSec_Arbitrator::SetZernikes")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::StopAo, 	   "AdSec_Arbitrator::StopAo")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SetGain, 	   "AdSec_Arbitrator::SetGain")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SetDisturb, 	   "AdSec_Arbitrator::SetDisturb")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SetRecMat, 	   "AdSec_Arbitrator::SetRecMat")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::LoadShape, 	   "AdSec_Arbitrator::LoadShape")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SaveShape, 	   "AdSec_Arbitrator::SaveShape")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::RecoverFailure, 	   "AdSec_Arbitrator::RecoverFailure")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SaveSlopes, 	   "AdSec_Arbitrator::SaveSlopes")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SaveStatus, 	   "AdSec_Arbitrator::SaveStatus")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SetDataDecimation,  "AdSec_Arbitrator::SetDataDecimation")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SelectFocalStation,  "AdSec_Arbitrator::SelectFocalStation")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::RequestAdSecStatus,  "AdSec_Arbitrator::RequestAdSecStatus")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::RecoverSkipFrame,  "AdSec_Arbitrator::RecoverSkipFrame")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::PauseAo,   "AdSec_Arbitrator::PauseAo")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::ResumeAo,  "AdSec_Arbitrator::ResumeAo")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::TTOffload,  "AdSec_Arbitrator::TTOffload")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::SetFlatFF,  "AdSec_Arbitrator::SetFlatFF")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::IfAcquire,  "AdSec_Arbitrator::IfAcquire")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::IfReduce,  "AdSec_Arbitrator::IfReduce")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::DustTest,  "AdSec_Arbitrator::DustTest")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::CoilTest,  "AdSec_Arbitrator::CoilTest")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::CapsensTest,  "AdSec_Arbitrator::CapsensTest")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::FFAcquire,  "AdSec_Arbitrator::FFAcquire")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::FFReduce,  "AdSec_Arbitrator::FFReduce")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::WdTest,  "AdSec_Arbitrator::WdTest")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::FlTest,  "AdSec_Arbitrator::FlTest")
//BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::PsTest,  "AdSec_Arbitrator::PsTest")
BOOST_CLASS_EXPORT_GUID(AdSec_Arbitrator::CalibrateCmd,  "AdSec_Arbitrator::CalibrateCmd")

#endif /*ADSECCOMMANDSEXPORT_H_INCLUDE*/
